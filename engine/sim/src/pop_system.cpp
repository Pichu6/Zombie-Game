#include "zs/pop_system.h"

#include <stdexcept>

namespace zs {

PopRegistry& PopSystem::setup(World& w) {
  if (!w.ledger.setup_open()) throw std::logic_error("pops: setup is closed; change pops by command");
  return registry_;
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
    case CohortApply:
      ok = registry_.apply({c.subject, kind, c.b, year, cause});
      if (ok) w.emit(EventType::PopEffect, c.subject, cause, c.a, c.b);
      break;
    default: known = false;
  }
  if (known && !ok) w.emit(EventType::PopCommandRefused, c.subject, cause, c.type);
}

void PopSystem::update(World& w) {
  if (!w.check_every_tick) return;
  if (const auto problem = registry_.audit()) throw std::logic_error("pops: " + *problem);
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
