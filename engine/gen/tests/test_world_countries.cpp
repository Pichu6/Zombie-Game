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
#include "zs/world.h"

using namespace zs::gen;

namespace {

struct Planet {
  WorldGrid grid;
  WorldFields fields;
  WorldLand land;
  LandGraph graph;
};

// The rule tests (A1) run on the coarse test planet; the size checks against Earth on the one world's grid.
const Planet& planet(std::uint64_t seed, int frequency = kTestFrequency) {
  static std::vector<std::pair<std::uint64_t, Planet>> cache;
  const std::uint64_t key = seed * 1000 + static_cast<std::uint64_t>(frequency);
  for (const auto& [k, p] : cache)
    if (k == key) return p;
  Planet p{WorldGrid(frequency, 6371.0), {}, {}, {}};
  WorldParams wp;
  wp.seed = seed;
  p.fields = generate_fields(p.grid, wp);
  p.land = generate_land(p.grid, p.fields);
  p.graph = build_land_graph(p.grid, p.fields);
  cache.emplace_back(key, std::move(p));
  return cache.back().second;
}

// Sizes by area, so they read the same on any grid (CF-57: UN members of at least 89,000, 180,000 and 1 million km2).
struct Sizes {
  std::size_t count = 0, over_89k = 0, over_180k = 0, over_1m = 0;
  std::size_t small_start = 0, small_later = 0, small_fill = 0, small_cells = 0;  // under 89,000 km2, by origin
  double top10_share = 0, river_edges = 0;
};

Sizes sizes(const Planet& p, const WorldCountries& w) {
  Sizes s;
  s.count = w.countries.size();
  double land = 0;
  std::vector<double> area;
  for (const CountryShape& c : w.countries) {
    area.push_back(c.area_km2);
    land += c.area_km2;
    s.over_89k += c.area_km2 >= 89'000;
    s.over_180k += c.area_km2 >= 180'000;
    s.over_1m += c.area_km2 >= 1e6;
    if (c.area_km2 < 89'000) {
      s.small_cells += c.cells.size();
      if (c.polity < 0) ++s.small_fill;
      else if (c.founded == 0) ++s.small_start;
      else ++s.small_later;
    }
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

TEST_CASE("countries: sizes near Earth's on the one world's grid; other seeds reported (A2)" * doctest::skip(zs::World::kDebugBuild)) {
  for (std::uint64_t seed : {1ull, 2ull, 3ull}) {
    const Planet& p = planet(seed, kOneWorldFrequency);
    const WorldCountries w = form_countries(p.grid, p.fields, p.land, p.graph, seed);
    const Sizes s = sizes(p, w);
    std::size_t annex = 0, secede = 0, split = 0, fill = 0;
    for (const HistoryEvent& e : w.events) {
      annex += e.kind == HistoryKind::Annex;
      secede += e.kind == HistoryKind::Secede;
      split += e.kind == HistoryKind::Split;
      fill += e.kind == HistoryKind::Fill;
    }
    MESSAGE("seed " << seed << ": " << s.count << " countries; " << s.over_89k << " of 89,000 km2 or more (Earth 110), "
                    << s.over_180k << " of 180,000 (88), " << s.over_1m << " of 1 million (29); top 10 hold " << s.top10_share
                    << " of land (about 0.5); border edges touching a river " << s.river_edges << "; start polities "
                    << w.start_polities << ", " << w.periods << " rounds; annex " << annex << ", secede " << secede
                    << ", split " << split << ", fill " << fill);
    if (seed != 1) continue;
    CHECK(s.over_89k >= 70);
    CHECK(s.over_89k <= 160);
    CHECK(s.over_1m >= 15);
    CHECK(s.over_1m <= 45);
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
  CHECK(h == 6845947420822028453ULL);  // re-pinned on MSVC, 2026-10-15 (the hydrology fix: wetter tropics, channel losses)
}

TEST_CASE("countries: calibration sweep on the one world's grid (probe)" * doctest::skip()) {
  for (std::uint64_t seed : {1ull, 2ull, 3ull})
    for (double beta : {3e-4, 1e-3, 3e-3})
      for (int periods : {60, 120, 240}) {
        const Planet& p = planet(seed, kOneWorldFrequency);
        CountryParams cp;
        cp.beta0 = beta;
        cp.periods = periods;
        const WorldCountries w = form_countries(p.grid, p.fields, p.land, p.graph, seed, cp);
        const Sizes s = sizes(p, w);
        MESSAGE("PROBE seed " << seed << " beta " << beta << " T " << periods << ": " << s.count << " / " << s.over_89k
                              << " / " << s.over_180k << " / " << s.over_1m << " top10 " << s.top10_share << "; small: start "
                              << s.small_start << ", seceded or split " << s.small_later << ", fill groups " << s.small_fill
                              << ", " << s.small_cells << " cells");
      }
}
