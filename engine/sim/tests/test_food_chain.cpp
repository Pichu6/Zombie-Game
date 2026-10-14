// Spec 80: food in play (decision 0079): the cold chain of the block's town and the errand that brings food home.
#include <doctest.h>

#include <algorithm>
#include <string>

#include "zs/body.h"
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
  CHECK(c.food().block_people == doctest::Approx(static_cast<double>(t.census->people().size())));
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
  CHECK(c.outlet_stock(*t.world, 1) == shop_before - c.carry_j());
  const std::int64_t back = c.errands().front().back;
  CHECK(back == t.world->clock.tick - 1 + c.errand_minutes(1) * kMin);
  t.world->submit({t.world->clock.tick, FoodErrand, body, 0, 0});  // already away
  t.world->run_ticks(1);
  CHECK(events(t, EventType::FoodRefused) == 1);
  t.world->jump(back - 1);
  CHECK(pantry(t) < before + c.carry_j());  // not home yet (the household eats meanwhile)
  t.world->jump(back + 1);
  CHECK(!t.bodies->get(body).away);
  CHECK(events(t, EventType::FoodErrandEnded) == 1);
  CHECK(pantry(t) > before);
  // The shut kiosk has nothing; an outlet that does not exist.
  t.world->submit({t.world->clock.tick, FoodErrand, body, 2, 0});
  t.world->run_ticks(1);
  t.world->submit({t.world->clock.tick, FoodErrand, body, 7, 0});
  t.world->run_ticks(1);
  CHECK(events(t, EventType::FoodRefused) == 3);
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
