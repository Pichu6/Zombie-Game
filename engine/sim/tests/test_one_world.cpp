// Spec 41: the one world. The census rows are the registry's founders; the economy, orders and the grid tree run
// in the same world as the Tier 0 run; save and load keep it whole.
#include <doctest.h>

#include <algorithm>
#include <cmath>

#include "zs/economy_system.h"
#include "zs/grid_system.h"
#include "zs/orders_system.h"
#include "zs/place.h"
#include "zs/pop_system.h"
#include "zs/tier0.h"

using namespace zs;
using namespace zs::tier0;

namespace {

void run_hours(Tier0& t, double h) {
  const auto end = t.world->clock.tick + static_cast<std::int64_t>(h * kTicksPerGameHour);
  while (t.world->clock.tick < end) t.world->step();
}

}  // namespace

TEST_CASE("one world: every census row is a registry founder with its age, sex and household") {
  Tier0 t = make_run(1);
  REQUIRE(t.pops);
  const PopRegistry& reg = t.pops->registry();
  const auto& people = t.census->people();
  REQUIRE(people.size() == 61);  // 60 neighbours and the player
  for (std::size_t i = 0; i < people.size(); ++i) {
    PopSnapshot s;
    REQUIRE(reg.snapshot(census_pop_id(static_cast<std::uint32_t>(i)), 0.0, s));
    CHECK(static_cast<int>(std::floor(-s.base.birth_year)) == people[i].age);
    CHECK((s.base.sex == PopSex::Female) == (people[i].sex == Sex::Female));
    CHECK((s.base.death_year <= 0.0) == is_dead(people[i].fate));
  }
  // Households: two people of the same household share a roster.
  std::size_t shared = 0;
  for (std::size_t i = 0; i + 1 < people.size(); ++i)
    if (!people[i].household.empty() && people[i].household == people[i + 1].household) {
      const auto a = census_pop_id(static_cast<std::uint32_t>(i)), b = census_pop_id(static_cast<std::uint32_t>(i + 1));
      bool together = false;
      for (std::uint64_t h = 0; h < 64 && !together; ++h)
        if (const Roster* r = reg.roster((std::uint64_t{1} << 32) | h)) {
          const auto m = r->members(0.0);
          together = std::find(m.begin(), m.end(), a) != m.end() && std::find(m.begin(), m.end(), b) != m.end();
        }
      CHECK(together);
      ++shared;
    }
  CHECK(shared > 0);
}

TEST_CASE("one world: owners, money, the grid tree and the orders audience") {
  Tier0 t = make_run(1);
  const Owners& o = t.econ->owners();
  CHECK(o.scope(t.owners.country) == Scope::Country);
  CHECK(o.parent(t.owners.region) == t.owners.country);
  CHECK(o.parent(t.owners.house_a) == t.owners.region);
  REQUIRE(t.owners.pops.size() == t.census->people().size());
  const ResourceId money = t.econ->money().resource();
  CHECK(t.world->ledger.balance(t.owners.region, money) > 0);
  const GridTree::Summary d = t.grids->tree().summary(t.owners.district);
  CHECK(d.demand_w == 12'000);
  CHECK(d.unserved_w == 0);
  std::size_t living = 0;
  for (const Person& p : t.census->people()) living += !is_dead(p.fate);
  CHECK(t.orders->audience().size() == living);
}

TEST_CASE("one world: a death in the census is a death in the registry; save and load keep the hash"
          * doctest::skip(World::kDebugBuild)) {
  Tier0 t = make_run(3);
  t.world->check_every_tick = true;
  run_hours(t, 1);
  const auto mid = t.world->save();
  run_hours(t, 47);
  const PopRegistry& reg = t.pops->registry();
  const double now = year_of_tick(t.world->clock.tick);
  const auto& people = t.census->people();
  for (std::size_t i = 0; i < people.size(); ++i) {
    if (!is_dead(people[i].fate)) continue;
    PopSnapshot s;
    REQUIRE(reg.snapshot(census_pop_id(static_cast<std::uint32_t>(i)), now, s));
    CHECK(s.base.death_year <= now);
  }
  CHECK(!t.world->last_violation);
  Tier0 u = make_run(3);
  u.world->load(mid);
  run_hours(u, 47);
  CHECK(u.world->world_hash() == t.world->world_hash());
}

// ---- Spec 42: the place.

TEST_CASE("place: the default place; the outdoor temperature over a year averages the climate's mean") {
  Tier0 t = make_run(1);
  REQUIRE(t.place);
  const Place& p = t.place->place();
  CHECK(p.frequency == 0);
  CHECK(p.climate.temp_mean_c == 12.0);
  // Daily means through the system over a whole year (no stepping: the place is a pure read of the tick).
  World probe(1);
  double sum = 0;
  for (int d = 0; d < fields::kDaysPerYear; ++d) {
    probe.clock.tick = static_cast<std::int64_t>(d) * kTicksPerGameDay;
    sum += t.place->today_mean_c(probe);
  }
  CHECK(sum / fields::kDaysPerYear == doctest::Approx(p.climate.temp_mean_c).epsilon(1e-9));
  CHECK(t.place->indoor_c(probe) == doctest::Approx(t.place->today_mean_c(probe) + 3.0));
}

TEST_CASE("place: an unpowered fridge warms towards a hot place faster than a cold one; save keeps the place") {
  Place hot = default_place(), cold = default_place();
  hot.climate = {30.0, 8.0, 800.0, false};
  cold.climate = {-5.0, 8.0, 800.0, false};
  double fridge[2];
  int k = 0;
  for (const Place& where : {hot, cold}) {
    Tier0 t = make_run(1, where);
    t.world->check_every_tick = false;
    t.power->fail_grid(*t.world, 0);
    run_hours(t, 3);
    fridge[k++] = t.power->fridge_temp_c();
  }
  CHECK(fridge[0] > fridge[1] + 1.0);
  Tier0 t = make_run(1, hot);
  const auto bytes = t.world->save();
  Tier0 u = make_run(1);  // a default place, then the save restores the hot one
  u.world->load(bytes);
  CHECK(u.place->place().climate.temp_mean_c == 30.0);
  CHECK(u.world->world_hash() == t.world->world_hash());
}
