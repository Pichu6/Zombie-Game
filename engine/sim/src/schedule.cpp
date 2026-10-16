#include "zs/schedule.h"

#include <algorithm>
#include <filesystem>
#include <stdexcept>

#include "toml.hpp"
#include "zs/detmath.h"

namespace zs {

namespace {

constexpr int kDay = 24 * 60;

// Integer-only keyed draws (spec 02's spirit, cross-compiler): a 64-bit mix of the key, the day and a field.
constexpr std::uint64_t mix(std::uint64_t x) {
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9ull;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebull;
  x ^= x >> 31;
  return x;
}
constexpr std::uint64_t mix(std::uint64_t a, std::uint64_t b) { return mix(a ^ mix(b + 0x9e3779b97f4a7c15ull)); }

enum Field : std::uint64_t { Pattern = 1, Weekend, Departure, Bedtime, Commute, Does, Hours, SleepNoise, Rotate };

double u01(std::uint64_t key, std::uint64_t day, Field f, std::uint64_t k = 0) {
  const std::uint64_t h = mix(mix(mix(key, day), static_cast<std::uint64_t>(f)), k);
  return static_cast<double>(h >> 11) * (1.0 / 9007199254740992.0);  // 53 bits in [0, 1)
}

// Roughly normal, mean 0, SD 1: the mean of four uniforms rescaled (no transcendental maths, bounded in [-3.46, 3.46]).
double z01(std::uint64_t key, std::uint64_t day, Field f, std::uint64_t k = 0) {
  double s = 0;
  for (std::uint64_t i = 0; i < 4; ++i) s += u01(key, day, f, k * 4 + i);
  return (s - 2.0) * 1.7320508075688772;
}

int pick(const std::vector<double>& shares, double u) {
  double acc = 0;
  for (std::size_t i = 0; i < shares.size(); ++i) {
    acc += shares[i];
    if (u < acc) return static_cast<int>(i);
  }
  return static_cast<int>(shares.size()) - 1;
}

DayActivity activity_of(const std::string& s) {
  static const char* const names[] = {"sleep", "selfcare", "commute", "work", "study", "household", "shop", "care", "social", "civic", "leisure"};
  for (int i = 0; i < static_cast<int>(DayActivity::kCount); ++i)
    if (s == names[i]) return static_cast<DayActivity>(i);
  throw std::runtime_error("timeuse: unknown activity " + s);
}

int employment_of(const std::string& s) {
  static const char* const names[] = {"full_time", "part_time", "not_employed", "student", "retired", "child"};
  for (int i = 0; i < 6; ++i)
    if (s == names[i]) return i;
  throw std::runtime_error("timeuse: unknown employment " + s);
}

int day_of(const std::string& s) {
  if (s == "any") return -1;
  if (s == "weekday") return 0;
  if (s == "saturday") return 1;
  if (s == "sunday_holiday") return 2;
  throw std::runtime_error("timeuse: unknown day " + s);
}

int minutes_of(const std::string& hhmm) {
  int h = 0, m = 0;
  if (hhmm.size() < 4) throw std::runtime_error("timeuse: bad time " + hhmm);
  h = std::stoi(hhmm.substr(0, hhmm.find(':')));
  m = std::stoi(hhmm.substr(hhmm.find(':') + 1));
  return h * 60 + m;
}

}  // namespace

const char* employment_name(Employment e) {
  static const char* const names[] = {"full time", "part time", "not employed", "student", "retired", "child"};
  return names[static_cast<int>(e)];
}

const char* activity_name(DayActivity a) {
  static const char* const names[] = {"sleep", "self-care", "commute", "work", "study", "household", "shopping", "care", "social", "civic", "leisure"};
  return names[static_cast<int>(a)];
}

const char* place_kind_name(PlaceKind p) {
  static const char* const names[] = {"home", "workplace", "school", "out", "travelling"};
  return names[static_cast<int>(p)];
}

Employment employment_from_occupation(const std::string& occupation, int age) {
  if (age < 6) return Employment::Child;
  if (occupation == "student" || (age < 18 && occupation.empty())) return Employment::Student;
  if (occupation.rfind("retired", 0) == 0) return Employment::Retired;  // the generator writes "retired janitor"
  if (occupation == "out of work" || occupation.empty()) return Employment::NotEmployed;
  if (age >= 67) return Employment::Retired;
  return Employment::FullTime;  // part time is a later draw by occupation (TU-4); spec 47 gives none today
}

const TimeUseRow* TimeUseTable::row(DayActivity a, Employment e, char sex, SurveyDay day) const {
  const TimeUseRow* best = nullptr;
  int best_score = -1;
  for (const TimeUseRow& r : rows) {
    if (r.activity != a) continue;
    if (r.employment >= 0 && r.employment != static_cast<int>(e)) continue;
    if (r.sex && r.sex != sex) continue;
    if (r.day >= 0 && r.day != static_cast<int>(day)) continue;
    const int score = (r.employment >= 0 ? 4 : 0) + (r.sex ? 2 : 0) + (r.day >= 0 ? 1 : 0);
    if (score > best_score) best = &r, best_score = score;
  }
  return best;
}

TimeUseTable load_timeuse(const std::string& path) {
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  TimeUseTable out;
  out.id = t["id"].value_or("");
  out.source = t["source"].value_or("");
  out.method = t["method"].value_or("");
  out.weekly_hours_target = t["weekly_hours_target"].value_or(0.0);
  out.women_share_of_employed = t["women_share_of_employed"].value_or(0.47);
  if (const toml::array* rows = t["row"].as_array())
    for (const auto& e : *rows)
      if (const toml::table* r = e.as_table()) {
        TimeUseRow row;
        row.activity = activity_of((*r)["activity"].value_or(""));
        row.day = day_of((*r)["day"].value_or("any"));
        row.participation = (*r)["participation"].value_or(0.0);
        row.hours_if_done = (*r)["hours_if_done"].value_or(0.0);
        if (const toml::table* who = (*r)["who"].as_table()) {
          if (auto emp = (*who)["employment"].value<std::string>()) row.employment = employment_of(*emp);
          if (auto sx = (*who)["sex"].value<std::string>()) row.sex = sx->empty() ? 0 : (*sx)[0];
        }
        if (row.participation < 0 || row.participation > 1 || row.hours_if_done < 0 || row.hours_if_done > 24)
          throw std::runtime_error(path + ": a row out of range");
        out.rows.push_back(row);
      }
  if (const toml::table* s = t["sleep"].as_table()) {
    out.sleep_weekday_h = (*s)["weekday_hours"].value_or(out.sleep_weekday_h);
    out.sleep_weekend_h = (*s)["weekend_hours"].value_or(out.sleep_weekend_h);
    out.sleep_sd_min = (*s)["night_sd_minutes"].value_or(out.sleep_sd_min);
    if (auto b = (*s)["bedtime_mean"].value<std::string>()) out.bedtime_mean_min = minutes_of(*b);
    out.bedtime_sd_min = (*s)["bedtime_sd_minutes"].value_or(out.bedtime_sd_min);
  }
  if (const toml::table* w = t["work_pattern"].as_table()) {
    out.pattern_day = (*w)["day"].value_or(out.pattern_day);
    out.pattern_evening = (*w)["evening"].value_or(out.pattern_evening);
    out.pattern_night = (*w)["night"].value_or(out.pattern_night);
    out.pattern_rotating = (*w)["rotating"].value_or(out.pattern_rotating);
    out.weekend_workers = (*w)["weekend_workers"].value_or(out.weekend_workers);
  }
  if (const toml::table* d = t["departure"].as_table()) {
    if (auto from = (*d)["bins_from"].value<std::string>()) out.departure_from_min = minutes_of(*from);
    if (const toml::array* shares = (*d)["shares"].as_array())
      for (const auto& v : *shares) out.departure_shares.push_back(v.value_or(0.0));
  }
  if (out.departure_shares.empty()) out.departure_shares = {0.06, 0.08, 0.10, 0.13, 0.141, 0.11, 0.09, 0.07, 0.05, 0.04, 0.03, 0.03, 0.03, 0.03, 0.03};
  if (const toml::table* c = t["commute"].as_table()) {
    out.commute_mean_min = (*c)["mean_minutes"].value_or(out.commute_mean_min);
    out.commute_sigma = (*c)["sigma"].value_or(out.commute_sigma);
  }
  return out;
}

TimeUseTable timeuse_for(const std::string& data_dir, const std::string& profile, const std::string& income) {
  const std::string dir = data_dir + "/timeuse/";
  std::error_code ec;
  if (!profile.empty() && std::filesystem::exists(dir + profile + ".toml", ec)) return load_timeuse(dir + profile + ".toml");
  const toml::table index = toml::parse_file(dir + "index.toml");
  const std::string id = index["groups"][income].value_or(std::string("USA"));
  TimeUseTable t = load_timeuse(dir + id + ".toml");
  const double target = index["weekly_hours_by_group"][income].value_or(0.0);
  if (target > 0 && t.weekly_hours_target > 0) t.work_scale = target / t.weekly_hours_target;
  return t;
}

ScheduleSkeleton skeleton_of(const TimeUseTable& t, const SchedulePerson& p) {
  ScheduleSkeleton s;
  s.pattern = static_cast<WorkPattern>(pick({t.pattern_day, t.pattern_evening, t.pattern_night, t.pattern_rotating}, u01(p.key, 0, Pattern)));
  s.weekend_worker = u01(p.key, 0, Weekend) < t.weekend_workers;
  const int bin = pick(t.departure_shares, u01(p.key, 0, Departure));
  s.departure_min = t.departure_from_min + bin * 30 + static_cast<int>(u01(p.key, 0, Departure, 1) * 30);
  s.bedtime_min = static_cast<int>(t.bedtime_mean_min + z01(p.key, 0, Bedtime) * t.bedtime_sd_min);
  s.bedtime_min = std::clamp(s.bedtime_min, 20 * 60, 26 * 60);  // 20:00 to 02:00
  // Lognormal commute: median = mean / exp(sigma^2 / 2).
  const double median = t.commute_mean_min / detmath::exp(t.commute_sigma * t.commute_sigma / 2);
  s.commute_min = p.has_workplace ? std::clamp(static_cast<int>(median * detmath::exp(t.commute_sigma * z01(p.key, 0, Commute))), 3, 180) : 0;
  if (p.has_workplace && p.commute_min >= 0) s.commute_min = std::clamp(p.commute_min, 1, 180);  // spec 99
  return s;
}

const Segment& DayPlan::at(int minute_of_day) const {
  const int m = ((minute_of_day % kDay) + kDay) % kDay;
  for (const Segment& s : segments)
    if (m >= s.start_min && m < s.end_min) return s;
  return segments.back();
}

DayPlan plan_day(const TimeUseTable& t, const SchedulePerson& p, const ScheduleSkeleton& s, std::int64_t day_number, SurveyDay day,
                 int school_day) {
  DayPlan plan;
  plan.day = day;
  const std::uint64_t d = static_cast<std::uint64_t>(day_number);
  const bool weekend = day != SurveyDay::Weekday;
  if (school_day < 0) school_day = weekend ? 0 : 1;

  // Rule 1: participation then duration, per activity, with the day's noise factor in [0.75, 1.25].
  auto drawn_minutes = [&](DayActivity a, Employment e) -> int {
    const TimeUseRow* r = t.row(a, e, p.sex, day);
    if (!r || r->participation <= 0 || r->hours_if_done <= 0) return 0;
    if (u01(p.key, d, Does, static_cast<std::uint64_t>(a)) >= r->participation) return 0;
    double hours = r->hours_if_done * (0.75 + 0.5 * u01(p.key, d, Hours, static_cast<std::uint64_t>(a)));
    if (a == DayActivity::Work) hours *= t.work_scale;
    return std::max(10, static_cast<int>(hours * 60 + 0.5));
  };

  // The skeleton's anchors for this day.
  const double sleep_h = weekend ? t.sleep_weekend_h : t.sleep_weekday_h;
  int sleep_min = static_cast<int>(sleep_h * 60 + z01(p.key, d, SleepNoise) * t.sleep_sd_min);
  sleep_min = std::clamp(sleep_min, 4 * 60, 13 * 60);
  plan.sleep_min = sleep_min;
  int bedtime = s.bedtime_min;  // may be past midnight (>= 1440): then the day starts asleep until the wake time
  // Wake time: last night's bedtime plus the sleep; the night before is the same skeleton.
  int wake = (bedtime - kDay) + sleep_min;  // bedtime relative to this day's 00:00 is bedtime - 1440
  wake = std::clamp(wake, 3 * 60, 12 * 60);

  // Work or study today?
  int work = 0, study = 0;
  const bool can_work = p.employment == Employment::FullTime || p.employment == Employment::PartTime;
  if (can_work) {
    const bool weekend_ok = !weekend || s.weekend_worker || true;  // the table's weekend participation already carries the share
    if (weekend_ok) work = drawn_minutes(DayActivity::Work, p.employment);
  }
  if (p.employment == Employment::Student && school_day) study = drawn_minutes(DayActivity::Study, p.employment);
  plan.works = work > 0;
  plan.studies = study > 0;
  plan.work_min = work;
  // The alarm clock (TU-1: weekdays are shorter nights): a day worker, or a student on a school day, wakes for the
  // departure when sleep would run past it; the night is cut, not the shift.
  {
    const bool day_shift = plan.works && (s.pattern == WorkPattern::Day || s.pattern == WorkPattern::Rotating);
    const int alarm = plan.studies ? 8 * 60 - 20 - 45 : day_shift ? s.departure_min - 45 : kDay;
    if (wake > alarm) {
      wake = std::max(alarm, 4 * 60);
      sleep_min = std::max(4 * 60, wake - (bedtime - kDay));
      plan.sleep_min = sleep_min;
    }
  }

  // The chain. `cursor` is the minute of day; segments are appended in order.
  std::vector<Segment> seg;
  auto push = [&](int from, int to, DayActivity a, PlaceKind where, bool commitment = false) {
    if (to <= from) return;
    seg.push_back({from, to, a, where, commitment});
  };
  int cursor = 0;
  // Night workers sleep in the day: their chain starts with work at 22:00 the previous night ending in the morning.
  WorkPattern pattern = s.pattern;
  if (pattern == WorkPattern::Rotating) pattern = static_cast<WorkPattern>(static_cast<int>(u01(p.key, d, Rotate) * 3));
  if (plan.works && pattern == WorkPattern::Night) {
    const int work_end = std::min(10 * 60, work - 2 * 60);  // the shift began at 22:00: `work - 120` minutes remain after midnight
    push(0, std::max(0, work_end), DayActivity::Work, PlaceKind::Workplace);
    push(work_end, work_end + s.commute_min, DayActivity::Commute, PlaceKind::Travelling);
    cursor = work_end + s.commute_min;
    push(cursor, cursor + 30, DayActivity::SelfCare, PlaceKind::Home);
    cursor += 30;
    push(cursor, cursor + sleep_min, DayActivity::Sleep, PlaceKind::Home);
    cursor += sleep_min;
    push(cursor, cursor + 45, DayActivity::SelfCare, PlaceKind::Home);
    cursor += 45;
  } else {
    push(0, wake, DayActivity::Sleep, PlaceKind::Home);
    cursor = wake;
    push(cursor, cursor + 45, DayActivity::SelfCare, PlaceKind::Home);
    cursor += 45;
    if (plan.works) {
      int start = pattern == WorkPattern::Evening ? 15 * 60 : std::max(cursor, s.departure_min);
      if (start > cursor) push(cursor, start, DayActivity::Leisure, PlaceKind::Home);
      cursor = start;
      const PlaceKind at = p.has_workplace ? PlaceKind::Workplace : PlaceKind::Home;
      if (p.has_workplace) {
        push(cursor, cursor + s.commute_min, DayActivity::Commute, PlaceKind::Travelling);
        cursor += s.commute_min;
      }
      push(cursor, cursor + work, DayActivity::Work, at, true);
      cursor += work;
      if (p.has_workplace) {
        push(cursor, cursor + s.commute_min, DayActivity::Commute, PlaceKind::Travelling, true);
        cursor += s.commute_min;
      }
    } else if (plan.studies) {
      const int start = std::max(cursor, 8 * 60 - 20);
      if (start > cursor) push(cursor, start, DayActivity::Leisure, PlaceKind::Home);
      cursor = start;
      push(cursor, cursor + 20, DayActivity::Commute, PlaceKind::Travelling);
      cursor += 20;
      push(cursor, cursor + study, DayActivity::Study, PlaceKind::School, true);
      cursor += study;
      push(cursor, cursor + 20, DayActivity::Commute, PlaceKind::Travelling, true);
      cursor += 20;
    }
  }
  // Discretionary activities, in a fixed order (rule 5), each only if drawn; bedtime bounds them.
  const int end = std::min(kDay, bedtime);
  struct Disc { DayActivity a; PlaceKind where; };
  const Disc order[] = {{DayActivity::Household, PlaceKind::Home}, {DayActivity::Shop, PlaceKind::Out}, {DayActivity::Care, PlaceKind::Home},
                        {DayActivity::Civic, PlaceKind::Out}, {DayActivity::Social, PlaceKind::Out}};
  std::vector<std::pair<Disc, int>> wanted;
  for (const Disc& x : order) {
    const int m = drawn_minutes(x.a, p.employment);
    if (m > 0) wanted.push_back({x, m});
  }
  // Leisure keeps at least 30 min: shorten the discretionary segments in reverse order until they fit.
  int total = 0;
  for (const auto& w : wanted) total += w.second;
  int room = end - cursor - 30;
  for (std::size_t i = wanted.size(); i-- > 0 && total > room;) {
    const int cut = std::min(wanted[i].second - 10, total - room);
    if (cut > 0) wanted[i].second -= cut, total -= cut;
  }
  for (const auto& w : wanted) {
    if (cursor + w.second > end) break;
    push(cursor, cursor + w.second, w.first.a, w.first.where);
    cursor += w.second;
  }
  if (cursor < end) push(cursor, end, DayActivity::Leisure, PlaceKind::Home);
  cursor = std::max(cursor, end);
  if (cursor < kDay) push(cursor, kDay, DayActivity::Sleep, PlaceKind::Home);
  // Clamp any overrun (a long night shift) to the day.
  for (Segment& x : seg) x.end_min = std::min(x.end_min, kDay);
  seg.erase(std::remove_if(seg.begin(), seg.end(), [](const Segment& x) { return x.end_min <= x.start_min; }), seg.end());
  if (seg.empty() || seg.back().end_min < kDay) seg.push_back({seg.empty() ? 0 : seg.back().end_min, kDay, DayActivity::Leisure, PlaceKind::Home, false});
  plan.segments = std::move(seg);
  return plan;
}

DayPlan plan_for_tick(const TimeUseTable& t, const CountryCalendar& cal, std::int64_t start_day, const SchedulePerson& p, std::int64_t tick) {
  const CivilTime now = civil_time(start_day, tick);
  const SurveyDay day = cal.survey_day(now.day_number, Sector::Private);
  return plan_day(t, p, skeleton_of(t, p), now.day_number, day, cal.is_school_day(now.day_number) ? 1 : 0);
}

PlaceKind where_at(const TimeUseTable& t, const CountryCalendar& cal, std::int64_t start_day, const SchedulePerson& p, std::int64_t tick) {
  const CivilTime now = civil_time(start_day, tick);
  return plan_for_tick(t, cal, start_day, p, tick).where(static_cast<int>(now.second_of_day / 60));
}

std::string clock_text(int minute_of_day) {
  const int m = ((minute_of_day % kDay) + kDay) % kDay;
  char buf[8];
  std::snprintf(buf, sizeof buf, "%02d:%02d", m / 60, m % 60);
  return buf;
}

std::vector<std::string> plan_lines(const DayPlan& plan) {
  std::vector<std::string> out;
  for (const Segment& s : plan.segments)
    out.push_back(clock_text(s.start_min) + "-" + clock_text(s.end_min) + "  " + activity_name(s.activity) + " (" + place_kind_name(s.place) + ")" +
                  (s.commitment ? "  *expected*" : ""));
  return out;
}

}  // namespace zs
