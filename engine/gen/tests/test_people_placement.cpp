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

TEST_CASE("placement: people against Earth measured at this cell size (spec 86 rule 1 recalibrated; EM-1, EM-2, EM-4, EM-5, EM-7, EM-8)" *
          doctest::skip(zs::World::kDebugBuild)) {
  PlacementReport m;
  double river_x = 0, large_x = 0;
  const int seeds = 5;
  for (std::uint64_t s = 1; s <= seeds; ++s) {
    const PlacementReport r = report_of(generate_world(s, {}));
    m.coast_100 += r.coast_100 / seeds, m.coast_200 += r.coast_200 / seeds, m.coast_400 += r.coast_400 / seeds;
    m.below_100m += r.below_100m / seeds, m.below_200m += r.below_200m / seeds, m.below_500m += r.below_500m / seeds;
    m.median_m += r.median_m / seeds;
    m.million_coastal += r.million_coastal / seeds;
    m.largest_country += r.largest_country / seeds;
    m.two_largest += r.two_largest / seeds, m.ten_largest += r.ten_largest / seeds;
    m.half_cells += r.half_cells / seeds, m.cells_80 += r.cells_80 / seeds, m.empty_cells += r.empty_cells / seeds;
    m.median_cell_density += r.median_cell_density / seeds, m.median_person_density += r.median_person_density / seeds;
    river_x += r.river_people / r.river_land / seeds;
    large_x += r.large_people / r.large_land / seeds;
  }
  MESSAGE("seeds 1-5: within 100/200/400 km of the sea " << m.coast_100 << " / " << m.coast_200 << " / " << m.coast_400 << "; below 100/200/500 m "
          << m.below_100m << " / " << m.below_200m << " / " << m.below_500m << ", median " << m.median_m << " m; river cells x" << river_x
          << ", large x" << large_x << "; half of people on " << m.half_cells << " of land cells, 80 % on " << m.cells_80 << "; empty cells "
          << m.empty_cells << "; median cell " << m.median_cell_density << " a km2, the median person's " << m.median_person_density
          << "; million-towns near the sea " << m.million_coastal << "; largest country " << m.largest_country << ", two largest "
          << m.two_largest << ", ten largest " << m.ten_largest);
  // EM-1: 38.1 / 50.1 / 66.0 %. The 200 and 400 km shares run high here (about 60 and 76 %): the generated land seems
  // more coastal than Earth's (open, the spec's amendment), so their bands are wider above.
  CHECK(m.coast_100 >= 0.32);
  CHECK(m.coast_100 <= 0.44);
  CHECK(m.coast_200 >= 0.44);
  CHECK(m.coast_200 <= 0.64);
  CHECK(m.coast_400 >= 0.60);
  CHECK(m.coast_400 <= 0.83);  // temperature recalibration (2026-10-17): hotter, drier continental interiors hold fewer people (0.81)
  // EM-2: 43.6 / 56.9 / 76.6 %, median 143 m.
  CHECK(m.below_100m >= 0.37);
  CHECK(m.below_100m <= 0.50);
  CHECK(m.below_200m >= 0.50);
  CHECK(m.below_200m <= 0.64);
  CHECK(m.below_500m >= 0.70);
  CHECK(m.below_500m <= 0.83);
  CHECK(m.median_m >= 90);
  CHECK(m.median_m <= 220);
  // EM-4, EM-5: river cells 1.77 times the land's mean density, large-river cells 2.78.
  CHECK(river_x >= 1.4);
  CHECK(river_x <= 2.3);
  CHECK(large_x >= 2.2);
  CHECK(large_x <= 3.5);
  // EM-7, EM-8: half of people on 3.1 % of land cells, 80 % on 10.6 %; 48.5 % of land cells under 1 a km2.
  CHECK(m.half_cells >= 0.02);
  CHECK(m.half_cells <= 0.05);
  CHECK(m.cells_80 >= 0.07);
  CHECK(m.cells_80 <= 0.14);
  CHECK(m.empty_cells >= 0.38);
  CHECK(m.empty_cells <= 0.58);
  // PTP-6: 53 % (about 65 % here, with the coastal surplus above).
  CHECK(m.million_coastal >= 0.45);
  CHECK(m.million_coastal <= 0.72);
  // Spec 87. PTP-7: 18 % and 35 %; PTP-11: 57 % (still high here: spec 87 As built).
  CHECK(m.largest_country >= 0.12);
  CHECK(m.largest_country <= 0.24);
  CHECK(m.two_largest >= 0.22);
  CHECK(m.two_largest <= 0.42);
  CHECK(m.ten_largest <= 0.82);  // 80 % since spec 89 (PTP-11: 57 %): open, spec 87 As built
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
