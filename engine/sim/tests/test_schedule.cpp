// Spec 54: daily schedules as a function of the pop, the day and the country's time-use table.
#include <doctest.h>

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "zs/calendar.h"
#include "zs/clock.h"
#include "zs/schedule.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

TimeUseTable usa() { return load_timeuse(tier0::data_dir() + "/timeuse/USA.toml"); }

SchedulePerson person(std::uint64_t key, Employment e, char sex = 'm', int age = 35) {
  SchedulePerson p;
  p.key = key;
  p.employment = e;
  p.sex = sex;
  p.age = age;
  return p;
}

constexpr std::int64_t kMonday = days_from_civil(2027, 1, 11);  // a weekday

}  // namespace

TEST_CASE("a plan covers the day exactly, in order, for every employment and day type (A1)") {
  const TimeUseTable t = usa();
  for (int e = 0; e < 6; ++e)
    for (int d = 0; d < 3; ++d)
      for (std::uint64_t k = 1; k <= 60; ++k) {
        const SchedulePerson p = person(k * 7919, static_cast<Employment>(e));
        const DayPlan plan = plan_day(t, p, skeleton_of(t, p), kMonday + d, static_cast<SurveyDay>(d));
        REQUIRE(!plan.segments.empty());
        CHECK(plan.segments.front().start_min == 0);
        CHECK(plan.segments.back().end_min == 1440);
        for (std::size_t i = 0; i < plan.segments.size(); ++i) {
          CHECK(plan.segments[i].end_min > plan.segments[i].start_min);
          if (i) CHECK(plan.segments[i].start_min == plan.segments[i - 1].end_min);
        }
        if (e == static_cast<int>(Employment::Child) || e == static_cast<int>(Employment::Retired) || e == static_cast<int>(Employment::NotEmployed))
          CHECK_FALSE(plan.works);
        if (e != static_cast<int>(Employment::Student)) CHECK_FALSE(plan.studies);
      }
}

TEST_CASE("the same key and day give the same plan; another key differs (A1)") {
  const TimeUseTable t = usa();
  const SchedulePerson a = person(42, Employment::FullTime), b = person(43, Employment::FullTime);
  const DayPlan p1 = plan_day(t, a, skeleton_of(t, a), kMonday, SurveyDay::Weekday);
  const DayPlan p2 = plan_day(t, a, skeleton_of(t, a), kMonday, SurveyDay::Weekday);
  REQUIRE(p1.segments.size() == p2.segments.size());
  for (std::size_t i = 0; i < p1.segments.size(); ++i) {
    CHECK(p1.segments[i].start_min == p2.segments[i].start_min);
    CHECK(p1.segments[i].activity == p2.segments[i].activity);
  }
  int different = 0;
  for (std::uint64_t k = 1; k <= 50; ++k) {
    const SchedulePerson x = person(k, Employment::FullTime), y = person(k + 1000, Employment::FullTime);
    const DayPlan px = plan_day(t, x, skeleton_of(t, x), kMonday, SurveyDay::Weekday), py = plan_day(t, y, skeleton_of(t, y), kMonday, SurveyDay::Weekday);
    if (px.segments.size() != py.segments.size() || px.work_min != py.work_min || px.sleep_min != py.sleep_min) ++different;
  }
  CHECK(different > 40);
  (void)b;
}

TEST_CASE("participation and hours when done reproduce the USA rows (A1)") {
  const TimeUseTable t = usa();
  struct Row { Employment e; SurveyDay d; double part, hours; };
  const Row rows[] = {{Employment::FullTime, SurveyDay::Weekday, 0.873, 8.45},
                      {Employment::FullTime, SurveyDay::SundayHoliday, 0.296, 5.49},
                      {Employment::PartTime, SurveyDay::Weekday, 0.543, 5.42}};
  for (const Row& r : rows) {
    int did = 0;
    double hours = 0;
    const int n = 20000;
    for (int i = 0; i < n; ++i) {
      const SchedulePerson p = person(1000 + static_cast<std::uint64_t>(i), r.e);
      const DayPlan plan = plan_day(t, p, skeleton_of(t, p), kMonday + i % 5, r.d);
      if (plan.works) ++did, hours += plan.work_min / 60.0;
    }
    const double part = static_cast<double>(did) / n;
    CHECK(std::abs(part - r.part) < 0.02);
    CHECK(std::abs(hours / did - r.hours) / r.hours < 0.05);
  }
  // Social: 28.4 % on weekdays, 35.2 % on weekends.
  int social_wd = 0, social_we = 0;
  for (int i = 0; i < 20000; ++i) {
    const SchedulePerson p = person(5000 + static_cast<std::uint64_t>(i), Employment::FullTime);
    const ScheduleSkeleton s = skeleton_of(t, p);
    for (const Segment& x : plan_day(t, p, s, kMonday, SurveyDay::Weekday).segments)
      if (x.activity == DayActivity::Social) { ++social_wd; break; }
    for (const Segment& x : plan_day(t, p, s, kMonday + 6, SurveyDay::SundayHoliday).segments)
      if (x.activity == DayActivity::Social) { ++social_we; break; }
  }
  CHECK(std::abs(social_wd / 20000.0 - 0.284) < 0.025);
  CHECK(std::abs(social_we / 20000.0 - 0.352) < 0.025);
}

TEST_CASE("weekly hours of full-time pops land near the ILOSTAT targets (A1)") {
  // USA: 5 weekdays + 2 weekend days of the table give about 35.6 h against 37.80 (rule 7).
  const TimeUseTable usa_t = usa();
  const TimeUseTable lm = timeuse_for(tier0::data_dir(), "", "lower_middle");  // IND, scaled to 43.91
  // ILOSTAT's mean is per employed person, so the sexes are weighted by the table's share of women among the employed.
  auto weekly = [](const TimeUseTable& t, int n) {
    double total[2] = {0, 0};
    for (int i = 0; i < n; ++i) {
      const char sex = i % 2 ? 'f' : 'm';
      const SchedulePerson p = person(9000 + static_cast<std::uint64_t>(i), Employment::FullTime, sex);
      const ScheduleSkeleton s = skeleton_of(t, p);
      for (int d = 0; d < 7; ++d) {
        const SurveyDay day = d < 5 ? SurveyDay::Weekday : d == 5 ? SurveyDay::Saturday : SurveyDay::SundayHoliday;
        total[sex == 'f'] += plan_day(t, p, s, kMonday + d, day).work_min / 60.0;
      }
    }
    const double men = total[0] / (n / 2), women = total[1] / (n / 2);
    return (1 - t.women_share_of_employed) * men + t.women_share_of_employed * women;
  };
  CHECK(std::abs(weekly(usa_t, 4000) - 37.80) / 37.80 < 0.10);
  CHECK(std::abs(weekly(lm, 4000) - 43.91) / 43.91 < 0.10);
}

TEST_CASE("day workers leave between 06:00 and 08:29 about 57 % of the time; sleep varies night to night by about 70 min (A1)") {
  const TimeUseTable t = usa();
  int in_window = 0, day_workers = 0;
  double sum = 0, sum2 = 0;
  int nights = 0;
  for (int i = 0; i < 5000; ++i) {
    const SchedulePerson p = person(20000 + static_cast<std::uint64_t>(i), Employment::FullTime);
    const ScheduleSkeleton s = skeleton_of(t, p);
    if (s.pattern == WorkPattern::Day) {
      ++day_workers;
      if (s.departure_min >= 6 * 60 && s.departure_min < 8 * 60 + 30) ++in_window;
    }
  }
  CHECK(std::abs(static_cast<double>(in_window) / day_workers - 0.57) < 0.05);
  const SchedulePerson p = person(77, Employment::Retired);
  const ScheduleSkeleton s = skeleton_of(t, p);
  for (int d = 0; d < 2000; ++d) {
    const double m = plan_day(t, p, s, kMonday + d, SurveyDay::Weekday).sleep_min;
    sum += m, sum2 += m * m, ++nights;
  }
  const double mean = sum / nights, sd = std::sqrt(sum2 / nights - mean * mean);
  CHECK(sd > 55);
  CHECK(sd < 85);
}

TEST_CASE("where a pop is at a tick on the one world's calendar (A1)") {
  const TimeUseTable t = usa();
  const CountryCalendar cal = load_country_calendar(tier0::data_dir() + "/setup/calendar/USA.toml");
  int at_work = 0, at_home_night = 0, worked = 0;
  for (int i = 0; i < 400; ++i) {
    SchedulePerson p = person(30000 + static_cast<std::uint64_t>(i), Employment::FullTime);
    const ScheduleSkeleton s = skeleton_of(t, p);
    if (s.pattern != WorkPattern::Day) continue;
    // Monday 11 January 2027 at 10:00 is tick (2 h) * kTicksPerGameHour after the 08:00 start; 03:00 is day 0 - 5 h: use day 1.
    const std::int64_t t10 = 2 * kTicksPerGameHour, t03 = (24 - 8 + 3) * kTicksPerGameHour;
    const DayPlan plan = plan_for_tick(t, cal, kDefaultStartDay, p, t10);
    if (plan.works) {
      ++worked;
      if (where_at(t, cal, kDefaultStartDay, p, t10) == PlaceKind::Workplace) ++at_work;
    }
    if (where_at(t, cal, kDefaultStartDay, p, t03) == PlaceKind::Home) ++at_home_night;
  }
  CHECK(worked > 200);
  CHECK(at_work > worked * 0.80);  // a late departure (the shares run to 12:30) or a long commute keeps some on the road at 10:00
  CHECK(at_home_night > 300);
  // Students study on school days only: 11 January 2027 is a school day in the US calendar, Sunday 17 is not.
  int monday_study = 0, sunday_study = 0;
  for (int i = 0; i < 200; ++i) {
    const SchedulePerson st = person(555 + static_cast<std::uint64_t>(i), Employment::Student, 'f', 15);
    if (plan_for_tick(t, cal, kDefaultStartDay, st, 2 * kTicksPerGameHour).studies) ++monday_study;
    const DayPlan sunday = plan_for_tick(t, cal, kDefaultStartDay, st, 6 * kTicksPerGameDay + 2 * kTicksPerGameHour);
    CHECK(sunday.day == SurveyDay::SundayHoliday);
    if (sunday.studies) ++sunday_study;
  }
  CHECK(monday_study > 140);  // 80 % participation on a school day
  CHECK(sunday_study < 60);   // 15 % on a Sunday
}
