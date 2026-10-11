// Spec 53: the calendar. Civil dates and time, the weather on civil time, Easter, day kinds, survey day types,
// school days, and the 187 country calendars in data/setup/calendar.
#include <doctest.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "zs/calendar.h"
#include "zs/clock.h"
#include "zs/fields.h"
#include "zs/place.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

const CountryCalendar& cal(const std::string& id) {
  static std::vector<std::unique_ptr<CountryCalendar>> loaded;
  for (const auto& c : loaded)
    if (c->id == id) return *c;
  loaded.push_back(std::make_unique<CountryCalendar>(load_country_calendar(calendar_path(ZS_DATA_DIR, id))));
  return *loaded.back();
}

std::int64_t day(int y, int m, int d) { return days_from_civil(y, m, d); }

std::vector<std::string> all_ids() {
  std::vector<std::string> ids;
  for (const auto& e : std::filesystem::directory_iterator(std::string(ZS_DATA_DIR) + "/setup/calendar"))
    if (e.path().extension() == ".toml") ids.push_back(e.path().stem().string());
  std::sort(ids.begin(), ids.end());
  return ids;
}

// Tick at a civil date and hour, from the default start date (tick 0 is 08:00 on it).
std::int64_t tick_at(int y, int m, int d, int hour) {
  return (day(y, m, d) - kDefaultStartDay) * kTicksPerGameDay + (hour - 8) * kTicksPerGameHour;
}

}  // namespace

TEST_CASE("calendar dates: every day 1900-2100 converts both ways; known weekdays and leap days (A1)") {
  CHECK(day(1970, 1, 1) == 0);
  CHECK(weekday_of(0) == 3);  // Thursday
  CHECK(weekday_of(day(2027, 1, 11)) == 0);  // Monday
  CHECK(weekday_of(day(2000, 1, 1)) == 5);   // Saturday
  CHECK(civil_from_days(day(2024, 2, 29)) == CivilDate{2024, 2, 29});
  CHECK(civil_from_days(day(2100, 2, 29)) == CivilDate{2100, 3, 1});  // 2100 is not a leap year
  CHECK(is_leap_year(2000));
  CHECK_FALSE(is_leap_year(1900));
  CivilDate prev = civil_from_days(day(1900, 1, 1) - 1);
  for (std::int64_t n = day(1900, 1, 1); n <= day(2100, 12, 31); ++n) {
    const CivilDate d = civil_from_days(n);
    REQUIRE(days_from_civil(d) == n);
    const bool next_day = d.year == prev.year && d.month == prev.month && d.day == prev.day + 1;
    const bool next_month = d.year == prev.year && d.month == prev.month + 1 && d.day == 1;
    const bool next_year = d.year == prev.year + 1 && d.month == 1 && d.day == 1 && prev.month == 12 && prev.day == 31;
    REQUIRE((next_day || next_month || next_year));
    prev = d;
  }
}

TEST_CASE("calendar civil time: tick 0 is 08:00 on the start date; the day count agrees (A1)") {
  const CivilTime t0 = civil_time(kDefaultStartDay, 0);
  CHECK(t0.date == CivilDate{2027, 1, 11});
  CHECK(t0.weekday == 0);
  CHECK(t0.hour == 8.0);
  CHECK(format_civil(t0) == "Mon 11 Jan 2027 08:00");
  const CivilTime midnight = civil_time(kDefaultStartDay, 16 * kTicksPerGameHour);
  CHECK(midnight.date == CivilDate{2027, 1, 12});
  CHECK(midnight.second_of_day == 0);
  for (std::int64_t tick : {std::int64_t{0}, 16 * kTicksPerGameHour - 1, 16 * kTicksPerGameHour, 40 * kTicksPerGameDay + 123}) {
    Clock c;
    c.tick = tick;
    const CivilTime t = civil_time(kDefaultStartDay, tick);
    CHECK(t.day_number - kDefaultStartDay == c.calendar_day());
    CHECK(t.hour == doctest::Approx(c.hour()));
  }
  // Before the start (allowed, exact): the evening before.
  CHECK(civil_time(kDefaultStartDay, -9 * kTicksPerGameHour).date == CivilDate{2027, 1, 10});
  CHECK(civil_time(kDefaultStartDay, -9 * kTicksPerGameHour).hour == 23.0);
}

TEST_CASE("calendar weather: warmest at 15:00 and coldest at 03:00 on the clock; days change at midnight; leap days (A1)") {
  World w(1);
  const Place place = default_place();
  auto ps = std::make_unique<PlaceSystem>(place);
  const PlaceSystem* p = ps.get();
  w.add_system(std::move(ps));
  // A day after the start: 03:00 and 15:00 of 12 January 2027 bracket the day's mean by the diurnal amplitude.
  w.clock.tick = tick_at(2027, 1, 12, 3);
  const double t03 = p->outdoor_c(w), mean = p->today_mean_c(w);
  w.clock.tick = tick_at(2027, 1, 12, 15);
  const double t15 = p->outdoor_c(w);
  CHECK(p->today_mean_c(w) == mean);
  CHECK(t15 > mean);
  CHECK(t03 < mean);
  CHECK(t15 - t03 == doctest::Approx(2 * (0.15 * place.climate.temp_range_c + 2.0)));
  for (int h = 0; h < 24; ++h) {
    w.clock.tick = tick_at(2027, 1, 12, h);
    CHECK(p->outdoor_c(w) <= t15 + 1e-12);
    CHECK(p->outdoor_c(w) >= t03 - 1e-12);
  }
  // The day's mean is the civil day's: it changes at midnight, not at 08:00.
  w.clock.tick = tick_at(2027, 1, 13, 0) - 1;
  const double late = p->today_mean_c(w);
  w.clock.tick = tick_at(2027, 1, 13, 0);
  CHECK(late == mean);
  CHECK(p->today_mean_c(w) == fields::daily_mean_c(place.world_seed, place.cell, place.climate, 2027, 12));
  // Leap year 2028: 29 February repeats 28 February; 1 March is weather day 59.
  CHECK(fields::weather_time(2028, day_of_year({2028, 2, 29}), true, 12).day == 58);
  CHECK(fields::weather_time(2028, day_of_year({2028, 2, 28}), true, 12).day == 58);
  CHECK(fields::weather_time(2028, day_of_year({2028, 3, 1}), true, 12).day == 59);
  CHECK(fields::weather_time(2028, day_of_year({2028, 12, 31}), true, 12).day == 364);
  CHECK(fields::weather_time(2027, day_of_year({2027, 3, 1}), false, 12).day == 59);
}

TEST_CASE("calendar Easter: both computuses on known dates and in their windows; Serbia's Good Friday (A1)") {
  CHECK(easter_western(2027) == CivilDate{2027, 3, 28});
  CHECK(easter_western(2028) == CivilDate{2028, 4, 16});
  CHECK(easter_western(2025) == CivilDate{2025, 4, 20});
  CHECK(easter_orthodox(2027) == CivilDate{2027, 5, 2});
  CHECK(easter_orthodox(2028) == CivilDate{2028, 4, 16});
  CHECK(easter_orthodox(2025) == CivilDate{2025, 4, 20});
  for (int y = 1900; y <= 2099; ++y) {
    const std::int64_t w = days_from_civil(easter_western(y)), o = days_from_civil(easter_orthodox(y));
    REQUIRE(weekday_of(w) == 6);
    REQUIRE(weekday_of(o) == 6);
    REQUIRE(w >= day(y, 3, 22));
    REQUIRE(w <= day(y, 4, 25));
    REQUIRE(o >= day(y, 4, 4));
    REQUIRE(o <= day(y, 5, 8));
  }
  const Holiday* gf = cal("SRB").holiday_on(day(2027, 4, 30));
  REQUIRE(gf);
  CHECK(gf->name.find("Good Friday") != std::string::npos);
  CHECK(days_from_civil(easter_orthodox(2027)) - 2 == day(2027, 4, 30));
}

TEST_CASE("calendar day kinds: rest days, the main rest day, holidays and short days by country (A1)") {
  const CountryCalendar& sau = cal("SAU");
  CHECK(sau.main_rest_day == 4);
  CHECK(sau.day_kind(day(2027, 1, 15), Sector::Official) == DayKind::Rest);     // Friday
  CHECK(sau.day_kind(day(2027, 1, 16), Sector::Official) == DayKind::Rest);     // Saturday
  CHECK(sau.day_kind(day(2027, 1, 17), Sector::Official) == DayKind::Workday);  // Sunday
  CHECK(cal("USA").day_kind(day(2027, 7, 5), Sector::Official) == DayKind::Holiday);  // Independence Day, substitute
  CHECK(cal("USA").day_kind(day(2027, 7, 5), Sector::Private) == DayKind::Holiday);
  CHECK(cal("SRB").day_kind(day(2027, 1, 7), Sector::Official) == DayKind::Holiday);  // Orthodox Christmas
  CHECK(cal("IND").day_kind(day(2027, 1, 16), Sector::Official) == DayKind::ShortDay);  // Saturday
  CHECK(cal("ARE").day_kind(day(2027, 1, 15), Sector::Official) == DayKind::ShortDay);  // Friday
  CHECK(cal("USA").day_kind(day(2027, 1, 13), Sector::Official) == DayKind::Workday);
  // A six-day private week works the other weekend day as a short day; the main rest day stays a rest day.
  CountryCalendar six = cal("USA");
  six.private_week_days = 6;
  CHECK(six.day_kind(day(2027, 1, 16), Sector::Private) == DayKind::ShortDay);  // Saturday
  CHECK(six.day_kind(day(2027, 1, 16), Sector::Official) == DayKind::Rest);
  CHECK(six.day_kind(day(2027, 1, 17), Sector::Private) == DayKind::Rest);  // Sunday
}

TEST_CASE("calendar survey day types: the main rest day and holidays are sunday_holiday, other rest days saturday (A1)") {
  const CountryCalendar& usa = cal("USA");
  CHECK(usa.survey_day(day(2027, 1, 16), Sector::Official) == SurveyDay::Saturday);
  CHECK(usa.survey_day(day(2027, 1, 17), Sector::Official) == SurveyDay::SundayHoliday);
  CHECK(usa.survey_day(day(2027, 11, 25), Sector::Official) == SurveyDay::SundayHoliday);  // Thanksgiving
  CHECK(usa.survey_day(day(2027, 1, 13), Sector::Official) == SurveyDay::Weekday);
  const CountryCalendar& sau = cal("SAU");
  CHECK(sau.survey_day(day(2027, 1, 15), Sector::Official) == SurveyDay::SundayHoliday);  // Friday
  CHECK(sau.survey_day(day(2027, 1, 16), Sector::Official) == SurveyDay::Saturday);
  const CountryCalendar& isr = cal("ISR");
  CHECK(isr.survey_day(day(2027, 1, 16), Sector::Official) == SurveyDay::SundayHoliday);  // Saturday
  CHECK(isr.survey_day(day(2027, 1, 15), Sector::Official) == SurveyDay::Saturday);       // Friday
}

TEST_CASE("calendar school days: the US year, April and southern starts, six-day weeks, never above the target (A1)") {
  const CountryCalendar& usa = cal("USA");
  const auto& y = usa.school_year(2027);
  CHECK(y.days.size() == static_cast<std::size_t>(usa.instruction_days));
  CHECK(usa.instruction_days == 180);
  CHECK(y.first == day(2027, 8, 25));
  for (std::int64_t d : y.days) {
    REQUIRE(usa.is_school_week_day(weekday_of(d)));
    REQUIRE((!usa.holiday_on(d) || usa.holiday_on(d)->half));
    REQUIRE(d >= y.first);
    REQUIRE(d < y.end);
    REQUIRE(usa.is_school_day(d));
  }
  CHECK_FALSE(usa.is_school_day(day(2027, 1, 16)));  // a Saturday
  CHECK_FALSE(usa.is_school_day(day(2027, 7, 14)));  // the long break
  CHECK(usa.is_school_day(day(2027, 1, 13)));        // a Wednesday in January (the 2026 school year)
  CHECK(cal("JPN").school_first_month == 4);
  CHECK((cal("ARG").school_first_month == 2 || cal("ARG").school_first_month == 3));
  const CountryCalendar& ind = cal("IND");
  REQUIRE(ind.school_week_days == 6);
  const auto& iy = ind.school_year(2027);
  CHECK(std::any_of(iy.days.begin(), iy.days.end(), [](std::int64_t d) { return weekday_of(d) == 5; }));
  for (const std::string& id : all_ids()) {
    const CountryCalendar& c = cal(id);
    const auto& s = c.school_year(2027);
    INFO(id);
    CHECK(s.days.size() <= static_cast<std::size_t>(c.instruction_days));
    for (std::int64_t d : s.days) {
      REQUIRE(c.is_school_week_day(weekday_of(d)));
      const Holiday* h = c.holiday_on(d);
      REQUIRE((!h || h->half));
    }
  }
}

TEST_CASE("calendar data: all 187 files load; holidays in every year; the 2025 median matches CK-275 (A1)") {
  const std::vector<std::string> ids = all_ids();
  CHECK(ids.size() == 187);
  std::vector<int> full_2025;
  std::vector<std::string> rule_less;
  for (const std::string& id : ids) {
    const CountryCalendar& c = cal(id);
    INFO(id);
    CHECK(c.is_weekend(c.main_rest_day));
    for (int y = 2021; y <= 2031; ++y) CHECK_FALSE(c.holidays_in(y).empty());
    // Beyond the table: rules only (spec 53 rule 7). Every country with a fixed or Easter rule keeps holidays there;
    // Israel and Oman have only moving holidays, so they have none (Open 1).
    const bool has_rules = !c.fixed.empty() || !c.easter_western.empty() || !c.easter_orthodox.empty();
    CHECK(c.holidays_in(2040).empty() == !has_rules);
    if (!has_rules) rule_less.push_back(c.from);
    if (id.rfind("base_", 0) == 0) continue;
    const auto h = c.holidays_in(2025);
    full_2025.push_back(static_cast<int>(std::count_if(h.begin(), h.end(), [](const Holiday& x) { return !x.half; })));
  }
  std::sort(rule_less.begin(), rule_less.end());
  rule_less.erase(std::unique(rule_less.begin(), rule_less.end()), rule_less.end());
  CHECK(rule_less == std::vector<std::string>{"ISR", "OMN"});
  REQUIRE(full_2025.size() == 183);
  std::sort(full_2025.begin(), full_2025.end());
  const int median = full_2025[full_2025.size() / 2];
  CHECK(median >= 12);
  CHECK(median <= 16);
  // Base countries copy their real country (decision 0072 rule 5).
  CHECK(cal("base_upper_middle").from == "SRB");
  CHECK(cal("base_high").from == "USA");
  CHECK(cal("base_upper_middle").holidays_in(2027).size() == cal("SRB").holidays_in(2027).size());
}

TEST_CASE("calendar in the one world: make_run adds it; save and load keep the start day (A1)") {
  tier0::Tier0 t = tier0::make_run(1);
  REQUIRE(t.calendar);
  CHECK(t.calendar->start_day() == kDefaultStartDay);
  CHECK(t.calendar->calendar().id == "base_upper_middle");
  CHECK(civil_now(*t.world).date == CivilDate{2027, 1, 11});
  const auto bytes = t.world->save();
  tier0::Tier0 u = tier0::make_run(1);
  u.world->load(bytes);
  CHECK(u.calendar->start_day() == kDefaultStartDay);
  CHECK(u.world->world_hash() == t.world->world_hash());
}
