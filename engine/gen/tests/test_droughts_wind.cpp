// Spec 101: regional anomalies (droughts, hard winters) and wind, against WorldClim / CRU TS on Earth (DW).
#include <doctest.h>

#include <cmath>
#include <vector>

#include "zs/anomalies.h"
#include "zs/fields.h"
#include "zs/gen/place.h"
#include "zs/gen/world_seasons.h"
#include "zs/world.h"

using namespace zs::fields;

namespace {
Climate climate(double rain, double lat, double lon) {
  Climate c{12, 20, rain, lat < 0};
  c.anomalies = true;
  c.lat_deg = lat, c.lon_deg = lon;
  return c;
}
}  // namespace

TEST_CASE("droughts: the coherent field is about standard normal, alike nearby and unrelated far away (rule 1)") {
  double s = 0, s2 = 0, near = 0, far = 0;
  const int n = 4000;
  for (int k = 0; k < n; ++k) {
    const double lat = -60 + 120.0 * k / n, lon = std::fmod(k * 37.0, 360.0) - 180;
    const double a = coherent_normal(9, Field::RainAnom, lat, lon, 850, k % 50);
    s += a, s2 += a * a;
    near += a * coherent_normal(9, Field::RainAnom, lat + 0.3, lon, 850, k % 50);   // about 33 km away
    far += a * coherent_normal(9, Field::RainAnom, lat, lon + 60, 850, k % 50);     // thousands of km away
  }
  const double mean = s / n, var = s2 / n - mean * mean;
  CHECK(std::abs(mean) < 0.1);
  CHECK(var > 0.7);
  CHECK(var < 1.3);
  CHECK(near / n > 0.8 * var);
  CHECK(std::abs(far / n) < 0.15);
}

TEST_CASE("droughts: a month's factor averages one; the year's rain varies as Earth's does at its mean (rule 2, DW-1)") {
  for (const double rain : {150.0, 600.0, 2000.0}) {
    const Climate c = climate(rain, 35, 20);
    double sum = 0, sumsq = 0;
    const int years = 400;
    for (int y = 0; y < years; ++y) {
      double t = 0;
      for (int m = 0; m < 12; ++m) t += rain_month_share(c, m) * rain_month_factor(3, c, y * 12 + m);
      sum += t, sumsq += t * t;
    }
    const double mean = sum / years, cv = std::sqrt(sumsq / years - mean * mean) / mean;
    CAPTURE(rain);
    CHECK(mean == doctest::Approx(1.0).epsilon(0.05));
    CHECK(cv == doctest::Approx(rain_cv_target(rain)).epsilon(0.25));
  }
}

TEST_CASE("droughts: the block's rain keeps each month exact to its anomaly; temperatures follow the month's anomaly (rules 2, 3)") {
  Climate c = climate(900, 45, 10);
  c.rain_kappa = 1.0, c.rain_peak = 6.8;
  const std::int64_t year = 2027;
  double year_sum = 0;
  for (int m = 0; m < 12; ++m) {
    double month = 0;
    for (int d = m * kDaysPerYear / 12; d < (m + 1) * kDaysPerYear / 12; ++d) month += rain_day_mm(5, 77, c, year, d);
    CHECK(month == doctest::Approx(900 * rain_month_share(c, m) * rain_month_factor(5, c, year * 12 + m)).epsilon(1e-9));
    year_sum += month;
  }
  CHECK(year_sum > 0);
  // A month's mean is the climate's seasonal mean plus about its regional anomaly (the daily weather sums to zero in it;
  // the anomaly eases between mid-months, so allow for half a neighbour's difference).
  for (int m : {0, 3, 6, 9}) {
    double mean = 0, seasonal = 0;
    const int d0 = m * kDaysPerYear / 12, d1 = (m + 1) * kDaysPerYear / 12;
    for (int d = d0; d < d1; ++d) {
      mean += daily_mean_c(5, 77, c, year, d) / (d1 - d0);
      seasonal += (12 - 0.5 * 20 * std::cos(2 * 3.14159265358979 * (d - 20.0) / kDaysPerYear)) / (d1 - d0);
    }
    CAPTURE(m);
    CHECK(std::abs(mean - seasonal - temp_month_anomaly_c(5, c, year * 12 + m)) < 1.5);
  }
  CHECK(temp_anomaly_sd(65, 0) > 2 * temp_anomaly_sd(5, 0));  // DW-4: high-latitude winters vary most
  CHECK(temp_anomaly_sd(50, 0) > temp_anomaly_sd(50, 6));     // winter more than summer in the north
}

TEST_CASE("wind: around the cell's mean, windier in winter, from about the prevailing side (rule 4)") {
  Climate c = climate(600, 50, 0);
  c.wind_mean_ms = 4.0, c.wind_from_deg = prevailing_from_deg(50);
  CHECK(prevailing_from_deg(50) == 225);  // the westerlies, from the south-west
  CHECK(prevailing_from_deg(15) == 45);   // the north-east trades
  double sum = 0, jan = 0, jul = 0, near = 0;
  for (int d = 0; d < kDaysPerYear; ++d)
    for (int h = 0; h < 24; h += 6) {
      const TimeOfYear t{2027, d, static_cast<double>(h)};
      const double w = wind_ms(1, 5, c, t);
      CHECK(w >= 0);
      sum += w;
      if (d < 31) jan += w;
      if (d >= 181 && d < 212) jul += w;
      const double dir = wind_from_deg(1, 5, c, t);
      near += std::abs(std::remainder(dir - 225, 360.0)) < 90;
    }
  CHECK(sum / (kDaysPerYear * 4) == doctest::Approx(4.0).epsilon(0.15));
  CHECK(jan > jul);
  CHECK(near / (kDaysPerYear * 4) > 0.9);
}

TEST_CASE("wind: the generated land's mean speed near Earth's, and the one world's place has anomalies and wind (rule 4, 5)") {
  const zs::gen::GeneratedWorld w = zs::gen::generate_world(1, {}, 48);
  double s = 0, a = 0;
  for (zs::gen::CellId i = 0; i < w.grid.size(); ++i)
    if (w.fields.land(i)) s += w.grid.area_km2(i) * w.fields.wind_ms[i], a += w.grid.area_km2(i);
  CHECK(s / a > 2.0);  // DW-6: 2.87 m/s over Earth's land
  CHECK(s / a < 4.0);
  const auto t = zs::gen::make_world(1);
  REQUIRE(t.place);
  const Climate& c = t.place->place().climate;
  CHECK(c.anomalies);
  CHECK(c.wind_mean_ms > 0);
  CHECK(t.place->wind_ms(*t.world) >= 0);
}
