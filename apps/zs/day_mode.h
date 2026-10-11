// zs --day (spec 54): a town's day hour by hour; and the schedule person of a generated pop, for the Atlas's pop card.
#pragma once
#include <cstdint>

#include "zs/gen/pops.h"
#include "zs/schedule.h"

namespace zs::atlas {

SchedulePerson schedule_person(const gen::Pop& p, std::uint64_t world_seed);
int day_main(int argc, char** argv);

}  // namespace zs::atlas
