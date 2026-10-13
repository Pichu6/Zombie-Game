// The officers of a town's police force as pops with shifts (spec 65). Each officer works a pattern in turn (day, evening, night)
// from the schedule's own day, so on duty is where the plan says the officer is at work. The force's capacity at a tick is how
// many of its officers are on duty and not absent, divided by the officers a search takes. Not a system: the officers' days are
// functions of the pop, the day and the calendar, so there is nothing to save.
#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "zs/calendar.h"
#include "zs/expectations.h"
#include "zs/schedule.h"

namespace zs {

struct ForceParams {
  int officers_per_search = 2;
  int min_officers = 6;
};
// data/institutions/police/force.toml: the table `country_profile`, or the one the income group's [index] names.
ForceParams load_force_params(const std::string& path, const std::string& country_profile, const std::string& income);
// How many officers a town has: spec 69 (police_agency.h) from the size curve, at least min_officers.

class PoliceStaff {
 public:
  PoliceStaff(ForceParams params, std::shared_ptr<const TimeUseTable> table, const CalendarSystem* calendar, std::uint64_t world_seed)
      : params_(params), table_(std::move(table)), calendar_(calendar), seed_(world_seed) {}

  // Setup: adds an officer; the patterns go round (day, evening, night) in the order added.
  void add_officer(PopId pop, int age, char sex);
  const std::vector<PopId>& officers() const { return ids_; }
  bool is_officer(PopId pop) const;

  // Where the plan puts the officer at `tick`: at work.
  bool on_duty(PopId pop, std::int64_t tick) const;
  // Officers on duty and present: no open absence in `exp` (null: every officer is present).
  int on_duty_count(std::int64_t tick, const Expectations* exp) const;
  // Searches the force can run: officers on duty and present, over the officers one takes.
  int capacity(std::int64_t tick, const Expectations* exp) const { return on_duty_count(tick, exp) / params_.officers_per_search; }
  // The next tick after `now` at which some officer's on-duty status changes (a shift starts or ends).
  std::int64_t next_change(std::int64_t now) const;
  // An officer's next shift start at or after `from`, for the ladder and the payroll: a police employer, no holders at home.
  std::optional<Commitment> next_commitment(PopId pop, std::int64_t from) const;

 private:
  struct Officer {
    PopId pop = 0;
    SchedulePerson person;
    WorkPattern pattern = WorkPattern::Day;
  };
  const Officer* find(PopId pop) const;
  const DayPlan& plan_on(const Officer& o, std::int64_t day_number) const;
  ForceParams params_;
  std::shared_ptr<const TimeUseTable> table_;
  const CalendarSystem* calendar_;
  std::uint64_t seed_;
  std::vector<Officer> officers_;
  std::vector<PopId> ids_;
  // An officer's plan for a day is a pure function of the officer, the day and the calendar: remembered, never saved.
  mutable std::map<std::pair<PopId, std::int64_t>, DayPlan> plans_;
};

}  // namespace zs
