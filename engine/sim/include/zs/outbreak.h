// Outbreak across levels: one encounter rule (a zombie meets a person: caught by speed, bitten, the zombie
// destroyed by people who know), measured into rates that the region's compartments use, so the aggregate and
// the entity-level run of the same scenario agree in distribution. Keyed draws, no state beyond the counts.
// Spec: docs/tier0/specs/38-outbreak.md
#pragma once
#include <cstdint>
#include <vector>

namespace zs::outbreak {

// The entity-level rules: what the simulation already has about zombies and people.
struct Micro {
  double zombie_speed_mps = 1.0;   // a shambler about 1, a runner about 4
  double human_speed_mps = 2.5;    // a fleeing adult
  double warning = 0.0;            // 0..1: how much people know (keep distance, fight back)
  double bite_base = 0.6;          // bite chance once caught, unwarned
  double kill_skill = 0.5;         // chance a warned person destroys the zombie in an encounter
  double convert = 0.9;            // infection established per bite (spec 21 rule 1)
  double turn_days = 1.5;          // mean days from bite to reanimation (0014: 1-2)
  double contacts_per_day = 2.0;   // encounters per zombie per day in a populated place
};

enum class Outcome : std::uint8_t { Nothing = 0, Bitten = 1, Destroyed = 2, BittenAndDestroyed = 3 };

// One encounter between zombie `zombie` and a person, draw `k`: a pure keyed function.
Outcome encounter(const Micro& m, std::uint64_t seed, std::uint64_t zombie, std::uint32_t k);
// The chances the encounter rule implies (closed form, for the measurement to be checked against).
double catch_chance(const Micro& m);
double bite_chance(const Micro& m);
double kill_chance(const Micro& m);

// Rates measured from n keyed encounters: the only numbers the aggregate uses.
struct Rates {
  double bite;          // encounters that bite
  double destroyed;     // encounters that destroy the zombie
  double convert;       // bites that infect (from Micro, a per-bite rule)
  double contacts_per_day;
  // Expected infections per zombie over its active life: contacts x bite x convert, over destruction per contact
  // (a zombie nobody destroys keeps biting: capped at a 60-day horizon).
  double r0() const;
};
Rates measure(const Micro& m, std::uint64_t seed, std::uint32_t encounters);

// A region's compartments: susceptible, exposed (bitten, not yet turned), zombies, removed (destroyed zombies).
struct Compartments {
  std::int64_t s = 0, e = 0, z = 0, r = 0;
  std::int64_t people() const { return s + e; }
  std::int64_t infected_ever() const { return e + z + r; }
};

// Keyed binomial by inversion: a pure function of (seed, id, field, k, n, p).
std::int64_t keyed_binomial(std::uint64_t seed, std::uint64_t id, std::uint32_t field, std::uint32_t k, std::int64_t n, double p);

// The aggregate day (tau-leap): each zombie has contacts_per_day encounters with people; a susceptible one is bitten
// and infected with bite x convert; the zombie is destroyed with `destroyed`; each exposed turns with 1 / turn_days.
void aggregate_day(Compartments& c, const Rates& r, const Micro& m, std::uint64_t seed, std::uint64_t region, std::int64_t day);
void aggregate_days(Compartments& c, const Rates& r, const Micro& m, std::uint64_t seed, std::uint64_t region,
                    std::int64_t from_day, std::int64_t to_day);

// The entity level: every person and zombie an entity, every encounter drawn by the encounter rule.
enum class State : std::uint8_t { Susceptible, Exposed, Zombie, Destroyed };
struct Entity {
  State state = State::Susceptible;
  std::int64_t turns_on_day = -1;  // for an exposed one
};

class Entities {
 public:
  Entities(std::int64_t people, std::int64_t zombies);
  void run_day(const Micro& m, std::uint64_t seed, std::uint64_t region, std::int64_t day);
  Compartments counts() const;  // enumeration: a sum over the entities, never stored
  const std::vector<Entity>& all() const { return all_; }

 private:
  std::vector<Entity> all_;
  std::uint32_t draws_ = 0;
};

}  // namespace zs::outbreak
