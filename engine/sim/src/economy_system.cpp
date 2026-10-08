// Spec: docs/tier0/specs/32-stocks-goods-and-money.md
#include "zs/economy_system.h"

#include <stdexcept>

namespace zs {

EconomySystem::EconomySystem(World& w) : money_(w.ledger) {
  decayed_ = owners_.add(w.ledger, "world.decayed", Scope::Sink);
  consumed_ = owners_.add(w.ledger, "world.consumed", Scope::Sink);
}

void EconomySystem::start_establishments(const World& w) {
  for (std::size_t i = 0; i < estabs_.size(); ++i) estabs_.at(i).caught_up = w.clock.tick;
  read_staff(w);
}

void EconomySystem::read_staff(const World& w) {
  if (!pops_) return;
  const double year = year_of_tick(w.clock.tick);
  for (std::size_t i = 0; i < estabs_.size(); ++i) {
    Establishment& e = estabs_.at(i);
    if (e.roster == 0) continue;
    const Roster* r = pops_->registry().roster(e.roster);
    if (!r) continue;
    const std::int64_t present = static_cast<std::int64_t>(r->size(year));
    const std::int64_t away = static_cast<std::int64_t>(r->count(EffectKind::LaidOff, year) + r->count(EffectKind::Ill, year));
    e.staff = present > away ? present - away : 0;
  }
}

void EconomySystem::catch_up(World& w, CauseId cause) {
  projects_.before_advance(w.ledger, goods_, money_, estabs_);
  for (const auto& [i, n] : estabs_.advance(w.ledger, goods_, recipes_, w.clock.tick, consumed_, cause))
    w.emit(EventType::EconProduced, estabs_.at(i).owner, cause, estabs_.at(i).recipe, n);
  for (const Projects::Change& ch : projects_.after_advance(w.ledger, goods_, recipes_, money_, estabs_, w.clock.tick, cause)) {
    switch (ch.what) {
      case Projects::Change::Finished: w.emit(EventType::ProjectFinished, ch.site, cause, ch.plan, ch.tick); break;
      case Projects::Change::Stalled: w.emit(EventType::ProjectStalled, ch.site, cause, static_cast<std::int64_t>(ch.stall)); break;
      case Projects::Change::Resumed: w.emit(EventType::ProjectResumed, ch.site, cause); break;
    }
  }
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
  if (c.type < EconVisit || c.type > ProjectCancel) return;
  const auto subject = static_cast<OwnerId>(c.subject);
  const std::int64_t tick = w.clock.tick;
  catch_up(w, cause);  // establishments run up to now before anything changes under them
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
    case EstabSetStaff: {
      Establishment* e = estabs_.find(subject);
      if (!e) return refuse(w, c, cause, EconRefusal::NoEstablishment);
      if (e->roster != 0) return refuse(w, c, cause, EconRefusal::StaffFromRoster);
      if (c.a < 0 || c.a == e->staff) return refuse(w, c, cause, EconRefusal::Nothing);
      e->staff = c.a;
      return;
    }
    case EstabSetPower: {
      Establishment* e = estabs_.find(subject);
      if (!e) return refuse(w, c, cause, EconRefusal::NoEstablishment);
      if (c.a < 0 || c.a == e->power_w) return refuse(w, c, cause, EconRefusal::Nothing);
      e->power_w = c.a;
      return;
    }
    case EconRunByHand: {
      if (c.a < 0 || static_cast<std::size_t>(c.a) >= recipes_.size()) return refuse(w, c, cause, EconRefusal::UnknownRecipe);
      const auto r = static_cast<RecipeId>(c.a);
      const std::int64_t n = recipes_.run(w.ledger, goods_, tick, subject, r, c.b, consumed_, cause);
      if (n <= 0) return refuse(w, c, cause, EconRefusal::Nothing);
      w.emit(EventType::EconProduced, subject, cause, c.a, n);
      return;
    }
    case ProjectOrder: {
      if (c.a < 0 || static_cast<std::size_t>(c.a) >= projects_.plans()) return refuse(w, c, cause, EconRefusal::UnknownPlan);
      const auto orderer = static_cast<OwnerId>(c.b);
      if (!owners_.known(orderer) || owners_.scope(orderer) == Scope::Sink) return refuse(w, c, cause, EconRefusal::UnknownOwner);
      switch (projects_.order(w.ledger, goods_, money_, estabs_, tick, subject, static_cast<PlanId>(c.a), orderer, cause)) {
        case Projects::OrderResult::Ok: w.emit(EventType::ProjectOrdered, subject, cause, c.a, c.b); return;
        case Projects::OrderResult::SiteBusy: return refuse(w, c, cause, EconRefusal::SiteBusy);
        case Projects::OrderResult::NoBudget: return refuse(w, c, cause, EconRefusal::NoBudget);
        case Projects::OrderResult::NoSite: return refuse(w, c, cause, EconRefusal::NoEstablishment);
      }
      return;
    }
    case ProjectCancel: {
      const std::int64_t returned = projects_.cancel(w.ledger, money_, estabs_, tick, subject, cause);
      if (returned < 0) return refuse(w, c, cause, EconRefusal::NoProject);
      w.emit(EventType::ProjectCancelled, subject, cause, returned);
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
  read_staff(w);  // the roster may have changed since the last hour
  catch_up(w, 0);
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
  const auto& es = estabs_.all();
  w.u64(es.size());
  for (const Establishment& e : es) {
    w.u32(e.owner); w.u16(e.recipe); w.u64(e.roster); w.i64(e.staff); w.i64(e.power_w); w.i64(e.labour_bank); w.i64(e.caught_up); w.i64(e.produced); w.i64(e.batch_cap);
  }
  projects_.save(w);
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
  std::vector<Establishment> es(r.u64());
  for (Establishment& e : es) {
    e.owner = r.u32(); e.recipe = r.u16(); e.roster = r.u64(); e.staff = r.i64(); e.power_w = r.i64(); e.labour_bank = r.i64(); e.caught_up = r.i64(); e.produced = r.i64(); e.batch_cap = r.i64();
  }
  estabs_.restore(std::move(es));
  projects_.load(r);
}

}  // namespace zs
