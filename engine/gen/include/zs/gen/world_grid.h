// The world grid: an icosahedral hexagon-and-pentagon grid on a sphere (spec 30, decision 0038).
// Derived from (frequency, radius) alone, never saved; built only from + - * / and sqrt, so it is
// bit-identical across compilers.
#pragma once
#include <cstdint>
#include <span>
#include <vector>

namespace zs::gen {

struct Vec3 {
  double x = 0, y = 0, z = 0;
};

inline double dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(const Vec3& a, const Vec3& b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

using CellId = std::uint32_t;

class WorldGrid {
 public:
  // frequency >= 1; cells = 10 f^2 + 2. radius_km > 0 (Earth 6371).
  WorldGrid(int frequency, double radius_km);

  // The frequency whose cell count is nearest `target_cells` (Earth size at 5,000 km^2: 101).
  static int frequency_for_cells(std::uint64_t target_cells);
  static std::uint64_t cell_count_for(int frequency) { return 10ull * frequency * frequency + 2; }

  int frequency() const { return frequency_; }
  double radius_km() const { return radius_km_; }
  std::size_t size() const { return centre_.size(); }
  double sphere_area_km2() const { return sphere_area_; }

  const Vec3& centre(CellId i) const { return centre_[i]; }
  double area_km2(CellId i) const { return area_[i]; }
  std::span<const CellId> neighbours(CellId i) const {
    return {nbr_.data() + nbr_start_[i], nbr_start_[i + 1] - nbr_start_[i]};
  }
  std::size_t edge_count() const { return edges_; }
  std::size_t triangle_count() const { return triangles_; }

  // Latitude helpers from the unit centre: z is the sine of latitude.
  double sin_lat(CellId i) const { return centre_[i].z; }

  // Great-circle distance in km between two cell centres or two points on the unit sphere.
  double distance_km(CellId a, CellId b) const { return distance_km(centre_[a], centre_[b]); }
  double distance_km(const Vec3& a, const Vec3& b) const;

  // Exact nearest cell centre to a unit vector. `hint` is where the walk starts.
  CellId nearest_cell(const Vec3& unit_point, CellId hint = 0) const;
  CellId cell_at(double lat_deg, double lon_deg) const;

  // Neighbour steps from the nearest of `sources` to every cell (multi-source breadth first).
  std::vector<std::uint32_t> graph_distance(std::span<const CellId> sources) const;

 private:
  int frequency_;
  double radius_km_;
  double sphere_area_ = 0;
  std::size_t edges_ = 0, triangles_ = 0;
  std::vector<Vec3> centre_;
  std::vector<double> area_;
  std::vector<std::uint32_t> nbr_start_;
  std::vector<CellId> nbr_;
};

// Angle in radians in [0, pi] between two unit vectors, by a deterministic arctangent.
double angle_between(const Vec3& a, const Vec3& b);

}  // namespace zs::gen
