// Spec: docs/tier0/specs/32-stocks-goods-and-money.md
#include "zs/economy_system.h"

#include <stdexcept>

namespace zs {

EconomySystem::EconomySystem(World& w) : money_(w.ledger) {
  decayed_ = owners_.add(w.ledger, "world.decayed", Scope::Sink);
}

void EconomySystem::start_lots(const World& w) {
  for (std::size_t g = 0; g < goods_.size(); ++g) {
    const GoodDef& d = goods_.def(static_cast<GoodId>(g));
    if (d.half_life_days <= 0) continue;
    for (const auto& [key, amount] : w.ledger.balances()) {
      if (key.second != d.bulk || amount <= 0 || key.first == decayed_ || !owners_.known(key.first)) continue;
      spoilage_.reset(w.ledger, goods_, key.first, static_cast<GoodId>(g), w.clock.tick);
    }
  }
}

Panel EconomySystem::panel(const World& w, Layer layer, const Context& ctx) const {
  return zs::panel(layer, w.ledger, owners_, goods_, money_, ctx);
}

void EconomySystem::refuse(World& w, const Command& c, EventId cause, EconRefusal why) {
  w.emit(EventType::EconRefused, c.subject, cause, c.type, static_cast<std::int64_t>(why));
}

void EconomySystem::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type < EconVisit || c.type > EconShip) return;
  const auto subject = static_cast<OwnerId>(c.subject);
  const std::int64_t tick = w.clock.tick;
  if (!owners_.known(subject) || owners_.scope(subject) == Scope::Sink) return refuse(w, c, cause, EconRefusal::UnknownOwner);
  switch (c.type) {
    case EconVisit: {
      if (!known_good(c.a)) return refuse(w, c, cause, EconRefusal::UnknownGood);
      const auto g = static_cast<GoodId>(c.a);
      if (goods_.def(g).half_life_days > 0) spoilage_.advance(w.ledger, goods_, decayed_, tick, cause);
      if (goods_.materialise(w.ledger, tick, subject, g, c.b, cause) <= 0) return refuse(w, c, cause, EconRefusal::Nothing);
      spoilage_.reset(w.ledger, goods_, subject, g, tick);  // the bulk changed: the lot restarts from what is left
      return;
    }
    case EconLeave: {
      if (!known_good(c.a)) return refuse(w, c, cause, EconRefusal::UnknownGood);
      const auto g = static_cast<GoodId>(c.a);
      if (goods_.def(g).half_life_days > 0) spoilage_.advance(w.ledger, goods_, decayed_, tick, cause);
      if (goods_.dematerialise(w.ledger, tick, subject, g, cause) <= 0) return refuse(w, c, cause, EconRefusal::Nothing);
      spoilage_.reset(w.ledger, goods_, subject, g, tick);
      return;
    }
    case EconPurchase: {
      const auto seller = static_cast<OwnerId>(c.a);
      if (!owners_.known(seller) || owners_.scope(seller) == Scope::Sink) return refuse(w, c, cause, EconRefusal::UnknownOwner);
      const std::int64_t good = c.b & 0xffff;
      const std::int64_t want = (c.b >> 16) & 0xffff;
      const std::int64_t price = (c.b >> 32) & 0xffffffff;
      if (!known_good(good)) return refuse(w, c, cause, EconRefusal::UnknownGood);
      const std::int64_t units = money_.purchase(w.ledger, tick, subject, seller, goods_, static_cast<GoodId>(good), price, want, cause);
      if (units <= 0) return refuse(w, c, cause, EconRefusal::Nothing);
      w.emit(EventType::EconTraded, subject, cause, c.a, units);
      return;
    }
    case EconPay: {
      const auto to = static_cast<OwnerId>(c.a);
      if (!owners_.known(to) || owners_.scope(to) == Scope::Sink) return refuse(w, c, cause, EconRefusal::UnknownOwner);
      const auto kind = static_cast<FlowKind>((c.b >> 56) & 0xff);
      const std::int64_t cents = c.b & ((std::int64_t{1} << 56) - 1);
      const std::int64_t moved = money_.pay(w.ledger, tick, subject, to, cents, kind, cause);
      if (moved <= 0) return refuse(w, c, cause, EconRefusal::Nothing);
      w.emit(EventType::EconPaid, to, cause, c.subject, moved);
      return;
    }
    case EconShip: {
      const auto to = static_cast<OwnerId>(c.a);
      if (!owners_.known(to) || owners_.scope(to) == Scope::Sink) return refuse(w, c, cause, EconRefusal::UnknownOwner);
      const std::int64_t good = c.b & 0xffff;
      const std::int64_t amount = c.b >> 16;
      if (!known_good(good)) return refuse(w, c, cause, EconRefusal::UnknownGood);
      if (to == subject || amount <= 0) return refuse(w, c, cause, EconRefusal::Nothing);
      const auto g = static_cast<GoodId>(good);
      const GoodDef& d = goods_.def(g);
      // Spoiling goods are caught up before they move, so what moves is what is still there.
      if (d.half_life_days > 0) spoilage_.advance(w.ledger, goods_, decayed_, tick, cause);
      if (w.ledger.transfer(tick, subject, to, d.bulk, amount, cause) <= 0) return refuse(w, c, cause, EconRefusal::Nothing);
      if (d.half_life_days > 0) {
        spoilage_.reset(w.ledger, goods_, subject, g, tick);
        spoilage_.reset(w.ledger, goods_, to, g, tick);
      }
      return;
    }
    default: return;
  }
}

void EconomySystem::update(World& w) {
  // Spoilage is a cold rule: it jumps once an hour (any cadence gives the same grams, spec 32 rule 4).
  if (w.clock.tick % kTicksPerGameHour != 0) return;
  const std::int64_t moved = spoilage_.advance(w.ledger, goods_, decayed_, w.clock.tick, 0);
  if (moved > 0) w.emit(EventType::EconSpoiled, 0, 0, moved);
}

void EconomySystem::save(ArchiveWriter& w) const {
  const auto& lots = spoilage_.lots();
  w.u64(lots.size());
  for (const auto& lot : lots) {
    w.u32(lot.owner); w.u16(lot.good); w.i64(lot.start_amount); w.i64(lot.start_tick); w.i64(lot.caught_up);
  }
  const auto write_tallies = [&](const Money::Tallies& t) {
    w.u64(t.size());
    for (const auto& [key, cents] : t) { w.u32(key.first); w.u8(static_cast<std::uint8_t>(key.second)); w.i64(cents); }
  };
  write_tallies(money_.received_tallies());
  write_tallies(money_.paid_tallies());
}

void EconomySystem::load(ArchiveReader& r) {
  std::vector<Spoilage::Lot> lots(r.u64());
  for (auto& lot : lots) {
    lot.owner = r.u32(); lot.good = r.u16(); lot.start_amount = r.i64(); lot.start_tick = r.i64(); lot.caught_up = r.i64();
  }
  spoilage_.restore(std::move(lots));
  const auto read_tallies = [&] {
    Money::Tallies t;
    const std::uint64_t n = r.u64();
    for (std::uint64_t i = 0; i < n; ++i) {
      const OwnerId o = r.u32();
      const auto k = static_cast<FlowKind>(r.u8());
      t[{o, k}] = r.i64();
    }
    return t;
  };
  Money::Tallies in = read_tallies();
  Money::Tallies out = read_tallies();
  money_.restore(std::move(in), std::move(out));
}

}  // namespace zs
