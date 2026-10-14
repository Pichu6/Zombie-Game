// Spec 88: extraction zones: fields, pastures and working forests in rings around their market towns; worked mines and
// oil and gas fields; fishing grounds from the ports; food grown from the fields in use.
#include <doctest.h>

#include <algorithm>
#include <vector>

#include "zs/gen/place.h"
#include "zs/gen/world_food.h"
#include "zs/gen/world_routes.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

double weighted_median(std::vector<std::pair<double, double>> v) {  // (value, weight)
  std::sort(v.begin(), v.end());
  double total = 0;
  for (const auto& x : v) total += x.second;
  double acc = 0;
  for (const auto& x : v)
    if ((acc += x.second) >= total / 2) return x.first;
  return 0;
}

}  // namespace

TEST_CASE("land use: farmed, grazed and logged land and the fish landed near Earth's totals (spec 88 rules 2, 4; EZ-1, EZ-2, EZ-5)" *
          doctest::skip(zs::World::kDebugBuild)) {
  double crop = 0, pasture = 0, forest = 0, fish = 0;
  const int seeds = 5;
  for (std::uint64_t s = 1; s <= seeds; ++s) {
    const WorldLandUse& u = generate_world(s, {}).use;
    crop += u.crop_km2 / u.land_km2 / seeds;
    pasture += u.pasture_km2 / u.land_km2 / seeds;
    forest += u.forest_worked_km2 / u.forest_km2 / seeds;
    fish += u.fish_landed_t / seeds;
  }
  MESSAGE("seeds 1-5: cropland " << crop << " of land, pasture " << pasture << ", working forest " << forest << " of forest, fish "
                                 << fish / 1e6 << " million t");
  CHECK(crop >= 0.09);
  CHECK(crop <= 0.15);
  CHECK(pasture >= 0.19);
  CHECK(pasture <= 0.31);
  CHECK(forest >= 0.22);
  CHECK(forest <= 0.38);
  CHECK(fish >= 65e6);
  CHECK(fish <= 95e6);
}

TEST_CASE("land use: use falls with distance to market, cropland nearest (spec 88 rule 2; EZ-6)") {
  const GeneratedWorld w = generate_world(1, {}, kTestFrequency);
  std::vector<std::pair<double, double>> crop, pasture, all;
  for (CellId c = 0; c < w.grid.size(); ++c) {
    if (!w.fields.land(c) || w.use.market[c] < 0) continue;
    const double a = w.grid.area_km2(c), d = w.use.market_km[c];
    crop.push_back({d, w.use.crop[c] * a});
    pasture.push_back({d, w.use.pasture[c] * a});
    all.push_back({d, a});
  }
  const double mc = weighted_median(crop), mp = weighted_median(pasture), ma = weighted_median(all);
  MESSAGE("median market distance: cropland " << mc << " km, pasture " << mp << " km, all land " << ma << " km");
  CHECK(mc <= mp);
  CHECK(mp <= ma);
}

TEST_CASE("land use: every worked site has a market in its own country; fishing stays within reach of its port (spec 88 rules 3, 4)") {
  const GeneratedWorld w = generate_world(2, {}, kTestFrequency);
  REQUIRE(!w.use.sites.empty());
  for (const WorkSite& s : w.use.sites) {
    REQUIRE(s.market >= 0);
    if (w.fields.land(s.cell)) {
      CHECK(s.country >= 0);
      CHECK(w.people.towns[static_cast<std::size_t>(s.market)].country == s.country);
    } else {
      const Deposit& d = w.deposits.deposits[static_cast<std::size_t>(s.deposit)];
      CHECK((d.kind == Resource::Oil || d.kind == Resource::Gas));
      CHECK(w.fields.elevation_m[s.cell] >= LandUseParams{}.shelf_m);
    }
  }
  for (CellId c = 0; c < w.grid.size(); ++c) {
    if (w.use.fish_t[c] <= 0) continue;
    REQUIRE(w.use.fish_port[c] >= 0);
    CHECK(w.grid.distance_km(c, w.people.towns[static_cast<std::size_t>(w.use.fish_port[c])].cell) <= LandUseParams{}.fish_km + 1);
  }
}

TEST_CASE("land use: food is grown from the fields in use; the world still makes 1.15 times its need (spec 88 rule 5)" *
          doctest::skip(zs::World::kDebugBuild)) {
  const GeneratedWorld w = generate_world(1, {});
  const WorldRoutes routes = build_routes(w.grid, w.fields, w.land, w.people);
  const WorldFood f = food_flows(w.land, w.people, routes, {}, &w.use);
  CHECK(f.production == doctest::Approx(1.15 * f.demand).epsilon(0.01));
  MESSAGE("seed 1 with the fields in use: across borders " << f.international_share() << ", traded " << f.traded / f.production);
  CHECK(f.international_share() > 0.12);
  CHECK(f.international_share() < 0.35);
}
