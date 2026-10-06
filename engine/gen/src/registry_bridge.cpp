#include "zs/gen/registry_bridge.h"

#include <map>
#include <stdexcept>

namespace zs::gen {

TownRegistryMap populate_registry(const Town& town, zs::PopRegistry& reg, std::uint32_t region, std::uint64_t cohort_id,
                                  std::uint32_t first_slot) {
  if (town.pops.empty()) throw std::invalid_argument("populate_registry: the town has no people");
  TownRegistryMap map;
  map.cohort = cohort_id;
  map.first_slot = first_slot;
  if (!reg.add_cohort({cohort_id, region, first_slot, town.pops.size()}))
    throw std::invalid_argument("populate_registry: the cohort id or slots are taken");
  std::vector<FounderSpec> specs;
  specs.reserve(town.pops.size());
  for (const Pop& p : town.pops)
    specs.push_back({static_cast<std::uint16_t>(p.age < 0 ? 0 : p.age), p.sex == 'f' ? Sex::Female : Sex::Male});
  reg.set_founder_specs(cohort_id, std::move(specs));

  std::map<std::string, int> business_by_name;
  for (std::size_t i = 0; i < town.businesses.size(); ++i) {
    business_by_name.emplace(town.businesses[i].name, static_cast<int>(i));
    reg.add_roster(workplace_roster(i), region, zs::RosterKind::Workplace);
    reg.add_roster(school_roster(i), region, zs::RosterKind::School);
  }
  for (std::size_t h = 0; h < town.households.size(); ++h) reg.add_roster(household_roster(h), region, zs::RosterKind::Household);

  for (std::size_t i = 0; i < town.pops.size(); ++i) {
    const Pop& p = town.pops[i];
    const PopId id = census_pop_id(first_slot + static_cast<std::uint32_t>(i));
    map.pop_ids.push_back(id);
    int business = -1;
    if (!p.workplace.empty()) {
      const auto it = business_by_name.find(p.workplace);
      if (it == business_by_name.end()) ++map.unmatched_workplaces;
      else business = it->second;
    }
    map.business_of_pop.push_back(business);
    const bool student = business >= 0 && p.job_id.empty();  // a place at a school, not a job
    map.student.push_back(student);
    if (p.household >= 0 && static_cast<std::size_t>(p.household) < town.households.size())
      reg.roster_join(household_roster(static_cast<std::size_t>(p.household)), id, 0.0);
    if (business >= 0) {
      if (student) map.students += reg.roster_join(school_roster(static_cast<std::size_t>(business)), id, 0.0) ? 1 : 0;
      else map.employed += reg.roster_join(workplace_roster(static_cast<std::size_t>(business)), id, 0.0) ? 1 : 0;
    }
  }
  return map;
}

}  // namespace zs::gen
