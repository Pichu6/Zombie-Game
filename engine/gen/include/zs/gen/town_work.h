// Spec 99: the pops in the town's workplaces. Each adult of working age draws a job from the town's jobs (spec 98) and
// a workplace of that job's kind by its staff; each child of school age (the country's, `school`) the nearest school of
// its stage; the commute is
// the town's streets from the pop's home plot (spec 75). docs/tier0/specs/99-pops-in-the-workplaces.md.
#pragma once
#include <cstdint>
#include <vector>

#include "zs/gen/catalog.h"
#include "zs/gen/school_ages.h"
#include "zs/gen/town_economy.h"
#include "zs/gen/town_travel.h"
#include "zs/pop.h"
#include "zs/work.h"

namespace zs::gen {

struct WorkRow {
  zs::PopId pop = 0;
  int age = 0;
  bool alive = true;
};

zs::WorkTable place_in_work(const TownEconomy& economy, const Catalog& cat, const TownLayout& layout, const TownTravel& travel,
                            const std::vector<WorkRow>& rows, std::size_t census_size, std::uint64_t seed, const SchoolAges& school = {});

}  // namespace zs::gen
