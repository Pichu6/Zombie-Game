// Spec: docs/tier0/specs/39-orders-and-belief.md
#include "zs/orders_system.h"

namespace zs {

void OrdersSystem::refuse(World& w, const Command& c, EventId cause, OrderRefusal why) {
  w.emit(EventType::OrderRefused, c.subject, cause, c.type, static_cast<std::int64_t>(why));
}

void OrdersSystem::broadcast(World& w, std::size_t edict, std::int64_t claim, EventId cause) {
  if (!comms_up_) return;
  for (PopId p : audience_) {
    Belief b;
    b.about = edict_entity(edict);
    b.claim = claim;
    b.source = Source::Broadcast;
    b.learned_tick = w.clock.tick;
    beliefs_.receive(p, b);
  }
  w.emit(EventType::Broadcast, edict_entity(edict), cause, static_cast<std::int64_t>(audience_.size()));
}

void OrdersSystem::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type < EdictSign || c.type > SetComms) return;
  const std::int64_t tick = w.clock.tick;
  switch (c.type) {
    case EdictSign: {
      if (c.a < 0 || static_cast<std::size_t>(c.a) >= edicts_.defs()) return refuse(w, c, cause, OrderRefusal::UnknownDef);
      const EdictDef& d = edicts_.def(static_cast<std::uint32_t>(c.a));
      if ((d.kind == EdictKind::Staffing && !pops_) || (d.kind != EdictKind::Staffing && !econ_)) return refuse(w, c, cause, OrderRefusal::NoSystem);
      // The signing event is the root cause of everything the edict does; its id is known before the edict is.
      const EventId signed_event = w.emit(EventType::EdictSigned, edicts_.size(), cause, c.a, static_cast<std::int64_t>(c.subject));
      const std::size_t i = edicts_.sign(static_cast<std::uint32_t>(c.a), static_cast<OwnerId>(c.subject), tick, signed_event);
      if (d.kind == EdictKind::Staffing) {
        // Once, on signing: a region event on the workplace rosters, with the edict as cause.
        const std::int64_t a = static_cast<std::int64_t>(EffectKind::LaidOff) | ((static_cast<std::int64_t>(RosterKind::Workplace) + 1) << 8);
        Command rc{tick, RegionApply, d.pop_region, a, d.delta};
        pops_->apply_command(w, rc, signed_event);
        edicts_.at(i).applications = 1;
        edicts_.at(i).moved = d.delta;
      }
      broadcast(w, i, d.kind == EdictKind::Staffing ? d.delta : d.per_mille, signed_event);
      return;
    }
    case EdictRepeal: {
      if (c.subject >= edicts_.size()) return refuse(w, c, cause, OrderRefusal::NoSuchEdict);
      if (!edicts_.repeal(c.subject, tick)) return refuse(w, c, cause, OrderRefusal::NotInForce);
      w.emit(EventType::EdictRepealed, c.subject, cause);
      return;
    }
    case Tell: {
      const auto from = static_cast<PopId>(c.subject), to = static_cast<PopId>(c.a);
      const auto about = static_cast<EntityId>(c.b);
      if (!beliefs_.about(from, about)) return refuse(w, c, cause, OrderRefusal::NothingKnown);
      if (!beliefs_.retell(w.rng.seed(), from, to, about, tick, distortion_)) return refuse(w, c, cause, OrderRefusal::Nothing);
      w.emit(EventType::Told, to, cause, static_cast<std::int64_t>(from), static_cast<std::int64_t>(about));
      return;
    }
    case SetComms: {
      const bool up = c.a != 0;
      if (up == comms_up_) return refuse(w, c, cause, OrderRefusal::Nothing);
      comms_up_ = up;
      return;
    }
    default: return;
  }
}

void OrdersSystem::apply_edict(World& w, std::size_t i) {
  Edict& e = edicts_.at(i);
  const EdictDef& d = edicts_.def(e.def);
  if (!econ_) return;
  const EventId applied = w.emit(EventType::EdictApplied, i, e.signed_event);
  const Ledger& l = w.ledger;
  const ResourceId res = d.kind == EdictKind::Levy ? econ_->goods().def(d.good).bulk : econ_->money().resource();
  const std::int64_t dest_before = l.balance(d.destination, res);
  for (OwnerId o : econ_->owners().descendants(d.scope)) {
    if (econ_->owners().scope(o) != Scope::Building || o == d.destination) continue;
    const std::int64_t amount = l.balance(o, res) * d.per_mille / 1000;
    if (amount <= 0) continue;
    Command c{w.clock.tick, 0, o, static_cast<std::int64_t>(d.destination), 0};
    if (d.kind == EdictKind::Levy) { c.type = EconShip; c.b = econ_ship_arg(d.good, amount); }
    else { c.type = EconPay; c.b = econ_pay_arg(FlowKind::Tax, amount); }
    econ_->apply_command(w, c, applied);
  }
  const std::int64_t moved = l.balance(d.destination, res) - dest_before;
  e.applications += 1;
  e.moved += moved;  // the edict's own panel reads this; the ledger entries under `applied` are the detail
}

void OrdersSystem::update(World& w) {
  if (w.clock.tick % kTicksPerGameDay != 0) return;
  for (std::size_t i = 0; i < edicts_.size(); ++i) {
    const Edict& e = edicts_.at(i);
    if (!e.in_force() || e.signed_tick == w.clock.tick) continue;  // signed today: it starts tomorrow
    if (edicts_.def(e.def).kind == EdictKind::Staffing) continue;    // once, on signing
    apply_edict(w, i);
  }
}

void OrdersSystem::save(ArchiveWriter& w) const {
  edicts_.save(w);
  beliefs_.save(w);
  w.boolean(comms_up_);
}

void OrdersSystem::load(ArchiveReader& r) {
  edicts_.load(r);
  beliefs_.load(r);
  comms_up_ = r.boolean();
}

}  // namespace zs
