// The jump test harness (layer jumps §5 step 2): the five properties every jump is tested for, and the
// statistics that compare a cold jump with hot stepping in distribution.
// Research: docs/research/catch-up-jump-forms.md §5 (50 hot against thousands of cold, Anderson–Darling
// and energy distance; the 90 % band alone only catches gross errors).
//
// Properties (each a one-line helper here; the test states the agreement level):
//   round_trip      demote then promote (or jump then jump back) gives the same state           (A1)
//   enumeration     an aggregate equals the sum over the enumerated members                     (A1)
//   agreement       a jump matches stepping in distribution (AD, energy distance, proportions)  (A2)
//   order           the result does not depend on the order of independent operations          (A1)
//   determinism     the same inputs give the same result                                        (A1)
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace zs::jumps {

using Sample = std::vector<double>;
using Vectors = std::vector<std::vector<double>>;  // one row per run, one column per quantity

inline double mean(const Sample& s) {
  return s.empty() ? 0.0 : std::accumulate(s.begin(), s.end(), 0.0) / static_cast<double>(s.size());
}
inline double variance(const Sample& s) {
  if (s.size() < 2) return 0.0;
  const double m = mean(s);
  double v = 0;
  for (double x : s) v += (x - m) * (x - m);
  return v / static_cast<double>(s.size() - 1);
}

// ---- Anderson–Darling k-sample test (Scholz and Stephens 1987), midrank version so ties (integer
// counts) are handled. T is the standardised statistic; the approximate p interpolates the published
// critical values. reject_5pct is the usual verdict for "the two samples differ".
struct ADResult {
  double a2 = 0;           // A²_akN
  double t = 0;            // standardised
  double p_approx = 1;     // interpolated, clamped to [0.001, 0.25] outside the table
  bool reject_5pct = false;
};

inline ADResult anderson_darling_ksample(const std::vector<Sample>& samples) {
  ADResult out;
  const std::size_t k = samples.size();
  std::size_t N = 0;
  std::vector<double> all;
  for (const Sample& s : samples) {
    N += s.size();
    all.insert(all.end(), s.begin(), s.end());
  }
  if (k < 2 || N < 4) return out;
  std::sort(all.begin(), all.end());
  std::vector<double> zstar;
  for (double x : all)
    if (zstar.empty() || zstar.back() != x) zstar.push_back(x);
  const std::size_t L = zstar.size();
  std::vector<double> lj(L), bj(L);
  for (std::size_t j = 0; j < L; ++j) {
    const auto lo = std::lower_bound(all.begin(), all.end(), zstar[j]) - all.begin();
    const auto hi = std::upper_bound(all.begin(), all.end(), zstar[j]) - all.begin();
    lj[j] = static_cast<double>(hi - lo);
    bj[j] = static_cast<double>(lo) + lj[j] / 2;
  }
  const double n = static_cast<double>(N);
  double a2 = 0;
  for (const Sample& s0 : samples) {
    Sample s = s0;
    std::sort(s.begin(), s.end());
    const double ni = static_cast<double>(s.size());
    double inner_sum = 0;
    for (std::size_t j = 0; j < L; ++j) {
      const auto lo = std::lower_bound(s.begin(), s.end(), zstar[j]) - s.begin();
      const auto hi = std::upper_bound(s.begin(), s.end(), zstar[j]) - s.begin();
      const double fij = static_cast<double>(hi - lo);
      const double mij = static_cast<double>(lo) + fij / 2;
      const double denom = bj[j] * (n - bj[j]) - n * lj[j] / 4;
      if (denom <= 0) continue;
      inner_sum += lj[j] / n * (n * mij - ni * bj[j]) * (n * mij - ni * bj[j]) / denom;
    }
    a2 += inner_sum / ni;
  }
  a2 *= (n - 1) / n;
  // Variance of A² under H0 (Scholz and Stephens, eq. 7).
  double H = 0, h = 0, g = 0;
  for (const Sample& s : samples) H += 1.0 / static_cast<double>(s.size());
  for (std::size_t i = 1; i < N; ++i) h += 1.0 / static_cast<double>(i);
  for (std::size_t i = 1; i + 1 < N; ++i)
    for (std::size_t j = i + 1; j < N; ++j) g += 1.0 / (static_cast<double>(N - i) * static_cast<double>(j));
  const double kk = static_cast<double>(k);
  const double a = (4 * g - 6) * (kk - 1) + (10 - 6 * g) * H;
  const double b = (2 * g - 4) * kk * kk + 8 * h * kk + (2 * g - 14 * h - 4) * H - 8 * h + 4 * g - 6;
  const double c = (6 * h + 2 * g - 2) * kk * kk + (4 * h - 4 * g + 6) * kk + (2 * h - 6) * H + 4 * h;
  const double d = (2 * h + 6) * kk * kk - 4 * h * kk;
  const double sigma2 = (a * n * n * n + b * n * n + c * n + d) / ((n - 1) * (n - 2) * (n - 3));
  out.a2 = a2;
  out.t = sigma2 > 0 ? (a2 - (kk - 1)) / std::sqrt(sigma2) : 0;
  // Critical values of T for m = k - 1 = 1 at p = .25 .10 .05 .025 .01 .005 .001 (Scholz and Stephens, table 1).
  const double tc[7] = {0.326, 1.225, 1.960, 2.719, 3.752, 4.592, 6.546};
  const double pc[7] = {0.25, 0.10, 0.05, 0.025, 0.01, 0.005, 0.001};
  if (out.t <= tc[0]) out.p_approx = 0.25;
  else if (out.t >= tc[6]) out.p_approx = 0.001;
  else
    for (int i = 0; i < 6; ++i)
      if (out.t >= tc[i] && out.t < tc[i + 1]) {
        const double f = (out.t - tc[i]) / (tc[i + 1] - tc[i]);
        out.p_approx = std::exp(std::log(pc[i]) + f * (std::log(pc[i + 1]) - std::log(pc[i])));
      }
  out.reject_5pct = out.t > 1.960;
  return out;
}

// ---- Energy distance (Székely and Rizzo) on standardised vectors, with a deterministic permutation
// test. The statistic is 2 E|A-B| - E|A-A'| - E|B-B'|; p is the share of permutations at least as large.
struct EnergyResult {
  double statistic = 0;
  double p_value = 1;
  bool reject_5pct = false;
};

namespace detail {
inline std::uint64_t splitmix(std::uint64_t& x) {
  x += 0x9E3779B97F4A7C15ULL;
  std::uint64_t z = x;
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}
inline double energy_stat(const Vectors& all, const std::vector<std::size_t>& idx, std::size_t na) {
  auto dist = [&](std::size_t i, std::size_t j) {
    double d = 0;
    for (std::size_t c = 0; c < all[i].size(); ++c) d += (all[i][c] - all[j][c]) * (all[i][c] - all[j][c]);
    return std::sqrt(d);
  };
  const std::size_t nb = idx.size() - na;
  double ab = 0, aa = 0, bb = 0;
  for (std::size_t i = 0; i < na; ++i)
    for (std::size_t j = na; j < idx.size(); ++j) ab += dist(idx[i], idx[j]);
  for (std::size_t i = 0; i < na; ++i)
    for (std::size_t j = 0; j < na; ++j) aa += dist(idx[i], idx[j]);
  for (std::size_t i = na; i < idx.size(); ++i)
    for (std::size_t j = na; j < idx.size(); ++j) bb += dist(idx[i], idx[j]);
  return 2 * ab / static_cast<double>(na * nb) - aa / static_cast<double>(na * na) - bb / static_cast<double>(nb * nb);
}
}  // namespace detail

inline EnergyResult energy_distance(const Vectors& a, const Vectors& b, int permutations = 200, std::uint64_t seed = 1) {
  EnergyResult out;
  if (a.empty() || b.empty()) return out;
  const std::size_t dims = a.front().size();
  Vectors all = a;
  all.insert(all.end(), b.begin(), b.end());
  // Standardise each column by the pooled standard deviation so no quantity dominates.
  for (std::size_t c = 0; c < dims; ++c) {
    Sample col;
    for (const auto& row : all) col.push_back(row[c]);
    const double sd = std::sqrt(variance(col));
    if (sd > 0)
      for (auto& row : all) row[c] /= sd;
  }
  std::vector<std::size_t> idx(all.size());
  std::iota(idx.begin(), idx.end(), 0);
  out.statistic = detail::energy_stat(all, idx, a.size());
  int at_least = 0;
  std::uint64_t state = seed;
  for (int p = 0; p < permutations; ++p) {
    for (std::size_t i = idx.size() - 1; i > 0; --i) std::swap(idx[i], idx[detail::splitmix(state) % (i + 1)]);
    if (detail::energy_stat(all, idx, a.size()) >= out.statistic) ++at_least;
  }
  out.p_value = static_cast<double>(at_least + 1) / static_cast<double>(permutations + 1);
  out.reject_5pct = out.p_value < 0.05;
  return out;
}

// ---- Smaller checks.

// Share of `cold` values inside the [lo, hi] quantile band of `hot` (the old 90 % band smoke test).
inline double band_share(Sample hot, const Sample& cold, double lo = 0.05, double hi = 0.95) {
  if (hot.empty() || cold.empty()) return 0;
  std::sort(hot.begin(), hot.end());
  const double a = hot[static_cast<std::size_t>(lo * static_cast<double>(hot.size() - 1))];
  const double b = hot[static_cast<std::size_t>(hi * static_cast<double>(hot.size() - 1))];
  std::size_t in = 0;
  for (double x : cold) in += x >= a && x <= b;
  return static_cast<double>(in) / static_cast<double>(cold.size());
}

// Two proportions k1/n1 and k2/n2 agree within z pooled standard errors.
inline bool proportions_agree(std::uint64_t k1, std::uint64_t n1, std::uint64_t k2, std::uint64_t n2, double z = 3) {
  const double p1 = static_cast<double>(k1) / static_cast<double>(n1), p2 = static_cast<double>(k2) / static_cast<double>(n2);
  const double p = static_cast<double>(k1 + k2) / static_cast<double>(n1 + n2);
  const double se = std::sqrt(p * (1 - p) * (1.0 / static_cast<double>(n1) + 1.0 / static_cast<double>(n2)));
  return std::fabs(p1 - p2) <= z * std::max(se, 1e-9);
}

// Means agree within z standard errors (the old check, kept for the deterministic parts).
inline bool means_agree(const Sample& a, const Sample& b, double z = 4) {
  const double se = std::sqrt(variance(a) / static_cast<double>(a.size()) + variance(b) / static_cast<double>(b.size()));
  return std::fabs(mean(a) - mean(b)) <= z * std::max(se, 1e-12);
}

}  // namespace zs::jumps
