// Spec: docs/tier0/specs/36-fields-and-weather.md
#include "zs/fields.h"

#include "zs/anomalies.h"

#include <algorithm>
#include <cmath>

#include "zs/clock.h"
#include "zs/detmath.h"
#include "zs/philox.h"

namespace zs::fields {

double keyed_unit(std::uint64_t seed, std::uint64_t id, std::uint32_t field, std::uint32_t k) {
  const auto out = philox4x32_10({static_cast<std::uint32_t>(id), static_cast<std::uint32_t>(id >> 32), field, k},
                                 {static_cast<std::uint32_t>(seed), static_cast<std::uint32_t>(seed >> 32)});
  const std::uint64_t bits = (static_cast<std::uint64_t>(out[0]) << 32) | out[1];
  return static_cast<double>(bits >> 11) * 0x1.0p-53;
}

double keyed_normal(std::uint64_t seed, std::uint64_t id, std::uint32_t field, std::uint32_t k) {
  // Box-Muller on two keyed units, with the deterministic transcendental functions.
  const double u1 = std::max(keyed_unit(seed, id, field, 2 * k), 1e-300);
  const double u2 = keyed_unit(seed, id, field, 2 * k + 1);
  return detmath::sqrt(-2.0 * detmath::log(u1)) * detmath::cos(2.0 * detmath::pi * u2);
}

std::vector<double> zero_mean_variation(std::uint64_t seed, std::uint64_t id, std::uint32_t field, std::size_t n,
                                        double amplitude) {
  std::vector<double> v(n);
  if (n == 0) return v;
  double sum = 0;
  for (std::size_t i = 0; i < n; ++i) {
    v[i] = amplitude * keyed_normal(seed, id, field, static_cast<std::uint32_t>(i));
    sum += v[i];
  }
  const double mean = sum / static_cast<double>(n);
  for (double& x : v) x -= mean;
  return v;
}

double coarsen(std::span<const double> values, std::span<const double> weights) {
  double sum = 0, w = 0;
  for (std::size_t i = 0; i < values.size() && i < weights.size(); ++i) {
    sum += values[i] * weights[i];
    w += weights[i];
  }
  return w > 0 ? sum / w : 0;
}

// ---- Weather

TimeOfYear weather_time(int year, int day_of_year, bool leap_year, double hour) {
  TimeOfYear t;
  t.year = year;
  t.day = leap_year && day_of_year >= 59 ? day_of_year - 1 : day_of_year;  // 29 February (58) repeats 28 February
  t.hour = hour;
  return t;
}

namespace {

// The 365 daily anomalies of a year for a cell: keyed normals smoothed over five days (weather has memory), scaled
// to a quarter of the seasonal range, then shifted so they sum to zero exactly.
std::vector<double> year_anomalies(std::uint64_t seed, std::uint64_t cell, const Climate& c, std::int64_t year) {
  const auto key = static_cast<std::uint32_t>(year * 1000003LL);
  std::vector<double> raw(kDaysPerYear);
  for (int d = 0; d < kDaysPerYear; ++d) raw[d] = keyed_normal(seed, cell, Field::DailyTemp, key + static_cast<std::uint32_t>(d));
  std::vector<double> v(kDaysPerYear);
  const double amp = 0.25 * c.temp_range_c;
  double sum = 0;
  for (int d = 0; d < kDaysPerYear; ++d) {
    double s = 0;
    for (int j = -2; j <= 2; ++j) s += raw[(d + j + kDaysPerYear) % kDaysPerYear];
    v[d] = amp * s / detmath::sqrt(5.0);
    sum += v[d];
  }
  const double mean = sum / kDaysPerYear;
  for (double& x : v) x -= mean;
  return v;
}

double seasonal_c(const Climate& c, int day) {
  // Coldest about day 20 (late January) in the north; the south is half a year away.
  const double phase = 2.0 * detmath::pi * (static_cast<double>(day) - 20.0) / kDaysPerYear + (c.south ? detmath::pi : 0.0);
  return -0.5 * c.temp_range_c * detmath::cos(phase);
}

}  // namespace

double daily_mean_c(std::uint64_t seed, std::uint64_t cell, const Climate& c, std::int64_t year, int day) {
  std::vector<double> a = year_anomalies(seed, cell, c, year);
  if (!c.anomalies) return c.temp_mean_c + seasonal_c(c, day) + a[static_cast<std::size_t>(day)];
  // Spec 101 rule 3: the day's weather has no monthly mean of its own (each month's daily anomalies sum to zero); the
  // month's regional anomaly, eased between mid-months, carries the month-to-month variation.
  const int m = day * 12 / kDaysPerYear;
  const int d0 = m * kDaysPerYear / 12, d1 = (m + 1) * kDaysPerYear / 12;
  double mean = 0;
  for (int d = d0; d < d1; ++d) mean += a[static_cast<std::size_t>(d)];
  mean /= d1 - d0;
  const double pos = (day + 0.5) * 12.0 / kDaysPerYear - 0.5;  // months from mid-January
  const std::int64_t m0 = static_cast<std::int64_t>(std::floor(pos));
  const double t = pos - static_cast<double>(m0);
  const double anom = (1 - t) * temp_month_anomaly_c(seed, c, year * 12 + m0) + t * temp_month_anomaly_c(seed, c, year * 12 + m0 + 1);
  return c.temp_mean_c + seasonal_c(c, day) + a[static_cast<std::size_t>(day)] - mean + anom;
}

double temperature_c(std::uint64_t seed, std::uint64_t cell, const Climate& c, const TimeOfYear& t) {
  const double diurnal_amp = 0.15 * c.temp_range_c + 2.0;  // placeholder: 2 °C plus a share of the seasonal swing
  const double diurnal = diurnal_amp * detmath::cos(2.0 * detmath::pi * (t.hour - 15.0) / 24.0);  // warmest 15:00, coldest 03:00
  return daily_mean_c(seed, cell, c, t.year, t.day) + diurnal;
}

namespace {

std::vector<double> year_rain(std::uint64_t seed, std::uint64_t cell, const Climate& c, std::int64_t year) {
  std::vector<double> v(kDaysPerYear, 0.0);
  if (c.rain_mm <= 0) return v;
  const auto key = static_cast<std::uint32_t>(year * 1000003LL);
  if (c.rain_kappa > 0 || c.anomalies) {
    // Spec 100 rule 5: each month takes its share of the year (the cell's seasonal curve); its wet days are drawn with
    // the year's wet-day share times the month's weight, and scaled so the month sums to its share exactly (A1).
    const double base = std::clamp(0.15 + 0.35 * (c.rain_mm / 2000.0), 0.1, 0.7);
    for (int m = 0; m < 12; ++m) {
      const int d0 = m * kDaysPerYear / 12, d1 = (m + 1) * kDaysPerYear / 12;
      const double share = rain_month_share(c, m);
      const double wet = std::clamp(base * 12 * share, 0.02, 0.95);
      double sum = 0;
      for (int d = d0; d < d1; ++d) {
        const auto kd = key + static_cast<std::uint32_t>(d);
        if (keyed_unit(seed, cell, Field::RainWet, kd) >= wet) continue;
        const double u = std::max(keyed_unit(seed, cell, Field::RainAmount, kd), 1e-12);
        v[static_cast<std::size_t>(d)] = -detmath::log(u);
        sum += v[static_cast<std::size_t>(d)];
      }
      // Spec 101 rule 2: the month's regional anomaly (1 without anomalies).
      const double month_mm = c.rain_mm * share * rain_month_factor(seed, c, year * 12 + m);
      if (sum <= 0) {  // no wet day drawn this month: its rain falls on its first day
        v[static_cast<std::size_t>(d0)] = month_mm;
        continue;
      }
      for (int d = d0; d < d1; ++d) v[static_cast<std::size_t>(d)] *= month_mm / sum;
    }
    return v;
  }
  // Wet-day share rises with annual rain: 15 % in a desert, about 50 % at 2,000 mm.
  const double wet_share = std::clamp(0.15 + 0.35 * (c.rain_mm / 2000.0), 0.1, 0.7);
  double sum = 0;
  for (int d = 0; d < kDaysPerYear; ++d) {
    const auto kd = key + static_cast<std::uint32_t>(d);
    if (keyed_unit(seed, cell, Field::RainWet, kd) >= wet_share) continue;
    const double u = std::max(keyed_unit(seed, cell, Field::RainAmount, kd), 1e-12);
    v[d] = -detmath::log(u);  // exponential amounts, mean 1
    sum += v[d];
  }
  if (sum <= 0) {  // a year with no wet day drawn (tiny chance): the rain falls on day 0
    v[0] = c.rain_mm;
    return v;
  }
  const double scale = c.rain_mm / sum;
  for (double& x : v) x *= scale;
  return v;
}

}  // namespace

double rain_month_share(const Climate& c, int month) {
  if (c.rain_kappa <= 0) return 1.0 / 12;
  double sum = 0, mine = 0;
  for (int m = 0; m < 12; ++m) {
    const double w = detmath::exp(c.rain_kappa * detmath::cos(2.0 * detmath::pi * (m + 0.5 - c.rain_peak) / 12.0));
    sum += w;
    if (m == month) mine = w;
  }
  return mine / sum;
}

double rain_day_mm(std::uint64_t seed, std::uint64_t cell, const Climate& c, std::int64_t year, int day) {
  return year_rain(seed, cell, c, year)[static_cast<std::size_t>(day)];
}

double rain_between_mm(std::uint64_t seed, std::uint64_t cell, const Climate& c, std::int64_t from_tick, std::int64_t to_tick) {
  if (to_tick <= from_tick || c.rain_mm <= 0) return 0;
  double total = 0;
  std::int64_t t = from_tick;
  std::int64_t cached_year = -1;
  std::vector<double> year;
  while (t < to_tick) {
    const std::int64_t day_abs = t / kTicksPerGameDay;
    const std::int64_t day_end = (day_abs + 1) * kTicksPerGameDay;
    const std::int64_t piece = std::min(day_end, to_tick) - t;
    const std::int64_t y = day_abs / kDaysPerYear;
    if (y != cached_year) { year = year_rain(seed, cell, c, y); cached_year = y; }
    const double day_mm = year[static_cast<std::size_t>(day_abs - y * kDaysPerYear)];
    total += day_mm * static_cast<double>(piece) / static_cast<double>(kTicksPerGameDay);
    t += piece;
  }
  return total;
}

// ---- Ambient noise

double ambient_db(std::span<const NoiseSource> sources, double floor_db) {
  double energy = detmath::pow(10.0, floor_db / 10.0);
  for (const NoiseSource& s : sources) {
    const double d = std::max(s.distance_m, 1.0);
    const double at = s.level_db_1m - 20.0 * detmath::log10(d);
    energy += detmath::pow(10.0, at / 10.0);
  }
  return 10.0 * detmath::log10(energy);
}

double audible_range_m(double level_db_1m, double ambient, double threshold_db) {
  const double margin = level_db_1m - ambient - threshold_db;
  if (margin <= 0) return 0;
  return detmath::pow(10.0, margin / 20.0);
}

}  // namespace zs::fields
