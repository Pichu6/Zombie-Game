// World grid (docs/tier0/specs/30-world-grid.md).
#include <doctest.h>

#include <algorithm>
#include <cmath>
#include <set>

#include "zs/detmath.h"
#include "zs/gen/keyed.h"
#include "zs/gen/world_grid.h"

using namespace zs::gen;

namespace {

Vec3 random_point(const Keyed& r, std::uint64_t id) {
  for (std::uint32_t k = 0;; ++k) {
    const Vec3 v{r.bell(id, 1, k) * 1.2, r.bell(id, 2, k) * 1.2, r.bell(id, 3, k) * 1.2};
    const double len = zs::detmath::sqrt(dot(v, v));
    if (len > 0.2) return {v.x / len, v.y / len, v.z / len};
  }
}

std::uint64_t grid_hash(const WorldGrid& g) {
  std::uint64_t h = 0;
  for (CellId i = 0; i < g.size(); ++i) {
    for (const CellId nb : g.neighbours(i)) h = mix64(h, nb);
    const Vec3& c = g.centre(i);
    h = mix64(h, static_cast<std::uint64_t>(static_cast<std::int64_t>(std::llround(c.x * 1e9))));
    h = mix64(h, static_cast<std::uint64_t>(static_cast<std::int64_t>(std::llround(c.y * 1e9))));
    h = mix64(h, static_cast<std::uint64_t>(static_cast<std::int64_t>(std::llround(c.z * 1e9))));
  }
  return h;
}

}  // namespace

TEST_CASE("grid: counts, pentagons, symmetry, connectivity, Euler") {
  for (const int n : {1, 2, 3, 4, 8, 16}) {
    const WorldGrid g(n, 6371);
    CHECK(g.size() == WorldGrid::cell_count_for(n));
    std::size_t pent = 0;
    for (CellId i = 0; i < g.size(); ++i) {
      const auto nb = g.neighbours(i);
      CHECK((nb.size() == 5 || nb.size() == 6));
      pent += nb.size() == 5;
      for (const CellId j : nb) {
        CHECK(j != i);
        const auto back = g.neighbours(j);
        CHECK(std::find(back.begin(), back.end(), i) != back.end());
      }
      const Vec3& c = g.centre(i);
      CHECK(std::abs(dot(c, c) - 1.0) < 1e-12);
    }
    CHECK(pent == 12);
    const auto d = g.graph_distance(std::vector<CellId>{0});
    CHECK(std::none_of(d.begin(), d.end(), [](std::uint32_t x) { return x == ~0u; }));
    // Euler: V - E + F = 2 with E = 30 n^2, F = 20 n^2.
    CHECK(g.edge_count() == 30u * n * n);
    CHECK(g.triangle_count() == 20u * n * n);
    CHECK(static_cast<long>(g.size()) - static_cast<long>(g.edge_count()) + static_cast<long>(g.triangle_count()) == 2);
  }
}

TEST_CASE("grid: areas sum to the sphere, spread stays bounded") {
  for (const int n : {8, 16, 32}) {
    const WorldGrid g(n, 6371);
    double sum = 0, lo = 1e30, hi = 0;
    for (CellId i = 0; i < g.size(); ++i) {
      sum += g.area_km2(i);
      lo = std::min(lo, g.area_km2(i));
      hi = std::max(hi, g.area_km2(i));
    }
    CHECK(std::abs(sum / g.sphere_area_km2() - 1.0) < 1e-9);
    CHECK(hi / lo < 1.5);
    CHECK(std::abs(sum / static_cast<double>(g.size()) / (g.sphere_area_km2() / static_cast<double>(g.size())) - 1.0) < 1e-12);
  }
}

TEST_CASE("grid: Earth size is about 5,000 km2 per cell") {
  const int f = WorldGrid::frequency_for_cells(102000);
  CHECK(f == 101);
  const double mean = 4.0 * zs::detmath::pi * 6371.0 * 6371.0 / static_cast<double>(WorldGrid::cell_count_for(f));
  CHECK(mean > 4900);
  CHECK(mean < 5100);
}

TEST_CASE("grid: mean area scales as 1 over n squared") {
  const WorldGrid a(8, 6371), b(16, 6371);
  const double ratio = static_cast<double>(b.size()) / static_cast<double>(a.size());
  CHECK(ratio > 3.7);
  CHECK(ratio < 4.3);
}

TEST_CASE("grid: nearest_cell matches brute force") {
  const WorldGrid g(12, 6371);
  const Keyed r(7);
  for (CellId i = 0; i < g.size(); ++i) CHECK(g.nearest_cell(g.centre(i), (i * 37u) % g.size()) == i);
  for (std::uint64_t k = 0; k < 400; ++k) {
    const Vec3 p = random_point(r, k);
    CellId best = 0;
    for (CellId i = 1; i < g.size(); ++i)
      if (dot(g.centre(i), p) > dot(g.centre(best), p)) best = i;
    CHECK(g.nearest_cell(p, static_cast<CellId>(k * 13 % g.size())) == best);
  }
}

TEST_CASE("grid: distances and the poles") {
  const WorldGrid g(8, 6371);
  CHECK(std::abs(g.centre(0).z - 1.0) < 1e-12);   // north pole is a cell
  CHECK(std::abs(g.centre(11).z + 1.0) < 1e-12);  // south pole is a cell
  CHECK(std::abs(g.distance_km(0, 11) - zs::detmath::pi * 6371.0) < 1e-6);
  CHECK(std::abs(g.distance_km(g.centre(0), g.centre(0))) < 1e-6);
  CHECK(g.cell_at(90, 0) == 0);
  CHECK(g.cell_at(-90, 0) == 11);
  // Neighbours are close: each is within a small multiple of the mean spacing.
  const double spacing = zs::detmath::sqrt(g.sphere_area_km2() / static_cast<double>(g.size()));
  for (CellId i = 0; i < g.size(); ++i)
    for (const CellId j : g.neighbours(i)) CHECK(g.distance_km(i, j) < 2.0 * spacing);
}

TEST_CASE("grid: deterministic, and the golden hash is pinned") {
  const WorldGrid a(8, 6371), b(8, 6371);
  CHECK(grid_hash(a) == grid_hash(b));
  CHECK(grid_hash(a) != grid_hash(WorldGrid(9, 6371)));
  // Pinned on Linux (GCC and Clang); Windows must reproduce it.
  CHECK(grid_hash(a) == 6600657962485029725ull);
}

TEST_CASE("grid: frequency n is nested inside 2n, so the grid is a zoom ladder") {
  // Every cell centre of a coarse grid is exactly a cell centre of the grid at double the frequency.
  for (const int n : {1, 2, 4, 6}) {
    const WorldGrid coarse(n, 6371), fine(2 * n, 6371);
    for (CellId i = 0; i < coarse.size(); ++i) {
      const CellId j = fine.nearest_cell(coarse.centre(i), 0);
      CHECK(std::abs(dot(fine.centre(j), coarse.centre(i)) - 1.0) < 1e-12);
    }
  }
}
