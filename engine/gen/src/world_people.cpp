// People, sectors, countries and towns on the world grid. Spec: docs/tier0/specs/48-people-sectors-countries.md
#include "zs/gen/world_people.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>
#include <queue>
#include <stdexcept>

#include "fields.h"
#include "toml.hpp"
#include "zs/detmath.h"
#include "zs/gen/keyed.h"

namespace zs::gen {

namespace {

constexpr double kAlpha = 1.5;            // rule 2: concentration (estimate)
constexpr double kSeaCost = 4.0;          // rules 3, 4: a sea step costs this times its distance (estimate)
constexpr double kClimbPerM = 1.0 / 500;  // rule 3: a land step costs (1 + |dh| / 500 m) times its distance (estimate)
constexpr double kSpacing = 0.7;          // rules 3, 4: seeds at least this times the root of the target area apart
constexpr int kSectorsPerCountry = 3;     // rule 4 (design)

double dist_km(const WorldGrid& g, const Vec3& a, const Vec3& b) { return angle_between(a, b) * g.radius_km(); }

Vec3 centroid(const WorldGrid& g, const std::vector<CellId>& cells) {
  Vec3 s;
  for (CellId c : cells) {
    const double a = g.area_km2(c);
    s.x += g.centre(c).x * a;
    s.y += g.centre(c).y * a;
    s.z += g.centre(c).z * a;
  }
  const double n = detmath::sqrt(dot(s, s));
  return n > 0 ? Vec3{s.x / n, s.y / n, s.z / n} : Vec3{0, 0, 1};
}

double step_cost(const WorldGrid& g, const WorldFields& f, CellId a, CellId b) {
  const double d = g.distance_km(a, b);
  if (!f.land(a) || !f.land(b)) return kSeaCost * d;
  return d * (1.0 + std::abs(f.elevation_m[a] - f.elevation_m[b]) * kClimbPerM);
}

// Multi-source shortest paths over the cells; ties by the lower source index, then the lower cell id.
std::vector<int> nearest_source(const WorldGrid& g, const WorldFields& f, const std::vector<CellId>& sources) {
  const std::size_t n = g.size();
  std::vector<double> best(n, 1e300);
  std::vector<int> from(n, -1);
  using Item = std::tuple<double, int, CellId>;
  std::priority_queue<Item, std::vector<Item>, std::greater<>> q;
  for (std::size_t s = 0; s < sources.size(); ++s) {
    best[sources[s]] = 0;
    from[sources[s]] = static_cast<int>(s);
    q.emplace(0.0, static_cast<int>(s), sources[s]);
  }
  while (!q.empty()) {
    const auto [d, s, c] = q.top();
    q.pop();
    if (d > best[c] || s != from[c]) continue;
    for (CellId nb : g.neighbours(c)) {
      const double nd = d + step_cost(g, f, c, nb);
      if (nd < best[nb] || (nd == best[nb] && s < from[nb])) {
        best[nb] = nd;
        from[nb] = s;
        q.emplace(nd, s, nb);
      }
    }
  }
  return from;
}

// Greedy spaced seeds: candidates in order, each at least `spacing` km from the chosen; then fills up without spacing.
std::vector<std::size_t> spaced(const std::vector<Vec3>& at, const std::vector<std::size_t>& order, std::size_t count,
                                double spacing, const WorldGrid& g) {
  std::vector<std::size_t> chosen;
  for (std::size_t i : order) {
    if (chosen.size() == count) break;
    bool ok = true;
    for (std::size_t c : chosen) ok = ok && dist_km(g, at[i], at[c]) >= spacing;
    if (ok) chosen.push_back(i);
  }
  for (std::size_t i : order) {
    if (chosen.size() == count) break;
    if (std::find(chosen.begin(), chosen.end(), i) == chosen.end()) chosen.push_back(i);
  }
  return chosen;
}

}  // namespace

GroupTables load_group_tables(const std::string& data_dir) {
  const toml::table t = toml::parse_file(data_dir + "/setup/world/wpp2024.toml");
  GroupTables out;
  out.world_population = t["world"]["population"].value<double>().value();
  for (int i = 0; i < kIncomeGroups; ++i) {
    const auto* g = t[kIncomeGroupIds[static_cast<std::size_t>(i)]].as_table();
    if (!g) throw std::runtime_error(std::string("world tables: no group ") + kIncomeGroupIds[static_cast<std::size_t>(i)]);
    out.population[static_cast<std::size_t>(i)] = (*g)["population"].value<double>().value();
    out.urban_share[static_cast<std::size_t>(i)] = (*g)["urban_share"].value<double>().value();
  }
  return out;
}

double suitability(const WorldFields& f, CellId c) {
  if (!f.land(c) || f.biome[c] == Biome::Ice) return 0.0;
  const double t = (f.temp_mean_c[c] - 13.0) / 7.0;
  const double rain = f.rain_mm[c];
  const double coast = f.sea_cells[c] <= 1 ? 1.5 : 1.0;
  return detmath::exp(-t * t) * (rain / (rain + 500.0)) * detmath::exp(-std::max(0.0, f.elevation_m[c]) / 1500.0) * coast;
}

WorldPeople populate_world(const WorldGrid& g, const WorldFields& f, std::uint64_t seed, const GroupTables& tables) {
  const std::size_t n = g.size();
  WorldPeople w;
  w.suitability.assign(n, 0.0);
  w.people.assign(n, 0.0);
  w.sector_of.assign(n, -1);

  // Rules 1, 2: suitability and people.
  double weight_sum = 0;
  std::vector<double> weight(n, 0.0);
  for (CellId c = 0; c < n; ++c) {
    if (f.land(c)) w.land_km2 += g.area_km2(c);
    w.suitability[c] = suitability(f, c);
    weight[c] = g.area_km2(c) * detmath::pow(std::max(w.suitability[c], 1e-300), kAlpha) * (w.suitability[c] > 0 ? 1 : 0);
    weight_sum += weight[c];
  }
  const double total = tables.world_population * w.land_km2 / kEarthLandKm2;
  for (CellId c = 0; c < n; ++c) {
    w.people[c] = weight_sum > 0 ? std::round(total * weight[c] / weight_sum) : 0.0;
    w.total_people += w.people[c];
  }

  // Rule 3: sectors.
  std::vector<CellId> land;
  for (CellId c = 0; c < n; ++c)
    if (f.land(c)) land.push_back(c);
  if (land.empty()) return w;
  const std::size_t sector_count = std::max<std::size_t>(1, static_cast<std::size_t>(std::llround(w.land_km2 / kSectorTargetKm2)));
  std::vector<std::size_t> order(land.size());
  std::iota(order.begin(), order.end(), std::size_t{0});
  std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return w.people[land[a]] > w.people[land[b]]; });
  std::vector<Vec3> land_at;
  for (CellId c : land) land_at.push_back(g.centre(c));
  const double spacing = kSpacing * detmath::sqrt(kSectorTargetKm2);
  std::vector<CellId> seeds;
  for (std::size_t i : spaced(land_at, order, std::min(sector_count, land.size()), spacing, g)) seeds.push_back(land[i]);
  const std::vector<int> from = nearest_source(g, f, seeds);
  w.sectors.resize(seeds.size());
  for (std::size_t s = 0; s < seeds.size(); ++s) w.sectors[s].seed = seeds[s];
  for (CellId c : land) {
    const int s = from[c];
    w.sector_of[c] = s;
    Sector& sec = w.sectors[static_cast<std::size_t>(s)];
    sec.cells.push_back(c);
    sec.people += w.people[c];
    sec.area_km2 += g.area_km2(c);
  }

  // Rule 4: countries over the sector graph (land adjacency; an island sector links to its nearest sector by sea).
  const std::size_t ns = w.sectors.size();
  std::vector<Vec3> sector_at;
  for (const Sector& s : w.sectors) sector_at.push_back(centroid(g, s.cells));
  std::vector<std::vector<std::pair<int, double>>> adj(ns);
  auto link = [&](int a, int b, double cost) {
    for (const auto& [x, c] : adj[static_cast<std::size_t>(a)])
      if (x == b) return;
    adj[static_cast<std::size_t>(a)].push_back({b, cost});
    adj[static_cast<std::size_t>(b)].push_back({a, cost});
  };
  for (CellId c : land)
    for (CellId nb : g.neighbours(c))
      if (f.land(nb) && w.sector_of[nb] != w.sector_of[c])
        link(w.sector_of[c], w.sector_of[nb], dist_km(g, sector_at[static_cast<std::size_t>(w.sector_of[c])], sector_at[static_cast<std::size_t>(w.sector_of[nb])]));
  // Join the components: repeatedly link the closest pair of sectors in different components by sea.
  std::vector<int> comp(ns, -1);
  auto label = [&]() {
    std::fill(comp.begin(), comp.end(), -1);
    int k = 0;
    for (std::size_t s = 0; s < ns; ++s) {
      if (comp[s] >= 0) continue;
      std::vector<std::size_t> stack{s};
      comp[s] = k;
      while (!stack.empty()) {
        const std::size_t x = stack.back();
        stack.pop_back();
        for (const auto& [y, c] : adj[x])
          if (comp[static_cast<std::size_t>(y)] < 0) {
            comp[static_cast<std::size_t>(y)] = k;
            stack.push_back(static_cast<std::size_t>(y));
          }
      }
      ++k;
    }
    return k;
  };
  while (label() > 1) {
    double best = 1e300;
    std::pair<int, int> pair{0, 0};
    for (std::size_t a = 0; a < ns; ++a)
      for (std::size_t b = a + 1; b < ns; ++b)
        if (comp[a] != comp[b]) {
          const double d = dist_km(g, sector_at[a], sector_at[b]);
          if (d < best) { best = d; pair = {static_cast<int>(a), static_cast<int>(b)}; }
        }
    link(pair.first, pair.second, kSeaCost * best);
  }
  const std::size_t country_count = std::max<std::size_t>(1, static_cast<std::size_t>(std::llround(static_cast<double>(ns) / kSectorsPerCountry)));
  std::vector<std::size_t> sector_order(ns);
  std::iota(sector_order.begin(), sector_order.end(), std::size_t{0});
  std::stable_sort(sector_order.begin(), sector_order.end(), [&](std::size_t a, std::size_t b) { return w.sectors[a].people > w.sectors[b].people; });
  const std::vector<std::size_t> capitals = spaced(sector_at, sector_order, country_count, spacing * detmath::sqrt(kSectorsPerCountry), g);
  {
    std::vector<double> best(ns, 1e300);
    using Item = std::tuple<double, int, int>;
    std::priority_queue<Item, std::vector<Item>, std::greater<>> q;
    for (std::size_t k = 0; k < capitals.size(); ++k) {
      best[capitals[k]] = 0;
      w.sectors[capitals[k]].country = static_cast<int>(k);
      q.emplace(0.0, static_cast<int>(k), static_cast<int>(capitals[k]));
    }
    while (!q.empty()) {
      const auto [d, k, s] = q.top();
      q.pop();
      if (d > best[static_cast<std::size_t>(s)] || k != w.sectors[static_cast<std::size_t>(s)].country) continue;
      for (const auto& [t, c] : adj[static_cast<std::size_t>(s)]) {
        const double nd = d + c;
        Sector& to = w.sectors[static_cast<std::size_t>(t)];
        if (nd < best[static_cast<std::size_t>(t)] || (nd == best[static_cast<std::size_t>(t)] && k < to.country)) {
          best[static_cast<std::size_t>(t)] = nd;
          to.country = k;
          q.emplace(nd, k, t);
        }
      }
    }
  }
  w.countries.resize(capitals.size());
  const Keyed r(seed);
  for (std::size_t k = 0; k < capitals.size(); ++k) w.countries[k].capital = static_cast<int>(capitals[k]);
  for (std::size_t s = 0; s < ns; ++s) {
    Country& c = w.countries[static_cast<std::size_t>(w.sectors[s].country)];
    c.sectors.push_back(static_cast<int>(s));
    c.people += w.sectors[s].people;
  }

  // Rule 5: income groups by development rank, filled to the groups' shares of people.
  for (std::size_t k = 0; k < w.countries.size(); ++k) {
    Country& c = w.countries[k];
    double s_sum = 0, a_sum = 0;
    for (int s : c.sectors)
      for (CellId cell : w.sectors[static_cast<std::size_t>(s)].cells) {
        s_sum += w.suitability[cell] * g.area_km2(cell);
        a_sum += g.area_km2(cell);
      }
    c.development = (a_sum > 0 ? s_sum / a_sum : 0) * (0.5 + r.u01(k, field::Development));
  }
  std::vector<std::size_t> by_dev(w.countries.size());
  std::iota(by_dev.begin(), by_dev.end(), std::size_t{0});
  std::stable_sort(by_dev.begin(), by_dev.end(), [&](std::size_t a, std::size_t b) { return w.countries[a].development > w.countries[b].development; });
  const double group_total = std::accumulate(tables.population.begin(), tables.population.end(), 0.0);
  double before = 0;
  for (std::size_t k : by_dev) {
    Country& c = w.countries[k];
    const double mid = (before + c.people / 2) / std::max(1.0, w.total_people);
    double edge = 0;
    c.income = kIncomeGroups - 1;
    for (int i = 0; i < kIncomeGroups; ++i) {
      edge += tables.population[static_cast<std::size_t>(i)] / group_total;
      if (mid < edge) { c.income = i; break; }
    }
    before += c.people;
  }

  // Rule 6: towns by rank-size in each country's most populous cells.
  for (std::size_t k = 0; k < w.countries.size(); ++k) {
    const Country& c = w.countries[k];
    const double urban = c.people * tables.urban_share[static_cast<std::size_t>(c.income)];
    // The sizes of K towns sum to 10,000 K^(1/zeta) H_K, H_K = sum of i^(-1/zeta): grows with K, so take the last K
    // that still fits.
    int count = 0;
    double harmonic = 0;
    for (int K = 1; K <= 1'000'000; ++K) {
      harmonic += detmath::pow(K, -1.0 / kRankSizeZeta);
      if (kTownMinPeople * detmath::pow(K, 1.0 / kRankSizeZeta) * harmonic > urban) break;
      count = K;
    }
    if (count == 0) continue;
    std::vector<std::pair<double, CellId>> left;  // people not yet in a town, by cell
    for (int s : c.sectors)
      for (CellId cell : w.sectors[static_cast<std::size_t>(s)].cells) left.push_back({w.people[cell], cell});
    const double first = kTownMinPeople * detmath::pow(count, 1.0 / kRankSizeZeta);
    for (int i = 1; i <= count; ++i) {
      auto it = std::max_element(left.begin(), left.end(), [](const auto& a, const auto& b) {
        return a.first < b.first || (a.first == b.first && a.second > b.second);
      });
      const double size = std::round(first * detmath::pow(i, -1.0 / kRankSizeZeta));
      w.towns.push_back({it->second, static_cast<int>(k), i, size});
      it->first -= size;
    }
  }
  return w;
}

}  // namespace zs::gen
