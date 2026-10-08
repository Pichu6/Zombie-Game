// Spec 35: a tree of grids; faults up, orders down, exact restore; inside a world: commands, events, save and replay.
#include <doctest.h>

#include <memory>

#include "zs/grid_system.h"

using namespace zs;

namespace {

// A nation with one plant (10 MW) feeding two districts through breakers; each district feeds buildings through
// its own breakers; buildings hold leaf loads. Building A also has a generator behind a transfer switch.
struct Net {
  GridTree t;
  GridId nation, d1, d2, a, b, c;
  std::size_t plant, br1, br2, d1_feed, d1_brA, d1_brB, d2_feed, d2_brC;
  std::size_t a_mains, a_gen, a_xfer, a_hub, a_fridge, a_lamps, a_saw;
  std::size_t b_mains, b_load, c_mains, c_load;
  std::size_t n_d1, n_d2, d1_a, d1_b, d2_c;

  Net() {
    nation = t.add_grid("nation");
    d1 = t.add_grid("district 1", nation);
    d2 = t.add_grid("district 2", nation);
    a = t.add_grid("building A", d1);
    b = t.add_grid("building B", d1);
    c = t.add_grid("building C", d2);
    // Nation: plant - hub - breakers - district loads.
    plant = t.add_node(nation, {"plant", NodeKind::Source, 10'000'000});
    const std::size_t hub = t.add_node(nation, {"bus", NodeKind::Hub});
    br1 = t.add_node(nation, {"breaker d1", NodeKind::Switch});
    br2 = t.add_node(nation, {"breaker d2", NodeKind::Switch});
    n_d1 = t.add_node(nation, {"district 1", NodeKind::Load});
    n_d2 = t.add_node(nation, {"district 2", NodeKind::Load});
    t.add_edge(nation, plant, hub); t.add_edge(nation, hub, br1); t.add_edge(nation, hub, br2);
    t.add_edge(nation, br1, n_d1); t.add_edge(nation, br2, n_d2);
    // District 1: feed hub - breakers - building loads.
    d1_feed = t.add_node(d1, {"substation", NodeKind::Hub});
    d1_brA = t.add_node(d1, {"breaker A", NodeKind::Switch});
    d1_brB = t.add_node(d1, {"breaker B", NodeKind::Switch});
    d1_a = t.add_node(d1, {"building A", NodeKind::Load});
    d1_b = t.add_node(d1, {"building B", NodeKind::Load});
    t.add_edge(d1, d1_feed, d1_brA); t.add_edge(d1, d1_feed, d1_brB); t.add_edge(d1, d1_brA, d1_a); t.add_edge(d1, d1_brB, d1_b);
    // District 2.
    d2_feed = t.add_node(d2, {"substation", NodeKind::Hub});
    d2_brC = t.add_node(d2, {"breaker C", NodeKind::Switch});
    d2_c = t.add_node(d2, {"building C", NodeKind::Load});
    t.add_edge(d2, d2_feed, d2_brC); t.add_edge(d2, d2_brC, d2_c);
    // Building A: mains hub and a 3 kW generator behind a transfer switch (open: on mains), then loads.
    a_mains = t.add_node(a, {"mains", NodeKind::Hub});
    a_gen = t.add_node(a, {"generator", NodeKind::Source, 3'000});
    a_xfer = t.add_node(a, {"transfer", NodeKind::Switch});
    a_hub = t.add_node(a, {"panel", NodeKind::Hub});
    a_fridge = t.add_node(a, {"fridge", NodeKind::Load, 0, 150});
    a_lamps = t.add_node(a, {"lamps", NodeKind::Load, 0, 60});
    a_saw = t.add_node(a, {"saw", NodeKind::Load, 0, 1'800});
    t.node(GridTree::node_id(a, a_xfer)).closed = false;
    t.add_edge(a, a_mains, a_hub); t.add_edge(a, a_gen, a_xfer); t.add_edge(a, a_xfer, a_hub);
    t.add_edge(a, a_hub, a_fridge); t.add_edge(a, a_hub, a_lamps); t.add_edge(a, a_hub, a_saw);
    // Buildings B and C: one load each.
    b_mains = t.add_node(b, {"mains", NodeKind::Hub});
    b_load = t.add_node(b, {"flat", NodeKind::Load, 0, 2'500});
    t.add_edge(b, b_mains, b_load);
    c_mains = t.add_node(c, {"mains", NodeKind::Hub});
    c_load = t.add_node(c, {"factory", NodeKind::Load, 0, 6'000'000});
    t.add_edge(c, c_mains, c_load);
    t.attach(nation, n_d1, d1, d1_feed);
    t.attach(nation, n_d2, d2, d2_feed);
    t.attach(d1, d1_a, a, a_mains);
    t.attach(d1, d1_b, b, b_mains);
    t.attach(d2, d2_c, c, c_mains);
    t.solve();
  }
  NodeId id(GridId g, std::size_t n) const { return GridTree::node_id(g, n); }
};

}  // namespace

TEST_CASE("demand sums up the tree and is served in full when the plant can; every level agrees (A1)") {
  Net n;
  const auto nat = n.t.summary(n.nation);
  CHECK(nat.demand_w == 150 + 60 + 1'800 + 2'500 + 6'000'000);
  CHECK(nat.served_w == nat.demand_w);
  CHECK(nat.unserved_w == 0);
  CHECK(nat.capacity_w == 10'000'000 + 3'000);
  CHECK(n.t.summary(n.d1).demand_w == 150 + 60 + 1'800 + 2'500);
  CHECK(n.t.grid(n.d1).served_w == 150 + 60 + 1'800 + 2'500);
  CHECK(n.t.node(n.id(n.nation, n.n_d1)).served_w == n.t.grid(n.d1).demand_w);
  CHECK(n.t.node(n.id(n.nation, n.plant)).served_w == nat.demand_w);  // the plant gives what is drawn, the generator idles
  CHECK(n.t.node(n.id(n.a, n.a_gen)).served_w == 0);
  CHECK(n.t.node(n.id(n.a, n.a_saw)).connected);
}

TEST_CASE("a fault in one building is unserved load in its district and the nation; the repair restores the exact state (A1)") {
  Net n;
  const Grid before_a = n.t.grid(n.a), before_d1 = n.t.grid(n.d1), before_nation = n.t.grid(n.nation);
  const auto before = n.t.summary(n.nation);
  // The panel in building A fails: its loads are cut off. The nation and the district see exactly those watts lost
  // from their served load; the demand they report falls by the same, because what is disconnected is not drawn.
  n.t.node(n.id(n.a, n.a_hub)).sound = false;
  const auto changed = n.t.solve();
  CHECK(changed.size() == 3);  // A, district 1, nation
  CHECK(n.t.grid(n.a).served_w == 0);
  CHECK(n.t.grid(n.d1).served_w == before_d1.served_w - (150 + 60 + 1'800));
  CHECK(n.t.grid(n.nation).served_w == before_nation.served_w - (150 + 60 + 1'800));
  CHECK(n.t.node(n.id(n.nation, n.plant)).served_w == before.served_w - (150 + 60 + 1'800));
  // A fault that leaves the load connected but unserved: the breaker to district 2 is faulted (not opened).
  n.t.node(n.id(n.nation, n.br2)).sound = false;
  n.t.solve();
  CHECK(n.t.grid(n.c).served_w == 0);
  CHECK(!n.t.node(n.id(n.c, n.c_load)).connected);
  // Repairs: the same state as before, node by node.
  n.t.node(n.id(n.nation, n.br2)).sound = true;
  n.t.node(n.id(n.a, n.a_hub)).sound = true;
  n.t.solve();
  for (std::size_t i = 0; i < before_a.nodes.size(); ++i) {
    CHECK(n.t.grid(n.a).nodes[i].served_w == before_a.nodes[i].served_w);
    CHECK(n.t.grid(n.a).nodes[i].connected == before_a.nodes[i].connected);
  }
  CHECK(n.t.grid(n.d1).served_w == before_d1.served_w);
  CHECK(n.t.grid(n.nation).served_w == before_nation.served_w);
  CHECK(n.t.summary(n.nation).served_w == before.served_w);
}

TEST_CASE("an order above is lost load below: load shedding opens a breaker; a plant cut sheds in a fixed order (A1)") {
  Net n;
  // Open the nation's breaker to district 1: every load in A and B is unserved and not connected.
  n.t.node(n.id(n.nation, n.br1)).closed = false;
  n.t.solve();
  CHECK(n.t.grid(n.d1).served_w == 0);
  CHECK(n.t.summary(n.d1).unserved_w == 150 + 60 + 1'800 + 2'500);
  CHECK(!n.t.node(n.id(n.b, n.b_load)).connected);
  CHECK(n.t.grid(n.c).served_w == 6'000'000);
  // Building A starts its generator and moves to it: its loads are served by the generator up to 3 kW, in node order
  // (fridge, lamps, then the saw gets what is left: 2,790 W of its 1,800? no: 3,000 - 210 = 2,790 >= 1,800, all served).
  n.t.node(n.id(n.a, n.a_xfer)).closed = true;
  n.t.solve();
  CHECK(n.t.grid(n.a).served_w == 150 + 60 + 1'800);
  CHECK(n.t.node(n.id(n.a, n.a_gen)).served_w == 150 + 60 + 1'800);
  // A bigger saw than the generator can carry: the loads before it are served, the saw partly (what is left).
  n.t.node(n.id(n.a, n.a_saw)).demand_w = 3'500;
  n.t.solve();
  CHECK(n.t.node(n.id(n.a, n.a_fridge)).served_w == 150);
  CHECK(n.t.node(n.id(n.a, n.a_lamps)).served_w == 60);
  CHECK(n.t.node(n.id(n.a, n.a_saw)).served_w == 3'000 - 210);
  CHECK(n.t.summary(n.a).unserved_w == 3'500 - (3'000 - 210));
  // The plant drops to 5 MW with district 1 back on: served in node order at the nation (district 1 first, then
  // district 2 gets what is left); the nation's unserved equals the shortfall exactly.
  n.t.node(n.id(n.nation, n.br1)).closed = true;
  n.t.node(n.id(n.a, n.a_xfer)).closed = false;
  n.t.node(n.id(n.a, n.a_saw)).demand_w = 1'800;
  n.t.node(n.id(n.nation, n.plant)).capacity_w = 5'000'000;
  n.t.solve();
  const auto nat = n.t.summary(n.nation);
  CHECK(nat.demand_w == 150 + 60 + 1'800 + 2'500 + 6'000'000);
  CHECK(nat.served_w == 5'000'000);
  CHECK(nat.unserved_w == nat.demand_w - 5'000'000);
  CHECK(n.t.grid(n.d1).served_w == 150 + 60 + 1'800 + 2'500);
  CHECK(n.t.grid(n.c).served_w == 5'000'000 - (150 + 60 + 1'800 + 2'500));
  CHECK(n.t.node(n.id(n.c, n.c_load)).served_w == n.t.grid(n.c).served_w);
}

TEST_CASE("solving twice changes nothing; the save holds the state and reloads to the same results (A1)") {
  Net n;
  n.t.node(n.id(n.nation, n.br2)).closed = false;
  n.t.node(n.id(n.a, n.a_saw)).sound = false;
  n.t.solve();
  CHECK(n.t.solve().empty());
  ArchiveWriter w;
  n.t.save(w);
  Net m;
  ArchiveReader r(w.bytes());
  m.t.load(r);
  CHECK(r.at_end());
  for (std::size_t g = 0; g < n.t.size(); ++g) {
    CHECK(m.t.grid(static_cast<GridId>(g)).served_w == n.t.grid(static_cast<GridId>(g)).served_w);
    CHECK(m.t.grid(static_cast<GridId>(g)).demand_w == n.t.grid(static_cast<GridId>(g)).demand_w);
  }
}

// ---- In a world.

namespace {
std::unique_ptr<World> make_world(std::uint64_t seed, Net& shape) {
  auto w = std::make_unique<World>(seed);
  auto grids = std::make_unique<GridSystem>();
  grids->tree() = shape.t;
  grids->start();
  w->add_system(std::move(grids));
  w->close_setup();
  w->check_every_tick = true;
  return w;
}

void script(World& w, const Net& n) {
  const std::int64_t t = w.clock.tick, hour = kTicksPerGameHour;
  if (t == 1 * hour) w.submit({0, GridFault, n.id(n.a, n.a_hub), 0, 0});
  if (t == 2 * hour) w.submit({0, GridSwitch, n.id(n.nation, n.br2), 0, 0});
  if (t == 2 * hour + 1) w.submit({0, GridSwitch, n.id(n.nation, n.br2), 0, 0});   // refused: already open
  if (t == 3 * hour) w.submit({0, GridSetDemand, n.id(n.nation, n.n_d1), 5, 0});   // refused: a child grid's load
  if (t == 4 * hour) w.submit({0, GridFault, n.id(n.a, n.a_hub), 1, 0});
  if (t == 5 * hour) w.submit({0, GridSetCapacity, n.id(n.nation, n.plant), 2'000'000, 0});
  if (t == 6 * hour) w.submit({0, GridSwitch, n.id(n.nation, n.br2), 1, 0});
}
}  // namespace

TEST_CASE("grids in a world: commands solve on change and report each grid whose service changed; save, replay (A1)") {
  Net shape;
  auto w = make_world(1, shape);
  const std::int64_t ticks = 7 * kTicksPerGameHour;
  for (std::int64_t i = 0; i < ticks; ++i) { script(*w, shape); w->step(); }
  const auto* grids = w->system<GridSystem>();
  REQUIRE(grids);
  int changed = 0, refused = 0;
  for (const Event& e : w->log.all()) {
    if (e.type == EventType::GridChanged) ++changed;
    if (e.type == EventType::GridRefused) ++refused;
  }
  CHECK(refused == 2);
  CHECK(changed >= 3 + 2 + 3 + 2);  // the fault (A, d1, nation), the breaker (c, d2... and the nation), the repair, the plant cut, the breaker back
  const auto nat = grids->tree().summary(shape.nation);
  CHECK(nat.capacity_w == 2'000'000 + 3'000);
  CHECK(nat.served_w == 2'000'000);
  CHECK(nat.unserved_w == nat.demand_w - 2'000'000);
  // Save, load, save; replay.
  const auto bytes = w->save();
  auto candidate = make_world(1, shape);
  candidate->load(bytes);
  CHECK(candidate->save() == bytes);
  CHECK(candidate->state_hash() == w->state_hash());
  auto r = make_world(1, shape);
  std::size_t next = 0;
  const auto& inputs = w->input_log();
  for (std::int64_t i = 0; i < ticks; ++i) {
    while (next < inputs.size() && inputs[next].tick == r->clock.tick) r->submit(inputs[next++]);
    r->step();
  }
  CHECK(r->state_hash() == w->state_hash());
}
