// Spec 94: each country's electricity by its people and income, its mix from what it has, calibrated to Ember's world
// mix; dams on rivers, reactors by water, every country's plants and large towns joined by its grid.
#include <doctest.h>

#include <map>
#include <numeric>
#include <set>

#include "zs/gen/place.h"

using namespace zs::gen;

namespace {

const GeneratedWorld& world1() {
  static const GeneratedWorld w = generate_world(1, {}, 48);
  return w;
}

}  // namespace

TEST_CASE("energy: demand by people and income; the mix calibrated to Ember where the land allows") {
  const GeneratedWorld& w = world1();
  const WorldEnergy& en = w.energy;
  const EnergyParams p;
  double demand = std::accumulate(en.demand_twh.begin(), en.demand_twh.end(), 0.0);
  double made = std::accumulate(en.world_twh.begin(), en.world_twh.end(), 0.0);
  CHECK(made == doctest::Approx(demand).epsilon(1e-6));  // every country makes what it uses
  for (std::size_t k = 0; k < w.people.countries.size(); ++k) {
    const Country& c = w.people.countries[k];
    CHECK(en.demand_twh[k] == doctest::Approx(c.people * p.kwh_per_person[static_cast<std::size_t>(c.income)] * (1 + p.losses) * p.growth / 1e9));
  }
  // Nuclear, wind and solar follow Ember's shares; hydro is limited by the rivers the world has (open in the spec).
  for (Fuel f : {Fuel::Nuclear, Fuel::Wind, Fuel::Solar}) {
    CAPTURE(fuel_name(f));
    CHECK(en.world_twh[static_cast<std::size_t>(f)] / made == doctest::Approx(p.world_share[static_cast<std::size_t>(f)]).epsilon(0.25));
  }
  CHECK(en.world_twh[static_cast<std::size_t>(Fuel::Hydro)] / made <= p.world_share[static_cast<std::size_t>(Fuel::Hydro)] + 0.01);
  MESSAGE("hydro " << 100 * en.world_twh[3] / made << " % of " << made << " TWh");
}

TEST_CASE("energy: dams on rivers, reactors at large towns by water") {
  const GeneratedWorld& w = world1();
  for (const PowerPlant& pl : w.energy.plants) {
    if (pl.fuel == Fuel::Hydro) CHECK(w.land.river[pl.cell] > 0);
    if (pl.fuel == Fuel::Nuclear) {
      REQUIRE(pl.town >= 0);
      const Settlement& t = w.people.towns[static_cast<std::size_t>(pl.town)];
      CHECK(t.people >= 100000);  // a large town by the sea or a large river
      CHECK((w.fields.sea_cells[t.cell] <= 1 || w.land.river[t.cell] == 2));
    }
  }
}

TEST_CASE("energy: each country's grid is a tree over its plants and large towns") {
  const GeneratedWorld& w = world1();
  std::map<int, std::set<CellId>> nodes;
  for (const PowerPlant& pl : w.energy.plants)
    if (pl.fuel != Fuel::Wind && pl.fuel != Fuel::Solar && pl.fuel != Fuel::Oil) nodes[pl.country].insert(pl.cell);
  for (const Settlement& t : w.people.towns)
    if (t.people >= 100000) nodes[t.country].insert(t.cell);
  std::map<int, int> lines;
  for (const GridLine& l : w.energy.lines)
    if (l.country >= 0) ++lines[l.country];
  for (const auto& [k, v] : nodes)
    if (k >= 0 && v.size() >= 2) CHECK(lines[k] == static_cast<int>(v.size()) - 1);
}
