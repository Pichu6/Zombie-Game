// People, sectors, countries and towns on the world grid (spec 48): where today's population lives on a generated
// planet, the countries the land forms (spec 52 slice 4), the sectors cut inside them, an income group each from
// what the land holds, and towns by rank-size. A pure function of (grid, fields, land, deposits, seed, the tables).
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "zs/gen/world_countries.h"
#include "zs/gen/world_deposits.h"
#include "zs/gen/world_fields.h"

namespace zs::gen {

inline constexpr int kIncomeGroups = 4;
inline constexpr std::array<const char*, kIncomeGroups> kIncomeGroupIds = {"high", "upper_middle", "lower_middle", "low"};

// The parts of data/setup/world/wpp2024.toml this step reads (specs 43 and 46).
struct GroupTables {
  double world_population = 0;                         // Earth, 2023
  std::array<double, kIncomeGroups> population{};      // by income group, 2023
  std::array<double, kIncomeGroups> urban_share{};
};
GroupTables load_group_tables(const std::string& data_dir);

// Spec 49: a real country's profile (data/setup/nations/real/<iso>.toml): its parody name, income group and people.
struct RealProfile {
  std::string iso, name;
  std::string color;  // spec 92: a custom country's map colour ("#rrggbb"); empty: keyed
  int income = 0;
  double population = 0;
};
std::vector<RealProfile> load_real_profiles(const std::string& data_dir);  // sorted by iso
// Spec 90: a user pack's profiles: the sourced countries its pack.toml includes (by ISO; "all" for every one) and its
// custom countries (<id>.toml with custom = true; iso "custom/<id>"). Sorted by iso. Empty `pack_dir`: the sourced ones.
std::vector<RealProfile> load_pack_profiles(const std::string& data_dir, const std::string& pack_dir);

struct Sector {
  CellId seed = 0;              // its most populous cell when seeded (rule 3)
  std::vector<CellId> cells;    // land cells, ascending
  double people = 0, area_km2 = 0;
  int country = -1;
};

struct Country {
  int capital = 0;              // sector index: the sector holding capital_cell
  CellId capital_cell = 0;      // spec 52 rule 22
  std::vector<int> sectors;
  double people = 0, area_km2 = 0, development = 0;
  double endowment = 0;         // spec 52 rule 26, [0, 1]
  bool landlocked = false;
  int income = 0;               // index into kIncomeGroupIds (rule 5)
  std::string profile, name;    // spec 49: the real profile it takes (ISO3) and its parody name; empty: the base country
  double zeta = 0;              // spec 78 rule 3: its towns' rank-size exponent (0: no towns)
  std::string origin;           // spec 91: "founded", "union of A and B in period P", "split from A in period P"
  std::string color;            // spec 92: its map colour ("#rrggbb"; empty: keyed by the map)
  int alliance = -1;            // spec 91: index into WorldPeople::alliance_names, -1 none
};

// Spec 86 rule 2: why a town stands where it does.
enum class TownSite : std::uint8_t { Inland, Port, Mouth, Confluence, River, Resource };
inline constexpr int kTownSites = 6;
const char* town_site_name(TownSite s);

struct Settlement {
  CellId cell = 0;
  int country = 0, rank = 0;    // rank 1 is the country's largest (its capital city)
  double people = 0;
  TownSite site = TownSite::Inland;  // spec 86
};

// Spec 86 rules 1-3, recalibrated on seeds 1-5 against Earth measured at this cell size (research earth-measured-open-data,
// EM-1 to EM-8; see the spec's amendment); resource_town and lake_w are estimates.
struct PlacementParams {
  double food_exp = 0.4;
  double river_w = 1.3, large_river_w = 2.5, lake_w = 1.5, arid_ai = 0.5;  // recalibrated with spec 89 and its second amendment
  double coast_w = 3.0, coast_km = 25;
  double height_m = 2200;
  double lowland_w = 1.8;      // extra weight on the lowest land, fading over 100 m (EM-2: 44 % of people below 100 m)
  double concentration = 1.8;  // climate and food's term is raised to this power (EM-7: half of people on 3 % of land cells)
  double port_w = 0.0, mouth_w = 0.8, river_w2 = 2.0, confluence_w = 2.0;
  double resource_rel = 3.0, resource_town = 20000;
};

struct WorldPeople {
  double land_km2 = 0, total_people = 0;
  std::vector<double> suitability, people;   // per cell (rules 1, 2)
  std::vector<int> sector_of;                // per cell; -1 at sea
  std::vector<Sector> sectors;
  std::vector<Country> countries;
  std::vector<Settlement> towns;
  WorldCountries formed;                     // spec 52 slice 4: the history and the shapes, country i = countries[i]
  // Spec 91: every entity's name (founders, unions, divisions; by entity id) and each alliance's name.
  std::vector<std::string> entity_names, alliance_names;
};
// Spec 91 rule 4: parody names for states the history makes, keyed by `key`.
std::string union_name(const std::string& a, const std::string& b, std::uint64_t key);
std::string division_name(const std::string& parent, const char* direction, std::uint64_t key);
std::string alliance_name(const std::string& leader, const std::string& second, const char* direction, std::uint64_t key);

inline constexpr double kEarthLandKm2 = 148.9e6;
inline constexpr double kSectorTargetKm2 = 3.0e6;
inline constexpr double kRankSizeZeta = 1.11;    // Soo (2005), mean over 73 countries
inline constexpr double kTownMinPeople = 10'000;
// Spec 78 rule 1: a country may be up to this times its income group's largest real profile (estimate).
inline constexpr double kGroupSizeTolerance = 1.25;
// Spec 78 rule 2: the largest town's share of urban people: the median min(cap, a x urban^-b) (b and a fitted by Claude to
// DR-4's twelve large countries; the cap is DR-4's median over all countries), times exp(sd x z) for a country's keyed
// unit-normal z (sd from DR-4's middle half, 20-40 %), clamped to [min, 1].
inline constexpr double kPrimacyA = 21143.0, kPrimacyB = 0.641, kPrimacyCap = 0.30, kPrimacySd = 0.52, kPrimacyMin = 0.02;
// Spec 78 rule 3: zeta's real range (DR-12).
inline constexpr double kZetaMin = 0.81, kZetaMax = 1.96;
// Spec 78 rule 2: no town larger than the world's largest agglomerations (DR-2: Jakarta about 42 million, Tokyo 37).
inline constexpr double kTownMaxPeople = 45e6;
double primacy_median(double urban_people);
double primacy_share(double urban_people, double z);

double suitability(const WorldFields& f, CellId c);  // spec 86: climate only (temperature and rain)
// Spec 86 rule 4: where the people and the towns are, against PTP's real shares.
struct PlacementReport {
  double coast_100 = 0, coast_200 = 0, coast_400 = 0;  // share of people within that many km of the sea
  double below_100m = 0, below_500m = 0, median_m = 0;
  double water_people = 0, water_land = 0;            // share of people, and of land, in river or open-lake-shore cells
  // Spec 86 recalibration (EM): river cells (spec 52 rule 4) and large-river cells apart from lakes; below 200 m;
  // concentration over land cells.
  double river_people = 0, river_land = 0, large_people = 0, large_land = 0, below_200m = 0;
  double half_cells = 0, cells_80 = 0, empty_cells = 0;  // shares of land cells: holding half / 80 % of people; under 1 a km2
  double median_cell_density = 0, median_person_density = 0;  // people a km2
  double largest_country = 0, two_largest = 0, ten_largest = 0;  // shares of the world's people (spec 87)
  int million_towns = 0;
  double million_coastal = 0;                           // share of towns of a million or more within 100 km of the sea
  std::array<int, kTownSites> sites{};
};
PlacementReport placement_report(const WorldGrid& g, const WorldFields& f, const WorldLand& land, const WorldPeople& people);

// Spec 86 rule 1: a land cell's weight for people (area times climate, food, fresh water, coast and height).
double people_weight(const WorldGrid& g, const WorldFields& f, const WorldLand& land, CellId c, double food_mean_per_km2,
                     const PlacementParams& p = {});
// With `profiles`, each country takes one (spec 49).
WorldPeople populate_world(const WorldGrid& grid, const WorldFields& fields, const WorldLand& land, const WorldDeposits& deposits,
                           std::uint64_t seed, const GroupTables& tables, const std::vector<RealProfile>& profiles = {},
                           const CountryParams& country_params = {});

}  // namespace zs::gen
