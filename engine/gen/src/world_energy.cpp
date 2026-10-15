// Spec 94: power plants and the grid. docs/tier0/specs/94-power-plants-and-the-grid.md (research electricity-supply, ELS).
#include "zs/gen/world_energy.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "zs/detmath.h"

namespace zs::gen {

namespace {

constexpr double kNearKm = 500;            // a plant's fuel "near" (estimate)
constexpr double kNuclearTownPeople = 100000;
constexpr double kGridTownPeople = 100000;  // towns on the high-voltage grid (rule 6)
constexpr double kNuclearMinTwh = 25;       // two reactors' worth of demand (1,800 MW at 0.8 makes 12.6 TWh a year)

std::vector<int> share_out(int n, const std::vector<double>& w) {
  std::vector<int> out(w.size(), 0);
  const double total = std::accumulate(w.begin(), w.end(), 0.0);
  if (n <= 0 || total <= 0) return out;
  std::vector<std::pair<double, std::size_t>> frac;
  int given = 0;
  for (std::size_t i = 0; i < w.size(); ++i) {
    const double q = n * w[i] / total;
    out[i] = static_cast<int>(q);
    given += out[i];
    if (w[i] > 0) frac.push_back({q - out[i], i});
  }
  std::stable_sort(frac.begin(), frac.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
  for (std::size_t k = 0; given < n && k < frac.size(); ++k, ++given) ++out[frac[k].second];
  return out;
}

}  // namespace

const char* fuel_name(Fuel f) {
  static const char* names[kFuels] = {"coal", "gas", "oil and other", "hydro", "nuclear", "wind", "solar"};
  return names[static_cast<int>(f)];
}

WorldEnergy make_energy(const WorldGrid& g, const WorldFields& f, const WorldLand& land, const WorldDeposits& deposits,
                        const WorldPeople& people, const WorldLandUse& use, const EnergyParams& p) {
  WorldEnergy w;
  const std::size_t nc = people.countries.size();
  const std::size_t n = g.size();
  w.demand_twh.assign(nc, 0.0);
  w.gen_twh.assign(nc, std::array<double, kFuels>{});
  if (nc == 0) return w;
  const std::vector<int>& country_of = people.formed.country_of;
  // Rule 1: demand.
  for (std::size_t k = 0; k < nc; ++k) {
    const Country& c = people.countries[k];
    w.demand_twh[k] = c.people * p.kwh_per_person[static_cast<std::size_t>(std::clamp(c.income, 0, 3))] * (1 + p.losses) * p.growth / 1e9;
  }
  // What each country has: worked coal, gas and oil; ports; windy and sunny land.
  std::vector<char> has_coal(nc, 0), has_gas(nc, 0), has_oil(nc, 0), has_port(nc, 0);
  std::vector<std::vector<CellId>> coal_sites(nc), gas_sites(nc);
  for (const WorkSite& s : use.sites) {
    if (s.country < 0) continue;
    const auto k = static_cast<std::size_t>(s.country);
    const Resource r = deposits.deposits[static_cast<std::size_t>(s.deposit)].kind;
    if (r == Resource::Coal) has_coal[k] = 1, coal_sites[k].push_back(s.cell);
    if (r == Resource::Gas) has_gas[k] = 1, gas_sites[k].push_back(s.cell);
    if (r == Resource::Oil) has_oil[k] = 1;
  }
  for (const Settlement& t : people.towns)
    if ((t.site == TownSite::Port || t.site == TownSite::Mouth) && t.country >= 0) has_port[static_cast<std::size_t>(t.country)] = 1;
  std::vector<double> windy(nc, 0.0), sunny(nc, 0.0), cells(nc, 0.0);
  for (CellId c = 0; c < n; ++c) {
    const int k = country_of[c];
    if (k < 0) continue;
    const double lat = std::abs(g.sin_lat(c));  // sine of latitude
    const bool coast = f.sea_cells[c] <= 1;
    const Biome b = f.biome[c];
    cells[static_cast<std::size_t>(k)] += 1;
    windy[static_cast<std::size_t>(k)] += (coast || (lat > 0.57 && lat < 0.87)) ? 1 : 0;  // coasts and 35-60 degrees
    sunny[static_cast<std::size_t>(k)] += (b == Biome::Desert || b == Biome::Savanna || b == Biome::Grassland || lat < 0.57) ? 1 : 0;
  }
  // Rule 3: a country can run reactors only with a large town by the sea or a large river and demand for at least two
  // of them (32 of Earth's countries do, ELS-5).
  std::vector<char> can_nuclear(nc, 0);
  for (const Settlement& t : people.towns)
    if (t.country >= 0 && t.people >= kNuclearTownPeople && (f.sea_cells[t.cell] <= 1 || land.river[t.cell] == 2))
      can_nuclear[static_cast<std::size_t>(t.country)] = 1;
  for (std::size_t k = 0; k < nc; ++k)
    if (w.demand_twh[k] < kNuclearMinTwh) can_nuclear[k] = 0;
  // Rule 2: dam sites, by country, best first.
  std::vector<std::vector<std::pair<double, CellId>>> dams(nc);
  for (CellId c = 0; c < n; ++c) {
    if (land.river[c] == 0 || country_of[c] < 0 || land.downstream[c] == kNoCell) continue;
    const CellId d = land.downstream[c];
    const double head = f.elevation_m[c] - std::max(0.0, f.elevation_m[d]);
    const double mw = 9.81 * land.discharge_m3s[c] * std::max(0.0, head) * 0.9 / 1000.0;
    if (mw < p.dam_min_mw) continue;
    dams[static_cast<std::size_t>(country_of[c])].push_back({mw, c});
    w.hydro_potential_mw += mw;
  }
  for (auto& v : dams) std::stable_sort(v.begin(), v.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
  // Rules 2-4 with calibration: one factor each for hydro uptake, nuclear and the five others.
  double hydro_use = 0.5, nuclear_k = 1.0;
  std::array<double, kFuels> factor{1, 1, 1, 1, 1, 1, 1};
  std::vector<int> dams_used(nc, 0);
  for (int round = 0; round <= p.rounds; ++round) {
    std::array<double, kFuels> total{};
    for (std::size_t k = 0; k < nc; ++k) {
      const Country& c = people.countries[k];
      const auto inc = static_cast<std::size_t>(std::clamp(c.income, 0, 3));
      const double demand = w.demand_twh[k];
      auto& gen = w.gen_twh[k];
      gen.fill(0.0);
      // Hydro: its best sites until the uptake share of its potential, never above 90 % of demand.
      double avail = 0;
      for (const auto& [mw, cell] : dams[k]) avail += mw * p.capacity_factor[3] * 8760 / 1e6;
      const double want = std::min(0.9 * demand, std::min(1.0, hydro_use) * avail);
      double got = 0;
      int used = 0;
      for (const auto& [mw, cell] : dams[k]) {
        if (got >= want) break;
        got += mw * p.capacity_factor[3] * 8760 / 1e6;
        ++used;
      }
      gen[3] = std::min(got, 0.95 * demand);
      dams_used[k] = used;
      gen[4] = can_nuclear[k] ? std::min(demand - gen[3], demand * p.nuclear_by_income[inc] * nuclear_k) : 0.0;
      const double rest = std::max(0.0, demand - gen[3] - gen[4]);
      std::array<double, kFuels> wt{};
      wt[0] = factor[0] * (1 + 3.0 * has_coal[k]) * (has_port[k] ? 1.3 : 1.0);
      wt[1] = factor[1] * (1 + 3.0 * has_gas[k]);
      wt[2] = factor[2] * (1 + 1.0 * has_oil[k]);
      const double cn = std::max(1.0, cells[k]);
      wt[5] = factor[5] * (0.5 + windy[k] / cn) * p.renewables_by_income[inc];
      wt[6] = factor[6] * (0.5 + sunny[k] / cn) * p.renewables_by_income[inc];
      const double sw = wt[0] + wt[1] + wt[2] + wt[5] + wt[6];
      for (int q : {0, 1, 2, 5, 6}) gen[static_cast<std::size_t>(q)] = sw > 0 ? rest * wt[static_cast<std::size_t>(q)] / sw : 0.0;
      for (int q = 0; q < kFuels; ++q) total[static_cast<std::size_t>(q)] += gen[static_cast<std::size_t>(q)];
    }
    const double all = std::accumulate(total.begin(), total.end(), 0.0);
    w.world_twh = total;
    if (round == p.rounds || all <= 0) break;
    // Nudge each factor toward Ember's share (rule 4).
    auto ratio = [&](int q) { return total[static_cast<std::size_t>(q)] > 0 ? p.world_share[static_cast<std::size_t>(q)] * all / total[static_cast<std::size_t>(q)] : 2.0; };
    hydro_use = std::clamp(hydro_use * ratio(3), 0.01, 1.0);
    nuclear_k = std::clamp(nuclear_k * ratio(4), 0.05, 4.0);
    for (int q : {0, 1, 2, 5, 6}) factor[static_cast<std::size_t>(q)] *= ratio(q);
  }
  // Rule 5: plants.
  std::vector<std::vector<int>> towns_of(nc);
  for (std::size_t t = 0; t < people.towns.size(); ++t)
    if (people.towns[t].country >= 0) towns_of[static_cast<std::size_t>(people.towns[t].country)].push_back(static_cast<int>(t));
  auto near = [&](CellId cell, const std::vector<CellId>& sites) {
    for (CellId s : sites)
      if (g.distance_km(cell, s) <= kNearKm) return true;
    return false;
  };
  for (std::size_t k = 0; k < nc; ++k) {
    // Dams at their sites.
    for (int i = 0; i < dams_used[k]; ++i) {
      const auto& [mw, cell] = dams[k][static_cast<std::size_t>(i)];
      w.plants.push_back({Fuel::Hydro, cell, -1, static_cast<int>(k), mw});
    }
    const std::vector<int>& towns = towns_of[k];
    if (towns.empty()) continue;
    for (int q : {0, 1, 2, 4, 5, 6}) {
      const double twh = w.gen_twh[k][static_cast<std::size_t>(q)];
      if (twh <= 0) continue;
      const double mw_total = twh * 1e6 / (8760 * p.capacity_factor[static_cast<std::size_t>(q)]);
      const int count = std::max(1, static_cast<int>(std::lround(mw_total / p.unit_mw[static_cast<std::size_t>(q)])));
      std::vector<double> wt(towns.size(), 0.0);
      for (std::size_t i = 0; i < towns.size(); ++i) {
        const Settlement& t = people.towns[static_cast<std::size_t>(towns[i])];
        const bool port = t.site == TownSite::Port || t.site == TownSite::Mouth;
        const bool water = f.sea_cells[t.cell] <= 1 || land.river[t.cell] == 2;
        switch (static_cast<Fuel>(q)) {
          case Fuel::Coal: wt[i] = detmath::pow(t.people, 0.5) * (near(t.cell, coal_sites[k]) ? 3.0 : 1.0) * (port ? 2.0 : 1.0); break;
          case Fuel::Gas: wt[i] = detmath::pow(t.people, 0.7) * (near(t.cell, gas_sites[k]) ? 2.0 : 1.0); break;
          case Fuel::Oil: wt[i] = detmath::pow(t.people, 0.5) * (port ? 1.0 : 0.1); break;
          case Fuel::Nuclear: wt[i] = t.people >= kNuclearTownPeople && water ? detmath::pow(t.people, 0.5) : 0.0; break;
          case Fuel::Wind: wt[i] = detmath::pow(t.people, 0.3) * (f.sea_cells[t.cell] <= 1 ? 2.0 : 1.0); break;
          case Fuel::Solar: {
            const Biome b = f.biome[t.cell];
            wt[i] = detmath::pow(t.people, 0.3) * (b == Biome::Desert || b == Biome::Savanna || b == Biome::Grassland ? 2.0 : 1.0);
            break;
          }
          default: break;
        }
      }
      if (std::accumulate(wt.begin(), wt.end(), 0.0) <= 0) wt[0] = 1;  // no town fits: the largest (first by rank? the first)
      const std::vector<int> per = share_out(count, wt);
      for (std::size_t i = 0; i < towns.size(); ++i)
        for (int j = 0; j < per[i]; ++j)
          w.plants.push_back({static_cast<Fuel>(q), people.towns[static_cast<std::size_t>(towns[i])].cell, towns[i], static_cast<int>(k),
                              mw_total / count});
    }
  }
  // Rule 6: the grid: per country the shortest tree over its plants and large towns; neighbours joined where nearest.
  std::vector<std::vector<CellId>> nodes(nc);
  // Wind and solar farms and small oil plants feed their town's own network; the large plants are the grid's nodes.
  for (const PowerPlant& pl : w.plants)
    if (pl.country >= 0 && pl.fuel != Fuel::Wind && pl.fuel != Fuel::Solar && pl.fuel != Fuel::Oil)
      nodes[static_cast<std::size_t>(pl.country)].push_back(pl.cell);
  for (const Settlement& t : people.towns)
    if (t.country >= 0 && t.people >= kGridTownPeople) nodes[static_cast<std::size_t>(t.country)].push_back(t.cell);
  for (auto& v : nodes) std::sort(v.begin(), v.end()), v.erase(std::unique(v.begin(), v.end()), v.end());
  for (std::size_t k = 0; k < nc; ++k) {
    const std::vector<CellId>& v = nodes[k];
    if (v.size() < 2) continue;
    std::vector<double> best(v.size(), 1e300);
    std::vector<int> from(v.size(), -1);
    std::vector<char> in(v.size(), 0);
    best[0] = 0;
    for (std::size_t step = 0; step < v.size(); ++step) {  // Prim
      std::size_t u = v.size();
      for (std::size_t i = 0; i < v.size(); ++i)
        if (!in[i] && (u == v.size() || best[i] < best[u])) u = i;
      in[u] = 1;
      if (from[u] >= 0) w.lines.push_back({v[static_cast<std::size_t>(from[u])], v[u], static_cast<int>(k), best[u]});
      for (std::size_t i = 0; i < v.size(); ++i)
        if (!in[i])
          if (const double d = g.distance_km(v[u], v[i]); d < best[i]) best[i] = d, from[i] = static_cast<int>(u);
    }
  }
  {
    std::vector<std::pair<int, int>> pairs;
    for (CellId c = 0; c < n; ++c) {
      const int a = country_of[c];
      if (a < 0) continue;
      for (CellId nb : g.neighbours(c))
        if (const int b = country_of[nb]; b > a) pairs.push_back({a, b});
    }
    std::sort(pairs.begin(), pairs.end());
    pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());
    for (const auto& [a, b] : pairs) {
      const auto& va = nodes[static_cast<std::size_t>(a)];
      const auto& vb = nodes[static_cast<std::size_t>(b)];
      double bd = 1e300;
      CellId x = 0, y = 0;
      for (CellId i : va)
        for (CellId j : vb)
          if (const double d = g.distance_km(i, j); d < bd) bd = d, x = i, y = j;
      if (bd < 1e300) w.lines.push_back({x, y, -1, bd});
    }
  }
  return w;
}

}  // namespace zs::gen
