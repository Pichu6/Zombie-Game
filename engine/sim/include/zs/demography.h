// Births and the jump form of life events. Spec: docs/tier0/specs/28-pop-levels.md (P5)
// A woman's births are a pure function of her id (random-access life, like her death), so a pop's whole
// descendant tree can be worked out at any time without stepping anyone. Marriage and job moves are not
// here: they need households and businesses and come with those systems.
#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "zs/pop.h"

namespace zs {

inline constexpr unsigned kMaxBirths = 15;

struct Birth {
  unsigned index;  // child number (0-based), part of the child's id
  double year;
};

// The births of one mother after `after_year` (births before it already exist as other pops), while she
// lives. Female pops only; a male pop has none.
std::vector<Birth> births_of(std::uint64_t world_seed, PopId mother, const PopBase& mother_base, double after_year,
                             const PopParams& params = default_pop_params());

// A founder: someone alive at the start of the run (a census person).
struct Founder {
  PopId id;
  double birth_year;
  double known_alive_age;
  std::optional<PopSex> sex = std::nullopt;  // generated, or drawn from the id when empty
};

// Overrides of derived death years (what happened to someone that the seed could not say).
using DeathOverrides = std::map<PopId, double>;

struct DemographyCounts {
  std::uint64_t alive_at_start = 0;  // alive at `from`
  std::uint64_t births = 0;          // born in (from, to]
  std::uint64_t deaths = 0;          // died in (from, to]
  std::uint64_t alive_at_end = 0;    // alive at `to`
  std::uint64_t people_visited = 0;  // the cost: everyone ever in the tree up to `to`
  std::uint64_t by_generation_alive[8] = {};  // alive at `to`, by generation below the founders
};

// The jump: everyone in the founders' trees from the start of the run to `to`, counted over (from, to].
// `from` must not be before the founders' epoch (year 0). Pure: the same inputs give the same counts.
DemographyCounts jump(std::uint64_t world_seed, const std::vector<Founder>& founders, double from, double to,
                      const DeathOverrides* overrides = nullptr, const PopParams& params = default_pop_params());

}  // namespace zs
