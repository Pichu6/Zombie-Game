// The calendar. Spec: docs/tier0/specs/53-calendar.md
#include "zs/calendar.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

#include "toml.hpp"
#include "zs/clock.h"

namespace zs {

namespace {

[[noreturn]] void fail(const std::string& path, const std::string& what) {
  throw std::runtime_error("calendar " + path + ": " + what);
}

const char* const kDayIds[7] = {"mon", "tue", "wed", "thu", "fri", "sat", "sun"};
const char* const kDayShort[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char* const kMonthShort[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int day_id(const std::string& s, const std::string& path) {
  for (int d = 0; d < 7; ++d)
    if (s == kDayIds[d]) return d;
  fail(path, "unknown day '" + s + "'");
}

std::uint8_t day_mask(const toml::table& t, const char* key, const std::string& path) {
  const toml::array* a = t[key].as_array();
  if (!a) fail(path, std::string("missing list '") + key + "'");
  std::uint8_t mask = 0;
  for (const auto& v : *a) {
    const auto s = v.value<std::string>();
    if (!s) fail(path, std::string("'") + key + "' must be day names");
    mask = static_cast<std::uint8_t>(mask | (1u << day_id(*s, path)));
  }
  return mask;
}

template <class T> T value(const toml::table& t, const char* key, const std::string& path) {
  const auto x = t[key].value<T>();
  if (!x) fail(path, std::string("missing '") + key + "'");
  return *x;
}

std::vector<std::string> strings(const toml::table& t, const char* key, const std::string& path) {
  const toml::array* a = t[key].as_array();
  if (!a) fail(path, std::string("missing list '") + key + "'");
  std::vector<std::string> out;
  for (const auto& v : *a) {
    const auto s = v.value<std::string>();
    if (!s) fail(path, std::string("'") + key + "' must be strings");
    out.push_back(*s);
  }
  return out;
}

// "MM-DD" -> month * 100 + day.
int month_day(const std::string& s, const std::string& path) {
  auto digit = [&](std::size_t i) { return i < s.size() && s[i] >= '0' && s[i] <= '9' ? s[i] - '0' : -1; };
  const int m1 = digit(0), m2 = digit(1), d1 = digit(3), d2 = digit(4);
  if (s.size() != 5 || s[2] != '-' || m1 < 0 || m2 < 0 || d1 < 0 || d2 < 0) fail(path, "bad month-day '" + s + "'");
  const int m = m1 * 10 + m2, d = d1 * 10 + d2;
  if (m < 1 || m > 12 || d < 1 || d > 31) fail(path, "bad month-day '" + s + "'");
  return m * 100 + d;
}

// "<left>:<name index>[h]" -> (left, index, half).
struct Code {
  std::string left;
  int index = 0;
  bool half = false;
};
Code code(const std::string& s, std::size_t names, const std::string& path) {
  const auto colon = s.rfind(':');
  if (colon == std::string::npos) fail(path, "bad holiday code '" + s + "'");
  Code c;
  c.left = s.substr(0, colon);
  std::string right = s.substr(colon + 1);
  if (!right.empty() && right.back() == 'h') {
    c.half = true;
    right.pop_back();
  }
  try {
    c.index = std::stoi(right);
  } catch (...) {
    fail(path, "bad holiday code '" + s + "'");
  }
  if (c.index < 0 || static_cast<std::size_t>(c.index) >= names) fail(path, "holiday name index out of range in '" + s + "'");
  return c;
}

std::int64_t date_or_neg(int year, int mmdd) {
  const int m = mmdd / 100, d = mmdd % 100;
  const std::int64_t n = days_from_civil(year, m, d);
  return civil_from_days(n) == CivilDate{year, m, d} ? n : -1;  // 29 February in a common year does not exist
}

}  // namespace

int day_of_year(const CivilDate& d) { return static_cast<int>(days_from_civil(d) - days_from_civil(d.year, 1, 1)); }

CivilDate easter_western(int y) {
  const int a = y % 19, b = y / 100, c = y % 100;
  const int d = b / 4, e = b % 4;
  const int f = (b + 8) / 25;
  const int g = (b - f + 1) / 3;
  const int h = (19 * a + b - d - g + 15) % 30;
  const int i = c / 4, k = c % 4;
  const int l = (32 + 2 * e + 2 * i - h - k) % 7;
  const int m = (a + 11 * h + 22 * l) / 451;
  return {y, (h + l - 7 * m + 114) / 31, (h + l - 7 * m + 114) % 31 + 1};
}

CivilDate easter_orthodox(int y) {
  const int a = y % 4, b = y % 7, c = y % 19;
  const int d = (19 * c + 15) % 30;
  const int e = (2 * a + 4 * b - d + 34) % 7;
  const int month = (d + e + 114) / 31, day = (d + e + 114) % 31 + 1;
  // The Julian date as a Gregorian one: the calendars differ by y/100 - y/400 - 2 days after the Julian 29 February.
  return civil_from_days(days_from_civil(y, month, day) + (y / 100 - y / 400 - 2));
}

CivilTime civil_time(std::int64_t start_day, std::int64_t tick) {
  const std::int64_t since_midnight = tick + kStartTimeOfDayTicks;
  std::int64_t days = since_midnight / kTicksPerGameDay;
  std::int64_t in_day = since_midnight - days * kTicksPerGameDay;
  if (in_day < 0) {  // ticks before the start (none in play; kept exact)
    in_day += kTicksPerGameDay;
    --days;
  }
  CivilTime t;
  t.day_number = start_day + days;
  t.date = civil_from_days(t.day_number);
  t.weekday = weekday_of(t.day_number);
  t.day_of_year = day_of_year(t.date);
  t.second_of_day = static_cast<std::int64_t>(static_cast<double>(in_day) * kTickSeconds);
  t.hour = static_cast<double>(in_day) / static_cast<double>(kTicksPerGameHour);
  return t;
}

std::string format_civil(const CivilTime& t) {
  char buf[48];
  const int h = static_cast<int>(t.second_of_day / 3600), m = static_cast<int>(t.second_of_day / 60 % 60);
  std::snprintf(buf, sizeof buf, "%s %d %s %d %02d:%02d", kDayShort[t.weekday], t.date.day, kMonthShort[t.date.month - 1],
                t.date.year, h, m);
  return buf;
}

const char* day_kind_name(DayKind k) {
  switch (k) {
    case DayKind::Workday: return "workday";
    case DayKind::ShortDay: return "short day";
    case DayKind::Rest: return "rest day";
    case DayKind::Holiday: return "holiday";
  }
  return "?";
}

const char* survey_day_name(SurveyDay d) {
  switch (d) {
    case SurveyDay::Weekday: return "weekday";
    case SurveyDay::Saturday: return "saturday";
    case SurveyDay::SundayHoliday: return "sunday_holiday";
  }
  return "?";
}

// ---- Holidays (rule 7)

std::vector<Holiday> CountryCalendar::holidays_in(int year) const {
  if (const auto it = dated.find(year); it != dated.end()) return it->second;
  std::vector<Holiday> out;
  auto add = [&](std::int64_t day, int name) {
    if (day < 0) return;
    for (const Holiday& h : out)
      if (h.day_number == day) return;  // the first rule to name a date keeps it
    out.push_back({day, false, names[static_cast<std::size_t>(name)]});
  };
  for (const auto& [mmdd, name] : fixed) add(date_or_neg(year, mmdd), name);
  const std::int64_t west = days_from_civil(zs::easter_western(year)), orth = days_from_civil(zs::easter_orthodox(year));
  for (const auto& [offset, name] : easter_western) add(west + offset, name);
  for (const auto& [offset, name] : easter_orthodox) add(orth + offset, name);
  std::erase_if(out, [&](const Holiday& h) { return civil_from_days(h.day_number).year != year; });
  std::sort(out.begin(), out.end(), [](const Holiday& a, const Holiday& b) { return a.day_number < b.day_number; });
  return out;
}

const Holiday* CountryCalendar::holiday_on(std::int64_t day_number) const {
  const int year = civil_from_days(day_number).year;
  auto it = holiday_cache_.find(year);
  if (it == holiday_cache_.end()) it = holiday_cache_.emplace(year, holidays_in(year)).first;
  const std::vector<Holiday>& v = it->second;
  const auto h = std::lower_bound(v.begin(), v.end(), day_number, [](const Holiday& x, std::int64_t d) { return x.day_number < d; });
  return h != v.end() && h->day_number == day_number ? &*h : nullptr;
}

// ---- Day kinds (rules 5 and 6)

DayKind CountryCalendar::day_kind(std::int64_t day_number, Sector sector) const {
  const Holiday* h = holiday_on(day_number);
  if (h && !h->half) return DayKind::Holiday;
  const int wd = weekday_of(day_number);
  if (is_weekend(wd)) {
    if (sector == Sector::Private && private_week_days >= 6 && wd != main_rest_day) return DayKind::ShortDay;
    return DayKind::Rest;
  }
  if (h || ((short_days >> wd) & 1u)) return DayKind::ShortDay;
  return DayKind::Workday;
}

SurveyDay CountryCalendar::survey_day(std::int64_t day_number, Sector sector) const {
  if (day_kind(day_number, sector) == DayKind::Holiday) return SurveyDay::SundayHoliday;
  const int wd = weekday_of(day_number);
  if (wd == main_rest_day) return SurveyDay::SundayHoliday;
  if (is_weekend(wd)) return SurveyDay::Saturday;
  return SurveyDay::Weekday;
}

// ---- School days (rule 8)

bool CountryCalendar::is_school_week_day(int weekday) const {
  return school_week_days >= 6 ? weekday != main_rest_day : !is_weekend(weekday);
}

const CountryCalendar::SchoolYear& CountryCalendar::school_year(int year) const {
  if (const auto it = school_cache_.find(year); it != school_cache_.end()) return it->second;
  auto first_day = [&](int y) {
    std::int64_t d = days_from_civil(y, school_first_month, school_first_day);
    while (!is_school_week_day(weekday_of(d))) ++d;
    return d;
  };
  SchoolYear s;
  s.first = first_day(year);
  const std::int64_t next = first_day(year + 1);
  const auto long_break = static_cast<std::int64_t>(std::lround(long_break_weeks * 7.0));
  s.end = next - long_break > s.first ? next - long_break : next;
  std::vector<std::int64_t> c;
  for (std::int64_t d = s.first; d < s.end; ++d) {
    if (!is_school_week_day(weekday_of(d))) continue;
    const Holiday* h = holiday_on(d);
    if (h && !h->half) continue;
    c.push_back(d);
  }
  const auto want = static_cast<std::size_t>(std::max(0, instruction_days));
  if (c.size() > want) {
    const std::size_t excess = c.size() - want, larger = (excess + 1) / 2;
    auto cut = [&](std::size_t count, std::int64_t at) {
      if (count == 0 || c.empty()) return;
      const auto it = std::lower_bound(c.begin(), c.end(), at);
      const std::size_t centre = static_cast<std::size_t>(it - c.begin());
      std::size_t from = centre >= count / 2 ? centre - count / 2 : 0;
      from = std::min(from, c.size() - count);
      c.erase(c.begin() + static_cast<std::ptrdiff_t>(from), c.begin() + static_cast<std::ptrdiff_t>(from + count));
    };
    const std::int64_t span = s.end - s.first;
    cut(larger, s.first + span / 3);
    cut(excess - larger, s.first + 2 * span / 3);
  } else {
    s.short_year = c.size() < want;
  }
  s.days = std::move(c);
  return school_cache_.emplace(year, std::move(s)).first->second;
}

bool CountryCalendar::is_school_day(std::int64_t day_number) const {
  const int year = civil_from_days(day_number).year;
  for (const int y : {year, year - 1}) {
    const SchoolYear& s = school_year(y);
    if (day_number >= s.first && day_number < s.end) return std::binary_search(s.days.begin(), s.days.end(), day_number);
  }
  return false;
}

// ---- Loading

CountryCalendar load_country_calendar(const std::string& path) {
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    fail(path, std::string(e.description()));
  }
  CountryCalendar c;
  c.id = value<std::string>(t, "id", path);
  c.from = value<std::string>(t, "from", path);
  c.weekend = day_mask(t, "weekend", path);
  c.main_rest_day = day_id(value<std::string>(t, "main_rest_day", path), path);
  c.short_days = day_mask(t, "short_days", path);
  c.private_week_days = static_cast<int>(value<std::int64_t>(t, "private_week_days", path));
  if (c.weekend == 0) fail(path, "no weekly rest day");
  if (!c.is_weekend(c.main_rest_day)) fail(path, "the main rest day is not a weekly rest day");

  const toml::table* h = t["holidays"].as_table();
  if (!h) fail(path, "missing table [holidays]");
  c.names = strings(*h, "names", path);
  for (const std::string& s : strings(*h, "fixed", path)) {
    const Code k = code(s, c.names.size(), path);
    c.fixed.emplace_back(month_day(k.left, path), k.index);
  }
  for (const char* key : {"easter_western", "easter_orthodox"}) {
    auto& out = std::string(key) == "easter_western" ? c.easter_western : c.easter_orthodox;
    for (const std::string& s : strings(*h, key, path)) {
      const Code k = code(s, c.names.size(), path);
      out.emplace_back(std::stoi(k.left), k.index);
    }
  }
  if (const toml::table* dates = (*h)["dates"].as_table()) {
    for (const auto& [key, node] : *dates) {
      const int year = std::stoi(std::string(key.str()));
      const toml::array* a = node.as_array();
      if (!a) fail(path, "holidays.dates." + std::string(key.str()) + " must be a list");
      std::vector<Holiday>& v = c.dated[year];
      for (const auto& x : *a) {
        const auto s = x.value<std::string>();
        if (!s) fail(path, "holiday codes must be strings");
        const Code k = code(*s, c.names.size(), path);
        const std::int64_t day = date_or_neg(year, month_day(k.left, path));
        if (day < 0) fail(path, "no such date " + std::to_string(year) + "-" + k.left);
        v.push_back({day, k.half, c.names[static_cast<std::size_t>(k.index)]});
      }
      std::sort(v.begin(), v.end(), [](const Holiday& a2, const Holiday& b) { return a2.day_number < b.day_number; });
    }
  }

  const toml::table* s = t["school"].as_table();
  if (!s) fail(path, "missing table [school]");
  const int first = month_day(value<std::string>(*s, "first_day", path), path);
  c.school_first_month = first / 100;
  c.school_first_day = first % 100;
  c.long_break_weeks = value<double>(*s, "long_break_weeks", path);
  c.instruction_days = static_cast<int>(value<std::int64_t>(*s, "instruction_days", path));
  c.school_week_days = static_cast<int>(value<std::int64_t>(*s, "week_days", path));
  return c;
}

std::string calendar_path(const std::string& data_dir, const std::string& id) {
  return data_dir + "/setup/calendar/" + id + ".toml";
}

std::string calendar_id(const std::string& profile, const std::string& income) {
  return profile.empty() ? "base_" + income : profile;
}

// ---- The system

void CalendarSystem::save(ArchiveWriter& a) const {
  a.i64(start_day_);
  a.str(calendar_.id);
}

void CalendarSystem::load(ArchiveReader& r) {
  start_day_ = r.i64();
  const std::string id = r.str();
  if (id != calendar_.id) throw std::runtime_error("save: calendar of " + id + ", setup has " + calendar_.id);
}

CivilTime civil_now(const World& w) { return civil_time(start_day_of(w), w.clock.tick); }

std::int64_t start_day_of(const World& w) {
  if (const auto* c = w.system<CalendarSystem>()) return c->start_day();
  return kDefaultStartDay;
}

}  // namespace zs
