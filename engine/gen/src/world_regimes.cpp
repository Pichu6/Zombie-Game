// Spec 102: snow by month and the rivers' seasonal regimes. docs/tier0/specs/102-snow-and-river-regimes.md.
#include "zs/gen/world_regimes.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "zs/detmath.h"
#include "zs/gen/world_seasons.h"

namespace zs::gen {

namespace {
constexpr double kDaysPerMonth = 365.0 / 12;
constexpr double kSecondsPerMonth = kDaysPerMonth * 86400;
}  // namespace

double snow_share(double temp_c, const RegimeParams& p) {
  return std::clamp((p.snow_none_c - temp_c) / (p.snow_none_c - p.snow_all_c), 0.0, 1.0);
}

std::array<double, 12> snow_months_mm(const WorldGrid& g, const WorldFields& f, CellId i, const RegimeParams& p,
                                      std::array<double, 12>* liquid) {
  std::array<double, 12> swe{};
  if (!f.land(i)) return swe;
  const auto P = rain_months_mm(f, i);
  const auto T = temp_months_c(g, f, i);
  double s = 0;
  for (int pass = 0; pass < 3; ++pass)  // spin-up: the snow left from the winters before
    for (int m = 0; m < 12; ++m) {
      const auto k = static_cast<std::size_t>(m);
      const double snow = P[k] * snow_share(T[k], p);
      const double melt = std::min(s + snow, p.melt_mm_per_c_day * std::max(0.0, T[k]) * kDaysPerMonth);
      s += snow - melt;
      if (pass == 2) {
        swe[k] = s;
        if (liquid) (*liquid)[k] = P[k] - snow + melt;
      }
    }
  return swe;
}

void fill_regimes(const WorldGrid& g, const WorldFields& f, WorldLand& w, const RegimeParams& p) {
  const std::size_t n = g.size();
  w.regime.assign(n, std::array<float, 12>{});
  w.snow_mm.assign(n, std::array<float, 12>{});
  std::vector<std::array<double, 12>> q(n, std::array<double, 12>{});
  std::vector<CellId> cells;
  for (CellId i = 0; i < n; ++i) {
    if (!f.land(i)) continue;
    cells.push_back(i);
    // Rule 3: a cell's runoff by month: its liquid water (rain and melt) less the month's potential evaporation,
    // scaled to its year's runoff (spec 52), part of it held back and released evenly (baseflow).
    std::array<double, 12> liquid{};
    const auto swe = snow_months_mm(g, f, i, p, &liquid);
    for (int m = 0; m < 12; ++m) w.snow_mm[i][static_cast<std::size_t>(m)] = static_cast<float>(swe[static_cast<std::size_t>(m)]);
    const auto T = temp_months_c(g, f, i);
    std::array<double, 12> s{};
    double sum = 0;
    for (int m = 0; m < 12; ++m) {
      const auto k = static_cast<std::size_t>(m);
      s[k] = std::max(0.0, liquid[k] - pet_mm(T[k]) / 12);
      sum += s[k];
    }
    if (sum <= 0) {
      for (int m = 0; m < 12; ++m) s[static_cast<std::size_t>(m)] = std::max(0.0, liquid[static_cast<std::size_t>(m)]);
      sum = std::accumulate(s.begin(), s.end(), 0.0);
    }
    const double year_m3s = w.runoff_mm[i] * 1e-3 * g.area_km2(i) * 1e6 / (12 * kSecondsPerMonth);  // the mean flow
    for (int m = 0; m < 12; ++m) {
      const double share = sum > 0 ? s[static_cast<std::size_t>(m)] / sum : 1.0 / 12;
      q[i][static_cast<std::size_t>(m)] = year_m3s * 12 * ((1 - p.baseflow) * share + p.baseflow / 12);
    }
  }
  // Rule 4: down the rivers, upstream first (a cell drains a larger catchment than any above it); each step delays the
  // water by its length at the river's speed, so a large basin's flood arrives weeks after its rain.
  std::stable_sort(cells.begin(), cells.end(), [&](CellId a, CellId b) { return w.catchment_km2[a] < w.catchment_km2[b]; });
  for (CellId c : cells) {
    const CellId d = w.downstream[c];
    if (d == kNoCell || !f.land(d)) continue;
    const double lag = g.distance_km(c, d) / (p.river_speed_ms * kSecondsPerMonth / 1000);  // months
    for (int m = 0; m < 12; ++m)
      q[d][static_cast<std::size_t>(m)] += (1 - lag) * q[c][static_cast<std::size_t>(m)] + lag * q[c][static_cast<std::size_t>((m + 11) % 12)];
  }
  // The regime: each month over the year's mean (Pardé coefficients, adding to 12).
  for (CellId c : cells) {
    const double mean = std::accumulate(q[c].begin(), q[c].end(), 0.0) / 12;
    for (int m = 0; m < 12; ++m)
      w.regime[c][static_cast<std::size_t>(m)] = static_cast<float>(mean > 0 ? q[c][static_cast<std::size_t>(m)] / mean : 1.0);
  }
}

}  // namespace zs::gen
