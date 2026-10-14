// Spec 88: extraction zones. docs/tier0/specs/88-extraction-zones.md
#include "zs/gen/world_land_use.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>
#include <string>
#include <tuple>

#include "zs/detmath.h"

namespace zs::gen {

namespace {

constexpr double kFar = 1e12;

// Multi-source Dijkstra over the grid from `sources` (cell, town), stepping only where `step` allows; ties by town index.
void nearest_town(const WorldGrid& g, const std::vector<std::pair<CellId, int>>& sources, const std::function<bool(CellId)>& step,
                  std::vector<int>& town, std::vector<double>& km) {
  const std::size_t n = g.size();
  town.assign(n, -1);
  km.assign(n, kFar);
  using Item = std::tuple<double, int, CellId>;
  std::priority_queue<Item, std::vector<Item>, std::greater<>> q;
  for (const auto& [c, t] : sources) {
    if (km[c] == 0 && town[c] <= t) continue;
    km[c] = 0;
    town[c] = t;
    q.emplace(0.0, t, c);
  }
  while (!q.empty()) {
    const auto [d, t, c] = q.top();
    q.pop();
    if (d > km[c] || t != town[c]) continue;
    for (const CellId nb : g.neighbours(c)) {
      if (!step(nb)) continue;
      const double nd = d + g.distance_km(c, nb);
      if (nd < km[nb] || (nd == km[nb] && t < town[nb])) {
        km[nb] = nd;
        town[nb] = t;
        q.emplace(nd, t, nb);
      }
    }
  }
}

}  // namespace

const char* site_kind_name(SiteKind k) { return k == SiteKind::Mine ? "mining district" : "oil or gas field"; }

WorldLandUse generate_land_use(const WorldGrid& g, const WorldFields& f, const WorldLand& land, const WorldDeposits& deposits,
                          const WorldPeople& people, const LandUseParams& p) {
  const std::size_t n = g.size();
  WorldLandUse u;
  u.crop.assign(n, 0);
  u.pasture.assign(n, 0);
  u.forest.assign(n, 0);
  u.fish_t.assign(n, 0);
  u.fish_port.assign(n, -1);

  // Rule 1: each land cell's market, over land.
  std::vector<std::pair<CellId, int>> towns, ports;
  for (std::size_t t = 0; t < people.towns.size(); ++t) {
    const Settlement& s = people.towns[t];
    towns.push_back({s.cell, static_cast<int>(t)});
    if (s.site == TownSite::Port || s.site == TownSite::Mouth || f.sea_cells[s.cell] == 1) ports.push_back({s.cell, static_cast<int>(t)});
  }
  nearest_town(g, towns, [&](CellId c) { return f.land(c); }, u.market, u.market_km);

  // Rule 2: the rings.
  for (CellId c = 0; c < n; ++c) {
    if (!f.land(c)) continue;
    const double a = g.area_km2(c);
    u.land_km2 += a;
    u.forest_km2 += land.forest_share[c] * a;
    if (u.market[c] < 0) continue;
    const double d = u.market_km[c];
    u.crop[c] = land.arable_share[c] * detmath::exp(-d / p.crop_km);
    u.pasture[c] = land.pasture_share[c] * detmath::exp(-d / p.pasture_km);
    u.forest[c] = land.forest_share[c] * detmath::exp(-d / p.forest_km);
    u.crop_km2 += u.crop[c] * a;
    u.pasture_km2 += u.pasture[c] * a;
    u.forest_worked_km2 += u.forest[c] * a;
  }

  // Rule 4: fishing grounds from the ports, over the sea (and the port's own coast).
  {
    std::vector<int> port;
    std::vector<double> km;
    nearest_town(g, ports, [&](CellId c) { return !f.land(c); }, port, km);
    double reach = 0;
    for (CellId c = 0; c < n; ++c) {
      if (f.land(c) || land.fish_catch_t[c] <= 0) continue;
      u.fish_available_t += land.fish_catch_t[c];
      if (port[c] >= 0 && km[c] <= p.fish_km) reach += land.fish_catch_t[c];
    }
    const double effort = reach > 0 ? std::min(1.0, p.fish_target_t / reach) : 0.0;
    for (CellId c = 0; c < n; ++c) {
      if (f.land(c) || land.fish_catch_t[c] <= 0 || port[c] < 0 || km[c] > p.fish_km) continue;
      u.fish_t[c] = land.fish_catch_t[c] * effort;
      u.fish_port[c] = port[c];
      u.fish_landed_t += u.fish_t[c];
    }
    // Rule 3: offshore oil and gas on the shelf, worked from the nearest port within reach.
    double median[resource_count] = {};
    for (int r = 0; r < resource_count; ++r) {
      std::vector<double> a;
      for (const Deposit& d : deposits.deposits)
        if (static_cast<int>(d.kind) == r) a.push_back(d.amount);
      if (a.empty()) continue;
      std::nth_element(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(a.size() / 2), a.end());
      median[r] = a[a.size() / 2];
    }
    for (std::size_t i = 0; i < deposits.deposits.size(); ++i) {
      const Deposit& d = deposits.deposits[i];
      const int r = static_cast<int>(d.kind);
      if (median[r] <= 0 || d.amount < p.work_rel * median[r]) continue;
      const bool fluid = d.kind == Resource::Oil || d.kind == Resource::Gas;
      WorkSite s;
      s.kind = fluid ? SiteKind::Field : SiteKind::Mine;
      s.cell = d.cell;
      s.deposit = static_cast<int>(i);
      if (f.land(d.cell)) {
        if (u.market[d.cell] < 0 || people.formed.country_of.empty() || people.formed.country_of[d.cell] < 0) continue;
        s.country = people.formed.country_of[d.cell];
        // A worked site sells to the nearest town of its own country (rule 3).
        double best = 1e300;
        for (std::size_t t = 0; t < people.towns.size(); ++t) {
          if (people.towns[t].country != s.country) continue;
          const double km = g.distance_km(d.cell, people.towns[t].cell);
          if (km < best) best = km, s.market = static_cast<int>(t);
        }
        if (s.market < 0) continue;
      } else {
        if (!fluid || f.elevation_m[d.cell] < p.shelf_m || port[d.cell] < 0 || km[d.cell] > p.fish_km) continue;
        s.market = port[d.cell];
      }
      u.sites.push_back(s);
    }
  }
  return u;
}

}  // namespace zs::gen
