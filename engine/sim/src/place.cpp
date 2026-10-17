// The block's place and its weather. Spec: docs/tier0/specs/42-place.md
#include "zs/place.h"

#include "zs/anomalies.h"

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
