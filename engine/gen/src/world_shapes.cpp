// Spec 89: the land's shapes. docs/tier0/specs/89-continents-and-poles.md
#include "zs/gen/world_shapes.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <queue>
#include <utility>

#include "zs/detmath.h"

namespace zs::gen {

LandShapes land_shapes(const WorldGrid& g, const WorldFields& f) {
  const std::size_t n = g.size();
  LandShapes s;
  // Landmasses: connected land cells.
  std::vector<int> comp(n, -1);
  for (CellId c = 0; c < n; ++c) {
    if (!f.land(c) || comp[c] >= 0) continue;
    const int k = static_cast<int>(s.landmasses_km2.size());
    s.landmasses_km2.push_back(0);
    std::vector<CellId> stack{c};
    comp[c] = k;
    while (!stack.empty()) {
      const CellId x = stack.back();
      stack.pop_back();
      s.landmasses_km2.back() += g.area_km2(x);
      for (const CellId nb : g.neighbours(x))
        if (f.land(nb) && comp[nb] < 0) comp[nb] = k, stack.push_back(nb);
    }
  }
  for (double a : s.landmasses_km2) s.land_km2 += a;
  std::sort(s.landmasses_km2.rbegin(), s.landmasses_km2.rend());
  for (double a : s.landmasses_km2) {
    s.over_1m += a >= 1e6;
    s.over_100k += a >= 1e5;
    s.over_10k += a >= 1e4;
    if (a < 1e6) s.islands += a;
  }
  {
    double sea = 0, ice = 0;
    for (CellId c = 0; c < n; ++c)
      if (!f.land(c)) sea += g.area_km2(c), ice += (!f.sea_ice.empty() && f.sea_ice[c]) ? g.area_km2(c) : 0.0;
    s.sea_ice = sea > 0 ? ice / sea : 0;
  }
  if (s.land_km2 <= 0) return s;
  s.islands /= s.land_km2;

  // Distance to the sea over land: from each coastal land cell, half a step to the sea, then over land.
  const double spacing = detmath::sqrt(g.sphere_area_km2() / static_cast<double>(n));
  std::vector<double> km(n, 1e300);
  using Item = std::pair<double, CellId>;
  std::priority_queue<Item, std::vector<Item>, std::greater<Item>> q;
  for (CellId c = 0; c < n; ++c) {
    if (!f.land(c)) continue;
    for (const CellId nb : g.neighbours(c))
      if (!f.land(nb)) {
        km[c] = 0.5 * spacing;
        q.emplace(km[c], c);
        break;
      }
  }
  while (!q.empty()) {
    const auto [d, c] = q.top();
    q.pop();
    if (d > km[c]) continue;
    for (const CellId nb : g.neighbours(c)) {
      if (!f.land(nb)) continue;
      const double nd = d + g.distance_km(c, nb);
      if (nd < km[nb]) km[nb] = nd, q.emplace(nd, nb);
    }
  }
  static const double limits[5] = {50, 100, 200, 400, 800};
  std::vector<std::pair<double, double>> dist;  // (km, area)
  for (CellId c = 0; c < n; ++c) {
    if (!f.land(c)) continue;
    const double a = g.area_km2(c);
    for (int i = 0; i < 5; ++i)
      if (km[c] <= limits[i]) s.near_sea[static_cast<std::size_t>(i)] += a;
    dist.push_back({km[c], a});
    const double lat = std::asin(std::clamp(g.sin_lat(c), -1.0, 1.0)) * 180.0 / 3.14159265358979323846;
    const int band = std::clamp(static_cast<int>(std::floor((lat + 90.0) / 10.0)), 0, 17);
    s.by_latitude[static_cast<std::size_t>(band)] += a;
    if (lat >= 66.56) s.arctic += a;
    if (lat <= -66.56) s.antarctic += a;
    if (lat >= 0) s.north += a;
    int land_nb = 0;
    for (const CellId nb : g.neighbours(c)) land_nb += f.land(nb);
    if (land_nb <= 2) s.thin += a;
  }
  for (double& x : s.near_sea) x /= s.land_km2;
  for (double& x : s.by_latitude) x /= s.land_km2;
  s.arctic /= s.land_km2, s.antarctic /= s.land_km2, s.north /= s.land_km2, s.thin /= s.land_km2;
  std::sort(dist.begin(), dist.end());
  double acc = 0;
  for (const auto& [d, a] : dist)
    if ((acc += a) >= s.land_km2 / 2) {
      s.median_sea_km = d;
      break;
    }
  return s;
}

}  // namespace zs::gen
