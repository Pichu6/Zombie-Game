// Food in play. Spec: docs/tier0/specs/80-food-in-play.md (decision 0079).
#include "zs/food_chain.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "toml.hpp"
#include "zs/actions.h"
#include "zs/body.h"
#include "zs/events.h"

namespace zs {

namespace {

enum FoodRefusal : int { NotABody = 1, NotAlive = 2, Away = 3, OffBlock = 4, NoOutlet = 5, Empty = 6 };

std::int64_t joules(double kj) { return static_cast<std::int64_t>(std::llround(kj * 1000.0)); }

}  // namespace

ErrandParams load_errand_params(const std::string& path) {
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  ErrandParams p;
  p.carry_kg = t["errand"]["carry_kg"].value_or(p.carry_kg);
  p.mj_per_kg = t["errand"]["mj_per_kg"].value_or(p.mj_per_kg);
  p.shop_minutes = t["errand"]["shop_minutes"].value_or(p.shop_minutes);
  if (p.carry_kg <= 0 || p.mj_per_kg <= 0 || p.shop_minutes < 0) throw std::runtime_error(path + ": [errand] values out of range");
  return p;
}

FoodChain::FoodChain(FoodSetup setup, ErrandParams errand, Bodies* bodies, Actors* actors, OwnerId pantry)
    : setup_(std::move(setup)), errand_(errand), bodies_(bodies), actors_(actors), pantry_(pantry) {}

std::int64_t FoodChain::town_need_j() const {
  return joules(std::max(0.0, setup_.town_people - setup_.block_people) * setup_.kj_per_person_day);
}

std::int64_t FoodChain::outlet_target(std::size_t o) const {
  const auto& out = setup_.outlets[o];
  return joules(setup_.town_people * setup_.kj_per_person_day * out.share * out.stock_days);
}

std::int64_t FoodChain::errand_minutes(std::size_t o) const {
  return 2 * setup_.outlets[o].minutes + static_cast<std::int64_t>(std::llround(errand_.shop_minutes));
}

std::int64_t FoodChain::carry_j() const { return static_cast<std::int64_t>(std::llround(errand_.carry_kg * errand_.mj_per_kg * 1e6)); }

std::int64_t FoodChain::next_day_tick(std::int64_t now) const {
  // 05:00: tick 0 is 08:00 of day 0, so 05:00 of day d is d days less three hours.
  const std::int64_t five = -3 * kTicksPerGameHour;
  std::int64_t d = (now - five + kTicksPerGameDay - 1) / kTicksPerGameDay;
  if (now - five < 0) d = 0;
  return five + d * kTicksPerGameDay;
}

void FoodChain::setup(World& w) {
  Ledger& l = w.ledger;
  const auto& names = l.resource_names();
  const auto it = std::find(names.begin(), names.end(), std::string("food_j"));
  food_ = it == names.end() ? l.add_resource("food_j") : static_cast<ResourceId>(it - names.begin());
  centre_ = l.add_owner("food:distribution centre");
  transit_ = l.add_owner("food:on the road");
  homeward_ = l.add_owner("food:on its way home");
  for (const auto& o : setup_.outlets) outlets_.push_back(l.add_owner("food:" + o.name));
  // Rule 2: the centre and every outlet at their targets.
  l.mint(centre_, food_, joules(setup_.town_people * setup_.kj_per_person_day * setup_.centre_days));
  for (std::size_t o = 0; o < outlets_.size(); ++o) l.mint(outlets_[o], food_, outlet_target(o));
  next_day_ = next_day_tick(w.clock.tick + 1);
}

std::int64_t FoodChain::centre_stock(const World& w) const { return w.ledger.balance(centre_, food_); }
std::int64_t FoodChain::outlet_stock(const World& w, std::size_t o) const { return w.ledger.balance(outlets_[o], food_); }
std::int64_t FoodChain::eaten_by_town(const World&) const { return eaten_; }

void FoodChain::refuse(World& w, const Command& c, EventId cause, int why) {
  w.emit(EventType::FoodRefused, c.subject, cause, c.a, why);
}

void FoodChain::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type != FoodErrand) return;
  // Rule 4: the errand.
  const EntityId body = c.subject;
  const Body* b = nullptr;
  try {
    b = &bodies_->get(body);
  } catch (const std::out_of_range&) {
    return refuse(w, c, cause, NotABody);
  }
  if (!b->alive || b->spec.walker) return refuse(w, c, cause, NotAlive);
  if (b->away) return refuse(w, c, cause, Away);
  if (actors_->actor(body).pos.z >= 10) return refuse(w, c, cause, OffBlock);  // the block's levels are below 10 (spec 70)
  if (c.a < 0 || static_cast<std::size_t>(c.a) >= outlets_.size()) return refuse(w, c, cause, NoOutlet);
  const auto o = static_cast<std::size_t>(c.a);
  const std::int64_t took = w.ledger.transfer(w.clock.tick, outlets_[o], homeward_, food_, carry_j(), cause);
  if (took <= 0) return refuse(w, c, cause, Empty);
  bodies_->get(body).away = true;  // off the block: no AI drives it, no walker sees it
  const std::int64_t back = w.clock.tick + errand_minutes(o) * (kTicksPerGameHour / 60);
  const EventId began = w.emit(EventType::FoodErrandBegan, body, cause, c.a, back);
  errands_.push_back({body, o, took, back, began});
}

void FoodChain::daily(World& w) {
  Ledger& l = w.ledger;
  const std::int64_t now = w.clock.tick;
  const std::int64_t day_need = joules(setup_.town_people * setup_.kj_per_person_day);
  // Rule 3: the centre's supply from the wider chain, up to its days of stock.
  const std::int64_t centre_target = joules(setup_.town_people * setup_.kj_per_person_day * setup_.centre_days);
  const std::int64_t supply = std::min(day_need, centre_target - l.balance(centre_, food_));
  if (supply > 0) l.declare_inflow(now, centre_, food_, supply, 0);
  // The town's people who are not the block's buy their day from the outlets by their shares (the day's sales come
  // before the order that replaces them).
  const std::int64_t need = town_need_j();
  for (std::size_t o = 0; o < outlets_.size(); ++o) {
    const std::int64_t sells = std::min(l.balance(outlets_[o], food_), joules(static_cast<double>(need) / 1000.0 * setup_.outlets[o].share));
    if (sells > 0) {
      l.declare_outflow(now, outlets_[o], food_, sells, 0);
      eaten_ += sells;
    }
  }
  // Orders: what brings each outlet's stock and what is on the road up to its target.
  const std::int64_t road = static_cast<std::int64_t>(std::llround(setup_.centre_hours * kTicksPerGameHour));
  for (std::size_t o = 0; o < outlets_.size(); ++o) {
    std::int64_t on_road = 0;
    for (const Delivery& d : deliveries_)
      if (d.outlet == o) on_road += d.amount;
    const std::int64_t want = outlet_target(o) - l.balance(outlets_[o], food_) - on_road;
    if (want <= 0) continue;
    const std::int64_t sent = l.transfer(now, centre_, transit_, food_, want, 0);
    if (sent > 0) deliveries_.push_back({o, sent, now + road});
  }
}

void FoodChain::update(World& w) {
  const std::int64_t now = w.clock.tick;
  // Deliveries that arrived.
  for (auto it = deliveries_.begin(); it != deliveries_.end();) {
    if (it->arrives <= now) {
      w.ledger.transfer(now, transit_, outlets_[it->outlet], food_, it->amount, 0);
      it = deliveries_.erase(it);
    } else {
      ++it;
    }
  }
  // Errands back home: the food goes into the block's pantry, the body is back where it left.
  for (auto it = errands_.begin(); it != errands_.end();) {
    if (it->back <= now) {
      const std::int64_t home = w.ledger.transfer(now, homeward_, pantry_, food_, it->amount, it->cause);
      Body& b = bodies_->get(it->body);
      b.away = false;
      w.emit(EventType::FoodErrandEnded, it->body, it->cause, static_cast<std::int64_t>(it->outlet), home);
      it = errands_.erase(it);
    } else {
      ++it;
    }
  }
  if (now >= next_day_) {
    daily(w);
    next_day_ = next_day_tick(now + 1);
  }
}

std::int64_t FoodChain::next_wake(const World&, std::int64_t now) const {
  std::int64_t t = next_day_;
  for (const Delivery& d : deliveries_) t = std::min(t, d.arrives);
  for (const Errand& e : errands_) t = std::min(t, e.back);
  return std::max(now, t);
}

void FoodChain::save(ArchiveWriter& a) const {
  a.i64(next_day_);
  a.i64(eaten_);
  a.u32(static_cast<std::uint32_t>(deliveries_.size()));
  for (const Delivery& d : deliveries_) {
    a.u32(static_cast<std::uint32_t>(d.outlet));
    a.i64(d.amount);
    a.i64(d.arrives);
  }
  a.u32(static_cast<std::uint32_t>(errands_.size()));
  for (const Errand& e : errands_) {
    a.u64(e.body);
    a.u32(static_cast<std::uint32_t>(e.outlet));
    a.i64(e.amount);
    a.i64(e.back);
    a.u64(e.cause);
  }
}

void FoodChain::load(ArchiveReader& a) {
  next_day_ = a.i64();
  eaten_ = a.i64();
  deliveries_.resize(a.u32());
  for (Delivery& d : deliveries_) {
    d.outlet = a.u32();
    d.amount = a.i64();
    d.arrives = a.i64();
  }
  errands_.resize(a.u32());
  for (Errand& e : errands_) {
    e.body = a.u64();
    e.outlet = a.u32();
    e.amount = a.i64();
    e.back = a.i64();
    e.cause = a.u64();
  }
}

}  // namespace zs
