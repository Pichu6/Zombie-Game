// Spec 100: monthly normals and rainy seasons, against WorldClim 2.1 measured on Earth (research rain-seasons, RS).
#include <doctest.h>

#include <array>
#include <cmath>
#include <map>

#include "zs/fields.h"
#include "zs/gen/place.h"
#include "zs/gen/world_seasons.h"
#include "zs/world.h"

using namespace zs::gen;

TEST_CASE("seasons: Peel et al.'s rules classify their own example station (rule 4)") {
  // Herberton Post Office, Australia (Peel et al. 2007, Table 2): Cw, not Cs.
  const std::array<double, 12> p{238.4, 229.7, 214.4, 86.0, 46.9, 33.3, 22.0, 18.2, 16.5, 25.3, 77.3, 137.9};
  const std::array<double, 12> t{23.2, 22.8, 21.9, 20.0, 17.9, 16.0, 15.5, 16.5, 18.6, 21.0, 22.4, 23.3};
  CHECK(koppen_class(p, t, true) == "Cw");
  std::array<double, 12> wet{}, hot{};
  wet.fill(250), hot.fill(27);
  CHECK(koppen_class(wet, hot, false) == "Af");
  std::array<double, 12> dry{};
  dry.fill(2);
  CHECK(koppen_class(dry, hot, false) == "BW");
}

TEST_CASE("seasons: the months add to the year; tropics rain in summer, subtropical west sides in winter (rules 1-3)") {
  const GeneratedWorld w = generate_world(1, {}, 48);
  int tropic = 0, tropic_summer = 0, winter = 0;
  for (CellId i = 0; i < w.grid.size(); ++i) {
    if (!w.fields.land(i)) {
      CHECK(w.fields.rain_kappa[i] == 0);
      continue;
    }
    const auto m = rain_months_mm(w.fields, i);
    double sum = 0;
    for (double x : m) sum += x;
    CHECK(sum == doctest::Approx(w.fields.rain_mm[i]).epsilon(1e-9));
    const double lat = std::asin(w.grid.sin_lat(i)) * 57.29578;
    const bool south = lat < 0;
    double summer = 0;
    for (int k = 0; k < 12; ++k) summer += (south ? (k <= 2 || k >= 9) : (k >= 3 && k <= 8)) ? m[static_cast<std::size_t>(k)] : 0.0;
    if (std::abs(lat) > 10 && std::abs(lat) < 20) tropic++, tropic_summer += summer > 0.6 * w.fields.rain_mm[i];
    if (std::abs(lat) > 32 && std::abs(lat) < 42 && east_index(w.fields, i) < 0.2) winter += summer < 0.45 * w.fields.rain_mm[i];
  }
  REQUIRE(tropic > 0);
  CHECK(tropic_summer > 0.9 * tropic);
  CHECK(winter > 0);
}

TEST_CASE("seasons: rain by latitude and Koppen's second letters near Earth's (RS-1, RS-2)" * doctest::skip(zs::World::kDebugBuild)) {
  // Earth (WorldClim 2.1, rain-weighted): the share of rain in the summer half, bands from -30 to +60 by 10 degrees.
  static const double earth[9] = {0.71, 0.83, 0.61, 0.63, 0.79, 0.78, 0.56, 0.58, 0.60};
  std::array<double, 9> mean{};
  std::map<std::string, double> kg;
  for (std::uint64_t seed = 1; seed <= 3; ++seed) {
    const GeneratedWorld w = generate_world(seed, {});
    std::array<double, 9> rain{}, summer{};
    for (CellId i = 0; i < w.grid.size(); ++i) {
      if (!w.fields.land(i)) continue;
      const auto m = rain_months_mm(w.fields, i);
      const bool south = w.grid.sin_lat(i) < 0;
      kg[koppen_class(m, temp_months_c(w.grid, w.fields, i), south)] += w.grid.area_km2(i);
      const int b = static_cast<int>(std::floor((std::asin(w.grid.sin_lat(i)) * 57.29578 + 30) / 10));
      if (b < 0 || b >= 9 || w.fields.rain_mm[i] <= 50) continue;
      double s = 0;
      for (int k = 0; k < 12; ++k) s += (south ? (k <= 2 || k >= 9) : (k >= 3 && k <= 8)) ? m[static_cast<std::size_t>(k)] : 0.0;
      rain[static_cast<std::size_t>(b)] += w.grid.area_km2(i) * w.fields.rain_mm[i];
      summer[static_cast<std::size_t>(b)] += w.grid.area_km2(i) * s;
    }
    for (std::size_t b = 0; b < 9; ++b) mean[b] += rain[b] > 0 ? summer[b] / rain[b] / 3 : earth[b] / 3;
  }
  for (std::size_t b = 0; b < 9; ++b) {
    CAPTURE(b);
    CHECK(std::abs(mean[b] - earth[b]) < 0.14);
  }
  // Inside the temperate group: Earth Cf 53 %, Cs 15 %, Cw 33 % (RS-1).
  const double c = kg["Cf"] + kg["Cs"] + kg["Cw"];
  REQUIRE(c > 0);
  MESSAGE("temperate: Cf ", kg["Cf"] / c, " Cs ", kg["Cs"] / c, " Cw ", kg["Cw"] / c);
  CHECK(kg["Cs"] / c > 0.08);
  CHECK(kg["Cs"] / c < 0.30);
  CHECK(kg["Cw"] / c > 0.15);
  CHECK(kg["Cw"] / c < 0.50);
  // Inside the tropical group: Earth Af 26 %, Am 16 %, Aw 57 %.
  const double a = kg["Af"] + kg["Am"] + kg["Aw"];
  MESSAGE("tropical: Af ", kg["Af"] / a, " Am ", kg["Am"] / a, " Aw ", kg["Aw"] / a);
  CHECK(kg["Aw"] / a > 0.3);
  CHECK(kg["Af"] / a < 0.5);
}

TEST_CASE("seasons: the block's daily rain follows its months, each month exact (rule 5, A1)") {
  zs::fields::Climate c{20, 8, 1200, false};
  c.rain_kappa = 2.0;
  c.rain_peak = 6.8;  // a wet summer
  double month[12] = {}, year = 0;
  for (int m = 0; m < 12; ++m)  // the code's months: days m x 365 / 12 to (m + 1) x 365 / 12
    for (int d = m * zs::fields::kDaysPerYear / 12; d < (m + 1) * zs::fields::kDaysPerYear / 12; ++d) {
      const double r = zs::fields::rain_day_mm(7, 42, c, 3, d);
      CHECK(r >= 0);
      month[m] += r;
      year += r;
    }
  CHECK(year == doctest::Approx(1200).epsilon(1e-9));
  for (int m = 0; m < 12; ++m) CHECK(month[m] == doctest::Approx(1200 * zs::fields::rain_month_share(c, m)).epsilon(1e-9));
  CHECK(month[6] > 4 * month[0]);  // July against January
}
