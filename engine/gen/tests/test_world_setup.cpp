// Spec 84: the world's size, richness and people as settings; the preview's coarse grid agrees with the full world.
#include <doctest.h>

#include <cmath>

#include "zs/gen/place.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

double biome_share(const GeneratedWorld& w, Biome b) {
  double in = 0, land = 0;
  for (CellId c = 0; c < w.grid.size(); ++c) {
    if (!w.fields.land(c) || w.fields.biome[c] == Biome::Ice) continue;
    land += w.grid.area_km2(c);
    if (w.fields.biome[c] == b) in += w.grid.area_km2(c);
  }
  return land > 0 ? in / land : 0;
}

double land_share(const GeneratedWorld& w) {
  double land = 0;
  for (CellId c = 0; c < w.grid.size(); ++c)
    if (w.fields.land(c)) land += w.grid.area_km2(c);
  return land / w.grid.sphere_area_km2();
}

}  // namespace

TEST_CASE("setup: Earth's radius keeps the one world's grid (spec 84 rule 1)") {
  CHECK(world_frequency(6371.0) == kOneWorldFrequency);
  const int small = world_frequency(3400.0);
  const double ratio = static_cast<double>(WorldGrid::cell_count_for(small)) / static_cast<double>(WorldGrid::cell_count_for(kOneWorldFrequency));
  MESSAGE("radius 3,400 km: frequency " << small << ", " << ratio << " of Earth's cells");
  CHECK(std::abs(ratio - (3400.0 * 3400.0) / (6371.0 * 6371.0)) < 0.05 * (3400.0 * 3400.0) / (6371.0 * 6371.0));
}

TEST_CASE("setup: richness scales the same deposits; people scale with the setting (spec 84 rule 2)") {
  const int f = kTestFrequency;
  WorldSettings base, rich;
  rich.fuel_richness = 2.0;
  rich.metal_richness = 0.5;
  rich.people = 1.5;
  const GeneratedWorld a = generate_world(1, base, f), b = generate_world(1, rich, f);
  REQUIRE(a.deposits.deposits.size() == b.deposits.deposits.size());
  for (std::size_t i = 0; i < a.deposits.deposits.size(); ++i) {
    const Deposit& x = a.deposits.deposits[i];
    const Deposit& y = b.deposits.deposits[i];
    CHECK(x.cell == y.cell);
    CHECK(x.kind == y.kind);
    const bool fuel = x.kind == Resource::Coal || x.kind == Resource::Oil || x.kind == Resource::Gas;
    CHECK(y.amount == doctest::Approx(x.amount * (fuel ? 2.0 : 0.5)));
  }
  CHECK(b.people.total_people == doctest::Approx(1.5 * a.people.total_people).epsilon(0.01));
  CHECK(a.fields.elevation_m == b.fields.elevation_m);  // the planet itself is the same
}

TEST_CASE("setup: fertility, forest and fish move their own fields (spec 84 rule 2)") {
  WorldSettings s;
  s.fertility = 0.5;
  s.forest = 0.5;
  s.fish = 2.0;
  const GeneratedWorld a = generate_world(2, {}, kTestFrequency), b = generate_world(2, s, kTestFrequency);
  double fa = 0, fb = 0, wa = 0, wb = 0, ka = 0, kb = 0;
  for (CellId c = 0; c < a.grid.size(); ++c) {
    fa += a.land.fertility[c], fb += b.land.fertility[c];
    wa += a.land.forest_share[c], wb += b.land.forest_share[c];
    ka += a.land.fish_catch_t[c], kb += b.land.fish_catch_t[c];
  }
  CHECK(fb == doctest::Approx(0.5 * fa).epsilon(0.02));
  CHECK(wb == doctest::Approx(0.5 * wa).epsilon(0.02));
  CHECK(kb == doctest::Approx(2.0 * ka));
}

TEST_CASE("setup: a smaller planet has less land and fewer people (spec 84 rule 1)") {
  WorldSettings s;
  s.radius_km = 3400;
  const GeneratedWorld w = generate_world(1, s, world_frequency(3400.0, kPreviewCellKm2));
  const double expect = 0.29 * 4 * 3.14159265358979 * 3400.0 * 3400.0;
  CHECK(w.people.land_km2 == doctest::Approx(expect).epsilon(0.03));
  CHECK(w.people.total_people == doctest::Approx(8.09e9 * w.people.land_km2 / kEarthLandKm2).epsilon(0.05));
}

TEST_CASE("setup: the preview agrees with the full world (spec 84 Tests)" * doctest::skip(zs::World::kDebugBuild)) {
  for (std::uint64_t seed : {1ull, 2ull, 3ull}) {
    const GeneratedWorld full = generate_world(seed, {}), coarse = generate_world(seed, {}, world_frequency(6371.0, kPreviewCellKm2));
    MESSAGE("seed " << seed << ": land " << land_share(full) << " / " << land_share(coarse) << "; desert " << biome_share(full, Biome::Desert)
                    << " / " << biome_share(coarse, Biome::Desert) << "; tropical forest " << biome_share(full, Biome::TropicalForest)
                    << " / " << biome_share(coarse, Biome::TropicalForest));
    CHECK(std::abs(land_share(full) - land_share(coarse)) < 0.03);
    for (Biome b : {Biome::Desert, Biome::Savanna, Biome::TropicalForest, Biome::TemperateForest, Biome::BorealForest, Biome::Grassland})
      CHECK(std::abs(biome_share(full, b) - biome_share(coarse, b)) < 0.045);  // spec 89 second amendment: Koppen's
                                                                                   // classes follow the range, which
                                                                                   // follows distance inland in cells
  }
}
