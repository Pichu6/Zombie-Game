// The block's place and its weather. Spec: docs/tier0/specs/42-place.md
#include "zs/place.h"

#include "zs/anomalies.h"

#include <algorithm>
#include <cmath>

#include "zs/calendar.h"
#include "zs/clock.h"

namespace zs {

namespace {
constexpr double kUnheatedGainC = 3.0;  // rule 5: internal gains of an unheated dwelling (placeholder estimate)
}

Place default_place() { return Place{}; }

void PlaceSystem::save(ArchiveWriter& a) const {
  a.u32(place_.cell);
  a.u32(static_cast<std::uint32_t>(place_.frequency));
  a.u64(place_.world_seed);
  a.f64(place_.lat_deg);
  a.f64(place_.lon_deg);
  a.f64(place_.elevation_m);
  a.str(place_.biome);
  a.f64(place_.climate.temp_mean_c);
  a.f64(place_.climate.temp_range_c);
  a.f64(place_.climate.rain_mm);
  a.boolean(place_.climate.south);
  a.f64(place_.climate.rain_kappa);  // spec 100
  a.f64(place_.climate.rain_peak);
  a.boolean(place_.climate.anomalies);  // spec 101 (the position is the place's own)
  a.f64(place_.climate.wind_mean_ms);
  a.f64(place_.climate.wind_from_deg);
  a.f64(place_.river_mean_m3s);  // spec 102
  for (double x : place_.river_regime) a.f64(x);
  a.f64(place_.town_people);  // spec 48
  a.i64(place_.sector);
  a.i64(place_.country);
  a.str(place_.income);
  a.str(place_.profile);  // spec 49
  a.str(place_.country_name);
}

void PlaceSystem::load(ArchiveReader& r) {
  place_.cell = r.u32();
  place_.frequency = static_cast<int>(r.u32());
  place_.world_seed = r.u64();
  place_.lat_deg = r.f64();
  place_.lon_deg = r.f64();
  place_.elevation_m = r.f64();
  place_.biome = r.str();
  place_.climate.temp_mean_c = r.f64();
  place_.climate.temp_range_c = r.f64();
  place_.climate.rain_mm = r.f64();
  place_.climate.south = r.boolean();
  place_.climate.rain_kappa = r.f64();  // spec 100
  place_.climate.rain_peak = r.f64();
  place_.climate.anomalies = r.boolean();  // spec 101
  place_.climate.wind_mean_ms = r.f64();
  place_.climate.wind_from_deg = r.f64();
  place_.river_mean_m3s = r.f64();  // spec 102
  for (double& x : place_.river_regime) x = r.f64();
  place_.climate.lat_deg = place_.lat_deg, place_.climate.lon_deg = place_.lon_deg;
  place_.town_people = r.f64();
  place_.sector = static_cast<int>(r.i64());
  place_.country = static_cast<int>(r.i64());
  place_.income = r.str();
  place_.profile = r.str();
  place_.country_name = r.str();
  cached_day_ = kNoDay;
}

namespace {
// Spec 53 rule 4: the weather reads civil time.
fields::TimeOfYear weather_now(const CivilTime& t) {
  return fields::weather_time(t.date.year, t.day_of_year, is_leap_year(t.date.year), t.hour);
}
}  // namespace

void PlaceSystem::refresh(const World& w) const {
  const CivilTime now = civil_now(w);
  if (now.day_number == cached_day_) return;
  const fields::TimeOfYear t = weather_now(now);
  mean_c_ = fields::daily_mean_c(place_.world_seed, place_.cell, place_.climate, t.year, t.day);
  rain_mm_ = fields::rain_day_mm(place_.world_seed, place_.cell, place_.climate, t.year, t.day);
  cached_day_ = now.day_number;
}

double PlaceSystem::outdoor_c(const World& w) const {
  return fields::temperature_c(place_.world_seed, place_.cell, place_.climate, weather_now(civil_now(w)));
}

double PlaceSystem::wind_ms(const World& w) const {
  return fields::wind_ms(place_.world_seed, place_.cell, place_.climate, weather_now(civil_now(w)));
}

double PlaceSystem::wind_from_deg(const World& w) const {
  return fields::wind_from_deg(place_.world_seed, place_.cell, place_.climate, weather_now(civil_now(w)));
}

namespace {
const fields::SnowDay& snow_on(const Place& p, std::int64_t& cached, std::vector<fields::SnowDay>& snow, const fields::TimeOfYear& t) {
  if (cached != t.year) snow = fields::snow_year(p.world_seed, p.cell, p.climate, t.year), cached = t.year;
  return snow[static_cast<std::size_t>(std::clamp(t.day, 0, fields::kDaysPerYear - 1))];
}
}  // namespace

double PlaceSystem::snow_water_mm(const World& w) const { return snow_on(place_, snow_year_, snow_, weather_now(civil_now(w))).swe_mm; }
double PlaceSystem::snow_depth_cm(const World& w) const { return snow_on(place_, snow_year_, snow_, weather_now(civil_now(w))).depth_cm; }
double PlaceSystem::frost_depth_cm(const World& w) const { return snow_on(place_, snow_year_, snow_, weather_now(civil_now(w))).frost_cm; }

double PlaceSystem::river_m3s(const World& w) const {
  if (place_.river_mean_m3s <= 0) return 0;
  // Spec 102 rule 6: the regime eased between mid-months, times the basin's rain anomaly of this month and the last.
  const fields::TimeOfYear t = weather_now(civil_now(w));
  const double pos = (t.day + 0.5) * 12.0 / fields::kDaysPerYear - 0.5;
  const auto m0 = static_cast<std::int64_t>(std::floor(pos));
  const double f = pos - static_cast<double>(m0);
  auto regime = [&](std::int64_t m) { return place_.river_regime[static_cast<std::size_t>(((m % 12) + 12) % 12)]; };
  const std::int64_t month = t.year * 12 + t.day * 12 / fields::kDaysPerYear;
  const double anomaly = 0.5 * (fields::rain_month_factor(place_.world_seed, place_.climate, month) +
                                fields::rain_month_factor(place_.world_seed, place_.climate, month - 1));
  return place_.river_mean_m3s * ((1 - f) * regime(m0) + f * regime(m0 + 1)) * anomaly;
}

double PlaceSystem::today_mean_c(const World& w) const {
  refresh(w);
  return mean_c_;
}

double PlaceSystem::today_rain_mm(const World& w) const {
  refresh(w);
  return rain_mm_;
}

double PlaceSystem::indoor_c(const World& w) const { return today_mean_c(w) + kUnheatedGainC; }

}  // namespace zs
