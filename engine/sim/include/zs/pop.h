// Pop identity, random-access life and cohort permutations. Spec: docs/tier0/specs/28-pop-levels.md
// Everything here is a pure function of its arguments: no counters, no state, so any pop can be
// evaluated at any time, in any order, without stepping the others.
#pragma once
#include <cstdint>

#include "zs/draw.h"
#include "zs/pop_params.h"

namespace zs {

using PopId = std::uint64_t;

// A stateless keyed draw: pure function of (world seed, purpose, a, b). Unlike Rng, it keeps no
// counter, so the order of calls never matters (spec 28, random-access life).
std::uint64_t keyed_u64(std::uint64_t world_seed, Purpose purpose, std::uint64_t a, std::uint64_t b = 0);
double keyed_uniform01(std::uint64_t world_seed, Purpose purpose, std::uint64_t a, std::uint64_t b = 0);  // [0, 1)

// Ids are 64 bits and never reused. A census person (a "founder") takes their slot as id. A born pop's
// id is invertible, so a lazy birth can always be found again: the top bit is set, then 3 bits of
// generation (1..7), 7 child numbers of 4 bits (the path from the founder), then the founder's slot.
// Child numbers are 0..14. Limits: 7 generations below a founder, 15 children per mother (a format
// version bit can widen this later).
constexpr PopId kNoPop = ~0ull;
PopId census_pop_id(std::uint32_t slot);
PopId birth_pop_id(PopId mother, unsigned child_index);  // kNoPop if the limits would be passed
bool is_born(PopId id);
unsigned generation_of(PopId id);            // 0 for a founder
PopId mother_of(PopId id);                   // kNoPop for a founder
unsigned child_index_of(PopId id);           // which child of the mother (born pops only)
std::uint32_t founder_slot_of(PopId id);

enum class Sex : std::uint8_t { Female = 0, Male = 1 };

// What derive() knows about a pop from its id alone. Times are in years on the world calendar
// (year 0 is the start of the run; a pop alive at the start has a negative birth_year).
struct PopBase {
  PopId id = 0;
  Sex sex = Sex::Female;
  double birth_year = 0.0;
  double death_year = 0.0;  // when they die if nothing else kills them first (illness, violence, outbreak)
};

// `known_alive_age` is the age the pop is known to have reached: 0 for a birth, the age at the start
// for someone already alive. The death age is drawn conditional on it.
// The rates come from the world's own parameters (stored in its save), never from compiled-in numbers.
PopBase derive_base(std::uint64_t world_seed, PopId id, double birth_year, double known_alive_age = 0.0,
                    const PopParams& params = default_pop_params());

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
