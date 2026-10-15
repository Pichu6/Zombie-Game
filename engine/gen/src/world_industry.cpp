// Spec 93: processing plants. docs/tier0/specs/93-processing-plants.md (research processing-plants, PRC).
#include "zs/gen/world_industry.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

#include "toml.hpp"
#include "zs/detmath.h"

namespace zs::gen {

namespace {

constexpr double kEarthPeople = 8.0e9;
constexpr double kNearKm = 800;        // rule 3: inputs "near" a port or a power town (estimate)
constexpr double kPowerKm = 500;       // a smelter's gas or coal within this (estimate)
constexpr double kHeavyTownPeople = 50000;  // towns that may hold a refinery or steel works (estimate)
constexpr double kPowerTownPeople = 20000;

PlantRule rule_of(const std::string& s) {
  if (s == "materials") return PlantRule::Materials;
  if (s == "port") return PlantRule::Port;
  if (s == "power") return PlantRule::Power;
  return PlantRule::Market;
}

// The deposit kind an input names, or -1 for a land input.
int resource_of(const std::string& in) {
  if (in == "coal") return static_cast<int>(Resource::Coal);
  if (in == "oil") return static_cast<int>(Resource::Oil);
  if (in == "gas") return static_cast<int>(Resource::Gas);
  if (in == "iron") return static_cast<int>(Resource::IronOre);
  if (in == "copper") return static_cast<int>(Resource::CopperOre);
  if (in == "bauxite") return static_cast<int>(Resource::Bauxite);
  return -1;
}

// Largest remainder: `n` plants over the weights, ties to the lower index.
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

std::vector<PlantKind> load_plant_kinds(const std::string& gen_dir) {
  const toml::table t = toml::parse_file(gen_dir + "/processing.toml");
  std::vector<PlantKind> out;
  if (const toml::array* ks = t["kind"].as_array())
    for (const auto& k : *ks) {
      const toml::table& x = *k.as_table();
      PlantKind p;
      p.id = x["id"].value_or(std::string{});
      p.label = x["label"].value_or(p.id);
      p.output = x["output"].value_or(std::string{});
      p.size = x["size"].value_or(std::string{});
      p.rule = rule_of(x["rule"].value_or(std::string("market")));
      if (const toml::array* in = x["input"].as_array())
        for (const auto& v : *in) p.inputs.push_back(v.value_or(std::string{}));
      p.earth_plants = x["earth_plants"].value_or(0.0);
      if (const toml::array* iw = x["income_weight"].as_array())
        for (std::size_t i = 0; i < 4 && i < iw->size(); ++i) p.income_weight[i] = (*iw)[i].value_or(1.0);
      if (p.id.empty()) throw std::runtime_error("processing.toml: a kind without an id");
      out.push_back(std::move(p));
    }
  return out;
}

WorldIndustry place_plants(const WorldGrid& g, const WorldFields& f, const WorldLand& land, const WorldDeposits& deposits,
                           const WorldPeople& people, const WorldLandUse& use, const std::vector<PlantKind>& kinds) {
  WorldIndustry w;
  w.kinds = kinds;
  const std::size_t nt = people.towns.size();
  w.of_town.assign(nt, {});
  if (nt == 0) return w;
  const double scale = people.total_people / kEarthPeople;
  // The worked sites by resource, and each resource's worked total (to weigh sites of one kind against each other).
  std::vector<std::vector<int>> sites_of(resource_count);
  std::vector<double> worked(resource_count, 0.0);
  for (std::size_t s = 0; s < use.sites.size(); ++s) {
    const Deposit& d = deposits.deposits[static_cast<std::size_t>(use.sites[s].deposit)];
    sites_of[static_cast<std::size_t>(d.kind)].push_back(static_cast<int>(s));
    worked[static_cast<std::size_t>(d.kind)] += d.amount;
  }
  auto town_ok = [&](int t) { return t >= 0 && static_cast<std::size_t>(t) < nt; };
  // Land inputs gathered by each town (spec 88's markets): crop, pasture and forest area in use; the catch landed.
  std::vector<double> crop(nt, 0.0), pasture(nt, 0.0), forest(nt, 0.0), fish(nt, 0.0);
  for (CellId c = 0; c < g.size(); ++c) {
    if (f.land(c)) {
      const int m = c < use.market.size() ? use.market[c] : -1;
      if (!town_ok(m)) continue;
      const double a = g.area_km2(c);
      crop[static_cast<std::size_t>(m)] += a * use.crop[c];
      pasture[static_cast<std::size_t>(m)] += a * use.pasture[c];
      forest[static_cast<std::size_t>(m)] += a * use.forest[c];
    } else if (c < use.fish_port.size() && town_ok(use.fish_port[c])) {
      fish[static_cast<std::size_t>(use.fish_port[c])] += use.fish_t[c];
    }
  }
  auto present = [&](const std::string& in) {
    const int r = resource_of(in);
    if (r >= 0) return !sites_of[static_cast<std::size_t>(r)].empty();
    if (in == "crop") return std::accumulate(crop.begin(), crop.end(), 0.0) > 0;
    if (in == "pasture") return std::accumulate(pasture.begin(), pasture.end(), 0.0) > 0;
    if (in == "forest") return std::accumulate(forest.begin(), forest.end(), 0.0) > 0;
    if (in == "fish") return std::accumulate(fish.begin(), fish.end(), 0.0) > 0;
    return true;  // people
  };
  // Whether a worked site of any of `res` lies within `km` of a cell (stops at the first).
  auto within = [&](CellId cell, const std::vector<int>& res, double km) {
    for (int r : res)
      for (int s : sites_of[static_cast<std::size_t>(r)])
        if (g.distance_km(cell, use.sites[static_cast<std::size_t>(s)].cell) <= km) return true;
    return false;
  };
  auto large_river_near = [&](CellId cell) {
    if (land.river[cell] == 2) return true;
    for (CellId nb : g.neighbours(cell))
      if (land.river[nb] == 2) return true;
    return false;
  };
  for (std::size_t k = 0; k < kinds.size(); ++k) {
    const PlantKind& K = kinds[k];
    bool ok = true;
    for (const std::string& in : K.inputs) ok = ok && present(in);
    const int n = static_cast<int>(std::llround(K.earth_plants * scale));
    if (!ok || n <= 0) continue;
    std::vector<int> res;  // the deposit inputs
    for (const std::string& in : K.inputs)
      if (resource_of(in) >= 0) res.push_back(resource_of(in));
    std::vector<double> weight(nt, 0.0);
    for (std::size_t t = 0; t < nt; ++t) {
      const Settlement& town = people.towns[t];
      const int income = town.country >= 0 ? people.countries[static_cast<std::size_t>(town.country)].income : 1;
      const bool port = town.site == TownSite::Port || town.site == TownSite::Mouth;
      switch (K.rule) {
        case PlantRule::Market:
          // People to the 0.85: a large town's plants are larger, not proportionally more (estimate).
          weight[t] = detmath::pow(town.people, 0.85) * K.income_weight[static_cast<std::size_t>(std::clamp(income, 0, 3))];
          break;
        case PlantRule::Materials:
          break;  // below, from the inputs each town gathers
        case PlantRule::Port:
          if (town.people >= kHeavyTownPeople)
            weight[t] = detmath::pow(town.people, 0.7) * (port ? 2.0 : 1.0) * (within(town.cell, res, kNearKm) ? 3.0 : 1.0);
          break;
        case PlantRule::Power: {
          if (town.people < kPowerTownPeople) break;
          double energy = 0;
          if (within(town.cell, {static_cast<int>(Resource::Gas), static_cast<int>(Resource::Coal)}, kPowerKm)) energy += 3;
          if (large_river_near(town.cell)) energy += 2;  // hydro
          if (energy > 0) weight[t] = energy * (port ? 1.5 : 1.0) * detmath::pow(town.people, 0.3);
          break;
        }
      }
    }
    if (K.rule == PlantRule::Materials) {
      for (const std::string& in : K.inputs) {
        const int r = resource_of(in);
        if (r >= 0) {
          for (int s : sites_of[static_cast<std::size_t>(r)]) {
            const WorkSite& site = use.sites[static_cast<std::size_t>(s)];
            if (town_ok(site.market))
              weight[static_cast<std::size_t>(site.market)] +=
                  deposits.deposits[static_cast<std::size_t>(site.deposit)].amount / std::max(1e-12, worked[static_cast<std::size_t>(r)]);
          }
        } else {
          const std::vector<double>& v = in == "crop" ? crop : in == "pasture" ? pasture : in == "forest" ? forest : fish;
          const double total = std::accumulate(v.begin(), v.end(), 0.0);
          for (std::size_t t = 0; t < nt; ++t) weight[t] += total > 0 ? v[t] / total : 0.0;
        }
      }
    }
    const std::vector<int> count = share_out(n, weight);
    for (std::size_t t = 0; t < nt; ++t)
      for (int i = 0; i < count[t]; ++i) {
        Plant p;
        p.kind = static_cast<int>(k);
        p.town = static_cast<int>(t);
        p.cell = people.towns[t].cell;
        p.country = people.towns[t].country;
        // Rule 4: the nearest worked sites of its deposit inputs, up to three.
        if (!res.empty()) {
          std::vector<std::pair<double, int>> near;
          for (int r : res)
            for (int s : sites_of[static_cast<std::size_t>(r)]) near.push_back({g.distance_km(p.cell, use.sites[static_cast<std::size_t>(s)].cell), s});
          const std::size_t top = std::min<std::size_t>(3, near.size());
          std::partial_sort(near.begin(), near.begin() + static_cast<std::ptrdiff_t>(top), near.end());
          for (std::size_t q = 0; q < top; ++q) p.feeds.push_back(near[q].second);
        }
        w.of_town[t].push_back(static_cast<int>(w.plants.size()));
        w.plants.push_back(std::move(p));
      }
  }
  return w;
}

}  // namespace zs::gen
