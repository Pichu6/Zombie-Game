// The block's place on a generated planet, and the one world built there. Spec: docs/tier0/specs/42-place.md
#pragma once
#include <cstdint>

#include "zs/gen/world_fields.h"
#include "zs/gen/world_people.h"
#include "zs/place.h"
#include "zs/tier0.h"

namespace zs::gen {

// Rule 2: the planet the one world is generated on: Earth-size cells of about 5,000 km2, 70 km apart (0038, 0063).
inline constexpr int kOneWorldFrequency = 101;
inline constexpr int kTestFrequency = 24;  // a coarse planet for fast tests of rules that do not depend on the grid

// Rule 1: a temperate land cell whose annual mean is nearest 12 C; ties by a keyed draw.
CellId choose_place(const WorldGrid& grid, const WorldFields& fields, std::uint64_t seed);
// The simulation's record of a cell: position, elevation, biome and climate.
zs::Place place_of(const WorldGrid& grid, const WorldFields& fields, CellId cell, std::uint64_t seed);

// Spec 48 rule 7: the town for the block: 10,000 to 200,000 people in a temperate cell whose mean is nearest 12 C
// (ties by a keyed draw); -1 if the world has none.
int choose_town(const WorldFields& fields, const WorldPeople& people, std::uint64_t seed);
// The place of that town: its cell's record plus the town, sector, country and income group.
zs::Place place_of_town(const WorldGrid& grid, const WorldFields& fields, const WorldPeople& people, int town, std::uint64_t seed);

// A world's settings (spec 55 slice 5, sandbox parameters §1.1: settings change numbers, never rules): the planet's
// sliders (spec 31), the country fragmentation (spec 52) and the start day (spec 53). The defaults are the one world's.
struct WorldSettings {
  double land_share = 0.29;
  double global_temp_c = 14.0;
  double relief = 1.0;
  double moisture = 1.0;
  double fragmentation = 0.5;
  std::int64_t start_day = kDefaultStartDay;
  bool operator==(const WorldSettings&) const = default;
};

// The one world (decision 0052, specs 41, 42, 48): generates the planet and its people from `seed`, chooses the block's
// town and builds the run there, with that town's country. What zs.exe opens and plays.
zs::tier0::Tier0 make_world(std::uint64_t seed, const WorldSettings& settings = {});

}  // namespace zs::gen
