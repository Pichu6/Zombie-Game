// Fields across levels: two operators, coarsen (an area-weighted mean of what is below) and refine (the coarse
// value plus keyed variation whose mean over the region is exactly the coarse value), applied to weather (a climate
// cell downscaled to a place and an hour) and to ambient noise (sources summed into an ambient; what a listener
// hears is the ambient plus nearby sources). Pure functions, keyed draws only, no state.
// Spec: docs/tier0/specs/36-fields-and-weather.md
#pragma once
#include <cstdint>
#include <span>
#include <vector>

namespace zs::fields {

// ---- Keyed draws (the same Philox the content tools use, so a field value is a pure function of its key).
double keyed_unit(std::uint64_t seed, std::uint64_t id, std::uint32_t field, std::uint32_t k);  // [0, 1)
double keyed_normal(std::uint64_t seed, std::uint64_t id, std::uint32_t field, std::uint32_t k);  // mean 0, sd 1

// Field numbers, append only (a key is (seed, id, field, k)).
enum Field : std::uint32_t { Variation = 1, DailyTemp = 2, RainWet = 3, RainAmount = 4,
                             RainAnom = 5, TempAnom = 6, WindSpeed = 7, WindTurn = 8 };  // 5-8: spec 101

// ---- The two operators.
// n keyed values with standard deviation about `amplitude` whose sum is exactly zero: refining a coarse value with
// them leaves the mean of the refined values equal to the coarse value (A1).
std::vector<double> zero_mean_variation(std::uint64_t seed, std::uint64_t id, std::uint32_t field, std::size_t n,
                                        double amplitude);
// Area-weighted mean (weights need not be normalised; all zero gives 0).
double coarsen(std::span<const double> values, std::span<const double> weights);

// ---- Weather: a climate cell (spec 31: annual mean, seasonal range, annual rain) at a place and a time.
inline constexpr int kDaysPerYear = 365;

struct Climate {
  double temp_mean_c;   // annual mean
  double temp_range_c;  // warmest minus coldest monthly mean
  double rain_mm;       // per year
  bool south = false;   // southern hemisphere: the seasons are shifted half a year
  // Spec 100: the rain's seasonal curve (von Mises strength, peak month 0-12 from 1 January); 0: no season (spec 36).
  double rain_kappa = 0, rain_peak = 0;
  // Spec 101: regional anomalies (off: every year the climate's own, spec 36) at the place's position, and its wind.
  bool anomalies = false;
  double lat_deg = 0, lon_deg = 0;
  double wind_mean_ms = 0, wind_from_deg = 0;  // the cell's mean speed at 10 m and its prevailing bearing
};
// Spec 100: the share of the year's rain in month m (0 = January), adding to 1.
double rain_month_share(const Climate& c, int month);

struct TimeOfYear {
  std::int64_t year;
  int day;      // 0..364
  double hour;  // 0..24
};
// Spec 53 rule 4: the weather's time for a civil date and hour (calendar.h gives them). A weather year is 365 days: in a
// leap year 29 February repeats 28 February (weather day 58) and later days shift back by one.
TimeOfYear weather_time(int year, int day_of_year, bool leap_year, double hour);

// Daily mean temperature for a day: seasonal cosine plus a keyed anomaly; the 365 anomalies of a year sum to zero,
// so the year's mean of daily means equals temp_mean_c exactly (A1). Anomalies are smoothed over neighbouring days
// (weather has memory) and scaled by the seasonal range.
double daily_mean_c(std::uint64_t seed, std::uint64_t cell, const Climate& c, std::int64_t year, int day);
// Temperature at an hour: the daily mean plus a diurnal cosine (coldest about 03:00, warmest about 15:00), whose
// amplitude is a share of the seasonal range (the dry-continental swing is wider; a placeholder).
double temperature_c(std::uint64_t seed, std::uint64_t cell, const Climate& c, const TimeOfYear& t);

// Rain on a day: keyed wet days (about a third, wetter cells wetter) with keyed amounts, the whole year normalised
// so the 365 days sum to rain_mm exactly (A1). Zero if the climate has no rain.
double rain_day_mm(std::uint64_t seed, std::uint64_t cell, const Climate& c, std::int64_t year, int day);
// Rain between two ticks of the weather's own timeline (tick 0 is midnight of weather day 0 of weather year 0; it is
// not the world's clock): each day's rain falls evenly through the day, so any partition sums to the same total.
double rain_between_mm(std::uint64_t seed, std::uint64_t cell, const Climate& c, std::int64_t from_tick, std::int64_t to_tick);

// ---- Ambient noise: sources at a distance summed by energy into one level (spec 18 rule 1 with spherical spreading).
struct NoiseSource {
  double level_db_1m;
  double distance_m;  // at least 1
};
inline constexpr double kSilenceDb = 20;  // the floor: a still night in the countryside
double ambient_db(std::span<const NoiseSource> sources, double floor_db = kSilenceDb);
// How far a source is heard above the ambient with a hearing margin (spec 18 rule 2).
double audible_range_m(double level_db_1m, double ambient_db, double threshold_db = 3);

}  // namespace zs::fields
