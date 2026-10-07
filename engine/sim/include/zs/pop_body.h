// The body a pop gets when it is embodied for the first time. Spec: docs/tier0/specs/29-embodied-pops.md (rule 2)
// A pure function of (world seed, pop id, age, sex): no counter, so a pop embodied twice from the same record, in any
// order, gets the same body.
#pragma once
#include <cstdint>

#include "zs/body.h"
#include "zs/pop.h"

namespace zs {

inline Sex body_sex(PopSex s) { return s == PopSex::Male ? Sex::Male : Sex::Female; }

// Same distributions as the Tier 0 census (NHANES-like adult heights and BMI, Deurenberg body fat, VO2max by age);
// placeholders until fitted to a nation's own profile. `age` is in years. The body is a living person, not a walker.
BodySpec body_spec_for(std::uint64_t world_seed, PopId id, double age, PopSex sex);

}  // namespace zs
