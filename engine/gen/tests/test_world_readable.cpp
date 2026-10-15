// Spec 83: rain by the side of a continent, biomes by aridity in Earth's shares, lakes that balance and closed basins.
#include <doctest.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "zs/gen/place.h"
#include "zs/gen/world_land.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

struct Planet {
  WorldGrid grid;
  WorldFields fields;
  WorldLand land;
};

Planet make_planet(int frequency, std::uint64_t seed) {
  Planet q{WorldGrid(frequency, 6371.0), {}, {}};
  WorldParams wp;
  wp.seed = seed;
  q.fields = generate_fields(q.grid, wp);
  q.land = generate_land(q.grid, q.fields);
  return q;
}

const Planet& seed1() {
  static const Planet p = make_planet(kOneWorldFrequency, 1);
  return p;
}

double median(std::vector<double> v) {
  if (v.empty()) return 0;
  std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
  return v[v.size() / 2];
}

bool subtropical(const WorldGrid& g, CellId i) {
  const double az = std::abs(g.sin_lat(i));
  return az >= 0.259 && az <= 0.574;  // 15 to 35 degrees
}

// The share of west-side subtropical land that is desert.
double west_desert(const Planet& p) {
  double in = 0, all = 0;
  for (CellId i = 0; i < p.grid.size(); ++i) {
    if (!p.fields.land(i) || !subtropical(p.grid, i) || east_index(p.fields, i) >= 0.25) continue;
    all += p.grid.area_km2(i);
    if (p.fields.biome[i] == Biome::Desert) in += p.grid.area_km2(i);
  }
  return all > 0 ? in / all : 0;
}

}  // namespace

TEST_CASE("readable: the west side of a continent in the subtropics is drier than its east side (spec 83 rule 2)") {
  const Planet& p = seed1();
  std::vector<double> west, east;
  for (CellId i = 0; i < p.grid.size(); ++i) {
    if (!p.fields.land(i) || !subtropical(p.grid, i)) continue;
    const double e = east_index(p.fields, i);
    if (e < 0.25) west.push_back(p.fields.rain_mm[i]);
    if (e > 0.75) east.push_back(p.fields.rain_mm[i]);
  }
  MESSAGE("subtropical median rain: west side ", median(west), " mm (", west.size(), " cells), east side ", median(east),
          " mm (", east.size(), ")");
  REQUIRE(!west.empty());
  REQUIRE(!east.empty());
  CHECK(median(west) < 0.5 * median(east));
}

TEST_CASE("readable: biome shares near Earth's (spec 83 rule 3, WMR-1)") {
  const Planet& p = seed1();
  double area[biome_count] = {};
  for (CellId i = 0; i < p.grid.size(); ++i)
    if (p.fields.land(i)) area[static_cast<int>(p.fields.biome[i])] += p.grid.area_km2(i);
  double non_ice = 0;
  for (int b = 2; b < biome_count; ++b) non_ice += area[b];
  auto share = [&](Biome b) { return 100.0 * area[static_cast<int>(b)] / non_ice; };
  for (int b = 2; b < biome_count; ++b) MESSAGE(std::string(biome_name(static_cast<Biome>(b))) << " " << share(static_cast<Biome>(b)) << " %");
  CHECK(share(Biome::TropicalForest) >= 12);
  CHECK(share(Biome::TropicalForest) <= 28);  // seed 1 26.7 % with spec 89; five seeds average 20.3 (Earth 18)  // seed 1 24.6 after the plate fix; five seeds average 19.5 (Earth 18)
  CHECK(share(Biome::Savanna) >= 11);
  CHECK(share(Biome::Savanna) <= 25);  // spec 89 second amendment: seed 1 23.6 (its land lies low)
  CHECK(share(Biome::TemperateForest) >= 9);
  CHECK(share(Biome::TemperateForest) <= 20);
  CHECK(share(Biome::Grassland) >= 4);
  CHECK(share(Biome::Grassland) <= 12);
  CHECK(share(Biome::Desert) >= 13);
  CHECK(share(Biome::Desert) <= 25);
  CHECK(share(Biome::BorealForest) >= 4);  // spec 89 second amendment: seed 1 5.4 (no land beyond 68 degrees; Koppen tundra)
  CHECK(share(Biome::BorealForest) <= 16);
  // Tundra: seed 1 has little land near the poles (4.3 % against Earth's 8.7); the band is wide.
  CHECK(share(Biome::Tundra) >= 2);  // spec 89 second amendment: no land beyond about 68 degrees; Koppen tundra
  CHECK(share(Biome::Tundra) <= 13);
}

TEST_CASE("readable: lakes and closed basins in Earth's bands (spec 83 rule 4, WMR-8, WMR-10)") {
  const Planet& p = seed1();
  double land = 0, lakes = 0, endorheic = 0;
  int terminal = 0, dry = 0;
  for (CellId i = 0; i < p.grid.size(); ++i) {
    if (!p.fields.land(i)) continue;
    land += p.grid.area_km2(i);
    if (p.land.lake[i] != kLakeNone) lakes += p.grid.area_km2(i);
    if (p.land.endorheic[i]) endorheic += p.grid.area_km2(i);
    terminal += p.land.lake[i] == kLakeTerminal;
    dry += p.land.lake[i] == kLakeDry;
  }
  MESSAGE("lakes ", 100 * lakes / land, " % of land, endorheic ", 100 * endorheic / land, " %; ", p.land.open_lakes,
          " open lakes, ", p.land.closed_basins, " closed basins (", terminal, " terminal and ", dry, " dry cells)");
  CHECK(lakes / land >= 0.005);
  CHECK(lakes / land <= 0.03);
  CHECK(endorheic / land >= 0.08);
  CHECK(endorheic / land <= 0.30);
  CHECK(terminal >= 1);
  CHECK(dry >= 1);
  // No river carries water out of a closed basin.
  for (CellId i = 0; i < p.grid.size(); ++i)
    if (p.fields.land(i) && p.land.endorheic[i] && p.land.downstream[i] != kNoCell)
      CHECK(p.land.endorheic[p.land.downstream[i]] == 1);
}

TEST_CASE("readable: the same desert sides on a coarser grid (spec 83 Tests)" * doctest::skip(zs::World::kDebugBuild)) {
  const Planet coarse = make_planet(48, 1);
  const double a = west_desert(seed1()), b = west_desert(coarse);
  MESSAGE("west-side subtropical land that is desert: frequency 101 ", a, ", frequency 48 ", b);
  CHECK(std::abs(a - b) < 0.15);
}

TEST_CASE("readable: biome shares over five seeds near Earth's (spec 83 As built)" * doctest::skip(zs::World::kDebugBuild)) {
  static const double earth[biome_count] = {0, 0, 8.7, 11.4, 14.5, 7.8, 19.3, 16.7, 18.0, 3.6};
  double mean[biome_count] = {};
  for (std::uint64_t s = 1; s <= 5; ++s) {
    const Planet p = make_planet(kOneWorldFrequency, s);
    double area[biome_count] = {}, non_ice = 0;
    for (CellId i = 0; i < p.grid.size(); ++i)
      if (p.fields.land(i)) area[static_cast<int>(p.fields.biome[i])] += p.grid.area_km2(i);
    for (int b = 2; b < biome_count; ++b) non_ice += area[b];
    for (int b = 2; b < biome_count; ++b) mean[b] += 100.0 * area[b] / non_ice / 5;
  }
  for (int b = 2; b < biome_count - 1; ++b) {  // mountain depends on collisions, not on this spec
    CAPTURE(biome_name(static_cast<Biome>(b)));
    MESSAGE(std::string(biome_name(static_cast<Biome>(b))) << " " << mean[b] << " % (Earth " << earth[b] << ")");
    
    // Spec 89 second amendment: no land beyond about 68 degrees, so part of Earth's tundra and boreal forest has no
    // place (five seeds: tundra 6.4 %, boreal 7.7 %).
    const bool cold = static_cast<Biome>(b) == Biome::Tundra || static_cast<Biome>(b) == Biome::BorealForest;
    CHECK(mean[b] >= (cold ? 0.6 : 0.7) * earth[b]);
    CHECK(mean[b] <= 1.3 * earth[b]);
  }
}
