// Hazards: every "may happen" is a rate with a source. Spec: docs/tier0/specs/04-hazard-event.md
#pragma once
#include <optional>
#include <span>

#include "zs/draw.h"

namespace zs {

// Probability that a hazard with rate h (per second) fires within dt seconds: 1 - exp(-h dt).
double fire_probability(double rate_per_second, double dt_seconds);

// One hazard on one entity.
bool hazard_fires(Rng& rng, EntityId entity, double rate_per_second, double dt_seconds);

// Competing hazards on one entity (an exit set). At most one fires. One draw decides whether any
// fires (total rate), a second picks which, weighted by rate, walking the list in its fixed order.
// Returns the index of the hazard that fired.
std::optional<std::size_t> resolve_exit_set(Rng& rng, EntityId entity, std::span<const double> rates_per_second,
                                            double dt_seconds);

}  // namespace zs
