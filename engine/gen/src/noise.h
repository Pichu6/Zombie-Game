// Resolution-independent noise on the sphere for the world generators (spec 31 rule 4). Value noise on a 3D lattice
// with quintic fade, summed in octaves set by wavelength in km, so a seed gives the same shapes on any grid; octaves
// shorter than twice the cell spacing are left out (they would only alias). Only + - * and floor: same bits everywhere.
#pragma once
#include <cmath>
#include <cstdint>

#include "zs/gen/keyed.h"
#include "zs/gen/world_grid.h"

namespace zs::gen::noise {

// Lattice value in [-1, 1] for integer point (x, y, z), keyed by seed and channel.
inline double lattice(std::uint64_t seed, std::uint64_t channel, std::int64_t x, std::int64_t y, std::int64_t z) {
  const std::uint64_t h = mix64(mix64(mix64(mix64(seed, channel), static_cast<std::uint64_t>(x)), static_cast<std::uint64_t>(y)),
                                static_cast<std::uint64_t>(z));
  return static_cast<double>(h >> 11) * 0x1.0p-52 - 1.0;
}

inline double fade(double t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

// Value noise at point p (in lattice units), about [-1, 1].
inline double value(std::uint64_t seed, std::uint64_t channel, double px, double py, double pz) {
  const double fx = std::floor(px), fy = std::floor(py), fz = std::floor(pz);
  const auto ix = static_cast<std::int64_t>(fx), iy = static_cast<std::int64_t>(fy), iz = static_cast<std::int64_t>(fz);
  const double u = fade(px - fx), v = fade(py - fy), w = fade(pz - fz);
  auto L = [&](int a, int b, int c) { return lattice(seed, channel, ix + a, iy + b, iz + c); };
  const double x00 = L(0, 0, 0) + u * (L(1, 0, 0) - L(0, 0, 0));
  const double x10 = L(0, 1, 0) + u * (L(1, 1, 0) - L(0, 1, 0));
  const double x01 = L(0, 0, 1) + u * (L(1, 0, 1) - L(0, 0, 1));
  const double x11 = L(0, 1, 1) + u * (L(1, 1, 1) - L(0, 1, 1));
  const double y0 = x00 + v * (x10 - x00), y1 = x01 + v * (x11 - x01);
  return y0 + w * (y1 - y0);
}

struct Fbm {
  double longest_km = 4000;  // first octave's wavelength
  int octaves = 7;           // each halves the wavelength
  double gain = 0.5;         // amplitude ratio per octave
  double min_km = 0;         // octaves shorter than this are left out (2 x cell spacing)
};

// Fractal sum at unit vector c on a sphere of radius_km, normalised by the amplitude kept: about [-1, 1].
inline double fbm(std::uint64_t seed, std::uint64_t channel, const Vec3& c, double radius_km, const Fbm& f) {
  double sum = 0, norm = 0, amp = 1, lambda = f.longest_km;
  for (int k = 0; k < f.octaves; ++k, amp *= f.gain, lambda *= 0.5) {
    if (lambda < f.min_km && k > 0) break;
    const double s = radius_km / lambda;
    sum += amp * value(seed, channel * 64 + static_cast<std::uint64_t>(k), c.x * s, c.y * s, c.z * s);
    norm += amp;
  }
  return norm > 0 ? sum / norm : 0.0;
}

}  // namespace zs::gen::noise
