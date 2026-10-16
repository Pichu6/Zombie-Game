// Daily schedules (spec 54): where any pop is at any time, as a function of the pop, the day and its country's
// time-use table. A day is a participation draw, then a duration draw (decision 0071), on a fixed skeleton drawn once
// per pop (the job's pattern, the departure and sleep anchors) plus daily noise. Pure: the same key gives the same day.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "zs/calendar.h"

namespace zs {

enum class Employment : std::uint8_t { FullTime = 0, PartTime = 1, NotEmployed = 2, Student = 3, Retired = 4, Child = 5 };
enum class DayActivity : std::uint8_t { Sleep = 0, SelfCare, Commute, Work, Study, Household, Shop, Care, Social, Civic, Leisure, kCount };
enum class PlaceKind : std::uint8_t { Home = 0, Workplace = 1, School = 2, Out = 3, Travelling = 4 };
enum class WorkPattern : std::uint8_t { Day = 0, Evening = 1, Night = 2, Rotating = 3 };

const char* employment_name(Employment e);
const char* activity_name(DayActivity a);
const char* place_kind_name(PlaceKind p);
Employment employment_from_occupation(const std::string& occupation, int age);  // the generator's words (spec 47)

// One row of a time-use table: who it is for (empty = everyone), the day type, participation and hours when done.
struct TimeUseRow {
  DayActivity activity = DayActivity::Work;
  int employment = -1;  // -1 = any
  char sex = 0;         // 0 = any
  int day = -1;         // SurveyDay, -1 = any
  double participation = 0, hours_if_done = 0;
};

struct TimeUseTable {
  std::string id, source, method;
  double weekly_hours_target = 0;  // ILOSTAT, for calibration (rule 7)
  double women_share_of_employed = 0.47;  // weights the two sexes in the calibration (ILOSTAT's mean is per employed person)
  std::vector<TimeUseRow> rows;
  double sleep_weekday_h = 8.76, sleep_weekend_h = 9.68, sleep_sd_min = 70, bedtime_mean_min = 23 * 60, bedtime_sd_min = 45;
  double pattern_day = 0.85, pattern_evening = 0.05, pattern_night = 0.03, pattern_rotating = 0.07, weekend_workers = 0.08;
  int departure_from_min = 5 * 60;
  std::vector<double> departure_shares;  // per half hour from departure_from_min
  double commute_mean_min = 27.6, commute_sigma = 0.7;
  double work_scale = 1.0;  // rule 2: target hours / the table's own target when a group borrows the table

  // The row for an activity, pop and day: the most specific match (employment and sex named beat "any").
  const TimeUseRow* row(DayActivity a, Employment e, char sex, SurveyDay day) const;
};

// Loads <data>/timeuse/<id>.toml. Throws on a missing file or bad data.
TimeUseTable load_timeuse(const std::string& path);
// Rule 2: the table for a country: its ISO3 file when it exists, else its income group's, with work hours scaled
// to the group's ILOSTAT target (index.toml).
TimeUseTable timeuse_for(const std::string& data_dir, const std::string& profile, const std::string& income);

// What the schedule needs to know of a pop. `key` is the pop's id mixed with the world seed: the skeleton is drawn
// from it alone, each day from it and the day number.
struct SchedulePerson {
  std::uint64_t key = 0;
  int age = 30;
  char sex = 'm';
  Employment employment = Employment::FullTime;
  bool has_workplace = true;  // false: works from home (TU-6) or the workplace is not placed
  int commute_min = -1;       // spec 99: the town's streets to the workplace, one way; -1: drawn from the survey
  bool married = false, young_child = false;
};

// Drawn once per pop (rule 4): the skeleton.
struct ScheduleSkeleton {
  WorkPattern pattern = WorkPattern::Day;
  bool weekend_worker = false;
  int departure_min = 7 * 60 + 15;  // minute of day the pop leaves for work
  int bedtime_min = 23 * 60;
  int commute_min = 28;             // one way
};
ScheduleSkeleton skeleton_of(const TimeUseTable& t, const SchedulePerson& p);

// One segment of a day: minutes from the day's start (00:00), the activity, where the pop is, and who expects it
// there (decision 0070: a commitment names its holder; 0 = nobody).
struct Segment {
  int start_min = 0, end_min = 0;
  DayActivity activity = DayActivity::Sleep;
  PlaceKind place = PlaceKind::Home;
  bool commitment = false;  // a shift start or a return home somebody expects
};

struct DayPlan {
  SurveyDay day = SurveyDay::Weekday;
  std::vector<Segment> segments;  // in order, covering 0..1440 exactly
  bool works = false, studies = false;
  int work_min = 0, sleep_min = 0;
  const Segment& at(int minute_of_day) const;
  PlaceKind where(int minute_of_day) const { return at(minute_of_day).place; }
};

// Rule 5: the day of a pop on a day number (days since 1970-01-01) of a given survey type.
// `school_day`: whether the calendar has school that day (rule 5); -1 = a weekday is a school day.
DayPlan plan_day(const TimeUseTable& t, const SchedulePerson& p, const ScheduleSkeleton& s, std::int64_t day_number, SurveyDay day,
                 int school_day = -1);
// Rule 6: where the pop is at a tick of a world with a calendar: the civil day's survey type from the country calendar.
DayPlan plan_for_tick(const TimeUseTable& t, const CountryCalendar& cal, std::int64_t start_day, const SchedulePerson& p, std::int64_t tick);
PlaceKind where_at(const TimeUseTable& t, const CountryCalendar& cal, std::int64_t start_day, const SchedulePerson& p, std::int64_t tick);

// "07:15" for a minute of day.
std::string clock_text(int minute_of_day);
// The plan as lines a player reads.
std::vector<std::string> plan_lines(const DayPlan& plan);

}  // namespace zs
