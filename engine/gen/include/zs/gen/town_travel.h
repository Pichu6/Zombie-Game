// Spec 75: travel inside the block's town. Which plot a level of the one world stands on, which plot a pop lives on,
// and the minutes between two plots by the town's streets. Pure functions of the town's record; nothing is saved.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "zs/gen/town_layout.h"
#include "zs/places.h"
#include "zs/pop.h"

namespace zs::gen {

struct TownTravel {
  double walk_kmh = 4.8, drive_kmh = 25, car_overhead_min = 5;  // estimates (data/gen/town_travel.toml)
};
TownTravel load_town_travel(const std::string& gen_dir);

// Rule 1: the plot a level stands on: below 10 the block's lot (the last lot), 10 to 999 station level / 10 - 1, 1,000 and
// up the opened place whose ten levels hold it; -1 when none.
int plot_of_level(const TownLayout& layout, std::size_t stations, const std::vector<zs::Places::Open>& opened, int level);
// Rule 2: a census pop lives on the block's lot; any other pop on a residential plot with households, keyed by its id.
int home_plot(const TownLayout& layout, zs::PopId pop, std::size_t census_size, std::uint64_t seed);
// Rule 3: whole minutes between two plots (at least 1): the faster of walking and driving the street-grid distance.
std::int64_t plot_minutes(const TownLayout& layout, int from, int to, const TownTravel& travel);

}  // namespace zs::gen
