// People, sectors, countries and towns on the world grid. Spec: docs/tier0/specs/48-people-sectors-countries.md
#include "zs/gen/world_people.h"

#include <algorithm>
#include <set>
#include <array>
#include <cmath>
#include <filesystem>
#include <functional>
#include <numeric>
#include <queue>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "fields.h"
#include "toml.hpp"
#include "zs/detmath.h"
#include "zs/gen/keyed.h"

namespace zs::gen {

namespace {

constexpr double kAlpha = 1.5;            // rule 2: concentration (estimate)

constexpr double kSeaCost = 4.0;          // rule 3: a sea crossing costs this times its distance (estimate)
constexpr double kClimbPerM = 1.0 / 500;  // rule 3: a land step costs (1 + |dh| / 500 m) times its distance (estimate)
constexpr double kSpacing = 0.7;          // rule 3: seeds at least this times the root of the target area apart

double dist_km(const WorldGrid& g, const Vec3& a, const Vec3& b) { return angle_between(a, b) * g.radius_km(); }

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
  return detmath::exp(-t * t) * (rain / (rain + 500.0));
}

PlacementReport placement_report(const WorldGrid& g, const WorldFields& f, const WorldLand& land, const WorldPeople& w) {
  PlacementReport r;
  const double spacing = detmath::sqrt(g.sphere_area_km2() / static_cast<double>(g.size()));
  auto dist = [&](CellId c) { return std::max(0.0, (static_cast<double>(f.sea_cells[c]) - 0.5) * spacing); };
  double total = 0, land_area = 0;
  std::vector<std::pair<double, double>> by_height;  // (height, people)
  for (CellId c = 0; c < g.size(); ++c) {
    if (!f.land(c)) continue;
    const double p = w.people[c], h = std::max(0.0, f.elevation_m[c]), d = dist(c);
    total += p;
    land_area += g.area_km2(c);
    if (d <= 100) r.coast_100 += p;
    if (d <= 200) r.coast_200 += p;
    if (d <= 400) r.coast_400 += p;
    if (h < 100) r.below_100m += p;
    if (h < 500) r.below_500m += p;
    bool water = land.river[c] != 0;
    for (const CellId nb : g.neighbours(c)) water = water || (f.land(nb) && land.lake[nb] == kLakeOpen);
    if (water) r.water_people += p, r.water_land += g.area_km2(c);
    if (land.river[c]) r.river_people += p, r.river_land += g.area_km2(c);
    if (land.river[c] == 2) r.large_people += p, r.large_land += g.area_km2(c);
    if (h < 200) r.below_200m += p;
    by_height.push_back({h, p});
  }
  if (total > 0) {
    for (double* x : {&r.coast_100, &r.coast_200, &r.coast_400, &r.below_100m, &r.below_200m, &r.below_500m, &r.water_people,
                      &r.river_people, &r.large_people})
      *x /= total;
    std::sort(by_height.begin(), by_height.end());
    double acc = 0;
    for (const auto& [h, p] : by_height)
      if ((acc += p) >= total / 2) {
        r.median_m = h;
        break;
      }
  }
  if (land_area > 0) r.water_land /= land_area, r.river_land /= land_area, r.large_land /= land_area;
  {  // concentration over land cells (EM-7, EM-8)
    std::vector<std::pair<double, double>> cells;  // (density, people)
    for (CellId c = 0; c < g.size(); ++c)
      if (f.land(c)) cells.push_back({w.people[c] / g.area_km2(c), w.people[c]});
    if (!cells.empty() && total > 0) {
      std::sort(cells.begin(), cells.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
      double acc = 0;
      std::size_t k = 0, half = 0, eighty = 0;
      for (; k < cells.size(); ++k) {
        acc += cells[k].second;
        if (!half && acc >= total / 2) half = k + 1, r.median_person_density = cells[k].first;
        if (!eighty && acc >= 0.8 * total) eighty = k + 1;
      }
      const double nc = static_cast<double>(cells.size());
      r.half_cells = static_cast<double>(half) / nc;
      r.cells_80 = static_cast<double>(eighty) / nc;
      std::size_t empty = 0;
      for (const auto& x : cells) empty += x.first < 1.0;
      r.empty_cells = static_cast<double>(empty) / nc;
      r.median_cell_density = cells[cells.size() / 2].first;
    }
  }
  std::vector<double> shares;
  for (const Country& c : w.countries) shares.push_back(c.people / std::max(1.0, w.total_people));
  std::sort(shares.rbegin(), shares.rend());
  if (!shares.empty()) r.largest_country = shares[0];
  if (shares.size() > 1) r.two_largest = shares[0] + shares[1];
  for (std::size_t i = 0; i < shares.size() && i < 10; ++i) r.ten_largest += shares[i];
  int coastal = 0;
  for (const Settlement& t : w.towns) {
    ++r.sites[static_cast<std::size_t>(t.site)];
    if (t.people < 1e6) continue;
    ++r.million_towns;
    coastal += dist(t.cell) <= 100;
  }
  r.million_coastal = r.million_towns ? static_cast<double>(coastal) / r.million_towns : 0;
  return r;
}

const char* town_site_name(TownSite s) {
  static const char* names[kTownSites] = {"inland", "port", "river mouth", "confluence", "river", "resource"};
  return names[static_cast<int>(s)];
}

double people_weight(const WorldGrid& g, const WorldFields& f, const WorldLand& land, CellId c, double food_mean, const PlacementParams& p) {
  const double s = suitability(f, c);
  if (s <= 0) return 0.0;
  const double area = g.area_km2(c);
  const double food = detmath::pow(std::max(land.capacity_food[c] / area / std::max(food_mean, 1e-9), 1e-3), p.food_exp);
  double water = 1.0;
  if (land.river[c] == 2) water *= p.large_river_w;
  else if (land.river[c] == 1) water *= p.river_w;
  for (const CellId nb : g.neighbours(c))
    if (f.land(nb) && land.lake[nb] == kLakeOpen) {
      water *= p.lake_w;
      break;
    }
  const double pet = pet_mm(f.temp_mean_c[c]);
  if (pet > 0 && f.rain_mm[c] / pet < p.arid_ai) water *= water;  // PTP-3: in dry lands people cluster at water
  const double spacing = detmath::sqrt(g.sphere_area_km2() / static_cast<double>(g.size()));
  const double d = std::max(0.0, (static_cast<double>(f.sea_cells[c]) - 0.5) * spacing);
  const double coast = 1.0 + p.coast_w * detmath::exp(-d / p.coast_km);
  const double h = std::max(0.0, f.elevation_m[c]);
  const double height = detmath::exp(-h / p.height_m) * (1.0 + p.lowland_w * detmath::exp(-h / 100.0));  // deltas and coastal plains
  return area * detmath::pow(detmath::pow(s, kAlpha) * food, p.concentration) * water * coast * height;
}

std::vector<RealProfile> load_real_profiles(const std::string& data_dir) {
  std::vector<RealProfile> out;
  const std::filesystem::path dir = std::filesystem::path(data_dir) / "setup" / "nations" / "real";
  if (!std::filesystem::exists(dir)) return out;
  for (const auto& e : std::filesystem::directory_iterator(dir)) {
    if (e.path().extension() != ".toml") continue;
    const toml::table t = toml::parse_file(e.path().string());
    RealProfile p;
    p.iso = t["id"].value_or(std::string{});
    p.name = t["name"].value_or(std::string{});
    const std::string income = t["country"]["income"].value_or(std::string{});
    p.population = t["country"]["population"].value_or(0.0);
    p.income = -1;
    for (int i = 0; i < kIncomeGroups; ++i)
      if (income == kIncomeGroupIds[static_cast<std::size_t>(i)]) p.income = i;
    if (p.income >= 0 && p.population > 0) out.push_back(std::move(p));
  }
  std::sort(out.begin(), out.end(), [](const RealProfile& a, const RealProfile& b) { return a.iso < b.iso; });  // directory order is not portable
  return out;
}

std::vector<RealProfile> load_pack_profiles(const std::string& data_dir, const std::string& pack_dir) {
  std::vector<RealProfile> all = load_real_profiles(data_dir);
  if (pack_dir.empty()) return all;
  const std::filesystem::path dir(pack_dir);
  std::vector<RealProfile> out;
  const toml::table pack = toml::parse_file((dir / "pack.toml").string());
  if (const toml::array* inc = pack["include"].as_array()) {
    bool every = false;
    std::vector<std::string> want;
    for (const auto& v : *inc)
      if (auto s = v.value<std::string>()) (*s == "all" ? every = true : (want.push_back(*s), false));
    for (const RealProfile& p : all)
      if (every || std::find(want.begin(), want.end(), p.iso) != want.end()) out.push_back(p);
  }
  for (const auto& e : std::filesystem::directory_iterator(dir)) {
    if (e.path().extension() != ".toml" || e.path().filename() == "pack.toml") continue;
    const toml::table t = toml::parse_file(e.path().string());
    if (!t["custom"].value_or(false)) continue;
    RealProfile p;
    p.iso = "custom/" + e.path().stem().string();
    p.name = t["name"].value_or(e.path().stem().string());
    p.color = t["color"].value_or(std::string{});
    const std::string income = t["country"]["income"].value_or(std::string{});
    p.population = t["country"]["population"].value_or(0.0);
    p.income = -1;
    for (int i = 0; i < kIncomeGroups; ++i)
      if (income == kIncomeGroupIds[static_cast<std::size_t>(i)]) p.income = i;
    if (p.income >= 0 && p.population > 0) out.push_back(std::move(p));
  }
  std::sort(out.begin(), out.end(), [](const RealProfile& a, const RealProfile& b) { return a.iso < b.iso; });
  return out;
}

double primacy_median(double urban_people) {
  if (urban_people <= 0) return 1.0;
  return std::min(kPrimacyCap, kPrimacyA * detmath::pow(urban_people, -kPrimacyB));
}

double primacy_share(double urban_people, double z) {
  const double s = primacy_median(urban_people) * detmath::exp(kPrimacySd * z);
  return s < kPrimacyMin ? kPrimacyMin : s > 1.0 ? 1.0 : s;
}

namespace {

bool starts_with(const std::string& s, const char* p) { return s.rfind(p, 0) == 0; }

// The word of a name to build on (blends, alliances): its longest word that is not a title or a direction
// ("United Arab Emirage" -> "Emirage"); hyphenated words stay whole ("E-stonia").
std::string core_word(const std::string& name) {
  static const char* kSkip[] = {"United", "Union", "Federation", "Republic", "Commonwealth", "Free", "State", "of", "and",
                                "the", "The", "North", "South", "East", "West", "New", "Old", "Upper", "Lower", "Greater",
                                "Northeast", "Northwest", "Southeast", "Southwest", "Saint", "Islands", "Coast"};
  std::string best, word;
  auto take = [&] {
    bool skip = word.empty();
    for (const char* k : kSkip) skip |= word == k;
    if (!skip && word.size() > best.size()) best = word;
    word.clear();
  };
  for (char c : name) {
    if (c == ' ') take();
    else word += c;
  }
  take();
  return best.empty() ? name : best;
}

// A compass direction from `from` to `to` on the sphere, without trigonometry: north by z, east along the parallel.
const char* direction(const WorldGrid& g, CellId from, CellId to) {
  const Vec3& a = g.centre(from);
  const Vec3& b = g.centre(to);
  const double north = b.z - a.z;
  const double el = detmath::sqrt(a.x * a.x + a.y * a.y);
  const double east = el > 1e-9 ? ((b.x - a.x) * -a.y + (b.y - a.y) * a.x) / el : 0.0;
  const double an = north < 0 ? -north : north, ae = east < 0 ? -east : east;
  if (an > 2 * ae) return north > 0 ? "North" : "South";
  if (ae > 2 * an) return east > 0 ? "East" : "West";
  return north > 0 ? (east > 0 ? "Northeast" : "Northwest") : (east > 0 ? "Southeast" : "Southwest");
}

}  // namespace

std::string union_name(const std::string& a, const std::string& b, std::uint64_t key) {
  const std::string x = core_word(a), y = core_word(b);
  switch (key % 5) {
    case 0: return "United " + x + " and " + y;
    case 1: return x + "-" + y + " Union";
    case 2: return "Federation of " + x + " and " + y;
    case 3: return "Commonwealth of " + x + "-" + y;
    default: {  // a blend: the first half of one and the second half of the other
      const std::size_t hx = std::max<std::size_t>(3, (x.size() + 1) / 2), hy = y.size() / 2;
      std::string tail = y.substr(std::min(hy, y.size()));
      for (char& c : tail) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
      return x.substr(0, std::min(hx, x.size())) + tail;
    }
  }
}

// A name to build on: the whole name when it is a plain one ("Fragrant Harbour"), its core word when the history already
// made it compound ("United Fjordway and Saunaland").
std::string base_name(const std::string& name) {
  static const char* kMarks[] = {" of ", "Union", "United", "Federation", "Commonwealth", "Republic", "Free State", "-",
                                 "North ", "South ", "East ", "West "};
  for (const char* m : kMarks)
    if (name.find(m) != std::string::npos) return core_word(name);
  return name;
}

std::string division_name(const std::string& parent, const char* dir, std::uint64_t key) {
  const std::string x = base_name(parent);
  switch (key % 4) {
    case 0: return std::string(dir) + " " + x;
    case 1: return "Free State of " + std::string(dir) + " " + x;
    case 2: return std::string(dir) + " " + x + " Republic";
    default: return "Republic of " + std::string(dir) + " " + x;
  }
}

std::string alliance_name(const std::string& leader, const std::string& second, const char* dir, std::uint64_t key) {
  const std::string x = core_word(leader), y = core_word(second);
  switch (key % 6) {
    case 0: return x + " Pact";
    case 1: return "League of " + x;
    case 2: return x + "-" + y + " Accord";
    case 3: return std::string(dir) + "ern Treaty Organisation";
    case 4: return "Concord of " + x;
    default: return "Organisation of " + std::string(dir) + "ern States";
  }
}

namespace {

// Spec 91: the founders take the pack's countries by people within their income group (spec 49's matching, on the
// founding map); the history's states inherit (a union its larger parent's profile, a division an unused profile of its
// parent's group or its parent's) under new names; the countries of today take their entity's.
void assign_by_history(WorldPeople& w, const WorldGrid& g, const std::vector<RealProfile>& profiles, const GroupTables& tables) {
  const WorldCountries& F = w.formed;
  const std::size_t ne = F.entities.size();
  std::size_t nf = 0;
  while (nf < ne && F.entities[nf].origin == Origin::Founder) ++nf;
  std::vector<double> people(ne, 0.0);
  for (CellId c = 0; c < F.founded_of.size(); ++c)
    if (F.founded_of[c] >= 0) people[static_cast<std::size_t>(F.founded_of[c])] += w.people[c];
  std::vector<int> income(ne, kIncomeGroups - 1);
  for (std::size_t e = 0; e < nf; ++e) {  // a founder's group: that of today's country holding its capital
    const int k = F.country_of[F.entities[e].capital];
    if (k >= 0) income[e] = w.countries[static_cast<std::size_t>(k)].income;
  }
  std::vector<std::string> profile(ne), name(ne), color(ne);
  std::vector<bool> used(profiles.size(), false);
  std::vector<int> reused(profiles.size(), 0);
  std::set<std::string> names;
  const double scale = tables.world_population > 0 ? w.total_people / tables.world_population : 1.0;
  auto nearest = [&](int group, double ppl, bool unused_only) {
    int best = -1;
    double gap = 1e300;
    for (std::size_t i = 0; i < profiles.size(); ++i) {
      if ((unused_only && used[i]) || profiles[i].income != group) continue;
      const double d = std::abs(detmath::log(std::max(1.0, ppl)) - detmath::log(profiles[i].population * scale));
      if (d < gap) gap = d, best = static_cast<int>(i);
    }
    return best;
  };
  auto unique = [&](std::string n) {
    static const char* kPrefixes[] = {"New", "Greater", "Upper", "Lower", "Old", "Free"};
    for (int k = 0; names.count(n); ++k) n = std::string(kPrefixes[k % 6]) + (k >= 6 ? " " + std::to_string(k / 6 + 1) : "") + " " + n;
    names.insert(n);
    return n;
  };
  std::vector<std::size_t> by_people(nf);
  std::iota(by_people.begin(), by_people.end(), std::size_t{0});
  std::stable_sort(by_people.begin(), by_people.end(), [&](std::size_t a, std::size_t b) { return people[a] > people[b]; });
  for (std::size_t e : by_people) {
    int best = -1;
    for (int step = 0; step < kIncomeGroups && best < 0; ++step)
      for (int sgn : {1, -1}) {
        const int grp = income[e] + sgn * step;
        if (best < 0 && grp >= 0 && grp < kIncomeGroups) best = nearest(grp, people[e], true);
        if (step == 0) break;
      }
    std::string prefix;
    if (best < 0) {  // the pack is spent: its own group's nearest under a derived name
      best = nearest(income[e], people[e], false);
      for (int step = 1; best < 0 && step < kIncomeGroups; ++step)
        for (int sgn : {1, -1})
          if (best < 0 && income[e] + sgn * step >= 0 && income[e] + sgn * step < kIncomeGroups) best = nearest(income[e] + sgn * step, people[e], false);
      if (best < 0) continue;
      ++reused[static_cast<std::size_t>(best)];
    }
    used[static_cast<std::size_t>(best)] = true;
    profile[e] = profiles[static_cast<std::size_t>(best)].iso;
    color[e] = profiles[static_cast<std::size_t>(best)].color;
    income[e] = profiles[static_cast<std::size_t>(best)].income;  // a pack country keeps its own group (spec 90)
    name[e] = unique(prefix + profiles[static_cast<std::size_t>(best)].name);
  }
  for (std::size_t e = nf; e < ne; ++e) {
    const Entity& E = F.entities[e];
    const auto a = static_cast<std::size_t>(E.a);
    if (E.origin == Origin::Union) {
      const auto b = static_cast<std::size_t>(E.b);
      const std::size_t big = people[a] >= people[b] ? a : b;
      people[e] = people[a] + people[b];
      profile[e] = profile[big];
      color[e] = color[big];  // spec 92: a union keeps its larger parent's colour
      income[e] = income[big];
      name[e] = unique(union_name(name[a], name[b], e * 2654435761u + static_cast<std::uint64_t>(E.period)));
    } else {
      people[e] = 0.3 * people[a];
      people[a] *= 0.7;
      income[e] = income[a];
      const int fresh = nearest(income[e], people[e], true);
      if (fresh >= 0) used[static_cast<std::size_t>(fresh)] = true;
      profile[e] = fresh >= 0 ? profiles[static_cast<std::size_t>(fresh)].iso : profile[a];
      name[e] = unique(division_name(name[a], direction(g, F.entities[a].capital, E.capital), e * 40503u + static_cast<std::uint64_t>(E.period)));
    }
  }
  w.entity_names = name;
  for (std::size_t k = 0; k < w.countries.size(); ++k) {
    Country& c = w.countries[k];
    const int e = F.countries[k].entity;
    if (e < 0) continue;
    const Entity& E = F.entities[static_cast<std::size_t>(e)];
    c.profile = profile[static_cast<std::size_t>(e)];
    c.color = color[static_cast<std::size_t>(e)];
    c.name = name[static_cast<std::size_t>(e)];
    c.income = income[static_cast<std::size_t>(e)];
    c.origin = E.origin == Origin::Founder ? "founded"
               : E.origin == Origin::Union ? "union of " + name[static_cast<std::size_t>(E.a)] + " and " + name[static_cast<std::size_t>(E.b)] +
                                                 " in history period " + std::to_string(E.period)
                                           : "split from " + name[static_cast<std::size_t>(E.a)] + " in history period " + std::to_string(E.period);
  }
  for (std::size_t i = 0; i < F.alliances.size(); ++i) {
    const Alliance& al = F.alliances[i];
    const std::string& lead = w.countries[static_cast<std::size_t>(al.leader)].name;
    const std::string& second = w.countries[static_cast<std::size_t>(al.members.front() == al.leader && al.members.size() > 1 ? al.members[1] : al.members.front())].name;
    // The bloc's direction from the planet's centre of land is not needed: from its leader to its first other member.
    const CellId from = w.countries[static_cast<std::size_t>(al.leader)].capital_cell;
    const int other = al.members.front() == al.leader && al.members.size() > 1 ? al.members[1] : al.members.front();
    std::string an = alliance_name(lead, second, direction(g, from, w.countries[static_cast<std::size_t>(other)].capital_cell), i * 97 + 13);
    for (std::uint64_t k = 1; std::find(w.alliance_names.begin(), w.alliance_names.end(), an) != w.alliance_names.end() && k < 12; ++k)
      an = alliance_name(lead, second, "North", i * 97 + 13 + k);  // a taken name: another form
    if (std::find(w.alliance_names.begin(), w.alliance_names.end(), an) != w.alliance_names.end()) an = lead + " Alliance";
    w.alliance_names.push_back(an);
    for (int m : al.members) w.countries[static_cast<std::size_t>(m)].alliance = static_cast<int>(i);
  }
}

}  // namespace

WorldPeople populate_world(const WorldGrid& g, const WorldFields& f, const WorldLand& land, const WorldDeposits& deposits,
                           std::uint64_t seed, const GroupTables& tables, const std::vector<RealProfile>& profiles,
                           const CountryParams& country_params) {
  const std::size_t n = g.size();
  WorldPeople w;
  w.suitability.assign(n, 0.0);
  w.people.assign(n, 0.0);
  w.sector_of.assign(n, -1);

  // Rules 1, 2: suitability and people.
  // Spec 86 rule 1: by climate, food, fresh water, coast and height.
  const PlacementParams pp{};
  double food_sum = 0, food_area = 0;
  for (CellId c = 0; c < n; ++c)
    if (f.land(c)) food_sum += land.capacity_food[c], food_area += g.area_km2(c);
  const double food_mean = food_area > 0 ? food_sum / food_area : 1.0;
  double weight_sum = 0;
  std::vector<double> weight(n, 0.0);
  for (CellId c = 0; c < n; ++c) {
    if (f.land(c)) w.land_km2 += g.area_km2(c);
    w.suitability[c] = suitability(f, c);
    weight[c] = people_weight(g, f, land, c, food_mean, pp);
    weight_sum += weight[c];
  }
  const double total = tables.world_population * w.land_km2 / kEarthLandKm2 * country_params.people_scale;
  for (CellId c = 0; c < n; ++c) {
    w.people[c] = weight_sum > 0 ? std::round(total * weight[c] / weight_sum) : 0.0;
    w.total_people += w.people[c];
  }

  // Spec 52 slice 4: countries from the land, then sectors cut inside each (spec 48 rule 3 per country).
  std::vector<CellId> land_cells;
  for (CellId c = 0; c < n; ++c)
    if (f.land(c) && f.biome[c] != Biome::Ice) land_cells.push_back(c);  // ice is in no country (spec 52 rule 24)
  if (land_cells.empty()) return w;
  const LandGraph graph = build_land_graph(g, f);
  // Spec 91: unless told otherwise, as many founding states as the pack has countries (no profiles: the old contest).
  CountryParams cp = country_params;
  if (cp.founders == 0) cp.founders = profiles.empty() ? -1 : static_cast<int>(profiles.size());
  w.formed = form_countries(g, f, land, graph, seed, cp);
  const double spacing = kSpacing * detmath::sqrt(kSectorTargetKm2);
  w.countries.resize(w.formed.countries.size());
  {
    std::vector<CellId> seeds;
    for (std::size_t k = 0; k < w.formed.countries.size(); ++k) {
      const CountryShape& shape = w.formed.countries[k];
      const std::size_t count = std::min(shape.cells.size(),
                                         std::max<std::size_t>(1, static_cast<std::size_t>(std::llround(shape.area_km2 / kSectorTargetKm2))));
      std::vector<std::size_t> order(shape.cells.size());
      std::iota(order.begin(), order.end(), std::size_t{0});
      std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return w.people[shape.cells[a]] > w.people[shape.cells[b]]; });
      std::vector<Vec3> at;
      for (CellId c : shape.cells) at.push_back(g.centre(c));
      for (std::size_t i : spaced(at, order, count, spacing, g)) seeds.push_back(shape.cells[i]);
    }
    // Multi-source shortest paths that never cross a border: land steps and one-sea-cell crossings within a country.
    std::vector<double> best(n, 1e300);
    using Item = std::tuple<double, int, CellId>;
    std::priority_queue<Item, std::vector<Item>, std::greater<>> q;
    for (std::size_t s = 0; s < seeds.size(); ++s) {
      best[seeds[s]] = 0;
      w.sector_of[seeds[s]] = static_cast<int>(s);
      q.emplace(0.0, static_cast<int>(s), seeds[s]);
    }
    while (!q.empty()) {
      const auto [d, s, c] = q.top();
      q.pop();
      if (d > best[c] || s != w.sector_of[c]) continue;
      for (const LandLink& l : graph.links[c]) {
        if (w.formed.country_of[l.to] != w.formed.country_of[c]) continue;
        const double dist = g.distance_km(c, l.to);
        const double nd = d + (l.sea ? kSeaCost * dist : dist * (1.0 + std::abs(f.elevation_m[c] - f.elevation_m[l.to]) * kClimbPerM));
        if (nd < best[l.to] || (nd == best[l.to] && s < w.sector_of[l.to])) {
          best[l.to] = nd;
          w.sector_of[l.to] = s;
          q.emplace(nd, s, l.to);
        }
      }
    }
    w.sectors.resize(seeds.size());
    for (std::size_t s = 0; s < seeds.size(); ++s) {
      w.sectors[s].seed = seeds[s];
      w.sectors[s].country = w.formed.country_of[seeds[s]];
    }
    // Spec 91: a founder's exclave (an island or a piece cut off) is reached by no path inside its country: it takes
    // its country's nearest sector seat.
    for (CellId c : land_cells) {
      if (w.sector_of[c] >= 0 || w.formed.country_of[c] < 0) continue;
      double bd = 1e300;
      for (std::size_t s = 0; s < seeds.size(); ++s)
        if (w.sectors[s].country == w.formed.country_of[c])
          if (const double d = g.distance_km(c, seeds[s]); d < bd) bd = d, w.sector_of[c] = static_cast<int>(s);
    }
    for (CellId c : land_cells) {
      if (w.sector_of[c] < 0 || w.formed.country_of[c] < 0)
        throw std::logic_error("populate_world: land cell " + std::to_string(c) + " has no country or sector");
      Sector& sec = w.sectors[static_cast<std::size_t>(w.sector_of[c])];
      sec.cells.push_back(c);
      sec.people += w.people[c];
      sec.area_km2 += g.area_km2(c);
    }
    for (std::size_t s = 0; s < w.sectors.size(); ++s) {
      Country& c = w.countries[static_cast<std::size_t>(w.sectors[s].country)];
      c.sectors.push_back(static_cast<int>(s));
      c.people += w.sectors[s].people;
    }
    for (std::size_t k = 0; k < w.countries.size(); ++k) {
      const CountryShape& shape = w.formed.countries[k];
      Country& c = w.countries[k];
      c.capital_cell = shape.capital;
      c.capital = w.sector_of[shape.capital];
      c.area_km2 = shape.area_km2;
      c.landlocked = shape.landlocked;
    }
  }

  // Spec 52 rule 26: wealth = endowment x access x history; income groups filled from it (spec 48 rule 5).
  {
    const std::size_t nc = w.countries.size();
    enum { Food, Water, Energy, Metal, Wood, Fish, Kinds };
    std::vector<std::array<double, Kinds>> amount(nc, std::array<double, Kinds>{});
    for (CellId c : land_cells) {
      auto& a = amount[static_cast<std::size_t>(w.formed.country_of[c])];
      a[Food] += land.capacity_food[c];
      a[Water] += land.capacity_water[c];
      a[Wood] += land.wood_m3[c];
    }
    for (const Deposit& d : deposits.deposits) {
      const int k = w.formed.country_of[d.cell];
      if (k < 0) continue;  // offshore: no country holds the sea in this slice
      auto& a = amount[static_cast<std::size_t>(k)];
      switch (d.kind) {
        case Resource::Coal: a[Energy] += d.amount * 24.0; break;   // GJ per t
        case Resource::Oil: a[Energy] += d.amount * 6.1; break;     // GJ per bbl
        case Resource::Gas: a[Energy] += d.amount * 0.038; break;   // GJ per m3
        default: a[Metal] += d.amount * d.grade; break;
      }
    }
    for (CellId c = 0; c < n; ++c) {  // each sea cell's catch, shared by the countries beside it
      if (f.land(c) || land.fish_catch_t[c] <= 0) continue;
      std::vector<int> beside;
      for (CellId nb : g.neighbours(c))
        if (w.formed.country_of[nb] >= 0) beside.push_back(w.formed.country_of[nb]);  // ice coasts are no one's
      std::sort(beside.begin(), beside.end());
      beside.erase(std::unique(beside.begin(), beside.end()), beside.end());
      for (int k : beside) amount[static_cast<std::size_t>(k)][Fish] += land.fish_catch_t[c] / static_cast<double>(beside.size());
    }
    std::vector<double> endow(nc, 0.0);
    for (int kind = 0; kind < Kinds; ++kind) {
      std::vector<std::pair<double, std::size_t>> per;
      for (std::size_t k = 0; k < nc; ++k)
        per.push_back({amount[k][static_cast<std::size_t>(kind)] / std::max(1.0, w.countries[k].people), k});
      std::stable_sort(per.begin(), per.end());
      for (std::size_t i = 0; i < nc; ++i)
        endow[per[i].second] += (nc > 1 ? static_cast<double>(i) / static_cast<double>(nc - 1) : 1.0) / static_cast<double>(Kinds);
    }
    const Keyed r(seed);
    for (std::size_t k = 0; k < nc; ++k) {
      Country& c = w.countries[k];
      const CountryShape& shape = w.formed.countries[k];
      c.endowment = endow[k];
      const double access = shape.coast ? 1.0 : 0.6;
      const double z = r.bell(k, field::CountryLuck) / 0.2887;  // about a unit normal
      const double age = w.formed.periods > 0 ? static_cast<double>(shape.capital_held) / w.formed.periods : 0.0;
      c.development = c.endowment * access * detmath::exp(0.25 * z) * (1.0 + 0.25 * age);
    }
  }
  std::vector<std::size_t> by_dev(w.countries.size());
  std::iota(by_dev.begin(), by_dev.end(), std::size_t{0});
  std::stable_sort(by_dev.begin(), by_dev.end(), [&](std::size_t a, std::size_t b) { return w.countries[a].development > w.countries[b].development; });
  const double group_total = std::accumulate(tables.population.begin(), tables.population.end(), 0.0);
  // Spec 78 rule 1: a group holds countries of its real sizes: none larger than the tolerance times the group's largest
  // real profile, scaled to this planet's people. Without profiles no group is closed.
  std::array<double, kIncomeGroups> largest{};
  for (const RealProfile& p : profiles) largest[static_cast<std::size_t>(p.income)] = std::max(largest[static_cast<std::size_t>(p.income)], p.population);
  const double planet_scale = tables.world_population > 0 ? w.total_people / tables.world_population : 1.0;
  auto fits = [&](int group, double people_count) {
    const double cap = largest[static_cast<std::size_t>(group)];
    return cap <= 0 || people_count <= kGroupSizeTolerance * cap * planet_scale;
  };
  // Each group keeps its own quota (spec 89 second amendment): a country takes the first group, in order of
  // development, with room for at least half of it that it fits; a large country sent elsewhere leaves its group's
  // quota to the next countries instead of shifting every later edge.
  std::array<double, kIncomeGroups> room{};
  for (int i = 0; i < kIncomeGroups; ++i)
    room[static_cast<std::size_t>(i)] = w.total_people * tables.population[static_cast<std::size_t>(i)] / group_total;
  for (std::size_t k : by_dev) {
    Country& c = w.countries[k];
    int pick = -1;
    for (int i = 0; i < kIncomeGroups && pick < 0; ++i)
      if (room[static_cast<std::size_t>(i)] >= c.people / 2 && fits(i, c.people)) pick = i;
    if (pick < 0) {  // no room left anywhere it fits: the group with the most room among those it fits, else the poorest
      double best = -1e300;
      for (int i = 0; i < kIncomeGroups; ++i)
        if (fits(i, c.people) && room[static_cast<std::size_t>(i)] > best) best = room[static_cast<std::size_t>(i)], pick = i;
      if (pick < 0) pick = kIncomeGroups - 1;
    }
    c.income = pick;
    room[static_cast<std::size_t>(pick)] -= c.people;
  }

  if (!w.formed.entities.empty()) assign_by_history(w, g, profiles, tables);  // spec 91
  else {
  // Spec 49: each country takes the unused real profile of its group nearest in people (scaled to this planet: the
  // profile's people times this planet's people over Earth's), largest countries first.
  {
    std::vector<bool> used(profiles.size(), false);
    std::vector<int> reused(profiles.size(), 0);
    std::vector<std::size_t> by_people(w.countries.size());
    std::iota(by_people.begin(), by_people.end(), std::size_t{0});
    std::stable_sort(by_people.begin(), by_people.end(), [&](std::size_t a, std::size_t b) { return w.countries[a].people > w.countries[b].people; });
    const double scale = w.total_people / tables.world_population;
    for (std::size_t k : by_people) {
      Country& c = w.countries[k];
      // Its own group first; when that group's profiles are all taken (spec 52 makes more, smaller countries than
      // spec 48 did), the nearest group that still has one.
      int best = -1;
      for (int step = 0; step < kIncomeGroups && best < 0; ++step) {
        double gap = 1e300;
        for (std::size_t i = 0; i < profiles.size(); ++i) {
          if (used[i] || std::abs(profiles[i].income - c.income) != step) continue;
          const double d = std::abs(detmath::log(std::max(1.0, c.people)) - detmath::log(profiles[i].population * scale));
          if (d < gap) { gap = d; best = static_cast<int>(i); }
        }
      }
      // All profiles taken (an Earth-size world forms more countries than Earth's 183 profiles): reuse its own
      // group's nearest under a derived name, so names stay distinct.
      std::string prefix;
      if (best < 0) {
        double gap = 1e300;
        for (std::size_t i = 0; i < profiles.size(); ++i) {
          if (profiles[i].income != c.income) continue;
          const double d = std::abs(detmath::log(std::max(1.0, c.people)) - detmath::log(profiles[i].population * scale));
          if (d < gap) { gap = d; best = static_cast<int>(i); }
        }
        if (best < 0) continue;
        static const char* kPrefixes[] = {"New", "North", "South", "East", "West", "Upper", "Lower", "Greater"};
        const int n_reuse = reused[static_cast<std::size_t>(best)]++;
        prefix = std::string(kPrefixes[n_reuse % 8]) + (n_reuse >= 8 ? " " + std::to_string(n_reuse / 8 + 1) : "") + " ";
      }
      used[static_cast<std::size_t>(best)] = true;
      c.profile = profiles[static_cast<std::size_t>(best)].iso;
      c.color = profiles[static_cast<std::size_t>(best)].color;
      c.name = prefix + profiles[static_cast<std::size_t>(best)].name;
    }
  }
  }

  // Spec 86 rule 2: a cell's site for a town.
  std::vector<std::uint8_t> feeders(n, 0);
  for (CellId c = 0; c < n; ++c)
    if (f.land(c) && land.river[c] && land.downstream[c] != kNoCell && f.land(land.downstream[c]) && feeders[land.downstream[c]] < 255)
      ++feeders[land.downstream[c]];
  auto site_of = [&](CellId c, double& factor) {
    const bool coastal = f.sea_cells[c] == 1;
    const bool mouth = land.river[c] && land.downstream[c] != kNoCell && !f.land(land.downstream[c]);
    factor = 1.0 + (coastal ? pp.port_w : 0.0) + (mouth ? pp.mouth_w : 0.0) + (land.river[c] == 2 ? pp.river_w2 : 0.0) +
             (feeders[c] >= 2 ? pp.confluence_w : 0.0);
    return mouth ? TownSite::Mouth : feeders[c] >= 2 ? TownSite::Confluence : coastal ? TownSite::Port
                                                                                   : land.river[c] ? TownSite::River : TownSite::Inland;
  };
  // Spec 86 rule 3: the large deposits on land, by country (a cell once).
  std::vector<std::vector<CellId>> resource_cells(w.countries.size());
  {
    double median[resource_count] = {};
    for (int r = 0; r < resource_count; ++r) {
      std::vector<double> a;
      for (const Deposit& d : deposits.deposits)
        if (static_cast<int>(d.kind) == r) a.push_back(d.amount);
      if (a.empty()) continue;
      std::nth_element(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(a.size() / 2), a.end());
      median[r] = a[a.size() / 2];
    }
    for (const Deposit& d : deposits.deposits) {
      if (!f.land(d.cell) || median[static_cast<int>(d.kind)] <= 0 || d.amount < pp.resource_rel * median[static_cast<int>(d.kind)]) continue;
      const int sec = w.sector_of[d.cell];
      if (sec < 0) continue;
      const int k = w.sectors[static_cast<std::size_t>(sec)].country;
      if (k < 0) continue;
      auto& v = resource_cells[static_cast<std::size_t>(k)];
      if (std::find(v.begin(), v.end(), d.cell) == v.end()) v.push_back(d.cell);
    }
    for (auto& v : resource_cells) std::sort(v.begin(), v.end());
  }

  // Rule 6: towns by rank-size in each country's most populous cells (spec 78 rules 2-3: the largest from primacy, the
  // others by the country's own exponent).
  for (std::size_t k = 0; k < w.countries.size(); ++k) {
    const Country& c = w.countries[k];
    const double urban_all = c.people * tables.urban_share[static_cast<std::size_t>(c.income)];
    if (urban_all < kTownMinPeople) continue;
    // Spec 86 rule 3: resource towns first, out of the urban people (at most a quarter of them).
    const std::vector<CellId>& rc = resource_cells[k];
    const double reserve = std::min(0.25 * urban_all, static_cast<double>(rc.size()) * pp.resource_town);
    const double urban = urban_all - reserve;
    if (urban < kTownMinPeople) continue;
    // Spec 78 rule 2: the largest town from primacy.
    const double z = Keyed(seed).bell(k, field::Primacy) / 0.2887;  // about a unit normal, as the country's luck
    double first = std::max(kTownMinPeople, std::min({urban, kTownMaxPeople, primacy_share(urban_all, z) * urban_all}));  // spec 86: primacy of all the urban people
    // Spec 78 rule 3: town i = first x i^(-1/zeta) down to the smallest town; zeta in the real range so they hold the
    // urban people. The sum is found in closed form (Euler-Maclaurin) for the search; the towns are then placed one by one
    // until the urban people are used up (the last takes what is left, none under the smallest size).
    auto sum_for = [&](double zeta, double top) {
      const double a = 1.0 / zeta, K = std::max(1.0, std::floor(detmath::pow(top / kTownMinPeople, zeta)));
      const double tail = std::abs(1 - a) < 1e-9 ? detmath::log(K) : (detmath::pow(K, 1 - a) - 1) / (1 - a);
      return top * (tail + 0.5 * (1 + detmath::pow(K, -a)));
    };
    auto approx_sum = [&](double zeta) { return sum_for(zeta, first); };
    // Even the flattest real exponent cannot hold the urban people under so small a largest town: the largest town grows
    // until they fit (up to the urban people or the largest agglomeration), rather than a spike of smallest towns.
    if (sum_for(kZetaMax, first) < urban) {
      double lo = first, hi = std::min(urban, std::max(first, kTownMaxPeople));
      if (sum_for(kZetaMax, hi) <= urban) {
        first = hi;
      } else {
        for (int it = 0; it < 60; ++it) {
          const double mid = 0.5 * (lo + hi);
          (sum_for(kZetaMax, mid) < urban ? lo : hi) = mid;
        }
        first = hi;
      }
    }
    double zeta = kZetaMin;
    if (approx_sum(kZetaMax) <= urban) {
      zeta = kZetaMax;
    } else if (approx_sum(kZetaMin) < urban) {
      double lo = kZetaMin, hi = kZetaMax;
      for (int it = 0; it < 60; ++it) {
        const double mid = 0.5 * (lo + hi);
        (approx_sum(mid) < urban ? lo : hi) = mid;
      }
      zeta = 0.5 * (lo + hi);
    }
    std::vector<std::pair<double, CellId>> left;  // people not yet in a town, by cell
    for (int s : c.sectors)
      for (CellId cell : w.sectors[static_cast<std::size_t>(s)].cells) left.push_back({w.people[cell], cell});
    double remaining = urban;
    for (int i = 1; remaining >= kTownMinPeople; ++i) {
      const double size = std::round(std::min(remaining, std::max(kTownMinPeople, first * detmath::pow(i, -1.0 / zeta))));
      // Spec 86 rule 2: the people left in a cell times its site.
      auto score = [&](const std::pair<double, CellId>& x) {
        double fac = 1;
        site_of(x.second, fac);
        return x.first * fac;
      };
      auto it = std::max_element(left.begin(), left.end(), [&](const auto& a, const auto& b) {
        const double sa = score(a), sb = score(b);
        return sa < sb || (sa == sb && a.second > b.second);
      });
      double fac = 1;
      const TownSite site = site_of(it->second, fac);
      w.towns.push_back({it->second, static_cast<int>(k), i, size, site});
      it->first -= size;
      remaining -= size;
    }
    // Spec 86 rule 3: a resource town at each large deposit with no town yet. What the reserve does not use (a deposit
    // whose cell already has a town, the remainder under the smallest town) goes to the country's last rank-size town,
    // which already holds what is left (spec 78 rule 3), so the country's towns still hold its urban people.
    std::size_t last = w.towns.size();
    int rank = 0;
    for (std::size_t ti = 0; ti < w.towns.size(); ++ti)
      if (w.towns[ti].country == static_cast<int>(k) && w.towns[ti].rank > rank) rank = w.towns[ti].rank, last = ti;
    double left_reserve = reserve;
    for (const CellId cell : rc) {
      if (left_reserve < kTownMinPeople) break;
      bool taken = false;
      for (const Settlement& t : w.towns) taken = taken || (t.country == static_cast<int>(k) && t.cell == cell);
      if (taken) continue;
      const double size = std::min(pp.resource_town, left_reserve);
      w.towns.push_back({cell, static_cast<int>(k), ++rank, size, TownSite::Resource});
      left_reserve -= size;
    }
    if (left_reserve > 0 && last < w.towns.size()) w.towns[last].people += left_reserve;
    w.countries[k].zeta = zeta;
  }
  return w;
}

}  // namespace zs::gen
