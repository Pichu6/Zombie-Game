// Spec 80: food in play (decision 0079): the cold chain of the block's town and the errand that brings food home.
#include <doctest.h>

#include <algorithm>
#include <string>

#include "zs/body.h"
#include "zs/dead.h"
#include "zs/events.h"
#include "zs/food_chain.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr std::int64_t kMin = kTicksPerGameHour / 60;

FoodSetup town() {
  FoodSetup s;
  s.town_people = 20000;
  s.kj_per_person_day = 10000;
  s.centre_days = 8;
  s.centre_hours = 6;
  s.outlets = {{"Main Supermarket", "supermarket", 0.6, 3, 9}, {"Corner Shop", "small shop", 0.4, 3, 4}, {"Shut Kiosk", "small shop", 0.0, 0, 2}};
  return s;
}

std::int64_t pantry(const tier0::Tier0& t) {
  const auto owner = t.actors->first_store("pantry");
  REQUIRE(owner);
  const auto& names = t.world->ledger.resource_names();
  const auto food = static_cast<ResourceId>(std::find(names.begin(), names.end(), std::string("food_j")) - names.begin());
  return t.world->ledger.balance(*owner, food);
}

int events(const tier0::Tier0& t, EventType type) {
  int n = 0;
  for (const Event& e : t.world->log.all()) n += e.type == type;
  return n;
}

}  // namespace

TEST_CASE("spec 80: without a food setup there is no chain; with one the centre and outlets start at their targets (A1)") {
  CHECK(tier0::make_run(1).food == nullptr);
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {}, town());
  REQUIRE(t.food);
  const FoodChain& c = *t.food;
  CHECK(c.centre_stock(*t.world) == static_cast<std::int64_t>(20000.0 * 10000 * 8 * 1000));
  for (std::size_t o = 0; o < 3; ++o) CHECK(c.outlet_stock(*t.world, o) == c.outlet_target(o));
  // The block's people: the run's group and the residents of the houses on the map, not the census rows that live off it.
  double on_map = static_cast<double>(t.group.size());
  for (const Person& p : t.census->people()) on_map += p.home.rfind("house_", 0) == 0 ? 1.0 : 0.0;
  CHECK(c.food().block_people == doctest::Approx(on_map));
  CHECK(c.food().block_people < static_cast<double>(t.census->people().size()));
  // Each house on the block has its own pantry, after the main building's (the first, where errands bring food).
  int pantries = 0;
  for (const auto& o : t.map->objects()) pantries += o.kind == "pantry";
  CHECK(pantries == 3);
}

TEST_CASE("spec 80: the town eats its day, the outlets are refilled after the road hours and stay at their targets (A1)") {
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {}, town());
  t.world->check_every_tick = false;
  const FoodChain& c = *t.food;
  t.world->jump(21 * kTicksPerGameHour + 1);  // past the first 05:00 (tick 0 is 08:00)
  CHECK(c.eaten_by_town(*t.world) == c.town_need_j());
  REQUIRE(!c.deliveries().empty());
  const std::int64_t arrives = c.deliveries().front().arrives;
  CHECK(arrives == 21 * kTicksPerGameHour + 6 * kTicksPerGameHour);
  CHECK(c.outlet_stock(*t.world, 0) < c.outlet_target(0));  // sold, the delivery still on the road
  t.world->jump(arrives + 1);
  CHECK(c.outlet_stock(*t.world, 0) == c.outlet_target(0));
  t.world->jump(5 * kTicksPerGameDay + 6 * kTicksPerGameHour);
  CHECK(c.eaten_by_town(*t.world) == 5 * c.town_need_j());
  CHECK(c.outlet_stock(*t.world, 1) == c.outlet_target(1));
}

TEST_CASE("spec 95: the centre is fed by its region's flows: its own produce at once, the rest after the suppliers' hours") {
  FoodSetup s = town();
  s.supply_share = 0.5;   // a region that gets half of what it needs
  s.local_share = 0.4;    // 40 % of that grown at home
  s.import_hours = 30;
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {}, s);
  t.world->check_every_tick = false;
  const FoodChain& c = *t.food;
  const std::int64_t full = c.centre_stock(*t.world);
  // The first morning the centre is full; by the second it has sent a day's orders out and half a day's need comes in:
  // 40 % of it now, 60 % on its way.
  t.world->jump(45 * kTicksPerGameHour + 1);
  std::int64_t to_centre = 0, arrives = 0;
  for (const auto& d : c.deliveries())
    if (d.outlet == FoodChain::kToCentre) to_centre += d.amount, arrives = d.arrives;
  const double day = 20000.0 * 10000 * 1000;
  CHECK(static_cast<double>(to_centre) == doctest::Approx(0.5 * 0.6 * day).epsilon(1e-6));
  CHECK(arrives == 45 * kTicksPerGameHour + 30 * kTicksPerGameHour);
  // Short of food, the centre falls day by day.
  t.world->jump(6 * kTicksPerGameDay);
  CHECK(c.centre_stock(*t.world) < full);
}

TEST_CASE("spec 95: a region that grows all it eats refills the centre at once, as spec 80 did") {
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {}, town());
  t.world->check_every_tick = false;
  const FoodChain& c = *t.food;
  t.world->jump(3 * kTicksPerGameDay);
  for (const auto& d : c.deliveries()) CHECK(d.outlet != FoodChain::kToCentre);
}

TEST_CASE("spec 80: an errand brings what one can carry home after the trip; refusals for away, off-limits and empty (A1)") {
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {}, town());
  t.world->check_every_tick = false;
  FoodChain& c = *t.food;
  REQUIRE(!t.group.empty());
  const EntityId body = t.group.front();
  const std::int64_t before = pantry(t), shop_before = c.outlet_stock(*t.world, 1);
  t.world->submit({t.world->clock.tick, FoodErrand, body, 1, 0});
  t.world->run_ticks(1);
  REQUIRE(c.errands().size() == 1);
  CHECK(t.bodies->get(body).away);
  const std::int64_t took = c.errands().front().amount;
  CHECK(took > 0);
  CHECK(took <= c.carry_j());  // spec 81: no more than one carries, nor than makes the household full
  CHECK(c.outlet_stock(*t.world, 1) == shop_before - took);
  const std::int64_t back = c.errands().front().back;
  CHECK(back == t.world->clock.tick - 1 + c.errand_minutes(1) * kMin);
  t.world->submit({t.world->clock.tick, FoodErrand, body, 0, 0});  // already away
  t.world->run_ticks(1);
  CHECK(events(t, EventType::FoodRefused) == 1);
  t.world->jump(back - 1);
  CHECK(pantry(t) < before + took);  // not home yet (the household eats meanwhile)
  t.world->jump(back + 1);
  CHECK(!t.bodies->get(body).away);
  CHECK(events(t, EventType::FoodErrandEnded) >= 1);
  // The shut kiosk has nothing; an outlet that does not exist.
  t.world->submit({t.world->clock.tick, FoodErrand, body, 2, 0});
  t.world->run_ticks(1);
  t.world->submit({t.world->clock.tick, FoodErrand, body, 7, 0});
  t.world->run_ticks(1);
  CHECK(events(t, EventType::FoodRefused) == 3);
}

// Spec 81 (decision 0080): the block's households shop by themselves and start at a random point of their cycle.

TEST_CASE("spec 81: the block's households, their stores and members; each starts between low and full (A1)") {
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {}, town());
  const FoodChain& c = *t.food;
  REQUIRE(!c.households().empty());
  CHECK(c.households().front().name == "main building");
  CHECK(c.households().front().members == t.group);
  for (std::size_t h = 0; h < c.households().size(); ++h) {
    const auto& x = c.households()[h];
    CHECK(x.need_j == static_cast<std::int64_t>(x.members.size()) * 10'000'000);
    CHECK(x.full_j == 5 * x.need_j);  // 3 days in the pantry and 2 in the fridge
    const std::int64_t s = c.household_stock(*t.world, h);
    CHECK(s <= x.full_j);
    CHECK(s >= c.household_low(h));
    MESSAGE(x.name << ": " << x.members.size() << " members, starts at " << static_cast<double>(s) / static_cast<double>(x.need_j) << " days");
  }
  CHECK(c.eaten_before_start(*t.world) >= 0);
}

TEST_CASE("spec 81: a household at or below its low point sends a member at 17:00 and comes back fuller (A1)") {
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {}, town());
  t.world->check_every_tick = false;
  FoodChain& c = *t.food;
  const auto& x = c.households().front();
  // Empty the main household's stores below its low point (as if the days had passed).
  const auto& names = t.world->ledger.resource_names();
  const auto food = static_cast<ResourceId>(std::find(names.begin(), names.end(), std::string("food_j")) - names.begin());
  const OwnerId sink = t.world->ledger.add_owner("test:eaten");
  t.world->ledger.transfer(0, x.pantry, sink, food, c.household_stock(*t.world, 0) - c.household_low(0) + x.need_j, 0);
  const std::int64_t low = c.household_stock(*t.world, 0);
  REQUIRE(low <= c.household_low(0));
  t.world->jump(9 * kTicksPerGameHour + 1);  // 17:00 of day 0 (tick 0 is 08:00)
  REQUIRE(events(t, EventType::FoodErrandBegan) == 1);
  REQUIRE(c.errands().size() == 1);
  CHECK(c.errands().front().household == 0);
  CHECK(std::find(x.members.begin(), x.members.end(), c.errands().front().body) != x.members.end());
  const std::int64_t back = c.errands().front().back;
  t.world->jump(back + 1);
  CHECK(events(t, EventType::FoodErrandEnded) == 1);
  CHECK(c.household_stock(*t.world, 0) > low);
}

TEST_CASE("spec 81: over a week the block's households keep their food between their low point and full (A1)" *
          doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {}, town());
  t.world->check_every_tick = false;
  const FoodChain& c = *t.food;
  for (int hour = 1; hour <= 7 * 24; ++hour) {
    t.world->jump(hour * kTicksPerGameHour);
    for (std::size_t h = 0; h < c.households().size(); ++h) {
      CHECK(c.household_stock(*t.world, h) <= c.households()[h].full_j);
      CHECK(c.household_stock(*t.world, h) >= c.household_low(h) - 2 * c.households()[h].need_j);  // a day to the next 17:00, and the trip
    }
  }
  CHECK(events(t, EventType::FoodErrandBegan) > 0);
  MESSAGE("errands in a week: " << events(t, EventType::FoodErrandBegan));
}

TEST_CASE("spec 80: saving mid-errand and mid-delivery gives the same world as never saving (A1)" * doctest::skip(World::kDebugBuild)) {
  auto setup = [] {
    tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {}, town());
    t.world->check_every_tick = false;
    return t;
  };
  tier0::Tier0 a = setup(), b = setup();
  const EntityId body = a.group.front();
  const std::int64_t at = 21 * kTicksPerGameHour + 2 * kTicksPerGameHour;  // a delivery on the road
  a.world->jump(at);
  a.world->submit({a.world->clock.tick, FoodErrand, body, 0, 0});
  a.world->run_ticks(1);
  REQUIRE(!a.food->deliveries().empty());
  REQUIRE(!a.food->errands().empty());
  b.world->load(a.world->save());
  a.world->jump(at + kTicksPerGameDay);
  b.world->jump(at + kTicksPerGameDay);
  CHECK(a.world->state_hash() == b.world->state_hash());
}
