// Spec 82: power by building. The grid feeds each building through its own service, its panel feeds its circuits, its
// circuits feed its appliances; the generator is the main building's; every fridge has its own temperature.
#include <doctest.h>

#include <cmath>
#include <string>
#include <vector>

#include "zs/events.h"
#include "zs/grid_system.h"
#include "zs/power.h"
#include "zs/tier0.h"

using namespace zs;
using namespace zs::tier0;

namespace {

std::vector<EntityId> objects(const Tier0& t, const std::string& kind) {
  std::vector<EntityId> out;
  const auto& objs = t.map->objects();
  for (std::size_t i = 0; i < objs.size(); ++i)
    if (objs[i].kind == kind) out.push_back(t.actors->object_entity(i));
  return out;
}

std::size_t building_of(const Tier0& t, EntityId object) {
  for (const Receiver& r : t.power->receivers())
    if (r.object == object) return r.building;
  return ~std::size_t{0};
}

int count(const World& w, EventType type) {
  int n = 0;
  for (const Event& e : w.log.all()) n += e.type == type;
  return n;
}

}  // namespace

TEST_CASE("spec 82: each appliance belongs to the building whose area holds it; the houses are not on the main panel (A1)") {
  Tier0 t = make_world(1);
  const Power& p = *t.power;
  REQUIRE(p.buildings().size() == 3);
  std::size_t main = 0;
  for (std::size_t b = 0; b < p.buildings().size(); ++b)
    if (p.buildings()[b].generator) main = b;
  CHECK(p.buildings()[main].id == "main");
  for (const Receiver& r : p.receivers()) CHECK(p.buildings()[r.building].holds(r.at));
  const auto fridges = objects(t, "fridge");
  REQUIRE(fridges.size() == 3);
  CHECK(building_of(t, fridges[0]) == main);  // the main building's kitchen, first on the map
  CHECK(building_of(t, fridges[1]) != main);
  CHECK(building_of(t, fridges[2]) != main);
  CHECK(building_of(t, fridges[1]) != building_of(t, fridges[2]));
  CHECK(p.fridges().size() == 3);
}

TEST_CASE("spec 82: on the grid every building is fed; when it fails all go dark; the generator brings back only the main building (A1)") {
  Tier0 t = make_world(1);
  Power& p = *t.power;
  World& w = *t.world;
  const auto fridges = objects(t, "fridge");
  for (EntityId f : fridges) CHECK(p.powered(f));
  p.fail_grid(w, 0);
  for (EntityId f : fridges) CHECK_FALSE(p.powered(f));
  REQUIRE(p.start_generator(w, 0));
  REQUIRE(p.set_transfer(w, Transfer::Generator, 0));
  CHECK(p.powered(fridges[0]));        // the main building's, on the generator
  CHECK_FALSE(p.powered(fridges[1]));  // the houses stay dark
  CHECK_FALSE(p.powered(fridges[2]));
  CHECK(p.building_load_w(building_of(t, fridges[1])) == 0);
}

TEST_CASE("spec 82: a circuit over its rating trips only its own building's breaker (A1)") {
  Tier0 t = make_world(1);
  Power& p = *t.power;
  World& w = *t.world;
  const auto fridges = objects(t, "fridge");
  REQUIRE(p.set_on(w, objects(t, "radio").front(), true, 0));
  p.set_on(w, objects(t, "saw").front(), true, 0);  // 1.8 kW and the radio on the main building's 1.8 kW general circuit
  CHECK(count(w, EventType::BreakerTripped) >= 1);
  CHECK(p.tripped(2));                              // the main building's "general"
  for (std::size_t b = 0; b < p.buildings().size(); ++b)
    if (!p.buildings()[b].generator) CHECK_FALSE(p.building_state(b).tripped[2]);
  CHECK(p.powered(fridges[1]));
  CHECK(p.powered(fridges[2]));
}

TEST_CASE("spec 82: in the one world each building's load is its node's demand in spec 35's tree; the failure faults the substation (A1)") {
  Tier0 t = make_run(1);
  t.world->check_every_tick = false;
  const Power& p = *t.power;
  const GridTree& tree = t.grids->tree();
  const Grid& g = tree.grid(t.owners.district);
  auto node = [&](const std::string& name) -> const GridNode& {
    for (const GridNode& n : g.nodes)
      if (n.name == name) return n;
    FAIL("no node " << name);
    return g.nodes.front();
  };
  std::int64_t sum = 0;
  for (std::size_t b = 0; b < p.buildings().size(); ++b) {
    CHECK(node(p.buildings()[b].grid_node).demand_w == std::llround(p.building_load_w(b)));
    sum += std::llround(p.building_load_w(b));
  }
  CHECK(sum > 0);
  t.world->jump(p.grid_failure_tick() + 2);
  CHECK_FALSE(node("substation").sound);
  for (std::size_t b = 0; b < p.buildings().size(); ++b) CHECK(node(p.buildings()[b].grid_node).served_w == 0);
}

TEST_CASE("spec 82: after the grid fails a house fridge warms and spoils its food; the main building's stays cold on the generator (A1)") {
  Tier0 t = make_world(1);
  t.world->check_every_tick = false;
  Power& p = *t.power;
  World& w = *t.world;
  const auto fridges = objects(t, "fridge");
  p.fail_grid(w, 0);
  REQUIRE(p.start_generator(w, 0));
  REQUIRE(p.set_transfer(w, Transfer::Generator, 0));
  std::int64_t first_spoiled = -1;
  double main_at_4 = 0, house_at_4 = 0, house_at_12 = 0;
  for (int h = 1; h <= 36 && first_spoiled < 0; ++h) {
    w.run_ticks(kTicksPerGameHour);
    if (h == 4) main_at_4 = p.fridge_temp_c(fridges[0]), house_at_4 = p.fridge_temp_c(fridges[1]);
    if (h == 12) house_at_12 = p.fridge_temp_c(fridges[1]);
    for (const Event& e : w.log.all())
      if (e.type == EventType::FoodSpoiled && e.subject != fridges[0]) first_spoiled = h;
  }
  CHECK(first_spoiled > 8);  // spec 17's band
  CHECK(first_spoiled < 36);
  // The tank starts with 4 L (the map's generator; the jerrycan holds the rest), about 6 h at this load: at 4 h the
  // generator still runs, so the main fridge is cold while a house's has warmed.
  CHECK(main_at_4 < 4);
  CHECK(house_at_4 > main_at_4 + 3);
  CHECK(house_at_12 > 10);
  MESSAGE("a house fridge spoiled after " << first_spoiled << " h; at 4 h the main building's was " << main_at_4 << " C and a house's "
                                          << house_at_4 << " C; the house's at 12 h " << house_at_12 << " C");
}

TEST_CASE("spec 82 rule 2: the tree decides each building's mains: a fault on one house's node darkens that house alone; a repair brings it back (A1)") {
  Tier0 t = make_run(1);
  t.world->check_every_tick = false;
  Power& p = *t.power;
  World& w = *t.world;
  const GridTree& tree = t.grids->tree();
  const Grid& g = tree.grid(t.owners.district);
  std::size_t house = ~std::size_t{0};
  for (std::size_t b = 0; b < p.buildings().size(); ++b)
    if (p.buildings()[b].id == "house_a") house = b;
  REQUIRE(house < p.buildings().size());
  NodeId node = 0;
  for (std::size_t i = 0; i < g.nodes.size(); ++i)
    if (g.nodes[i].name == p.buildings()[house].grid_node) node = GridTree::node_id(t.owners.district, i);
  REQUIRE(node != 0);
  EntityId house_fridge = 0, main_fridge = 0;
  for (const Fridge& f : p.fridges()) {
    if (f.building == house && !house_fridge) house_fridge = f.object;
    if (p.buildings()[f.building].generator && !main_fridge) main_fridge = f.object;
  }
  REQUIRE(house_fridge);
  REQUIRE(main_fridge);
  CHECK(p.powered(house_fridge));
  w.submit({w.clock.tick, GridFault, node, 0, 0});  // the house's service drop fails
  w.run_ticks(2);
  CHECK_FALSE(p.building_mains(house));
  CHECK_FALSE(p.powered(house_fridge));
  CHECK(p.powered(main_fridge));  // the rest of the block keeps its mains
  CHECK(p.mains_alive());
  w.submit({w.clock.tick, GridFault, node, 1, 0});  // repaired
  w.run_ticks(2);
  CHECK(p.building_mains(house));
  CHECK(p.powered(house_fridge));
}
