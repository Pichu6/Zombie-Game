#include "zs/pop.h"

#include "zs/detmath.h"
#include "zs/philox.h"

namespace zs {

namespace {
constexpr std::uint64_t kTopBit = 1ull << 63;
constexpr int kFeistelRounds = 6;
// Which life draw (the `b` key of Purpose::PopLife).
constexpr std::uint64_t kDrawSex = 0, kDrawConstantHazard = 1, kDrawGompertz = 2;
constexpr double kMaleShareAtBirth = 0.512;  // about 105 boys per 100 girls; the source is to be cited in the calibration pass
}  // namespace

std::uint64_t keyed_u64(std::uint64_t world_seed, Purpose purpose, std::uint64_t a, std::uint64_t b) {
  const auto purpose_id = static_cast<std::uint32_t>(purpose);
  const PhiloxCounter c = {static_cast<std::uint32_t>(a), static_cast<std::uint32_t>(a >> 32),
                           static_cast<std::uint32_t>(b), static_cast<std::uint32_t>(b >> 32)};
  const PhiloxKey k = {static_cast<std::uint32_t>(world_seed) ^ purpose_id * 0x9E3779B9u,
                       static_cast<std::uint32_t>(world_seed >> 32)};
  const PhiloxCounter out = philox4x32_10(c, k);
  return (static_cast<std::uint64_t>(out[0]) << 32) | out[1];
}

double keyed_uniform01(std::uint64_t world_seed, Purpose purpose, std::uint64_t a, std::uint64_t b) {
  return static_cast<double>(keyed_u64(world_seed, purpose, a, b) >> 11) * (1.0 / 9007199254740992.0);
}

PopId census_pop_id(std::uint32_t slot) { return slot; }

PopId birth_pop_id(std::uint64_t world_seed, std::uint64_t household, std::uint32_t birth_index) {
  return keyed_u64(world_seed, Purpose::PopIdMix, household, birth_index) | kTopBit;
}

Mortality mortality_for(Sex sex) {
  // About 81 years for women and 75 for men, to be replaced by a fit to real life tables.
  return sex == Sex::Female ? Mortality{3.0e-4, 1.5e-5, 0.10} : Mortality{3.0e-4, 3.0e-5, 0.10};
}

PopBase derive_base(std::uint64_t world_seed, PopId id, double birth_year, double known_alive_age) {
  PopBase p;
  p.id = id;
  p.birth_year = birth_year;
  p.sex = keyed_uniform01(world_seed, Purpose::PopLife, id, kDrawSex) < kMaleShareAtBirth ? Sex::Male : Sex::Female;
  const Mortality m = mortality_for(p.sex);
  const double e1 = -detmath::log1p(-keyed_uniform01(world_seed, Purpose::PopLife, id, kDrawConstantHazard));
  const double e2 = -detmath::log1p(-keyed_uniform01(world_seed, Purpose::PopLife, id, kDrawGompertz));
  // First death of two independent hazards, each drawn conditional on having reached known_alive_age.
  const double age_constant = known_alive_age + e1 / m.constant_hazard;
  const double age_gompertz =
      detmath::log(detmath::exp(m.gompertz_c * known_alive_age) + m.gompertz_c * e2 / m.gompertz_b) / m.gompertz_c;
  p.death_year = birth_year + (age_constant < age_gompertz ? age_constant : age_gompertz);
  return p;
}

CohortPermutation::CohortPermutation(std::uint64_t world_seed, std::uint64_t cohort_id, std::uint64_t n)
    : seed_(world_seed), cohort_(cohort_id), n_(n), half_bits_(1) {
  // The smallest even number of bits that holds 0..n-1, split into two halves.
  while ((1ull << (2 * half_bits_)) < n_ && half_bits_ < 32) ++half_bits_;
}

std::uint64_t CohortPermutation::round(std::uint64_t half, int r) const {
  const std::uint64_t mask = (1ull << half_bits_) - 1;
  return keyed_u64(seed_, Purpose::PopRank, cohort_, (static_cast<std::uint64_t>(r) << 56) ^ half) & mask;
}

std::uint64_t CohortPermutation::feistel(std::uint64_t x, bool inverse) const {
  const std::uint64_t mask = (1ull << half_bits_) - 1;
  std::uint64_t left = (x >> half_bits_) & mask, right = x & mask;
  if (!inverse) {
    for (int r = 0; r < kFeistelRounds; ++r) {
      const std::uint64_t next = left ^ round(right, r);
      left = right;
      right = next;
    }
  } else {
    for (int r = kFeistelRounds - 1; r >= 0; --r) {
      const std::uint64_t prev = right ^ round(left, r);
      right = left;
      left = prev;
    }
  }
  return (left << half_bits_) | right;
}

std::uint64_t CohortPermutation::forward(std::uint64_t member) const {
  if (n_ <= 1) return 0;
  std::uint64_t x = member;
  do { x = feistel(x, false); } while (x >= n_);  // cycle walking keeps it inside 0..n-1
  return x;
}

std::uint64_t CohortPermutation::inverse(std::uint64_t rank) const {
  if (n_ <= 1) return 0;
  std::uint64_t x = rank;
  do { x = feistel(x, true); } while (x >= n_);
  return x;
}

}  // namespace zs
