// People, sectors, countries and towns on the world grid (spec 48): where today's population lives on a generated
// planet, the sectors it is divided into, the countries those form with an income group each, and towns by rank-size.
// A pure function of (grid, fields, seed, the global tables).
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

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

struct Sector {
  CellId seed = 0;              // its most populous cell when seeded (rule 3)
  std::vector<CellId> cells;    // land cells, ascending
  double people = 0, area_km2 = 0;
  int country = -1;
};

struct Country {
  int capital = 0;              // sector index (rule 4)
  std::vector<int> sectors;
  double people = 0, development = 0;
  int income = 0;               // index into kIncomeGroupIds (rule 5)
};

struct Settlement {
  CellId cell = 0;
  int country = 0, rank = 0;    // rank 1 is the country's largest (its capital city)
  double people = 0;
};

struct WorldPeople {
  double land_km2 = 0, total_people = 0;
  std::vector<double> suitability, people;   // per cell (rules 1, 2)
  std::vector<int> sector_of;                // per cell; -1 at sea
  std::vector<Sector> sectors;
  std::vector<Country> countries;
  std::vector<Settlement> towns;
};

inline constexpr double kEarthLandKm2 = 148.9e6;
inline constexpr double kSectorTargetKm2 = 3.0e6;
inline constexpr double kRankSizeZeta = 1.11;    // Soo (2005), mean over 73 countries
inline constexpr double kTownMinPeople = 10'000;

double suitability(const WorldFields& f, CellId c);
WorldPeople populate_world(const WorldGrid& grid, const WorldFields& fields, std::uint64_t seed, const GroupTables& tables);

}  // namespace zs::gen
