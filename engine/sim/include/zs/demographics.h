// A nation's demographic profile: who lives there, in numbers. Spec: docs/tier0/specs/28-pop-levels.md and
// docs/design/sandbox-parameters.md section 5. It is data (data/setup/nations/<id>.toml), so a country can differ
// from every other the way real countries do: age structure, education, work, households, where people live.
// Every table carries its source. A profile is "placeholder" (a plausible shape, not data) or "sourced", and the
// loader refuses "sourced" without a citation for every table: we never present invented numbers as real.
#pragma once
#include <array>
#include <string>
#include <vector>

#include "zs/pop_params.h"

namespace zs {

// Fixed orders (never reorder: the profile files and saves rely on them).
inline constexpr int kEducationLevels = 4;   // none, primary, secondary, tertiary
inline constexpr int kEducationAgeGroups = 3;  // 25-34, 35-64, 65 and over
inline constexpr int kOccupationGroups = 10;   // ISCO-08 major groups: armed forces, managers, professionals, technicians,
                                               // clerks, services and sales, skilled agriculture, craft trades, plant and
                                               // machine operators, elementary occupations
inline constexpr int kSectors = 3;             // agriculture, industry, services
inline constexpr int kHouseholdSizes = 4;      // 1, 2-3, 4-5, 6 or more people (the UN household database's groups, spec 45)
inline constexpr int kLabourAgeGroups = 4;     // 15-24, 25-54, 55-64, 65 and over

struct DataSource {
  std::string text;  // who published it (an organisation, a table)
  int year = 0;      // the data year
};

struct NationDemographics {
  std::string id, name;
  bool sourced = false;  // false: a placeholder shape
  std::string pop_params;  // a demography preset under data/setup/ for mortality and fertility ("" = the defaults)

  std::vector<double> age_pyramid;                                     // shares by five-year band, 20 bands, add to 1
  DataSource age_pyramid_source;
  std::array<std::array<double, kEducationLevels>, kEducationAgeGroups> education{};  // attainment shares by age group
  DataSource education_source;
  std::array<std::array<double, 2>, kLabourAgeGroups> participation{};  // share in the labour force: [age group][men, women]
  double unemployment_rate = 0;   // of the labour force
  double retirement_age = 65;
  DataSource labour_source;
  std::array<double, kOccupationGroups> occupations{};  // shares of the employed
  DataSource occupation_source;
  std::array<double, kSectors> sectors{};               // shares of the employed
  DataSource sector_source;
  std::array<double, kHouseholdSizes> household_sizes{};  // shares of households
  double household_mean_size = 0;  // members per household as published (0 = not given: estimated from the groups)
  DataSource household_source;
  double urban_share = 0;  // share of people living in towns and cities
  DataSource settlement_source;
};

// Loads and checks a profile. Throws with a plain message on a missing table, an unknown key, shares that do not
// add up to 1 (within 1%; they are then normalised), a rate outside 0..1, or "sourced" with a missing citation.
NationDemographics load_nation(const std::string& path);

// The pop parameters of a world in this nation: the profile's mortality and fertility preset (or the defaults)
// with its age pyramid, so founders drawn by rule have the nation's age structure. `data_dir` is the data root.
PopParams make_pop_params(const NationDemographics& nation, const std::string& data_dir);

// Readouts the generators and the realism report use.
double mean_household_size(const NationDemographics& nation);
double share_under(const NationDemographics& nation, double age);   // share of the population younger than `age`
const char* occupation_group_name(int group);
const char* sector_name(int sector);
const char* education_level_name(int level);

}  // namespace zs
