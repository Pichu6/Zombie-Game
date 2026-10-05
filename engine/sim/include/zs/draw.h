// Keyed randomness. Spec: docs/tier0/specs/02-draw.md
// A draw is a pure function of (world seed, entity, purpose, counter). Each (entity, purpose)
// stream keeps its own counter, so drawing for one purpose never shifts another.
#pragma once
#include <cstdint>
#include <map>
#include <span>
#include <utility>

namespace zs {

using EntityId = std::uint64_t;

// Fixed list of purposes. Append only: never renumber, or old saves draw differently.
enum class Purpose : std::uint32_t {
  Test = 0,
  HazardFire = 1,
  HazardChoice = 2,
  WoundInfection = 3,
  BiteHit = 4,
  CensusFate = 5,
};

class Rng {
 public:
  explicit Rng(std::uint64_t world_seed = 0) : seed_(world_seed) {}

  std::uint64_t seed() const { return seed_; }

  // Raw 64 random bits from the next counter value of this stream.
  std::uint64_t next_u64(EntityId entity, Purpose purpose);

  double uniform01(EntityId e, Purpose p);                         // [0, 1), 53 bits
  std::uint64_t uniform_int(EntityId e, Purpose p, std::uint64_t n);  // [0, n), unbiased
  bool bernoulli(EntityId e, Purpose p, double probability);
  double exponential(EntityId e, Purpose p, double rate);           // mean 1 / rate
  double normal(EntityId e, Purpose p, double mean, double stddev);  // Box-Muller
  std::size_t weighted_choice(EntityId e, Purpose p, std::span<const double> weights);

  // Counters are part of the saved state.
  const std::map<std::pair<EntityId, std::uint32_t>, std::uint64_t>& counters() const { return counters_; }
  void set_counter(EntityId e, std::uint32_t purpose, std::uint64_t value) { counters_[{e, purpose}] = value; }

 private:
  std::uint64_t seed_;
  std::map<std::pair<EntityId, std::uint32_t>, std::uint64_t> counters_;
};

}  // namespace zs
