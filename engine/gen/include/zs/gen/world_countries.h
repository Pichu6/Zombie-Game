// Spec 52 slice 4: countries formed from the land. Core values from food, grain, rivers, coast and circumscription;
// a contest history on the grid (the Fractured-Land model: conflict, terrain stalemates, secession, splits); the
// unruled rest filled by reach. A pure function of (grid, fields, land, params, seed).
#pragma once
#include <cstdint>
#include <vector>

#include "zs/gen/world_land.h"

namespace zs::gen {

// Land cells and their links: land neighbours, and crossings of one sea cell (rule 22).
struct LandLink {
  CellId to = 0;
  bool sea = false;
};
struct LandGraph {
  std::vector<std::vector<LandLink>> links;  // per cell; empty at sea
};
LandGraph build_land_graph(const WorldGrid& grid, const WorldFields& fields);

struct CountryParams {
  double fragmentation = 0.5;   // rule 23: [0, 1]
  int periods = 100;            // rule 23: calibrated with beta0 (seed 1: 115 countries, Earth 110)
  double beta0 = 3e-4;          // rule 23: secession per border cell at fragmentation 0.5 (calibrated, see the spec)
  double state_density = 25;    // rule 21: people of capacity per km2 to start as a polity
  double sea_weight = 0.1, sea_sigma = 0.33;  // rule 22 (CF-26)
  double fill_limit = 5000;     // rule 24: km-equivalent (about five temperate steps of a frequency-24 cell)
};

enum class HistoryKind : std::uint8_t { Annex, Secede, Split, Fill };
const char* history_name(HistoryKind k);

struct HistoryEvent {
  int period = 0;               // the fill is period `periods`
  HistoryKind kind = HistoryKind::Annex;
  CellId cell = 0;
  int winner = -1, loser = -1;  // polity ids (not country indices); -1 unruled
};

struct CountryShape {
  std::vector<CellId> cells;    // ascending
  double area_km2 = 0, core = 0;  // core: the sum of y over its cells
  CellId capital = 0;
  int polity = -1;              // the polity id it was in the history
  int founded = 0;              // period the polity appeared (0: at the start)
  int capital_held = 0;         // periods its capital cell was held
  bool coast = false, landlocked = true;
};

struct WorldCountries {
  std::vector<double> core, terrain;  // y and Theta*x per cell (rules 19, 20)
  std::vector<int> country_of;        // per cell; -1 at sea
  std::vector<CountryShape> countries;
  std::vector<HistoryEvent> events;
  int start_polities = 0, periods = 0;
};

WorldCountries form_countries(const WorldGrid& grid, const WorldFields& fields, const WorldLand& land,
                              const LandGraph& graph, std::uint64_t seed, const CountryParams& params = {});

}  // namespace zs::gen
