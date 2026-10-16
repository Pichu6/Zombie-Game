// Spec 98: the town scaled. Its workforce from the country's people and labour (spec 46's profile), its jobs by the
// country's ILO occupation shares over data/gen/jobs.toml, farm work by the country's agriculture share, the region's
// plants and power stations (specs 93, 94) as its large employers, and its workplaces: as many of each kind as its jobs
// fill (data/gen/town_economy.toml), on plots of their land use. docs/tier0/specs/98-the-town-scaled.md.
#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "zs/demographics.h"
#include "zs/gen/catalog.h"
#include "zs/gen/school_ages.h"
#include "zs/gen/town_layout.h"
#include "zs/gen/world_energy.h"
#include "zs/gen/world_industry.h"
#include "zs/gen/world_people.h"

namespace zs::gen {

struct TownEconomyParams {
  std::map<std::string, double> staff;                 // per business type
  std::map<std::string, double> plant_staff;           // per spec 93 plant kind id
  std::array<double, kFuels> power_staff_per_mw{};     // per spec 94 fuel
  double min_fill = 0.35, resource_staff = 500;
  double farm_factor = 0.5, farm_people = 20000, farm_exp = 0.3, farm_min = 0.1, farm_max = 0.6;
};
TownEconomyParams load_town_economy(const std::string& gen_dir);

// Rule 3: a large employer the region puts in the town.
struct TownPlant {
  std::string label;
  double staff = 0;
};
std::vector<TownPlant> town_plants(const WorldIndustry& industry, const WorldEnergy& energy, const WorldPeople& people, int town,
                                   const TownEconomyParams& params);

// A workplace already planned by another spec (a food outlet, a police station, a filling station): the economy gives it
// its jobs rather than making another.
struct GivenWorkplace {
  std::string name, address;
  int plot = -1;
  double weight = 1;  // its share of the kind's jobs (relative)
};

struct TownWorkplace {
  std::string type;   // a business type, or "plant" for the region's employers
  std::string name, address;
  int plot = -1;      // -1: no plot of its use left (a directory address only)
  double staff = 0;
};

struct TownEconomy {
  double people = 0;
  double labour_force = 0, employed = 0, unemployed = 0;
  std::map<std::string, double> jobs;            // by job id
  std::array<double, kSectors> by_sector{};      // agriculture, industry, services (rule 5)
  double farm = 0, plants = 0, commuting = 0;    // farm work, the plants' staff, jobs held in a larger town nearby
  std::vector<TownWorkplace> workplaces;
};

// `given` lists the workplaces other specs planned, by business type; `taken` plots already used.
TownEconomy plan_town_economy(const Catalog& cat, const zs::NationDemographics& nation, const TownEconomyParams& params, double people,
                              const std::vector<TownPlant>& plants, bool resource_town, const TownLayout& layout,
                              const std::string& town_name, const std::string& culture, std::uint64_t seed,
                              const std::map<std::string, std::vector<GivenWorkplace>>& given = {}, const std::vector<int>& taken = {},
                              const SchoolAges& school = {});

// A line for the town card and `zs gen economy`.
std::string describe_economy(const TownEconomy& e);

}  // namespace zs::gen
