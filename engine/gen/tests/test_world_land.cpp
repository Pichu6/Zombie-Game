// Spec 52 slice 1: drainage to the sea, conserved water, an Earth-like runoff ratio and largest river, arable land and
// capacity in realistic bands, an oasis on a desert river, and a pinned golden.
#include <doctest.h>

#include <algorithm>
#include <cstring>

#include "zs/gen/keyed.h"
#include "zs/gen/world_land.h"

using namespace zs::gen;

namespace {

struct Planet {
  WorldGrid grid;
  WorldFields fields;
  WorldLand land;
};

const Planet& earthlike() {
  static const Planet p = [] {
    Planet q{WorldGrid(24, 6371.0), {}, {}};
    WorldParams wp;
    wp.seed = 1;
    q.fields = generate_fields(q.grid, wp);
    q.land = generate_land(q.grid, q.fields);
    return q;
  }();
  return p;
}

}  // namespace

TEST_CASE("land: every land cell drains to the sea and water is conserved (A1)") {
  const Planet& p = earthlike();
  for (CellId i = 0; i < p.grid.size(); ++i) {
    if (!p.fields.land(i)) continue;
    CellId c = i;
    std::size_t steps = 0;
    while (p.fields.land(c) && steps <= p.grid.size()) {
      c = p.land.downstream[c];
      REQUIRE(c != kNoCell);
      ++steps;
    }
    CHECK(!p.fields.land(c));
  }
  const double runoff_m3s = p.land.runoff_km3 * 1e9 / (365.25 * 86400);
  CHECK(p.land.sea_inflow_m3s == doctest::Approx(runoff_m3s).epsilon(1e-9));
}

TEST_CASE("land: runoff ratio and the largest river are Earth-like (A2)") {
  const Planet& p = earthlike();
  const double ratio = p.land.runoff_km3 / p.land.land_rain_km3;
  MESSAGE("runoff ", p.land.runoff_km3, " km3 of ", p.land.land_rain_km3, " km3 rain: ", ratio);
  CHECK(ratio >= 0.25);
  CHECK(ratio <= 0.45);
  const double largest = *std::max_element(p.land.discharge_m3s.begin(), p.land.discharge_m3s.end());
  MESSAGE("largest river ", largest, " m3/s");
  CHECK(largest >= 20000);
  CHECK(largest <= 400000);
  // Rivers are lines through the land, not the land itself. On cells about 300 km wide a river draining 100,000 km2
  // passes through many of them (most of Europe is that close to one); large rivers are fewer.
  int land_cells = 0, river_cells = 0, large_cells = 0;
  for (CellId i = 0; i < p.grid.size(); ++i)
    if (p.fields.land(i)) {
      ++land_cells;
      river_cells += p.land.river[i] ? 1 : 0;
      large_cells += p.land.river[i] == 2 ? 1 : 0;
    }
  MESSAGE(river_cells, " river cells (", large_cells, " large) of ", land_cells, " land cells");
  CHECK(large_cells > 0);
  CHECK(large_cells * 100 < 15 * land_cells);
  CHECK(river_cells * 2 < land_cells);
}

TEST_CASE("land: arable share and capacity in realistic bands (A2)") {
  const Planet& p = earthlike();
  double land_km2 = 0, arable_km2 = 0, capacity = 0;
  for (CellId i = 0; i < p.grid.size(); ++i) {
    if (!p.fields.land(i)) continue;
    land_km2 += p.grid.area_km2(i);
    arable_km2 += p.land.arable_share[i] * p.grid.area_km2(i);
    capacity += p.land.capacity(i);
  }
  const double people_today = 8091734930.0 * land_km2 / 148.94e6;  // spec 48 rule 2
  MESSAGE("arable ", arable_km2 / land_km2, ", capacity ", capacity, " against ", people_today, " people");
  CHECK(arable_km2 / land_km2 >= 0.10);
  CHECK(arable_km2 / land_km2 <= 0.35);
  CHECK(capacity >= 0.5 * people_today);
  CHECK(capacity <= 5.0 * people_today);
}

TEST_CASE("land: forests, game and fish against today's world (A2)") {
  const Planet& p = earthlike();
  double land_km2 = 0, forest_km2 = 0, wood = 0, game = 0, fish = 0;
  for (CellId i = 0; i < p.grid.size(); ++i) {
    fish += p.land.fish_catch_t[i];
    if (!p.fields.land(i)) continue;
    land_km2 += p.grid.area_km2(i);
    forest_km2 += p.land.forest_share[i] * p.grid.area_km2(i);
    wood += p.land.wood_m3[i];
    game += p.land.game_kg[i];
    CHECK(p.land.wood_r[i] <= 0.1);  // WSE-34
  }
  const double scale = land_km2 / 148.94e6;  // this planet's land against Earth's (WNC-1)
  MESSAGE("forest ", forest_km2 / land_km2, " of land; wood ", wood / 1e9, " billion m3; game ", game / 1e9, " Mt; fish ", fish / 1e6, " Mt/yr");
  CHECK(forest_km2 / land_km2 >= 0.25);  // WSE-1: 31 %
  CHECK(forest_km2 / land_km2 <= 0.40);
  CHECK(wood >= 0.6 * 557e9 * scale);  // RV-6
  CHECK(wood <= 1.6 * 557e9 * scale);
  CHECK(game >= 0.2 * 47e9 * scale);  // RV-10: land and sea together weigh 47 Mt
  CHECK(game <= 1.0 * 47e9 * scale);
  CHECK(fish >= 50e6);  // RV-11
  CHECK(fish <= 150e6);
}

TEST_CASE("land: a river through a desert feeds more people than the desert beside it (A1)") {
  const Planet& p = earthlike();
  int compared = 0;
  for (CellId i = 0; i < p.grid.size(); ++i) {
    if (!p.fields.land(i) || p.fields.biome[i] != Biome::Desert || !p.land.river[i]) continue;
    for (CellId nb : p.grid.neighbours(i))
      if (p.fields.land(nb) && p.fields.biome[nb] == Biome::Desert && !p.land.river[nb]) {
        CHECK(p.land.capacity(i) > p.land.capacity(nb));
        ++compared;
      }
  }
  MESSAGE(compared, " desert river cells compared");
}

TEST_CASE("land: deterministic, golden pinned (A1)") {
  const Planet& p = earthlike();
  std::uint64_t h = 0xCBF29CE484222325ull;
  auto mix_in = [&](double v) {
    std::uint64_t b = 0;
    std::memcpy(&b, &v, sizeof b);
    h = mix64(h, b);
  };
  for (CellId i = 0; i < p.grid.size(); ++i) {
    mix_in(p.land.runoff_mm[i]);
    mix_in(p.land.discharge_m3s[i]);
    mix_in(p.land.capacity(i));
    mix_in(p.land.wood_m3[i]);
    mix_in(p.land.game_kg[i]);
    mix_in(p.land.fish_catch_t[i]);
    h = mix64(h, p.land.downstream[i]);
  }
  MESSAGE("land hash: ", h);
  CHECK(h == 8347828370111059165ULL);  // pinned on MSVC, 2026-10-09
}
