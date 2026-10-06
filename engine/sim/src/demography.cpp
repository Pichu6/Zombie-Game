#include "zs/demography.h"

#include "zs/detmath.h"

namespace zs {

namespace {
// The `b` keys of Purpose::PopLife (pop.cpp uses 0..2 and the registry 10).
constexpr std::uint64_t kDrawChildless = 20, kDrawFirst = 21, kDrawGapBase = 100;

double exp_draw(std::uint64_t seed, PopId id, std::uint64_t key, double mean) {
  return -mean * detmath::log1p(-keyed_uniform01(seed, Purpose::PopLife, id, key));
}
}  // namespace

std::vector<Birth> births_of(std::uint64_t seed, PopId mother, const PopBase& base, double after_year, const PopParams& params) {
  const Fertility& f = params.fertility;
  std::vector<Birth> out;
  if (base.sex != Sex::Female) return out;
  if (keyed_uniform01(seed, Purpose::PopLife, mother, kDrawChildless) < f.childless_share) return out;
  double age = f.first_min_age + exp_draw(seed, mother, kDrawFirst, f.first_mean_delay);
  for (unsigned k = 0; k < kMaxBirths && age < f.end_age; ++k) {
    const double year = base.birth_year + age;
    if (year >= base.death_year) break;  // she has to be alive
    if (year > after_year) out.push_back({k, year});
    age += f.gap_min + exp_draw(seed, mother, kDrawGapBase + k, f.gap_mean_extra);
  }
  return out;
}

DemographyCounts jump(std::uint64_t seed, const std::vector<Founder>& founders, double from, double to,
                      const DeathOverrides* overrides, const PopParams& params) {
  DemographyCounts c;
  struct Node { PopId id; double birth_year, known_alive_age; std::optional<Sex> sex; };
  std::vector<Node> stack;
  stack.reserve(founders.size());
  for (const Founder& fo : founders) stack.push_back({fo.id, fo.birth_year, fo.known_alive_age, fo.sex});
  while (!stack.empty()) {
    const Node n = stack.back();
    stack.pop_back();
    PopBase base = derive_base(seed, n.id, n.birth_year, n.known_alive_age, params, n.sex);
    if (overrides) {
      const auto it = overrides->find(n.id);
      if (it != overrides->end()) base.death_year = it->second;
    }
    ++c.people_visited;
    const bool born_in_window = is_born(n.id) && base.birth_year > from && base.birth_year <= to;
    if (born_in_window) ++c.births;
    if (base.birth_year <= from && base.death_year > from) ++c.alive_at_start;
    if (base.death_year > from && base.death_year <= to && base.birth_year <= to) ++c.deaths;
    if (base.birth_year <= to && base.death_year > to) {
      ++c.alive_at_end;
      ++c.by_generation_alive[generation_of(n.id)];
    }
    // Founders' births before the epoch (year 0) already exist as other founders.
    const double after = is_born(n.id) ? -1e9 : 0.0;
    for (const Birth& b : births_of(seed, n.id, base, after, params)) {
      if (b.year > to) break;
      const PopId child = birth_pop_id(n.id, b.index);
      if (child == kNoPop) continue;  // the id format's limit: seven generations
      stack.push_back({child, b.year, 0.0, std::nullopt});
    }
  }
  return c;
}

}  // namespace zs
