// Spec 97: the town's filling stations by its income group, on plots the food outlets left; the depot over the routes.
#include <doctest.h>

#include <algorithm>
#include <set>

#include "zs/fuel_chain.h"
#include "zs/gen/place.h"
#include "zs/gen/town_fuel.h"
#include "zs/tier0.h"

using namespace zs::gen;

TEST_CASE("town fuel: stations by people and income, their stock in the tanks (rule 1)") {
  const FuelRetailParams p = load_fuel_retail(zs::tier0::data_dir() + "/gen");
  const TownLayout none{};
  const TownFuel rich = plan_town_fuel(none, 45000, 0, p, 1), poor = plan_town_fuel(none, 45000, 3, p, 1);
  CHECK(rich.stations.size() == 10);  // 45,000 / 4,500
  CHECK(poor.stations.size() == 2);   // 45,000 / 30,000, rounded
  CHECK(rich.litres_per_day == doctest::Approx(45000 * p.groups[0].litres_per_person_day));
  double share = 0;
  for (const FuelStation& s : rich.stations) {
    share += s.share;
    CHECK(s.stock_l == doctest::Approx(s.tank_l * p.groups[0].stock_share));
  }
  CHECK(share == doctest::Approx(1.0));
  CHECK(plan_town_fuel(none, 300, 0, p, 1).stations.size() == 1);  // a town keeps one
}

TEST_CASE("town fuel: the depot is a refinery or a port reached over the routes, at least an hour away (rule 2)") {
  const GeneratedWorld w = generate_world(1, {}, 32);
  int found = 0;
  for (std::size_t t = 0; t < w.people.towns.size(); t += 61) {
    const FuelDepot d = fuel_depot(w.routes, w.people, w.industry, static_cast<int>(t));
    if (d.town < 0) continue;
    ++found;
    CHECK(d.hours >= 1.0);
    const Settlement& s = w.people.towns[static_cast<std::size_t>(d.town)];
    CHECK((d.refinery || s.site == TownSite::Port || s.site == TownSite::Mouth));
  }
  CHECK(found > 0);
}

TEST_CASE("town fuel: the one world's stations stand on plots no food outlet holds, and the chain runs (rules 1, 5)") {
  const auto t = make_world(1);
  REQUIRE(t.fuel);
  REQUIRE(t.food);
  const zs::FuelSetup& f = t.fuel->fuel();
  CHECK(!f.stations.empty());
  CHECK(f.town_litres_day > 0);
  CHECK(f.mains_until == t.place->place().grid_fail_tick);
  CHECK(f.depot_until > 0);
  std::set<std::string> outlets;
  for (const auto& o : t.food->food().outlets) outlets.insert(o.name);
  for (const auto& s : f.stations) CHECK(outlets.count(s.name) == 0);
}
