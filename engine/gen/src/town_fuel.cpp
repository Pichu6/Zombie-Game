// Spec 97: the town's fuel stations and their depot.
#include "zs/gen/town_fuel.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <queue>
#include <stdexcept>

#include "toml.hpp"
#include "zs/gen/keyed.h"

namespace zs::gen {

namespace {
double need(const toml::node_view<const toml::node>& v, const std::string& what, const std::string& path) {
  const auto x = v.value<double>();
  if (!x || *x < 0) throw std::runtime_error(path + ": " + what + " needs a non-negative number");
  return *x;
}
double from_centre(const TownLayout& l, const Plot& p) {
  const double dx = (2.0 * p.x + p.w) / 2.0 - l.width_m / 2.0, dy = (2.0 * p.y + p.h) / 2.0 - l.height_m / 2.0;
  return std::sqrt(dx * dx + dy * dy);
}
}  // namespace

FuelRetailParams load_fuel_retail(const std::string& gen_dir) {
  const std::string path = gen_dir + "/fuel_retail.toml";
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  const toml::node_view<const toml::node> root{t};
  FuelRetailParams p;
  p.fill_minutes = need(root["errand"]["fill_minutes"], "fill_minutes", path);
  p.hand_pump_minutes = need(root["errand"]["hand_pump_minutes"], "hand_pump_minutes", path);
  for (int g = 0; g < kIncomeGroups; ++g) {
    const auto gv = root["group"][kIncomeGroupIds[static_cast<std::size_t>(g)]];
    auto& gp = p.groups[static_cast<std::size_t>(g)];
    gp.people_per_station = need(gv["people_per_station"], "people_per_station", path);
    gp.litres_per_person_day = need(gv["litres_per_person_day"], "litres_per_person_day", path);
    gp.tank_l = need(gv["tank_l"], "tank_l", path);
    gp.stock_share = need(gv["stock_share"], "stock_share", path);
    gp.generator_share = need(gv["generator_share"], "generator_share", path);
    if (gp.people_per_station <= 0 || gp.stock_share > 1 || gp.generator_share > 1)
      throw std::runtime_error(path + ": a group needs people_per_station > 0 and shares of at most 1");
  }
  return p;
}

FuelDepot fuel_depot(const WorldRoutes& routes, const WorldPeople& people, const WorldIndustry& industry, int town) {
  FuelDepot out;
  if (town < 0 || static_cast<std::size_t>(town) >= people.towns.size()) return out;
  const Settlement& t = people.towns[static_cast<std::size_t>(town)];
  if (t.cell >= routes.region_of.size() || routes.region_of[t.cell] < 0) return out;
  // Route hours from the town's region node to every node (routes run both ways).
  const int from = routes.region_of[t.cell];
  std::vector<double> hours(routes.nodes.size(), 1e300);
  using Item = std::pair<double, int>;
  std::priority_queue<Item, std::vector<Item>, std::greater<>> q;
  hours[static_cast<std::size_t>(from)] = 0;
  q.push({0.0, from});
  while (!q.empty()) {
    const auto [d, x] = q.top();
    q.pop();
    if (d > hours[static_cast<std::size_t>(x)]) continue;
    for (int r : routes.of_node[static_cast<std::size_t>(x)]) {
      const Route& route = routes.routes[static_cast<std::size_t>(r)];
      const int m = route.a == x ? route.b : route.a;
      if (d + route.hours < hours[static_cast<std::size_t>(m)]) hours[static_cast<std::size_t>(m)] = d + route.hours, q.push({d + route.hours, m});
    }
  }
  int refinery_kind = -1;
  for (std::size_t k = 0; k < industry.kinds.size(); ++k)
    if (industry.kinds[k].id == "oil_refinery") refinery_kind = static_cast<int>(k);
  // The best candidate in the town's own country first, then anywhere.
  for (const bool own : {true, false}) {
    double best = 1e300;
    for (std::size_t n = 0; n < routes.nodes.size(); ++n) {
      if (hours[n] >= best) continue;
      for (int ti : routes.nodes[n].towns) {
        const Settlement& s = people.towns[static_cast<std::size_t>(ti)];
        if (own && s.country != t.country) continue;
        bool refinery = false;
        if (refinery_kind >= 0 && static_cast<std::size_t>(ti) < industry.of_town.size())
          for (int pl : industry.of_town[static_cast<std::size_t>(ti)])
            refinery |= industry.plants[static_cast<std::size_t>(pl)].kind == refinery_kind;
        const bool port = s.site == TownSite::Port || s.site == TownSite::Mouth;
        if (!refinery && !port) continue;
        // A tanker trip takes at least an hour (loading, the town's own roads), even within the depot's route cell.
        best = hours[n], out.town = ti, out.hours = std::max(1.0, hours[n]), out.refinery = refinery;
        break;
      }
    }
    if (out.town >= 0) break;
  }
  return out;
}

TownFuel plan_town_fuel(const TownLayout& layout, double people, int income, const FuelRetailParams& params, std::uint64_t seed,
                        const FuelDepot& depot, const std::vector<int>& taken) {
  TownFuel f;
  f.income = income;
  f.depot = depot;
  const FuelRetailGroup& g = params.groups[static_cast<std::size_t>(std::clamp(income, 0, kIncomeGroups - 1))];
  f.litres_per_day = people * g.litres_per_person_day;
  const int n = std::max(1, static_cast<int>(std::lround(people / g.people_per_station)));
  // Rule 1: commercial plots from the town's edge inward (stations stand on the roads in and out), skipping the taken.
  std::vector<int> plots;
  for (std::size_t i = 0; i < layout.plots.size(); ++i) {
    const Plot& p = layout.plots[i];
    if (p.lot >= 0 || p.use != LandUse::Commercial) continue;
    if (std::find(taken.begin(), taken.end(), static_cast<int>(i)) != taken.end()) continue;
    plots.push_back(static_cast<int>(i));
  }
  std::stable_sort(plots.begin(), plots.end(), [&](int a, int b) {
    return from_centre(layout, layout.plots[static_cast<std::size_t>(a)]) > from_centre(layout, layout.plots[static_cast<std::size_t>(b)]);
  });
  // Spread the stations over the edge plots: every k-th of the outer half.
  const std::size_t outer = std::max<std::size_t>(1, plots.size() / 2);
  const Keyed rnd(seed ^ hash_str("fuel"));
  for (int i = 0; i < n; ++i) {
    FuelStation s;
    s.share = 1.0 / n;
    s.tank_l = g.tank_l;
    s.stock_l = g.tank_l * g.stock_share;
    s.generator = rnd.chance(static_cast<std::uint64_t>(i), 0, g.generator_share);
    if (!plots.empty()) {
      const std::size_t k = std::min(plots.size() - 1, static_cast<std::size_t>(i) * outer / static_cast<std::size_t>(n));
      s.plot = plots[k];
      s.address = layout.plots[static_cast<std::size_t>(s.plot)].address;
    }
    s.name = "Filling station" + (s.address.empty() ? std::string{} : ", " + s.address);
    f.stations.push_back(std::move(s));
  }
  return f;
}

}  // namespace zs::gen
