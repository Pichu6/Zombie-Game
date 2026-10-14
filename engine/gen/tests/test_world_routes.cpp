// Spec 74: routes between towns.
#include <doctest.h>

#include <algorithm>
#include <cmath>
#include <map>

#include "zs/gen/place.h"
#include "zs/gen/world_routes.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

struct Planet {
  WorldGrid grid;
  WorldFields fields;
  WorldLand land;
  WorldPeople people;
  WorldRoutes routes;
};

const Planet& planet(std::uint64_t seed, int frequency = kTestFrequency) {
  static std::map<std::pair<std::uint64_t, int>, Planet> cache;
  const auto key = std::make_pair(seed, frequency);
  if (auto it = cache.find(key); it != cache.end()) return it->second;
  Planet p{WorldGrid(frequency, 6371.0), {}, {}, {}, {}};
  WorldParams wp;
  wp.seed = seed;
  p.fields = generate_fields(p.grid, wp);
  p.land = generate_land(p.grid, p.fields);
  DepositParams dp;
  dp.seed = seed;
  const WorldDeposits deposits = generate_deposits(p.grid, p.fields, dp);
  p.people = populate_world(p.grid, p.fields, p.land, deposits, seed, load_group_tables(ZS_DATA_DIR));
  p.routes = build_routes(p.grid, p.fields, p.land, p.people);
  return cache.emplace(key, std::move(p)).first->second;
}

bool neighbours(const WorldGrid& g, CellId a, CellId b) {
  const auto n = g.neighbours(a);
  return std::find(n.begin(), n.end(), b) != n.end();
}

}  // namespace

TEST_CASE("routes: every town cell is a node, the tree joins them all, paths run through neighbouring cells (A1)") {
  for (std::uint64_t seed : {1ull, 2ull}) {
    const Planet& p = planet(seed);
    const WorldRoutes& w = p.routes;
    REQUIRE(w.nodes.size() >= 2);
    for (std::size_t t = 0; t < p.people.towns.size(); ++t) {
      const int k = w.node_of[p.people.towns[t].cell];
      REQUIRE(k >= 0);
      const auto& towns = w.nodes[static_cast<std::size_t>(k)].towns;
      CHECK(std::find(towns.begin(), towns.end(), static_cast<int>(t)) != towns.end());
    }
    std::size_t tree = 0, loops = 0;
    for (const Route& r : w.routes) {
      tree += r.why == RouteWhy::Tree;
      loops += r.why == RouteWhy::Loop;
      CHECK(r.a < r.b);
      REQUIRE(r.path.size() >= 2);
      CHECK(r.path.front() == w.nodes[static_cast<std::size_t>(r.a)].cell);
      CHECK(r.path.back() == w.nodes[static_cast<std::size_t>(r.b)].cell);
      for (std::size_t i = 1; i < r.path.size(); ++i) CHECK(neighbours(p.grid, r.path[i - 1], r.path[i]));
    }
    CHECK(tree == w.nodes.size() - 1);  // rule 4: one component, a tree
    for (std::size_t k = 1; k < w.nodes.size(); ++k) CHECK(node_hours(w, 0, static_cast<int>(k)) > 0);
    MESSAGE("seed " << seed << ": " << w.nodes.size() << " town cells, " << tree << " tree routes, " << loops << " loops");
  }
}

TEST_CASE("routes: a route's hours are the sum of its steps; loops only where the network's detour was large (A1)") {
  const Planet& p = planet(1);
  const WorldRoutes& w = p.routes;
  for (const Route& r : w.routes) {
    double hours = 0;
    for (std::size_t i = 1; i < r.path.size(); ++i)
      hours += route_step(p.grid, p.fields, p.land, p.people, w.params, r.path[i - 1], r.path[i]).hours;
    CHECK(r.hours == doctest::Approx(hours).epsilon(1e-12));
    CHECK(r.km[0] + r.km[1] + r.km[2] > 0);
  }
  // Rebuilding the network in order: when a loop was added, the network without it was slower than the detour allows.
  WorldRoutes partial = w;
  partial.routes.clear();
  for (auto& v : partial.of_node) v.clear();
  for (const Route& r : w.routes) {
    if (r.why == RouteWhy::Loop) {
      const double before = node_hours(partial, r.a, r.b);
      CHECK((before < 0 || before > w.params.loop_detour * r.hours * (1 - 1e-9)));
    }
    const int idx = static_cast<int>(partial.routes.size());
    partial.of_node[static_cast<std::size_t>(r.a)].push_back(idx);
    partial.of_node[static_cast<std::size_t>(r.b)].push_back(idx);
    partial.routes.push_back(r);
  }
}

TEST_CASE("routes: a step is symmetric; rivers are faster than roads only where large; ports and borders cost time (A1)") {
  const Planet& p = planet(1);
  const RouteParams& rp = p.routes.params;
  int ports = 0, borders = 0, rivers = 0;
  for (CellId u = 0; u < p.grid.size(); ++u)
    for (CellId v : p.grid.neighbours(u)) {
      const Step a = route_step(p.grid, p.fields, p.land, p.people, rp, u, v);
      const Step b = route_step(p.grid, p.fields, p.land, p.people, rp, v, u);
      CHECK(a.hours == doctest::Approx(b.hours).epsilon(1e-12));
      CHECK(a.hours > 0);
      if (a.mode == RouteMode::River) {
        ++rivers;
        CHECK(p.land.river[u] == 2);
        CHECK(p.land.river[v] == 2);
      }
      if (a.port) {
        ++ports;
        CHECK(a.hours >= rp.port_hours);
      }
      if (a.border) {
        ++borders;
        CHECK(a.border_hours >= rp.border_hours[0]);
        CHECK(a.border_hours <= rp.border_hours[kIncomeGroups - 1]);
        CHECK(a.hours >= a.border_hours);
      }
    }
  CHECK(ports > 0);
  CHECK(borders > 0);
  MESSAGE("steps: " << rivers << " by river, " << ports << " through a port, " << borders << " across a border");
}

TEST_CASE("routes: travel time is symmetric, zero to itself, no longer than a direct route; islands by sea (A1)") {
  const Planet& p = planet(1);
  const WorldRoutes& w = p.routes;
  for (const Route& r : w.routes) {
    const double h = node_hours(w, r.a, r.b);
    CHECK(h <= r.hours * (1 + 1e-12));
    CHECK(h == doctest::Approx(node_hours(w, r.b, r.a)).epsilon(1e-12));
  }
  CHECK(node_hours(w, 3, 3) == 0);
  // The fastest way's routes chain from one end to the other and add up to the travel time.
  for (int to : {1, 7, 42}) {
    int at = 0;
    double sum = 0;
    for (int r : node_route_path(w, 0, to)) {
      const Route& route = w.routes[static_cast<std::size_t>(r)];
      REQUIRE((route.a == at || route.b == at));
      at = route.a == at ? route.b : route.a;
      sum += route.hours;
    }
    CHECK(at == to);
    CHECK(sum == doctest::Approx(node_hours(w, 0, to)).epsilon(1e-12));
  }
  // Two towns in one cell: rule 2's in-cell term.
  for (const RouteNode& n : w.nodes)
    if (n.towns.size() >= 2) {
      const double h = town_hours(p.grid, p.fields, p.people, w, n.towns[0], n.towns[1]);
      CHECK(h > 0);
      CHECK(h < 10);
      break;
    }
  std::size_t by_sea = 0;
  for (const Route& r : w.routes) by_sea += r.km[static_cast<int>(RouteMode::Sea)] > 0;
  CHECK(by_sea > 0);
}

TEST_CASE("routes: the same seed gives the same routes (A1)") {
  const Planet& p = planet(1);
  const WorldRoutes again = build_routes(p.grid, p.fields, p.land, p.people);
  CHECK(routes_digest(again) == routes_digest(p.routes));
  MESSAGE("routes digest: " << routes_digest(p.routes));
  CHECK(routes_digest(p.routes) == 14769525099496291230ULL);  // re-pinned on MSVC, 2026-10-14 (spec 86: people by food, water, coast and height; towns by site)
}

TEST_CASE("routes: the one world's grid, its trunk network and circuity, the block's town to its capital (A2)" *
          doctest::skip(zs::World::kDebugBuild)) {
  const Planet& p = planet(1, kOneWorldFrequency);
  const WorldRoutes& w = p.routes;
  std::size_t tree = 0;
  double km[kRouteModes] = {};
  for (const Route& r : w.routes) {
    tree += r.why == RouteWhy::Tree;
    for (int m = 0; m < kRouteModes; ++m) km[m] += r.km[static_cast<std::size_t>(m)];
  }
  CHECK(tree == w.nodes.size() - 1);
  // Rule 8: circuity in the band between European countries (WSR-43: 1.15-1.76, median 1.25).
  const double circuity = road_circuity(p.grid, w, 150, 400, 1), far = road_circuity(p.grid, w, 500, 200, 1);
  CHECK(circuity >= 1.15);
  CHECK(circuity <= 1.76);
  CHECK(far >= 1.15);
  CHECK(far <= 1.76);
  const int town = choose_town(p.fields, p.people, 1);
  REQUIRE(town >= 0);
  const Settlement& t = p.people.towns[static_cast<std::size_t>(town)];
  int capital = -1;
  for (std::size_t i = 0; i < p.people.towns.size(); ++i)
    if (p.people.towns[i].country == t.country && p.people.towns[i].rank == 1) capital = static_cast<int>(i);
  REQUIRE(capital >= 0);
  const double h = town_hours(p.grid, p.fields, p.people, w, town, capital);
  CHECK(h >= 0);
  MESSAGE(w.nodes.size() << " town cells, " << w.routes.size() << " routes (" << w.routes.size() - tree << " loops); trunk km road "
                         << km[0] << ", river " << km[1] << ", sea " << km[2] << "; road circuity " << circuity << " (500-2000 km: " << far << ")"
                         << "; the block's town to its capital " << h << " h");
}
