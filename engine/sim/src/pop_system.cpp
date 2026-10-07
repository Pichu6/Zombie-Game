#include "zs/pop_system.h"

#include <stdexcept>

#include "zs/pop_body.h"

namespace zs {

PopRegistry& PopSystem::setup(World& w) {
  if (!w.ledger.setup_open()) throw std::logic_error("pops: setup is closed; change pops by command");
  return registry_;
}

void PopSystem::attach_bodies(World& w, Bodies& bodies) {
  if (!w.ledger.setup_open()) throw std::logic_error("pops: bodies may only be attached during setup");
  bodies_ = &bodies;
}

void PopSystem::refuse(World& w, const Command& c, EventId cause, Refusal why) {
  w.emit(EventType::PopCommandRefused, c.subject, cause, c.type, static_cast<std::int64_t>(why));
}

void PopSystem::embody(World& w, const Command& c, EventId cause) {
  if (!bodies_) return refuse(w, c, cause, Refusal::NoBodies);
  const double year = year_of_tick(w.clock.tick);
  PopSnapshot snap;
  if (!registry_.snapshot(c.subject, year, snap)) return refuse(w, c, cause, Refusal::NoSuchPop);
  if (year < snap.base.birth_year || year >= snap.base.death_year) return refuse(w, c, cause, Refusal::Dead);
  if (registry_.embodied_body(c.subject) != 0) return refuse(w, c, cause, Refusal::AlreadyEmbodied);
  if (bodies_->free_slots() == 0) return refuse(w, c, cause, Refusal::PoolFull);
  const std::string label = "pop " + std::to_string(c.subject);
  EntityId body = 0;
  bool restored = false;
  if (const auto* saved = registry_.saved_body(c.subject)) {
    ArchiveReader r(*saved);
    BodyState state = read_body_state(r);
    // The tick of the demotion follows the state in the blob (older blobs have none: no gap applied).
    if (!r.at_end()) {
      const std::int64_t left_at = r.i64();
      AdvancedBody adv = advance_demoted(state, left_at, w.clock.tick, w.rng);
      if (adv.death) {  // the pop died while it was a record: the registry takes the exception
        registry_.set_death_year(c.subject, year_of_tick(adv.death_tick));
        registry_.end_embodied(c.subject, {});
        w.emit(EventType::PopDiedDemoted, c.subject, cause, static_cast<std::int64_t>(*adv.death), adv.death_tick);
        return refuse(w, c, cause, Refusal::Dead);
      }
      state = adv.state;
    }
    body = bodies_->embody_state(w, state, c.subject, cause);
    restored = true;
  } else {
    body = bodies_->embody(w, body_spec_for(w.rng.seed(), c.subject, year - snap.base.birth_year, snap.base.sex), label, c.subject, cause);
  }
  registry_.begin_embodied(c.subject, body, w.clock.tick);
  w.emit(EventType::PopEmbodied, c.subject, cause, static_cast<std::int64_t>(body), restored ? 1 : 0);
}

void PopSystem::disembody(World& w, const Command& c, EventId cause) {
  if (!bodies_) return refuse(w, c, cause, Refusal::NoBodies);
  const EntityId body = registry_.embodied_body(c.subject);
  if (body == 0) return refuse(w, c, cause, Refusal::NotEmbodied);
  if (w.clock.tick - registry_.embodied_since(c.subject) < dwell_ticks_) return refuse(w, c, cause, Refusal::Dwell);
  if (!bodies_->cannot_disembody(w, body).empty()) return refuse(w, c, cause, Refusal::BodyBusy);
  ArchiveWriter out;
  write_body_state(out, bodies_->disembody(w, body, cause));
  out.i64(w.clock.tick);  // when it left: the next embody advances the body over the gap (spec 29 rule 6)
  registry_.end_embodied(c.subject, out.bytes());
  w.emit(EventType::PopDisembodied, c.subject, cause, static_cast<std::int64_t>(body), 0);
}

void PopSystem::apply_command(World& w, const Command& c, EventId cause) {
  const double year = year_of_tick(w.clock.tick);
  const auto kind = static_cast<EffectKind>(c.a);
  bool known = true, ok = false;
  switch (c.type) {
    case PopPromote: ok = registry_.promote(c.subject); break;
    case PopSetOffice: ok = registry_.set_office(c.subject, static_cast<std::uint32_t>(c.a)); break;
    case PopSetFollowed: ok = registry_.set_followed(c.subject, c.a != 0); break;
    case RosterJoin: ok = registry_.roster_join(static_cast<std::uint64_t>(c.a), c.subject, year); break;
    case RosterLeave: ok = registry_.roster_leave(static_cast<std::uint64_t>(c.a), c.subject, year); break;
    case RosterApply:
      ok = registry_.roster_apply(c.subject, kind, c.b, year, cause);
      if (ok) w.emit(EventType::PopEffect, c.subject, cause, c.a, c.b);
      break;
    case RegionApply:
      ok = registry_.region_apply(static_cast<std::uint32_t>(c.subject), static_cast<EffectKind>(c.a & 0xff), c.b, year, cause,
                                  (c.a >> 8) ? std::optional<RosterKind>(static_cast<RosterKind>((c.a >> 8) - 1)) : std::nullopt);
      if (ok) w.emit(EventType::PopEffect, c.subject, cause, c.a, c.b);
      break;
    case PopEmbody: embody(w, c, cause); return;
    case PopDisembody: disembody(w, c, cause); return;
    case CohortApply:
      ok = registry_.apply({c.subject, kind, c.b, year, cause});
      if (ok) w.emit(EventType::PopEffect, c.subject, cause, c.a, c.b);
      break;
    default: known = false;
  }
  if (known && !ok) w.emit(EventType::PopCommandRefused, c.subject, cause, c.type);
}

std::optional<std::string> PopSystem::audit_bodies() const {
  if (!bodies_) return std::nullopt;
  const auto list = registry_.embodied_list();
  std::size_t occupied = 0;
  for (EntityId id : bodies_->slots()) occupied += bodies_->get(id).vacant ? 0 : 1;
  if (occupied != list.size()) return "embodied pops and occupied body slots differ in number";
  for (const auto& [pop, body] : list) {
    if (!bodies_->exists(body) || bodies_->get(body).vacant) return "an embodied pop holds a vacant slot";
    if (bodies_->get(body).pop != pop) return "a body slot belongs to another pop than the one that holds it";
  }
  return std::nullopt;
}

void PopSystem::update(World& w) {
  if (bodies_) {  // a pop whose body died while embodied is dead; its slot is freed (spec 29 rule 4)
    for (const auto& [pop, body] : registry_.embodied_list()) {
      if (bodies_->get(body).alive) continue;
      const EventId ev = w.emit(EventType::PopDisembodied, pop, 0, static_cast<std::int64_t>(body), 1);
      bodies_->release_dead(w, body, ev);
      registry_.set_death_year(pop, year_of_tick(w.clock.tick));
      registry_.end_embodied(pop, {});
    }
  }
  if (!w.check_every_tick) return;
  if (const auto problem = registry_.audit()) throw std::logic_error("pops: " + *problem);
  if (const auto problem = audit_bodies()) throw std::logic_error("pops: " + *problem);
}

void PopSystem::save(ArchiveWriter& w) const {
  const auto bytes = registry_.save();
  w.u64(bytes.size());
  for (std::uint8_t b : bytes) w.u8(b);
}

void PopSystem::load(ArchiveReader& r) {
  std::vector<std::uint8_t> bytes(r.u64());
  for (auto& b : bytes) b = r.u8();
  registry_.load(bytes);
}

}  // namespace zs
