// World fields (docs/tier0/specs/31-terrain-climate.md).
#include <doctest.h>

#include <algorithm>
#include <cmath>
#include <set>

#include "zs/gen/keyed.h"
#include "zs/gen/world_fields.h"

using namespace zs::gen;

namespace {

const WorldGrid& grid16() {
  static const WorldGrid g(16, 6371);
  return g;
}

WorldFields make(const WorldGrid& g, std::uint64_t seed, double land = 0.29, double temp = 14, double relief = 1,
                 double moisture = 1) {
  WorldParams p;
  p.seed = seed;
  p.land_share = land;
  p.global_temp_c = temp;
  p.relief = relief;
  p.moisture = moisture;
  return generate_fields(g, p);
}

double land_share(const WorldGrid& g, const WorldFields& f) {
  return area_share(g, [&](CellId i) { return f.land(i); });
}

double max_cell_share(const WorldGrid& g) {
  double m = 0;
  for (CellId i = 0; i < g.size(); ++i) m = std::max(m, g.area_km2(i));
  return m / g.sphere_area_km2();
}

// Area-weighted mean of `v` over cells where `pred` holds; NaN if none.
template <class V, class Pred>
double mean_where(const WorldGrid& g, V v, Pred pred) {
  double s = 0, a = 0;
  for (CellId i = 0; i < g.size(); ++i)
    if (pred(i)) s += v(i) * g.area_km2(i), a += g.area_km2(i);
  return a > 0 ? s / a : std::nan("");
}

std::uint64_t fields_hash(const WorldFields& f) {
  auto q = [](double x) { return static_cast<std::uint64_t>(static_cast<std::int64_t>(std::llround(x * 1000.0))); };
  std::uint64_t h = 0;
  for (std::size_t i = 0; i < f.plate.size(); ++i) {
    h = mix64(h, f.plate[i]);
    h = mix64(h, q(f.elevation_m[i]));
    h = mix64(h, q(f.temp_mean_c[i]));
    h = mix64(h, q(f.temp_range_c[i]));
    h = mix64(h, q(f.rain_mm[i]));
    h = mix64(h, static_cast<std::uint64_t>(f.biome[i]));
  }
  return h;
}

}  // namespace

TEST_CASE("fields: same seed same world, another seed another world") {
  const auto a = make(grid16(), 5), b = make(grid16(), 5), c = make(grid16(), 6);
  CHECK(fields_hash(a) == fields_hash(b));
  CHECK(fields_hash(a) != fields_hash(c));
}

TEST_CASE("fields: golden hash for n = 8, seed 1, is pinned") {
  const WorldGrid g(8, 6371);
  // Re-pinned on MSVC 2026-10-09 for spec 31's revised terrain (rules 2-4); GCC and Clang must reproduce it.
  const std::uint64_t h = fields_hash(make(g, 1));
  MESSAGE("fields hash: " << h);
  CHECK(h == 16843323902738425681ull);  // re-pinned on MSVC, 2026-10-15 (the hydrology fix: wetter tropics, channel losses)
}

TEST_CASE("fields: land share is exact to one cell") {
  const auto& g = grid16();
  for (const double want : {0.1, 0.29, 0.5, 0.8})
    for (const std::uint64_t seed : {1, 2, 3})
      CHECK(std::abs(land_share(g, make(g, seed, want)) - want) <= max_cell_share(g));
}

TEST_CASE("fields: plates are non-empty and connected") {
  const auto& g = grid16();
  for (const std::uint64_t seed : {1, 2, 3, 4}) {
    const auto f = make(g, seed);
    CHECK(f.plate_count >= 6);
    std::vector<int> count(static_cast<std::size_t>(f.plate_count), 0);
    for (CellId i = 0; i < g.size(); ++i) {
      REQUIRE(f.plate[i] < f.plate_count);
      ++count[f.plate[i]];
    }
    for (int p = 0; p < f.plate_count; ++p) {
      CHECK(count[static_cast<std::size_t>(p)] > 0);
      // Flood from one cell of the plate through its own cells reaches them all.
      CellId start = 0;
      while (f.plate[start] != p) ++start;
      std::set<CellId> seen{start};
      std::vector<CellId> stack{start};
      while (!stack.empty()) {
        const CellId c = stack.back();
        stack.pop_back();
        for (const CellId nb : g.neighbours(c))
          if (f.plate[nb] == p && seen.insert(nb).second) stack.push_back(nb);
      }
      CHECK(static_cast<int>(seen.size()) == count[static_cast<std::size_t>(p)]);
    }
  }
}

TEST_CASE("fields: values are finite and inside their ranges") {
  const auto& g = grid16();
  for (const std::uint64_t seed : {1, 2, 3}) {
    const auto f = make(g, seed);
    for (CellId i = 0; i < g.size(); ++i) {
      CHECK(std::isfinite(f.elevation_m[i]));
      CHECK(std::isfinite(f.temp_mean_c[i]));
      CHECK(std::isfinite(f.temp_range_c[i]));
      CHECK(f.elevation_m[i] >= -11000);
      CHECK(f.elevation_m[i] <= 9000);
      CHECK(f.rain_mm[i] >= 0);
      CHECK(f.rain_mm[i] <= 4000);
      CHECK(f.temp_range_c[i] > 0);
    }
  }
}

TEST_CASE("fields: biome table rules and purity") {
  CHECK(classify_biome(false, -3000, 25, 2000) == Biome::Ocean);
  CHECK(classify_biome(true, 100, -20, 100) == Biome::Ice);
  CHECK(classify_biome(true, 3000, 5, 800) == Biome::Mountain);
  CHECK(classify_biome(true, 100, 25, 2500) == Biome::TropicalForest);
  CHECK(classify_biome(true, 100, 25, 100) == Biome::Desert);
  CHECK(classify_biome(true, 100, 12, 900) == Biome::TemperateForest);
  CHECK(classify_biome(true, 100, 0, 400) == Biome::BorealForest);
  // Over a sweep: no tropical forest below 18 C, no desert above 1,000 mm, and equal inputs agree.
  for (double t = -30; t <= 35; t += 1)
    for (double r = 0; r <= 4000; r += 50) {
      const Biome b = classify_biome(true, 200, t, r);
      if (b == Biome::TropicalForest) CHECK(t >= 18);
      if (b == Biome::Desert) CHECK(r <= 1000);
      CHECK(b == classify_biome(true, 200, t, r));
    }
  const auto& g = grid16();
  const auto f = make(g, 9);
  for (CellId i = 0; i < g.size(); ++i)
    // Spec 89 amendment: a cold polar landmass's ice sheet overrides the table.
    CHECK((f.biome[i] == classify_biome(f.land(i), f.elevation_m[i], f.temp_mean_c[i], f.rain_mm[i], f.temp_range_c[i]) ||
           f.biome[i] == Biome::Ice));
}

TEST_CASE("fields: hypsometry sits near Earth's") {
  const auto& g = grid16();
  double above = 0, depth = 0;
  const int seeds = 10;
  for (int s = 1; s <= seeds; ++s) {
    const auto f = make(g, static_cast<std::uint64_t>(s));
    const double land_a = area_share(g, [&](CellId i) { return f.land(i); });
    above += area_share(g, [&](CellId i) { return f.elevation_m[i] > 1000; }) / land_a;
    depth += -mean_where(g, [&](CellId i) { return f.elevation_m[i]; }, [&](CellId i) { return !f.land(i); });
  }
  MESSAGE("land above 1,000 m " << above / seeds << " (Earth about 0.24, MG-24); mean ocean depth " << depth / seeds << " m (3,682, MG-25)");
  CHECK(above / seeds > 0.05);   // Earth about 0.24 on a 1-degree grid (MG-24)
  CHECK(above / seeds < 0.40);  // this coarse grid overstates high land since spec 89 (the one world's grid: about 16 %)
  CHECK(depth / seeds > 2500);   // Earth 3,682 m (MG-25)
  CHECK(depth / seeds < 5000);
}

TEST_CASE("fields: climate relationships") {
  const auto& g = grid16();
  double polar_warm = 0, tropic_rain = 0, subtrop_rain = 0, inland = 0, coast = 0;
  int worlds = 0;
  for (int s = 1; s <= 10; ++s) {
    const auto f = make(g, static_cast<std::uint64_t>(s));
    auto z = [&](CellId i) { return std::abs(g.sin_lat(i)); };
    const double eq = mean_where(g, [&](CellId i) { return f.temp_mean_c[i]; }, [&](CellId i) { return z(i) < 0.2; });
    const double pole = mean_where(g, [&](CellId i) { return f.temp_mean_c[i]; }, [&](CellId i) { return z(i) > 0.8; });
    polar_warm += eq - pole;
    // Colder with height, within the tropics where latitude barely varies.
    const double high = mean_where(g, [&](CellId i) { return f.temp_mean_c[i]; },
                                   [&](CellId i) { return f.land(i) && z(i) < 0.4 && f.elevation_m[i] > 1000; });
    const double low = mean_where(g, [&](CellId i) { return f.temp_mean_c[i]; },
                                  [&](CellId i) { return f.land(i) && z(i) < 0.4 && f.elevation_m[i] < 300; });
    if (!std::isnan(high)) CHECK(high < low);
    tropic_rain += mean_where(g, [&](CellId i) { return f.rain_mm[i]; }, [&](CellId i) { return f.land(i) && z(i) < 0.15; });
    subtrop_rain += mean_where(g, [&](CellId i) { return f.rain_mm[i]; },
                               [&](CellId i) { return f.land(i) && z(i) > 0.4 && z(i) < 0.6; });
    // Interior drier than coast at the same latitudes.
    const auto mid = [&](CellId i) { return f.land(i) && z(i) > 0.4 && z(i) < 0.95; };
    inland += mean_where(g, [&](CellId i) { return f.rain_mm[i]; }, [&](CellId i) { return mid(i) && f.sea_cells[i] >= 3; });
    coast += mean_where(g, [&](CellId i) { return f.rain_mm[i]; }, [&](CellId i) { return mid(i) && f.sea_cells[i] == 1; });
    ++worlds;
  }
  CHECK(polar_warm / worlds > 20);
  CHECK(tropic_rain > subtrop_rain);
  CHECK(inland < coast);
}

TEST_CASE("fields: the global temperature slider moves the mean and nothing else about the land") {
  const auto& g = grid16();
  const auto cold = make(g, 3, 0.29, 9), base = make(g, 3, 0.29, 14), warm = make(g, 3, 0.29, 19);
  auto sea_level_mean = [&](const WorldFields& f) {
    return mean_where(g, [&](CellId i) { return f.temp_mean_c[i] + 6.5 * std::max(0.0, f.elevation_m[i]) / 1000.0; },
                      [&](CellId) { return true; });
  };
  CHECK(std::abs(sea_level_mean(base) - 14) < 0.5);
  CHECK(std::abs(sea_level_mean(cold) - 9) < 0.5);
  CHECK(std::abs(sea_level_mean(warm) - 19) < 0.5);
  CHECK(land_share(g, cold) == doctest::Approx(land_share(g, warm)));
  CHECK(cold.plate == warm.plate);
}

TEST_CASE("fields: relief and moisture sliders move their own field") {
  const auto& g = grid16();
  auto spread = [&](const WorldFields& f) {
    const double m = mean_where(g, [&](CellId i) { return f.elevation_m[i]; }, [&](CellId i) { return f.land(i); });
    return mean_where(g, [&](CellId i) { return std::abs(f.elevation_m[i] - m); }, [&](CellId i) { return f.land(i); });
  };
  const auto flat = make(g, 4, 0.29, 14, 0.5), steep = make(g, 4, 0.29, 14, 1.5);
  CHECK(spread(steep) > spread(flat));
  CHECK(land_share(g, flat) == doctest::Approx(land_share(g, steep)).epsilon(0.01));
  auto rain = [&](const WorldFields& f) {
    return mean_where(g, [&](CellId i) { return f.rain_mm[i]; }, [&](CellId i) { return f.land(i); });
  };
  const auto dry = make(g, 4, 0.29, 14, 1, 0.5), wet = make(g, 4, 0.29, 14, 1, 1.5);
  CHECK(rain(wet) > 1.5 * rain(dry));
  CHECK(wet.elevation_m == dry.elevation_m);
}

TEST_CASE("fields: biome mix over 50 seeds") {
  const auto& g = grid16();
  int appears[biome_count] = {};
  double share_sum[biome_count] = {};
  for (int s = 1; s <= 50; ++s) {
    const auto f = make(g, static_cast<std::uint64_t>(s));
    const double land_a = land_share(g, f) * g.sphere_area_km2();
    for (int b = 1; b < biome_count; ++b) {
      const double a = area_share(g, [&](CellId i) { return f.biome[i] == static_cast<Biome>(b); }) * g.sphere_area_km2();
      if (a > 0) ++appears[b];
      share_sum[b] += a / land_a;
    }
  }
  for (int b = 1; b < biome_count; ++b) {
    const Biome bi = static_cast<Biome>(b);
    CAPTURE(biome_name(bi));
    // Ice and mountains depend on a polar continent and a collision; every other class is in all worlds.
    const int need = (bi == Biome::Ice || bi == Biome::Mountain) ? 35 : 48;
    CHECK(appears[b] >= need);
  }
  const double desert = share_sum[static_cast<int>(Biome::Desert)] / 50;
  const double cold = (share_sum[static_cast<int>(Biome::Ice)] + share_sum[static_cast<int>(Biome::Tundra)]) / 50;
  CHECK(desert > 0.08);  // Earth about 0.14 (hot deserts)
  CHECK(desert < 0.25);
  CHECK(cold < 0.30);
}

TEST_CASE("fields: a finer grid gives a similar biome mix") {
  const WorldGrid fine(32, 6371);
  double a[biome_count] = {}, b[biome_count] = {};
  const int seeds = 10;
  for (int s = 1; s <= seeds; ++s) {
    const auto f1 = make(grid16(), static_cast<std::uint64_t>(s)), f2 = make(fine, static_cast<std::uint64_t>(s));
    for (int k = 1; k < biome_count; ++k) {
      a[k] += area_share(grid16(), [&](CellId i) { return f1.biome[i] == static_cast<Biome>(k); }) / land_share(grid16(), f1);
      b[k] += area_share(fine, [&](CellId i) { return f2.biome[i] == static_cast<Biome>(k); }) / land_share(fine, f2);
    }
  }
  for (int k = 1; k < biome_count; ++k) CHECK(std::abs(a[k] - b[k]) / seeds < 0.08);
}

namespace {

// Connected components of cells where `pred` holds; returns the area of each.
template <class Pred>
std::vector<double> components(const WorldGrid& g, Pred pred, std::vector<std::size_t>* cells = nullptr) {
  std::vector<int> comp(g.size(), -1);
  std::vector<double> area;
  for (CellId s = 0; s < g.size(); ++s) {
    if (!pred(s) || comp[s] >= 0) continue;
    const int k = static_cast<int>(area.size());
    area.push_back(0);
    if (cells) cells->push_back(0);
    std::vector<CellId> stack{s};
    comp[s] = k;
    while (!stack.empty()) {
      const CellId c = stack.back();
      stack.pop_back();
      area.back() += g.area_km2(c);
      if (cells) ++cells->back();
      for (const CellId nb : g.neighbours(c))
        if (pred(nb) && comp[nb] < 0) comp[nb] = k, stack.push_back(nb);
    }
  }
  return area;
}

}  // namespace

TEST_CASE("fields: the same seed gives the same continents on a finer grid (A2)") {
  const WorldGrid fine(32, 6371);
  for (const std::uint64_t seed : {1, 2, 3}) {
    const auto f1 = make(grid16(), seed), f2 = make(fine, seed);
    double agree = 0, total = 0;
    for (CellId i = 0; i < grid16().size(); ++i) {
      const Vec3& c = grid16().centre(i);
      const double lat = std::asin(std::clamp(c.z, -1.0, 1.0)) * 180.0 / 3.14159265358979323846;
      const double lon = std::atan2(c.y, c.x) * 180.0 / 3.14159265358979323846;
      const CellId j = fine.cell_at(lat, lon);
      agree += (f1.land(i) == f2.land(j)) * grid16().area_km2(i);
      total += grid16().area_km2(i);
    }
    MESSAGE("seed " << seed << ": land agrees on " << agree / total << " of the surface between n = 16 and n = 32");
    CHECK(agree / total >= 0.85);
  }
}

TEST_CASE("fields: land forms continents with islands, not holes (A2)") {
  const WorldGrid g(24, 6371);
  double masses = 0, islands = 0, enclosed = 0, island_land = 0;
  const int seeds = 10;
  for (int s = 1; s <= seeds; ++s) {
    const auto f = make(g, static_cast<std::uint64_t>(s));
    std::vector<std::size_t> cells;
    const auto land = components(g, [&](CellId i) { return f.land(i); }, &cells);
    double land_area = 0, small = 0;
    for (std::size_t k = 0; k < land.size(); ++k) {
      land_area += land[k];
      if (cells[k] <= 3) ++islands, small += land[k];
    }
    masses += static_cast<double>(land.size());
    island_land += small / land_area;
    const auto sea = components(g, [&](CellId i) { return !f.land(i); });
    double sea_area = 0, biggest = 0;
    for (double a : sea) sea_area += a, biggest = std::max(biggest, a);
    enclosed += (sea_area - biggest) / sea_area;
  }
  MESSAGE("mean over " << seeds << " seeds at n = 24: " << masses / seeds << " landmasses, " << islands / seeds
                       << " islands of 1-3 cells holding " << island_land / seeds << " of land; enclosed seas "
                       << enclosed / seeds << " of the sea");
  CHECK(masses / seeds >= 8);
  CHECK(islands / seeds >= 2.0);  // spec 89 (2.0 since old hotspot volcanoes sink): arcs are chains of small islands, below this grid's cell (one world: 3 % of land)
  CHECK(enclosed / seeds <= 0.02);
}
