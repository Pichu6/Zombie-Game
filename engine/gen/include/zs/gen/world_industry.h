// Spec 93: processing plants between extraction and the towns: refineries, steel works, smelters, cement, mills, meat
// and fish plants, sawmills, each at a town where Weber's material index puts it (research processing-plants, PRC).
// A pure function of the world above it. docs/tier0/specs/93-processing-plants.md.
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "zs/gen/world_land_use.h"

namespace zs::gen {

enum class PlantRule : std::uint8_t { Materials, Market, Port, Power };

// One kind of plant (data/gen/processing.toml).
struct PlantKind {
  std::string id, label, output, size;
  PlantRule rule = PlantRule::Market;
  std::vector<std::string> inputs;  // oil, iron, coal, bauxite, copper, crop, pasture, forest, fish, people
  double earth_plants = 0;          // at Earth's 8 billion people
  std::array<double, 4> income_weight{1, 1, 1, 1};
};
std::vector<PlantKind> load_plant_kinds(const std::string& gen_dir);

struct Plant {
  int kind = 0;       // index into WorldIndustry::kinds
  int town = 0;       // index into WorldPeople::towns
  CellId cell = 0;
  int country = -1;
  std::vector<int> feeds;  // rule 4: the worked sites nearest it that supply its input (WorldLandUse::sites), up to three
};

struct WorldIndustry {
  std::vector<PlantKind> kinds;
  std::vector<Plant> plants;                 // by kind, then by town
  std::vector<std::vector<int>> of_town;     // per town: its plants
};

WorldIndustry place_plants(const WorldGrid& grid, const WorldFields& fields, const WorldLand& land, const WorldDeposits& deposits,
                           const WorldPeople& people, const WorldLandUse& use, const std::vector<PlantKind>& kinds);

}  // namespace zs::gen
