// Spec 100: monthly normals and rainy seasons. Each land cell's annual rain spread over its months by a von Mises
// curve (strength kappa, peak month) from the drivers that make Earth's seasons: the tropical rain belt following the sun,
// dry summers on the west sides of the subtropics (Mediterranean), wet summers on their east sides (monsoon), summer
// rain in continental interiors and winter rain on mid-latitude west coasts. Calibrated to WorldClim 2.1 measured on
// Earth (research rain-seasons, RS). docs/tier0/specs/100-monthly-normals-and-rainy-seasons.md.
#pragma once
#include <array>
#include <string>

#include "zs/gen/world_fields.h"
#include "zs/gen/world_grid.h"

namespace zs::gen {

// Rule 2 (tuned in the spec's As built): the weights of each driver on the cell's season (positive: a summer peak,
// negative: a winter peak) and the strongest curve.
struct SeasonParams {
  double tropic = 0.48, tropic_equator = 0.18;   // the rain belt: at the tropics' edges, and at the equator
  double mediterranean = 0.4;                    // west sides at 28-48 degrees
  double monsoon = 1.0, monsoon_width_km = 8000;  // east sides at 16-54 degrees of continents this wide (Asia's)
  double continental = 0.5, continental_km = 900;  // interiors at 38-78 degrees
  double maritime = 0.25, maritime_km = 500;     // mid-latitude west coasts
  double kappa_max = 2.8;
};

// Rule 1: fills f.rain_kappa and f.rain_peak (land; 0 at sea).
void fill_rain_seasons(const WorldGrid& grid, WorldFields& f, const SeasonParams& p = {});

// Spec 101 rule 4: every cell's mean wind speed at 10 m: over land Earth's by latitude (WorldClim, DW-6), more on coasts
// and heights; at sea an estimate by belt.
void fill_winds(const WorldGrid& grid, WorldFields& f);

// Rule 3: the cell's twelve monthly normals (January first): rain in mm adding to rain_mm; mean temperature by spec 36's
// seasonal cosine (coldest near 20 January in the north, July in the south; amplitude half the annual range).
std::array<double, 12> rain_months_mm(const WorldFields& f, CellId i);
std::array<double, 12> temp_months_c(const WorldGrid& g, const WorldFields& f, CellId i);

// Rule 4: the Koppen-Geiger class (Peel et al. 2007's rules) from the monthly normals: "Af", "Am", "Aw", "BW", "BS",
// "Cs", "Cw", "Cf", "Ds", "Dw", "Df", "ET", "EF".
std::string koppen_class(const std::array<double, 12>& rain_mm, const std::array<double, 12>& temp_c, bool south);

}  // namespace zs::gen
