// Spec 96: the town's power from its country's grid, and the outage from its causes.
#include "zs/gen/town_power.h"

#include <algorithm>
#include <cstdio>
#include <map>
#include <queue>
#include <set>

#include "fields.h"
#include "zs/detmath.h"
#include "zs/gen/keyed.h"

namespace zs::gen {

const char* outage_cause_name(OutageCause c) {
  switch (c) {
    case OutageCause::Generation: return "the grid's plants stopped";
    case OutageCause::Line: return "a fault on the lines to its plants";
    case OutageCause::Feeder: return "a fault on its own feeder";
  }
  return "?";
}

namespace {
bool large(Fuel f) { return f == Fuel::Coal || f == Fuel::Gas || f == Fuel::Hydro || f == Fuel::Nuclear; }
bool fossil(Fuel f) { return f == Fuel::Coal || f == Fuel::Gas || f == Fuel::Oil; }
}  // namespace

TownFeed town_feed(const WorldGrid& grid, const WorldPeople& people, const WorldEnergy& energy, int town) {
  TownFeed out;
  out.town = town;
  if (town < 0 || static_cast<std::size_t>(town) >= people.towns.size()) return out;
  const Settlement& t = people.towns[static_cast<std::size_t>(town)];
  // Rule 1: the grid's nodes and their lines.
  std::map<CellId, std::vector<std::pair<CellId, double>>> adj;
  std::map<CellId, int> node_country;
  for (const GridLine& l : energy.lines) {
    adj[l.a].push_back({l.b, l.km});
    adj[l.b].push_back({l.a, l.km});
    if (l.country >= 0) node_country[l.a] = node_country[l.b] = l.country;
  }
  // The town's node: its own cell, or its country's nearest node by a feeder.
  out.node = t.cell;
  if (adj.count(t.cell)) {
    out.on_grid = true;
  } else {
    double best = 1e300;
    for (const auto& [cell, c] : node_country)
      if (c == t.country)
        if (const double d = grid.distance_km(t.cell, cell); d < best) best = d, out.node = cell;
    if (best < 1e300) out.on_grid = true, out.feeder_km = best;
  }
  // The island: every node the lines reach, in hours of nothing but km (Dijkstra for the line length).
  std::map<CellId, double> dist;
  if (out.on_grid) {
    using Item = std::pair<double, CellId>;
    std::priority_queue<Item, std::vector<Item>, std::greater<>> q;
    dist[out.node] = 0;
    q.push({0.0, out.node});
    while (!q.empty()) {
      const auto [d, c] = q.top();
      q.pop();
      if (d > dist[c]) continue;
      for (const auto& [m, km] : adj[c])
        if (auto it = dist.find(m); it == dist.end() || d + km < it->second) dist[m] = d + km, q.push({d + km, m});
    }
  } else {
    dist[t.cell] = 0;
  }
  // Rule 2: the island's plants (on its nodes, and the town's own), its countries and its load.
  std::set<int> countries{t.country};
  out.line_km = -1;
  for (std::size_t i = 0; i < energy.plants.size(); ++i) {
    const PowerPlant& pl = energy.plants[i];
    const auto it = dist.find(pl.cell);
    const bool own = pl.town == town;
    if (it == dist.end() && !own) continue;
    out.plants.push_back(static_cast<int>(i));
    out.mw[static_cast<std::size_t>(pl.fuel)] += pl.mw;
    if (pl.country >= 0) countries.insert(pl.country);
    if (large(pl.fuel) && it != dist.end() && (out.line_km < 0 || it->second < out.line_km)) out.line_km = it->second;
  }
  if (out.line_km < 0) out.line_km = 0;
  for (const auto& [cell, d] : dist)
    if (auto c = node_country.find(cell); c != node_country.end()) countries.insert(c->second);
  for (int c : countries)
    if (c >= 0 && static_cast<std::size_t>(c) < energy.demand_twh.size()) {
      out.countries.push_back(c);
      out.load_mw += energy.demand_twh[static_cast<std::size_t>(c)] * 1e6 / 8760;
    }
  if (!out.on_grid) {
    // A town with no grid: its load is its share of its country's demand by people.
    double country_people = 0;
    for (const Settlement& s : people.towns)
      if (s.country == t.country) country_people += s.people;
    if (t.country >= 0 && country_people > 0 && static_cast<std::size_t>(t.country) < energy.demand_twh.size())
      out.load_mw = energy.demand_twh[static_cast<std::size_t>(t.country)] * 1e6 / 8760 * t.people / country_people;
  }
  return out;
}

OutageTimeline outage_timeline(const TownFeed& feed, const WorldEnergy& energy, const EnergyParams& ep, std::uint64_t seed,
                               const OutageParams& p) {
  OutageTimeline out;
  const Keyed rnd(seed);
  auto between = [&](std::uint64_t id, std::uint32_t k, double lo, double hi) { return lo + (hi - lo) * rnd.u01(id, field::Outage, k); };
  // Rule 3: the staff leave (one day for the world, a stand-in until spec 85), then each plant stops after its own time.
  out.staff_leave_h = 24 * between(0, 0, p.staff_day_min, p.staff_day_max);
  struct Stop {
    double h, mw;
  };
  std::vector<Stop> stops;
  double running = 0;
  for (int i : feed.plants) {
    const PowerPlant& pl = energy.plants[static_cast<std::size_t>(i)];
    const auto id = static_cast<std::uint64_t>(i) + 1;
    double h = -1, mw = pl.mw;
    if (fossil(pl.fuel)) h = out.staff_leave_h + between(id, 1, p.fossil_h_min, p.fossil_h_max), out.fossil_last_h = std::max(out.fossil_last_h, h);
    if (pl.fuel == Fuel::Nuclear) h = out.staff_leave_h + 24 * between(id, 2, p.nuclear_d_min, p.nuclear_d_max), out.nuclear_last_h = std::max(out.nuclear_last_h, h);
    if (pl.fuel == Fuel::Hydro) {
      h = out.staff_leave_h + 24 * between(id, 3, p.hydro_d_min, p.hydro_d_max), out.hydro_last_h = std::max(out.hydro_last_h, h);
      mw *= ep.capacity_factor[static_cast<std::size_t>(Fuel::Hydro)];  // what its water gives, not its turbines
    }
    if (h < 0) continue;  // wind and solar follow the grid; they cannot hold it up
    running += mw;
    stops.push_back({h, mw});
  }
  std::sort(stops.begin(), stops.end(), [](const Stop& a, const Stop& b) { return a.h < b.h; });
  // Rule 3: the island is dark when what still runs falls below its load once work stops.
  const double need = p.load_after * feed.load_mw;
  out.island_dark_h = out.staff_leave_h;
  if (running >= need) {
    out.island_dark_h = stops.empty() ? out.staff_leave_h : stops.back().h;
    for (const Stop& s : stops) {
      running -= s.mw;
      if (running < need) {
        out.island_dark_h = s.h;
        break;
      }
    }
  }
  // Rule 4: once the crews are gone, the first fault on the lines or the feeder stays.
  auto first_fault = [&](double per_year, std::uint32_t k) {
    if (per_year <= 0) return 1e9;
    const double u = std::max(1e-12, rnd.u01(static_cast<std::uint64_t>(feed.town) + 1, field::Outage, k));
    return out.staff_leave_h - zs::detmath::log(u) / per_year * 8766.0;
  };
  out.line_fault_h = first_fault(p.line_faults_per_100km_year * feed.line_km / 100, 10);
  out.feeder_fault_h = first_fault(p.feeder_faults_per_year + p.line_faults_per_100km_year * feed.feeder_km / 100, 11);
  out.dark_h = out.island_dark_h;
  out.cause = OutageCause::Generation;
  if (out.line_fault_h < out.dark_h) out.dark_h = out.line_fault_h, out.cause = OutageCause::Line;
  if (out.feeder_fault_h < out.dark_h) out.dark_h = out.feeder_fault_h, out.cause = OutageCause::Feeder;
  return out;
}

std::string describe_feed(const TownFeed& feed, const OutageTimeline& t) {
  double all = 0;
  for (double x : feed.mw) all += x;
  char buf[256];
  std::string s;
  std::snprintf(buf, sizeof buf, "%s %.0f GW for a load of %.1f GW (", feed.on_grid ? "a grid of" : "off the grid, its own plants of",
                all / 1000, feed.load_mw / 1000);
  s = buf;
  bool first = true;
  for (int q = 0; q < kFuels; ++q)
    if (feed.mw[static_cast<std::size_t>(q)] > 0 && all > 0) {
      std::snprintf(buf, sizeof buf, "%s%s %.0f %%", first ? "" : ", ", fuel_name(static_cast<Fuel>(q)), 100 * feed.mw[static_cast<std::size_t>(q)] / all);
      s += buf, first = false;
    }
  std::snprintf(buf, sizeof buf, "); %.0f km of feeder and %.0f km of lines to its nearest large plant. Staff leave on day %.1f; "
                "the island holds to day %.1f, ", feed.feeder_km, feed.line_km, t.staff_leave_h / 24, t.island_dark_h / 24);
  s += buf;
  if (feed.line_km > 0) std::snprintf(buf, sizeof buf, "its lines to %.1f, ", t.line_fault_h / 24), s += buf;
  std::snprintf(buf, sizeof buf, "its feeder to %.1f: dark on day %.1f (%s)", t.feeder_fault_h / 24, t.dark_h / 24, outage_cause_name(t.cause));
  return s + buf;
}

}  // namespace zs::gen
