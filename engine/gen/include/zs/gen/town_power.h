// Spec 96: the town's power from its country's grid (spec 94), and the outage from its causes: the plants stopping when
// their staff leave, the grid island falling below its load, a fault on the town's line that no one repairs.
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "zs/gen/world_energy.h"
#include "zs/gen/world_grid.h"
#include "zs/gen/world_people.h"

namespace zs::gen {

// Rules 3-4 (research grid-without-crews, GWC).
struct OutageParams {
  double staff_day_min = 1, staff_day_max = 4;  // stand-in until the outbreak reaches the plants (spec 85): the day staff leave
  double fossil_h_min = 2, fossil_h_max = 24;   // GWC-2: a fossil plant without operators stops within hours
  double nuclear_d_min = 3, nuclear_d_max = 7;  // GWC-3: a reactor runs unattended for days to a week
  double hydro_d_min = 14, hydro_d_max = 60;    // GWC-4: a dam runs unattended for weeks
  double load_after = 0.35;                     // GWC-5: the island's load once work stops, a share of its mean demand
  double line_faults_per_100km_year = 2.0;      // GWC-6: forced outages of overhead lines
  double feeder_faults_per_year = 1.5;          // GWC-7: faults on a town's own distribution feeder
};

// Rules 1-2: what feeds the town.
struct TownFeed {
  int town = -1;
  bool on_grid = false;                 // its country has high-voltage lines (spec 94 rule 6)
  CellId node = 0;                      // the grid node it hangs from (its own cell when it is one)
  double feeder_km = 0;                 // from the town to that node
  double line_km = 0;                   // along the lines from the node to the nearest large plant
  std::vector<int> countries;           // the countries the island joins
  std::vector<int> plants;              // indices into WorldEnergy::plants on the island (and the town's own)
  std::array<double, kFuels> mw{};      // installed on the island, by fuel
  double load_mw = 0;                   // the island's mean demand
};
TownFeed town_feed(const WorldGrid& grid, const WorldPeople& people, const WorldEnergy& energy, int town);

enum class OutageCause { Generation, Line, Feeder };
const char* outage_cause_name(OutageCause c);

// Rules 3-5: hours from the run's start.
struct OutageTimeline {
  double staff_leave_h = 0;
  double fossil_last_h = -1, nuclear_last_h = -1, hydro_last_h = -1;  // the last plant of each kind stops (-1: none)
  double island_dark_h = 0;      // the island's running plants fall below its load
  double line_fault_h = 0;       // the first fault on the lines to the nearest large plant
  double feeder_fault_h = 0;     // the first fault on the town's feeder
  double dark_h = 0;             // the earliest of the three
  OutageCause cause = OutageCause::Generation;
};
OutageTimeline outage_timeline(const TownFeed& feed, const WorldEnergy& energy, const EnergyParams& ep, std::uint64_t seed,
                               const OutageParams& p = {});

// A line for the town card and `zs gen power`: the island's mix, the town's line and the outage's day and cause.
std::string describe_feed(const TownFeed& feed, const OutageTimeline& t);

}  // namespace zs::gen
