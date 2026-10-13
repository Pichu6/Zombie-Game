// Spec 69: a town's police stations (decision 0077): the agency's staff split over its stations, each station a building
// with a name and an address on the town's streets, its rooms sized from its staff and furnished, written as a site map.
// A pure function of the town, the agency and a seed: the record is the truth, the plan and furniture follow from it.
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "zs/gen/dwellings.h"
#include "zs/police_agency.h"

namespace zs::gen {

struct StationRules {
  double desk_m2 = 6, locker_m2 = 1;
  int min_width = 9, max_width = 24;
  std::map<std::string, double> room_m2;
};
// Reads data/gen/stations.toml from `gen_dir`.
StationRules load_station_rules(const std::string& gen_dir);

struct Station {
  int index = 0;  // 0-based, in the agency's order
  std::string name, street, address;
  int number = 0;
  // This station's share of the agency (rule 6): the first stations take the remainders.
  int sworn = 0, patrol = 0, investigators = 0, civilians = 0, cells = 0;
  int shifts = 3;
  Dwelling plan;  // the building (a corridor, two bands of rooms, a lot), as houses are planned
};

// Rules 6-8: one record per station of the agency, numbered on the town's streets.
std::vector<Station> plan_stations(const Catalog& cat, const Town& town, const AgencyPlan& agency, int shifts,
                                   const StationRules& rules, const DwellingRules& dwelling_rules, std::uint64_t seed);
// The station on its lot with its interior (rule 9), as a site-map TOML; the object `police_station` marks its front door.
std::string station_toml(const Town& town, const Station& s, const DwellingRules& dwelling_rules, std::uint64_t seed);

}  // namespace zs::gen
