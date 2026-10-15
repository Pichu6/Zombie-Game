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
  int periods = 85;             // rule 23: in frequency-24 periods; calibrated on the one world's grid, seeds 1-3 (see the spec)
  double beta0 = 3e-4;          // rule 23: secession per border cell at fragmentation 0.5 (calibrated, see the spec)
  double state_density = 25;    // rule 21: people of capacity per km2 to start as a polity
  double sea_weight = 0.1, sea_sigma = 0.33;  // rule 22 (CF-26)
  double fill_limit = 5000;     // rule 24: km-equivalent (about five temperate steps of a frequency-24 cell)
  double people_scale = 1.0;    // spec 84: people per unit of land against today's (spec 48 rule 2)
  double reach_km = 2000;       // spec 87: a polity's strength at a cell falls by exp(-distance from its capital / reach);
                                // calibrated on seeds 1-5 (the most populous country 15 %, PTP-7 18 %)
  // Spec 91: N founding states, then a recorded history. 0 or less: the contest of rule 21 from every polity, as before
  // (populate_world reads 0 as the pack's size).
  int founders = 0;
  std::uint64_t history_seed = 0;   // 0: drawn from the planet seed
  int history_periods = 20;         // in frequency-24 periods, scaled like rule 23
  double history = 1.0;             // the setting: scales divisions and unions (0: none)
  double division_rate = 0.08;      // expected divisions per country over the history at history 1 (estimate, calibrated)
  double union_rate = 0.6;          // expected unions per country over the history at history 1 (estimate, calibrated)
  double alliance_share = 0.5;      // the share of countries in an alliance (estimate: NATO, the EU, the AU, ASEAN...)
  double founder_power_sigma = 0.65;  // the spread of founders' power (log scale; calibrated on country sizes)
  double founder_core_power = 0.3;    // its rise with the founder's core (calibrated on the largest country's people)
};

enum class HistoryKind : std::uint8_t { Annex, Secede, Split, Fill, Found, Unite, Divide };
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
  int entity = -1;              // spec 91: the entity it is (a founder, a union, a division); -1 without founders
};

// Spec 91: every state the history made: the founders first (0..N-1), then unions and divisions in order.
enum class Origin : std::uint8_t { Founder, Union, Division };
struct Entity {
  Origin origin = Origin::Founder;
  int a = -1, b = -1;   // parents (entity ids): a union's two, a division's one
  int period = 0;       // the history period it was born in (0 for founders)
  CellId capital = 0;
};
// An alliance groups countries without merging them (0082 rule 4).
struct Alliance {
  std::vector<int> members;  // country indices
  int leader = -1;           // the member it formed around
};

struct WorldCountries {
  std::vector<double> core, terrain;  // y and Theta*x per cell (rules 19, 20)
  std::vector<int> country_of;        // per cell; -1 at sea
  std::vector<CountryShape> countries;
  std::vector<HistoryEvent> events;
  int start_polities = 0, periods = 0;
  // Spec 91 (empty without founders).
  std::vector<Entity> entities;
  std::vector<int> founded_of;  // per cell: its founder at the end of the founding (-1 at sea and ice)
  std::vector<Alliance> alliances;
};

WorldCountries form_countries(const WorldGrid& grid, const WorldFields& fields, const WorldLand& land,
                              const LandGraph& graph, std::uint64_t seed, const CountryParams& params = {});

}  // namespace zs::gen
