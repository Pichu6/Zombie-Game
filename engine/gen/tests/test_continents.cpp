// Spec 89: continents from cratons apart from plates, island arcs as chains, no seas shut in the land, sea ice; the land's
// shapes against Earth's (tools/data/earth_shapes.py, research continents-and-coasts CC).
#include <doctest.h>

#include <algorithm>
#include <vector>

#include "zs/gen/place.h"
#include "zs/gen/world_shapes.h"
#include "zs/world.h"

using namespace zs::gen;

TEST_CASE("continents: the land's shapes near Earth's over five seeds (spec 89; CC)" * doctest::skip(zs::World::kDebugBuild)) {
  double l1 = 0, l2 = 0, m1 = 0, m100 = 0, isl = 0, n200 = 0, n400 = 0, n800 = 0, med = 0, ice = 0;
  const int seeds = 5;
  for (std::uint64_t s = 1; s <= seeds; ++s) {
    const WorldGrid g(kOneWorldFrequency, 6371.0);
    WorldParams wp;
    wp.seed = s;
    const LandShapes ls = land_shapes(g, generate_fields(g, wp));
    l1 += ls.landmasses_km2[0] / ls.land_km2 / seeds;
    l2 += ls.landmasses_km2[1] / ls.land_km2 / seeds;
    m1 += static_cast<double>(ls.over_1m) / seeds;
    m100 += static_cast<double>(ls.over_100k) / seeds;
    isl += ls.islands / seeds;
    n200 += ls.near_sea[2] / seeds, n400 += ls.near_sea[3] / seeds, n800 += ls.near_sea[4] / seeds;
    med += ls.median_sea_km / seeds;
    ice += ls.sea_ice / seeds;
  }
  MESSAGE("seeds 1-5: largest landmasses " << l1 << " / " << l2 << "; of 1M km2 " << m1 << ", of 100k " << m100 << "; islands " << isl
          << "; land within 200/400/800 km of the sea " << n200 << " / " << n400 << " / " << n800 << ", median " << med << " km; sea ice "
          << ice << " of the sea");
  CHECK(l1 >= 0.40);  // Earth 53.5 %
  CHECK(l1 <= 0.65);
  CHECK(l2 >= 0.12);  // 26.1 %
  CHECK(l2 <= 0.32);
  CHECK(m1 >= 3);  // 5
  CHECK(m1 <= 9);
  CHECK(m100 >= 10);  // spec 89 amendment: no land beyond 60 degrees leaves fewer middle-sized masses  // 20
  CHECK(m100 <= 30);
  CHECK(isl >= 0.015);  // 4.4 %
  CHECK(isl <= 0.08);
  CHECK(n200 >= 0.21);  // 29.3 %
  CHECK(n200 <= 0.37);
  CHECK(n400 >= 0.40);  // 47.6 %
  CHECK(n400 <= 0.56);
  CHECK(n800 >= 0.64);  // 72.6 %
  CHECK(n800 <= 0.81);
  CHECK(med >= 330);  // 431 km
  CHECK(med <= 560);
  CHECK(ice >= 0.03);  // about 6 % (NSIDC annual means)
  CHECK(ice <= 0.10);
}

TEST_CASE("continents: no small sea is shut inside the land; land stands on continental crust (spec 89 rules 1, 2)") {
  const WorldGrid g(kTestFrequency, 6371.0);
  for (std::uint64_t s = 1; s <= 4; ++s) {
    WorldParams wp;
    wp.seed = s;
    const WorldFields f = generate_fields(g, wp);
    // Sea components: every one but the largest is at least two million km2.
    std::vector<int> comp(g.size(), -1);
    std::vector<double> area;
    for (CellId c = 0; c < g.size(); ++c) {
      if (f.land(c) || comp[c] >= 0) continue;
      area.push_back(0);
      std::vector<CellId> stack{c};
      comp[c] = static_cast<int>(area.size()) - 1;
      while (!stack.empty()) {
        const CellId x = stack.back();
        stack.pop_back();
        area.back() += g.area_km2(x);
        for (const CellId nb : g.neighbours(x))
          if (!f.land(nb) && comp[nb] < 0) comp[nb] = comp[c], stack.push_back(nb);
      }
    }
    std::sort(area.rbegin(), area.rend());
    // After the level is found again a basin cell may drop back below it: shut seas under two million km2 hold at most
    // 1 % of the sea (on this coarse grid a cell is 88,000 km2).
    double sea = 0, shut = 0;
    for (std::size_t k = 0; k < area.size(); ++k) sea += area[k], shut += (k > 0 && area[k] < 2.0e6) ? area[k] : 0.0;
    CHECK(shut <= 0.01 * sea);
    double land = 0, on_crust = 0;
    for (CellId c = 0; c < g.size(); ++c)
      if (f.land(c)) land += g.area_km2(c), on_crust += f.crust[c] ? g.area_km2(c) : 0.0;
    CHECK(on_crust / land >= 0.85);
  }
}

TEST_CASE("continents: one craton makes a supercontinent, many make an archipelago world (spec 89 setting)") {
  const WorldGrid g(48, 6371.0);
  WorldParams one, many;
  one.seed = many.seed = 3;
  one.cratons = 1;
  many.cratons = 30;
  const LandShapes a = land_shapes(g, generate_fields(g, one)), b = land_shapes(g, generate_fields(g, many));
  MESSAGE("largest landmass: one craton " << a.landmasses_km2[0] / a.land_km2 << ", thirty " << b.landmasses_km2[0] / b.land_km2);
  CHECK(a.landmasses_km2[0] / a.land_km2 > 0.6);
  CHECK(b.landmasses_km2[0] / b.land_km2 < a.landmasses_km2[0] / a.land_km2);
}
