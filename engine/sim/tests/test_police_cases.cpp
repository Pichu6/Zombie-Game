// Spec 64: the police's missing-person case: a risk tier, a queue for free searches (high risk first), the national record,
// and who finds the pop.
#include <doctest.h>

#include <cstdio>
#include <filesystem>
#include <initializer_list>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "zs/missing_persons.h"
#include "zs/police.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr std::int64_t kMin = kTicksPerGameHour / 60;
constexpr PopId kChildFrom = 10'000;  // ids from here are children in these tests

// Every pop's commitment falls due 30 minutes after the absence begins, and somebody at home holds it: the ladder breaks it at
// 60 minutes, asks at 3 hours, reports at 7 hours.
std::optional<Commitment> due_in_half_an_hour(PopId pop, std::int64_t from) {
  return Commitment{CommitmentKind::ShiftStart, from + 30 * kMin, {pop + 1'000'000}};
}

constexpr std::int64_t kReportedAfter = 420 * kMin;  // 30 + 30 tolerance + 360

struct PoliceRig {
  std::unique_ptr<World> world;
  Expectations* exp = nullptr;
  PoliceCases* police = nullptr;
  explicit PoliceRig(PoliceParams params, std::uint64_t seed = 1) {
    world = std::make_unique<World>(seed);
    world->check_every_tick = false;
    auto e = std::make_unique<Expectations>(load_expectation_params(tier0::data_dir() + "/life/expectations.toml"), due_in_half_an_hour);
    e->set_external_search(true);
    auto p = std::make_unique<PoliceCases>(params, e.get(), [](PopId id) { return id >= kChildFrom; });
    e->add_listener(p.get());
    exp = e.get();
    police = p.get();
    world->add_system(std::move(e));
    world->add_system(std::move(p));
    world->close_setup();
  }
  void begin(PopId pop, AbsenceCause cause = AbsenceCause::Unknown) {
    world->submit({0, AbsenceBegin, pop, static_cast<std::int64_t>(cause), 0});
  }
  void end(PopId pop) { world->submit({0, AbsenceEnd, pop, 0, 0}); }
  std::vector<Event> events(EventType type, PopId pop = 0) const {
    std::vector<Event> out;
    for (const Event& e : world->log.all())
      if (e.type == type && (pop == 0 || e.subject == pop)) out.push_back(e);
    return out;
  }
};

PoliceParams params_with(double adult_high, double adult_medium, double child_high, int at_once) {
  PoliceParams p;
  p.adult_tiers[0] = adult_high;
  p.adult_tiers[1] = adult_medium;
  p.adult_tiers[2] = 1 - adult_high - adult_medium;
  p.child_tiers[0] = child_high;
  p.child_tiers[1] = 1 - child_high;
  p.child_tiers[2] = 0;
  p.searches_at_once = at_once;
  return p;
}

}  // namespace

TEST_CASE("the police parameters load, and a bad table is refused (A1)") {
  const std::string path = tier0::data_dir() + "/institutions/police/missing.toml";
  const PoliceParams p = load_police_params(path, "", "high");
  CHECK(p.adult_tiers[0] == doctest::Approx(0.31));
  CHECK(p.child_tiers[1] == doctest::Approx(0.83));
  CHECK(p.found_by[1] == doctest::Approx(0.33));
  CHECK(p.searches_at_once >= 1);
  CHECK(load_police_params(path, "", "low").adult_tiers[0] == doctest::Approx(0.31));  // every group borrows gb_ew for now
  CHECK_THROWS(load_police_params(tier0::data_dir() + "/institutions/police/no_such.toml", "", "high"));
  CHECK_THROWS(load_police_params(path, "", "no_such_group"));
  const std::string bad_path = (std::filesystem::temp_directory_path() / "zs_bad_police.toml").string();
  {
    std::ofstream bad(bad_path);
    bad << "[t]\nadult_tiers=[0.5,0.2,0.1]\nchild_tiers=[0.1,0.8,0.1]\nfound_by=[0.4,0.3,0.3]\nchild_age_max=17\n"
           "searches_at_once=1\nwandered_is_high=true\n[index]\nhigh=\"t\"\n";
  }
  CHECK_THROWS(load_police_params(bad_path, "t", "high"));  // the adult tiers add up to 0.8
  std::remove(bad_path.c_str());
}

TEST_CASE("a report opens a case and the police open the search at once when they have staff (A1)") {
  PoliceRig r(PoliceParams{});
  r.begin(1);
  r.world->jump(kReportedAfter - 1);
  CHECK(r.events(EventType::PoliceCaseOpened).empty());
  CHECK(r.events(EventType::SearchStarted).empty());
  r.world->jump(kReportedAfter + 1);
  const auto opened = r.events(EventType::PoliceCaseOpened, 1);
  REQUIRE(opened.size() == 1);
  CHECK(opened[0].tick == kReportedAfter);
  CHECK(opened[0].b == 0);  // staffed at once
  const auto search = r.events(EventType::SearchStarted, 1);
  REQUIRE(search.size() == 1);
  CHECK(search[0].tick == kReportedAfter);  // not an hour later: the police own the search
  CHECK(r.exp->find(1)->stage == AbsenceStage::Searching);
  CHECK(r.police->has_record(1));  // the national record
  CHECK(r.police->active_searches() == 1);
  // The chain: the case follows the report, which follows the whole ladder back to the absence.
  std::vector<EventType> types;
  for (const Event& e : r.world->log.cause_chain(opened[0].id)) types.push_back(e.type);
  REQUIRE(types.size() >= 2);
  CHECK(types[0] == EventType::PoliceCaseOpened);
  CHECK(types[1] == EventType::MissingReported);
}

TEST_CASE("cases wait for a free search, high risk first, then the oldest (A1)") {
  PoliceRig r(params_with(0, 1, 1, 1));  // adults all medium, children all high; one search at once
  r.begin(1);
  r.world->jump(kReportedAfter + 1);    // pop 1 is reported and searched
  r.begin(2);
  r.world->jump(r.world->clock.tick + kReportedAfter + 2);  // pop 2 (medium) is reported: waits (the command applies a tick later)
  r.begin(kChildFrom);
  r.world->jump(r.world->clock.tick + kReportedAfter + 2);  // a child (high) is reported later: ahead of pop 2
  const auto opened = r.events(EventType::PoliceCaseOpened);
  REQUIRE(opened.size() == 3);
  CHECK(opened[0].b == 0);
  CHECK(opened[1].b == 1);  // number 1 in the queue
  CHECK(opened[2].b == 1);  // the high-risk case goes ahead of the medium one
  CHECK(opened[2].a == static_cast<std::int64_t>(RiskTier::High));
  CHECK(r.police->active_searches() == 1);
  CHECK(r.police->queued() == 2);
  CHECK(r.exp->find(2)->stage == AbsenceStage::Reported);  // still waiting for staff
  // A free search goes to the high-risk case first, then to the older medium one.
  r.end(1);
  r.world->run_ticks(1);
  CHECK(r.exp->find(kChildFrom)->stage == AbsenceStage::Searching);
  CHECK(r.exp->find(2)->stage == AbsenceStage::Reported);
  r.end(kChildFrom);
  r.world->run_ticks(1);
  CHECK(r.exp->find(2)->stage == AbsenceStage::Searching);
  std::vector<PopId> order;
  for (const Event& e : r.events(EventType::SearchStarted)) order.push_back(e.subject);
  REQUIRE(order.size() == 3);
  CHECK(order[0] == 1);
  CHECK(order[1] == kChildFrom);
  CHECK(order[2] == 2);
}

TEST_CASE("a pop that wandered off is graded high risk whatever the shares say (A1)") {
  PoliceRig r(params_with(0, 0, 0, 2));  // adults all low
  r.begin(1, AbsenceCause::Wandered);
  r.begin(2, AbsenceCause::Chose);
  r.world->jump(kReportedAfter + 1);
  CHECK(r.events(EventType::PoliceCaseOpened, 1)[0].a == static_cast<std::int64_t>(RiskTier::High));
  CHECK(r.events(EventType::PoliceCaseOpened, 2)[0].a == static_cast<std::int64_t>(RiskTier::Low));
}

TEST_CASE("the pop being found resolves the case and frees the search; a case waiting when the pop returns is resolved unsearched (A1)") {
  PoliceRig r(params_with(1, 0, 1, 1));  // everyone high; one search at once
  r.begin(1);
  r.begin(2);
  r.world->jump(kReportedAfter + 1);
  REQUIRE(r.police->cases().size() == 2);
  CHECK(r.police->active_searches() == 1);
  r.end(2);  // the one waiting returns first
  r.world->run_ticks(1);
  CHECK(r.events(EventType::SearchStarted, 2).empty());  // never searched
  const auto resolved = r.events(EventType::PoliceCaseResolved, 2);
  REQUIRE(resolved.size() == 1);
  CHECK(resolved[0].b > 0);
  CHECK_FALSE(r.police->has_record(2));
  CHECK(r.police->active_searches() == 1);  // pop 1 is still being searched for
  r.end(1);
  r.world->run_ticks(1);
  CHECK(r.police->cases().empty());
  CHECK(r.police->opened_total() == 2);
  CHECK(r.police->resolved_total() == 2);
  CHECK(r.police->active_searches() == 0);
}

TEST_CASE("a pop back before it is reported has no case and no record (A1)") {
  PoliceRig r(PoliceParams{});
  r.begin(1);
  r.world->jump(5 * kTicksPerGameHour);  // asked around, not yet reported
  r.end(1);
  r.world->run_ticks(1);
  CHECK(r.events(EventType::PoliceCaseOpened).empty());
  CHECK(r.events(EventType::PoliceCaseResolved).empty());
  CHECK_FALSE(r.police->has_record(1));
}

TEST_CASE("the tiers and who finds the pop follow the table's shares, for adults and children (A1)") {
  PoliceRig r(PoliceParams{});  // the England and Wales shares
  constexpr int kAdults = 3000, kChildren = 1500;
  for (int i = 1; i <= kAdults; ++i) r.begin(static_cast<PopId>(i));
  for (int i = 0; i < kChildren; ++i) r.begin(kChildFrom + static_cast<PopId>(i));
  r.world->jump(kReportedAfter + 1);
  int adult_tier[3] = {0, 0, 0}, child_tier[3] = {0, 0, 0};
  for (const Event& e : r.events(EventType::PoliceCaseOpened)) (e.subject >= kChildFrom ? child_tier : adult_tier)[e.a]++;
  CHECK(adult_tier[0] + adult_tier[1] + adult_tier[2] == kAdults);
  CHECK(static_cast<double>(adult_tier[0]) / kAdults == doctest::Approx(0.31).epsilon(0.1));
  CHECK(static_cast<double>(adult_tier[1]) / kAdults == doctest::Approx(0.58).epsilon(0.06));
  CHECK(static_cast<double>(adult_tier[2]) / kAdults == doctest::Approx(0.11).epsilon(0.2));
  CHECK(static_cast<double>(child_tier[0]) / kChildren == doctest::Approx(0.14).epsilon(0.2));
  CHECK(static_cast<double>(child_tier[1]) / kChildren == doctest::Approx(0.83).epsilon(0.05));
  CHECK(r.police->queued() > 0);  // two searches at once: nearly everyone waits
  for (int i = 1; i <= kAdults; ++i) r.end(static_cast<PopId>(i));
  r.world->run_ticks(1);
  int found[3] = {0, 0, 0};
  for (const Event& e : r.events(EventType::PoliceCaseResolved))
    if (e.subject < kChildFrom) found[e.a]++;
  CHECK(found[0] + found[1] + found[2] == kAdults);
  CHECK(static_cast<double>(found[0]) / kAdults == doctest::Approx(0.39).epsilon(0.08));
  CHECK(static_cast<double>(found[1]) / kAdults == doctest::Approx(0.33).epsilon(0.1));
  CHECK(static_cast<double>(found[2]) / kAdults == doctest::Approx(0.28).epsilon(0.1));
}

TEST_CASE("the cases jump as they step, and a save carries the queue (A1)") {
  const auto drive = [](PoliceRig& r, bool jump) {
    for (PopId p : std::initializer_list<PopId>{1, 2, 3, kChildFrom}) r.begin(p);
    const std::int64_t end = 2 * kTicksPerGameDay;
    if (jump) r.world->jump(end);
    else r.world->run_ticks(end);
    r.end(2);
    r.world->run_ticks(1);
    if (jump) r.world->jump(end + kTicksPerGameDay);
    else r.world->run_ticks(kTicksPerGameDay - 1);
  };
  PoliceParams one;
  one.searches_at_once = 1;
  PoliceRig stepped(one), jumped(one);
  drive(stepped, false);
  drive(jumped, true);
  CHECK(stepped.world->state_hash() == jumped.world->state_hash());
  CHECK(stepped.police->queued() == jumped.police->queued());
  REQUIRE(stepped.police->queued() > 0);

  PoliceRig a(one), b(one);
  for (PopId p : std::initializer_list<PopId>{1, 2, 3}) a.begin(p);
  a.world->jump(kReportedAfter + 5);
  b.world->load(a.world->save());
  REQUIRE(b.police->cases().size() == 3);
  CHECK(b.police->queued() == 2);
  a.end(1);
  b.end(1);
  a.world->jump(kTicksPerGameDay);
  b.world->jump(kTicksPerGameDay);
  CHECK(a.world->state_hash() == b.world->state_hash());
}

TEST_CASE("in the one world, every report opens a case and the police run the search (A1)" * doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 t = tier0::make_run(1);
  t.world->check_every_tick = false;
  REQUIRE(t.police);
  t.missing->set_rate_scale(8000);
  t.missing->redraw(*t.world, 0);
  t.world->jump(200 * kTicksPerGameDay);
  int reports = 0, opened = 0, resolved = 0, searches = 0, suspended = 0;
  for (const Event& e : t.world->log.all()) {
    reports += e.type == EventType::MissingReported;
    opened += e.type == EventType::PoliceCaseOpened;
    resolved += e.type == EventType::PoliceCaseResolved;
    searches += e.type == EventType::SearchStarted;
    suspended += e.type == EventType::SearchSuspended;
  }
  CHECK(reports > 3);
  CHECK(opened == reports);
  CHECK(static_cast<std::int64_t>(opened) == t.police->opened_total());
  CHECK(resolved + static_cast<int>(t.police->cases().size()) == opened);
  CHECK(searches <= opened + suspended);  // a suspended search is started again (spec 65)
  CHECK(t.police->active_searches() <= 2);
}
