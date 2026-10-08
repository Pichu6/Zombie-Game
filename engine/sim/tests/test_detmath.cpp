// Spec 01 rule 5: our own transcendental functions.
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

#include <doctest.h>

#include "zs/archive.h"
#include "zs/detmath.h"

namespace dm = zs::detmath;

namespace {
double rel_err(double a, double b) {
  if (a == b) return 0.0;
  return std::fabs(a - b) / std::fmax(std::fabs(b), 1e-300);
}
// Fixed inputs from a simple integer sequence (no libm involved).
double input(std::uint64_t i, double lo, double hi) {
  const std::uint64_t x = (i * 0x9E3779B97F4A7C15ULL) >> 11;
  return lo + (hi - lo) * (static_cast<double>(x) * (1.0 / 9007199254740992.0));
}
}  // namespace

TEST_CASE("detmath stays close to the platform libm") {
  double worst_exp = 0, worst_log = 0, worst_sin = 0, worst_cos = 0, worst_expm1 = 0, worst_log1p = 0;
  for (std::uint64_t i = 0; i < 200000; ++i) {
    const double x = input(i, -700.0, 700.0);
    worst_exp = std::fmax(worst_exp, rel_err(dm::exp(x), std::exp(x)));
    const double y = input(i, 1e-300, 1e300);
    worst_log = std::fmax(worst_log, rel_err(dm::log(y), std::log(y)));
    const double a = input(i, -1000.0, 1000.0);
    if (std::fabs(std::sin(a)) > 1e-6) worst_sin = std::fmax(worst_sin, rel_err(dm::sin(a), std::sin(a)));
    if (std::fabs(std::cos(a)) > 1e-6) worst_cos = std::fmax(worst_cos, rel_err(dm::cos(a), std::cos(a)));
    const double s = input(i, -0.6, 0.6);
    if (s != 0.0) {
      worst_expm1 = std::fmax(worst_expm1, rel_err(dm::expm1(s), std::expm1(s)));
      worst_log1p = std::fmax(worst_log1p, rel_err(dm::log1p(s), std::log1p(s)));
    }
  }
  CHECK(worst_exp < 1e-14);
  CHECK(worst_log < 1e-14);
  CHECK(worst_sin < 1e-9);  // relative error grows near zeros of sin and cos; absolute error stays tiny
  CHECK(worst_cos < 1e-9);
  CHECK(worst_expm1 < 1e-14);
  CHECK(worst_log1p < 1e-14);
  CHECK(dm::log(1.0) == 0.0);
  CHECK(dm::exp(0.0) == 1.0);
  CHECK(dm::log10(1000.0) == doctest::Approx(3.0).epsilon(1e-15));
}

TEST_CASE("detmath gives the same bits on every compiler (A1 across compilers)") {
  zs::ArchiveWriter w;
  for (std::uint64_t i = 0; i < 100000; ++i) {
    w.f64(dm::exp(input(i, -50.0, 50.0)));
    w.f64(dm::log(input(i, 1e-10, 1e10)));
    w.f64(dm::sin(input(i, -100.0, 100.0)));
    w.f64(dm::cos(input(i, -100.0, 100.0)));
    w.f64(dm::log10(input(i, 1.0, 1e6)));
  }
  // Pinned on Linux (GCC). Windows (MSVC) must produce the same value.
  CHECK(zs::hash_bytes(w.bytes()) == 0x7fccc6b0bd223228ULL);
}

TEST_CASE("sin and cos return NaN for infinities and beyond 2^30 instead of an unbounded cast (DET-8)") {
  const double inf = std::numeric_limits<double>::infinity();
  CHECK(std::isnan(dm::sin(inf)));
  CHECK(std::isnan(dm::cos(-inf)));
  CHECK(std::isnan(dm::sin(1e19)));
  CHECK(std::isnan(dm::cos(-1e19)));
  CHECK(std::isfinite(dm::sin(0x1p30)));
  CHECK(std::isfinite(dm::cos(-0x1p30)));
}
