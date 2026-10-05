#include "zs/hazard.h"

#include "zs/detmath.h"

namespace zs {

double fire_probability(double rate_per_second, double dt_seconds) {
  if (rate_per_second <= 0.0 || dt_seconds <= 0.0) return 0.0;
  return -detmath::expm1(-rate_per_second * dt_seconds);
}

bool hazard_fires(Rng& rng, EntityId entity, double rate_per_second, double dt_seconds) {
  const double p = fire_probability(rate_per_second, dt_seconds);
  if (p <= 0.0) return false;  // a zeroed hazard draws nothing and never fires
  return rng.uniform01(entity, Purpose::HazardFire) < p;
}

std::optional<std::size_t> resolve_exit_set(Rng& rng, EntityId entity, std::span<const double> rates_per_second,
                                            double dt_seconds) {
  double total = 0.0;
  for (double h : rates_per_second)
    if (h > 0.0) total += h;
  if (!hazard_fires(rng, entity, total, dt_seconds)) return std::nullopt;
  double x = rng.uniform01(entity, Purpose::HazardChoice) * total;
  std::size_t last_positive = 0;
  for (std::size_t i = 0; i < rates_per_second.size(); ++i) {
    const double h = rates_per_second[i];
    if (h <= 0.0) continue;
    last_positive = i;
    if (x < h) return i;
    x -= h;
  }
  return last_positive;  // rounding fallback
}

}  // namespace zs
