// Spec 103: the living stocks of the block's town's region from the generated world, the run's system, and the world's
// checks against Earth (LS-2, LS-3, LS-4).
#include <doctest.h>

#include <cmath>

#include "zs/gen/region_stocks.h"

using namespace zs;
using namespace zs::gen;

TEST_CASE("living region: the town's region has game, wood, pasture and crops at their equilibrium; the world checks") {
  const GeneratedWorld world = generate_world(1, WorldSettings{});
  const int town = choose_town(world.fields, world.people, 1);
  REQUIRE(town >= 0);
  const LivingSetup setup = region_stocks(world, town, 1);
  std::array<int, kLivingKinds> kinds{};
  for (const LivingStock& s : setup.stocks) {
    ++kinds[static_cast<std::size_t>(s.kind)];
    CHECK(s.peak > 0);
    // Rule 6: game and wood keep spec 52's stock of today as their equilibrium's mean.
    if (s.kind == LivingKind::Game) CHECK(std::abs(living::equilibrium_mean(s) - world.land.game_kg[s.cell]) <= 1e-6 * world.land.game_kg[s.cell]);
    if (s.kind == LivingKind::Wood)
      CHECK(std::abs(living::equilibrium_mean(s) - 1000.0 * world.land.wood_m3[s.cell]) <= 1e-6 * 1000.0 * world.land.wood_m3[s.cell]);
  }
  MESSAGE("region cells " << region_cells(world, town).size() << ", stocks game " << kinds[0] << " wood " << kinds[1] << " pasture "
                          << kinds[2] << " crops " << kinds[3] << " fish " << kinds[4]);
  CHECK(kinds[static_cast<std::size_t>(LivingKind::Game)] > 0);
  CHECK(kinds[static_cast<std::size_t>(LivingKind::Wood)] > 0);
  CHECK(kinds[static_cast<std::size_t>(LivingKind::Pasture)] > 0);
  CHECK(kinds[static_cast<std::size_t>(LivingKind::Crops)] > 0);

  const LivingWorldChecks c = living_world_checks(world, 1);
  const double removed = c.wood_removed_m3 / c.wood_stock_m3, overfished = static_cast<double>(c.fish_overfished) / c.fish_cells,
               eaten = c.grass_eaten_kg / c.grass_peak_kg;
  MESSAGE("wood removed " << 100 * removed << " % a year, overfished " << 100 * overfished << " %, grass eaten " << 100 * eaten << " %");
  CHECK(removed > 0.003);
  CHECK(removed < 0.015);
  CHECK(overfished > 0.30);
  CHECK(overfished < 0.45);
  CHECK(eaten > 0.15);
  CHECK(eaten < 0.35);
}

TEST_CASE("living region: the one world runs its town's region's stocks") {
  const auto t = make_world(1);
  REQUIRE(t.living);
  CHECK(t.living->size() > 0);
  std::int64_t held = 0;
  for (std::size_t i = 0; i < t.living->size(); ++i) held += t.living->amount(*t.world, i);
  CHECK(held > 0);
  t.living->advance(*t.world, 40 * kTicksPerGameDay);  // the stocks alone, not the block's every tick
  std::int64_t taken = 0;
  for (int k = 0; k < kLivingKinds; ++k) taken += t.living->taken(*t.world, static_cast<LivingKind>(k));
  CHECK(taken > 0);  // a month's start has passed: hunting, fishing, logging or grazing took their share
  CHECK(!t.world->ledger.check().has_value());
}
