// Spec 59: broken commitments are noticed. A kept commitment costs nothing; a broken one starts the ladder
// (phone, ask around, report missing, search); a phoned-in absence and an absence nobody expects start nothing.
#include <doctest.h>

#include <memory>
#include <string>
#include <vector>

#include "zs/expectations.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr std::int64_t kMin = kTicksPerGameHour / 60;
constexpr PopId kOfficer = 1, kBaker = 2, kHermit = 3;
constexpr PopId kSupervisor = 10, kSpouse = 11, kMother = 20;

// The officer is due at work an hour in, with the supervisor and then the spouse holding the commitment; the baker is
// due home two hours in, held by her mother; the hermit is expected nowhere.
std::optional<Commitment> schedule(PopId pop, std::int64_t) {
  if (pop == kOfficer) return Commitment{CommitmentKind::ShiftStart, 60 * kMin, {kSupervisor, kSpouse}};
  if (pop == kBaker) return Commitment{CommitmentKind::ArriveHome, 120 * kMin, {kMother}};
  return std::nullopt;
}

struct Rig {
  std::unique_ptr<World> world;
  Expectations* exp = nullptr;
  explicit Rig(std::uint64_t seed = 1, NextCommitment next = schedule) {
    world = std::make_unique<World>(seed);
    world->check_every_tick = false;
    auto e = std::make_unique<Expectations>(load_expectation_params(tier0::data_dir() + "/life/expectations.toml"), std::move(next));
    exp = e.get();
    world->add_system(std::move(e));
    world->close_setup();
  }
  void begin(PopId pop, AbsenceCause cause = AbsenceCause::Possessed, bool phoned_in = false) {
    world->submit({0, AbsenceBegin, pop, static_cast<std::int64_t>(cause), phoned_in ? 1 : 0});
  }
  void end(PopId pop) { world->submit({0, AbsenceEnd, pop, 0, 0}); }
  // Events of a type, in order.
  std::vector<Event> events(EventType type) const {
    std::vector<Event> out;
    for (const Event& e : world->log.all())
      if (e.type == type) out.push_back(e);
    return out;
  }
};

}  // namespace

TEST_CASE("the ladder's timings load from data, and a bad file is refused (A1)") {
  const ExpectationParams p = load_expectation_params(tier0::data_dir() + "/life/expectations.toml");
  const LadderTimes& shift = p.times[static_cast<int>(CommitmentKind::ShiftStart)];
  const LadderTimes& home = p.times[static_cast<int>(CommitmentKind::ArriveHome)];
  CHECK(shift.tolerance_min == 30);
  CHECK(home.tolerance_min == 60);
  CHECK(shift.report_after_min >= shift.ask_after_min);
  CHECK(home.report_after_min >= home.ask_after_min);
  CHECK_THROWS(load_expectation_params(tier0::data_dir() + "/life/no_such_file.toml"));
}

TEST_CASE("a commitment kept costs nothing: back before the tolerance runs out, nobody hears of it (A1)") {
  Rig r;
  r.begin(kOfficer);
  r.world->run_ticks(1);
  REQUIRE(r.exp->cases().size() == 1);
  CHECK(r.exp->cases()[0].stage == AbsenceStage::Waiting);
  r.world->jump(80 * kMin);  // the shift started 20 minutes ago; the tolerance is 30
  r.end(kOfficer);
  r.world->run_ticks(1);
  CHECK(r.exp->cases().empty());
  CHECK(r.events(EventType::CommitmentBroken).empty());
  CHECK(r.events(EventType::HolderPhoned).empty());
  const auto ended = r.events(EventType::AbsenceEnded);
  REQUIRE(ended.size() == 1);
  CHECK(ended[0].a == static_cast<std::int64_t>(AbsenceStage::Waiting));
  CHECK(r.events(EventType::CaseClosed).empty());  // no case was ever opened
}

TEST_CASE("a broken commitment runs the ladder at the data's times, each step caused by the last (A1)") {
  Rig r;
  r.begin(kOfficer, AbsenceCause::Possessed);
  const LadderTimes shift = load_expectation_params(tier0::data_dir() + "/life/expectations.toml").times[0];
  const std::int64_t broken = (60 + shift.tolerance_min) * kMin;  // due at 60, plus the tolerance
  r.world->jump(broken - 1);
  CHECK(r.events(EventType::CommitmentBroken).empty());
  r.world->jump(broken + 1);
  const auto b = r.events(EventType::CommitmentBroken);
  REQUIRE(b.size() == 1);
  CHECK(b[0].tick == broken);
  CHECK(b[0].subject == kOfficer);
  CHECK(b[0].a == static_cast<std::int64_t>(CommitmentKind::ShiftStart));
  CHECK(b[0].b == static_cast<std::int64_t>(kSupervisor));
  const auto phoned = r.events(EventType::HolderPhoned);
  REQUIRE(phoned.size() == 1);
  CHECK(phoned[0].subject == kSupervisor);  // the first holder phones
  CHECK(phoned[0].a == static_cast<std::int64_t>(kOfficer));
  CHECK(phoned[0].tick == broken);

  r.world->jump(broken + shift.ask_after_min * kMin + 1);
  const auto asked = r.events(EventType::HolderAskedAround);
  REQUIRE(asked.size() == 1);
  CHECK(asked[0].tick == broken + shift.ask_after_min * kMin);
  CHECK(asked[0].b == 2);  // both holders asked around

  r.world->jump(broken + shift.report_after_min * kMin + 1);
  const auto reported = r.events(EventType::MissingReported);
  REQUIRE(reported.size() == 1);
  CHECK(reported[0].tick == broken + shift.report_after_min * kMin);
  CHECK(reported[0].a == static_cast<std::int64_t>(kSpouse));  // the last holder reports

  r.world->jump(broken + (shift.report_after_min + shift.search_after_min) * kMin + 1);
  const auto search = r.events(EventType::SearchStarted);
  REQUIRE(search.size() == 1);
  CHECK(search[0].tick == broken + (shift.report_after_min + shift.search_after_min) * kMin);
  CHECK(r.exp->cases()[0].stage == AbsenceStage::Searching);
  CHECK(r.exp->next_wake(*r.world, r.world->clock.tick) == System::kNever);  // the ladder has nothing left to do

  // The cause chain of the search leads back to the absence's start, through every step.
  const auto chain = r.world->log.cause_chain(search[0].id);
  std::vector<EventType> types;
  for (const Event& e : chain) types.push_back(e.type);
  const std::vector<EventType> want{EventType::SearchStarted, EventType::MissingReported, EventType::HolderAskedAround,
                                    EventType::HolderPhoned, EventType::CommitmentBroken, EventType::AbsenceBegan,
                                    EventType::Intent};
  REQUIRE(types.size() >= want.size() - 1);
  for (std::size_t i = 0; i + 1 < want.size(); ++i) CHECK(types[i] == want[i]);
}

TEST_CASE("a phoned-in absence, and an absence nobody expects, start nothing (A1)") {
  Rig r;
  r.begin(kOfficer, AbsenceCause::Ill, true);  // called in sick: an ordinary absence
  r.begin(kHermit, AbsenceCause::Chose);       // expected nowhere
  r.world->jump(3 * kTicksPerGameDay);
  CHECK(r.exp->cases().size() == 2);
  CHECK(r.exp->cases()[0].stage == AbsenceStage::Ordinary);
  CHECK(r.exp->cases()[1].stage == AbsenceStage::Ordinary);
  CHECK(r.events(EventType::CommitmentBroken).empty());
  CHECK(r.events(EventType::HolderPhoned).empty());
  CHECK(r.events(EventType::MissingReported).empty());
  CHECK(r.events(EventType::AbsenceBegan).size() == 2);
}

TEST_CASE("coming back after the report closes the case and says how far it got (A1)") {
  Rig r;
  r.begin(kBaker);
  const LadderTimes home = load_expectation_params(tier0::data_dir() + "/life/expectations.toml").times[1];
  const std::int64_t broken = (120 + home.tolerance_min) * kMin;
  r.world->jump(broken + (home.report_after_min + 5) * kMin);
  REQUIRE(r.exp->cases().size() == 1);
  CHECK(r.exp->cases()[0].stage == AbsenceStage::Reported);
  r.end(kBaker);
  r.world->run_ticks(1);
  CHECK(r.exp->cases().empty());
  const auto ended = r.events(EventType::AbsenceEnded);
  REQUIRE(ended.size() == 1);
  CHECK(ended[0].a == static_cast<std::int64_t>(AbsenceStage::Reported));
  CHECK(ended[0].b > 0);  // ticks gone
  const auto closed = r.events(EventType::CaseClosed);
  REQUIRE(closed.size() == 1);
  CHECK(closed[0].subject == kBaker);
  CHECK(r.events(EventType::SearchStarted).empty());  // she was back before the search opened
}

TEST_CASE("an absence that starts after the commitment fell due is broken at once (A1)") {
  Rig r;
  r.world->jump(5 * kTicksPerGameHour);
  r.begin(kOfficer);
  r.world->run_ticks(1);
  const auto b = r.events(EventType::CommitmentBroken);
  REQUIRE(b.size() == 1);
  CHECK(b[0].tick == 5 * kTicksPerGameHour);
}

TEST_CASE("two begins and an end without a begin are refused (A1)") {
  Rig r;
  r.begin(kOfficer);
  r.begin(kOfficer);
  r.end(kBaker);
  r.world->run_ticks(1);
  const auto refused = r.events(EventType::ExpectationRefused);
  REQUIRE(refused.size() == 2);
  CHECK(refused[0].b == 1);  // already absent
  CHECK(refused[1].b == 2);  // not absent
  CHECK(r.exp->cases().size() == 1);
}

TEST_CASE("a world with open absences jumps exactly as it steps, and a save carries the ladder (A1)") {
  const auto drive = [](Rig& r, bool jump) {
    r.begin(kOfficer);
    r.begin(kBaker, AbsenceCause::Ill);
    r.begin(kHermit);
    const std::int64_t end = 2 * kTicksPerGameDay;
    if (jump) r.world->jump(end);
    else r.world->run_ticks(end);
  };
  Rig stepped, jumped;
  drive(stepped, false);
  drive(jumped, true);
  CHECK(stepped.world->state_hash() == jumped.world->state_hash());
  CHECK(stepped.world->log.all().size() == jumped.world->log.all().size());

  Rig a, b;
  a.begin(kOfficer);
  a.begin(kBaker);
  a.world->jump(3 * kTicksPerGameHour + 15 * kMin);  // mid-ladder
  const auto bytes = a.world->save();
  b.world->load(bytes);
  REQUIRE(b.exp->cases().size() == 2);
  CHECK(b.exp->cases()[0].stage == a.exp->cases()[0].stage);
  a.world->jump(2 * kTicksPerGameDay);
  b.world->jump(2 * kTicksPerGameDay);
  CHECK(a.world->state_hash() == b.world->state_hash());
}

TEST_CASE("an idle expectations system costs no wake ticks (A1)") {
  Rig r;
  CHECK(r.exp->next_wake(*r.world, 0) == System::kNever);
  r.begin(kOfficer);
  r.world->run_ticks(1);
  CHECK(r.exp->next_wake(*r.world, r.world->clock.tick) == (60 + 30) * kMin);
}

TEST_CASE("in the one world, a household member's absence is noticed by the household, and a lone pop's is not (A1)" *
          doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 t = tier0::make_run(1);
  t.world->check_every_tick = false;
  REQUIRE(t.expectations);
  const auto& people = t.census->people();
  // A living adult with another living adult in the household, and a living pop alone in its household.
  PopId held = kNoPop, alone = kNoPop;
  for (std::size_t i = 0; i < people.size(); ++i) {
    const Person& p = people[i];
    if (is_dead(p.fate) || p.age < 18 || p.household.empty()) continue;
    int others = 0;
    for (std::size_t j = 0; j < people.size(); ++j)
      if (j != i && people[j].household == p.household && !is_dead(people[j].fate) && people[j].age >= 16) ++others;
    if (others > 0 && held == kNoPop) held = census_pop_id(static_cast<std::uint32_t>(i));
    if (others == 0 && alone == kNoPop) alone = census_pop_id(static_cast<std::uint32_t>(i));
  }
  REQUIRE(held != kNoPop);
  t.world->submit({0, AbsenceBegin, held, static_cast<std::int64_t>(AbsenceCause::Possessed), 0});
  if (alone != kNoPop) t.world->submit({0, AbsenceBegin, alone, static_cast<std::int64_t>(AbsenceCause::Possessed), 0});
  t.world->jump(3 * kTicksPerGameDay);
  const Expectations::Case* c = t.expectations->find(held);
  REQUIRE(c);
  CHECK(c->stage >= AbsenceStage::Broken);  // somebody expected it within three days, and the household acted
  CHECK(!c->holders.empty());
  bool broken = false;
  for (const Event& e : t.world->log.all())
    if (e.type == EventType::CommitmentBroken && e.subject == held) broken = true;
  CHECK(broken);
  if (alone != kNoPop) {
    const Expectations::Case* lone = t.expectations->find(alone);
    REQUIRE(lone);
    CHECK(lone->stage == AbsenceStage::Ordinary);  // nobody lives with it: nobody notices
  }
}
