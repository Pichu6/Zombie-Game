// Spec 79: the town's food. docs/tier0/specs/79-the-towns-food-shops-and-pantries.md (research food-retail-and-stocks, FR)
#include "zs/gen/town_food.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>

#include "toml.hpp"
#include "zs/gen/keyed.h"

namespace zs::gen {

namespace {

constexpr std::array<const char*, kOutletKinds> kKindIds = {"supermarket", "convenience", "small", "market"};

double need(const toml::node_view<const toml::node>& v, const std::string& what, const std::string& path) {
  const auto x = v.value<double>();
  if (!x || *x < 0) throw std::runtime_error(path + ": " + what + " needs a non-negative number");
  return *x;
}

// The plot's centre's distance from the town's centre, in metres.
double from_centre(const TownLayout& l, const Plot& p) {
  const double dx = (2.0 * p.x + p.w) / 2.0 - l.width_m / 2.0, dy = (2.0 * p.y + p.h) / 2.0 - l.height_m / 2.0;
  return std::sqrt(dx * dx + dy * dy);
}

}  // namespace

const char* outlet_kind_name(OutletKind k) {
  switch (k) {
    case OutletKind::Supermarket: return "supermarket";
    case OutletKind::Convenience: return "convenience store";
    case OutletKind::Small: return "small shop";
    case OutletKind::Market: return "market";
  }
  return "?";
}

FoodRetailParams load_food_retail(const std::string& gen_dir) {
  const std::string path = gen_dir + "/food_retail.toml";
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  const toml::node_view<const toml::node> root{t};
  FoodRetailParams p;
  for (int k = 0; k < kOutletKinds; ++k) {
    const auto kv = root["kind"][kKindIds[static_cast<std::size_t>(k)]];
    auto& kp = p.kinds[static_cast<std::size_t>(k)];
    kp.floor_m2 = need(kv["floor_m2"], "floor_m2", path);
    kp.shelf_days = need(kv["shelf_days"], "shelf_days", path);
    kp.back_days = need(kv["back_days"], "back_days", path);
  }
  for (int g = 0; g < kIncomeGroups; ++g) {
    const auto gv = root["group"][kIncomeGroupIds[static_cast<std::size_t>(g)]];
    auto& gp = p.groups[static_cast<std::size_t>(g)];
    double total = 0;
    for (int k = 0; k < kOutletKinds; ++k) {
      gp.per_10k[static_cast<std::size_t>(k)] = need(gv["per_10k"][kKindIds[static_cast<std::size_t>(k)]], "per_10k", path);
      total += gp.share[static_cast<std::size_t>(k)] = need(gv["share"][kKindIds[static_cast<std::size_t>(k)]], "share", path);
    }
    if (std::abs(total - 1.0) > 1e-6) throw std::runtime_error(path + ": a group's shares must add to 1");
    gp.pantry_days = need(gv["pantry_days"], "pantry_days", path);
    gp.fridge_days = need(gv["fridge_days"], "fridge_days", path);
    gp.trips_per_week = need(gv["trips_per_week"], "trips_per_week", path);
  }
  const auto d = root["distribution"];
  p.dc_people_served = need(d["people_served"], "people_served", path);
  p.dc_max_km = need(d["max_km"], "max_km", path);
  p.dc_min_town_people = need(d["min_town_people"], "min_town_people", path);
  p.dc_dry_days = need(d["dry_days"], "dry_days", path);
  p.dc_fresh_days = need(d["fresh_days"], "fresh_days", path);
  return p;
}

ServingCentre serving_centre(const WorldGrid&, const WorldFields&, const WorldPeople& people, const WorldRoutes& routes, int town,
                             const FoodRetailParams& params) {
  ServingCentre out;
  if (town < 0) return out;
  if (people.towns[static_cast<std::size_t>(town)].people >= params.dc_min_town_people) {
    out.town = town;
    return out;
  }
  const int from = routes.node_of[people.towns[static_cast<std::size_t>(town)].cell];
  if (from < 0) return out;
  // Rule 5: the nearest node by road time holding a town large enough; its largest such town.
  const std::size_t n = routes.nodes.size();
  std::vector<double> hours(n, std::numeric_limits<double>::infinity()), km(n, 0);
  using Item = std::pair<double, int>;
  std::priority_queue<Item, std::vector<Item>, std::greater<Item>> q;
  hours[static_cast<std::size_t>(from)] = 0;
  q.push({0.0, from});
  while (!q.empty()) {
    const auto [h, u] = q.top();
    q.pop();
    if (h > hours[static_cast<std::size_t>(u)]) continue;
    int best = -1;
    for (int t : routes.nodes[static_cast<std::size_t>(u)].towns)
      if (people.towns[static_cast<std::size_t>(t)].people >= params.dc_min_town_people &&
          (best < 0 || people.towns[static_cast<std::size_t>(t)].people > people.towns[static_cast<std::size_t>(best)].people))
        best = t;
    if (best >= 0) {
      out.town = best;
      out.hours = h;
      out.km = km[static_cast<std::size_t>(u)];
      return out;
    }
    for (int r : routes.of_node[static_cast<std::size_t>(u)]) {
      const Route& route = routes.routes[static_cast<std::size_t>(r)];
      const int v = route.a == u ? route.b : route.a;
      const double nh = h + route.hours;
      if (nh < hours[static_cast<std::size_t>(v)]) {
        hours[static_cast<std::size_t>(v)] = nh;
        km[static_cast<std::size_t>(v)] = km[static_cast<std::size_t>(u)] + route.km[0] + route.km[1] + route.km[2];
        q.push({nh, v});
      }
    }
  }
  return out;
}

ServingCentre nearest_centre(const WorldGrid& grid, const WorldFields& fields, const WorldPeople& people, int town,
                             const FoodRetailParams& params) {
  ServingCentre out;
  if (town < 0) return out;
  const Settlement& t = people.towns[static_cast<std::size_t>(town)];
  if (t.people >= params.dc_min_town_people) {
    out.town = town;
    return out;
  }
  double best = std::numeric_limits<double>::infinity();
  for (std::size_t i = 0; i < people.towns.size(); ++i) {
    if (people.towns[i].people < params.dc_min_town_people) continue;
    const double d = grid.distance_km(t.cell, people.towns[i].cell);
    if (d < best) best = d, out.town = static_cast<int>(i);
  }
  if (out.town < 0) return out;
  out.km = best * kRoadCircuity;
  out.hours = out.km / road_kmh(fields, people, RouteParams{}, t.cell);
  return out;
}

TownFood plan_town_food(const Town& town, const TownLayout& layout, double people, int income, const FoodRetailParams& params,
                        std::uint64_t seed, const ServingCentre& centre) {
  TownFood f;
  f.income = income;
  f.centre = centre;
  f.people_days_per_day = people;
  const FoodRetailGroup& g = params.groups[static_cast<std::size_t>(income)];
  f.pantry_days = g.pantry_days;
  f.fridge_days = g.fridge_days;
  f.trips_per_week = g.trips_per_week;
  // Rule 1: how many of each kind; the group's main kind (its largest share) has at least one.
  std::array<int, kOutletKinds> count{};
  int main_kind = 0;
  for (int k = 0; k < kOutletKinds; ++k) {
    count[static_cast<std::size_t>(k)] = static_cast<int>(std::lround(people / 10000.0 * g.per_10k[static_cast<std::size_t>(k)]));
    if (g.share[static_cast<std::size_t>(k)] > g.share[static_cast<std::size_t>(main_kind)]) main_kind = k;
  }
  count[static_cast<std::size_t>(main_kind)] = std::max(1, count[static_cast<std::size_t>(main_kind)]);
  // Rule 2: a kind with no outlet in this town passes its share to the main kind.
  std::array<double, kOutletKinds> share = g.share;
  for (int k = 0; k < kOutletKinds; ++k)
    if (count[static_cast<std::size_t>(k)] == 0 && k != main_kind)
      share[static_cast<std::size_t>(main_kind)] += share[static_cast<std::size_t>(k)], share[static_cast<std::size_t>(k)] = 0;
  // Rule 3: the plots, in order of preference.
  std::vector<int> commercial, homes, open;
  for (std::size_t i = 0; i < layout.plots.size(); ++i) {
    const Plot& p = layout.plots[i];
    if (p.lot >= 0) continue;  // a station or the block
    if (p.use == LandUse::Commercial) commercial.push_back(static_cast<int>(i));
    if (p.use == LandUse::Residential && p.households > 0) homes.push_back(static_cast<int>(i));
    if (p.use == LandUse::Park) open.push_back(static_cast<int>(i));
  }
  auto nearest_first = [&](std::vector<int>& v) {
    std::stable_sort(v.begin(), v.end(), [&](int a, int b) {
      return from_centre(layout, layout.plots[static_cast<std::size_t>(a)]) < from_centre(layout, layout.plots[static_cast<std::size_t>(b)]);
    });
  };
  nearest_first(commercial);
  nearest_first(open);
  std::stable_sort(homes.begin(), homes.end(), [&](int a, int b) {  // small shops spread over the town by a keyed draw
    return mix64(seed ^ hash_str("food_small"), static_cast<std::uint64_t>(a)) < mix64(seed ^ hash_str("food_small"), static_cast<std::uint64_t>(b));
  });
  std::size_t next_commercial = 0, next_home = 0, next_open = 0;
  auto take = [&](std::vector<int>& v, std::size_t& at) { return at < v.size() ? v[at++] : -1; };
  std::vector<const Business*> grocers;
  for (const Business& b : town.businesses)
    if (b.type == "grocery") grocers.push_back(&b);
  std::size_t next_grocer = 0;
  for (int k : {0, 1, 3, 2}) {  // supermarkets first, then convenience stores, the market, small shops
    const auto kind = static_cast<OutletKind>(k);
    const int n = count[static_cast<std::size_t>(k)];
    for (int i = 0; i < n; ++i) {
      Outlet o;
      o.kind = kind;
      o.floor_m2 = params.kinds[static_cast<std::size_t>(k)].floor_m2;
      o.stock_days = params.kinds[static_cast<std::size_t>(k)].shelf_days + params.kinds[static_cast<std::size_t>(k)].back_days;
      o.share = share[static_cast<std::size_t>(k)] / n;
      if (kind == OutletKind::Small) o.plot = take(homes, next_home);
      else if (kind == OutletKind::Market) {
        o.plot = take(open, next_open);  // a park near the centre, else a commercial plot
        if (o.plot < 0) o.plot = take(commercial, next_commercial);
      } else {
        o.plot = take(commercial, next_commercial);
      }
      if (o.plot >= 0) o.address = layout.plots[static_cast<std::size_t>(o.plot)].address;
      if ((kind == OutletKind::Supermarket || kind == OutletKind::Convenience) && next_grocer < grocers.size())
        o.name = grocers[next_grocer++]->name;
      else
        o.name = std::string(outlet_kind_name(kind)) + (o.address.empty() ? "" : ", " + o.address);
      f.outlets.push_back(std::move(o));
    }
  }
  return f;
}

std::uint64_t town_food_digest(const TownFood& f) {
  std::uint64_t h = mix64(f.outlets.size(), static_cast<std::uint64_t>(f.centre.town + 1));
  for (const Outlet& o : f.outlets) {
    std::uint64_t bits = 0;
    std::memcpy(&bits, &o.share, sizeof bits);
    h = mix64(h, mix64(static_cast<std::uint64_t>(o.kind) << 32 | static_cast<std::uint32_t>(o.plot + 1), bits));
  }
  return h;
}

}  // namespace zs::gen
