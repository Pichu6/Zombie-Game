// Spec 93: processing plants at towns, by Weber's rules, in Earth's numbers scaled by the world's people.
#include <doctest.h>

#include <cmath>

#include "zs/gen/place.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

const GeneratedWorld& world1() {
  static const GeneratedWorld w = generate_world(1, {}, 48);
  return w;
}

}  // namespace

TEST_CASE("industry: Earth's plant counts scaled by the world's people; a kind without its input has none") {
  const GeneratedWorld& w = world1();
  const WorldIndustry& ind = w.industry;
  REQUIRE_FALSE(ind.kinds.empty());
  const double scale = w.people.total_people / 8.0e9;
  std::vector<int> count(ind.kinds.size(), 0);
  for (const Plant& p : ind.plants) ++count[static_cast<std::size_t>(p.kind)];
  for (std::size_t k = 0; k < ind.kinds.size(); ++k) {
    CAPTURE(ind.kinds[k].id);
    const long want = std::lround(ind.kinds[k].earth_plants * scale);
    CHECK((count[k] == want || count[k] == 0));  // all of Earth's share, or none when the world lacks the input
  }
  MESSAGE(ind.plants.size() << " plants on the coarse grid");
}

TEST_CASE("industry: every plant at a town; materials plants by their inputs") {
  const GeneratedWorld& w = world1();
  for (const Plant& p : w.industry.plants) {
    REQUIRE(p.town >= 0);
    REQUIRE(p.town < static_cast<int>(w.people.towns.size()));
    CHECK(p.cell == w.people.towns[static_cast<std::size_t>(p.town)].cell);
    const PlantKind& k = w.industry.kinds[static_cast<std::size_t>(p.kind)];
    if (k.rule == PlantRule::Materials && !p.feeds.empty()) {
      const WorkSite& s = w.use.sites[static_cast<std::size_t>(p.feeds.front())];
      // Smelters and alumina refineries stand by their mines: within 800 km on the one world, 1,200 on this coarse grid
      // (its cells are about 200 km apart, and a site sells to the market town nearest it over land).
      // Temperature recalibration (2026-10-17): with people moved, the nearest market town can lie a cell further.
      CHECK(w.grid.distance_km(p.cell, s.cell) <= 1200);
    }
  }
}

TEST_CASE("industry: deterministic") {
  const GeneratedWorld a = generate_world(3, {}, 32), b = generate_world(3, {}, 32);
  REQUIRE(a.industry.plants.size() == b.industry.plants.size());
  for (std::size_t i = 0; i < a.industry.plants.size(); ++i) {
    CHECK(a.industry.plants[i].town == b.industry.plants[i].town);
    CHECK(a.industry.plants[i].kind == b.industry.plants[i].kind);
  }
}
