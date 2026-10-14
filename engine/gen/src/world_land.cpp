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
// Spec 83 rule 4: a depression shallower than this is a pit of the coarse terrain that a river would cut through
// (estimate, calibrated on seeds 1-5 against WMR-8's lake share).
constexpr double kLakeMinDepthM = 30;

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

// Rule 10 (estimates): {forest share, growing stock m3/ha of forest, regrowth r}. The shares are today's cover,
// cleared land taken off (spec 83 As built: with Earth's biome areas they give FRA's 31 %, WMR-13).
struct ForestOf {
  double share, m3_per_ha, r;
};
ForestOf forest_of(Biome b) {
  switch (b) {
    case Biome::TropicalForest: return {0.6, 195, 0.06};
    case Biome::TemperateForest: return {0.45, 140, 0.04};
    case Biome::BorealForest: return {0.6, 90, 0.02};
    case Biome::Mountain: return {0.2, 75, 0.03};
    case Biome::Savanna: return {0.12, 40, 0.03};
    case Biome::Grassland: return {0.05, 35, 0.03};
    case Biome::Tundra: return {0.03, 20, 0.03};
    case Biome::Desert: return {0.01, 10, 0.03};
    default: return {0, 0, 0};
  }
}

// Rule 11 (estimates): wild herbivore wet biomass today, kg per km2.
double game_of(Biome b) {
  switch (b) {
    case Biome::Savanna: return 300;
    case Biome::Grassland: return 125;
    case Biome::TemperateForest: return 75;
    case Biome::TropicalForest: return 60;
    case Biome::BorealForest: return 40;
    case Biome::Mountain: return 30;
    case Biome::Tundra: return 20;
    case Biome::Desert: return 5;
    default: return 0;
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
  w.endorheic.assign(n, 0);
  w.fertility.assign(n, 0);
  w.arable_share.assign(n, 0);
  w.pasture_share.assign(n, 0);
  w.capacity_food.assign(n, 0);
  w.capacity_water.assign(n, 0);
  w.forest_share.assign(n, 0);
  w.wood_m3.assign(n, 0);
  w.wood_r.assign(n, 0);
  w.game_kg.assign(n, 0);
  w.fish_catch_t.assign(n, 0);
  w.fish_k_t.assign(n, 0);
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
  // Spec 83 rule 4: lakes that balance. A depression is the connected lake cells at one fill level; taken from
  // upstream to downstream, it spills on if its inflow covers its surface's evaporation, else it is closed.
  {
    std::vector<std::size_t> rank(n, 0);  // flood order; larger is further upstream
    for (std::size_t k = 0; k < order.size(); ++k) rank[order[k]] = k + 1;
    std::vector<int> group(n, -1);
    std::vector<std::vector<CellId>> groups;
    for (const CellId s : order) {
      if (!w.lake[s] || group[s] >= 0) continue;
      const int k = static_cast<int>(groups.size());
      groups.emplace_back();
      std::vector<CellId> stack{s};
      group[s] = k;
      while (!stack.empty()) {
        const CellId c = stack.back();
        stack.pop_back();
        groups[k].push_back(c);
        for (const CellId nb : grid.neighbours(c))
          if (f.land(nb) && w.lake[nb] && group[nb] < 0 && fill[nb] == fill[c]) group[nb] = k, stack.push_back(nb);
      }
    }
    // Upstream first: the group whose earliest-reached cell came last in the flood.
    std::vector<std::size_t> gorder(groups.size());
    std::vector<std::size_t> first(groups.size(), 0);
    for (std::size_t k = 0; k < groups.size(); ++k) {
      gorder[k] = k;
      std::size_t r = rank[groups[k][0]];
      for (const CellId c : groups[k]) r = std::min(r, rank[c]);
      first[k] = r;
    }
    std::sort(gorder.begin(), gorder.end(), [&](std::size_t a, std::size_t b) { return first[a] > first[b]; });
    constexpr double kSec = kSecondsPerYear;
    auto evap_m3s = [&](CellId c) {
      return std::max(0.0, pet_mm(f.temp_mean_c[c]) - f.rain_mm[c]) * 1e-3 * grid.area_km2(c) * 1e6 / kSec;
    };
    for (const std::size_t k : gorder) {
      std::vector<CellId>& cells = groups[k];
      double depth = 0;
      for (const CellId c : cells) depth = std::max(depth, fill[c] - f.elevation_m[c]);
      if (depth < kLakeMinDepthM) {  // a shallow pit of the coarse terrain: a river runs through it, no lake
        for (const CellId c : cells) w.lake[c] = kLakeNone;
        continue;
      }
      double inflow = 0, demand = 0;
      std::vector<CellId> exits;
      for (const CellId c : cells) {
        demand += evap_m3s(c);
        const CellId d = w.downstream[c];
        if (d == kNoCell || group[d] != static_cast<int>(k)) exits.push_back(c), inflow += w.discharge_m3s[c];
      }
      if (inflow >= demand) {
        ++w.open_lakes;
        continue;
      }
      ++w.closed_basins;
      std::sort(cells.begin(), cells.end(), [&](CellId a, CellId b) {
        return f.elevation_m[a] != f.elevation_m[b] ? f.elevation_m[a] < f.elevation_m[b] : a < b;
      });
      double evap = 0;
      for (const CellId c : cells) {
        w.lake[c] = evap < inflow || c == cells.front() ? kLakeTerminal : kLakeDry;
        evap += evap_m3s(c);
      }
      w.closed_evap_m3s += inflow;
      for (const CellId x : exits) {
        const double q = w.discharge_m3s[x], a = w.catchment_km2[x];
        CellId d = w.downstream[x];
        w.downstream[x] = kNoCell;
        if (d != kNoCell && !f.land(d)) w.sea_inflow_m3s -= q;  // the exit spilled straight into the sea
        while (d != kNoCell && f.land(d)) {
          w.discharge_m3s[d] -= q;
          w.catchment_km2[d] -= a;
          if (w.downstream[d] != kNoCell && !f.land(w.downstream[d])) w.sea_inflow_m3s -= q;
          d = w.downstream[d];
        }
      }
    }
    // Endorheic: the water ends at a land cell with no downstream. Downstream cells come first in `order`.
    for (const CellId c : order) {
      const CellId d = w.downstream[c];
      w.endorheic[c] = d == kNoCell ? 1 : f.land(d) ? w.endorheic[d] : 0;
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
    // Rules 10-11: forest and game.
    const ForestOf fo = forest_of(b);
    w.forest_share[i] = fo.share;
    w.wood_m3[i] = fo.share * ha * fo.m3_per_ha;
    w.wood_r[i] = fo.r;
    w.game_kg[i] = game_of(b) * grid.area_km2(i) * (w.river[i] ? 1.5 : 1.0);
  }
  // Rule 12: fish on the shelf; colder and coastal seas (within 300 km of land, on any grid) are richer.
  std::vector<CellId> land_cells;
  for (CellId i = 0; i < n; ++i)
    if (f.land(i)) land_cells.push_back(i);
  const std::vector<std::uint32_t> to_land = grid.graph_distance(land_cells);
  const double km_per_step = detmath::sqrt(grid.sphere_area_km2() / static_cast<double>(n));
  for (CellId i = 0; i < n; ++i) {
    if (f.land(i) || f.elevation_m[i] < p.fish_shelf_m) continue;
    const bool coastal = to_land[i] * km_per_step <= 300.0;
    const double temp = 1.0 + (15.0 - f.temp_mean_c[i]) / 30.0;
    w.fish_catch_t[i] = std::max(0.0, p.fish_t_per_km2 * grid.area_km2(i) * (coastal ? 1.5 : 1.0) * temp);
    w.fish_k_t[i] = 4.0 * w.fish_catch_t[i] / p.fish_r;  // Schaefer: catch = rK/4
  }
  return w;
}

}  // namespace zs::gen
