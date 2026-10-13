// The officers of a town's police force as pops with shifts. Spec: docs/tier0/specs/65-police-staff.md
#include "zs/police_staff.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "toml.hpp"

namespace zs {
namespace {

constexpr std::int64_t kTicksPerMinute = kTicksPerGameHour / 60;
constexpr int kDaysAhead = 4;  // how far a search for the next shift or the next change looks

// A pop's key for its schedule: its id mixed with the world's seed (the officer's day is a function of this and the day).
std::uint64_t key_of(std::uint64_t seed, PopId pop) { return keyed_u64(seed, Purpose::PopLife, pop, 0x0F1CE5); }

std::int64_t midnight_of(std::int64_t start_day, std::int64_t day_number) {
  return (day_number - start_day) * kTicksPerGameDay - kStartTimeOfDayTicks;
}

}  // namespace

ForceParams load_force_params(const std::string& path, const std::string& country_profile, const std::string& income) {
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  std::string id = country_profile;
  if (id.empty() || !t[id].is_table()) {
    const auto named = t["index"][income].value<std::string>();
    if (!named) throw std::runtime_error(path + ": [index] has no table for " + income);
    id = *named;
  }
  const toml::table* c = t[id].as_table();
  if (!c) throw std::runtime_error(path + ": no table [" + id + "]");
  const auto per_search = (*c)["officers_per_search"].value<std::int64_t>();
  const auto minimum = (*c)["min_officers"].value<std::int64_t>();
  if (!per_search || *per_search < 1 || !minimum || *minimum < 1)
    throw std::runtime_error(path + ": [" + id + "] needs officers_per_search and min_officers of at least 1");
  ForceParams p;
  p.officers_per_search = static_cast<int>(*per_search);
  p.min_officers = static_cast<int>(*minimum);
  return p;
}

void PoliceStaff::add_officer(PopId pop, int age, char sex) {
  Officer o;
  o.pop = pop;
  o.person.key = key_of(seed_, pop);
  o.person.age = age;
  o.person.sex = sex;
  o.person.employment = Employment::FullTime;
  o.person.has_workplace = true;  // the station: a commute, a place to be at work
  static constexpr WorkPattern kPatterns[3] = {WorkPattern::Day, WorkPattern::Evening, WorkPattern::Night};
  o.pattern = kPatterns[officers_.size() % 3];
  officers_.push_back(o);
  ids_.push_back(pop);
}

const PoliceStaff::Officer* PoliceStaff::find(PopId pop) const {
  for (const Officer& o : officers_)
    if (o.pop == pop) return &o;
  return nullptr;
}

bool PoliceStaff::is_officer(PopId pop) const { return find(pop) != nullptr; }

// The officer's day: the schedule's own, on the pattern the force gave it, on the official sector's calendar (a public servant).
const DayPlan& PoliceStaff::plan_on(const Officer& o, std::int64_t day_number) const {
  const auto key = std::make_pair(o.pop, day_number);
  const auto hit = plans_.find(key);
  if (hit != plans_.end()) return hit->second;
  if (plans_.size() > officers_.size() * 16) {  // keep a few days only: drop what is older than the day asked for
    for (auto it = plans_.begin(); it != plans_.end();) it = it->first.second + 2 < day_number ? plans_.erase(it) : std::next(it);
  }
  ScheduleSkeleton skeleton = skeleton_of(*table_, o.person);
  skeleton.pattern = o.pattern;
  const CountryCalendar& cal = calendar_->calendar();
  return plans_
      .emplace(key, plan_day(*table_, o.person, skeleton, day_number, cal.survey_day(day_number, Sector::Official), cal.is_school_day(day_number) ? 1 : 0))
      .first->second;
}

bool PoliceStaff::on_duty(PopId pop, std::int64_t tick) const {
  const Officer* o = find(pop);
  if (!o) return false;
  const CivilTime now = civil_time(calendar_->start_day(), tick);
  return plan_on(*o, now.day_number).where(static_cast<int>(now.second_of_day / 60)) == PlaceKind::Workplace;
}

int PoliceStaff::on_duty_count(std::int64_t tick, const Expectations* exp) const {
  int n = 0;
  for (const Officer& o : officers_)
    if (on_duty(o.pop, tick) && !(exp && exp->find(o.pop))) ++n;
  return n;
}

std::int64_t PoliceStaff::next_change(std::int64_t now) const {
  const std::int64_t start_day = calendar_->start_day();
  const std::int64_t first_day = civil_time(start_day, now).day_number;
  std::int64_t best = System::kNever;
  for (const Officer& o : officers_)
    for (int k = 0; k < kDaysAhead; ++k) {
      const std::int64_t dn = first_day + k;
      const std::int64_t midnight = midnight_of(start_day, dn);
      const DayPlan plan = plan_on(o, dn);
      for (const Segment& s : plan.segments) {
        if (s.place != PlaceKind::Workplace) continue;
        for (const std::int64_t at : {midnight + s.start_min * kTicksPerMinute, midnight + s.end_min * kTicksPerMinute})
          if (at > now) best = std::min(best, at);
      }
    }
  return best;
}

std::optional<Commitment> PoliceStaff::next_commitment(PopId pop, std::int64_t from) const {
  const Officer* o = find(pop);
  if (!o) return std::nullopt;
  const std::int64_t start_day = calendar_->start_day();
  const std::int64_t first_day = civil_time(start_day, from).day_number;
  std::optional<Commitment> best;
  for (int k = 0; k < kDaysAhead; ++k) {
    const std::int64_t dn = first_day + k;
    const std::int64_t midnight = midnight_of(start_day, dn);
    for (const Segment& s : plan_on(*o, dn).segments) {
      if (s.activity != DayActivity::Work) continue;
      const std::int64_t due = midnight + s.start_min * kTicksPerMinute;
      if (due < from || (best && due >= best->due_tick)) continue;
      Commitment c;
      c.kind = CommitmentKind::ShiftStart;
      c.due_tick = due;
      c.employer = EmployerKind::Police;
      c.minutes = s.end_min - s.start_min;
      best = c;
    }
    if (best) break;  // the earliest day with a shift is enough
  }
  return best;
}

}  // namespace zs
