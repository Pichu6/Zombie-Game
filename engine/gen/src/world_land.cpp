// Spec 52 slice 1: runoff, drainage, rivers, lakes, fertility, arable land and capacity.
// docs/tier0/specs/52-land-water-capacity.md
#include "zs/gen/world_land.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <utility>

#include "zs/detmath.h"

namespace zs::gen {
namespace {

constexpr double kSecondsPerYear = 365.25 * 86400;

// Rule 5 (estimates).
double fertility_of(Biome b) {
  switch (b) {
    case Biome::Grassland: return 0.9;
    case Biome::TemperateForest: return 0.8;
    case Biome::Savanna: return 0.6;
    case Biome::TropicalForest: return 0.5;
    case Biome::BorealForest: return 0.4;
    case Biome::Mountain: return 0.25;
    case Biome::Tundra: return 0.15;
    case Biome::Desert: return 0.1;
    default: return 0;
  }
}

// Rule 6 (estimates): {arable, pasture}.
std::pair<double, double> shares_of(Biome b) {
  switch (b) {
    case Biome::Grassland: return {0.45, 0.35};
    case Biome::TemperateForest: return {0.35, 0.05};
    case Biome::Savanna: return {0.25, 0.45};
    case Biome::TropicalForest: return {0.15, 0.05};
    case Biome::BorealForest: return {0.08, 0.05};
    case Biome::Mountain: return {0.04, 0.15};
    case Biome::Desert: return {0.02, 0.08};
    case Biome::Tundra: return {0, 0.15};
    default: return {0, 0};
  }
}

// Rule 1: P - E, with E from Fu's equation and PET from the annual mean temperature (RV-3, RV-5).
double runoff_mm(double rain_mm, double temp_c, const LandParams& p) {
  const double omega = p.fu_omega;
  if (rain_mm <= 0) return 0;
  const double pet = std::max(p.pet_min_mm, p.pet_base_mm + p.pet_per_c * temp_c);
  const double x = pet / rain_mm;
  const double e_over_p = 1.0 + x - detmath::pow(1.0 + detmath::pow(x, omega), 1.0 / omega);
  return std::max(0.0, rain_mm * (1.0 - std::clamp(e_over_p, 0.0, 1.0)));
}

}  // namespace

WorldLand generate_land(const WorldGrid& grid, const WorldFields& f, const LandParams& p) {
  const std::size_t n = grid.size();
  WorldLand w;
  w.runoff_mm.assign(n, 0);
  w.downstream.assign(n, kNoCell);
  w.discharge_m3s.assign(n, 0);
  w.catchment_km2.assign(n, 0);
  w.river.assign(n, 0);
  w.lake.assign(n, 0);
  w.fertility.assign(n, 0);
  w.arable_share.assign(n, 0);
  w.pasture_share.assign(n, 0);
  w.capacity_food.assign(n, 0);
  w.capacity_water.assign(n, 0);
  for (CellId i = 0; i < n; ++i) {
    if (!f.land(i)) continue;
    w.runoff_mm[i] = runoff_mm(f.rain_mm[i], f.temp_mean_c[i], p);
    w.land_rain_km3 += f.rain_mm[i] * 1e-6 * grid.area_km2(i);
    w.runoff_km3 += w.runoff_mm[i] * 1e-6 * grid.area_km2(i);
  }
  // Rule 2: a priority flood from the sea, lowest first, ties by cell id.
  using Item = std::pair<double, CellId>;
  std::priority_queue<Item, std::vector<Item>, std::greater<Item>> open;
  std::vector<std::uint8_t> seen(n, 0);
  std::vector<double> fill(n, 0);
  for (CellId i = 0; i < n; ++i)
    if (!f.land(i)) {
      seen[i] = 1;
      open.emplace(0.0, i);
    }
  if (open.empty()) {  // a planet without sea: its lowest cell is the outlet
    CellId low = 0;
    for (CellId i = 1; i < n; ++i)
      if (f.elevation_m[i] < f.elevation_m[low]) low = i;
    seen[low] = 1;
    fill[low] = f.elevation_m[low];
    open.emplace(fill[low], low);
  }
  std::vector<CellId> order;  // land cells in the order the flood reached them (downstream first)
  order.reserve(n);
  while (!open.empty()) {
    const auto [level, c] = open.top();
    open.pop();
    for (CellId nb : grid.neighbours(c)) {
      if (seen[nb]) continue;
      seen[nb] = 1;
      fill[nb] = std::max(f.elevation_m[nb], level);
      w.downstream[nb] = c;
      w.lake[nb] = fill[nb] > f.elevation_m[nb] ? 1 : 0;
      open.emplace(fill[nb], nb);
      order.push_back(nb);
    }
  }
  // Rule 3: discharge, upstream first; what reaches a sea cell flows out.
  for (CellId i = 0; i < n; ++i)
    if (f.land(i)) {
      w.discharge_m3s[i] = w.runoff_mm[i] * 1e-3 * grid.area_km2(i) * 1e6 / kSecondsPerYear;
      w.catchment_km2[i] = grid.area_km2(i);
    }
  for (auto it = order.rbegin(); it != order.rend(); ++it) {
    const CellId c = *it, d = w.downstream[c];
    if (d == kNoCell) continue;
    if (f.land(d)) {
      w.discharge_m3s[d] += w.discharge_m3s[c];
      w.catchment_km2[d] += w.catchment_km2[c];
    } else {
      w.sea_inflow_m3s += w.discharge_m3s[c];
    }
  }
  // Rules 4-9.
  for (CellId i = 0; i < n; ++i) {
    if (!f.land(i)) continue;
    // Rule 4: a river needs both water and a catchment, so a single wet cell of a coarse grid is not one.
    const double q = w.discharge_m3s[i], a = w.catchment_km2[i];
    w.river[i] = q >= p.large_river_m3s && a >= p.large_river_km2 ? 2 : q >= p.river_m3s && a >= p.river_km2 ? 1 : 0;
    const Biome b = f.biome[i];
    w.fertility[i] = std::min(1.0, fertility_of(b) + (w.river[i] ? 0.2 : 0.0));
    auto [arable, pasture] = shares_of(b);
    if (b == Biome::Desert && w.river[i]) arable += 0.25;  // an oasis valley
    w.arable_share[i] = std::min(arable, 1.0);
    w.pasture_share[i] = std::min(pasture, 1.0 - w.arable_share[i]);
    const double ha = grid.area_km2(i) * 100;
    const double crop_kcal_per_ha = p.yield_t_per_ha * w.fertility[i] * p.kcal_per_t;
    const double kcal = w.arable_share[i] * ha * crop_kcal_per_ha * (1 - p.feed_share) +
                        w.pasture_share[i] * ha * crop_kcal_per_ha * p.pasture_vs_crop;
    w.capacity_food[i] = kcal / (365.0 * p.kcal_per_person_day);
    const double water_m3 = w.runoff_mm[i] * 1e-3 * grid.area_km2(i) * 1e6 + (w.river[i] ? w.discharge_m3s[i] * kSecondsPerYear : 0.0);
    w.capacity_water[i] = water_m3 / p.water_m3_per_person;
  }
  return w;
}

}  // namespace zs::gen
