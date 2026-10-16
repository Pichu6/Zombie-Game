// Spec 97: the town's fuel in play: stations sell to the town while their pumps run, tankers refill them while the depot
// has staff, and an errand fills the cans a pop carries (by hand pump once the mains are gone).
#include <doctest.h>

#include <algorithm>
#include <string>

#include "zs/assembly.h"
#include "zs/body.h"
#include "zs/events.h"
#include "zs/fuel_chain.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr std::int64_t kMin = kTicksPerGameHour / 60;
constexpr std::int64_t kL = 1'000'000;  // uL

FuelSetup town() {
  FuelSetup s;
  s.town_litres_day = 10000;
  s.depot_hours = 30;
  s.depot_until = 3 * kTicksPerGameDay;
  s.mains_until = 2 * kTicksPerGameDay;
  s.stations = {{"North Station", 0.5, 20000, false, 10}, {"South Station", 0.5, 20000, true, 25}};
  return s;
}

tier0::Tier0 run() {
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {}, {}, town());
  t.world->check_every_tick = false;
  return t;
}

EntityId fuel_can(const tier0::Tier0& t) {
  for (const auto& [id, inst] : t.items->all())
    if (t.items->def(inst.def).id == "fuel_can") return id;
  return 0;
}

std::int64_t can_fuel(const tier0::Tier0& t, EntityId can) {
  const auto& names = t.world->ledger.resource_names();
  const auto gas = static_cast<ResourceId>(std::find(names.begin(), names.end(), std::string("gasoline_ul")) - names.begin());
  return t.world->ledger.balance(*t.actors->store_of(can), gas);
}

int events(const tier0::Tier0& t, EventType type) {
  int n = 0;
  for (const Event& e : t.world->log.all()) n += e.type == type;
  return n;
}

}  // namespace

TEST_CASE("spec 97: the stations sell the town its day, tankers refill them after the depot's hours (rules 2, 3)") {
  tier0::Tier0 t = run();
  REQUIRE(t.fuel);
  const FuelChain& c = *t.fuel;
  CHECK(c.station_stock(*t.world, 0) == 20000 * kL);
  t.world->jump(21 * kTicksPerGameHour + 1);  // past the first 05:00
  CHECK(c.station_stock(*t.world, 0) == 15000 * kL);
  CHECK(c.sold_to_town() == 10000 * kL);
  REQUIRE(c.deliveries().size() == 2);
  CHECK(c.deliveries().front().amount == 5000 * kL);
  CHECK(c.deliveries().front().arrives == 21 * kTicksPerGameHour + 30 * kTicksPerGameHour);
  t.world->jump(51 * kTicksPerGameHour + 1);  // day 2's sales (5,000 L) out at 05:00, the first tanker (5,000 L) in at 11:00
  CHECK(c.station_stock(*t.world, 0) == 15000 * kL);
}

TEST_CASE("spec 97: without the mains only a station with its own generator sells; no tankers once the depot's staff leave (rule 3)") {
  tier0::Tier0 t = run();
  const FuelChain& c = *t.fuel;
  t.world->jump(2 * kTicksPerGameDay + 1);
  CHECK_FALSE(c.pumps_run(*t.world, 0));
  CHECK(c.pumps_run(*t.world, 1));
  const std::int64_t north = c.station_stock(*t.world, 0), south = c.station_stock(*t.world, 1);
  t.world->jump(3 * kTicksPerGameDay + 1);  // past day 3's 05:00: the depot still has staff until day 3, 08:00
  CHECK(c.station_stock(*t.world, 0) >= north);
  CHECK(c.station_stock(*t.world, 1) <= south);
  t.world->jump(6 * kTicksPerGameDay + 1);  // the last tankers have arrived; none ordered since
  const std::int64_t after = c.station_stock(*t.world, 0);
  CHECK(c.deliveries().empty());
  t.world->jump(7 * kTicksPerGameDay + 1);
  CHECK(c.station_stock(*t.world, 0) == after);  // no pumps, no tankers: the tanks keep what they hold
}

TEST_CASE("spec 97: an errand fills the cans a pop carries; by hand pump once the mains are gone; refusals (rule 4)") {
  tier0::Tier0 t = run();
  const FuelChain& c = *t.fuel;
  const EntityId body = t.group.front();
  // No can carried: refused.
  t.world->submit({t.world->clock.tick, FuelErrand, body, 0, 0});
  t.world->run_ticks(1);
  CHECK(events(t, EventType::FuelRefused) == 1);
  // With the block's 20 L can (15 L in it): 5 L more, back after the walk both ways and the filling.
  const EntityId can = fuel_can(t);
  REQUIRE(can != 0);
  t.world->locations.place(can, body);
  const std::int64_t before = can_fuel(t, can), station = c.station_stock(*t.world, 0);
  const std::int64_t start = t.world->clock.tick;
  t.world->submit({start, FuelErrand, body, 0, 0});
  t.world->run_ticks(1);
  REQUIRE(c.errands().size() == 1);
  CHECK(c.errands().front().back == start + (2 * 10 + 15) * kMin);
  CHECK(c.station_stock(*t.world, 0) == station - (20 * kL - before));
  t.world->jump(start + (2 * 10 + 15) * kMin + 1);
  CHECK(c.errands().empty());
  CHECK(can_fuel(t, can) == 20 * kL);
  CHECK(events(t, EventType::FuelErrandEnded) == 1);
  // A full can: refused. After the mains fail, the errand takes the hand pump's time.
  t.world->submit({t.world->clock.tick, FuelErrand, body, 0, 0});
  t.world->run_ticks(1);
  CHECK(events(t, EventType::FuelRefused) == 2);
  t.world->jump(2 * kTicksPerGameDay + 1);
  REQUIRE_FALSE(c.pumps_run(*t.world, 0));
  CHECK(c.errand_minutes(*t.world, 0) == 2 * 10 + 45);
}

TEST_CASE("spec 97: saving mid-errand and mid-delivery gives the same world as never saving (rule 5)" * doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 a = run(), b = run();
  const EntityId body = a.group.front();
  for (tier0::Tier0* t : {&a, &b}) t->world->locations.place(fuel_can(*t), t->group.front());
  const std::int64_t at = 21 * kTicksPerGameHour + 2 * kTicksPerGameHour;  // tankers on the road
  a.world->jump(at);
  b.world->jump(at);
  a.world->submit({a.world->clock.tick, FuelErrand, body, 0, 0});
  a.world->run_ticks(1);
  REQUIRE(!a.fuel->deliveries().empty());
  REQUIRE(!a.fuel->errands().empty());
  b.world->load(a.world->save());
  a.world->jump(at + kTicksPerGameDay);
  b.world->jump(at + kTicksPerGameDay);
  CHECK(a.world->state_hash() == b.world->state_hash());
}
