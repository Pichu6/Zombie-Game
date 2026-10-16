// Spec 96: the town's power from its grid, and the outage from its causes.
#include <doctest.h>

#include <algorithm>
#include <set>

#include "zs/gen/place.h"
#include "zs/gen/town_power.h"
#include "zs/power.h"

using namespace zs::gen;

namespace {
// An island of one plant of a kind for a load, the town on its node (no feeder, no line).
OutageTimeline island_of(Fuel fuel, double mw, double load_mw, WorldEnergy& e) {
  e.plants = {{fuel, 0, 0, 0, mw}};
  TownFeed feed;
  feed.town = 0;
  feed.on_grid = true;
  feed.plants = {0};
  feed.mw[static_cast<std::size_t>(fuel)] = mw;
  feed.load_mw = load_mw;
  return outage_timeline(feed, e, {}, 7);
}
}  // namespace

TEST_CASE("town power: the island holds the plants the lines reach, its load its countries' demand (rules 1, 2)") {
  const GeneratedWorld w = generate_world(1, {}, 32);
  std::set<CellId> ends;
  for (const GridLine& l : w.energy.lines) ends.insert(l.a), ends.insert(l.b);
  int on_grid = 0;
  for (std::size_t t = 0; t < w.people.towns.size(); t += 53) {
    const TownFeed f = town_feed(w.grid, w.people, w.energy, static_cast<int>(t));
    CHECK(f.load_mw > 0);
    double mw = 0;
    for (int i : f.plants) {
      const PowerPlant& pl = w.energy.plants[static_cast<std::size_t>(i)];
      CHECK((pl.town == static_cast<int>(t) || ends.count(pl.cell) == 1));
      mw += pl.mw;
    }
    double sum = 0;
    for (double x : f.mw) sum += x;
    CHECK(sum == doctest::Approx(mw));
    if (f.on_grid) {
      ++on_grid;
      CHECK(ends.count(f.node) == 1);
      double demand = 0;
      for (int c : f.countries) demand += w.energy.demand_twh[static_cast<std::size_t>(c)] * 1e6 / 8760;
      CHECK(f.load_mw == doctest::Approx(demand));
    }
  }
  CHECK(on_grid > 0);
}

TEST_CASE("town power: hydro outlasts coal; an island below its load is dark when the staff leave (rule 3)") {
  WorldEnergy e;
  const OutageTimeline coal = island_of(Fuel::Coal, 1000, 1000, e);
  const OutageTimeline hydro = island_of(Fuel::Hydro, 1000, 1000, e);  // 400 MW of water against 350 of load
  CHECK(coal.staff_leave_h == hydro.staff_leave_h);
  CHECK(coal.island_dark_h > coal.staff_leave_h);
  CHECK(coal.island_dark_h <= coal.staff_leave_h + 24);
  CHECK(hydro.island_dark_h >= hydro.staff_leave_h + 14 * 24);
  const OutageTimeline small = island_of(Fuel::Coal, 1000, 10000, e);
  CHECK(small.island_dark_h == small.staff_leave_h);
  const OutageTimeline wind = island_of(Fuel::Wind, 5000, 1000, e);  // wind cannot hold a grid
  CHECK(wind.island_dark_h == wind.staff_leave_h);
}

TEST_CASE("town power: the outage is the earliest cause, the same for the same seed (rules 4, 5)") {
  const GeneratedWorld w = generate_world(1, {}, 32);
  for (std::size_t t = 0; t < w.people.towns.size(); t += 71) {
    const TownFeed f = town_feed(w.grid, w.people, w.energy, static_cast<int>(t));
    const OutageTimeline a = outage_timeline(f, w.energy, {}, 3), b = outage_timeline(f, w.energy, {}, 3);
    CHECK(a.dark_h == b.dark_h);
    CHECK(a.dark_h == std::min({a.island_dark_h, a.line_fault_h, a.feeder_fault_h}));
    CHECK(a.dark_h >= a.staff_leave_h);
    CHECK(!describe_feed(f, a).empty());
  }
}

TEST_CASE("town power: the one world's grid fails at its town's tick (rule 5)") {
  const auto t = make_world(1);
  REQUIRE(t.place);
  REQUIRE(t.power);
  const zs::Place& p = t.place->place();
  CHECK(p.grid_fail_tick >= 0);
  CHECK(t.power->grid_failure_tick() == p.grid_fail_tick);
  MESSAGE("seed 1's town: dark on day ", static_cast<double>(p.grid_fail_tick) / zs::kTicksPerGameDay, " (", p.grid_fail_cause, ")");
}
