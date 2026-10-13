// Routes between towns. Spec: docs/tier0/specs/74-routes-between-towns.md
#include "zs/gen/world_routes.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <map>
#include <numeric>
#include <queue>
#include <utility>

#include "zs/gen/keyed.h"

namespace zs::gen {

namespace {

constexpr double kInf = std::numeric_limits<double>::infinity();

int country_at(const WorldPeople& people, CellId c) {
  return people.formed.country_of.empty() ? -1 : people.formed.country_of[c];
}

// Smallest index first among equal times, so the search is the same on every compiler.
using Item = std::pair<double, std::uint32_t>;
using MinQueue = std::priority_queue<Item, std::vector<Item>, std::greater<Item>>;

struct Dsu {
  std::vector<int> up;
  explicit Dsu(std::size_t n) : up(n) { std::iota(up.begin(), up.end(), 0); }
  int find(int x) {
    while (up[static_cast<std::size_t>(x)] != x) x = up[static_cast<std::size_t>(x)] = up[static_cast<std::size_t>(up[static_cast<std::size_t>(x)])];
    return x;
  }
  bool join(int a, int b) {
    a = find(a), b = find(b);
    if (a == b) return false;
    up[static_cast<std::size_t>(std::max(a, b))] = std::min(a, b);
    return true;
  }
};

// Rule 5: the network's time from `from` to `to`, searched only up to `bound`; kInf beyond it.
double bounded_hours(const WorldRoutes& w, int from, int to, double bound, std::vector<double>& dist, std::vector<int>& touched) {
  if (from == to) return 0;
  MinQueue q;
  dist[static_cast<std::size_t>(from)] = 0;
  touched.push_back(from);
  q.push({0.0, static_cast<std::uint32_t>(from)});
  double found = kInf;
  while (!q.empty()) {
    const auto [d, n] = q.top();
    q.pop();
    if (d > dist[n]) continue;
    if (static_cast<int>(n) == to) { found = d; break; }
    for (int r : w.of_node[n]) {
      const Route& route = w.routes[static_cast<std::size_t>(r)];
      const int m = route.a == static_cast<int>(n) ? route.b : route.a;
      const double nd = d + route.hours;
      if (nd > bound || nd >= dist[static_cast<std::size_t>(m)]) continue;
      if (dist[static_cast<std::size_t>(m)] == kInf) touched.push_back(m);
      dist[static_cast<std::size_t>(m)] = nd;
      q.push({nd, static_cast<std::uint32_t>(m)});
    }
  }
  for (int t : touched) dist[static_cast<std::size_t>(t)] = kInf;
  touched.clear();
  return found;
}

}  // namespace

const char* route_mode_name(RouteMode m) {
  switch (m) {
    case RouteMode::Road: return "road";
    case RouteMode::River: return "river";
    case RouteMode::Sea: return "sea";
  }
  return "?";
}

double road_kmh(const WorldFields& fields, const WorldPeople& people, const RouteParams& params, CellId c) {
  const int k = country_at(people, c);
  const int income = k >= 0 ? people.countries[static_cast<std::size_t>(k)].income : kIncomeGroups - 1;
  return params.flat_kmh[static_cast<std::size_t>(income)] * params.terrain[static_cast<std::size_t>(fields.biome[c])];
}

Step route_step(const WorldGrid& grid, const WorldFields& fields, const WorldLand& land, const WorldPeople& people,
                const RouteParams& params, CellId u, CellId v) {
  Step s;
  s.km = grid.distance_km(u, v);
  const bool lu = fields.land(u), lv = fields.land(v);
  if (!lu && !lv) {
    s.mode = RouteMode::Sea;
    s.hours = s.km / params.sea_kmh;
    return s;
  }
  if (lu != lv) {  // a port: half the step by sea, half by road
    const CellId l = lu ? u : v;
    s.mode = RouteMode::Sea;
    s.port = true;
    s.hours = params.port_hours + 0.5 * s.km / params.sea_kmh + 0.5 * s.km / road_kmh(fields, people, params, l);
    return s;
  }
  const double climb = std::abs(fields.elevation_m[u] - fields.elevation_m[v]) / 1000.0;
  const double road = s.km / std::min(road_kmh(fields, people, params, u), road_kmh(fields, people, params, v)) +
                      params.climb_hours_per_km * climb;
  s.hours = road;
  if ((land.downstream[u] == v || land.downstream[v] == u) && land.river[u] == 2 && land.river[v] == 2) {
    const double river = s.km / params.river_kmh;
    if (river < road) s.hours = river, s.mode = RouteMode::River;
  }
  const int cu = country_at(people, u), cv = country_at(people, v);
  if (cu != cv && cu >= 0 && cv >= 0) s.border = true, s.hours += params.border_hours;
  return s;
}

WorldRoutes build_routes(const WorldGrid& grid, const WorldFields& fields, const WorldLand& land, const WorldPeople& people,
                         const RouteParams& params) {
  WorldRoutes w;
  w.params = params;
  const std::size_t n = grid.size();
  w.node_of.assign(n, -1);
  // Rule 2: every town cell is a node.
  {
    std::map<CellId, std::vector<int>> by_cell;
    for (std::size_t t = 0; t < people.towns.size(); ++t) by_cell[people.towns[t].cell].push_back(static_cast<int>(t));
    for (auto& [cell, towns] : by_cell) {
      w.node_of[cell] = static_cast<int>(w.nodes.size());
      w.nodes.push_back({cell, std::move(towns)});
    }
  }
  w.of_node.assign(w.nodes.size(), {});
  if (w.nodes.size() < 2) return w;

  // Rule 3: nearest node in time for every cell, and the path back to it.
  std::vector<double> dist(n, kInf);
  std::vector<int> owner(n, -1);
  std::vector<CellId> parent(n, kNoCell);
  MinQueue q;
  for (std::size_t k = 0; k < w.nodes.size(); ++k) {
    const CellId c = w.nodes[k].cell;
    dist[c] = 0;
    owner[c] = static_cast<int>(k);
    q.push({0.0, c});
  }
  auto step_hours = [&](CellId u, CellId v) { return route_step(grid, fields, land, people, params, u, v).hours; };
  while (!q.empty()) {
    const auto [d, u] = q.top();
    q.pop();
    if (d > dist[u]) continue;
    for (CellId v : grid.neighbours(u)) {
      const double nd = d + step_hours(u, v);
      if (nd < dist[v]) {  // strict: an owner and its path change together, once a cell is settled never again
        dist[v] = nd;
        owner[v] = owner[u];
        parent[v] = u;
        q.push({nd, v});
      }
    }
  }

  // Rule 3: candidates where two regions touch, the best crossing per pair.
  struct Candidate {
    double hours;
    int a, b;
    CellId u, v;  // u in a's region, v in b's
  };
  std::map<std::pair<int, int>, Candidate> best;
  for (CellId u = 0; u < n; ++u) {
    if (owner[u] < 0) continue;
    for (CellId v : grid.neighbours(u)) {
      if (owner[v] < 0 || owner[u] >= owner[v]) continue;
      const double h = dist[u] + step_hours(u, v) + dist[v];
      const auto key = std::make_pair(owner[u], owner[v]);
      auto it = best.find(key);
      if (it == best.end() || h < it->second.hours) best[key] = {h, owner[u], owner[v], u, v};
    }
  }
  std::vector<Candidate> cands;
  cands.reserve(best.size());
  for (const auto& [key, c] : best) cands.push_back(c);
  std::stable_sort(cands.begin(), cands.end(), [](const Candidate& x, const Candidate& y) {
    if (x.hours != y.hours) return x.hours < y.hours;
    return x.a != y.a ? x.a < y.a : x.b < y.b;
  });

  auto make_route = [&](const Candidate& c, RouteWhy why) {
    Route r;
    r.a = c.a, r.b = c.b, r.why = why;
    for (CellId x = c.u; x != kNoCell; x = parent[x]) r.path.push_back(x);
    std::reverse(r.path.begin(), r.path.end());
    for (CellId x = c.v; x != kNoCell; x = parent[x]) r.path.push_back(x);
    for (std::size_t i = 1; i < r.path.size(); ++i) {
      const Step s = route_step(grid, fields, land, people, params, r.path[i - 1], r.path[i]);
      r.hours += s.hours;
      if (s.port) {
        r.km[static_cast<int>(RouteMode::Sea)] += 0.5 * s.km;
        r.km[static_cast<int>(RouteMode::Road)] += 0.5 * s.km;
      } else {
        r.km[static_cast<int>(s.mode)] += s.km;
      }
      r.crosses_border = r.crosses_border || s.border;
    }
    const int idx = static_cast<int>(w.routes.size());
    w.of_node[static_cast<std::size_t>(r.a)].push_back(idx);
    w.of_node[static_cast<std::size_t>(r.b)].push_back(idx);
    w.routes.push_back(std::move(r));
  };

  // Rule 4: the tree.
  Dsu dsu(w.nodes.size());
  std::vector<char> used(cands.size(), 0);
  for (std::size_t i = 0; i < cands.size(); ++i)
    if (dsu.join(cands[i].a, cands[i].b)) used[i] = 1, make_route(cands[i], RouteWhy::Tree);
  // Rule 5: loops where the network's detour is large.
  std::vector<double> ndist(w.nodes.size(), kInf);
  std::vector<int> touched;
  for (std::size_t i = 0; i < cands.size(); ++i) {
    if (used[i]) continue;
    const double bound = params.loop_detour * cands[i].hours;
    if (bounded_hours(w, cands[i].a, cands[i].b, bound, ndist, touched) > bound) make_route(cands[i], RouteWhy::Loop);
  }
  return w;
}

double node_hours(const WorldRoutes& w, int from, int to) {
  if (from < 0 || to < 0) return -1;
  std::vector<double> dist(w.nodes.size(), kInf);
  std::vector<int> touched;
  const double h = bounded_hours(w, from, to, kInf, dist, touched);
  return h == kInf ? -1 : h;
}

std::vector<int> node_route_path(const WorldRoutes& w, int from, int to) {
  std::vector<int> out;
  if (from < 0 || to < 0 || from == to) return out;
  std::vector<double> dist(w.nodes.size(), kInf);
  std::vector<int> via(w.nodes.size(), -1);
  MinQueue q;
  dist[static_cast<std::size_t>(from)] = 0;
  q.push({0.0, static_cast<std::uint32_t>(from)});
  while (!q.empty()) {
    const auto [d, n] = q.top();
    q.pop();
    if (d > dist[n]) continue;
    if (static_cast<int>(n) == to) break;
    for (int r : w.of_node[n]) {
      const Route& route = w.routes[static_cast<std::size_t>(r)];
      const int m = route.a == static_cast<int>(n) ? route.b : route.a;
      const double nd = d + route.hours;
      if (nd >= dist[static_cast<std::size_t>(m)]) continue;
      dist[static_cast<std::size_t>(m)] = nd;
      via[static_cast<std::size_t>(m)] = r;
      q.push({nd, static_cast<std::uint32_t>(m)});
    }
  }
  if (via[static_cast<std::size_t>(to)] < 0) return out;
  for (int n = to; n != from;) {
    const int r = via[static_cast<std::size_t>(n)];
    out.push_back(r);
    const Route& route = w.routes[static_cast<std::size_t>(r)];
    n = route.a == n ? route.b : route.a;
  }
  std::reverse(out.begin(), out.end());
  return out;
}

double road_circuity(const WorldGrid& grid, const WorldRoutes& w, double min_km, int pairs, std::uint64_t seed) {
  double sum = 0;
  int got = 0;
  const auto nodes = static_cast<std::uint64_t>(w.nodes.size());
  for (std::uint64_t k = 0; got < pairs && k < static_cast<std::uint64_t>(pairs) * 50 && nodes >= 2; ++k) {
    const int a = static_cast<int>(mix64(seed, 2 * k) % nodes), b = static_cast<int>(mix64(seed, 2 * k + 1) % nodes);
    const double straight = grid.distance_km(w.nodes[static_cast<std::size_t>(a)].cell, w.nodes[static_cast<std::size_t>(b)].cell);
    if (a == b || straight < min_km || straight > 4 * min_km) continue;
    double km = 0;
    bool road_only = true;
    for (int r : node_route_path(w, a, b)) {
      const Route& route = w.routes[static_cast<std::size_t>(r)];
      road_only = road_only && route.km[static_cast<int>(RouteMode::Sea)] == 0;
      km += route.km[0] + route.km[1] + route.km[2];
    }
    if (!road_only || km == 0) continue;
    sum += km / straight;
    ++got;
  }
  return got ? sum / got : 0;
}

double town_hours(const WorldGrid& grid, const WorldFields& fields, const WorldPeople& people, const WorldRoutes& routes,
                  int from, int to) {
  if (from == to) return 0;
  const CellId a = people.towns[static_cast<std::size_t>(from)].cell, b = people.towns[static_cast<std::size_t>(to)].cell;
  if (a == b) return 0.5 * std::sqrt(grid.area_km2(a)) / road_kmh(fields, people, routes.params, a);  // rule 2
  return node_hours(routes, routes.node_of[a], routes.node_of[b]);
}

std::uint64_t routes_digest(const WorldRoutes& w) {
  std::uint64_t h = mix64(w.nodes.size(), w.routes.size());
  for (const Route& r : w.routes) {
    std::uint64_t bits = 0;
    std::memcpy(&bits, &r.hours, sizeof bits);
    h = mix64(h, mix64(static_cast<std::uint64_t>(r.a) << 32 | static_cast<std::uint32_t>(r.b), bits ^ r.path.size()));
  }
  return h;
}

}  // namespace zs::gen
