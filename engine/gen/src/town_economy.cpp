// Spec 98: the town scaled.
#include "zs/gen/town_economy.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

#include "toml.hpp"
#include "zs/gen/keyed.h"
#include "zs/gen/names.h"

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
// The land use a workplace of a kind stands on.
LandUse use_of(const std::string& type) {
  if (type == "factory" || type == "warehouse" || type == "plant") return LandUse::Industrial;
  if (type == "hospital" || type == "care_home" || type == "college" || type == "fire_station" || type == "church" || type == "police" ||
      (type.size() > 7 && type.compare(type.size() - 7, 7, "_school") == 0))
    return LandUse::Civic;
  return LandUse::Commercial;
}
}  // namespace

TownEconomyParams load_town_economy(const std::string& gen_dir) {
  const std::string path = gen_dir + "/town_economy.toml";
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  const toml::node_view<const toml::node> root{t};
  TownEconomyParams p;
  auto table_of = [&](const char* name, std::map<std::string, double>& into) {
    if (const auto* s = root[name].as_table())
      for (const auto& [k, v] : *s) {
        const auto x = v.value<double>();
        if (!x || *x < 0) throw std::runtime_error(path + ": [" + name + "] " + std::string(k.str()) + " needs a non-negative number");
        into[std::string(k.str())] = *x;
      }
  };
  table_of("staff", p.staff);
  table_of("plant_staff", p.plant_staff);
  for (int q = 0; q < kFuels; ++q)
    p.power_staff_per_mw[static_cast<std::size_t>(q)] = need(root["power_staff_per_mw"][fuel_name(static_cast<Fuel>(q)) == std::string("oil and other") ? "oil" : fuel_name(static_cast<Fuel>(q))], "power_staff_per_mw", path);
  p.min_fill = need(root["min_fill"], "min_fill", path);
  p.resource_staff = need(root["resource_staff"], "resource_staff", path);
  p.farm_factor = need(root["farm_factor"], "farm_factor", path);
  p.farm_people = need(root["farm_people"], "farm_people", path);
  p.farm_exp = need(root["farm_exp"], "farm_exp", path);
  p.farm_min = need(root["farm_min"], "farm_min", path);
  p.farm_max = need(root["farm_max"], "farm_max", path);
  return p;
}

std::vector<TownPlant> town_plants(const WorldIndustry& industry, const WorldEnergy& energy, const WorldPeople& people, int town,
                                   const TownEconomyParams& params) {
  std::vector<TownPlant> out;
  if (town < 0 || static_cast<std::size_t>(town) >= people.towns.size()) return out;
  if (static_cast<std::size_t>(town) < industry.of_town.size())
    for (int i : industry.of_town[static_cast<std::size_t>(town)]) {
      const PlantKind& k = industry.kinds[static_cast<std::size_t>(industry.plants[static_cast<std::size_t>(i)].kind)];
      const auto s = params.plant_staff.find(k.id);
      out.push_back({k.label, s == params.plant_staff.end() ? 0.0 : s->second});
    }
  for (const PowerPlant& pl : energy.plants)
    if (pl.town == town)
      out.push_back({std::string(fuel_name(pl.fuel)) + " power station",
                     std::round(pl.mw * params.power_staff_per_mw[static_cast<std::size_t>(pl.fuel)])});
  return out;
}

TownEconomy plan_town_economy(const Catalog& cat, const zs::NationDemographics& n, const TownEconomyParams& params, double people,
                              const std::vector<TownPlant>& plants, bool resource_town, const TownLayout& layout,
                              const std::string& town_name, const std::string& culture, std::uint64_t seed,
                              const std::map<std::string, std::vector<GivenWorkplace>>& given, const std::vector<int>& taken,
                              const SchoolAges& school) {
  // Jobs whose workplace is "school" work at any "_school" type (data/gen/jobs.toml): elementary schools hold primary,
  // high schools secondary, each by its share of the country's school years.
  auto school_share = [&](const std::string& type) {
    return type == "elementary_school" ? primary_share(school) : type == "high_school" ? 1.0 - primary_share(school) : 0.0;
  };
  TownEconomy e;
  e.people = people;
  // Rule 1: the labour force by age band (five-year bands from 15) and the country's participation; then the employed.
  auto part = [&](int band) {
    const int g = band <= 4 ? 0 : band <= 10 ? 1 : band <= 12 ? 2 : 3;
    return (n.participation[static_cast<std::size_t>(g)][0] + n.participation[static_cast<std::size_t>(g)][1]) / 2;
  };
  for (int b = 3; b < static_cast<int>(n.age_pyramid.size()); ++b) e.labour_force += people * n.age_pyramid[static_cast<std::size_t>(b)] * part(b);
  e.employed = e.labour_force * (1 - n.unemployment_rate);
  e.unemployed = e.labour_force - e.employed;
  // Rule 2: jobs by the country's occupation groups, each group's over its jobs by their shares.
  std::array<double, kOccupationGroups> group_share_sum{};
  for (const Job& j : cat.jobs) group_share_sum[static_cast<std::size_t>(j.group)] += j.share;
  for (const Job& j : cat.jobs) {
    const auto g = static_cast<std::size_t>(j.group);
    e.jobs[j.id] = group_share_sum[g] > 0 ? e.employed * n.occupations[g] * j.share / group_share_sum[g] : 0.0;
  }
  auto total_of = [&](auto pred) {
    double t = 0;
    for (const Job& j : cat.jobs)
      if (pred(j)) t += e.jobs[j.id];
    return t;
  };
  // Move `amount` of jobs into the jobs `to` (by their shares) from the jobs `from` (by their counts).
  auto shift = [&](double amount, auto to, auto from) {
    const double pool = total_of(from);
    amount = std::min(amount, pool);
    if (amount <= 0) return;
    double to_shares = 0;
    for (const Job& j : cat.jobs)
      if (to(j)) to_shares += j.share;
    for (const Job& j : cat.jobs)
      if (from(j)) e.jobs[j.id] -= amount * e.jobs[j.id] / pool;
    for (const Job& j : cat.jobs)
      if (to(j)) e.jobs[j.id] += to_shares > 0 ? amount * j.share / to_shares : 0.0;
  };
  // Rule 3: farm work by the country's agriculture share, less in a larger town; taken from the elementary group first.
  const double f = std::clamp(params.farm_factor * std::pow(params.farm_people / std::max(1.0, people), params.farm_exp), params.farm_min, params.farm_max);
  e.farm = e.employed * n.sectors[0] * f;
  auto is_farm = [](const Job& j) { return j.id == "farmworker"; };
  const double farm_now = total_of(is_farm);
  if (e.farm > farm_now) {
    const double from7 = std::min(e.farm - farm_now, total_of([&](const Job& j) { return j.group == 7 && !is_farm(j); }));
    shift(from7, is_farm, [&](const Job& j) { return j.group == 7 && !is_farm(j); });
    shift(e.farm - farm_now - from7, is_farm, [&](const Job& j) { return !is_farm(j); });
  } else {
    shift(farm_now - e.farm, [&](const Job& j) { return j.group == 7 && !is_farm(j); }, is_farm);
  }
  // Rule 4: the region's plants (and a resource town's mine or field) hire from the town; factory jobs grow to their staff.
  std::vector<TownPlant> employers = plants;
  if (resource_town) employers.push_back({"Mine or field", params.resource_staff});
  for (const TownPlant& p : employers) e.plants += p.staff;
  auto is_factory = [](const Job& j) { return j.workplace == "factory"; };
  const double factory = total_of(is_factory);
  if (e.plants > factory) shift(e.plants - factory, is_factory, [&](const Job& j) { return !is_factory(j) && !is_farm(j); });
  // Rule 5: by sector: farm work; industry: factories and plants, the building trades; the rest services.
  for (const Job& j : cat.jobs) {
    const double c = e.jobs[j.id];
    const bool trade = j.workplace.empty() && j.group == 5;  // carpenters, electricians, plumbers
    e.by_sector[is_farm(j) ? 0 : (is_factory(j) || trade || j.id == "laborer") ? 1 : 2] += c;
  }
  // Rule 6: workplaces. Jobs by kind of workplace ("school" over the "_school" types).
  std::map<std::string, double> at;
  for (const Job& j : cat.jobs) {
    if (j.workplace.empty()) continue;
    if (j.workplace == "school") {
      for (const BusinessType& b : cat.businesses)
        if (school_share(b.id) > 0) at[b.id] += e.jobs[j.id] * school_share(b.id);
    } else {
      at[j.workplace] += e.jobs[j.id];
    }
  }
  // Plots by use, the taken ones left out: commercial and civic from the centre outward, industrial from the edge in.
  std::map<LandUse, std::vector<int>> free;
  for (std::size_t i = 0; i < layout.plots.size(); ++i) {
    const Plot& p = layout.plots[i];
    if (p.lot >= 0 || p.use == LandUse::Residential || p.use == LandUse::Park) continue;
    if (std::find(taken.begin(), taken.end(), static_cast<int>(i)) != taken.end()) continue;
    free[p.use].push_back(static_cast<int>(i));
  }
  for (auto& [use, v] : free)
    std::stable_sort(v.begin(), v.end(), [&, u = use](int a, int b) {
      const double da = from_centre(layout, layout.plots[static_cast<std::size_t>(a)]), db = from_centre(layout, layout.plots[static_cast<std::size_t>(b)]);
      return u == LandUse::Industrial ? da > db : da < db;
    });
  std::map<LandUse, std::size_t> next;
  auto take_plot = [&](LandUse use) {
    auto& v = free[use];
    std::size_t& k = next[use];
    return k < v.size() ? v[k++] : -1;
  };
  const Keyed rnd(seed ^ hash_str("economy"));
  const Culture& c = cat.culture(culture);
  std::uint64_t id = 0;
  auto place = [&](TownWorkplace w) {
    if (w.plot < 0) w.plot = take_plot(use_of(w.type));
    if (w.plot < 0 && use_of(w.type) == LandUse::Civic) w.plot = take_plot(LandUse::Commercial);  // civic plots are the stations' lots
    if (w.plot >= 0) w.address = layout.plots[static_cast<std::size_t>(w.plot)].address;
    e.workplaces.push_back(std::move(w));
  };
  // The region's employers first (their factory jobs), then the other kinds by the catalog's order.
  double factory_left = at["factory"];
  for (const TownPlant& p : employers) {
    place({"plant", p.label, "", -1, p.staff});
    factory_left -= p.staff;
  }
  at["factory"] = std::max(0.0, factory_left);
  // Civic workplaces first (schools, the hospital, a church): the town's few civic plots go to the stations, so they take
  // commercial plots before the shops and offices fill them.
  std::vector<const BusinessType*> order;
  for (const BusinessType& b : cat.businesses)
    if (use_of(b.id) == LandUse::Civic) order.push_back(&b);
  for (const BusinessType& b : cat.businesses)
    if (use_of(b.id) != LandUse::Civic) order.push_back(&b);
  for (const BusinessType* bp : order) {
    const BusinessType& b = *bp;
    const double jobs = at[b.id];
    if (jobs <= 0) continue;
    if (const auto g = given.find(b.id); g != given.end() && !g->second.empty()) {
      double wsum = 0;
      for (const GivenWorkplace& x : g->second) wsum += x.weight;
      for (const GivenWorkplace& x : g->second) {
        TownWorkplace w{b.id, x.name, x.address, x.plot, wsum > 0 ? jobs * x.weight / wsum : 0.0};
        e.workplaces.push_back(std::move(w));
      }
      continue;
    }
    const auto s = params.staff.find(b.id);
    const double staff = s == params.staff.end() ? 10.0 : s->second;
    int count = static_cast<int>(std::lround(jobs / staff));
    if (count == 0 && jobs >= params.min_fill * staff) count = 1;
    if (count == 0) {  // too small a town for one: those jobs are held in a larger town nearby
      e.commuting += jobs;
      continue;
    }
    for (int i = 0; i < count; ++i)
      place({b.id, business_name(cat, b, c, c, town_name, rnd, mix64(hash_str(b.id), ++id)), "", -1, jobs / count});
  }
  return e;
}

std::string describe_economy(const TownEconomy& e) {
  std::map<std::string, int> count;
  for (const TownWorkplace& w : e.workplaces) ++count[w.type];
  const double jobs = e.by_sector[0] + e.by_sector[1] + e.by_sector[2];
  char buf[512];
  std::snprintf(buf, sizeof buf, "%.0f in work of %.0f in the labour force (%.0f without); farm %.0f %%, industry %.0f %%, services %.0f %%; "
                "%.0f with the region's plants, %.0f working in a larger town nearby; %zu workplaces:",
                e.employed, e.labour_force, e.unemployed, jobs > 0 ? 100 * e.by_sector[0] / jobs : 0.0, jobs > 0 ? 100 * e.by_sector[1] / jobs : 0.0,
                jobs > 0 ? 100 * e.by_sector[2] / jobs : 0.0, e.plants, e.commuting, e.workplaces.size());
  std::string s = buf;
  for (const auto& [type, k] : count) s += " " + type + " " + std::to_string(k) + ",";
  if (s.back() == ',') s.pop_back();
  return s;
}

}  // namespace zs::gen
