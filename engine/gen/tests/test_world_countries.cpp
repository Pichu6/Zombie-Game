// Spec 52 slice 4: countries formed from the land.
#include <doctest.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <vector>

#include "zs/gen/keyed.h"
#include "zs/gen/place.h"
#include "zs/gen/world_countries.h"

using namespace zs::gen;

namespace {

struct Planet {
  WorldGrid grid;
  WorldFields fields;
  WorldLand land;
  LandGraph graph;
};

const Planet& planet(std::uint64_t seed) {
  static std::vector<std::pair<std::uint64_t, Planet>> cache;
  for (const auto& [s, p] : cache)
    if (s == seed) return p;
  Planet p{WorldGrid(kOneWorldFrequency, 6371.0), {}, {}, {}};
  WorldParams wp;
  wp.seed = seed;
  p.fields = generate_fields(p.grid, wp);
  p.land = generate_land(p.grid, p.fields);
  p.graph = build_land_graph(p.grid, p.fields);
  cache.emplace_back(seed, std::move(p));
  return cache.back().second;
}

struct Sizes {
  std::size_t count = 0, two_cells = 0, million = 0;
  double top10_share = 0, river_edges = 0;
};

Sizes sizes(const Planet& p, const WorldCountries& w) {
  Sizes s;
  s.count = w.countries.size();
  double cell = 0, land = 0;
  std::size_t cells = 0;
  std::vector<double> area;
  for (const CountryShape& c : w.countries) {
    area.push_back(c.area_km2);
    land += c.area_km2;
    cells += c.cells.size();
  }
  cell = land / static_cast<double>(std::max<std::size_t>(1, cells));
  for (double a : area) {
    s.two_cells += a >= 1.5 * cell;
    s.million += a >= 1e6;
  }
  std::sort(area.begin(), area.end(), std::greater<>());
  double top = 0;
  for (std::size_t i = 0; i < std::min<std::size_t>(10, area.size()); ++i) top += area[i];
  s.top10_share = top / land;
  double edges = 0, river = 0;
  for (CellId c = 0; c < p.grid.size(); ++c)
    for (CellId nb : p.grid.neighbours(c))
      if (c < nb && p.fields.land(c) && p.fields.land(nb) && w.country_of[c] != w.country_of[nb]) {
        ++edges;
        river += p.land.river[c] > 0 || p.land.river[nb] > 0;
      }
  s.river_edges = edges > 0 ? river / edges : 0;
  return s;
}

}  // namespace

TEST_CASE("countries: every land cell in one country, each country connected (A1)") {
  const Planet& p = planet(1);
  const WorldCountries w = form_countries(p.grid, p.fields, p.land, p.graph, 1);
  std::size_t in = 0, land = 0;
  for (CellId c = 0; c < p.grid.size(); ++c) {
    const bool held = p.fields.land(c) && p.fields.biome[c] != Biome::Ice;  // ice is no one's (rule 24)
    CHECK((w.country_of[c] >= 0) == held);
    land += held;
  }
  for (std::size_t k = 0; k < w.countries.size(); ++k) {
    const CountryShape& s = w.countries[k];
    REQUIRE(!s.cells.empty());
    in += s.cells.size();
    CHECK(w.country_of[s.capital] == static_cast<int>(k));
    // Connected over its own land links and one-sea-cell crossings.
    std::vector<CellId> stack{s.cells.front()};
    std::vector<char> seen(p.grid.size(), 0);
    seen[s.cells.front()] = 1;
    std::size_t reached = 0;
    while (!stack.empty()) {
      const CellId x = stack.back();
      stack.pop_back();
      ++reached;
      for (const LandLink& l : p.graph.links[x])
        if (w.country_of[l.to] == static_cast<int>(k) && !seen[l.to]) { seen[l.to] = 1; stack.push_back(l.to); }
    }
    CHECK(reached == s.cells.size());
  }
  CHECK(in == land);
}

TEST_CASE("countries: sizes near Earth's on the default planet; other seeds reported (A2)") {
  for (std::uint64_t seed : {1ull, 2ull, 3ull}) {
    const Planet& p = planet(seed);
    const WorldCountries w = form_countries(p.grid, p.fields, p.land, p.graph, seed);
    const Sizes s = sizes(p, w);
    std::size_t annex = 0, secede = 0, split = 0, fill = 0;
    for (const HistoryEvent& e : w.events) {
      annex += e.kind == HistoryKind::Annex;
      secede += e.kind == HistoryKind::Secede;
      split += e.kind == HistoryKind::Split;
      fill += e.kind == HistoryKind::Fill;
    }
    MESSAGE("seed " << seed << ": " << s.count << " countries (Earth 110 of one cell), " << s.two_cells
                    << " of two cells (Earth 88), " << s.million << " over 1 million km2 (Earth 29); top 10 hold "
                    << s.top10_share << " of land (Earth about 0.5); border edges touching a river " << s.river_edges
                    << "; start polities " << w.start_polities << "; annex " << annex << ", secede " << secede
                    << ", split " << split << ", fill " << fill);
    if (seed != 1) continue;
    CHECK(s.count >= 60);
    CHECK(s.count <= 160);
    const double want_two = 88.0 / 110.0 * static_cast<double>(s.count);
    CHECK(static_cast<double>(s.two_cells) >= 0.5 * want_two);
    CHECK(static_cast<double>(s.two_cells) <= 1.5 * want_two);
    CHECK(s.top10_share >= 0.35);
    CHECK(s.top10_share <= 0.65);
  }
}

TEST_CASE("countries: more fragmentation gives more countries (A1)") {
  const Planet& p = planet(1);
  CountryParams lo, hi;
  lo.fragmentation = 0.1;
  hi.fragmentation = 0.9;
  const std::size_t a = form_countries(p.grid, p.fields, p.land, p.graph, 1, lo).countries.size();
  const std::size_t b = form_countries(p.grid, p.fields, p.land, p.graph, 1, hi).countries.size();
  MESSAGE("fragmentation 0.1: " << a << " countries; 0.9: " << b);
  CHECK(b > a);
}

TEST_CASE("countries: a desert river cell is a better core than the desert beside it (A1)") {
  const Planet& p = planet(1);
  const WorldCountries w = form_countries(p.grid, p.fields, p.land, p.graph, 1);
  int checked = 0;
  for (CellId c = 0; c < p.grid.size(); ++c) {
    if (p.fields.biome[c] != Biome::Desert || p.land.river[c] == 0) continue;
    for (CellId nb : p.grid.neighbours(c))
      if (p.fields.biome[nb] == Biome::Desert && p.land.river[nb] == 0) {
        CHECK(w.core[c] > w.core[nb]);
        ++checked;
      }
  }
  MESSAGE(checked << " desert river cells against their desert neighbours");
}

TEST_CASE("countries: deterministic, golden pinned (A1)") {
  const Planet& p = planet(1);
  const WorldCountries a = form_countries(p.grid, p.fields, p.land, p.graph, 1);
  const WorldCountries b = form_countries(p.grid, p.fields, p.land, p.graph, 1);
  CHECK(a.country_of == b.country_of);
  CHECK(a.events.size() == b.events.size());
  std::uint64_t h = 0xCBF29CE484222325ull;
  for (CellId c = 0; c < p.grid.size(); ++c) h = mix64(h, static_cast<std::uint64_t>(a.country_of[c] + 2));
  h = mix64(h, a.events.size());
  MESSAGE("countries hash: " << h << ", " << a.countries.size() << " countries, " << a.events.size() << " events");
  CHECK(h == 4412980911924084775ULL);  // re-pinned on MSVC, 2026-10-09 (spec 31 terrain revised)
}

TEST_CASE("countries: calibration sweep (probe)" * doctest::skip()) {
  for (std::uint64_t seed : {1ull, 2ull, 3ull})
    for (double beta : {3e-4, 6e-4, 1e-3, 2e-3})
      for (int periods : {40, 60, 100}) {
        const Planet& p = planet(seed);
        CountryParams cp;
        cp.beta0 = beta;
        cp.periods = periods;
        const WorldCountries w = form_countries(p.grid, p.fields, p.land, p.graph, seed, cp);
        const Sizes s = sizes(p, w);
        MESSAGE("PROBE seed " << seed << " beta " << beta << " T " << periods << ": " << s.count << " / " << s.two_cells
                              << " / " << s.million << " top10 " << s.top10_share);
      }
}
