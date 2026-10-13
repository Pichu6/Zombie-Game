// Spec 72: what stands on a town's plots. docs/tier0/specs/72-what-stands-on-a-plot.md
#include "zs/gen/plot_contents.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <span>
#include <sstream>
#include <stdexcept>

#include "toml.hpp"
#include "zs/gen/keyed.h"

namespace zs::gen {
namespace {

enum Field : std::uint32_t { kSizeGroup = 1, kSizeIn = 2, kCouple = 3, kTree = 4, kBench = 5 };

double area_per_person(const DwellingRules& dr, const std::string& income) {
  const auto it = dr.area_per_person_m2.find(income);
  return it == dr.area_per_person_m2.end() ? 30.0 : it->second;
}

const Plot& plot_at(const TownLayout& t, int plot) {
  if (plot < 0 || plot >= static_cast<int>(t.plots.size())) throw std::out_of_range("plot " + std::to_string(plot) + " is not in the town");
  return t.plots[static_cast<std::size_t>(plot)];
}

// The plot seen from its street: frontage along the street, depth back from it.
std::pair<int, int> seen_from_street(const TownLayout& t, const Plot& p) {
  const bool vertical = t.streets[static_cast<std::size_t>(p.street)].vertical;
  return vertical ? std::pair{p.h, p.w} : std::pair{p.w, p.h};
}

}  // namespace

PlotRules load_plot_rules(const std::string& gen_dir) {
  const std::string path = gen_dir + "/plots.toml";
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  PlotRules r;
  r.couple_share = t["couple_share"].value_or(r.couple_share);
  r.coverage_commercial = t["coverage_commercial"].value_or(r.coverage_commercial);
  r.coverage_industrial = t["coverage_industrial"].value_or(r.coverage_industrial);
  r.trees_per_100m2 = t["trees_per_100m2"].value_or(r.trees_per_100m2);
  r.benches_per_400m2 = t["benches_per_400m2"].value_or(r.benches_per_400m2);
  if (r.coverage_commercial <= 0 || r.coverage_commercial > 1 || r.coverage_industrial <= 0 || r.coverage_industrial > 1)
    throw std::runtime_error(path + ": a coverage out of range");
  return r;
}

std::vector<Home> homes_on(const TownSpec& spec, const TownLayout& t, int plot, const PlotRules& rules, std::uint64_t seed) {
  const Plot& p = plot_at(t, plot);
  const Keyed r(mix64(seed, hash_str("homes")));
  std::vector<Home> out;
  double shares = 0;
  for (double s : spec.household_shares) shares += s;
  for (int i = 0; i < p.households; ++i) {
    const std::uint64_t id = mix64(static_cast<std::uint64_t>(plot), static_cast<std::uint64_t>(i));
    Home h;
    if (shares > 0) {
      // Rule 1: the UN groups 1, 2-3, 4-5 and 6+ (spec 45), then an even draw within the group (6+ as 6 or 7, an estimate).
      const std::size_t g = r.weighted(id, kSizeGroup, std::span<const double>(spec.household_shares.data(), spec.household_shares.size()));
      static const int low[4] = {1, 2, 4, 6};
      h.members = g == 0 ? 1 : low[g] + (r.chance(id, kSizeIn, 0.5) ? 1 : 0);
    } else {
      h.members = std::max(1, static_cast<int>(std::lround(spec.mean_household_size)));
    }
    h.couple = h.members >= 2 && r.chance(id, kCouple, rules.couple_share);
    out.push_back(h);
  }
  return out;
}

PlotPlan plan_plot(const TownSpec& spec, const TownLayout& t, int plot, const DwellingRules& dr, const PlotRules& rules,
                   std::uint64_t seed, Frame frame) {
  const Plot& p = plot_at(t, plot);
  PlotPlan out;
  auto [frontage, depth] = seen_from_street(t, p);
  if (frame.w > 0) frontage = std::min(frontage, frame.w);  // spec 73: cut to the frame around the building
  if (frame.h > 0) depth = std::min(depth, frame.h);
  out.frontage_m = frontage;
  out.depth_m = depth;
  if (p.use == LandUse::Park) {
    out.kind = "park";
    return out;
  }
  if (p.use == LandUse::Civic) {
    out.kind = "civic";
    return out;
  }
  if (p.use == LandUse::Residential && p.lot >= 0) {
    out.kind = "block";
    return out;
  }
  // Rule 2: the building's frame: side gaps where the frontage allows them (a narrow plot builds wall to wall), a front
  // and a back yard as deep as the plot allows.
  const int side = frontage >= 9 + 2 * dr.lot_side ? dr.lot_side : 0;
  const int front = std::min(dr.lot_front, std::max(1, depth / 6));
  const int back = std::min(dr.lot_back, std::max(1, depth / 6));
  const int max_w = std::max(5, frontage - 2 * side);
  const int max_depth = std::max(4, depth - front - back);
  const double mean = std::max(1.0, spec.mean_household_size);
  Dwelling d;
  if (p.use == LandUse::Residential) {
    out.homes = homes_on(spec, t, plot, rules, seed);
    if (p.units <= 1) {
      // Rule 3: a house for its household (spec 51), or for a household of the mean size if it stands empty.
      out.kind = "house";
      const Home h = out.homes.empty() ? Home{static_cast<int>(std::lround(mean)), mean >= 2} : out.homes.front();
      d = plan_house(plot, h.members, h.couple, dr, spec.income, seed, max_w);
    } else {
      // Rule 4: an apartment house: a stair hall at the front, a flat per unit sized for its household (an empty one for
      // a household of the mean size), on as many floors as the plot's depth needs.
      out.kind = "apartments";
      std::vector<Room> rooms;
      std::vector<double> target;
      Room hall;
      hall.type = "hall";
      rooms.push_back(hall);
      target.push_back(8);
      for (int u = 0; u < p.units; ++u) {
        const int members = u < static_cast<int>(out.homes.size()) ? out.homes[static_cast<std::size_t>(u)].members : static_cast<int>(std::lround(mean));
        Room flat;
        flat.type = "flat";
        flat.occupants = members;
        rooms.push_back(flat);
        target.push_back(std::max(dr.min_area_m2, members * area_per_person(dr, spec.income)));
      }
      d.fridge = std::find(dr.no_fridge.begin(), dr.no_fridge.end(), spec.income) == dr.no_fridge.end();
      lay_out_rooms(d, std::move(rooms), target, std::min(9, max_w), max_w, dr, max_depth, "hall");
      d.fridge = std::find(dr.no_fridge.begin(), dr.no_fridge.end(), spec.income) == dr.no_fridge.end();
      const double day = mean * dr.food_kj_per_person_day;
      const auto pantry = dr.pantry_days.find(spec.income);
      d.pantry_kj = static_cast<std::int64_t>(std::llround(day * (pantry == dr.pantry_days.end() ? 7.0 : pantry->second)));
      d.fridge_kj = d.fridge ? static_cast<std::int64_t>(std::llround(day * dr.fridge_days)) : 0;
    }
  } else {
    // Rule 5: a shop or a workshop covering its share of the plot: the counter or the office at the front.
    const bool shop = p.use == LandUse::Commercial;
    const double covered = (shop ? rules.coverage_commercial : rules.coverage_industrial) * frontage * depth;
    std::vector<Room> rooms;
    std::vector<double> target;
    const auto add = [&](const std::string& type, double m2, int seats = 0, const std::string& seat = "bed") {
      Room room;
      room.type = type;
      room.occupants = seats;
      room.seat = seat;
      rooms.push_back(room);
      target.push_back(std::max(4.0, m2));
    };
    if (shop) {
      out.kind = "shop";
      add("shop", covered * 0.65);
      add("storeroom", covered * 0.30);
    } else {
      out.kind = "workshop";
      add("office", 10, 1, "desk");
      add("workshop", covered * 0.60);
      add("storeroom", covered * 0.25);
    }
    add("washroom", 4);
    d.fridge = false;
    lay_out_rooms(d, std::move(rooms), target, std::min(9, max_w), max_w, dr, max_depth, shop ? "shop" : "office");
  }
  // The building on its plot, centred on the frontage, behind its front yard.
  d.household = 2'000'000 + plot;  // keys its furniture apart from every house and station
  d.street = t.streets[static_cast<std::size_t>(p.street)].name;
  d.number = p.number;
  d.address = p.address;
  d.lot_width = frontage;
  d.lot_depth = depth;
  d.house_x = std::max(0, (frontage - d.width) / 2);
  d.house_y = std::min(front, std::max(1, depth - d.depth - 1));
  d.lot_depth = std::max(depth, d.house_y + d.depth + 1);  // never cut: a building deeper than its plot shows as a longer lot
  d.lot_width = std::max(frontage, d.house_x + d.width);
  out.building = std::move(d);
  return out;
}

std::string plot_toml(const TownSpec& spec, const TownLayout& t, int plot, const DwellingRules& dr, const PlotRules& rules,
                      std::uint64_t seed, Frame frame) {
  const Plot& p = plot_at(t, plot);
  const PlotPlan plan = plan_plot(spec, t, plot, dr, rules, seed, frame);
  if (plan.kind == "civic" || plan.kind == "block") return "";
  if (plan.kind == "park") {
    // Rule 6: a park is grass behind the street row, with trees and benches at keyed cells.
    const int w = plan.frontage_m, h = plan.depth_m;
    const Keyed r(mix64(seed, hash_str("park")));
    std::set<std::pair<int, int>> used;
    std::vector<std::pair<std::string, std::pair<int, int>>> objects;
    const auto place = [&](const char* kind, int count, std::uint32_t field) {
      for (int i = 0, tries = 0; i < count && tries < count * 20; ++tries) {
        const auto id = mix64(static_cast<std::uint64_t>(plot), static_cast<std::uint64_t>(tries));
        const int x = static_cast<int>(r.below(id, field, static_cast<std::uint64_t>(w)));
        const int y = 1 + static_cast<int>(r.below(id, field + 10, static_cast<std::uint64_t>(std::max(1, h - 1))));
        if (!used.insert({x, y}).second) continue;
        objects.push_back({kind, {x, y}});
        ++i;
      }
    };
    place("tree", static_cast<int>(std::lround(w * h / 100.0 * rules.trees_per_100m2)), kTree);
    place("bench", static_cast<int>(std::lround(w * h / 400.0 * rules.benches_per_400m2)), kBench);
    std::ostringstream o;
    o << "# Generated by zs gen plot (spec 72): " << p.address << ", " << spec.name << "; a park of " << w * h << " m2.\n\n";
    o << "blocking = []\n\n[map]\nname = \"" << p.address << "\"\nwidth = " << w << "\nheight = " << h << "\nbuild_quality = 50\n\n";
    o << "[[level]]\nz = 0\ngrid = '''\n";
    for (int gy = 0; gy <= 2 * h; ++gy) {
      for (int gx = 0; gx <= 2 * w; ++gx) {
        if (gy % 2 == 0) o << (gx % 2 == 0 ? '+' : ' ');
        else o << (gx % 2 == 0 ? ' ' : (gy / 2 == 0 ? '_' : '"'));
      }
      o << "\n";
    }
    o << "'''\n";
    for (const auto& [kind, at] : objects) o << "\n[[object]]\nkind = \"" << kind << "\"\nat = [0, " << at.first << ", " << at.second << "]\n";
    return o.str();
  }
  std::ostringstream header;
  header << "# Generated by zs gen plot (spec 72): " << p.address << ", " << spec.name << "; " << (plan.kind == "apartments" ? "an apartment house of " + std::to_string(p.units) + " flats" : "a " + plan.kind)
         << ", " << plan.homes.size() << (plan.homes.size() == 1 ? " household, " : " households, ") << plan.building.floor_area() << " m2"
         << (plan.building.floors > 1 ? " on " + std::to_string(plan.building.floors) + " floors" : "") << " on a plot of "
         << plan.frontage_m << " x " << plan.depth_m << " m.";
  return building_toml(plan.building, dr, seed, header.str(), "plot", p.address);
}

}  // namespace zs::gen
