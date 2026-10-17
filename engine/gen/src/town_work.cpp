// Spec 99: the pops in the town's workplaces.
#include "zs/gen/town_work.h"

#include <algorithm>

#include "zs/gen/keyed.h"

namespace zs::gen {

zs::WorkTable place_in_work(const TownEconomy& economy, const Catalog& cat, const TownLayout& layout, const TownTravel& travel,
                            const std::vector<WorkRow>& rows, std::size_t census_size, std::uint64_t seed, const SchoolAges& school) {
  zs::WorkTable out(rows.size());
  const Keyed rnd(seed ^ hash_str("work"));
  // The jobs in the catalog's order with their counts in the town (rule 1's weights).
  std::vector<double> weight;
  for (const Job& j : cat.jobs) {
    const auto it = economy.jobs.find(j.id);
    weight.push_back(it == economy.jobs.end() ? 0.0 : it->second);
  }
  auto minutes = [&](int from, int to) { return from >= 0 && to >= 0 ? static_cast<int>(plot_minutes(layout, from, to, travel)) : -1; };
  auto school_of = [](const std::string& type) { return type.size() > 7 && type.compare(type.size() - 7, 7, "_school") == 0; };
  for (std::size_t i = 0; i < rows.size(); ++i) {
    const WorkRow& r = rows[i];
    zs::PopWork& w = out[i];
    if (!r.alive) continue;
    const int home = home_plot(layout, r.pop, census_size, seed);
    if (r.age < 18) {
      // Rule 3 (amended): the nearest school of the pupil's stage by the country's school ages; before school or past it,
      // at home.
      const std::string stage = school_stage(school, r.age);
      if (stage.empty()) continue;
      if (rnd.chance(r.pop, 2, out_of_school(school, r.age))) {  // second amendment: out of school by the country's rate
        w.workplace = "out of school";
        continue;
      }
      const TownWorkplace* best = nullptr;
      int best_min = 0;
      for (const TownWorkplace& x : economy.workplaces) {
        if (x.type != stage) continue;
        const int m = minutes(home, x.plot);
        if (!best || (m >= 0 && (best_min < 0 || m < best_min))) best = &x, best_min = m;
      }
      w.school = w.placed = true;
      if (best) w.workplace = best->name, w.address = best->address, w.commute_min = best_min;
      else w.workplace = "a school in a larger town nearby";
      continue;
    }
    if (r.age < 18 || r.age >= 65) continue;  // spec 54's workers are 18 to 64
    // Rule 1: a job by the town's jobs, then a workplace of its kind by the workplaces' staff.
    const Job& job = cat.jobs[rnd.weighted(r.pop, 0, weight)];
    w.job = job.label;
    w.placed = true;
    if (job.workplace.empty()) {  // rule 2: works around the town (a building site, the fields, a route)
      w.workplace = "around the town";
      continue;
    }
    std::vector<const TownWorkplace*> kind;
    std::vector<double> staff;
    for (const TownWorkplace& x : economy.workplaces)
      if (x.type == job.workplace || (job.workplace == "school" && school_of(x.type)) ||
          (job.workplace == "factory" && x.type == "plant"))
        kind.push_back(&x), staff.push_back(x.staff);
    if (kind.empty()) {  // the town has none of its kind: it works in a larger town nearby
      w.workplace = "a workplace in a larger town nearby";
      continue;
    }
    const TownWorkplace& x = *kind[rnd.weighted(r.pop, 1, staff)];
    w.workplace = x.name;
    w.address = x.address;
    w.commute_min = minutes(home, x.plot);
  }
  return out;
}

}  // namespace zs::gen
