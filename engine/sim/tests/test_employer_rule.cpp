// Spec 61: the employer's absence rule. Shifts missed without contact are counted; at the limit the employer separates (or
// opens a process); a missing-person report pauses the count; a return meets the employer.
#include <doctest.h>

#include <memory>
#include <string>
#include <vector>

#include "zs/expectations.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr std::int64_t kMin = kTicksPerGameHour / 60;
constexpr PopId kLone = 1, kHeld = 2, kCop = 3, kMother = 20;

// A shift every day at 09:00 (tick 0 is 08:00): a private employer for the first two, the police for the third. The
// household pop has a holder; the others live alone.
std::optional<Commitment> daily_shift(PopId pop, std::int64_t from) {
  const std::int64_t due = ((from + kTicksPerGameDay - 1) / kTicksPerGameDay) * kTicksPerGameDay + 60 * kMin;
  Commitment c{CommitmentKind::ShiftStart, due, {}, pop == kCop ? EmployerKind::Police : EmployerKind::Private};
  if (pop == kHeld) c.holders = {kMother};
  return c;
}

struct JobRig {
  std::unique_ptr<World> world;
  Expectations* exp = nullptr;
  explicit JobRig(EmployerPolicy policy = {}, std::uint64_t seed = 1) {
    world = std::make_unique<World>(seed);
    world->check_every_tick = false;
    auto e = std::make_unique<Expectations>(load_expectation_params(tier0::data_dir() + "/life/expectations.toml"), daily_shift, policy);
    exp = e.get();
    world->add_system(std::move(e));
    world->close_setup();
  }
  void begin(PopId pop, bool phoned_in = false) { world->submit({0, AbsenceBegin, pop, static_cast<std::int64_t>(AbsenceCause::Chose), phoned_in ? 1 : 0}); }
  void end(PopId pop) { world->submit({0, AbsenceEnd, pop, 0, 0}); }
  std::vector<Event> events(EventType type, PopId pop = 0) const {
    std::vector<Event> out;
    for (const Event& e : world->log.all())
      if (e.type == type && (pop == 0 || e.subject == pop)) out.push_back(e);
    return out;
  }
};

constexpr std::int64_t kFirstCount = 90 * kMin;  // the shift is due at 60 minutes, the tolerance is 30

}  // namespace

TEST_CASE("the employer's policy loads per country and income group, and a bad file is refused (A1)") {
  const std::string path = tier0::data_dir() + "/employment/absence_policy.toml";
  const EmployerPolicy us = load_employer_policy(path, "", "upper_middle");
  CHECK(us.abandonment_days == 3);
  CHECK(us.automatic_separation);
  CHECK(us.pause_on_report);
  const EmployerPolicy gb = load_employer_policy(path, "", "high");
  CHECK_FALSE(gb.automatic_separation);  // no automatic resignation: a process instead
  CHECK(load_employer_policy(path, "us", "high").automatic_separation);  // a table of its own wins
  CHECK_THROWS(load_employer_policy(tier0::data_dir() + "/employment/no_such.toml", "", "high"));
  CHECK_THROWS(load_employer_policy(path, "", "no_such_group"));
}

TEST_CASE("three scheduled days missed without contact: the employer separates, and the pop has no more shifts (A1)") {
  JobRig r;
  r.begin(kLone);
  r.world->jump(6 * kTicksPerGameDay);
  const auto missed = r.events(EventType::ShiftMissed, kLone);
  REQUIRE(missed.size() == 3);  // counting stops at the separation
  for (int i = 0; i < 3; ++i) {
    CHECK(missed[i].a == i + 1);
    CHECK(missed[i].tick == i * kTicksPerGameDay + kFirstCount);
    CHECK(missed[i].b == static_cast<std::int64_t>(EmployerKind::Private));
  }
  const auto sep = r.events(EventType::Separated, kLone);
  REQUIRE(sep.size() == 1);
  CHECK(sep[0].tick == missed[2].tick);
  CHECK(sep[0].a == 3);
  CHECK(r.exp->lost_job(kLone));
  CHECK(r.events(EventType::CommitmentBroken).empty());  // nobody lives with the pop: no ladder
  // The separation's cause chain leads back to the absence, through every missed shift.
  std::vector<EventType> types;
  for (const Event& e : r.world->log.cause_chain(sep[0].id)) types.push_back(e.type);
  REQUIRE(types.size() >= 5);
  CHECK(types[0] == EventType::Separated);
  CHECK(types[1] == EventType::ShiftMissed);
  CHECK(types[2] == EventType::ShiftMissed);
  CHECK(types[3] == EventType::ShiftMissed);
  CHECK(types[4] == EventType::AbsenceBegan);
  r.end(kLone);
  r.world->run_ticks(1);
  CHECK(r.events(EventType::ReturnedToWork).empty());  // the job is gone: no meeting
  CHECK(r.exp->lost_job(kLone));
}

TEST_CASE("a missing-person report pauses the count; on return the employer forgives it (A1)") {
  JobRig r;
  r.begin(kHeld);
  r.world->jump(5 * kTicksPerGameDay);
  REQUIRE(r.events(EventType::MissingReported, kHeld).size() == 1);
  const auto missed = r.events(EventType::ShiftMissed, kHeld);
  REQUIRE(missed.size() == 1);  // counted once before the report, then paused
  CHECK(missed[0].tick == kFirstCount);
  CHECK(r.events(EventType::Separated, kHeld).empty());
  CHECK_FALSE(r.exp->lost_job(kHeld));
  CHECK(r.exp->find(kHeld)->paused);
  r.end(kHeld);
  r.world->run_ticks(1);
  const auto back = r.events(EventType::ReturnedToWork, kHeld);
  REQUIRE(back.size() == 1);
  CHECK(back[0].a == static_cast<std::int64_t>(ReturnOutcome::Forgiven));
  CHECK(back[0].b == 1);
}

TEST_CASE("back after one or two missed shifts without a report is a warning; before the first is counted, nothing (A1)") {
  JobRig r;
  r.begin(kLone);
  r.world->jump(kTicksPerGameDay + 2 * kTicksPerGameHour);  // two shifts counted
  REQUIRE(r.events(EventType::ShiftMissed, kLone).size() == 2);
  r.end(kLone);
  r.world->run_ticks(1);
  const auto back = r.events(EventType::ReturnedToWork, kLone);
  REQUIRE(back.size() == 1);
  CHECK(back[0].a == static_cast<std::int64_t>(ReturnOutcome::Warning));
  CHECK(back[0].b == 2);
  CHECK_FALSE(r.exp->lost_job(kLone));

  JobRig q;
  q.begin(kLone);
  q.world->jump(80 * kMin);  // before the count at 90 minutes
  q.end(kLone);
  q.world->run_ticks(1);
  CHECK(q.events(EventType::ShiftMissed).empty());
  CHECK(q.events(EventType::ReturnedToWork).empty());
}

TEST_CASE("a police employer opens a conduct case after the return instead of separating (A1)") {
  JobRig r;
  r.begin(kCop);
  r.world->jump(6 * kTicksPerGameDay);
  CHECK(r.events(EventType::ShiftMissed, kCop).size() == 3);
  CHECK(r.events(EventType::Separated, kCop).empty());
  CHECK_FALSE(r.exp->lost_job(kCop));
  CHECK(r.events(EventType::ConductCaseOpened, kCop).empty());  // not yet: it follows the return
  r.end(kCop);
  r.world->run_ticks(1);
  const auto conduct = r.events(EventType::ConductCaseOpened, kCop);
  REQUIRE(conduct.size() == 1);
  CHECK(conduct[0].a == 3);
  CHECK(r.events(EventType::ReturnedToWork, kCop).empty());
}

TEST_CASE("where the country has no automatic separation the employer opens a process instead (A1)") {
  EmployerPolicy gb;
  gb.automatic_separation = false;
  JobRig r(gb);
  r.begin(kLone);
  r.world->jump(6 * kTicksPerGameDay);
  CHECK(r.events(EventType::Separated).empty());
  const auto process = r.events(EventType::DisciplinaryOpened, kLone);
  REQUIRE(process.size() == 1);
  CHECK(process[0].a == 3);
  CHECK_FALSE(r.exp->lost_job(kLone));
  r.end(kLone);
  r.world->run_ticks(1);
  const auto back = r.events(EventType::ReturnedToWork, kLone);
  REQUIRE(back.size() == 1);
  CHECK(back[0].a == static_cast<std::int64_t>(ReturnOutcome::Disciplinary));
}

TEST_CASE("a phoned-in absence is never counted (A1)") {
  JobRig r;
  r.begin(kLone, true);
  r.world->jump(6 * kTicksPerGameDay);
  CHECK(r.events(EventType::ShiftMissed).empty());
  CHECK(r.events(EventType::Separated).empty());
  r.end(kLone);
  r.world->run_ticks(1);
  CHECK(r.events(EventType::ReturnedToWork).empty());
}

TEST_CASE("the employer's count jumps as it steps, and a save carries the count and the lost job (A1)") {
  const auto drive = [](JobRig& r, bool jump) {
    r.begin(kLone);
    r.begin(kHeld);
    r.begin(kCop);
    if (jump) r.world->jump(5 * kTicksPerGameDay);
    else r.world->run_ticks(5 * kTicksPerGameDay);
  };
  JobRig stepped, jumped;
  drive(stepped, false);
  drive(jumped, true);
  CHECK(stepped.world->state_hash() == jumped.world->state_hash());
  CHECK(stepped.world->log.all().size() == jumped.world->log.all().size());

  JobRig a, b;
  a.begin(kLone);
  a.begin(kHeld);
  a.world->jump(kTicksPerGameDay + 3 * kTicksPerGameHour);  // two counted for the lone pop, one then paused for the other
  const auto bytes = a.world->save();
  b.world->load(bytes);
  REQUIRE(b.exp->cases().size() == 2);
  CHECK(b.exp->cases()[0].missed == 2);
  CHECK(b.exp->cases()[1].paused);
  a.world->jump(5 * kTicksPerGameDay);
  b.world->jump(5 * kTicksPerGameDay);
  CHECK(a.world->state_hash() == b.world->state_hash());
  CHECK(a.exp->lost_job(kLone));
  JobRig c;
  c.world->load(a.world->save());
  CHECK(c.exp->lost_job(kLone));  // the lost job is state, not a log line
}
