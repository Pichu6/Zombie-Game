#include "zs/hypergeometric.h"

#include <algorithm>
#include <vector>

#include "zs/detmath.h"

namespace zs {

namespace {
constexpr std::uint64_t kTableSize = 1025;
const std::vector<double>& table() {
  static const std::vector<double> t = [] {
    std::vector<double> v(kTableSize, 0.0);
    for (std::uint64_t n = 2; n < kTableSize; ++n) v[n] = v[n - 1] + detmath::log(static_cast<double>(n));
    return v;
  }();
  return t;
}
}  // namespace

double log_factorial(std::uint64_t n) {
  if (n < kTableSize) return table()[n];
  const double x = static_cast<double>(n);
  const double inv = 1.0 / x;
  // Stirling: n ln n - n + (1/2) ln(2 pi n) + 1/(12 n) - 1/(360 n^3) + 1/(1260 n^5); error below 1e-17 here.
  return x * detmath::log(x) - x + 0.5 * detmath::log(2.0 * detmath::pi * x) + inv / 12.0 - inv * inv * inv / 360.0 +
         inv * inv * inv * inv * inv / 1260.0;
}

std::uint64_t hypergeometric(double u, std::uint64_t N, std::uint64_t K, std::uint64_t n) {
  const std::uint64_t bad = N - K;
  const std::uint64_t kmin = n > bad ? n - bad : 0;
  const std::uint64_t kmax = std::min(n, K);
  if (kmin >= kmax) return kmin;
  auto mode = static_cast<std::uint64_t>(static_cast<double>(n + 1) * static_cast<double>(K + 1) / static_cast<double>(N + 2));
  mode = std::clamp(mode, kmin, kmax);
  const double log_p = log_factorial(K) - log_factorial(mode) - log_factorial(K - mode) + log_factorial(bad) -
                       log_factorial(n - mode) - log_factorial(bad - (n - mode)) -
                       (log_factorial(N) - log_factorial(n) - log_factorial(N - n));
  double cum = detmath::exp(log_p);
  if (u < cum) return mode;
  std::uint64_t lo = mode, hi = mode;
  double p_lo = cum, p_hi = cum;
  const auto up = [&](std::uint64_t k) {  // pmf(k+1) / pmf(k)
    return static_cast<double>(K - k) * static_cast<double>(n - k) /
           (static_cast<double>(k + 1) * static_cast<double>(bad - n + k + 1));
  };
  const auto down = [&](std::uint64_t k) {  // pmf(k-1) / pmf(k)
    return static_cast<double>(k) * static_cast<double>(bad - n + k) /
           (static_cast<double>(K - k + 1) * static_cast<double>(n - k + 1));
  };
  for (;;) {
    const double next_hi = hi < kmax ? p_hi * up(hi) : 0.0;
    const double next_lo = lo > kmin ? p_lo * down(lo) : 0.0;
    if (next_hi <= 0.0 && next_lo <= 0.0) return mode;  // all mass walked (rounding): the mode is the safe answer
    if (next_hi >= next_lo) { ++hi; p_hi = next_hi; cum += next_hi; if (u < cum) return hi; }
    else { --lo; p_lo = next_lo; cum += next_lo; if (u < cum) return lo; }
  }
}

}  // namespace zs
