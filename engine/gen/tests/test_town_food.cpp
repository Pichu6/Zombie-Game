// Spec 79: the town's food: shops by the country's income, the distribution centre, household pantries.
#include <doctest.h>

#include <cmath>
#include <map>
#include <set>

#include "zs/gen/place.h"
#include "zs/gen/town_food.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

const TownRecord& town() {
  static const auto t = town_record(zs::default_place(), 1);
  return *t;
}

const FoodRetailParams& params() {
  static const FoodRetailParams p = load_food_retail(ZS_DATA_DIR "/gen");
  return p;
}

}  // namespace

TEST_CASE("town food: the retail data loads, shares add to one, and a bad directory is refused (A1)") {
  const FoodRetailParams& p = params();
  for (const FoodRetailGroup& g : p.groups) {
    double s = 0;
    for (double x : g.share) s += x;
    CHECK(s == doctest::Approx(1.0));
    CHECK(g.trips_per_week > 0);
  }
  CHECK(p.dc_dry_days > p.kinds[0].shelf_days + p.kinds[0].back_days);
  CHECK_THROWS(load_food_retail(ZS_DATA_DIR "/no_such_dir"));
}

TEST_CASE("town food: outlets by income group, on distinct plots, shares add up, supermarkets rise with income (A1)") {
  const TownRecord& t = town();
  const double people = t.spec.people;
  double last_super = -1;
  for (int g = kIncomeGroups - 1; g >= 0; --g) {  // low to high
    const TownFood f = plan_town_food(t.town, t.layout, people, g, params(), 1);
    REQUIRE(!f.outlets.empty());
    double share = 0, super = 0;
    std::set<int> plots;
    std::map<OutletKind, int> count;
    for (const Outlet& o : f.outlets) {
      share += o.share;
      ++count[o.kind];
      if (o.kind == OutletKind::Supermarket) super += o.share;
      CHECK(o.stock_days > 0);
      CHECK(o.stock_days < params().dc_dry_days);  // rule 4: the centre holds more than any shop
      if (o.plot >= 0) {
        CHECK(plots.insert(o.plot).second);         // no plot holds two outlets
        CHECK(t.layout.plots[static_cast<std::size_t>(o.plot)].lot < 0);
        CHECK(!o.address.empty());
      }
    }
    CHECK(share == doctest::Approx(1.0));
    CHECK(super >= last_super - 1e-12);
    last_super = super;
    // FR: outlets per 10,000 within the group's density (rounded), the main kind at least one.
    for (int k = 0; k < kOutletKinds; ++k) {
      const double want = people / 10000.0 * params().groups[static_cast<std::size_t>(g)].per_10k[static_cast<std::size_t>(k)];
      CHECK(std::abs(count[static_cast<OutletKind>(k)] - want) <= 1.0 + 1e-9);
    }
    MESSAGE(std::string(kIncomeGroupIds[static_cast<std::size_t>(g)]) << ": " << count[OutletKind::Supermarket] << " supermarkets, "
            << count[OutletKind::Convenience] << " convenience stores, " << count[OutletKind::Small] << " small shops, "
            << count[OutletKind::Market] << " markets for " << people << " people; supermarkets sell " << super);
  }
}

TEST_CASE("town food: households hold about three days in rich countries and fewer in poor ones (A1)") {
  const FoodRetailParams& p = params();
  CHECK(p.groups[0].pantry_days == doctest::Approx(3.0));  // FR-9
  for (int g = 1; g < kIncomeGroups; ++g) CHECK(p.groups[static_cast<std::size_t>(g)].pantry_days <= p.groups[static_cast<std::size_t>(g - 1)].pantry_days);
  CHECK(p.groups[kIncomeGroups - 1].trips_per_week >= p.groups[0].trips_per_week);
}

TEST_CASE("town food: the same seed gives the same town food (A1)") {
  const TownRecord& t = town();
  const TownFood a = plan_town_food(t.town, t.layout, t.spec.people, 1, params(), 1);
  const TownFood b = plan_town_food(t.town, t.layout, t.spec.people, 1, params(), 1);
  CHECK(town_food_digest(a) == town_food_digest(b));
  MESSAGE("town food digest: " << town_food_digest(a));
  CHECK(town_food_digest(a) == 14711340773777966511ULL);  // pinned on MSVC, 2026-10-13
}

TEST_CASE("town food: the one world's town is served by a distribution centre within reach (A2)" * doctest::skip(zs::World::kDebugBuild)) {
  for (std::uint64_t seed : {1ull, 4ull}) {
    const WorldGrid grid(kOneWorldFrequency, 6371.0);
    WorldParams wp;
    wp.seed = seed;
    const WorldFields f = generate_fields(grid, wp);
    const WorldLand land = generate_land(grid, f);
    DepositParams dp;
    dp.seed = seed;
    const WorldDeposits deposits = generate_deposits(grid, f, dp);
    const WorldPeople people = populate_world(grid, f, land, deposits, seed, load_group_tables(ZS_DATA_DIR), load_real_profiles(ZS_DATA_DIR));
    const WorldRoutes routes = build_routes(grid, f, land, people);
    const int t = choose_town(f, people, seed);
    REQUIRE(t >= 0);
    const ServingCentre c = serving_centre(grid, f, people, routes, t, params());
    REQUIRE(c.town >= 0);
    CHECK(people.towns[static_cast<std::size_t>(c.town)].people >= params().dc_min_town_people);
    CHECK(c.km <= 2 * params().dc_max_km);  // FR-10: about 300 km; a sparse region may reach further
    MESSAGE("seed " << seed << ": the block's town is served from a town of " << people.towns[static_cast<std::size_t>(c.town)].people
                    << " people, " << c.km << " km and " << c.hours << " h away");
  }
}
