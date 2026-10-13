// Food flows on the routes. Spec: docs/tier0/specs/76-food-flows-on-the-routes.md
#include "zs/gen/world_food.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <utility>

#include "zs/gen/keyed.h"

namespace zs::gen {

namespace {

constexpr double kInf = std::numeric_limits<double>::infinity();
constexpr double kEps = 1e-6;  // a millionth of a person's year of food: below it, nothing is left

int country_of_node(const WorldPeople& people, const WorldRoutes& routes, int node) {
  return people.formed.country_of.empty() ? -1 : people.formed.country_of[routes.nodes[static_cast<std::size_t>(node)].cell];
}

}  // namespace

double freight_cost(const Route& r, const FreightParams& p) {
  return r.km[static_cast<int>(RouteMode::Road)] * p.road + r.km[static_cast<int>(RouteMode::River)] * p.river +
         r.km[static_cast<int>(RouteMode::Sea)] * p.sea + r.ports * p.port_road_km + r.borders * p.border_road_km;
}

WorldFood food_flows(const WorldLand& land, const WorldPeople& people, const WorldRoutes& routes, const FreightParams& params) {
  WorldFood f;
  const std::size_t n = routes.nodes.size();
  f.nodes.assign(n, {});
  // Rule 1: production and demand by region. What is grown is each cell's share of the land's capacity, scaled so the
  // world grows `grown_vs_need` times what its people need.
  double capacity = 0, need = 0;
  for (std::size_t c = 0; c < routes.region_of.size(); ++c) capacity += land.capacity_food[c], need += people.people[c];
  const double grown = capacity > 0 ? params.grown_vs_need * need / capacity : 0;
  for (std::size_t c = 0; c < routes.region_of.size(); ++c) {
    const int k = routes.region_of[c];
    if (k < 0) continue;
    NodeFood& nf = f.nodes[static_cast<std::size_t>(k)];
    nf.made += land.capacity_food[c] * grown;
    nf.demand += people.people[c];
  }
  // Rule 2: home first.
  for (NodeFood& nf : f.nodes) {
    nf.eaten = std::min(nf.made, nf.demand);
    nf.surplus = nf.made - nf.eaten;
    nf.unmet = nf.demand - nf.eaten;
    f.production += nf.made;
    f.demand += nf.demand;
  }
  std::vector<double> cost(routes.routes.size());
  for (std::size_t r = 0; r < cost.size(); ++r) cost[r] = freight_cost(routes.routes[r], params);
  // Rule 4: cheapest supplier first, in rounds.
  std::map<std::pair<int, int>, std::size_t> flow_of;
  std::vector<double> dist(n);
  std::vector<int> src(n);
  using Item = std::pair<double, int>;
  for (;;) {
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> q;
    std::fill(dist.begin(), dist.end(), kInf);
    std::fill(src.begin(), src.end(), -1);
    bool any_deficit = false;
    for (std::size_t k = 0; k < n; ++k) {
      any_deficit = any_deficit || f.nodes[k].unmet > kEps;
      if (f.nodes[k].surplus > kEps) dist[k] = 0, src[k] = static_cast<int>(k), q.push({0.0, static_cast<int>(k)});
    }
    if (q.empty() || !any_deficit) break;
    ++f.rounds;
    while (!q.empty()) {
      const auto [d, u] = q.top();
      q.pop();
      if (d > dist[static_cast<std::size_t>(u)]) continue;
      for (int r : routes.of_node[static_cast<std::size_t>(u)]) {
        const Route& route = routes.routes[static_cast<std::size_t>(r)];
        const int v = route.a == u ? route.b : route.a;
        const double nd = d + cost[static_cast<std::size_t>(r)];
        if (nd < dist[static_cast<std::size_t>(v)]) {
          dist[static_cast<std::size_t>(v)] = nd;
          src[static_cast<std::size_t>(v)] = src[static_cast<std::size_t>(u)];
          q.push({nd, v});
        }
      }
    }
    std::vector<int> order;
    for (std::size_t k = 0; k < n; ++k)
      if (f.nodes[k].unmet > kEps && src[k] >= 0) order.push_back(static_cast<int>(k));
    if (order.empty()) break;
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
      return dist[static_cast<std::size_t>(a)] != dist[static_cast<std::size_t>(b)] ? dist[static_cast<std::size_t>(a)] < dist[static_cast<std::size_t>(b)] : a < b;
    });
    bool progress = false;
    for (int d : order) {
      const int s = src[static_cast<std::size_t>(d)];
      NodeFood& sup = f.nodes[static_cast<std::size_t>(s)];
      NodeFood& dem = f.nodes[static_cast<std::size_t>(d)];
      if (sup.surplus <= kEps) continue;  // ran out this round: searched again in the next
      const double amount = std::min(sup.surplus, dem.unmet);
      sup.surplus -= amount;
      sup.sent += amount;
      dem.unmet -= amount;
      dem.received += amount;
      progress = true;
      const auto key = std::make_pair(s, d);
      auto it = flow_of.find(key);
      if (it == flow_of.end()) {
        flow_of.emplace(key, f.flows.size());
        f.flows.push_back({s, d, amount, dist[static_cast<std::size_t>(d)],
                           country_of_node(people, routes, s) != country_of_node(people, routes, d)});
      } else {
        f.flows[it->second].amount += amount;
      }
    }
    if (!progress) break;
  }
  // Rule 5: totals.
  for (const FoodFlow& fl : f.flows) {
    f.traded += fl.amount;
    if (fl.international) f.international += fl.amount;
  }
  for (const NodeFood& nf : f.nodes) f.unmet += nf.unmet;
  return f;
}

std::uint64_t food_digest(const WorldFood& food) {
  std::uint64_t h = mix64(food.nodes.size(), food.flows.size());
  for (const FoodFlow& fl : food.flows) {
    std::uint64_t bits = 0;
    std::memcpy(&bits, &fl.amount, sizeof bits);
    h = mix64(h, mix64(static_cast<std::uint64_t>(fl.from) << 32 | static_cast<std::uint32_t>(fl.to), bits));
  }
  return h;
}

}  // namespace zs::gen
