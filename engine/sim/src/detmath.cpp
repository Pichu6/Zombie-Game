#include "zs/detmath.h"

#include <bit>
#include <cassert>
#include <cmath>  // only for std::sqrt, which IEEE 754 requires to be correctly rounded
#include <cstdint>
#include <limits>

namespace zs::detmath {
namespace {

constexpr double ln2_hi = 6.93147180369123816490e-01;  // fdlibm split of ln 2
constexpr double ln2_lo = 1.90821492927058770002e-10;
constexpr double inv_ln2 = 1.44269504088896338700e+00;
constexpr double inv_ln10 = 4.34294481903251827651e-01;
constexpr double pio2_1 = 1.57079632673412561417e+00;  // fdlibm three-part pi/2
constexpr double pio2_2 = 6.07710050630396597660e-11;
constexpr double pio2_3 = 2.02226624879595063154e-21;
constexpr double two_over_pi = 6.36619772367581382433e-01;
constexpr double sqrt2 = 1.41421356237309504880;

// Callers keep |t| well below 2^62, so the cast is defined (DET-8).
std::int64_t round_to_int(double t) {
  assert(t > -0x1p62 && t < 0x1p62);
  return static_cast<std::int64_t>(t >= 0.0 ? t + 0.5 : t - 0.5);
}

// sin and cos reduce by multiples of pi/2 with a three-part constant, exact while |x| <= 2^30; the simulation never
// needs more. Beyond it, and for infinities, they return NaN instead of casting an unbounded double (DET-8).
constexpr double kTrigLimit = 0x1p30;

// Exact multiplication by 2^k, built from the bit pattern.
double scale2(double x, std::int64_t k) {
  while (k > 1023) { x *= std::bit_cast<double>(std::uint64_t{2046} << 52); k -= 1023; }
  while (k < -1022) { x *= std::bit_cast<double>(std::uint64_t{1} << 52); k += 1022; }  // 2^-1022
  return x * std::bit_cast<double>(static_cast<std::uint64_t>(k + 1023) << 52);
}

// exp(r) for |r| <= ~0.35, Taylor series to r^13 (error below 1e-17 relative).
double exp_small(double r) {
  double p = 1.0 / 6227020800.0;  // 1/13!
  constexpr double inv_fact[] = {1.0 / 479001600.0, 1.0 / 39916800.0, 1.0 / 3628800.0, 1.0 / 362880.0,
                                 1.0 / 40320.0, 1.0 / 5040.0, 1.0 / 720.0, 1.0 / 120.0, 1.0 / 24.0,
                                 1.0 / 6.0, 0.5, 1.0, 1.0};
  for (double c : inv_fact) p = p * r + c;
  return p;
}

// 2 * atanh(f) for |f| <= 0.18: log((1 + f) / (1 - f)).
double two_atanh(double f) {
  const double f2 = f * f;
  double s = 1.0 / 25.0;
  for (int k = 11; k >= 0; --k) s = s * f2 + 1.0 / (2.0 * k + 1.0);
  return 2.0 * f * s;
}

// Polynomials on |r| <= pi/4.
double sin_poly(double r) {
  const double r2 = r * r;
  double s = 1.0 / 355687428096000.0;  // 1/17!
  constexpr double c[] = {-1.0 / 1307674368000.0, 1.0 / 6227020800.0, -1.0 / 39916800.0, 1.0 / 362880.0,
                          -1.0 / 5040.0, 1.0 / 120.0, -1.0 / 6.0, 1.0};
  for (double k : c) s = s * r2 + k;
  return r * s;
}
double cos_poly(double r) {
  const double r2 = r * r;
  // (-1)^k / (2k)! for k = 9 down to 0.
  constexpr double c[] = {-1.0 / 6402373705728000.0, 1.0 / 20922789888000.0, -1.0 / 87178291200.0,
                          1.0 / 479001600.0, -1.0 / 3628800.0, 1.0 / 40320.0, -1.0 / 720.0,
                          1.0 / 24.0, -0.5, 1.0};
  double s = 0.0;
  for (double k : c) s = s * r2 + k;
  return s;
}

}  // namespace

double sqrt(double x) { return std::sqrt(x); }

double exp(double x) {
  if (x != x) return x;
  if (x > 709.782712893384) return std::numeric_limits<double>::infinity();
  if (x < -745.1332191019412) return 0.0;
  const std::int64_t k = round_to_int(x * inv_ln2);
  const double kd = static_cast<double>(k);
  const double r = (x - kd * ln2_hi) - kd * ln2_lo;
  return scale2(exp_small(r), k);
}

double expm1(double x) {
  if (x > -0.5 && x < 0.5) {
    // Taylor series of exp(x) - 1: x * sum x^n / (n + 1)!, n = 0..17.
    double p = 0.0;
    double inv_fact[18];
    double f = 1.0;
    for (int n = 0; n < 18; ++n) { f *= (n + 1); inv_fact[n] = 1.0 / f; }
    for (int n = 17; n >= 0; --n) p = p * x + inv_fact[n];
    return p * x;
  }
  return exp(x) - 1.0;
}

double log(double x) {
  if (x != x || x < 0.0) return std::numeric_limits<double>::quiet_NaN();
  if (x == 0.0) return -std::numeric_limits<double>::infinity();
  if (x == std::numeric_limits<double>::infinity()) return x;
  std::int64_t e = 0;
  if (x < 2.2250738585072014e-308) { x *= 18014398509481984.0; e = -54; }  // subnormal: scale by 2^54
  const std::uint64_t bits = std::bit_cast<std::uint64_t>(x);
  e += static_cast<std::int64_t>((bits >> 52) & 0x7ff) - 1023;
  double m = std::bit_cast<double>((bits & 0x000fffffffffffffULL) | 0x3ff0000000000000ULL);  // [1, 2)
  if (m > sqrt2) { m *= 0.5; ++e; }
  const double lm = two_atanh((m - 1.0) / (m + 1.0));
  const double ed = static_cast<double>(e);
  return ed * ln2_hi + (ed * ln2_lo + lm);
}

double log1p(double x) {
  if (x > -0.25 && x < 0.25) return two_atanh(x / (2.0 + x));
  return log(1.0 + x);
}

double log10(double x) { return log(x) * inv_ln10; }

double pow(double base, double exponent) { return exp(exponent * log(base)); }

double sin(double x) {
  if (x != x) return x;
  if (!(x >= -kTrigLimit && x <= kTrigLimit)) return std::numeric_limits<double>::quiet_NaN();
  const std::int64_t k = round_to_int(x * two_over_pi);
  const double kd = static_cast<double>(k);
  const double r = ((x - kd * pio2_1) - kd * pio2_2) - kd * pio2_3;
  switch (k & 3) {
    case 0: return sin_poly(r);
    case 1: return cos_poly(r);
    case 2: return -sin_poly(r);
    default: return -cos_poly(r);
  }
}

double cos(double x) {
  if (x != x) return x;
  if (!(x >= -kTrigLimit && x <= kTrigLimit)) return std::numeric_limits<double>::quiet_NaN();
  const std::int64_t k = round_to_int(x * two_over_pi);
  const double kd = static_cast<double>(k);
  const double r = ((x - kd * pio2_1) - kd * pio2_2) - kd * pio2_3;
  switch (k & 3) {
    case 0: return cos_poly(r);
    case 1: return -sin_poly(r);
    case 2: return -cos_poly(r);
    default: return sin_poly(r);
  }
}

}  // namespace zs::detmath
