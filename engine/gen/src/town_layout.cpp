// Spec 71: a town's streets, blocks and plots. docs/tier0/specs/71-streets-and-plots.md (research city-layout-generation)
#include "zs/gen/town_layout.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <sstream>
#include <stdexcept>

#include "toml.hpp"
#include "zs/gen/keyed.h"
#include "zs/gen/names.h"

namespace zs::gen {
namespace {

enum Field : std::uint32_t { kStreetName = 1, kPark = 2, kTie = 3 };

const UrbanGroup& group_of(const UrbanForm& f, const std::string& income) {
  const auto it = f.groups.find(income);
  if (it != f.groups.end()) return it->second;
  const auto fallback = f.groups.find("upper_middle");
  if (fallback == f.groups.end()) throw std::runtime_error("urban_form.toml: no group for " + income);
  return fallback->second;
}

// A raw plot before numbering: which side of its block it is on (0: the north or west row, 1: the south or east row).
struct Raw {
  Plot plot;
  int side = 0;
  int order = 0;  // its position along the street within the block side
  bool merged_away = false;
};

std::int64_t sq(std::int64_t v) { return v * v; }

}  // namespace

const char* land_use_name(LandUse u) {
  switch (u) {
    case LandUse::Residential: return "residential";
    case LandUse::Commercial: return "commercial";
    case LandUse::Industrial: return "industrial";
    case LandUse::Park: return "park";
    case LandUse::Civic: return "civic";
    default: return "?";
  }
}

UrbanForm load_urban_form(const std::string& gen_dir) {
  const std::string path = gen_dir + "/urban_form.toml";
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  UrbanForm f;
  f.street_share = t["street_share"].value_or(f.street_share);
  f.open_share = t["open_share"].value_or(f.open_share);
  f.commercial_share = t["commercial_share"].value_or(f.commercial_share);
  f.industrial_share = t["industrial_share"].value_or(f.industrial_share);
  f.vacancy = t["vacancy"].value_or(f.vacancy);
  f.arterial_every = t["arterial_every"].value_or(f.arterial_every);
  f.arterial_width_factor = t["arterial_width_factor"].value_or(f.arterial_width_factor);
  f.max_units_per_plot = t["max_units_per_plot"].value_or(f.max_units_per_plot);
  f.commercial_frontage_m = t["commercial_frontage_m"].value_or(f.commercial_frontage_m);
  f.industrial_frontage_m = t["industrial_frontage_m"].value_or(f.industrial_frontage_m);
  if (const auto* g = t["group"].as_table())
    for (const auto& [k, v] : *g) {
      const toml::node_view<const toml::node> gv{v};
      UrbanGroup u;
      u.density_per_ha = gv["density_per_ha"].value_or(u.density_per_ha);
      u.intersections_per_km2 = gv["intersections_per_km2"].value_or(u.intersections_per_km2);
      u.frontage_m = gv["frontage_m"].value_or(u.frontage_m);
      if (u.density_per_ha <= 0 || u.intersections_per_km2 <= 0 || u.frontage_m < 3) throw std::runtime_error(path + ": a group out of range");
      f.groups[std::string(k.str())] = u;
    }
  if (f.groups.empty() || f.street_share <= 0 || f.street_share >= 0.6 || f.arterial_every < 1 || f.max_units_per_plot < 1)
    throw std::runtime_error(path + ": shares or counts out of range");
  return f;
}

double TownLayout::street_share() const {
  double blocks_m2 = 0;
  for (const Block& b : blocks) blocks_m2 += static_cast<double>(b.w) * b.h;
  const double total = static_cast<double>(width_m) * height_m;
  return total > 0 ? 1.0 - blocks_m2 / total : 0;
}

double TownLayout::intersections_per_km2() const {
  std::size_t v = 0, h = 0;
  for (const Street& s : streets) (s.vertical ? v : h)++;
  const double km2 = static_cast<double>(width_m) * height_m / 1e6;
  return km2 > 0 ? static_cast<double>(v * h) / km2 : 0;
}

double TownLayout::street_km() const {
  double m = 0;
  for (const Street& s : streets) m += s.vertical ? height_m : width_m;
  return m / 1000.0;
}

TownLayout lay_out_town(const Catalog& cat, const UrbanForm& form, const TownSpec& spec, std::uint64_t seed) {
  const Keyed r(mix64(seed, hash_str("town layout")));
  const UrbanGroup& g = group_of(form, spec.income);
  TownLayout t;
  t.households = std::max(0, static_cast<int>(std::lround(spec.people / std::max(1.0, spec.mean_household_size))));
  // Rule 1: the land budget. Built-up area from the people and the group's density (CL-30); the street spacing from the
  // region's intersections per km2 (CL-35, CL-51); the reserve so that streets take their share of the land (CL-23 to CL-26).
  const double area_m2 = std::max(1.0, spec.people) / g.density_per_ha * 10000.0;
  const int cell = std::max(40, static_cast<int>(std::lround(std::sqrt(1e6 / g.intersections_per_km2))));
  // The mean reserve that gives the street share, shared out so that arterials are wider and locals narrower.
  const double mean_reserve = cell * (1.0 - std::sqrt(1.0 - form.street_share));
  const double widen = 1.0 + (form.arterial_width_factor - 1.0) / form.arterial_every;
  const int reserve = std::max(4, static_cast<int>(std::lround(mean_reserve / widen)));
  const int arterial = std::max(reserve, static_cast<int>(std::lround(reserve * form.arterial_width_factor)));
  const int wanted = std::max(1, static_cast<int>(std::lround(area_m2 / (static_cast<double>(cell) * cell))));
  const int kx = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<double>(wanted)))));
  const int ky = std::max(1, (wanted + kx - 1) / kx);
  t.cell_m = cell;
  // Rule 2: a grid of named streets; every arterial_every-th line is an arterial. The town's land runs from the outer
  // edge of its outer streets.
  const int margin = arterial - arterial / 2;
  t.width_m = kx * cell + 2 * margin;
  t.height_m = ky * cell + 2 * margin;
  const Culture& culture = cat.culture(spec.culture);
  std::set<std::string> roots;
  std::uint32_t k = 0;
  auto next_name = [&]() {
    for (; k < 100000; ++k) {
      const std::uint64_t id = mix64(kStreetName, k);
      if (roots.insert(street_root(culture, r, id)).second) return street_name(culture, r, id);
    }
    return std::string("Street ") + std::to_string(k++);
  };
  for (int i = 0; i <= kx; ++i) {
    const bool art = i % form.arterial_every == 0 || i == kx;
    t.streets.push_back({next_name(), true, margin + i * cell, art ? arterial : reserve, art});
  }
  for (int j = 0; j <= ky; ++j) {
    const bool art = j % form.arterial_every == 0 || j == ky;
    t.streets.push_back({next_name(), false, margin + j * cell, art ? arterial : reserve, art});
  }
  const auto vstreet = [&](int i) -> const Street& { return t.streets[static_cast<std::size_t>(i)]; };
  const auto hstreet = [&](int j) -> const Street& { return t.streets[static_cast<std::size_t>(kx + 1 + j)]; };
  const auto far_edge = [](const Street& s) { return s.at_m - s.width_m / 2 + s.width_m; };
  const auto near_edge = [](const Street& s) { return s.at_m - s.width_m / 2; };
  // Rule 3: blocks between the reserves, and their districts by distance from the centre: the nearest are commercial
  // (and civic), the farthest industrial, parks among the rest, the others residential.
  for (int by = 0; by < ky; ++by)
    for (int bx = 0; bx < kx; ++bx) {
      Block b;
      b.bx = bx;
      b.by = by;
      b.x = far_edge(vstreet(bx));
      b.y = far_edge(hstreet(by));
      b.w = near_edge(vstreet(bx + 1)) - b.x;
      b.h = near_edge(hstreet(by + 1)) - b.y;
      t.blocks.push_back(b);
    }
  const int n = static_cast<int>(t.blocks.size());
  std::vector<int> order(static_cast<std::size_t>(n));
  for (int i = 0; i < n; ++i) order[static_cast<std::size_t>(i)] = i;
  const auto centre_d = [&](int i) {  // twice the block distance from the centre, squared
    const Block& b = t.blocks[static_cast<std::size_t>(i)];
    return sq(2 * b.bx + 1 - kx) + sq(2 * b.by + 1 - ky);
  };
  std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
    const auto da = centre_d(a), db = centre_d(b);
    if (da != db) return da < db;
    return r.bits(static_cast<std::uint64_t>(a), kTie) < r.bits(static_cast<std::uint64_t>(b), kTie);
  });
  // A town of one block is all homes; otherwise at least one commercial block and at least one block left for homes.
  const int n_com = n > 1 ? std::min(n - 1, std::max(1, static_cast<int>(std::lround(form.commercial_share * n)))) : 0;
  const int n_ind = std::min(n - n_com, static_cast<int>(std::lround(form.industrial_share * n)));
  for (int i = 0; i < n_com; ++i) t.blocks[static_cast<std::size_t>(order[static_cast<std::size_t>(i)])].use = LandUse::Commercial;
  for (int i = n - n_ind; i < n; ++i) t.blocks[static_cast<std::size_t>(order[static_cast<std::size_t>(i)])].use = LandUse::Industrial;
  {
    std::vector<int> middle(order.begin() + n_com, order.begin() + (n - n_ind));
    std::stable_sort(middle.begin(), middle.end(), [&](int a, int b) {
      return r.bits(static_cast<std::uint64_t>(a), kPark) < r.bits(static_cast<std::uint64_t>(b), kPark);
    });
    const int n_park = std::min(static_cast<int>(middle.size()), static_cast<int>(std::lround(form.open_share * n)));
    for (int i = 0; i < n_park; ++i) t.blocks[static_cast<std::size_t>(middle[static_cast<std::size_t>(i)])].use = LandUse::Park;
  }
  // Rule 4: plots. A block is cut into two rows along its long side, each plot fronting the street on its side, its
  // frontage by land use and income (CL-45 to CL-47); a park is one plot. Plots partition the block exactly.
  std::vector<Raw> raws;
  for (int bi = 0; bi < n; ++bi) {
    const Block& b = t.blocks[static_cast<std::size_t>(bi)];
    const bool along_x = b.w >= b.h;  // the rows run along x and front the horizontal streets
    const int length = along_x ? b.w : b.h, depth = along_x ? b.h : b.w;
    const int street_a = along_x ? kx + 1 + b.by : b.bx, street_b = along_x ? kx + 1 + b.by + 1 : b.bx + 1;
    if (b.use == LandUse::Park) {
      Raw p;
      p.plot = {bi, b.x, b.y, b.w, b.h, LandUse::Park, street_a};
      raws.push_back(p);
      continue;
    }
    const double frontage = b.use == LandUse::Commercial ? form.commercial_frontage_m
                            : b.use == LandUse::Industrial ? form.industrial_frontage_m
                                                           : g.frontage_m;
    const int per_side = std::clamp(static_cast<int>(length / frontage), 1, 49);  // numbers 2k and 2k+1 below 100 per block
    const int rows = depth >= 16 ? 2 : 1;  // a shallow block keeps one row (ESTIMATE: 8 m is the least plot depth)
    for (int side = 0; side < rows; ++side) {
      const int d0 = side == 0 ? 0 : depth / 2, d1 = rows == 1 ? depth : (side == 0 ? depth / 2 : depth);
      int at = 0;
      for (int q = 0; q < per_side; ++q) {
        const int len = length / per_side + (q < length % per_side ? 1 : 0);
        Raw p;
        p.side = side;
        p.order = q;
        p.plot.block = bi;
        p.plot.use = b.use;
        p.plot.street = side == 0 ? street_a : street_b;
        if (along_x) p.plot = {bi, b.x + at, b.y + d0, len, d1 - d0, b.use, p.plot.street};
        else p.plot = {bi, b.x + d0, b.y + at, d1 - d0, len, b.use, p.plot.street};
        raws.push_back(p);
        at += len;
      }
    }
  }
  // Rule 6: a lot that needs its own plot (a police station) takes a run of plots side by side in a block of its district
  // (commercial for a station; spec 72: residential for the Tier 0 block) nearest the centre, wide and deep enough; they
  // become one plot: civic for a commercial lot, residential (holding the lot's own households) for a residential one.
  t.lot_plot.assign(spec.lots.size(), -1);
  std::vector<int> lot_raw(spec.lots.size(), -1);
  for (std::size_t li = 0; li < spec.lots.size(); ++li) {
    const TownSpec::Lot& lot = spec.lots[li];
    for (int oi = 0; oi < n && lot_raw[li] < 0; ++oi) {
      const int bi = order[static_cast<std::size_t>(oi)];
      if (t.blocks[static_cast<std::size_t>(bi)].use != lot.in) continue;
      for (std::size_t s = 0; s < raws.size() && lot_raw[li] < 0; ++s) {
        const Raw& first = raws[s];
        if (first.plot.block != bi || first.merged_away || first.plot.lot >= 0 || first.plot.use != lot.in) continue;
        const bool along_x = t.blocks[static_cast<std::size_t>(bi)].w >= t.blocks[static_cast<std::size_t>(bi)].h;
        const int deep = along_x ? first.plot.h : first.plot.w;
        if (deep < lot.depth_m) continue;
        int wide = 0;
        std::size_t e = s;
        while (e < raws.size() && raws[e].plot.block == bi && raws[e].side == first.side && !raws[e].merged_away &&
               raws[e].plot.lot < 0 && wide < lot.width_m) {
          wide += along_x ? raws[e].plot.w : raws[e].plot.h;
          ++e;
        }
        if (wide < lot.width_m) continue;
        Raw& keep = raws[s];
        for (std::size_t m = s + 1; m < e; ++m) {
          raws[m].merged_away = true;
          if (along_x) keep.plot.w += raws[m].plot.w;
          else keep.plot.h += raws[m].plot.h;
        }
        keep.plot.use = lot.in == LandUse::Commercial ? LandUse::Civic : lot.in;
        keep.plot.lot = static_cast<int>(li);
        lot_raw[li] = static_cast<int>(s);
      }
    }
  }
  // Rule 5: numbers and addresses. Along each street a block's plots take the hundreds of its place along the street, even
  // numbers on one side and odd on the other, so every address is unique.
  std::vector<int> raw_to_plot(raws.size(), -1);
  for (std::size_t s = 0; s < raws.size(); ++s) {
    if (raws[s].merged_away) continue;
    Plot p = raws[s].plot;
    const Block& b = t.blocks[static_cast<std::size_t>(p.block)];
    const Street& st = t.streets[static_cast<std::size_t>(p.street)];
    const int along = st.vertical ? b.by : b.bx;
    p.number = 100 * (along + 1) + 2 * raws[s].order + (raws[s].side == 0 ? 0 : 1) + (p.use == LandUse::Park ? 0 : 1);
    p.address = std::to_string(p.number) + " " + st.name;
    raw_to_plot[s] = static_cast<int>(t.plots.size());
    t.plots.push_back(std::move(p));
  }
  for (std::size_t li = 0; li < lot_raw.size(); ++li)
    if (lot_raw[li] >= 0) t.lot_plot[li] = raw_to_plot[static_cast<std::size_t>(lot_raw[li])];
  // Rule 7: dwelling units. Every residential plot is a house; where houses cannot hold the households and the vacancy,
  // the plots nearest the centre become apartment houses, up to max_units_per_plot each. Households fill the units from
  // the centre out.
  std::vector<int> res;
  int on_lots = 0;  // spec 72: households living on a residential lot (the Tier 0 block) are housed there
  for (int i = 0; i < static_cast<int>(t.plots.size()); ++i) {
    Plot& p = t.plots[static_cast<std::size_t>(i)];
    if (p.use != LandUse::Residential) continue;
    if (p.lot >= 0) {
      p.units = p.households = std::min(spec.lots[static_cast<std::size_t>(p.lot)].households, t.households - on_lots);
      on_lots += p.households;
      continue;
    }
    res.push_back(i);
  }
  const auto plot_d = [&](int i) {
    const Plot& p = t.plots[static_cast<std::size_t>(i)];
    return sq(2 * p.x + p.w - t.width_m) + sq(2 * p.y + p.h - t.height_m);
  };
  std::stable_sort(res.begin(), res.end(), [&](int a, int b) { return plot_d(a) < plot_d(b); });
  const int want_units = static_cast<int>(std::ceil((t.households - on_lots) * (1.0 + form.vacancy)));
  for (int i : res) t.plots[static_cast<std::size_t>(i)].units = 1;
  int extra = want_units - static_cast<int>(res.size());
  for (int i : res) {
    if (extra <= 0) break;
    const int add = std::min(extra, form.max_units_per_plot - 1);
    t.plots[static_cast<std::size_t>(i)].units += add;
    extra -= add;
  }
  // Still short: density rises, the counts never fall (CL pipeline step 1): the plots nearest the centre take one more
  // unit each in turn (taller buildings) until every household and the vacancy fit.
  for (std::size_t q = 0; extra > 0 && !res.empty(); q = (q + 1) % res.size()) {
    ++t.plots[static_cast<std::size_t>(res[q])].units;
    --extra;
  }
  int left = t.households - on_lots;
  for (int i : res) {
    Plot& p = t.plots[static_cast<std::size_t>(i)];
    const int take = std::min(left, p.units);
    p.households = take;
    left -= take;
  }
  return t;
}

std::string layout_summary(const TownSpec& spec, const TownLayout& t) {
  int by_use[5] = {}, plots_by_use[5] = {}, units = 0, placed = 0, apartments = 0;
  for (const Block& b : t.blocks) ++by_use[static_cast<int>(b.use)];
  for (const Plot& p : t.plots) {
    ++plots_by_use[static_cast<int>(p.use)];
    units += p.units;
    placed += p.households;
    apartments += p.units > 1 ? 1 : 0;
  }
  std::ostringstream o;
  o.setf(std::ios::fixed);
  o.precision(1);
  o << spec.name << ": " << spec.people << " people, " << t.households << " households (" << spec.income << " income)\n";
  o << "  built-up " << t.built_up_ha() << " ha (" << spec.people / std::max(0.1, t.built_up_ha()) << " people per ha); streets "
    << t.street_km() << " km, " << 100.0 * t.street_share() << " % of the land; " << t.intersections_per_km2() << " intersections per km2; spacing "
    << t.cell_m << " m\n";
  o << "  " << t.blocks.size() << " blocks: " << by_use[0] << " residential, " << by_use[1] << " commercial, " << by_use[2] << " industrial, "
    << by_use[3] << " parks\n";
  o << "  " << t.plots.size() << " plots: " << plots_by_use[0] << " residential (" << apartments << " apartment houses), " << plots_by_use[1]
    << " commercial, " << plots_by_use[2] << " industrial, " << plots_by_use[3] << " parks, " << plots_by_use[4] << " civic\n";
  o << "  " << units << " dwelling units, " << placed << " households placed (" << t.households - placed << " without a home), "
    << 100.0 * (units - placed) / std::max(1, units) << " % vacant\n";
  for (std::size_t li = 0; li < spec.lots.size(); ++li) {
    const int pi = t.lot_plot[li];
    o << "  " << spec.lots[li].name << ": " << (pi >= 0 ? t.plots[static_cast<std::size_t>(pi)].address : std::string("no plot found")) << "\n";
  }
  return o.str();
}

}  // namespace zs::gen
