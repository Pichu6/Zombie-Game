// Spec 52: what the land holds. Slice 1: runoff, rivers, lakes, fresh water, soil fertility, arable land and the
// people a cell can feed and water. A pure function of (grid, fields, params); saved with the world like the fields.
#pragma once
#include <cstdint>
#include <limits>
#include <vector>

#include "zs/gen/world_fields.h"

namespace zs::gen {

inline constexpr CellId kNoCell = std::numeric_limits<CellId>::max();

enum : std::uint8_t { kLakeNone = 0, kLakeOpen = 1, kLakeTerminal = 2, kLakeDry = 3 };

// The start's technology and diet (rule 7; estimates, WNC-13, WNC-25, WNC-26).
struct LandParams {
  double fu_omega = 2.6;               // RV-3
  double pet_per_c = 40, pet_base_mm = 200, pet_min_mm = 100;  // RV-5: PET = max(min, base + per_c * T), calibrated to RV-1
  double yield_t_per_ha = 4.0;         // cereal-equivalent yield at the start
  double kcal_per_t = 3.4e6;
  double feed_share = 0.36;            // share of crop calories fed to animals
  double kcal_per_person_day = 2500;
  double pasture_vs_crop = 0.1;        // pasture calories per ha against cropland's (WNC-25)
  double water_m3_per_person = 500;    // absolute scarcity (WNC-17)
  double river_m3s = 100, large_river_m3s = 1000;            // RV-9, on the water from upstream
  double river_km2 = 100000, large_river_km2 = 500000;       // RV-9: catchment, so the classes do not depend on cell size
  // Slice 2 (rules 10-12; estimates).
  double game_r = 0.25, fish_r = 0.5;
  double fish_t_per_km2 = 0.75, fish_shelf_m = -1500;
  // Spec 84: a world's richness settings (1 on Earth).
  double fertility_scale = 1.0, forest_scale = 1.0, fish_scale = 1.0;
};

struct WorldLand {
  std::vector<double> runoff_mm;        // per year; 0 at sea
  std::vector<CellId> downstream;       // kNoCell at sea
  std::vector<double> discharge_m3s;    // own runoff plus everything upstream
  std::vector<double> catchment_km2;    // the land that drains through this cell, itself included
  std::vector<std::uint8_t> river;      // 0 none, 1 river, 2 large river
  std::vector<std::uint8_t> lake;       // spec 83 rule 4: 0 none, 1 open lake, 2 terminal (salt) lake, 3 dry basin
  std::vector<std::uint8_t> endorheic;  // 1 where the water ends in a closed depression, not the sea
  std::vector<double> fertility, arable_share, pasture_share;
  std::vector<double> capacity_food, capacity_water;  // people
  // Slice 2: living resources, today's stocks (rules 10-12). On land: forest and game; at sea: fish.
  std::vector<double> forest_share, wood_m3, wood_r;  // growing stock in m3; regrowth per year
  std::vector<double> game_kg;                         // wild herbivore wet biomass
  std::vector<double> fish_catch_t, fish_k_t;          // sustainable catch per year, and the stock's capacity
  double capacity(CellId i) const { return capacity_food[i] < capacity_water[i] ? capacity_food[i] : capacity_water[i]; }
  // Totals for the checks.
  double land_rain_km3 = 0, runoff_km3 = 0, sea_inflow_m3s = 0;
  double closed_evap_m3s = 0;  // what closed depressions evaporate (spec 83 rule 4)
  int open_lakes = 0, closed_basins = 0;
};

WorldLand generate_land(const WorldGrid& grid, const WorldFields& fields, const LandParams& params = {});

}  // namespace zs::gen
