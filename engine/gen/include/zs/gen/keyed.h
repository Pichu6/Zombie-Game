// Stateless keyed draws for content generators (docs/design/content-tools.md section 5).
// A draw is a pure function of (seed, id, field, k): no counters, so generating one person never
// shifts another, and any person can be regenerated alone from the world seed and their id.
#pragma once
#include <cstdint>
#include <span>
#include <string_view>

#include "zs/philox.h"

namespace zs::gen {

// SplitMix64 finaliser: mixes a 64-bit value into a well-spread one.
constexpr std::uint64_t mix64(std::uint64_t x) {
  x += 0x9E3779B97F4A7C15ull;
  x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
  x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
  return x ^ (x >> 31);
}
constexpr std::uint64_t mix64(std::uint64_t a, std::uint64_t b) { return mix64(a ^ mix64(b)); }

// FNV-1a, to turn names (culture ids, kinds) into stable ids.
constexpr std::uint64_t hash_str(std::string_view s) {
  std::uint64_t h = 0xCBF29CE484222325ull;
  for (const char c : s) h = (h ^ static_cast<unsigned char>(c)) * 0x100000001B3ull;
  return h;
}

class Keyed {
 public:
  explicit Keyed(std::uint64_t seed) : seed_(seed) {}
  std::uint64_t seed() const { return seed_; }

  std::uint64_t bits(std::uint64_t id, std::uint32_t field, std::uint32_t k = 0) const {
    const auto out = philox4x32_10(
        {static_cast<std::uint32_t>(id), static_cast<std::uint32_t>(id >> 32), field, k},
        {static_cast<std::uint32_t>(seed_), static_cast<std::uint32_t>(seed_ >> 32)});
    return (static_cast<std::uint64_t>(out[0]) << 32) | out[1];
  }
  double u01(std::uint64_t id, std::uint32_t field, std::uint32_t k = 0) const {
    return static_cast<double>(bits(id, field, k) >> 11) * 0x1.0p-53;
  }
  bool chance(std::uint64_t id, std::uint32_t field, double p, std::uint32_t k = 0) const {
    return u01(id, field, k) < p;
  }
  // [0, n); n > 0. Bias is below 2^-40 for the small n used here.
  std::uint64_t below(std::uint64_t id, std::uint32_t field, std::uint64_t n, std::uint32_t k = 0) const {
    return bits(id, field, k) % n;
  }
  // Integer in [lo, hi].
  int range(std::uint64_t id, std::uint32_t field, int lo, int hi, std::uint32_t k = 0) const {
    return hi <= lo ? lo : lo + static_cast<int>(below(id, field, static_cast<std::uint64_t>(hi - lo + 1), k));
  }
  // Roughly normal in [-1, 1] (mean of 4 uniforms, rescaled): no transcendental maths.
  double bell(std::uint64_t id, std::uint32_t field, std::uint32_t k = 0) const {
    double s = 0;
    for (std::uint32_t i = 0; i < 4; ++i) s += u01(id, field, k * 4 + i);
    return s / 2.0 - 1.0;
  }
  std::size_t weighted(std::uint64_t id, std::uint32_t field, std::span<const double> w, std::uint32_t k = 0) const {
    double total = 0;
    for (const double x : w) total += x;
    double t = u01(id, field, k) * total;
    for (std::size_t i = 0; i < w.size(); ++i) {
      if (t < w[i]) return i;
      t -= w[i];
    }
    return w.empty() ? 0 : w.size() - 1;
  }

 private:
  std::uint64_t seed_;
};

}  // namespace zs::gen
