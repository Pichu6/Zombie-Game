// Spec 86: people by food, fresh water, coast and height; towns at ports, river mouths, confluences, rivers and resources.
#include <doctest.h>

#include <algorithm>
#include <cmath>
#include <string>

#include "zs/gen/place.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

PlacementReport report_of(const GeneratedWorld& w) { return placement_report(w.grid, w.fields, w.land, w.people); }

}  // namespace

TEST_CASE("placement: people crowd the coast, low land and water as on Earth (spec 86 rule 1, PTP-1, PTP-4, PTP-5)" *
          doctest::skip(zs::World::kDebugBuild)) {
  PlacementReport mean;
  const int seeds = 5;
  for (std::uint64_t s = 1; s <= seeds; ++s) {
    const PlacementReport r = report_of(generate_world(s, {}));
    mean.coast_100 += r.coast_100 / seeds, mean.coast_200 += r.coast_200 / seeds, mean.coast_400 += r.coast_400 / seeds;
    mean.below_100m += r.below_100m / seeds, mean.below_500m += r.below_500m / seeds, mean.median_m += r.median_m / seeds;
    mean.million_coastal += r.million_coastal / seeds;
    mean.largest_country += r.largest_country / seeds;
    CHECK(r.water_people > r.water_land);  // people gather where rivers and lakes are
  }
  MESSAGE("seeds 1-5: within 100/200/400 km of the sea " << mean.coast_100 << " / " << mean.coast_200 << " / " << mean.coast_400
          << "; below 100 m " << mean.below_100m << ", below 500 m " << mean.below_500m << ", median " << mean.median_m
          << " m; million-towns near the sea " << mean.million_coastal << "; largest country " << mean.largest_country);
  CHECK(mean.coast_100 >= 0.32);
  CHECK(mean.coast_100 <= 0.44);
  CHECK(mean.coast_200 >= 0.44);
  CHECK(mean.coast_200 <= 0.56);
  CHECK(mean.coast_400 >= 0.61);
  CHECK(mean.coast_400 <= 0.73);
  CHECK(mean.below_100m >= 0.27);
  CHECK(mean.below_100m <= 0.40);
  CHECK(mean.below_500m >= 0.72);
  CHECK(mean.below_500m <= 0.84);
  CHECK(mean.median_m >= 50);
  CHECK(mean.median_m <= 350);
  CHECK(mean.million_coastal >= 0.45);  // PTP-6: 53 %
  CHECK(mean.million_coastal <= 0.61);
  CHECK(mean.largest_country >= 0.10);  // PTP-7: 18 %; spec 86 Tests: reported, banded wide
  CHECK(mean.largest_country <= 0.28);
}

TEST_CASE("placement: towns record their site; large deposits have resource towns (spec 86 rules 2, 3)") {
  const GeneratedWorld w = generate_world(1, {}, kTestFrequency);
  const PlacementReport r = report_of(w);
  int sited = 0;
  for (int k = 1; k < kTownSites; ++k) sited += r.sites[static_cast<std::size_t>(k)];
  CHECK(sited > 0);
  CHECK(r.sites[static_cast<std::size_t>(TownSite::Resource)] > 0);
  for (const Settlement& t : w.people.towns) {
    if (t.site != TownSite::Resource) continue;
    CHECK(t.people >= kTownMinPeople);
    CHECK(t.people <= PlacementParams{}.resource_town + 1e-6);
    // A resource town stands on a large deposit of its country.
    bool on_deposit = false;
    for (const Deposit& d : w.deposits.deposits) on_deposit = on_deposit || d.cell == t.cell;
    CHECK(on_deposit);
  }
}

TEST_CASE("placement: the people weight prefers the coast, rivers and low land, all else equal (spec 86 rule 1)") {
  const GeneratedWorld w = generate_world(2, {}, kTestFrequency);
  // A cell's weight per km2 over the land: coastal cells and river cells weigh more on average than inland dry cells.
  double coast = 0, inland = 0;
  int nc = 0, ni = 0;
  double food = 0, area = 0;
  for (CellId c = 0; c < w.grid.size(); ++c)
    if (w.fields.land(c)) food += w.land.capacity_food[c], area += w.grid.area_km2(c);
  for (CellId c = 0; c < w.grid.size(); ++c) {
    if (!w.fields.land(c) || suitability(w.fields, c) <= 0) continue;
    const double per = people_weight(w.grid, w.fields, w.land, c, food / area) / w.grid.area_km2(c) / std::pow(suitability(w.fields, c), 1.5);
    if (w.fields.sea_cells[c] == 1) coast += per, ++nc;
    else if (w.fields.sea_cells[c] >= 4 && !w.land.river[c]) inland += per, ++ni;
  }
  REQUIRE(nc > 0);
  REQUIRE(ni > 0);
  CHECK(coast / nc > inland / ni);
}
