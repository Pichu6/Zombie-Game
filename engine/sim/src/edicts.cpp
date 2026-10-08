// Spec: docs/tier0/specs/39-orders-and-belief.md
#include "zs/edicts.h"

#include <stdexcept>

namespace zs {

std::uint32_t Edicts::add_def(EdictDef d) {
  if (d.per_mille < 0 || d.per_mille > 1000) throw std::invalid_argument("Edicts::add_def: per mille out of range");
  defs_.push_back(std::move(d));
  return static_cast<std::uint32_t>(defs_.size() - 1);
}

std::size_t Edicts::sign(std::uint32_t def, OwnerId signer, std::int64_t tick, EventId signed_event) {
  if (def >= defs_.size()) throw std::out_of_range("Edicts::sign: unknown def");
  Edict e;
  e.def = def;
  e.signer = signer;
  e.signed_tick = tick;
  e.signed_event = signed_event;
  all_.push_back(e);
  return all_.size() - 1;
}

bool Edicts::repeal(std::size_t edict, std::int64_t tick) {
  if (edict >= all_.size() || !all_[edict].in_force()) return false;
  all_[edict].repealed_tick = tick;
  return true;
}

std::vector<EventId> Edicts::trace(const EventLog& log, EventId event) {
  std::vector<EventId> chain;
  EventId cur = event;
  while (cur != 0 && chain.size() < 64) {
    const Event* e = log.get(cur);
    if (!e) break;
    chain.push_back(cur);
    cur = e->cause;
  }
  return chain;
}

std::optional<std::size_t> Edicts::behind(const EventLog& log, EventId event) const {
  for (EventId id : trace(log, event)) {
    const Event* e = log.get(id);
    if (!e || e->type != EventType::EdictSigned) continue;
    for (std::size_t i = 0; i < all_.size(); ++i)
      if (all_[i].signed_event == id) return i;
  }
  return std::nullopt;
}

void Edicts::save(ArchiveWriter& w) const {
  w.u64(all_.size());
  for (const Edict& e : all_) {
    w.u32(e.def); w.u32(e.signer); w.i64(e.signed_tick); w.i64(e.repealed_tick); w.u64(e.signed_event);
    w.i64(e.applications); w.i64(e.moved);
  }
}

void Edicts::load(ArchiveReader& r) {
  std::vector<Edict> all(r.u64());
  for (Edict& e : all) {
    e.def = r.u32(); e.signer = r.u32(); e.signed_tick = r.i64(); e.repealed_tick = r.i64(); e.signed_event = r.u64();
    e.applications = r.i64(); e.moved = r.i64();
  }
  all_ = std::move(all);
}

}  // namespace zs
