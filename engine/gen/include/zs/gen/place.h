// The block's place on a generated planet, and the one world built there. Spec: docs/tier0/specs/42-place.md
#pragma once
#include <cstdint>

#include "zs/gen/world_fields.h"
#include "zs/gen/world_people.h"
#include "zs/place.h"
#include "zs/tier0.h"

namespace zs::gen {

// Rule 2: the planet the one world is generated on.
inline constexpr int kOneWorldFrequency = 24;

// Rule 1: a temperate land cell whose annual mean is nearest 12 C; ties by a keyed draw.
CellId choose_place(const WorldGrid& grid, const WorldFields& fields, std::uint64_t seed);
// The simulation's record of a cell: position, elevation, biome and climate.
zs::Place place_of(const WorldGrid& grid, const WorldFields& fields, CellId cell, std::uint64_t seed);

// Spec 48 rule 7: the town for the block: 10,000 to 200,000 people in a temperate cell whose mean is nearest 12 C
// (ties by a keyed draw); -1 if the world has none.
int choose_town(const WorldFields& fields, const WorldPeople& people, std::uint64_t seed);
// The place of that town: its cell's record plus the town, sector, country and income group.
zs::Place place_of_town(const WorldGrid& grid, const WorldFields& fields, const WorldPeople& people, int town, std::uint64_t seed);

// The one world (decision 0052, specs 41, 42, 48): generates the planet and its people from `seed`, chooses the block's
// town and builds the run there, with that town's country. What zs.exe opens and plays.
zs::tier0::Tier0 make_world(std::uint64_t seed);

}  // namespace zs::gen
