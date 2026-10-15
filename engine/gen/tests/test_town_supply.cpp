// Spec 95: the generated world carries its routes and its food by kind; a town's supply from its region's flows.
#include <doctest.h>

#include "zs/gen/place.h"
#include "zs/gen/town_food.h"

using namespace zs::gen;

TEST_CASE("town supply: the world's routes and diet are those built on it") {
  const GeneratedWorld w = generate_world(1, {}, 32);
  const WorldRoutes r = build_routes(w.grid, w.fields, w.land, w.people);
  CHECK(r.routes.size() == w.routes.routes.size());
  CHECK(r.nodes.size() == w.routes.nodes.size());
  LandParams lp;
  const WorldDiet d = food_by_kind(w.fields, w.land, w.people, w.routes, {}, {}, lp, &w.use);
  CHECK(diet_digest(d) == diet_digest(w.diet));
}

TEST_CASE("town supply: shares in range, suppliers' hours from the routes") {
  const GeneratedWorld w = generate_world(1, {}, 32);
  int short_regions = 0, importing = 0;
  for (std::size_t t = 0; t < w.people.towns.size(); t += 97) {
    const TownSupply s = town_supply(w.routes, w.diet, w.people.towns[t].cell);
    REQUIRE(s.region >= 0);
    CHECK(s.supplied >= 0);
    CHECK(s.supplied <= 1);
    CHECK(s.local >= 0);
    CHECK(s.local <= 1);
    short_regions += s.supplied < 0.999;
    importing += !s.from.empty();
    double shares = 0;
    for (const auto& f : s.from) {
      CHECK(f.share > 0);
      shares += f.share;
      if (f.hours >= 0) CHECK(f.hours > 0);
    }
    CHECK(shares <= 1.0 + 1e-9);
  }
  MESSAGE(short_regions << " sampled towns in regions short of food, " << importing << " importing");
  CHECK(importing > 0);
}
