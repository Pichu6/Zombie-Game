// Spec 102: snow by month and the rivers' seasonal regimes. A cell's precipitation falls as snow when it is cold
// (half and half at 1 C, Jennings et al. 2018), the snowpack melts by degree-days (4 mm a day for each degree above 0);
// a cell's runoff by month is its liquid water (rain and melt) over the month's potential evaporation, scaled to its
// year's runoff, part of it released evenly (baseflow); down the rivers each step delays the water by its length.
// Calibrated against measured regimes (research snow-and-rivers, SR). docs/tier0/specs/102-snow-and-river-regimes.md.
#pragma once
#include <array>

#include "zs/gen/world_fields.h"
#include "zs/gen/world_grid.h"
#include "zs/gen/world_land.h"

namespace zs::gen {

struct RegimeParams {
  double snow_none_c = 3, snow_all_c = -1;  // SR-1: a linear share between, half at 1 C
  double melt_mm_per_c_day = 4;             // SR-2: 3-5 for deep snowpacks
  double baseflow = 0.35;                   // the share of runoff released evenly through the year (estimate)
  double river_speed_ms = 1.0;              // how fast a flood travels down a river (estimate)
};

// Rule 1: the share of precipitation falling as snow at a temperature.
double snow_share(double temp_c, const RegimeParams& p = {});
// Rule 2: the snowpack's water at each month's end (mm, spun up over two years); `liquid` gets each month's rain and melt.
std::array<double, 12> snow_months_mm(const WorldGrid& g, const WorldFields& f, CellId i, const RegimeParams& p = {},
                                      std::array<double, 12>* liquid = nullptr);
// Rules 3-4: fills w.regime (each land cell's flow by month over its mean) and w.snow_mm.
void fill_regimes(const WorldGrid& g, const WorldFields& f, WorldLand& w, const RegimeParams& p = {});

}  // namespace zs::gen
