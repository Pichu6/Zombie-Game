// Countries formed from the land. Spec: docs/tier0/specs/52-land-water-capacity.md, slice 4 (rules 19-24).
#include "zs/gen/world_countries.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <queue>
#include <tuple>

#include "fields.h"
#include "zs/detmath.h"
#include "zs/gen/keyed.h"

namespace zs::gen {

namespace {

constexpr int kSea = -2, kUnruled = -1;
constexpr double kCrossingCost = 4.0;     // rule 24 (spec 48 rule 3)
constexpr double kClimbPerM = 1.0 / 500;  // rule 24 (spec 48 rule 3)

double biome_factor(Biome b) {  // rule 24 (CF-46)
  switch (b) {
    case Biome::Desert: return 3;
    case Biome::Tundra:
    case Biome::Ice: return 5;
    case Biome::TropicalForest: return 2;
    default: return 1;
  }
}

double percentile90(std::vector<double> v) {  // of the values above 0; 0 when none
  v.erase(std::remove_if(v.begin(), v.end(), [](double x) { return x <= 0; }), v.end());
  if (v.empty()) return 0;
  std::sort(v.begin(), v.end());
  return v[static_cast<std::size_t>(0.9 * static_cast<double>(v.size() - 1))];
}

struct Polity {
  double Y = 0;
  CellId capital = 0;
  int cells = 0, founded = 0, capital_since = 0;
};

}  // namespace

const char* history_name(HistoryKind k) {
  switch (k) {
    case HistoryKind::Annex: return "annex";
    case HistoryKind::Secede: return "secede";
    case HistoryKind::Split: return "split";
    case HistoryKind::Fill: return "fill";
  }
  return "?";
}

LandGraph build_land_graph(const WorldGrid& g, const WorldFields& f) {
  LandGraph lg;
  lg.links.resize(g.size());
  for (CellId c = 0; c < g.size(); ++c) {
    if (!f.land(c)) continue;
    auto& out = lg.links[c];
    for (CellId nb : g.neighbours(c))
      if (f.land(nb)) out.push_back({nb, false});
    auto has = [&](CellId t) {
      for (const LandLink& l : out)
        if (l.to == t) return true;
      return false;
    };
    for (CellId s : g.neighbours(c)) {
      if (f.land(s)) continue;
      for (CellId t : g.neighbours(s))
        if (t != c && f.land(t) && !has(t)) out.push_back({t, true});
    }
  }
  return lg;
}

WorldCountries form_countries(const WorldGrid& g, const WorldFields& f, const WorldLand& land, const LandGraph& lg,
                              std::uint64_t seed, const CountryParams& p) {
  const std::size_t n = g.size();
  WorldCountries w;
  w.core.assign(n, 0.0);
  w.terrain.assign(n, 0.0);
  w.country_of.assign(n, -1);
  w.periods = p.periods;

  // Rule 19: core values.
  for (CellId c = 0; c < n; ++c) {
    if (!f.land(c) || f.biome[c] == Biome::Ice) continue;
    const double cap = land.capacity(c);
    if (cap <= 0 || land.capacity_food[c] <= 0) continue;
    const double a = land.arable_share[c], pa = land.pasture_share[c];
    const double cereal = a + pa > 0 ? a / (a + pa) : 0.0;
    const double river = land.river[c] == 2 ? 1.5 : land.river[c] == 1 ? 1.2 : 1.0;
    const double coast = f.sea_cells[c] <= 1 ? 1.2 : 1.0;
    std::vector<CellId> ring;
    for (CellId a1 : g.neighbours(c)) {
      ring.push_back(a1);
      for (CellId a2 : g.neighbours(a1))
        if (a2 != c) ring.push_back(a2);
    }
    std::sort(ring.begin(), ring.end());
    ring.erase(std::unique(ring.begin(), ring.end()), ring.end());
    int low = 0;
    for (CellId x : ring) low += (f.land(x) ? land.capacity(x) : 0.0) < 0.1 * cap;
    const double circ = ring.empty() ? 0.0 : static_cast<double>(low) / static_cast<double>(ring.size());
    w.core[c] = land.capacity_food[c] * cereal * river * coast * (1.0 + circ);
  }

  // Rule 20: terrain.
  std::vector<double> rug(n, 0.0), heat(n, 0.0), cold(n, 0.0);
  std::vector<double> rug_l, heat_l, cold_l;
  for (CellId c = 0; c < n; ++c) {
    if (!f.land(c)) continue;
    double s = 0;
    int k = 0;
    for (CellId nb : g.neighbours(c))
      if (f.land(nb)) s += std::abs(f.elevation_m[c] - f.elevation_m[nb]), ++k;
    rug[c] = k ? s / k : 0.0;
    heat[c] = std::max(0.0, f.temp_mean_c[c] - 24.0);
    cold[c] = std::max(0.0, 2.0 - f.temp_mean_c[c]);
    rug_l.push_back(rug[c]);
    heat_l.push_back(heat[c]);
    cold_l.push_back(cold[c]);
  }
  const double r90 = percentile90(rug_l), h90 = percentile90(heat_l), k90 = percentile90(cold_l);
  const double shift = 2.0 * (p.fragmentation - 0.5);
  const double theta = 2.0 * detmath::pow(2.0, shift);
  const double beta = p.beta0 * detmath::pow(10.0, shift);
  for (CellId c = 0; c < n; ++c) {
    if (!f.land(c)) continue;
    double x = 0;
    if (r90 > 0) x = std::max(x, rug[c] / r90);
    if (h90 > 0) x = std::max(x, heat[c] / h90);
    if (k90 > 0) x = std::max(x, cold[c] / k90);
    w.terrain[c] = theta * x;
  }

  // Rule 21: the start.
  std::vector<int> owner(n, kSea);
  std::vector<Polity> pol;
  double ymax = 0;
  for (CellId c = 0; c < n; ++c) {
    if (!f.land(c) || f.biome[c] == Biome::Ice) continue;  // ice belongs to no one, as Antarctica (rule 24)
    owner[c] = kUnruled;
    ymax = std::max(ymax, w.core[c]);
    if (w.core[c] > 0 && land.capacity(c) / g.area_km2(c) >= p.state_density) {
      owner[c] = static_cast<int>(pol.size());
      pol.push_back({w.core[c], c, 1, 0, 0});
    }
  }
  w.start_polities = static_cast<int>(pol.size());
  const double alpha = ymax > 0 ? 1.0 / ymax : 0.0;
  const Keyed r(seed);

  auto move = [&](CellId c, int to) {
    const int from = owner[c];
    if (from >= 0) pol[static_cast<std::size_t>(from)].Y -= w.core[c], --pol[static_cast<std::size_t>(from)].cells;
    owner[c] = to;
    pol[static_cast<std::size_t>(to)].Y += w.core[c];
    ++pol[static_cast<std::size_t>(to)].cells;
  };

  // Rule 22: the history.
  std::vector<double> weight;
  std::vector<CellId> cand;
  std::vector<bool> cand_sea;
  for (int period = 0; period < p.periods; ++period) {
    std::vector<char> dirty(pol.size(), 0);
    for (CellId c = 0; c < n; ++c) {
      const int i = owner[c];
      if (i < 0) continue;
      weight.clear();
      cand.clear();
      cand_sea.clear();
      for (const LandLink& l : lg.links[c]) {
        if (owner[l.to] == i || owner[l.to] == kSea) continue;
        cand.push_back(l.to);
        cand_sea.push_back(l.sea);
        weight.push_back(w.core[l.to] * (l.sea ? p.sea_weight : 1.0));
      }
      double total = 0;
      for (double x : weight) total += x;
      if (total <= 0) continue;
      if (!r.chance(c, field::Conflict, alpha * w.core[c], static_cast<std::uint32_t>(period))) continue;
      const std::size_t k = r.weighted(c, field::Opponent, weight, static_cast<std::uint32_t>(period));
      const CellId d = cand[k];
      const int j = owner[d];
      const double yi = pol[static_cast<std::size_t>(i)].Y * (cand_sea[k] ? p.sea_sigma : 1.0);
      const double yj = j >= 0 ? pol[static_cast<std::size_t>(j)].Y : w.core[d];
      const double win = yi / (yi + yj) / (1.0 + w.terrain[d]);
      if (!r.chance(c, field::Win, win, static_cast<std::uint32_t>(period))) continue;
      move(d, i);
      if (j >= 0) dirty[static_cast<std::size_t>(j)] = 1;
      w.events.push_back({period, HistoryKind::Annex, d, i, j});
    }
    // Secession (CF-27): border cells, counted once per polity at this point.
    std::vector<int> border(pol.size(), 0);
    std::vector<char> is_border(n, 0);
    for (CellId c = 0; c < n; ++c) {
      if (owner[c] < 0) continue;
      for (const LandLink& l : lg.links[c])
        if (owner[l.to] != owner[c] && owner[l.to] != kSea) { is_border[c] = 1; break; }
      border[static_cast<std::size_t>(owner[c])] += is_border[c];
    }
    for (CellId c = 0; c < n; ++c) {
      const int i = owner[c];
      if (i < 0 || !is_border[c] || pol[static_cast<std::size_t>(i)].cells <= 1) continue;
      const double prob = beta * w.terrain[c] * border[static_cast<std::size_t>(i)];
      if (!r.chance(c, field::Secede, prob, static_cast<std::uint32_t>(period))) continue;
      const int ni = static_cast<int>(pol.size());
      pol.push_back({0, c, 0, period + 1, period + 1});
      dirty.push_back(0);
      move(c, ni);
      dirty[static_cast<std::size_t>(i)] = 1;
      w.events.push_back({period, HistoryKind::Secede, c, ni, i});
    }
    // Splits and lost capitals.
    std::vector<std::vector<CellId>> cells_of(pol.size());
    bool any = false;
    for (std::size_t i = 0; i < pol.size(); ++i) any = any || dirty[i];
    if (!any) continue;
    for (CellId c = 0; c < n; ++c)
      if (owner[c] >= 0 && dirty[static_cast<std::size_t>(owner[c])]) cells_of[static_cast<std::size_t>(owner[c])].push_back(c);
    const std::size_t before = pol.size();
    for (std::size_t i = 0; i < before; ++i) {
      if (!dirty[i] || cells_of[i].empty()) continue;
      // Components over the polity's own links.
      std::vector<std::vector<CellId>> parts;
      std::vector<char> seen_local;
      std::vector<CellId>& mine = cells_of[i];
      auto index_of = [&](CellId c) { return static_cast<std::size_t>(std::lower_bound(mine.begin(), mine.end(), c) - mine.begin()); };
      seen_local.assign(mine.size(), 0);
      for (std::size_t s = 0; s < mine.size(); ++s) {
        if (seen_local[s]) continue;
        parts.emplace_back();
        std::vector<CellId> stack{mine[s]};
        seen_local[s] = 1;
        while (!stack.empty()) {
          const CellId x = stack.back();
          stack.pop_back();
          parts.back().push_back(x);
          for (const LandLink& l : lg.links[x]) {
            if (owner[l.to] != static_cast<int>(i)) continue;
            const std::size_t li = index_of(l.to);
            if (!seen_local[li]) { seen_local[li] = 1; stack.push_back(l.to); }
          }
        }
        std::sort(parts.back().begin(), parts.back().end());
      }
      auto best_cell = [&](const std::vector<CellId>& cs) {
        CellId b = cs.front();
        for (CellId c : cs)
          if (w.core[c] > w.core[b]) b = c;
        return b;
      };
      Polity& P = pol[i];
      std::size_t keep = parts.size();
      for (std::size_t k = 0; k < parts.size(); ++k)
        if (std::binary_search(parts[k].begin(), parts[k].end(), P.capital) && owner[P.capital] == static_cast<int>(i)) keep = k;
      if (keep == parts.size()) {  // the capital was lost: the part with the best cell keeps the polity
        keep = 0;
        for (std::size_t k = 1; k < parts.size(); ++k)
          if (w.core[best_cell(parts[k])] > w.core[best_cell(parts[keep])]) keep = k;
        pol[i].capital = best_cell(parts[keep]);
        pol[i].capital_since = period + 1;
      }
      for (std::size_t k = 0; k < parts.size(); ++k) {
        if (k == keep) continue;
        const int ni = static_cast<int>(pol.size());
        const CellId cap = best_cell(parts[k]);
        pol.push_back({0, cap, 0, period + 1, period + 1});
        for (CellId c : parts[k]) move(c, ni);
        w.events.push_back({period, HistoryKind::Split, cap, ni, static_cast<int>(i)});
      }
    }
  }

  // Countries from the polities that hold land, in polity order.
  std::vector<int> country_of_polity(pol.size(), -1);
  for (CellId c = 0; c < n; ++c) {
    const int i = owner[c];
    if (i < 0) continue;
    if (country_of_polity[static_cast<std::size_t>(i)] < 0) country_of_polity[static_cast<std::size_t>(i)] = 0;
  }
  {
    int next = 0;
    for (std::size_t i = 0; i < pol.size(); ++i)
      if (country_of_polity[i] == 0 || country_of_polity[i] > 0) country_of_polity[i] = next++;
    w.countries.resize(static_cast<std::size_t>(next));
    for (std::size_t i = 0; i < pol.size(); ++i) {
      if (country_of_polity[i] < 0) continue;
      CountryShape& s = w.countries[static_cast<std::size_t>(country_of_polity[i])];
      s.polity = static_cast<int>(i);
      s.capital = pol[i].capital;
      s.founded = pol[i].founded;
      s.capital_held = p.periods - pol[i].capital_since;
    }
  }
  for (CellId c = 0; c < n; ++c)
    if (owner[c] >= 0) w.country_of[c] = country_of_polity[static_cast<std::size_t>(owner[c])];

  // Rule 24: the present-day fill.
  {
    std::vector<double> best(n, 1e300);
    using Item = std::tuple<double, int, CellId>;
    std::priority_queue<Item, std::vector<Item>, std::greater<>> q;
    for (CellId c = 0; c < n; ++c)
      if (w.country_of[c] >= 0) { best[c] = 0; q.emplace(0.0, w.country_of[c], c); }
    std::vector<int> reach(w.country_of);
    while (!q.empty()) {
      const auto [d, k, c] = q.top();
      q.pop();
      if (d > best[c] || k != reach[c]) continue;
      for (const LandLink& l : lg.links[c]) {
        if (w.country_of[l.to] >= 0 || owner[l.to] == kSea) continue;  // held land stays with its holder; ice is no one's
        const double dist = g.distance_km(c, l.to);
        const double step = (l.sea ? kCrossingCost * dist : dist * (1.0 + std::abs(f.elevation_m[c] - f.elevation_m[l.to]) * kClimbPerM)) *
                            biome_factor(f.biome[l.to]);
        const double nd = d + step;
        if (nd < best[l.to] || (nd == best[l.to] && k < reach[l.to])) {
          best[l.to] = nd;
          reach[l.to] = k;
          q.emplace(nd, k, l.to);
        }
      }
    }
    std::vector<CellId> left;
    for (CellId c = 0; c < n; ++c) {
      if (owner[c] == kSea || w.country_of[c] >= 0) continue;
      if (reach[c] >= 0 && best[c] <= p.fill_limit) {
        w.country_of[c] = reach[c];
        w.events.push_back({p.periods, HistoryKind::Fill, c, w.countries[static_cast<std::size_t>(reach[c])].polity, kUnruled});
      } else {
        left.push_back(c);
      }
    }
    // Groups of land no country reaches become countries of their own.
    for (CellId s : left) {
      if (w.country_of[s] >= 0) continue;
      const int k = static_cast<int>(w.countries.size());
      w.countries.emplace_back();
      w.countries.back().founded = p.periods;
      std::vector<CellId> stack{s};
      w.country_of[s] = k;
      CellId cap = s;
      while (!stack.empty()) {
        const CellId x = stack.back();
        stack.pop_back();
        if (w.core[x] > w.core[cap]) cap = x;
        for (const LandLink& l : lg.links[x])
          if (w.country_of[l.to] < 0 && std::binary_search(left.begin(), left.end(), l.to)) {
            w.country_of[l.to] = k;
            stack.push_back(l.to);
          }
      }
      w.countries.back().capital = cap;
    }
  }

  // Rule 27: what each country records.
  for (CellId c = 0; c < n; ++c) {
    const int k = w.country_of[c];
    if (k < 0) continue;
    CountryShape& s = w.countries[static_cast<std::size_t>(k)];
    s.cells.push_back(c);
    s.area_km2 += g.area_km2(c);
    s.core += w.core[c];
    if (f.sea_cells[c] <= 1) s.coast = true;
  }
  for (CountryShape& s : w.countries) s.landlocked = !s.coast;
  return w;
}

}  // namespace zs::gen
