// Spec 102: snow, frozen ground and the rivers' seasonal regimes (research snow-and-rivers, SR).
#include <doctest.h>

#include <algorithm>
#include <cmath>
#include <numeric>

#include "zs/anomalies.h"
#include "zs/gen/place.h"
#include "zs/gen/world_regimes.h"
#include "zs/gen/world_seasons.h"
#include "zs/world.h"

using namespace zs::gen;

TEST_CASE("snow: half and half at 1 C, all snow at -1 and below, none at 3 and above (rule 1, SR-1)") {
  CHECK(snow_share(-5) == 1);
  CHECK(snow_share(1) == doctest::Approx(0.5));
  CHECK(snow_share(4) == 0);
  CHECK(zs::fields::snow_share(1) == doctest::Approx(0.5));  // the block's rule is the same
}

TEST_CASE("snow: a generated world's cold land holds snow in winter and none in summer; regimes average one (rules 2-4)") {
  const GeneratedWorld w = generate_world(2, {}, 48);
  int cold = 0, snowy = 0, summer_snow = 0;
  for (CellId i = 0; i < w.grid.size(); ++i) {
    if (!w.fields.land(i)) continue;
    const auto& r = w.land.regime[i];
    CHECK(std::accumulate(r.begin(), r.end(), 0.0) == doctest::Approx(12).epsilon(1e-4));
    const auto T = temp_months_c(w.grid, w.fields, i);
    const bool north = w.grid.sin_lat(i) >= 0;
    const double winter_t = north ? T[0] : T[6];
    if (w.fields.biome[i] == Biome::Ice || winter_t > -8 || w.fields.rain_mm[i] < 200) continue;
    ++cold;
    snowy += w.land.snow_mm[i][north ? 0 : 6] >= 10;
    summer_snow += w.land.snow_mm[i][north ? 7 : 1] >= 10 && (north ? T[7] : T[1]) > 10;
  }
  REQUIRE(cold > 0);
  CHECK(snowy > 0.9 * cold);
  CHECK(summer_snow == 0);
}

TEST_CASE("rivers: big rivers have seasons, most peaking in the warm half (rule 4, SR-4)" * doctest::skip(zs::World::kDebugBuild)) {
  const GeneratedWorld w = generate_world(1, {});
  int big = 0, warm = 0;
  for (CellId i = 0; i < w.grid.size(); ++i) {
    if (!w.fields.land(i)) continue;
    const CellId d = w.land.downstream[i];
    if (!(d == kNoCell || !w.fields.land(d)) || w.land.discharge_m3s[i] < 1000) continue;
    const auto& r = w.land.regime[i];
    const auto hi = std::max_element(r.begin(), r.end());
    const int peak = static_cast<int>(hi - r.begin());
    const double ratio = *hi / std::max(0.01f, *std::min_element(r.begin(), r.end()));
    CHECK(ratio >= 1.05);   // every river rises and falls
    CHECK(ratio <= 200);
    ++big;
    warm += w.grid.sin_lat(i) < 0 ? (peak <= 2 || peak >= 8) : (peak >= 2 && peak <= 8);
  }
  REQUIRE(big > 50);
  CHECK(warm > 0.7 * big);
}

TEST_CASE("snow: the block's snow builds in a cold winter and melts; the ground freezes under thin snow (rules 5, 6)") {
  zs::fields::Climate c{0, 30, 500, false};  // a continental interior: -15 C in January, +15 in July
  c.anomalies = false;
  const auto year = zs::fields::snow_year(4, 9, c, 2027);
  REQUIRE(year.size() == zs::fields::kDaysPerYear);
  CHECK(year[40].swe_mm > 20);       // February
  CHECK(year[40].depth_cm > 5);
  CHECK(year[200].swe_mm == 0);      // July
  CHECK(year[60].frost_cm > 10);     // March: frozen ground
  CHECK(year[200].frost_cm == 0);
  zs::fields::Climate warm{18, 10, 900, false};
  for (const auto& d : zs::fields::snow_year(4, 9, warm, 2027)) CHECK(d.swe_mm == 0);
}

TEST_CASE("rivers: the one world's place carries its river and the day's flow follows its regime (rule 6)") {
  const auto t = make_world(1);
  REQUIRE(t.place);
  const zs::Place& p = t.place->place();
  double sum = 0;
  for (double x : p.river_regime) sum += x;
  CHECK(sum == doctest::Approx(12).epsilon(1e-4));
  CHECK(t.place->river_m3s(*t.world) >= 0);
  if (p.river_mean_m3s > 0) CHECK(t.place->river_m3s(*t.world) > 0);
  CHECK(t.place->snow_depth_cm(*t.world) >= 0);
}
