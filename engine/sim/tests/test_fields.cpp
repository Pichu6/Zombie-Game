// Spec 36: coarsen and refine, weather from a climate cell, ambient noise and the audible range.
#include <doctest.h>
#include "property.h"

#include <cmath>
#include <vector>

#include "zs/clock.h"
#include "zs/fields.h"

using namespace zs;
using namespace zs::fields;

TEST_CASE("refine then coarsen is the identity: zero-mean variation leaves the coarse value exact (A1)") {
  property("round trip", [](std::uint32_t seed, std::uint32_t id, std::uint8_t n_raw, double coarse_raw) {
    const std::size_t n = 1 + n_raw % 64;
    const double coarse = std::fmod(coarse_raw, 1000.0);
    const std::vector<double> v = zero_mean_variation(seed, id, Field::Variation, n, 3.0);
    std::vector<double> refined(n), weights(n, 1.0);
    double sum = 0;
    for (std::size_t i = 0; i < n; ++i) { refined[i] = coarse + v[i]; sum += v[i]; }
    RC_ASSERT(std::fabs(sum) < 1e-9 * n);
    RC_ASSERT(std::fabs(coarsen(refined, weights) - coarse) < 1e-9);
  });
  // Deterministic and keyed: the same key gives the same values, another key other values; the spread is about the amplitude.
  const auto a = zero_mean_variation(7, 42, Field::Variation, 2000, 3.0);
  const auto b = zero_mean_variation(7, 42, Field::Variation, 2000, 3.0);
  const auto c = zero_mean_variation(7, 43, Field::Variation, 2000, 3.0);
  CHECK(a == b);
  CHECK(a != c);
  double ss = 0;
  for (double x : a) ss += x * x;
  const double sd = std::sqrt(ss / a.size());
  CHECK(sd > 2.7);
  CHECK(sd < 3.3);
  // Weighted coarsen: the bigger cell counts more.
  const double vals[] = {10.0, 20.0};
  const double w[] = {3.0, 1.0};
  CHECK(coarsen(vals, w) == doctest::Approx(12.5));
}

TEST_CASE("weather: a year's daily means average to the climate's mean exactly; seasons and hemispheres (A1, A3)") {
  const Climate temperate{10.0, 20.0, 800.0, false};
  for (std::uint64_t cell : {1u, 2u, 3u}) {
    double sum = 0;
    for (int d = 0; d < kDaysPerYear; ++d) sum += daily_mean_c(5, cell, temperate, 0, d);
    CHECK(sum / kDaysPerYear == doctest::Approx(10.0).epsilon(1e-9));
  }
  // January colder than July in the north by about the range; the reverse in the south.
  auto month_mean = [](const Climate& c, std::uint64_t cell, int first_day) {
    double s = 0;
    for (int d = first_day; d < first_day + 30; ++d) s += daily_mean_c(5, cell, c, 0, d);
    return s / 30;
  };
  const double jan = month_mean(temperate, 1, 5), jul = month_mean(temperate, 1, 186);
  CHECK(jul - jan > 12.0);  // the range (20) with the noise of two 30-day means on top
  CHECK(jul - jan < 28.0);
  const Climate south{10.0, 20.0, 800.0, true};
  CHECK(month_mean(south, 1, 5) - month_mean(south, 1, 186) > 14.0);
  // Same key, same weather; another year differs; another cell differs.
  CHECK(daily_mean_c(5, 1, temperate, 3, 100) == daily_mean_c(5, 1, temperate, 3, 100));
  CHECK(daily_mean_c(5, 1, temperate, 3, 100) != daily_mean_c(5, 1, temperate, 4, 100));
  CHECK(daily_mean_c(5, 1, temperate, 3, 100) != daily_mean_c(5, 2, temperate, 3, 100));
  // Weather has memory: neighbouring days are closer than days a fortnight apart, on average.
  double near = 0, far = 0;
  for (int d = 20; d < 300; ++d) {
    near += std::fabs(daily_mean_c(5, 1, temperate, 0, d) - daily_mean_c(5, 1, temperate, 0, d + 1));
    far += std::fabs(daily_mean_c(5, 1, temperate, 0, d) - daily_mean_c(5, 1, temperate, 0, d + 14));
  }
  CHECK(near < far);
  // The hour: warmest mid-afternoon, coldest small hours, around the daily mean.
  const std::int64_t day = 100 * kTicksPerGameDay;
  const double t03 = temperature_c(5, 1, temperate, day + 3 * kTicksPerGameHour);
  const double t15 = temperature_c(5, 1, temperate, day + 15 * kTicksPerGameHour);
  const double mean = daily_mean_c(5, 1, temperate, 0, 100);
  CHECK(t15 > mean);
  CHECK(t03 < mean);
  CHECK(t15 - t03 == doctest::Approx(2 * (0.15 * 20.0 + 2.0)));
}

TEST_CASE("rain: a year's days sum to the annual total exactly; wetter cells rain oftener; between ticks is partition-exact (A1)") {
  const Climate desert{25.0, 15.0, 100.0, false};
  const Climate rainforest{26.0, 3.0, 2500.0, false};
  const Climate dry{20.0, 10.0, 0.0, false};
  for (const Climate* c : {&desert, &rainforest}) {
    for (std::uint64_t cell : {11u, 12u}) {
      double sum = 0;
      int wet = 0;
      for (int d = 0; d < kDaysPerYear; ++d) {
        const double r = rain_day_mm(9, cell, *c, 1, d);
        CHECK(r >= 0);
        sum += r;
        if (r > 0) ++wet;
      }
      CHECK(sum == doctest::Approx(c->rain_mm).epsilon(1e-9));
      if (c == &desert) { CHECK(wet > 25); CHECK(wet < 90); }
      else { CHECK(wet > 150); CHECK(wet < 230); }
    }
  }
  CHECK(rain_day_mm(9, 11, dry, 1, 50) == 0);
  property("partition", [](std::vector<std::uint16_t> steps) {
    const Climate c{12.0, 18.0, 900.0, false};
    std::int64_t t = 3 * kTicksPerGameDay + 17;
    const std::int64_t start = t;
    double pieces = 0;
    for (std::uint16_t s : steps) {
      const std::int64_t next = t + static_cast<std::int64_t>(s) * 7 + 1;
      pieces += rain_between_mm(9, 11, c, t, next);
      t = next;
    }
    const double whole = rain_between_mm(9, 11, c, start, t);
    RC_ASSERT(std::fabs(pieces - whole) <= 1e-9 * (1 + whole));
  });
  // A whole year between ticks is the annual total; the second year differs day by day but sums the same.
  const Climate c{12.0, 18.0, 900.0, false};
  CHECK(rain_between_mm(9, 11, c, 0, kDaysPerYear * kTicksPerGameDay) == doctest::Approx(900.0).epsilon(1e-9));
  CHECK(rain_between_mm(9, 11, c, kDaysPerYear * kTicksPerGameDay, 2 * kDaysPerYear * kTicksPerGameDay) == doctest::Approx(900.0).epsilon(1e-9));
  CHECK(rain_day_mm(9, 11, c, 0, 40) != rain_day_mm(9, 11, c, 1, 40));
}

TEST_CASE("ambient noise: sources sum by energy; when the grid dies the ambient drops and a voice carries ten times farther (A1)") {
  // A district at night: traffic hum and a hundred fridges far away, a generator next door.
  const NoiseSource grid_on[] = {{70.0, 300.0}, {60.0, 100.0}, {89.0, 10.0}};
  const NoiseSource grid_off[] = {{89.0, 10.0}};
  const double on = ambient_db(grid_on), off = ambient_db(grid_off), quiet = ambient_db({});
  CHECK(quiet == doctest::Approx(kSilenceDb));
  CHECK(on > off);
  CHECK(off > quiet);
  // One source alone at 1 m is its own level (plus a hair of floor); two equal sources add 3 dB.
  const NoiseSource one[] = {{60.0, 1.0}};
  const NoiseSource two[] = {{60.0, 1.0}, {60.0, 1.0}};
  CHECK(ambient_db(one, 0.0) == doctest::Approx(60.0).epsilon(1e-3));
  CHECK(ambient_db(two, 0.0) - ambient_db(one, 0.0) == doctest::Approx(10.0 * std::log10(2.0)).epsilon(1e-3));
  // Spherical spreading: 20 dB less at ten times the distance, so a 20 dB quieter ambient is a tenfold range.
  CHECK(audible_range_m(60.0, 20.0) == doctest::Approx(10.0 * audible_range_m(60.0, 40.0)));
  CHECK(audible_range_m(60.0, 23.0) == doctest::Approx(10.0 * audible_range_m(60.0, 43.0)));
  CHECK(audible_range_m(30.0, 50.0) == 0);  // below the ambient: not heard at any distance
  // The design's claim (world simulation §9): a city ambient of about 50 dB at night falling to about 30 when the
  // grid dies lifts a voice's range tenfold; an engine at 89 dB carries from about 90 m to about 900 m.
  CHECK(audible_range_m(60.0, 30.0) / audible_range_m(60.0, 50.0) == doctest::Approx(10.0));
  CHECK(audible_range_m(89.0, 50.0) == doctest::Approx(63.1).epsilon(0.01));
  CHECK(audible_range_m(89.0, 30.0) == doctest::Approx(631.0).epsilon(0.01));
}
