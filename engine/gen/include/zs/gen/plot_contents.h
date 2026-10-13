// Spec 72: what stands on a town's plots (decision 0077 item 3.3): the generator of a plot's interior. A residential plot
// holds a house (spec 51's planner fitted to the plot) or an apartment house (a flat per unit, on as many floors as it
// takes); a commercial plot a shop; an industrial plot a workshop; a park its trees and benches; a civic plot its station
// (spec 69). Each is a site map with the street along its top row, made from the layout's record when someone asks: a
// pure function of the town, the plot and a seed, so the same plot always gives the same building.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "zs/gen/dwellings.h"
#include "zs/gen/town_layout.h"

namespace zs::gen {

struct PlotRules {
  double couple_share = 0.7, coverage_commercial = 0.6, coverage_industrial = 0.5;
  double trees_per_100m2 = 1, benches_per_400m2 = 1;
};
// data/gen/plots.toml from `gen_dir`.
PlotRules load_plot_rules(const std::string& gen_dir);

// One household living on a plot: how many and whether the head lives with a partner.
struct Home {
  int members = 1;
  bool couple = false;
};
// Rule 1: the households of a residential plot, one per taken unit, their sizes drawn from the country's shares.
std::vector<Home> homes_on(const TownSpec& spec, const TownLayout& t, int plot, const PlotRules& rules, std::uint64_t seed);

// What a plot holds, planned (rules 2-5): the building and the households in it. `kind` is house, apartments, shop,
// workshop, park, civic (spec 69's station: drawn by its own generator) or block (the Tier 0 block's hand-made map).
struct PlotPlan {
  std::string kind;
  Dwelling building;        // empty for a park, civic plot or the block
  std::vector<Home> homes;  // a residential plot's households
  int frontage_m = 0, depth_m = 0;  // the plot seen from its street: width along it, depth back from it
};
PlotPlan plan_plot(const TownSpec& spec, const TownLayout& t, int plot, const DwellingRules& dr, const PlotRules& rules,
                   std::uint64_t seed);
// The plot as a site-map TOML (rule 6); empty for a civic plot or the block, which have their own maps.
std::string plot_toml(const TownSpec& spec, const TownLayout& t, int plot, const DwellingRules& dr, const PlotRules& rules,
                      std::uint64_t seed);

}  // namespace zs::gen
