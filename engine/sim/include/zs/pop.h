// Pop identity, random-access life and cohort permutations. Spec: docs/tier0/specs/28-pop-levels.md
// Everything here is a pure function of its arguments: no counters, no state, so any pop can be
// evaluated at any time, in any order, without stepping the others.
#pragma once
#include <cstdint>

#include "zs/draw.h"

namespace zs {

using PopId = std::uint64_t;

// A stateless keyed draw: pure function of (world seed, purpose, a, b). Unlike Rng, it keeps no
// counter, so the order of calls never matters (spec 28, random-access life).
std::uint64_t keyed_u64(std::uint64_t world_seed, Purpose purpose, std::uint64_t a, std::uint64_t b = 0);
double keyed_uniform01(std::uint64_t world_seed, Purpose purpose, std::uint64_t a, std::uint64_t b = 0);  // [0, 1)

// Ids are 64 bits and never reused. Census people take their slot; the top bit is kept clear.
PopId census_pop_id(std::uint32_t slot);
// A birth: a hash of the household and the household's birth counter, so a lazy birth can be
// promoted later and always be the same child. The top bit is set, so it never meets a census id.
PopId birth_pop_id(std::uint64_t world_seed, std::uint64_t household, std::uint32_t birth_index);

enum class Sex : std::uint8_t { Female = 0, Male = 1 };

// What derive() knows about a pop from its id alone. Times are in years on the world calendar
// (year 0 is the start of the run; a pop alive at the start has a negative birth_year).
struct PopBase {
  PopId id = 0;
  Sex sex = Sex::Female;
  double birth_year = 0.0;
  double death_year = 0.0;  // when they die if nothing else kills them first (illness, violence, outbreak)
};

// Placeholder mortality: two independent hazards (a constant one for accidents and a Gompertz one
// for ageing), so the death age is closed-form and conditional on being alive at `known_alive_age`.
// Starting values, to be calibrated against a real life table (spec 28, Calibration). Not a model of
// infancy or of any illness.
struct Mortality {
  double constant_hazard;  // per year
  double gompertz_b;       // per year at age 0
  double gompertz_c;       // per year of age
};
Mortality mortality_for(Sex sex);

// `known_alive_age` is the age the pop is known to have reached: 0 for a birth, the age at the start
// for someone already alive. The death age is drawn conditional on it.
PopBase derive_base(std::uint64_t world_seed, PopId id, double birth_year, double known_alive_age = 0.0);

// A fixed pseudo-random permutation of 0..n-1 (a Feistel network with cycle walking), the "rank" of
// spec 28. forward(i) is member i's rank; inverse(r) is the member with rank r. No storage per member.
class CohortPermutation {
 public:
  CohortPermutation(std::uint64_t world_seed, std::uint64_t cohort_id, std::uint64_t n);
  std::uint64_t size() const { return n_; }
  std::uint64_t forward(std::uint64_t member) const;
  std::uint64_t inverse(std::uint64_t rank) const;
  // An aggregate event that hit `k` members: member i was hit iff its rank is below k.
  bool hit(std::uint64_t member, std::uint64_t k) const { return forward(member) < k; }

 private:
  std::uint64_t round(std::uint64_t half, int r) const;
  std::uint64_t feistel(std::uint64_t x, bool inverse) const;
  std::uint64_t seed_, cohort_, n_;
  int half_bits_;
};

}  // namespace zs
