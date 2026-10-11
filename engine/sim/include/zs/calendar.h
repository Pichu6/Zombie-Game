// The calendar: civil dates and time from a start date, and each country's workdays, rest days, holidays and school
// days. Spec: docs/tier0/specs/53-calendar.md (decision 0072)
#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "zs/world.h"

namespace zs {

// ---- Rule 1: civil dates (proleptic Gregorian). Day numbers count from 1970-01-01 (day 0, a Thursday).
struct CivilDate {
  int year = 1970, month = 1, day = 1;
  bool operator==(const CivilDate&) const = default;
};

// Howard Hinnant's algorithms: exact integer arithmetic for any year that fits.
constexpr std::int64_t days_from_civil(int year, int month, int day) {
  const std::int64_t y = static_cast<std::int64_t>(year) - (month <= 2 ? 1 : 0);
  const std::int64_t era = (y >= 0 ? y : y - 399) / 400;
  const std::int64_t yoe = y - era * 400;                                    // [0, 399]
  const std::int64_t doy = (153 * (month > 2 ? month - 3 : month + 9) + 2) / 5 + day - 1;  // [0, 365]
  const std::int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;            // [0, 146096]
  return era * 146097 + doe - 719468;
}
constexpr std::int64_t days_from_civil(const CivilDate& d) { return days_from_civil(d.year, d.month, d.day); }
constexpr CivilDate civil_from_days(std::int64_t z) {
  z += 719468;
  const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097;
  const std::int64_t doe = z - era * 146097;                                     // [0, 146096]
  const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;  // [0, 399]
  const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);               // [0, 365]
  const std::int64_t mp = (5 * doy + 2) / 153;                                   // [0, 11]
  const int day = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
  const int month = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
  return {static_cast<int>(yoe + era * 400 + (month <= 2 ? 1 : 0)), month, day};
}
constexpr bool is_leap_year(int year) { return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0; }
// 0 Monday ... 6 Sunday (day 0, 1970-01-01, is a Thursday).
constexpr int weekday_of(std::int64_t day_number) {
  const std::int64_t r = (day_number + 3) % 7;
  return static_cast<int>(r < 0 ? r + 7 : r);
}
// 0 .. 365 (1 January is 0).
int day_of_year(const CivilDate& d);

// Western Easter Sunday (anonymous Gregorian computus) and Orthodox Easter Sunday (Julian computus, as a Gregorian
// date), rule 7.
CivilDate easter_western(int year);
CivilDate easter_orthodox(int year);

// ---- Rules 2 and 3: civil time. Tick 0 is 08:00 (Clock, spec 05) on the start date.
inline constexpr std::int64_t kDefaultStartDay = days_from_civil(2027, 1, 11);  // Monday 11 January 2027 (decision 0072)

struct CivilTime {
  std::int64_t day_number = 0;
  CivilDate date;
  int weekday = 0;      // 0 Monday ... 6 Sunday
  int day_of_year = 0;  // 0 .. 365
  std::int64_t second_of_day = 0;
  double hour = 0;      // 0 .. 24
};
CivilTime civil_time(std::int64_t start_day, std::int64_t tick);
// "Mon 11 Jan 2027 08:00".
std::string format_civil(const CivilTime& t);

// ---- A country's calendar (rules 5-9), loaded from data/setup/calendar/<id>.toml.
enum class Sector : std::uint8_t { Official = 0, Private = 1 };
enum class DayKind : std::uint8_t { Workday = 0, ShortDay = 1, Rest = 2, Holiday = 3 };
enum class SurveyDay : std::uint8_t { Weekday = 0, Saturday = 1, SundayHoliday = 2 };
const char* day_kind_name(DayKind k);
const char* survey_day_name(SurveyDay d);

struct Holiday {
  std::int64_t day_number = 0;
  bool half = false;
  std::string name;
};

struct CountryCalendar {
  std::string id, from;
  std::uint8_t weekend = 0;     // bit d set: weekday d (0 Monday) is an official weekly rest day
  int main_rest_day = 6;        // 0 Monday ... 6 Sunday
  std::uint8_t short_days = 0;  // official short days that are not rest days
  int private_week_days = 5;    // 5 or 6
  // Holidays: dated per year where the feed covers it (rule 7), else fixed month-days and Easter offsets.
  std::vector<std::string> names;
  std::vector<std::pair<int, int>> fixed;  // (month * 100 + day, name index)
  std::vector<std::pair<int, int>> easter_western, easter_orthodox;  // (offset in days, name index)
  std::map<int, std::vector<Holiday>> dated;  // year -> holidays sorted by day number
  // School (rule 8).
  int school_first_month = 9, school_first_day = 1;
  double long_break_weeks = 11;
  int instruction_days = 180;
  int school_week_days = 5;

  bool is_weekend(int weekday) const { return (weekend >> weekday) & 1u; }
  // The year's holidays, sorted: the dated table if it covers the year, else the rules.
  std::vector<Holiday> holidays_in(int year) const;
  // A holiday on that day, or nullptr. Uses a per-year cache (derived, not saved).
  const Holiday* holiday_on(std::int64_t day_number) const;
  DayKind day_kind(std::int64_t day_number, Sector sector) const;
  SurveyDay survey_day(std::int64_t day_number, Sector sector) const;
  // Rule 8: school-week days, and whether a date is a school day.
  bool is_school_week_day(int weekday) const;
  bool is_school_day(std::int64_t day_number) const;
  // The school year starting in `year`: its first day number, the day after its last span day, the school days, and
  // whether it is short (fewer candidates than instruction days).
  struct SchoolYear {
    std::int64_t first = 0, end = 0;
    std::vector<std::int64_t> days;  // sorted
    bool short_year = false;
  };
  const SchoolYear& school_year(int year) const;

 private:
  mutable std::map<int, std::vector<Holiday>> holiday_cache_;
  mutable std::map<int, SchoolYear> school_cache_;
};

CountryCalendar load_country_calendar(const std::string& path);
// data/setup/calendar/<id>.toml under a data directory.
std::string calendar_path(const std::string& data_dir, const std::string& id);
// Rule 10: the calendar id of a place's country: its real profile, else its base country.
std::string calendar_id(const std::string& profile, const std::string& income);

// ---- Rule 10: the world's calendar. Never wakes; saves the start day and checks the country id on load.
class CalendarSystem : public System {
 public:
  CalendarSystem(std::int64_t start_day, CountryCalendar calendar) : start_day_(start_day), calendar_(std::move(calendar)) {}
  const char* name() const override { return "calendar"; }
  void update(World&) override {}
  std::int64_t next_wake(const World&, std::int64_t) const override { return kNever; }
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& r) override;

  std::int64_t start_day() const { return start_day_; }
  const CountryCalendar& calendar() const { return calendar_; }
  CivilTime now(const World& w) const { return civil_time(start_day_, w.clock.tick); }

 private:
  std::int64_t start_day_;
  CountryCalendar calendar_;
};

// Civil time of a world: from its calendar system, or from the default start date if it has none (earlier stages).
CivilTime civil_now(const World& w);
std::int64_t start_day_of(const World& w);

}  // namespace zs
