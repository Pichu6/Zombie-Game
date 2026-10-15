// Spec 52 slice 3: deposits from the geology. Coal, oil, gas, iron ore, copper ore and bauxite, placed by the
// geological setting of each cell (plates, climate, height), sized by lognormal or Pareto draws, each with its reason.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "zs/gen/world_fields.h"

namespace zs::gen {

enum class Resource : std::uint8_t { Coal, Oil, Gas, IronOre, CopperOre, Bauxite };
inline constexpr int resource_count = 6;
const char* resource_name(Resource r);
const char* resource_unit(Resource r);  // t, bbl or m3

// Rule 14: the settings a cell can have (bit flags).
enum Setting : std::uint8_t { kArc = 1, kForeland = 2, kRift = 4, kCraton = 8, kBasin = 16, kLaterite = 32 };
std::string setting_names(std::uint8_t settings);

struct DepositParams {
  // Rule 16: the median size per frequency-24 cell's area, in each resource's unit, calibrated on the seed-1 planet
  // of the one world's grid (frequency 101, 0063) so the totals land between today's reserves and resources (coal,
  // oil, gas, iron ore, copper ore, bauxite).
  double median[resource_count] = {1.34e10, 2.9e9, 2.5e11, 1.05e10, 3.5e10, 3.5e8};  // iron, copper, bauxite recalibrated for spec 89 (fewer arcs); bauxite again for the wetter tropics (2026-10-15)
  double sigma = 1.0;
  double pareto_cap = 1000;
  std::uint64_t seed = 1;
};

struct Deposit {
  CellId cell = 0;
  Resource kind = Resource::Coal;
  double amount = 0;  // in resource_unit(kind): tonnes of coal or ore, barrels of oil, m3 of gas
  double grade = 0;   // ores: fraction of metal (Cu, Fe) or of Al2O3; 0 for fuels
  std::uint8_t settings = 0;
  std::uint32_t boundary_cells = 0;
};

struct WorldDeposits {
  std::vector<std::uint8_t> settings;  // per cell
  std::vector<Deposit> deposits;       // by cell, then resource
  double total(Resource r) const;
  double metal(Resource r) const;      // amount x grade (ores)
};

std::uint8_t cell_settings(const WorldFields& f, CellId i, double km_per_step);  // km_per_step: the cell spacing
WorldDeposits generate_deposits(const WorldGrid& grid, const WorldFields& fields, const DepositParams& params = {});

}  // namespace zs::gen
