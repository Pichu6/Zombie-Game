// The block's place and its weather. Spec: docs/tier0/specs/42-place.md
#include "zs/place.h"

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
  cached_day_ = -1;
}

void PlaceSystem::refresh(std::int64_t tick) const {
  const std::int64_t day = tick / kTicksPerGameDay;
  if (day == cached_day_) return;
  const fields::TimeOfYear t = fields::time_of_year(tick);
  mean_c_ = fields::daily_mean_c(place_.world_seed, place_.cell, place_.climate, t.year, t.day);
  rain_mm_ = fields::rain_day_mm(place_.world_seed, place_.cell, place_.climate, t.year, t.day);
  cached_day_ = day;
}

double PlaceSystem::outdoor_c(const World& w) const {
  return fields::temperature_c(place_.world_seed, place_.cell, place_.climate, w.clock.tick);
}

double PlaceSystem::today_mean_c(const World& w) const {
  refresh(w.clock.tick);
  return mean_c_;
}

double PlaceSystem::today_rain_mm(const World& w) const {
  refresh(w.clock.tick);
  return rain_mm_;
}

double PlaceSystem::indoor_c(const World& w) const { return today_mean_c(w) + kUnheatedGainC; }

}  // namespace zs
