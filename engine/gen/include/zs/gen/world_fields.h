// World fields: plates, elevation, sea, temperature, rain, biome (spec 31).
// A pure function of (grid, parameters, seed); saved with the world, never regenerated over a save.
#pragma once
#include <cstdint>
#include <vector>

#include "zs/gen/world_grid.h"

namespace zs::gen {

enum class Biome : std::uint8_t {
  Ocean, Ice, Tundra, BorealForest, TemperateForest, Grassland, Desert, Savanna, TropicalForest, Mountain,
};
inline constexpr int biome_count = 10;
const char* biome_name(Biome b);

// Why a cell's elevation is what it is: the kind of the strongest plate boundary effect reaching it.
enum class BoundaryKind : std::uint8_t { None, Collision, Subduction, Trench, IslandArc, Ridge, Rift };
const char* boundary_name(BoundaryKind k);

// The world sliders (spec 31 State). They lock at creation.
struct WorldParams {
  double land_share = 0.29;      // area share above sea level, exact to one cell
  double global_temp_c = 14.0;   // area-weighted mean sea-level temperature
  double relief = 1.0;           // scales elevation spread
  double moisture = 1.0;         // scales rain
  int plate_count = 0;           // 0 = by planet size (12 at Earth size)
  std::uint64_t seed = 1;
};

struct WorldFields {
  std::vector<std::uint16_t> plate;
  std::vector<double> elevation_m;   // above sea level; negative under water
  std::vector<double> temp_mean_c;   // annual mean at the cell's elevation
  std::vector<double> temp_range_c;  // warmest minus coldest month
  std::vector<double> rain_mm;       // per year
  std::vector<Biome> biome;
  // Cause record (spec 31 rule 9).
  std::vector<BoundaryKind> boundary_kind;
  std::vector<std::uint32_t> boundary_cells;  // graph distance to the nearest plate boundary
  std::vector<std::uint32_t> sea_cells;       // graph distance to the nearest sea cell (0 at sea)
  std::vector<double> west_km, east_km;       // land: km to the sea walking west and east along the parallel (spec 83 rule 1)
  int plate_count = 0;
  double sea_level_raw_m = 0;                 // the pre-shift elevation that became zero

  bool land(CellId i) const { return elevation_m[i] > 0; }
};

WorldFields generate_fields(const WorldGrid& grid, const WorldParams& params);

// The biome table (spec 31 rule 8, by aridity since spec 83 rule 3): a function of its inputs only.
Biome classify_biome(bool land, double elevation_m, double temp_mean_c, double rain_mm);

// Potential evapotranspiration in mm a year from the annual mean temperature (Holdridge: 58.93 x biotemperature,
// the mean clamped to 0-30 C standing in for it; spec 83 rule 3).
double pet_mm(double temp_mean_c);

// Where a land cell sits between its continent's west (0) and east (1) coasts (spec 83 rule 1).
inline double east_index(const WorldFields& f, CellId i) {
  const double w = f.west_km[i], e = f.east_km[i];
  return w + e > 0 ? w / (w + e) : 0.5;
}

// Area-weighted share of cells for which `pred(cell)` holds.
template <class Pred>
double area_share(const WorldGrid& g, Pred pred) {
  double in = 0;
  for (CellId i = 0; i < g.size(); ++i)
    if (pred(i)) in += g.area_km2(i);
  return in / g.sphere_area_km2();
}

}  // namespace zs::gen
