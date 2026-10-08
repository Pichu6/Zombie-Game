// The block's place on a generated planet, and the one world built there. Spec: docs/tier0/specs/42-place.md
#pragma once
#include <cstdint>

#include "zs/gen/world_fields.h"
#include "zs/place.h"
#include "zs/tier0.h"

namespace zs::gen {

// Rule 2: the planet the one world is generated on.
inline constexpr int kOneWorldFrequency = 24;

// Rule 1: a temperate land cell whose annual mean is nearest 12 C; ties by a keyed draw.
CellId choose_place(const WorldGrid& grid, const WorldFields& fields, std::uint64_t seed);
// The simulation's record of a cell: position, elevation, biome and climate.
zs::Place place_of(const WorldGrid& grid, const WorldFields& fields, CellId cell, std::uint64_t seed);

// The one world (decision 0052, specs 41 and 42): generates the planet from `seed`, chooses the block's cell and
// builds the run there. What zs.exe opens and plays.
zs::tier0::Tier0 make_world(std::uint64_t seed);

}  // namespace zs::gen
