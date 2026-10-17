// Spec 101: regional anomalies and wind. docs/tier0/specs/101-droughts-and-wind.md.
#include "zs/anomalies.h"

#include <algorithm>
#include <cmath>

#include "zs/detmath.h"

namespace zs::fields {

namespace {
constexpr double kEarthRadiusKm = 6371.0;
// Rule 1 (tuned in the spec's As built to DW-3 and DW-5): the lattice spacing that gives Earth's decay distances.
constexpr double kRainScaleKm = 850;  // monthly rain anomalies fall to 1/e at about 600 km
constexpr double kTempScaleKm = 2000;  // monthly temperature anomalies at about 1,600 km
constexpr double kTempPersist = 0.3;   // DW-5: lag-one correlation about 0.29
constexpr double kNoiseSd = 0.6402639991163567;  // the mean standard deviation of smoothstep value noise of unit normals

double smooth(double u) { return u * u * (3 - 2 * u); }

std::uint64_t corner_id(std::int64_t x, std::int64_t y, std::int64_t z) {
  std::uint64_t h = static_cast<std::uint64_t>(x) * 0x9E3779B97F4A7C15ull;
  h ^= static_cast<std::uint64_t>(y) * 0xC2B2AE3D27D4EB4Full + (h << 6) + (h >> 2);
  h ^= static_cast<std::uint64_t>(z) * 0x165667B19E3779F9ull + (h << 6) + (h >> 2);
  return h;
}

double month_weight_winter(double lat_deg, int month) {  // 1 in midwinter, 0 in midsummer
  const double peak = lat_deg >= 0 ? 0.5 : 6.5;          // mid-January in the north, mid-July in the south
  return 0.5 * (1 + detmath::cos(2 * detmath::pi * (month + 0.5 - peak) / 12));
}
}  // namespace

double coherent_normal(std::uint64_t seed, std::uint32_t field, double lat_deg, double lon_deg, double scale_km, std::int64_t index) {
  const double la = lat_deg * detmath::pi / 180, lo = lon_deg * detmath::pi / 180;
  const double k = kEarthRadiusKm / scale_km;
  const auto key = static_cast<std::uint32_t>(index);
  // A keyed shift per index, so the lattice (and its slightly lower variance mid-cell) falls elsewhere each month.
  const double p[3] = {detmath::cos(la) * detmath::cos(lo) * k + keyed_unit(seed, 1, field, key) * 7.0,
                       detmath::cos(la) * detmath::sin(lo) * k + keyed_unit(seed, 2, field, key) * 7.0,
                       detmath::sin(la) * k + keyed_unit(seed, 3, field, key) * 7.0};
  const double f[3] = {std::floor(p[0]), std::floor(p[1]), std::floor(p[2])};
  const double u[3] = {smooth(p[0] - f[0]), smooth(p[1] - f[1]), smooth(p[2] - f[2])};
  double v = 0;
  for (int c = 0; c < 8; ++c) {
    const int dx = c & 1, dy = (c >> 1) & 1, dz = (c >> 2) & 1;
    const double w = (dx ? u[0] : 1 - u[0]) * (dy ? u[1] : 1 - u[1]) * (dz ? u[2] : 1 - u[2]);
    const std::uint64_t id = corner_id(static_cast<std::int64_t>(f[0]) + dx, static_cast<std::int64_t>(f[1]) + dy, static_cast<std::int64_t>(f[2]) + dz);
    v += w * keyed_normal(seed, id, field, key);
  }
  return v / kNoiseSd;
}

double rain_cv_target(double rain_mm) { return std::clamp(0.329 - 0.0278 * detmath::log(std::max(rain_mm, 10.0)), 0.05, 0.4); }

double rain_month_factor(std::uint64_t seed, const Climate& c, std::int64_t month_abs) {
  if (!c.anomalies || c.rain_mm <= 0) return 1.0;
  double s2 = 0;
  for (int k = 0; k < 12; ++k) s2 += rain_month_share(c, k) * rain_month_share(c, k);
  // Months independent (DW-3: lag-one 0.08): the year's CV is the months' times the root of the summed squared shares.
  const double s = rain_cv_target(c.rain_mm) / detmath::sqrt(s2);
  const double sigma = detmath::sqrt(detmath::log(1 + s * s));
  const double z = coherent_normal(seed, Field::RainAnom, c.lat_deg, c.lon_deg, kRainScaleKm, month_abs);
  return detmath::exp(sigma * z - sigma * sigma / 2);
}

double temp_anomaly_sd(double lat_deg, int month) {
  // DW-4: Earth's land, north of the equator by 15-degree band (December-February, June-August); the south's lower.
  static const double centre[6] = {7.5, 22.5, 37.5, 52.5, 67.5, 82.5};
  static const double winter[6] = {0.65, 1.1, 1.9, 2.8, 3.0, 2.3};
  static const double summer[6] = {0.46, 0.62, 1.0, 1.37, 1.42, 1.16};
  const double a = std::abs(lat_deg);
  int i = 0;
  while (i < 4 && a > centre[i + 1]) ++i;
  const double t = std::clamp((a - centre[i]) / (centre[i + 1] - centre[i]), 0.0, 1.0);
  double w = winter[i] + t * (winter[i + 1] - winter[i]), s = summer[i] + t * (summer[i + 1] - summer[i]);
  if (lat_deg < 0 && a > 45) w = std::min(w, 1.2), s = std::min(s, 0.95);  // DW-4: the southern lands' 1.15 and 0.88
  return s + (w - s) * month_weight_winter(lat_deg, month);
}

double temp_month_anomaly_c(std::uint64_t seed, const Climate& c, std::int64_t month_abs) {
  if (!c.anomalies) return 0.0;
  const double a = kTempPersist;
  double z = 0;
  for (int k = 0; k < 3; ++k) z += detmath::pow(a, k) * coherent_normal(seed, Field::TempAnom, c.lat_deg, c.lon_deg, kTempScaleKm, month_abs - k);
  z /= detmath::sqrt(1 + a * a + a * a * a * a);
  return temp_anomaly_sd(c.lat_deg, static_cast<int>(((month_abs % 12) + 12) % 12)) * z;
}

double prevailing_from_deg(double lat_deg) {
  const double a = std::abs(lat_deg);
  const bool north = lat_deg >= 0;
  if (a < 30) return north ? 45 : 135;    // the trades: north-east and south-east
  if (a < 60) return north ? 225 : 315;   // the westerlies: south-west and north-west
  return north ? 45 : 135;                // polar easterlies
}

double wind_ms(std::uint64_t seed, std::uint64_t cell, const Climate& c, const TimeOfYear& t) {
  if (c.wind_mean_ms <= 0) return 0.0;
  const auto key = static_cast<std::uint32_t>(t.year * 1000003LL + t.day);
  // DW-7: a day's mean speed Weibull with shape 2 (Rayleigh) around the cell's mean; DW-6: the windiest month about 21 %
  // above the mean in winter; over land the afternoon windier than the night (estimate).
  const double u = std::max(keyed_unit(seed, cell, Field::WindSpeed, key), 1e-12);
  const double day = c.wind_mean_ms * detmath::sqrt(-detmath::log(u)) / 0.8862269254527580;
  const double winter = c.south ? 0.5 * (1 + detmath::cos(2 * detmath::pi * (t.day - 20 - 182.5) / kDaysPerYear))
                                : 0.5 * (1 + detmath::cos(2 * detmath::pi * (t.day - 20) / kDaysPerYear));
  const double season = 1 + 0.21 * (2 * winter - 1);
  const double diurnal = 1 + 0.2 * detmath::cos(2 * detmath::pi * (t.hour - 14.0) / 24.0);
  return std::max(0.0, day * season * diurnal);
}

double wind_from_deg(std::uint64_t seed, std::uint64_t cell, const Climate& c, const TimeOfYear& t) {
  const auto key = static_cast<std::uint32_t>(t.year * 1000003LL + t.day);
  const double turn = 45.0 * keyed_normal(seed, cell, Field::WindTurn, key);  // estimate: a day's wind within about 45 degrees
  double d = std::fmod(c.wind_from_deg + turn, 360.0);
  return d < 0 ? d + 360 : d;
}

double snow_share(double temp_c) { return std::clamp((kSnowNoneC - temp_c) / (kSnowNoneC - kSnowAllC), 0.0, 1.0); }

std::vector<SnowDay> snow_year(std::uint64_t seed, std::uint64_t cell, const Climate& c, std::int64_t year) {
  std::vector<SnowDay> out(kDaysPerYear);
  // From the warmest day of the year before (late July north, late January south), when the snow is gone.
  const int start = c.south ? 20 : 20 + kDaysPerYear / 2;
  double swe = 0, cold = 0;
  for (std::int64_t y = year - 1; y <= year; ++y)
    for (int d = y < year ? start : 0; d < kDaysPerYear; ++d) {
      const double t = daily_mean_c(seed, cell, c, y, d);
      const double p = rain_day_mm(seed, cell, c, y, d);
      const double snow = p * snow_share(t);
      const double melt = std::min(swe + snow, kMeltMmPerCDay * std::max(0.0, t));
      swe += snow - melt;
      const double depth = swe / (10 * kSnowDensity);  // mm of water to cm of snow
      cold = std::max(0.0, cold + (t < 0 ? -t * detmath::exp(-depth / 20) : -2 * t));
      if (y == year) out[static_cast<std::size_t>(d)] = {swe, depth, 5 * detmath::sqrt(cold)};
    }
  return out;
}

}  // namespace zs::fields
