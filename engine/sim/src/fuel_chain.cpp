// The town's fuel in play. Spec: docs/tier0/specs/97-the-towns-fuel.md.
#include "zs/fuel_chain.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "zs/actions.h"
#include "zs/assembly.h"
#include "zs/body.h"
#include "zs/events.h"

namespace zs {

namespace {
enum FuelRefusal : int { NotABody = 1, NotAlive = 2, Away = 3, OffBlock = 4, NoStation = 5, Empty = 6, NoContainer = 7 };
std::int64_t microlitres(double litres) { return static_cast<std::int64_t>(std::llround(litres * 1e6)); }
}  // namespace

FuelChain::FuelChain(FuelSetup setup, Bodies* bodies, Actors* actors, Assemblies* items)
    : setup_(std::move(setup)), bodies_(bodies), actors_(actors), items_(items) {}

std::int64_t FuelChain::next_at(std::int64_t now, int hour) const {
  // Tick 0 is 08:00 of day 0, so that hour of day d is d days plus (hour - 8) hours.
  const std::int64_t base = static_cast<std::int64_t>(hour - 8) * kTicksPerGameHour;
  std::int64_t d = now - base <= 0 ? 0 : (now - base + kTicksPerGameDay - 1) / kTicksPerGameDay;
  return base + d * kTicksPerGameDay;
}

void FuelChain::setup(World& w) {
  Ledger& l = w.ledger;
  const auto& names = l.resource_names();
  const auto it = std::find(names.begin(), names.end(), std::string("gasoline_ul"));
  fuel_ = it == names.end() ? l.add_resource("gasoline_ul") : static_cast<ResourceId>(it - names.begin());
  homeward_ = l.add_owner("fuel:on its way home");
  // Rule 1: every station at its stock.
  for (const auto& s : setup_.stations) {
    stations_.push_back(l.add_owner("fuel:" + s.name));
    l.mint(stations_.back(), fuel_, microlitres(s.stock_l));
  }
  next_day_ = next_at(w.clock.tick + 1, 5);
}

std::int64_t FuelChain::station_stock(const World& w, std::size_t s) const { return w.ledger.balance(stations_[s], fuel_); }

bool FuelChain::pumps_run(const World& w, std::size_t s) const {
  return setup_.stations[s].generator || setup_.mains_until < 0 || w.clock.tick < setup_.mains_until;
}

std::int64_t FuelChain::errand_minutes(const World& w, std::size_t s) const {
  const double at = pumps_run(w, s) ? setup_.fill_minutes : setup_.hand_pump_minutes;
  return 2 * setup_.stations[s].minutes + static_cast<std::int64_t>(std::llround(at));
}

std::int64_t FuelChain::cans_room(const World& w, EntityId body) const {
  std::int64_t room = 0;
  for (EntityId c : actors_->carried(w, body)) {
    const auto cs = actors_->store_of(c);
    if (!cs) continue;
    const auto cap = static_cast<std::int64_t>(items_->prop(c, Prop::ContainerMl) * 1000);
    std::int64_t other = 0;  // a container holds one liquid: never fuel into a water bottle
    for (ResourceId r = 0; r < static_cast<ResourceId>(w.ledger.resource_names().size()); ++r)
      if (r != fuel_) other += w.ledger.balance(*cs, r);
    if (other == 0) room += std::max<std::int64_t>(0, cap - w.ledger.balance(*cs, fuel_));
  }
  return room;
}

void FuelChain::refuse(World& w, const Command& c, EventId cause, int why) { w.emit(EventType::FuelRefused, c.subject, cause, c.a, why); }

void FuelChain::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type != FuelErrand) return;
  // Rule 4: the errand, by any cause.
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
  if (c.a < 0 || static_cast<std::size_t>(c.a) >= stations_.size()) return refuse(w, c, cause, NoStation);
  const auto s = static_cast<std::size_t>(c.a);
  const std::int64_t room = cans_room(w, body);
  if (room <= 0) return refuse(w, c, cause, NoContainer);
  const std::int64_t took = w.ledger.transfer(w.clock.tick, stations_[s], homeward_, fuel_, room, cause);
  if (took <= 0) return refuse(w, c, cause, Empty);
  bodies_->get(body).away = true;  // off the block: no AI drives it, no walker sees it
  const std::int64_t back = w.clock.tick + errand_minutes(w, s) * (kTicksPerGameHour / 60);
  const EventId began = w.emit(EventType::FuelErrandBegan, body, cause, c.a, back);
  errands_.push_back({body, s, took, back, began});
}

void FuelChain::daily(World& w) {
  Ledger& l = w.ledger;
  const std::int64_t now = w.clock.tick;
  // Rule 3: the town's vehicles take their day from the stations whose pumps run.
  for (std::size_t s = 0; s < stations_.size(); ++s) {
    if (!pumps_run(w, s)) continue;
    const std::int64_t sells = std::min(l.balance(stations_[s], fuel_), microlitres(setup_.town_litres_day * setup_.stations[s].share));
    if (sells > 0) {
      l.declare_outflow(now, stations_[s], fuel_, sells, 0);
      sold_ += sells;
    }
  }
  // Rule 2: orders to the depot while it has staff; the tankers arrive after its hours.
  if (setup_.depot_until >= 0 && now >= setup_.depot_until) return;
  const std::int64_t road = static_cast<std::int64_t>(std::llround(setup_.depot_hours * kTicksPerGameHour));
  for (std::size_t s = 0; s < stations_.size(); ++s) {
    std::int64_t on_road = 0;
    for (const Delivery& d : deliveries_)
      if (d.station == s) on_road += d.amount;
    const std::int64_t want = microlitres(setup_.stations[s].stock_l) - l.balance(stations_[s], fuel_) - on_road;
    if (want > 0) deliveries_.push_back({s, want, now + road});
  }
}

void FuelChain::update(World& w) {
  const std::int64_t now = w.clock.tick;
  for (auto it = deliveries_.begin(); it != deliveries_.end();) {
    if (it->arrives <= now) {
      w.ledger.declare_inflow(now, stations_[it->station], fuel_, it->amount, 0);
      it = deliveries_.erase(it);
    } else {
      ++it;
    }
  }
  // Errands back: the fuel goes into the body's carried cans by their room; what does not fit stays at the station.
  for (auto it = errands_.begin(); it != errands_.end();) {
    if (it->back <= now) {
      std::int64_t home = 0;
      for (EntityId c : actors_->carried(w, it->body)) {
        const auto cs = actors_->store_of(c);
        if (!cs || home >= it->amount) continue;
        const auto cap = static_cast<std::int64_t>(items_->prop(c, Prop::ContainerMl) * 1000);
        const std::int64_t room = cap - w.ledger.balance(*cs, fuel_);
        if (room > 0) home += w.ledger.transfer(now, homeward_, *cs, fuel_, std::min(room, it->amount - home), it->cause);
      }
      if (home < it->amount) w.ledger.transfer(now, homeward_, stations_[it->station], fuel_, it->amount - home, it->cause);
      bodies_->get(it->body).away = false;
      w.emit(EventType::FuelErrandEnded, it->body, it->cause, static_cast<std::int64_t>(it->station), home);
      it = errands_.erase(it);
    } else {
      ++it;
    }
  }
  if (now >= next_day_) {
    daily(w);
    next_day_ = next_at(now + 1, 5);
  }
}

std::int64_t FuelChain::next_wake(const World&, std::int64_t now) const {
  std::int64_t t = next_day_;
  for (const Delivery& d : deliveries_) t = std::min(t, d.arrives);
  for (const Errand& e : errands_) t = std::min(t, e.back);
  return std::max(now, t);
}

void FuelChain::save(ArchiveWriter& a) const {
  a.i64(next_day_);
  a.i64(sold_);
  a.u32(static_cast<std::uint32_t>(deliveries_.size()));
  for (const Delivery& d : deliveries_) {
    a.u32(static_cast<std::uint32_t>(d.station));
    a.i64(d.amount);
    a.i64(d.arrives);
  }
  a.u32(static_cast<std::uint32_t>(errands_.size()));
  for (const Errand& e : errands_) {
    a.u64(e.body);
    a.u32(static_cast<std::uint32_t>(e.station));
    a.i64(e.amount);
    a.i64(e.back);
    a.u64(e.cause);
  }
}

void FuelChain::load(ArchiveReader& a) {
  next_day_ = a.i64();
  sold_ = a.i64();
  deliveries_.resize(a.u32());
  for (Delivery& d : deliveries_) {
    d.station = a.u32();
    d.amount = a.i64();
    d.arrives = a.i64();
  }
  errands_.resize(a.u32());
  for (Errand& e : errands_) {
    e.body = a.u64();
    e.station = a.u32();
    e.amount = a.i64();
    e.back = a.i64();
    e.cause = a.u64();
  }
}

}  // namespace zs
