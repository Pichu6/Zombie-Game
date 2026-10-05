#include "zs/draw.h"

#include "zs/detmath.h"
#include "zs/philox.h"

namespace zs {

std::uint64_t Rng::next_u64(EntityId entity, Purpose purpose) {
  const auto purpose_id = static_cast<std::uint32_t>(purpose);
  std::uint64_t& counter = counters_[{entity, purpose_id}];
  const std::uint64_t n = counter++;
  const PhiloxCounter c = {static_cast<std::uint32_t>(n), static_cast<std::uint32_t>(n >> 32),
                           static_cast<std::uint32_t>(entity), static_cast<std::uint32_t>(entity >> 32)};
  const PhiloxKey k = {static_cast<std::uint32_t>(seed_) ^ purpose_id * 0x9E3779B9u,
                       static_cast<std::uint32_t>(seed_ >> 32)};
  const PhiloxCounter out = philox4x32_10(c, k);
  return (static_cast<std::uint64_t>(out[0]) << 32) | out[1];
}

double Rng::uniform01(EntityId e, Purpose p) {
  return static_cast<double>(next_u64(e, p) >> 11) * (1.0 / 9007199254740992.0);  // 2^-53
}

std::uint64_t Rng::uniform_int(EntityId e, Purpose p, std::uint64_t n) {
  if (n <= 1) return 0;
  const std::uint64_t limit = UINT64_MAX - (UINT64_MAX % n);  // reject the biased tail
  std::uint64_t x;
  do { x = next_u64(e, p); } while (x >= limit);
  return x % n;
}

bool Rng::bernoulli(EntityId e, Purpose p, double probability) {
  return uniform01(e, p) < probability;
}

double Rng::exponential(EntityId e, Purpose p, double rate) {
  return -detmath::log1p(-uniform01(e, p)) / rate;
}

double Rng::normal(EntityId e, Purpose p, double mean, double stddev) {
  const double u1 = 1.0 - uniform01(e, p);  // (0, 1]
  const double u2 = uniform01(e, p);
  const double z = detmath::sqrt(-2.0 * detmath::log(u1)) * detmath::cos(2.0 * detmath::pi * u2);
  return mean + stddev * z;
}

std::size_t Rng::weighted_choice(EntityId e, Purpose p, std::span<const double> weights) {
  double total = 0.0;
  for (double w : weights) total += w;
  double x = uniform01(e, p) * total;
  for (std::size_t i = 0; i < weights.size(); ++i) {
    if (x < weights[i]) return i;
    x -= weights[i];
  }
  return weights.empty() ? 0 : weights.size() - 1;
}

}  // namespace zs
