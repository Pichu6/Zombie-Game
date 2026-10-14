// Food in play. Specs: docs/tier0/specs/80-food-in-play.md (decision 0079), 81-households-shop-by-themselves.md (0080).
#include "zs/food_chain.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "toml.hpp"
#include "zs/actions.h"
#include "zs/body.h"
#include "zs/events.h"
#include "zs/pop.h"

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

FoodChain::FoodChain(FoodSetup setup, ErrandParams errand, Bodies* bodies, Actors* actors, OwnerId pantry, std::uint64_t world_seed)
    : setup_(std::move(setup)), errand_(errand), bodies_(bodies), actors_(actors), pantry_(pantry), seed_(world_seed) {}

void FoodChain::add_household(std::string name, OwnerId pantry, OwnerId fridge, std::vector<EntityId> members) {
  if (members.empty()) return;  // rule 1: no members, no household
  Household h;
  h.name = std::move(name);
  h.pantry = pantry;
  h.fridge = fridge;
  h.members = std::move(members);
  households_.push_back(std::move(h));
}

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

std::int64_t FoodChain::next_at(std::int64_t now, int hour) const {
  // Tick 0 is 08:00 of day 0, so that hour of day d is d days plus (hour - 8) hours.
  const std::int64_t base = static_cast<std::int64_t>(hour - 8) * kTicksPerGameHour;
  std::int64_t d = now - base <= 0 ? 0 : (now - base + kTicksPerGameDay - 1) / kTicksPerGameDay;
  return base + d * kTicksPerGameDay;
}

std::int64_t FoodChain::household_stock(const World& w, std::size_t h) const {
  const Household& x = households_[h];
  return w.ledger.balance(x.pantry, food_) + (x.fridge ? w.ledger.balance(x.fridge, food_) : 0);
}

std::int64_t FoodChain::household_low(std::size_t h) const {
  const Household& x = households_[h];
  const double interval = 7.0 / std::max(1.0, setup_.trips_per_week);
  return x.full_j - static_cast<std::int64_t>(std::llround(static_cast<double>(x.need_j) * interval));
}

void FoodChain::setup(World& w) {
  Ledger& l = w.ledger;
  const auto& names = l.resource_names();
  const auto it = std::find(names.begin(), names.end(), std::string("food_j"));
  food_ = it == names.end() ? l.add_resource("food_j") : static_cast<ResourceId>(it - names.begin());
  centre_ = l.add_owner("food:distribution centre");
  transit_ = l.add_owner("food:on the road");
  homeward_ = l.add_owner("food:on its way home");
  before_ = l.add_owner("food:eaten before the start");
  for (const auto& o : setup_.outlets) outlets_.push_back(l.add_owner("food:" + o.name));
  // Spec 80 rule 2: the centre and every outlet at their targets.
  l.mint(centre_, food_, joules(setup_.town_people * setup_.kj_per_person_day * setup_.centre_days));
  for (std::size_t o = 0; o < outlets_.size(); ++o) l.mint(outlets_[o], food_, outlet_target(o));
  // Spec 81 rules 2-3: each household's full stock, its outlet, and where it starts in its shopping cycle.
  std::size_t nearest = 0;
  for (std::size_t o = 0; o < setup_.outlets.size(); ++o)
    if (outlet_target(o) > 0 && (outlet_target(nearest) <= 0 || setup_.outlets[o].minutes < setup_.outlets[nearest].minutes)) nearest = o;
  const double days = setup_.pantry_days + setup_.fridge_days;
  const double interval = 7.0 / std::max(1.0, setup_.trips_per_week);
  for (std::size_t h = 0; h < households_.size(); ++h) {
    Household& x = households_[h];
    x.need_j = joules(static_cast<double>(x.members.size()) * setup_.kj_per_person_day);
    x.full_j = static_cast<std::int64_t>(std::llround(static_cast<double>(x.need_j) * days));
    x.outlet = nearest;
    const double u = keyed_uniform01(seed_, Purpose::FoodStart, h, 0);
    const std::int64_t start = x.full_j - static_cast<std::int64_t>(std::llround(static_cast<double>(x.need_j) * interval * u));
    const std::int64_t pantry_want = static_cast<std::int64_t>(std::llround(static_cast<double>(start) * (x.fridge ? setup_.pantry_days / days : 1.0)));
    const std::int64_t fridge_want = x.fridge ? start - pantry_want : 0;
    auto set = [&](OwnerId owner, std::int64_t want) {
      const std::int64_t have = l.balance(owner, food_);
      if (want > have) l.mint(owner, food_, want - have);
      else if (want < have) l.transfer(w.clock.tick, owner, before_, food_, have - want, 0);
    };
    set(x.pantry, pantry_want);
    if (x.fridge) set(x.fridge, fridge_want);
  }
  next_day_ = next_at(w.clock.tick + 1, 5);
  next_shop_ = next_at(w.clock.tick + 1, 17);
}

std::int64_t FoodChain::centre_stock(const World& w) const { return w.ledger.balance(centre_, food_); }
std::int64_t FoodChain::outlet_stock(const World& w, std::size_t o) const { return w.ledger.balance(outlets_[o], food_); }
std::int64_t FoodChain::eaten_by_town(const World&) const { return eaten_; }
std::int64_t FoodChain::eaten_before_start(const World& w) const { return w.ledger.balance(before_, food_); }

void FoodChain::refuse(World& w, const Command& c, EventId cause, int why) {
  w.emit(EventType::FoodRefused, c.subject, cause, c.a, why);
}

int FoodChain::household_of(EntityId body) const {
  for (std::size_t h = 0; h < households_.size(); ++h)
    if (std::find(households_[h].members.begin(), households_[h].members.end(), body) != households_[h].members.end())
      return static_cast<int>(h);
  return -1;
}

bool FoodChain::can_go(const World&, EntityId body) const {
  const Body& b = bodies_->get(body);
  if (!b.alive || b.spec.walker || b.away) return false;
  if (actors_->actor(body).pos.z >= 10) return false;  // the block's levels are below 10 (spec 70)
  return !(held_ && held_(body));
}

bool FoodChain::start_errand(World& w, EntityId body, std::size_t o, std::int64_t want, int household, EventId cause) {
  const std::int64_t took = w.ledger.transfer(w.clock.tick, outlets_[o], homeward_, food_, std::min(want, carry_j()), cause);
  if (took <= 0) return false;
  bodies_->get(body).away = true;  // off the block: no AI drives it, no walker sees it
  const std::int64_t back = w.clock.tick + errand_minutes(o) * (kTicksPerGameHour / 60);
  const EventId began = w.emit(EventType::FoodErrandBegan, body, cause, static_cast<std::int64_t>(o), back);
  errands_.push_back({body, o, took, back, began, household});
  return true;
}

void FoodChain::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type != FoodErrand) return;
  // Spec 80 rule 4: the errand, by any cause (a player's order, a script).
  const EntityId body = c.subject;
  const Body* b = nullptr;
  try {
    b = &bodies_->get(body);
  } catch (const std::out_of_range&) {
    return refuse(w, c, cause, NotABody);
  }
  if (!b->alive || b->spec.walker) return refuse(w, c, cause, NotAlive);
  if (b->away) return refuse(w, c, cause, Away);
  if (actors_->actor(body).pos.z >= 10) return refuse(w, c, cause, OffBlock);
  if (c.a < 0 || static_cast<std::size_t>(c.a) >= outlets_.size()) return refuse(w, c, cause, NoOutlet);
  const int h = household_of(body);
  const std::int64_t want = h >= 0 ? std::max<std::int64_t>(0, households_[static_cast<std::size_t>(h)].full_j - household_stock(w, static_cast<std::size_t>(h))) : carry_j();
  if (!start_errand(w, body, static_cast<std::size_t>(c.a), want > 0 ? want : carry_j(), h, cause)) return refuse(w, c, cause, Empty);
}

void FoodChain::daily(World& w) {
  Ledger& l = w.ledger;
  const std::int64_t now = w.clock.tick;
  const std::int64_t day_need = joules(setup_.town_people * setup_.kj_per_person_day);
  // Spec 80 rule 3: the centre's supply from the wider chain, up to its days of stock.
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

void FoodChain::shop(World& w) {
  // Spec 81 rule 4: a household at or below its low point sends the first member who can go; none: it waits a day.
  for (std::size_t h = 0; h < households_.size(); ++h) {
    const Household& x = households_[h];
    bool out = false;  // one errand at a time per household
    for (const Errand& e : errands_) out |= e.household == static_cast<int>(h);
    const std::int64_t stock = household_stock(w, h);
    if (out || stock > household_low(h)) continue;
    for (EntityId body : x.members) {
      if (!can_go(w, body)) continue;
      start_errand(w, body, x.outlet, x.full_j - stock, static_cast<int>(h), 0);
      break;
    }
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
  // Errands back home: the food goes into the household's pantry and fridge by what each lacks (spec 81 rule 4), or the
  // block's first pantry; the body is back where it left.
  for (auto it = errands_.begin(); it != errands_.end();) {
    if (it->back <= now) {
      std::int64_t home = 0;
      if (it->household >= 0) {
        const Household& x = households_[static_cast<std::size_t>(it->household)];
        const double days = setup_.pantry_days + setup_.fridge_days;
        const std::int64_t fridge_full = x.fridge ? static_cast<std::int64_t>(std::llround(static_cast<double>(x.full_j) * setup_.fridge_days / days)) : 0;
        const std::int64_t fridge_lacks = x.fridge ? std::max<std::int64_t>(0, fridge_full - w.ledger.balance(x.fridge, food_)) : 0;
        const std::int64_t to_fridge = std::min(it->amount, fridge_lacks);
        if (to_fridge > 0) home += w.ledger.transfer(now, homeward_, x.fridge, food_, to_fridge, it->cause);
        home += w.ledger.transfer(now, homeward_, x.pantry, food_, it->amount - to_fridge, it->cause);
      } else {
        home = w.ledger.transfer(now, homeward_, pantry_, food_, it->amount, it->cause);
      }
      bodies_->get(it->body).away = false;
      w.emit(EventType::FoodErrandEnded, it->body, it->cause, static_cast<std::int64_t>(it->outlet), home);
      it = errands_.erase(it);
    } else {
      ++it;
    }
  }
  if (now >= next_day_) {
    daily(w);
    next_day_ = next_at(now + 1, 5);
  }
  if (now >= next_shop_) {
    shop(w);
    next_shop_ = next_at(now + 1, 17);
  }
}

std::int64_t FoodChain::next_wake(const World&, std::int64_t now) const {
  std::int64_t t = std::min(next_day_, next_shop_);
  for (const Delivery& d : deliveries_) t = std::min(t, d.arrives);
  for (const Errand& e : errands_) t = std::min(t, e.back);
  return std::max(now, t);
}

void FoodChain::save(ArchiveWriter& a) const {
  a.i64(next_day_);
  a.i64(next_shop_);
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
    a.i64(e.household);
  }
}

void FoodChain::load(ArchiveReader& a) {
  next_day_ = a.i64();
  next_shop_ = a.i64();
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
    e.household = static_cast<int>(a.i64());
  }
}

}  // namespace zs
