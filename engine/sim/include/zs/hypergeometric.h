// An exact, deterministic hypergeometric draw: how many of `good` marked items land in `draws` items taken
// without replacement from `population`. Used to split a region-wide count down to its groups (spec 28).
#pragma once
#include <cstdint>

namespace zs {

// u is a uniform in [0, 1); the result is a pure function of the four arguments. Requires good <= population
// and draws <= population. Inversion in mode-outward order (any fixed order is a valid inversion), so the
// work is about the spread of the distribution, not the sizes.
std::uint64_t hypergeometric(double u, std::uint64_t population, std::uint64_t good, std::uint64_t draws);

// ln(n!) from a table for small n and Stirling's series above, with our own log (deterministic).
double log_factorial(std::uint64_t n);

}  // namespace zs
