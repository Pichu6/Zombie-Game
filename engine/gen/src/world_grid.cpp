// World grid construction (spec 30). Only + - * / and sqrt, plus detmath::sin for the spacing warp.
#include "zs/gen/world_grid.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <utility>

#include "zs/detmath.h"

namespace zs::gen {
namespace {

Vec3 normalised(const Vec3& v) {
  const double len = detmath::sqrt(dot(v, v));
  return {v.x / len, v.y / len, v.z / len};
}

// Deterministic arctangent of t >= 0: halve the argument twice, then the Taylor series.
double atan_nonneg(double t) {
  double scale = 1;
  for (int i = 0; i < 3; ++i) {
    t = t / (1.0 + detmath::sqrt(1.0 + t * t));
    scale *= 2;
  }
  const double t2 = t * t;
  double term = t, sum = 0;
  for (int k = 0; k < 12; ++k) {
    sum += term / static_cast<double>(2 * k + 1) * ((k & 1) ? -1.0 : 1.0);
    term *= t2;
  }
  return sum * scale;
}

// atan2(y, x) for y >= 0, result in [0, pi].
double atan2_upper(double y, double x) {
  if (x == 0 && y == 0) return 0;
  if (x >= 0) return y >= x ? detmath::pi / 2 - atan_nonneg(x / y) : atan_nonneg(y / x);
  const double ax = -x;
  return detmath::pi - (y >= ax ? detmath::pi / 2 - atan_nonneg(ax / y) : atan_nonneg(y / ax));
}

struct Face {
  int a, b, c;
};

// Icosahedron with a vertex on each pole: 0 = north, 1..5 upper ring, 6..10 lower ring, 11 = south.
void icosahedron(std::vector<Vec3>& v, std::vector<Face>& f) {
  const double s5 = detmath::sqrt(5.0);
  const double ring_z = 1.0 / s5, ring_r = 2.0 / s5;
  const double c72 = (s5 - 1) / 4, s72 = detmath::sqrt(10 + 2 * s5) / 4;
  const double c144 = -(s5 + 1) / 4, s144 = detmath::sqrt(10 - 2 * s5) / 4;
  // cos and sin of 0, 72, 144, 216, 288 degrees; the lower ring is rotated by 36 degrees.
  const double cs[5][2] = {{1, 0}, {c72, s72}, {c144, s144}, {c144, -s144}, {c72, -s72}};
  const double c36 = (s5 + 1) / 4, s36 = detmath::sqrt(10 - 2 * s5) / 4;
  v.assign(12, {});
  v[0] = {0, 0, 1};
  v[11] = {0, 0, -1};
  for (int k = 0; k < 5; ++k) {
    v[1 + k] = {ring_r * cs[k][0], ring_r * cs[k][1], ring_z};
    const double cx = cs[k][0] * c36 - cs[k][1] * s36, cy = cs[k][1] * c36 + cs[k][0] * s36;
    v[6 + k] = {ring_r * cx, ring_r * cy, -ring_z};
  }
  f.clear();
  for (int k = 0; k < 5; ++k) {
    const int k1 = (k + 1) % 5;
    f.push_back({0, 1 + k, 1 + k1});
    f.push_back({1 + k, 6 + k, 1 + k1});
    f.push_back({6 + k, 6 + k1, 1 + k1});
    f.push_back({11, 6 + k1, 6 + k});
  }
}

}  // namespace

double angle_between(const Vec3& a, const Vec3& b) {
  const Vec3 c = cross(a, b);
  return atan2_upper(detmath::sqrt(dot(c, c)), dot(a, b));
}

int WorldGrid::frequency_for_cells(std::uint64_t target_cells) {
  int best = 1;
  std::uint64_t best_gap = ~0ull;
  for (int f = 1; f <= 4000; ++f) {
    const std::uint64_t n = cell_count_for(f);
    const std::uint64_t gap = n > target_cells ? n - target_cells : target_cells - n;
    if (gap < best_gap) best_gap = gap, best = f;
    if (n > target_cells) break;
  }
  return best;
}

WorldGrid::WorldGrid(int frequency, double radius_km) : frequency_(frequency), radius_km_(radius_km) {
  if (frequency < 1) throw std::invalid_argument("WorldGrid: frequency must be at least 1");
  if (!(radius_km > 0)) throw std::invalid_argument("WorldGrid: radius must be positive");
  const int n = frequency;
  std::vector<Vec3> base;
  std::vector<Face> faces;
  icosahedron(base, faces);
  // Spacing warp (spec 30 rule 1): a lattice weight w becomes sin(w theta) / sin(theta), theta being
  // the angle between icosahedron vertices, so cells near face centres are not oversized.
  const double theta = angle_between(base[0], base[1]);
  const double sin_theta = detmath::sin(theta);
  auto warp = [&](double w) { return detmath::sin(w * theta) / sin_theta; };

  centre_.assign(cell_count_for(n), {});
  for (int i = 0; i < 12; ++i) centre_[static_cast<std::size_t>(i)] = base[static_cast<std::size_t>(i)];
  CellId next = 12;

  // Edge points, ids in order of first appearance across the faces.
  std::map<std::pair<int, int>, CellId> edge_first;
  auto edge_cell = [&](int u, int w, int step) -> CellId {  // step counted from u toward w
    const int lo = std::min(u, w), hi = std::max(u, w);
    const int s = (u == lo) ? step : n - step;
    return edge_first.at({lo, hi}) + static_cast<CellId>(s - 1);
  };
  for (const Face& f : faces) {
    const int e[3][2] = {{f.a, f.b}, {f.a, f.c}, {f.b, f.c}};
    for (const auto& uv : e) {
      const std::pair<int, int> key{std::min(uv[0], uv[1]), std::max(uv[0], uv[1])};
      if (edge_first.count(key)) continue;
      edge_first[key] = next;
      const Vec3& p = base[static_cast<std::size_t>(key.first)];
      const Vec3& q = base[static_cast<std::size_t>(key.second)];
      for (int t = 1; t < n; ++t) {
        const double wq = warp(static_cast<double>(t) / n), wp = warp(1.0 - static_cast<double>(t) / n);
        centre_[next++] = normalised({p.x * wp + q.x * wq, p.y * wp + q.y * wq, p.z * wp + q.z * wq});
      }
    }
  }

  // Lattice point (i, j) of a face has weights a: n-i-j, b: i, c: j.
  std::vector<std::vector<CellId>> face_cell(faces.size());
  for (std::size_t fi = 0; fi < faces.size(); ++fi) {
    const Face& f = faces[fi];
    face_cell[fi].assign(static_cast<std::size_t>((n + 1) * (n + 1)), 0);
    for (int i = 0; i <= n; ++i) {
      for (int j = 0; i + j <= n; ++j) {
        CellId id;
        if (i == 0 && j == 0) id = static_cast<CellId>(f.a);
        else if (i == n) id = static_cast<CellId>(f.b);
        else if (j == n) id = static_cast<CellId>(f.c);
        else if (j == 0) id = edge_cell(f.a, f.b, i);
        else if (i == 0) id = edge_cell(f.a, f.c, j);
        else if (i + j == n) id = edge_cell(f.b, f.c, j);
        else {
          id = next++;
          const double wa = warp(static_cast<double>(n - i - j) / n), wb = warp(static_cast<double>(i) / n),
                       wc = warp(static_cast<double>(j) / n);
          const Vec3 &a = base[static_cast<std::size_t>(f.a)], &b = base[static_cast<std::size_t>(f.b)],
                     &c = base[static_cast<std::size_t>(f.c)];
          centre_[id] = normalised({a.x * wa + b.x * wb + c.x * wc, a.y * wa + b.y * wb + c.y * wc,
                                    a.z * wa + b.z * wb + c.z * wc});
        }
        face_cell[fi][static_cast<std::size_t>(i * (n + 1) + j)] = id;
      }
    }
  }
  if (next != centre_.size()) throw std::logic_error("WorldGrid: cell count mismatch");

  // Triangles: areas and neighbour pairs.
  area_.assign(centre_.size(), 0.0);
  std::vector<std::pair<CellId, CellId>> pairs;
  pairs.reserve(centre_.size() * 6);
  auto add_triangle = [&](CellId p, CellId q, CellId r) {
    const Vec3 &a = centre_[p], &b = centre_[q], &c = centre_[r];
    const Vec3 ab{b.x - a.x, b.y - a.y, b.z - a.z}, ac{c.x - a.x, c.y - a.y, c.z - a.z};
    const Vec3 x = cross(ab, ac);
    const double flat = 0.5 * detmath::sqrt(dot(x, x));
    area_[p] += flat / 3;
    area_[q] += flat / 3;
    area_[r] += flat / 3;
    pairs.push_back({p, q});
    pairs.push_back({q, p});
    pairs.push_back({q, r});
    pairs.push_back({r, q});
    pairs.push_back({p, r});
    pairs.push_back({r, p});
    ++triangles_;
  };
  for (std::size_t fi = 0; fi < faces.size(); ++fi) {
    auto id_of = [&](int i, int j) -> CellId { return face_cell[fi][static_cast<std::size_t>(i * (n + 1) + j)]; };
    for (int i = 0; i < n; ++i)
      for (int j = 0; i + j < n; ++j) {
        add_triangle(id_of(i, j), id_of(i + 1, j), id_of(i, j + 1));
        if (i + j < n - 1) add_triangle(id_of(i + 1, j), id_of(i + 1, j + 1), id_of(i, j + 1));
      }
  }
  std::sort(pairs.begin(), pairs.end());
  pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());
  nbr_start_.assign(centre_.size() + 1, 0);
  for (const auto& pr : pairs) ++nbr_start_[pr.first + 1];
  for (std::size_t i = 0; i < centre_.size(); ++i) nbr_start_[i + 1] += nbr_start_[i];
  nbr_.reserve(pairs.size());
  for (const auto& pr : pairs) nbr_.push_back(pr.second);
  edges_ = pairs.size() / 2;

  // One common factor makes the areas sum to the sphere's area (spec 30 rule 4).
  sphere_area_ = 4.0 * detmath::pi * radius_km * radius_km;
  double flat_total = 0;
  for (const double a : area_) flat_total += a;
  const double k = sphere_area_ / flat_total;
  for (double& a : area_) a *= k;
}

double WorldGrid::distance_km(const Vec3& a, const Vec3& b) const { return radius_km_ * angle_between(a, b); }

CellId WorldGrid::nearest_cell(const Vec3& p, CellId hint) const {
  CellId best = hint;
  double best_dot = dot(centre_[best], p);
  for (;;) {
    CellId step = best;
    for (const CellId nb : neighbours(best)) {
      const double d = dot(centre_[nb], p);
      if (d > best_dot) best_dot = d, step = nb;
    }
    if (step == best) return best;
    best = step;
  }
}

CellId WorldGrid::cell_at(double lat_deg, double lon_deg) const {
  const double k = detmath::pi / 180.0;
  const double la = lat_deg * k, lo = lon_deg * k;
  const double cl = detmath::cos(la);
  return nearest_cell({cl * detmath::cos(lo), cl * detmath::sin(lo), detmath::sin(la)});
}

std::vector<std::uint32_t> WorldGrid::graph_distance(std::span<const CellId> sources) const {
  constexpr std::uint32_t unreached = ~0u;
  std::vector<std::uint32_t> dist(size(), unreached);
  std::vector<CellId> frontier(sources.begin(), sources.end()), next;
  for (const CellId s : frontier) dist[s] = 0;
  for (std::uint32_t d = 0; !frontier.empty(); ++d) {
    next.clear();
    for (const CellId c : frontier)
      for (const CellId nb : neighbours(c))
        if (dist[nb] == unreached) dist[nb] = d + 1, next.push_back(nb);
    frontier.swap(next);
  }
  return dist;
}

}  // namespace zs::gen
