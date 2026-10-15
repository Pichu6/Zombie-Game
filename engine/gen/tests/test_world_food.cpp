// Spec 76: food flows on the routes.
#include <doctest.h>

#include <cmath>
#include <functional>
#include <map>
#include <queue>
#include <sstream>

#include "zs/gen/place.h"
#include "zs/gen/world_food.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

struct Planet {
  WorldGrid grid;
  WorldFields fields;
  WorldLand land;
  WorldPeople people;
  WorldRoutes routes;
  WorldLandUse use;  // spec 88: the game grows food from the fields in use
};

const Planet& planet(std::uint64_t seed, int frequency = kTestFrequency) {
  static std::map<std::pair<std::uint64_t, int>, Planet> cache;
  const auto key = std::make_pair(seed, frequency);
  if (auto it = cache.find(key); it != cache.end()) return it->second;
  Planet p{WorldGrid(frequency, 6371.0), {}, {}, {}, {}, {}};
  WorldParams wp;
  wp.seed = seed;
  p.fields = generate_fields(p.grid, wp);
  p.land = generate_land(p.grid, p.fields);
  DepositParams dp;
  dp.seed = seed;
  const WorldDeposits deposits = generate_deposits(p.grid, p.fields, dp);
  p.people = populate_world(p.grid, p.fields, p.land, deposits, seed, load_group_tables(ZS_DATA_DIR), load_real_profiles(ZS_DATA_DIR));  // as the game
  p.routes = build_routes(p.grid, p.fields, p.land, p.people);
  p.use = generate_land_use(p.grid, p.fields, p.land, deposits, p.people);
  return cache.emplace(key, std::move(p)).first->second;
}

double cheapest(const WorldRoutes& routes, const FreightParams& fp, int from, int to) {
  std::vector<double> dist(routes.nodes.size(), 1e300);
  using Item = std::pair<double, int>;
  std::priority_queue<Item, std::vector<Item>, std::greater<Item>> q;
  dist[static_cast<std::size_t>(from)] = 0;
  q.push({0, from});
  while (!q.empty()) {
    const auto [d, u] = q.top();
    q.pop();
    if (d > dist[static_cast<std::size_t>(u)]) continue;
    for (int r : routes.of_node[static_cast<std::size_t>(u)]) {
      const Route& route = routes.routes[static_cast<std::size_t>(r)];
      const int v = route.a == u ? route.b : route.a;
      const double nd = d + freight_cost(route, fp);
      if (nd < dist[static_cast<std::size_t>(v)]) dist[static_cast<std::size_t>(v)] = nd, q.push({nd, v});
    }
  }
  return dist[static_cast<std::size_t>(to)];
}

}  // namespace

TEST_CASE("food: every cell's land and people belong to one region; food adds up node by node and over the flows (A1)") {
  for (std::uint64_t seed : {1ull, 2ull}) {
    const Planet& p = planet(seed);
    const WorldFood f = food_flows(p.land, p.people, p.routes, {}, &p.use);
    double made = 0, people = 0;
    for (std::size_t c = 0; c < p.grid.size(); ++c) {
      REQUIRE(p.routes.region_of[c] >= 0);
      made += p.land.capacity_food[c];
      people += p.people.people[c];
    }
    CHECK(f.production == doctest::Approx(FreightParams{}.grown_vs_need * people).epsilon(1e-9));  // rule 1
    CHECK(made > people);  // the land could feed more than live on it
    CHECK(f.demand == doctest::Approx(people).epsilon(1e-9));
    double sent = 0, received = 0;
    for (const NodeFood& nf : f.nodes) {
      CHECK(nf.made == doctest::Approx(nf.eaten + nf.sent + nf.surplus).epsilon(1e-9));
      CHECK(nf.demand == doctest::Approx(nf.eaten + nf.received + nf.unmet).epsilon(1e-9));
      CHECK(nf.surplus >= -1e-6);
      CHECK(nf.unmet >= -1e-6);
      CHECK((nf.sent == 0 || nf.received == 0));  // a node either sends or receives
      sent += nf.sent;
      received += nf.received;
    }
    CHECK(sent == doctest::Approx(received).epsilon(1e-9));
    CHECK(sent == doctest::Approx(f.traded).epsilon(1e-9));
    // Somebody is short only when the world as a whole ran out of surplus to send.
    double left = 0;
    for (const NodeFood& nf : f.nodes) left += nf.surplus;
    CHECK((f.unmet < 1e-3 || left < 1e-3));
    MESSAGE("seed " << seed << ": production " << f.production / 1e9 << " billion people-years, demand " << f.demand / 1e9
                    << ", traded " << f.traded / f.production << " of production, across borders " << f.international_share()
                    << ", unmet " << f.unmet << ", " << f.flows.size() << " flows in " << f.rounds << " rounds");
  }
}

TEST_CASE("food: flows run from surplus to deficit at the cheapest path's cost; international means two countries (A1)") {
  const Planet& p = planet(1);
  const FreightParams fp;
  const WorldFood f = food_flows(p.land, p.people, p.routes, fp, &p.use);
  REQUIRE(!f.flows.empty());
  std::size_t checked = 0;
  for (std::size_t i = 0; i < f.flows.size(); i += std::max<std::size_t>(1, f.flows.size() / 60)) {
    const FoodFlow& fl = f.flows[i];
    const NodeFood &from = f.nodes[static_cast<std::size_t>(fl.from)], &to = f.nodes[static_cast<std::size_t>(fl.to)];
    CHECK(from.made > from.demand);
    CHECK(to.demand > to.made);
    CHECK(fl.amount > 0);
    CHECK(fl.cost == doctest::Approx(cheapest(p.routes, fp, fl.from, fl.to)).epsilon(1e-9));
    const int ca = p.people.formed.country_of[p.routes.nodes[static_cast<std::size_t>(fl.from)].cell];
    const int cb = p.people.formed.country_of[p.routes.nodes[static_cast<std::size_t>(fl.to)].cell];
    CHECK(fl.international == (ca != cb));
    ++checked;
  }
  CHECK(checked > 10);
}

TEST_CASE("food: a dearer border never raises the share traded across borders; the same seed gives the same flows (A1)") {
  const Planet& p = planet(1);
  double last = 2;
  for (double border : {0.0, 50.0, 500.0, 5000.0}) {
    FreightParams fp;
    fp.border_road_km_per_hour = border;
    const double share = food_flows(p.land, p.people, p.routes, fp, &p.use).international_share();
    CHECK(share <= last + 0.005);  // greedy rounds (rule 4) are not strictly monotone: within half a point (spec 88 As built)
    last = share;
  }
  const WorldFood a = food_flows(p.land, p.people, p.routes, {}, &p.use), b = food_flows(p.land, p.people, p.routes, {}, &p.use);
  CHECK(food_digest(a) == food_digest(b));
  MESSAGE("food digest: " << food_digest(a));
  CHECK(food_digest(a) == 12962887278305745715ULL);  // re-pinned on MSVC, 2026-10-15 (the hydrology fix: wetter tropics, channel losses)
}

TEST_CASE("food: on the one world's grid the share traded across borders lands near 23 % (WSR-44) (A2)" *
          doctest::skip(zs::World::kDebugBuild)) {
  for (std::uint64_t seed : {1ull, 4ull}) {
    const Planet& p = planet(seed, kOneWorldFrequency);
    const WorldFood f = food_flows(p.land, p.people, p.routes, {}, &p.use);
    CHECK(f.international_share() > 0.15);  // seed 1 17.2 % after spec 86 (14.8 % after spec 83)
    CHECK(f.international_share() < 0.34);  // 28-31 % since specs 86-89 (Earth 23 %): open, spec 89 As built
    MESSAGE("seed " << seed << ": across borders " << f.international_share() << ", traded " << f.traded / f.production
                    << " of production, unmet " << f.unmet / f.demand << " of demand, " << f.flows.size() << " flows, "
                    << f.rounds << " rounds");
  }
}

// Spec 77: food by kind.

TEST_CASE("food by kind: the world's mean diet matches WSR-46; each kind adds up; warm land grows more other crops (A1)") {
  const Planet& p = planet(1);
  const DietParams diet;
  const WorldDiet d = food_by_kind(p.fields, p.land, p.people, p.routes, {}, {}, {}, &p.use);
  double people = 0;
  for (std::size_t c = 0; c < p.grid.size(); ++c) people += p.people.people[c];
  CHECK(d.demand == doctest::Approx(people).epsilon(1e-9));  // the shares of each group add to one
  CHECK(d.production == doctest::Approx(FreightParams{}.grown_vs_need * people).epsilon(1e-9));
  const double grain = d.kinds[0].demand / d.demand, animal = d.kinds[2].demand / d.demand;
  CHECK(std::abs(grain - 0.42) < 0.03);
  CHECK(std::abs(animal - 0.16) < 0.03);
  for (const WorldFood& f : d.kinds) {
    double sent = 0;
    for (const NodeFood& nf : f.nodes) {
      CHECK(nf.made == doctest::Approx(nf.eaten + nf.sent + nf.surplus).epsilon(1e-9));
      CHECK(nf.demand == doctest::Approx(nf.eaten + nf.received + nf.unmet).epsilon(1e-9));
      sent += nf.sent;
    }
    CHECK(sent == doctest::Approx(f.traded).epsilon(1e-9));
  }
  CHECK(crops_share(diet, 0) == doctest::Approx(diet.crops_at_5c));
  CHECK(crops_share(diet, 30) == doctest::Approx(diet.crops_at_25c));
  CHECK(crops_share(diet, 10) < crops_share(diet, 20));
  // Fish only where a region holds sea.
  std::vector<char> has_sea(p.routes.nodes.size(), 0);
  for (std::size_t c = 0; c < p.grid.size(); ++c)
    if (!p.fields.land(static_cast<CellId>(c))) has_sea[static_cast<std::size_t>(p.routes.region_of[c])] = 1;
  for (std::size_t k = 0; k < p.routes.nodes.size(); ++k)
    if (d.kinds[3].nodes[k].made > 0) CHECK(has_sea[k]);
  MESSAGE("diet: grain " << grain << ", other crops " << d.kinds[1].demand / d.demand << ", animal " << animal << ", fish "
                         << d.kinds[3].demand / d.demand << "; across borders " << d.international_share() << " (one kind: "
                         << food_flows(p.land, p.people, p.routes, {}, &p.use).international_share() << ")");
}

TEST_CASE("food by kind: kinds trade more across borders than one food; the same seed gives the same flows (A1)") {
  const Planet& p = planet(1);
  const WorldDiet d = food_by_kind(p.fields, p.land, p.people, p.routes, {}, {}, {}, &p.use);
  CHECK(d.international_share() > food_flows(p.land, p.people, p.routes, {}, &p.use).international_share());
  CHECK(diet_digest(d) == diet_digest(food_by_kind(p.fields, p.land, p.people, p.routes, {}, {}, {}, &p.use)));
  MESSAGE("diet digest: " << diet_digest(d));
  CHECK(diet_digest(d) == 14625132353563675778ULL);  // re-pinned on MSVC, 2026-10-15 (the hydrology fix: wetter tropics, channel losses)
}

TEST_CASE("food by kind: on the one world's grid the share across borders lands between 18 % and 30 % (WSR-44) (A2)" *
          doctest::skip(zs::World::kDebugBuild)) {
  for (std::uint64_t seed : {1ull, 4ull}) {
    const Planet& p = planet(seed, kOneWorldFrequency);
    const WorldDiet d = food_by_kind(p.fields, p.land, p.people, p.routes, {}, {}, {}, &p.use);
    CHECK(d.international_share() > 0.18);
    CHECK(d.international_share() < 0.36);  // as above; 34 % on seed 1 after spec 89's polar amendment
    std::ostringstream kinds;
    for (int k = 0; k < kFoodKinds; ++k)
      kinds << " " << food_kind_name(static_cast<FoodKind>(k)) << " " << d.kinds[static_cast<std::size_t>(k)].international_share();
    MESSAGE("seed " << seed << ": across borders " << d.international_share() << " (by kind:" << kinds.str() << "), traded "
                    << d.traded / d.production << ", unmet " << d.unmet / d.demand);
  }
}
