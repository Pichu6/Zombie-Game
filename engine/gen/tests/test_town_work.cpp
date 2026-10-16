// Spec 99: the pops in the town's workplaces: jobs by the town's jobs, workplaces by their staff, pupils at the nearest
// school, the commute by the town's streets; the one world's schedules read it.
#include <doctest.h>

#include <map>

#include "zs/dead.h"
#include "zs/demographics.h"
#include "zs/gen/place.h"
#include "zs/gen/town_work.h"
#include "zs/schedule.h"
#include "zs/tier0.h"

using namespace zs::gen;

TEST_CASE("town work: adults by the town's jobs, pupils at a school of their stage, the old and the dead at home (rules 1-3)") {
  const Catalog cat = load_catalog(zs::tier0::data_dir() + "/gen");
  const TownEconomyParams ep = load_town_economy(zs::tier0::data_dir() + "/gen");
  const zs::NationDemographics n = zs::load_nation(zs::tier0::data_dir() + "/setup/nations/base_high.toml");
  const TownEconomy e = plan_town_economy(cat, n, ep, 20000, {}, false, {}, "Testville", cat.cultures.front().id, 1);
  std::vector<WorkRow> rows;
  for (std::uint32_t i = 0; i < 2000; ++i) rows.push_back({zs::census_pop_id(i), 30, true});
  rows.push_back({zs::census_pop_id(2000), 8, true});
  rows.push_back({zs::census_pop_id(2001), 15, true});
  rows.push_back({zs::census_pop_id(2002), 70, true});
  rows.push_back({zs::census_pop_id(2003), 40, false});
  const zs::WorkTable w = place_in_work(e, cat, {}, load_town_travel(zs::tier0::data_dir() + "/gen"), rows, rows.size(), 1);
  REQUIRE(w.size() == rows.size());
  std::map<std::string, int> labels;
  for (std::size_t i = 0; i < 2000; ++i) {
    CHECK(w[i].placed);
    CHECK_FALSE(w[i].school);
    ++labels[w[i].job];
  }
  // Managers hold about the country's managers' share.
  CHECK(labels["manager"] / 2000.0 == doctest::Approx(n.occupations[0]).epsilon(0.35));
  CHECK(w[2000].school);
  CHECK(w[2000].workplace.find("Elementary") != std::string::npos);
  CHECK(w[2001].school);
  CHECK_FALSE(w[2002].placed);
  CHECK_FALSE(w[2003].placed);
}

TEST_CASE("town work: a placed commute replaces the survey's draw in the schedule (rule 4)") {
  const zs::TimeUseTable table = zs::timeuse_for(zs::tier0::data_dir(), "", "high");
  zs::SchedulePerson p;
  p.key = 12345;
  p.commute_min = 12;
  CHECK(zs::skeleton_of(table, p).commute_min == 12);
  p.commute_min = -1;
  CHECK(zs::skeleton_of(table, p).commute_min >= 3);
  p.has_workplace = false;
  CHECK(zs::skeleton_of(table, p).commute_min == 0);
}

TEST_CASE("town work: the one world's living census has its workplaces, on the town's streets (rule 5)") {
  const auto t = make_world(1);
  REQUIRE(t.census);
  const auto& people = t.census->people();
  REQUIRE(t.work->size() == people.size());
  int placed = 0, timed = 0;
  for (std::size_t i = 0; i < people.size(); ++i) {
    const zs::PopWork& w = (*t.work)[i];
    if (zs::is_dead(people[i].fate)) CHECK_FALSE(w.placed);
    if (!zs::is_dead(people[i].fate) && people[i].age >= 18 && people[i].age < 65) CHECK(w.placed);
    placed += w.placed;
    timed += w.commute_min >= 1;
  }
  MESSAGE(placed, " of ", people.size(), " placed, ", timed, " with a commute by the streets");
  CHECK(placed > 0);
  CHECK(timed > 0);
}

TEST_CASE("town work: school stages by the country's official ages (UNESCO UIS), not one cut for all (rule 3 amended)") {
  const std::string data = zs::tier0::data_dir();
  const SchoolAges deu = load_school_ages(data, "DEU"), arg = load_school_ages(data, "ARG"), usa = load_school_ages(data, "base_high");
  CHECK(std::string(school_stage(deu, 9)) == "elementary_school");
  CHECK(std::string(school_stage(deu, 11)) == "high_school");   // Germany's lower secondary starts at 10
  CHECK(std::string(school_stage(arg, 11)) == "elementary_school");
  CHECK(std::string(school_stage(arg, 12)) == "high_school");   // Argentina's at 12
  CHECK(std::string(school_stage(arg, 5)).empty());             // primary from 6
  CHECK(std::string(school_stage(usa, 17)) == "high_school");
  CHECK(std::string(school_stage(load_school_ages(data, "", "low"), 5)).empty());  // the low-income base (Ethiopia) starts at 7
  CHECK(primary_share(deu) < primary_share(arg));
}
