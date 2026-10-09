// Spec 42: the block's cell on a generated planet, and the one world built there.
#include <doctest.h>

#include "zs/gen/place.h"

using namespace zs::gen;

TEST_CASE("place: a temperate land cell, the same for the same seed; the one world stands on it") {
  const WorldGrid grid(kOneWorldFrequency, 6371.0);
  for (std::uint64_t seed : {1ull, 2ull, 7ull}) {
    WorldParams wp;
    wp.seed = seed;
    const WorldFields f = generate_fields(grid, wp);
    const CellId c = choose_place(grid, f, seed);
    CHECK(f.land(c));
    CHECK((f.biome[c] == Biome::TemperateForest || f.biome[c] == Biome::Grassland));
    CHECK(choose_place(grid, f, seed) == c);
    const zs::Place p = place_of(grid, f, c, seed);
    CHECK(p.lat_deg >= -90.0);
    CHECK(p.lat_deg <= 90.0);
    CHECK(p.climate.temp_mean_c == f.temp_mean_c[c]);
    CHECK(p.climate.south == (grid.centre(c).z < 0));
  }
  const auto t = make_world(1);  // spec 48: the cell is now its town's (test_world_people.cpp)
  REQUIRE(t.place);
  CHECK(t.place->place().frequency == kOneWorldFrequency);
}

