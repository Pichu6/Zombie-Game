// Spec 65: the police force as pops with shifts. Officers on duty and present set how many searches the force can run; a
// shift change or an officer's absence can suspend a search and a free staff restarts it.
#include <doctest.h>

#include <cstdio>
#include <filesystem>
#include <functional>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "zs/missing_persons.h"
#include "zs/police.h"
#include "zs/police_staff.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr std::int64_t kMin = kTicksPerGameHour / 60;

struct Force {
  tier0::Tier0 t = tier0::make_run(1);
  std::shared_ptr<PoliceStaff> staff;
  explicit Force(int officers, ForceParams params = {}) {
    t.world->check_every_tick = false;
    staff = std::make_shared<PoliceStaff>(params, std::make_shared<TimeUseTable>(timeuse_for(tier0::data_dir(), "", "upper_middle")),
                                          t.calendar, 7);
    for (int i = 0; i < officers; ++i) staff->add_officer(census_pop_id(1000 + static_cast<std::uint32_t>(i)), 25 + (i * 7) % 31, i % 4 == 0 ? 'f' : 'm');
  }
};

}  // namespace

TEST_CASE("the force's parameters load from data, and a bad table is refused (A1)") {
  const std::string path = tier0::data_dir() + "/institutions/police/force.toml";
  const ForceParams p = load_force_params(path, "", "high");
  CHECK(p.officers_per_search >= 1);
  CHECK(p.min_officers >= 3);  // a small town's station still keeps three shifts
  CHECK_THROWS(load_force_params(tier0::data_dir() + "/institutions/police/no_such.toml", "", "high"));
  CHECK_THROWS(load_force_params(path, "", "no_such_group"));
  const std::string bad_path = (std::filesystem::temp_directory_path() / "zs_bad_force.toml").string();
  {
    std::ofstream bad(bad_path);
    bad << "[t]\nofficers_per_search = 0\nmin_officers = 6\n[index]\nhigh = \"t\"\n";
  }
  CHECK_THROWS(load_force_params(bad_path, "t", "high"));  // a search that needs nobody
  std::remove(bad_path.c_str());
}

TEST_CASE("officers work in turn on day, evening and night shifts, so the station is covered round the clock (A1)") {
  Force f(36);
  // Over a week, every hour of the day sees officers on duty; the day shift covers the morning, the night shift the small hours.
  for (int hour = 0; hour < 24; ++hour) {
    int most = 0;
    for (int day = 0; day < 7; ++day) most = std::max(most, f.staff->on_duty_count(day * kTicksPerGameDay + (hour - 8) * kTicksPerGameHour + 30 * kMin, nullptr));
    CAPTURE(hour);
    CHECK(most > 0);
  }
  // Not everyone is on at once: the force is spread over three shifts and rest days.
  for (int day = 0; day < 7; ++day) CHECK(f.staff->on_duty_count(day * kTicksPerGameDay + 2 * kTicksPerGameHour, nullptr) < 36);
  // Deterministic: the same officer, the same tick, the same answer; a non-officer is never on duty.
  for (PopId pop : f.staff->officers()) CHECK(f.staff->on_duty(pop, 3 * kTicksPerGameDay + kTicksPerGameHour) == f.staff->on_duty(pop, 3 * kTicksPerGameDay + kTicksPerGameHour));
  CHECK_FALSE(f.staff->on_duty(5, 0));
  CHECK_FALSE(f.staff->is_officer(5));
  CHECK(f.staff->is_officer(f.staff->officers()[0]));
}

TEST_CASE("capacity is officers on duty and present over the officers a search takes; an absent officer is staff short (A1)") {
  ForceParams params;
  params.officers_per_search = 2;
  Force f(30, params);
  // Find a tick with some officers on duty.
  std::int64_t tick = 0;
  while (f.staff->on_duty_count(tick, nullptr) < 4) tick += kTicksPerGameHour;
  const int on_duty = f.staff->on_duty_count(tick, nullptr);
  CHECK(f.staff->capacity(tick, nullptr) == on_duty / 2);
  // One of them goes absent: the ladder system has an open case for the pop, so it does not count.
  PopId officer = 0;
  for (PopId p : f.staff->officers())
    if (f.staff->on_duty(p, tick)) officer = p;
  REQUIRE(officer != 0);
  f.t.world->submit({0, AbsenceBegin, officer, static_cast<std::int64_t>(AbsenceCause::Unknown), 0});
  f.t.world->run_ticks(1);
  REQUIRE(f.t.expectations->find(officer));
  CHECK(f.staff->on_duty_count(tick, f.t.expectations) == on_duty - 1);
}

TEST_CASE("the next change is a later tick at which some officer's duty begins or ends (A1)") {
  Force f(24);
  std::int64_t now = 0;
  for (int i = 0; i < 40; ++i) {
    const std::int64_t next = f.staff->next_change(now);
    REQUIRE(next > now);
    REQUIRE(next < System::kNever);
    bool differs = false;
    for (PopId p : f.staff->officers()) differs = differs || f.staff->on_duty(p, next - 1) != f.staff->on_duty(p, next);
    CHECK(differs);  // somebody's status really changes there
    now = next;
  }
  CHECK(now < 4 * kTicksPerGameDay);  // forty changes in under four days: shifts start and end through the day
}

TEST_CASE("an officer's next shift is a police employer's, with the shift's length and no holder at home (A1)") {
  Force f(12);
  const PopId officer = f.staff->officers()[0];
  const auto c = f.staff->next_commitment(officer, 0);
  REQUIRE(c);
  CHECK(c->kind == CommitmentKind::ShiftStart);
  CHECK(c->employer == EmployerKind::Police);
  CHECK(c->minutes > 0);
  CHECK(c->holders.empty());
  CHECK(c->due_tick >= 0);
  const auto later = f.staff->next_commitment(officer, c->due_tick + 1);
  REQUIRE(later);
  CHECK(later->due_tick > c->due_tick);
  CHECK(f.staff->next_commitment(officer, 0)->due_tick == c->due_tick);  // the same answer again
  CHECK_FALSE(f.staff->next_commitment(5, 0));  // not an officer
}


namespace {

// A world with the ladder and the police, a staffing that is a function of the tick (as the real one is: a shift's hours are
// fixed), and every pop expected 30 minutes after it goes away. `capacity_at` says how many searches can run at a tick; `changes`
// are the ticks at which that may change.
struct StaffRig {
  std::unique_ptr<World> world;
  Expectations* exp = nullptr;
  PoliceCases* police = nullptr;
  std::function<int(std::int64_t)> capacity_at = [](std::int64_t) { return 1; };
  std::vector<std::int64_t> changes;
  StaffRig() {
    world = std::make_unique<World>(1);
    world->check_every_tick = false;
    auto e = std::make_unique<Expectations>(
        load_expectation_params(tier0::data_dir() + "/life/expectations.toml"), [](PopId pop, std::int64_t from) -> std::optional<Commitment> {
          return Commitment{CommitmentKind::ShiftStart, from + 30 * kMin, {pop + 1'000'000}};
        });
    e->set_external_search(true);
    PoliceParams params;
    params.adult_tiers[0] = 1;
    params.adult_tiers[1] = 0;
    params.adult_tiers[2] = 0;
    auto p = std::make_unique<PoliceCases>(params, e.get(), [](PopId) { return false; });
    p->set_staffing([this](const World& w) { return capacity_at(w.clock.tick); },
                    [this](const World&, std::int64_t now) {
                      std::int64_t next = System::kNever;
                      for (std::int64_t c : changes)
                        if (c > now) next = std::min(next, c);
                      return next;
                    });
    e->add_listener(p.get());
    exp = e.get();
    police = p.get();
    world->add_system(std::move(e));
    world->add_system(std::move(p));
    world->close_setup();
  }
  void begin(PopId pop) { world->submit({0, AbsenceBegin, pop, static_cast<std::int64_t>(AbsenceCause::Unknown), 0}); }
  std::vector<Event> events(EventType type) const {
    std::vector<Event> out;
    for (const Event& e : world->log.all())
      if (e.type == type) out.push_back(e);
    return out;
  }
};

constexpr std::int64_t kReportedAfter = 420 * kMin;

}  // namespace

TEST_CASE("a shift change that leaves too few officers suspends a search, and the next shift restarts it (A1)") {
  StaffRig r;
  const std::int64_t change = kReportedAfter + 2 * kTicksPerGameHour, morning = change + 6 * kTicksPerGameHour;
  r.changes = {change, morning};
  r.capacity_at = [=](std::int64_t tick) { return tick < change ? 1 : tick < morning ? 0 : 1; };  // nobody takes over at the change
  r.begin(1);
  r.world->jump(kReportedAfter + 2);
  REQUIRE(r.exp->find(1)->stage == AbsenceStage::Searching);
  r.world->jump(change + 1);
  CHECK(r.exp->find(1)->stage == AbsenceStage::Reported);  // back to waiting for staff
  CHECK(r.police->active_searches() == 0);
  CHECK(r.police->has_record(1));  // the case is still open
  const auto suspended = r.events(EventType::SearchSuspended);
  REQUIRE(suspended.size() == 1);
  CHECK(suspended[0].tick == change);
  CHECK(suspended[0].subject == 1);
  r.world->jump(morning + 1);  // the day shift starts
  CHECK(r.exp->find(1)->stage == AbsenceStage::Searching);  // restarted
  CHECK(r.events(EventType::SearchStarted).size() == 2);
  CHECK(r.police->active_searches() == 1);
}

TEST_CASE("when staff are short the lowest risk and the newest search is the one suspended (A1)") {
  StaffRig r;
  const std::int64_t change = 3 * kReportedAfter;
  r.changes = {change};
  r.capacity_at = [=](std::int64_t tick) { return tick < change ? 2 : 1; };
  r.begin(1);
  r.world->jump(kReportedAfter + 2);
  r.begin(2);
  r.world->jump(2 * kReportedAfter + 4);
  CHECK(r.police->active_searches() == 2);
  r.world->jump(change + 1);
  CHECK(r.police->active_searches() == 1);
  CHECK(r.exp->find(1)->stage == AbsenceStage::Searching);  // the older case keeps its staff
  CHECK(r.exp->find(2)->stage == AbsenceStage::Reported);   // the newer one waits
  REQUIRE(r.events(EventType::SearchSuspended).size() == 1);
  CHECK(r.events(EventType::SearchSuspended)[0].subject == 2);
}

TEST_CASE("an officer going absent suspends a search at once, without waiting for a shift change (A1)") {
  StaffRig r;
  bool officer_absent = false;
  r.capacity_at = [&](std::int64_t) { return officer_absent ? 0 : 1; };
  r.begin(1);
  r.world->jump(kReportedAfter + 2);
  REQUIRE(r.exp->find(1)->stage == AbsenceStage::Searching);
  officer_absent = true;  // the only officer on duty is the one who just did not turn up
  r.begin(2);             // an absence begins: the listener looks at the staff
  r.world->run_ticks(2);
  CHECK(r.exp->find(1)->stage == AbsenceStage::Reported);
  CHECK(r.events(EventType::SearchSuspended).size() == 1);
}

TEST_CASE("suspension and restart jump as they step, and a save carries the cases (A1)") {
  const std::int64_t first = kReportedAfter + 2 * kTicksPerGameHour, second = first + 5 * kTicksPerGameHour;
  const auto setup = [&](StaffRig& r) {
    r.changes = {first, second};
    r.capacity_at = [=](std::int64_t tick) { return tick < first ? 2 : tick < second ? 0 : 1; };
    r.begin(1);
    r.begin(2);
  };
  StaffRig stepped, jumped;
  setup(stepped);
  setup(jumped);
  stepped.world->run_ticks(second + kTicksPerGameDay);
  jumped.world->jump(second + kTicksPerGameDay);
  CHECK(stepped.world->state_hash() == jumped.world->state_hash());
  CHECK(stepped.police->active_searches() == jumped.police->active_searches());
  CHECK(stepped.events(EventType::SearchSuspended).size() == jumped.events(EventType::SearchSuspended).size());
  REQUIRE(stepped.events(EventType::SearchSuspended).size() == 2);  // both searches lost their staff at the second change
  CHECK(stepped.police->active_searches() == 1);                    // and one was restarted

  StaffRig a, b;
  setup(a);
  setup(b);
  a.world->jump(first + 100);
  b.world->load(a.world->save());
  a.world->jump(second + kTicksPerGameDay);
  b.world->jump(second + kTicksPerGameDay);
  CHECK(a.world->state_hash() == b.world->state_hash());
}

TEST_CASE("in the one world, the force's searches never exceed its staff on duty (A1)" * doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 t = tier0::make_run(1);
  t.world->check_every_tick = false;
  REQUIRE(t.force);
  CHECK(t.force->officers().size() == 80);  // spec 69: 1.6 per 1,000 for a town of 50,000 (BJS, SV-5)
  t.missing->set_rate_scale(8000);
  t.missing->redraw(*t.world, 0);
  for (int day = 1; day <= 20; ++day) {
    t.world->jump(day * kTicksPerGameDay);
    CHECK(t.police->active_searches() <= t.force->capacity(t.world->clock.tick, t.expectations));
  }
  int opened = 0;
  for (const Event& e : t.world->log.all()) opened += e.type == EventType::PoliceCaseOpened;
  CHECK(opened > 0);
}
