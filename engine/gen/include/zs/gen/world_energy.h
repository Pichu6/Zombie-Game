// Spec 94: power plants and the grid. Each country's electricity demand by its people and income (research electricity-
// supply, ELS-2), its mix from what it has (rivers with head, coal and gas it works, sun, wind; nuclear by income),
// calibrated so the world's mix is Ember's 2023 (ELS-1); the plants that make it and the high-voltage lines joining
// them to the towns. A pure function of the world above it. docs/tier0/specs/94-power-plants-and-the-grid.md.
#pragma once
#include <array>
#include <cstdint>
#include <vector>

#include "zs/gen/world_land_use.h"

namespace zs::gen {

enum class Fuel : std::uint8_t { Coal, Gas, Oil, Hydro, Nuclear, Wind, Solar };
inline constexpr int kFuels = 7;
const char* fuel_name(Fuel f);

struct EnergyParams {
  std::array<double, 4> kwh_per_person{8744, 2986, 702, 180};  // ELS-2, by income group (high first)
  double losses = 0.08;                                          // grid losses (estimate)
  double growth = 1.12;  // 2015's use to 2023's generation: Ember's 29,925 TWh over 8.0 billion x 3,083 kWh x 1.08 (derived)
  // ELS-1, Ember 2023 shares of generation: coal, gas, oil and other (bioenergy, oil), hydro, nuclear, wind, solar.
  std::array<double, kFuels> world_share{0.349, 0.222, 0.064, 0.140, 0.091, 0.079, 0.055};
  std::array<double, kFuels> capacity_factor{0.5, 0.45, 0.15, 0.4, 0.8, 0.25, 0.15};  // ELS-7, estimates
  std::array<double, kFuels> unit_mw{1000, 500, 200, 0, 1800, 100, 50};               // ELS-7 (hydro: its site)
  double dam_min_mw = 30;
  std::array<double, 4> nuclear_by_income{0.18, 0.06, 0.02, 0.0};  // starting shares of demand, then calibrated
  std::array<double, 4> renewables_by_income{1.5, 1.2, 0.6, 0.3};   // wind and solar uptake by income (estimate)
  int rounds = 12;                                                    // calibration rounds (rule 4)
};

struct PowerPlant {
  Fuel fuel = Fuel::Coal;
  CellId cell = 0;
  int town = -1;      // the town it stands by; -1 for a dam on its river
  int country = -1;
  double mw = 0;
};

struct GridLine {
  CellId a = 0, b = 0;
  int country = -1;   // -1 for a line between two countries
  double km = 0;
};

struct WorldEnergy {
  std::vector<double> demand_twh;                    // per country, with losses
  std::vector<std::array<double, kFuels>> gen_twh;   // per country, by fuel
  std::array<double, kFuels> world_twh{};
  std::vector<PowerPlant> plants;
  std::vector<GridLine> lines;
  double hydro_potential_mw = 0;                     // all dam sites of the world
};

WorldEnergy make_energy(const WorldGrid& grid, const WorldFields& fields, const WorldLand& land, const WorldDeposits& deposits,
                        const WorldPeople& people, const WorldLandUse& use, const EnergyParams& params = {});

}  // namespace zs::gen
