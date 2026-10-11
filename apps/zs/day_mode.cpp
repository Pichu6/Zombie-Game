// zs --day (spec 54): a generated town's day, hour by hour, and three pops' plans as lines.
//   zs --day [--seed N] [--country base_low|base_high|real/ARG...] [--people N] [--date YYYY-MM-DD]
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "zs/calendar.h"
#include "zs/gen/catalog.h"
#include "zs/gen/dwellings.h"
#include "zs/gen/keyed.h"
#include "zs/gen/pops.h"
#include "zs/schedule.h"
#include "zs/tier0.h"
#include "day_mode.h"

namespace zs::atlas {

SchedulePerson schedule_person(const gen::Pop& p, std::uint64_t world_seed) {
  SchedulePerson sp;
  sp.key = gen::mix64(world_seed, p.id);
  sp.age = p.age;
  sp.sex = p.sex;
  sp.employment = employment_from_occupation(p.occupation, p.age);
  sp.has_workplace = !p.workplace.empty();
  return sp;
}

std::string calendar_id_for(const std::string& country) {
  // "real/ARG" -> "ARG"; "base_low" -> "base_low".
  const auto slash = country.find('/');
  return slash == std::string::npos ? country : country.substr(slash + 1);
}

int day_main(int argc, char** argv) {
  std::uint64_t seed = 1;
  std::string country = "base_high", date;
  int people = 300;
  for (int i = 1; i + 1 < argc; ++i) {
    if (!std::strcmp(argv[i], "--seed")) seed = std::strtoull(argv[i + 1], nullptr, 10);
    else if (!std::strcmp(argv[i], "--country")) country = argv[i + 1];
    else if (!std::strcmp(argv[i], "--people")) people = std::atoi(argv[i + 1]);
    else if (!std::strcmp(argv[i], "--date")) date = argv[i + 1];
  }
  try {
    const std::string data = tier0::data_dir();
    const gen::Catalog cat = gen::load_catalog(data + "/gen");
    gen::Mix mix = gen::load_mix(data + "/gen/populations/tier0_block.toml");
    gen::set_country(mix, country, data);
    mix.people = people;
    gen::Town town = gen::generate_town(cat, mix, seed);
    const std::string income = gen::income_group(mix);
    const std::string id = calendar_id_for(country);
    const bool base = id.rfind("base_", 0) == 0;
    const TimeUseTable table = timeuse_for(data, base ? "" : id, income);
    const CountryCalendar cal = load_country_calendar(data + "/setup/calendar/" + id + ".toml");
    std::int64_t day0 = kDefaultStartDay;
    if (!date.empty()) {
      int y = 0, m = 0, d = 0;
      if (std::sscanf(date.c_str(), "%d-%d-%d", &y, &m, &d) == 3) day0 = days_from_civil(y, m, d);
    }
    std::printf("Town %s (%s, %s income), %zu people; time use %s (%s), work scale %.2f; calendar %s\n", town.name.c_str(), country.c_str(),
                income.c_str(), town.pops.size(), table.id.c_str(), table.method.c_str(), table.work_scale, cal.id.c_str());
    // Two days: the date, and the next Sunday or holiday.
    std::int64_t sunday = day0;
    while (cal.survey_day(sunday, Sector::Private) != SurveyDay::SundayHoliday && sunday < day0 + 14) ++sunday;
    for (const std::int64_t day : {day0, sunday}) {
      const CivilDate cd = civil_from_days(day);
      const SurveyDay sd = cal.survey_day(day, Sector::Private);
      std::printf("\n%04d-%02d-%02d, %s%s\n", cd.year, cd.month, cd.day, survey_day_name(sd), cal.is_school_day(day) ? ", schools open" : ", schools closed");
      std::printf("hour  asleep   home   work school    out travel\n");
      std::vector<DayPlan> plans;
      for (const gen::Pop& p : town.pops) {
        const SchedulePerson sp = schedule_person(p, seed);
        plans.push_back(plan_day(table, sp, skeleton_of(table, sp), day, sd, cal.is_school_day(day) ? 1 : 0));
      }
      for (int h = 0; h < 24; ++h) {
        int counts[6] = {};
        for (const DayPlan& plan : plans) {
          const Segment& s = plan.at(h * 60);
          if (s.activity == DayActivity::Sleep) ++counts[0];
          else ++counts[1 + static_cast<int>(s.place)];
        }
        std::printf("%02d:00 %6d %6d %6d %6d %6d %6d\n", h, counts[0], counts[1], counts[2], counts[3], counts[4], counts[5]);
      }
      int working = 0;
      double hours = 0;
      for (const DayPlan& plan : plans)
        if (plan.works) ++working, hours += plan.work_min / 60.0;
      std::printf("%d of %zu work today, %.1f h on average when they do\n", working, plans.size(), working ? hours / working : 0.0);
    }
    // Three pops' days: a full-time worker, a student, a retired pop.
    const Employment wanted[] = {Employment::FullTime, Employment::Student, Employment::Retired};
    for (Employment e : wanted)
      for (const gen::Pop& p : town.pops) {
        const SchedulePerson sp = schedule_person(p, seed);
        if (sp.employment != e) continue;
        const ScheduleSkeleton sk = skeleton_of(table, sp);
        static const char* const patterns[] = {"day", "evening", "night", "rotating"};
        std::printf("\n%s, %d, %s, %s%s%s: pattern %s, leaves at %s, bed at %s, commute %d min\n", p.full_name.c_str(), p.age, employment_name(e),
                    p.occupation.c_str(), p.workplace.empty() ? "" : " at ", p.workplace.c_str(), patterns[static_cast<int>(sk.pattern)],
                    clock_text(sk.departure_min).c_str(), clock_text(sk.bedtime_min).c_str(), sk.commute_min);
        for (const auto& line : plan_lines(plan_day(table, sp, sk, day0, cal.survey_day(day0, Sector::Private), cal.is_school_day(day0) ? 1 : 0)))
          std::printf("  %s\n", line.c_str());
        break;
      }
    return 0;
  } catch (const std::exception& ex) {
    std::fprintf(stderr, "zs --day: %s\n", ex.what());
    return 1;
  }
}

}  // namespace zs::atlas
