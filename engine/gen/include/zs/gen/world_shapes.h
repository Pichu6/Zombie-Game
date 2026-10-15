// Spec 89: the land's shapes, measured as tools/data/earth_shapes.py measures Earth (research continents-and-coasts, CC):
// landmasses and their areas, land near the sea, land by latitude. A pure function of the fields.
#pragma once
#include <array>
#include <vector>

#include "zs/gen/world_fields.h"
#include "zs/gen/world_grid.h"

namespace zs::gen {

struct LandShapes {
  double land_km2 = 0;
  std::vector<double> landmasses_km2;            // largest first
  int over_1m = 0, over_100k = 0, over_10k = 0;  // landmasses of at least that many km2
  double islands = 0;                             // share of land in landmasses under 1 million km2
  std::array<double, 5> near_sea{};               // share of land within 50 / 100 / 200 / 400 / 800 km of the sea
  double median_sea_km = 0;                       // area-weighted median distance of land to the sea
  std::array<double, 18> by_latitude{};           // share of land per 10-degree band, -90..-80 first
  double arctic = 0, antarctic = 0, north = 0;    // beyond the polar circles; in the northern hemisphere
  double sea_ice = 0;                             // share of the sea under ice (spec 89 rule 4)
  double thin = 0;                                // share of land in cells with at most two land neighbours (strips)
};

LandShapes land_shapes(const WorldGrid& grid, const WorldFields& fields);

}  // namespace zs::gen
