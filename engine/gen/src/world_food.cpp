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
         r.km[static_cast<int>(RouteMode::Sea)] * p.sea + r.ports * p.port_road_km + r.border_hours * p.border_road_km_per_hour;
}

namespace {

// Spec 76 rules 2-5 on nodes whose `made` and `demand` are set: home first, then the cheapest supplier, in rounds.
WorldFood flows_of(std::vector<NodeFood> nodes, const WorldPeople& people, const WorldRoutes& routes, const FreightParams& params);

}  // namespace

WorldFood food_flows(const WorldLand& land, const WorldPeople& people, const WorldRoutes& routes, const FreightParams& params) {
  std::vector<NodeFood> nodes(routes.nodes.size());
  // Rule 1: production and demand by region. What is grown is each cell's share of the land's capacity, scaled so the
  // world grows `grown_vs_need` times what its people need.
  double capacity = 0, need = 0;
  for (std::size_t c = 0; c < routes.region_of.size(); ++c) capacity += land.capacity_food[c], need += people.people[c];
  const double grown = capacity > 0 ? params.grown_vs_need * need / capacity : 0;
  for (std::size_t c = 0; c < routes.region_of.size(); ++c) {
    const int k = routes.region_of[c];
    if (k < 0) continue;
    NodeFood& nf = nodes[static_cast<std::size_t>(k)];
    nf.made += land.capacity_food[c] * grown;
    nf.demand += people.people[c];
  }
  return flows_of(std::move(nodes), people, routes, params);
}

namespace {

WorldFood flows_of(std::vector<NodeFood> nodes, const WorldPeople& people, const WorldRoutes& routes, const FreightParams& params) {
  WorldFood f;
  const std::size_t n = nodes.size();
  f.nodes = std::move(nodes);
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

}  // namespace

const char* food_kind_name(FoodKind k) {
  switch (k) {
    case FoodKind::Grain: return "grain";
    case FoodKind::Crops: return "other crops";
    case FoodKind::Animal: return "animal foods";
    case FoodKind::Fish: return "fish";
  }
  return "?";
}

double crops_share(const DietParams& d, double temp_c) {
  const double t = (temp_c - 5.0) / 20.0;
  return d.crops_at_5c + (d.crops_at_25c - d.crops_at_5c) * (t < 0 ? 0 : t > 1 ? 1 : t);
}

WorldDiet food_by_kind(const WorldFields& fields, const WorldLand& land, const WorldPeople& people, const WorldRoutes& routes,
                       const FreightParams& params, const DietParams& diet, const LandParams& lp) {
  const std::size_t cells = routes.region_of.size();
  // Rule 1: each cell's potential for each kind (the kind's own units; rule 2 scales them).
  std::array<std::vector<double>, kFoodKinds> potential;
  std::array<std::vector<double>, kFoodKinds> need;
  for (int k = 0; k < kFoodKinds; ++k) potential[static_cast<std::size_t>(k)].assign(cells, 0), need[static_cast<std::size_t>(k)].assign(cells, 0);
  for (std::size_t c = 0; c < cells; ++c) {
    const double crop = land.arable_share[c] * (1 - lp.feed_share), pasture = land.pasture_share[c] * lp.pasture_vs_crop;
    if (land.capacity_food[c] > 0 && crop + pasture > 0) {
      const double unit = land.capacity_food[c] / (crop + pasture);  // spec 52's capacity, per unit of its two terms
      const double warm = crops_share(diet, fields.temp_mean_c[c]);
      potential[0][c] = unit * crop * (1 - warm);
      potential[1][c] = unit * crop * warm;
      potential[2][c] = unit * (pasture + land.arable_share[c] * lp.feed_share * diet.feed_conversion);
    }
    potential[3][c] = land.fish_catch_t[c];
    // Rule 3: what each people eats, by its country's income group.
    const int country = people.formed.country_of.empty() ? -1 : people.formed.country_of[c];
    const int group = country >= 0 ? people.countries[static_cast<std::size_t>(country)].income : kIncomeGroups - 1;
    for (int k = 0; k < kFoodKinds; ++k)
      need[static_cast<std::size_t>(k)][c] = people.people[c] * diet.share[static_cast<std::size_t>(group)][static_cast<std::size_t>(k)];
  }
  WorldDiet out;
  for (int k = 0; k < kFoodKinds; ++k) {
    const auto& pot = potential[static_cast<std::size_t>(k)];
    const auto& nd = need[static_cast<std::size_t>(k)];
    double total_pot = 0, total_need = 0;
    for (std::size_t c = 0; c < cells; ++c) total_pot += pot[c], total_need += nd[c];
    const double grown = total_pot > 0 ? params.grown_vs_need * total_need / total_pot : 0;  // rule 2
    std::vector<NodeFood> nodes(routes.nodes.size());
    for (std::size_t c = 0; c < cells; ++c) {
      const int r = routes.region_of[c];
      if (r < 0) continue;
      nodes[static_cast<std::size_t>(r)].made += pot[c] * grown;
      nodes[static_cast<std::size_t>(r)].demand += nd[c];
    }
    WorldFood& f = out.kinds[static_cast<std::size_t>(k)] = flows_of(std::move(nodes), people, routes, params);  // rule 4
    out.production += f.production;
    out.demand += f.demand;
    out.international += f.international;
    out.traded += f.traded;
    out.unmet += f.unmet;
  }
  return out;
}

std::uint64_t diet_digest(const WorldDiet& diet) {
  std::uint64_t h = 0;
  for (const WorldFood& f : diet.kinds) h = mix64(h, food_digest(f));
  return h;
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
