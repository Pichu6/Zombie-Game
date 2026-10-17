// Spec 103: the living stocks of a town's region from the generated world (rules 1, 5, 6), and the world's checks
// against Earth (rule 8). docs/tier0/specs/103-living-stocks-in-the-region.md.
#pragma once
#include <cstdint>
#include <vector>

#include "zs/gen/place.h"
#include "zs/living_stocks.h"

namespace zs::gen {

// Rule 5 (LS-1, LS-3, LS-4, LS-6; the hunting figures are estimates).
struct RegionStockParams {
  double hunt_share = 0.1, hunt_km = 100;  // game taken a year at the town, falling as exp(-km / hunt_km)
  double log_of_rate = 0.5;                // a worked forest is cut at this share of its growth rate
  double overfished = 0.377;               // the keyed share of fished sea cells on the overfished side
  double graze_share = 0.25;               // livestock's share of the year's peak
  double grass_g_per_mm = 0.6, grass_less_g = 34, grass_rain_cap_mm = 2000;  // LS-7, g/m2 a year
};

// Rule 1: the town's region (land cells marketed to it, its own cell, sea cells fished from it).
std::vector<CellId> region_cells(const GeneratedWorld& world, int town);

// One cell's stocks of every kind it supports, calibrated by rule 6 (game and wood to spec 52's stock of today).
std::vector<LivingStock> cell_stocks(const GeneratedWorld& world, CellId cell, std::uint64_t seed, const RegionStockParams& p = {});

// The region's stocks for the run.
LivingSetup region_stocks(const GeneratedWorld& world, int town, std::uint64_t seed, const RegionStockParams& p = {});

// Rule 8: the world's checks at equilibrium, every cell's stocks (not only a region's).
struct LivingWorldChecks {
  double wood_stock_m3 = 0, wood_removed_m3 = 0;  // a year
  int fish_cells = 0, fish_overfished = 0;
  double grass_peak_kg = 0, grass_eaten_kg = 0;  // a year, on the pasture in use
};
LivingWorldChecks living_world_checks(const GeneratedWorld& world, std::uint64_t seed, const RegionStockParams& p = {});

}  // namespace zs::gen
