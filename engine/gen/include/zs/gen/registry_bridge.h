// The bridge from the content generators to the pop registry (spec 28): a generated town becomes founders
// with their generated age and sex, a roster per household and a roster per business.
// Lives in the generator library because it needs both the town and the registry.
#pragma once
#include <cstdint>
#include <vector>

#include "zs/gen/pops.h"
#include "zs/pop_registry.h"

namespace zs::gen {

// Roster ids: the kind in the high half, the index in the low half. A town's households and businesses
// never share an id, and the ids say what they are.
inline constexpr std::uint64_t kHouseholdRosterKind = 1ull << 32;
inline constexpr std::uint64_t kWorkplaceRosterKind = 2ull << 32;
inline constexpr std::uint64_t kSchoolRosterKind = 3ull << 32;
inline std::uint64_t household_roster(std::size_t household_index) { return kHouseholdRosterKind | household_index; }
// Staff of a business (RosterKind::Workplace) and the pupils of a school (RosterKind::School) are different rosters:
// a layoff aimed at workplaces never touches a child.
inline std::uint64_t workplace_roster(std::size_t business_index) { return kWorkplaceRosterKind | business_index; }
inline std::uint64_t school_roster(std::size_t business_index) { return kSchoolRosterKind | business_index; }

struct TownRegistryMap {
  std::uint64_t cohort = 0;
  std::uint32_t first_slot = 0;
  std::vector<PopId> pop_ids;            // registry id of Town::pops[i]
  std::vector<int> business_of_pop;      // index into Town::businesses, or -1 (no workplace, or a name that matches none)
  std::vector<bool> student;             // true: the pop is on a school roster, not a workplace one
  std::size_t unmatched_workplaces = 0;  // pops who named a workplace the town's directory does not list
  std::size_t employed = 0;              // pops on a workplace roster (they have a job)
  std::size_t students = 0;              // pops on a school roster
};

// Adds the town to `reg` as one cohort of founders in `region` plus its rosters, everyone joining at year 0.
// Pops take slots first_slot, first_slot + 1, ... in the town's order.
TownRegistryMap populate_registry(const Town& town, zs::PopRegistry& reg, std::uint32_t region, std::uint64_t cohort_id,
                                  std::uint32_t first_slot = 0);

}  // namespace zs::gen
