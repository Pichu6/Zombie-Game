// Spec 63: the household after an absence or a lost job: a strain each holder carries, easing with time; past a limit the
// holder leaves the household.
#include <doctest.h>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "zs/bonds.h"
#include "zs/expectations.h"
#include "zs/missing_persons.h"
#include "zs/pop_system.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr std::int64_t kMin = kTicksPerGameHour / 60;
constexpr std::uint64_t kHousehold = 55;
constexpr int kSubject = 0, kHolderA = 1, kHolderB = 2, kLone = 3;  // census slots

PopId pop_of(int slot) { return census_pop_id(static_cast<std::uint32_t>(slot)); }

struct BondRig {
  std::unique_ptr<World> world;
  PopSystem* pops = nullptr;
  Expectations* exp = nullptr;
  Bonds* bonds = nullptr;
  std::shared_ptr<const Bonds*> link = std::make_shared<const Bonds*>(nullptr);

  explicit BondRig(EmployerPolicy policy = {}, std::uint64_t seed = 1) {
    world = std::make_unique<World>(seed);
    world->check_every_tick = false;
    auto p = std::make_unique<PopSystem>(*world);
    PopRegistry& reg = p->setup(*world);
    reg.add_cohort({1, 1, 0, 6});
    reg.add_roster(kHousehold, 1, RosterKind::Household);
    for (int slot : {kSubject, kHolderA, kHolderB}) reg.roster_join(kHousehold, pop_of(slot), 0.0);
    // The subject has a shift every day at 09:00; the two others at home hold it, unless they left the household.
    auto link_copy = link;
    const NextCommitment next = [link_copy](PopId pop, std::int64_t from) -> std::optional<Commitment> {
      if (pop != pop_of(kSubject) && pop != pop_of(kLone)) return std::nullopt;
      const std::int64_t due = ((from + kTicksPerGameDay - 1) / kTicksPerGameDay) * kTicksPerGameDay + 60 * kMin;
      Commitment c{CommitmentKind::ShiftStart, due, {}, EmployerKind::Private};
      c.minutes = 480;
      if (pop == pop_of(kSubject))
        for (int slot : {kHolderA, kHolderB})
          if (!(*link_copy && (*link_copy)->left_household(pop_of(slot)))) c.holders.push_back(pop_of(slot));
      return c;
    };
    auto x = std::make_unique<Expectations>(load_expectation_params(tier0::data_dir() + "/life/expectations.toml"), next, policy);
    auto b = std::make_unique<Bonds>(load_bond_params(tier0::data_dir() + "/life/household.toml"),
                                     [](PopId) -> std::uint64_t { return kHousehold; });
    x->set_listener(b.get());
    *link = b.get();
    pops = p.get();
    exp = x.get();
    bonds = b.get();
    world->add_system(std::move(p));
    world->add_system(std::move(x));
    world->add_system(std::move(b));
    world->close_setup();
  }
  void begin(int slot, AbsenceCause cause = AbsenceCause::Possessed, bool phoned_in = false) {
    world->submit({0, AbsenceBegin, pop_of(slot), static_cast<std::int64_t>(cause), phoned_in ? 1 : 0});
  }
  void end(int slot) { world->submit({0, AbsenceEnd, pop_of(slot), 0, 0}); }
  // An absence that reaches the report and not yet the search, from the next midnight: the shift is due at 60 minutes, the
  // tolerance 30, the report 360 after that (7.5 hours), the search an hour later: it ends at 8 hours.
  void absence_until_reported(int slot, AbsenceCause cause) {
    const std::int64_t day = ((world->clock.tick + kTicksPerGameDay - 1) / kTicksPerGameDay) * kTicksPerGameDay;
    world->jump(day);
    begin(slot, cause);
    world->jump(day + 480 * kMin);
    end(slot);
    world->run_ticks(1);
  }
  std::vector<Event> events(EventType type) const {
    std::vector<Event> out;
    for (const Event& e : world->log.all())
      if (e.type == type) out.push_back(e);
    return out;
  }
  bool present(int slot) const { return pops->registry().roster(kHousehold)->present(pop_of(slot), year_of_tick(world->clock.tick)); }
};

}  // namespace

TEST_CASE("the household parameters load, and a bad file is refused (A1)") {
  const BondParams p = load_bond_params(tier0::data_dir() + "/life/household.toml");
  CHECK(p.stage_milli[static_cast<int>(AbsenceStage::Reported)] == 200);
  CHECK(p.stage_milli[static_cast<int>(AbsenceStage::Waiting)] == 0);  // nobody noticed: nothing
  CHECK(p.cause_percent[static_cast<int>(AbsenceCause::Chose)] > p.cause_percent[static_cast<int>(AbsenceCause::Wandered)]);
  CHECK(p.half_life_days == doctest::Approx(30));
  CHECK(p.leave_threshold_milli == 900);
  CHECK_THROWS(load_bond_params(tier0::data_dir() + "/life/no_such.toml"));
  const std::string bad_path = (std::filesystem::temp_directory_path() / "zs_bad_household.toml").string();
  {
    std::ofstream bad(bad_path);
    bad << "[stage_milli]\nbroken=1\nasked=1\nreported=1\nsearching=1\n[cause_percent]\npossessed=100\nill=100\nchose=100\nunknown=100\n"
           "wandered=100\n[job_loss]\nmilli=1\n[bond]\nhalf_life_days=30\nleave_threshold_milli=0\n";
  }
  CHECK_THROWS(load_bond_params(bad_path));  // a threshold of 0 would empty every household at once
  std::remove(bad_path.c_str());
}

TEST_CASE("an absence that reached the household leaves a strain on each holder, by how far it went and why (A1)") {
  struct Row { AbsenceCause cause; int milli; };
  for (const Row& row : {Row{AbsenceCause::Possessed, 200}, Row{AbsenceCause::Chose, 300}, Row{AbsenceCause::Wandered, 100},
                         Row{AbsenceCause::Ill, 60}}) {
    BondRig r;
    r.absence_until_reported(kSubject, row.cause);
    const auto strained = r.events(EventType::BondStrained);
    REQUIRE(strained.size() == 2);  // both holders
    for (const Event& e : strained) {
      CHECK(e.a == static_cast<std::int64_t>(pop_of(kSubject)));
      CHECK(e.b == row.milli);
    }
    CHECK(r.bonds->strain(pop_of(kHolderA), pop_of(kSubject), r.world->clock.tick) == row.milli);
    CHECK(r.bonds->strain(pop_of(kSubject), pop_of(kHolderA), r.world->clock.tick) == 0);  // the bond is toward the pop, not back
  }
  BondRig r;
  r.absence_until_reported(kSubject, AbsenceCause::Possessed);
  // The strain's cause chain leads to the return, and from it to the absence's start.
  std::vector<EventType> types;
  for (const Event& e : r.world->log.cause_chain(r.events(EventType::BondStrained)[0].id)) types.push_back(e.type);
  REQUIRE(types.size() >= 4);
  CHECK(types[0] == EventType::BondStrained);
  CHECK(types[1] == EventType::AbsenceEnded);
}

TEST_CASE("nobody noticed, nothing strained: back in time, phoned in, or nobody at home (A1)") {
  BondRig early;
  early.begin(kSubject);
  early.world->jump(80 * kMin);  // the shift was due at 60 minutes, the tolerance runs to 90
  early.end(kSubject);
  early.world->run_ticks(1);
  CHECK(early.events(EventType::BondStrained).empty());

  BondRig called;
  called.begin(kSubject, AbsenceCause::Ill, true);
  called.world->jump(2 * kTicksPerGameDay);
  called.end(kSubject);
  called.world->run_ticks(1);
  CHECK(called.events(EventType::BondStrained).empty());

  BondRig lone;
  lone.begin(kLone, AbsenceCause::Chose);  // no holders: no household to strain
  lone.world->jump(2 * kTicksPerGameDay);
  lone.end(kLone);
  lone.world->run_ticks(1);
  CHECK(lone.events(EventType::BondStrained).empty());
}

TEST_CASE("strain eases with time: half in a month, an eighth in three (A1)") {
  BondRig r;
  r.absence_until_reported(kSubject, AbsenceCause::Possessed);  // 200
  const std::int64_t t0 = r.world->clock.tick;
  const PopId h = pop_of(kHolderA), s = pop_of(kSubject);
  CHECK(r.bonds->strain(h, s, t0) == 200);
  CHECK(r.bonds->strain(h, s, t0 + 30 * kTicksPerGameDay) == 100);
  CHECK(r.bonds->strain(h, s, t0 + 90 * kTicksPerGameDay) == 25);
  CHECK(r.bonds->strain(h, s, t0 + 300 * kTicksPerGameDay) <= 1);
}

TEST_CASE("repeated absences pile up, and past the limit the holder leaves the household (A1)") {
  BondRig r;
  for (int i = 0; i < 3; ++i) {
    r.absence_until_reported(kSubject, AbsenceCause::Chose);  // 300 each
    r.world->jump(r.world->clock.tick + kTicksPerGameDay);
    CHECK(r.events(EventType::LeftHousehold).empty());
    CHECK(r.present(kHolderA));
  }
  CHECK(r.bonds->strain(pop_of(kHolderA), pop_of(kSubject), r.world->clock.tick) > 700);  // piled up, a little eased
  r.absence_until_reported(kSubject, AbsenceCause::Chose);
  r.world->run_ticks(2);  // the roster change is a queued command, applied next tick
  const auto left = r.events(EventType::LeftHousehold);
  REQUIRE(left.size() == 2);  // both holders crossed the limit together
  CHECK(left[0].b >= 900);
  CHECK(r.bonds->left_household(pop_of(kHolderA)));
  CHECK(r.bonds->left_household(pop_of(kHolderB)));
  CHECK_FALSE(r.present(kHolderA));  // really off the roster
  CHECK_FALSE(r.present(kHolderB));
  CHECK(r.present(kSubject));
  // The next absence has nobody to notice it: the leavers no longer hold the subject's commitments.
  r.begin(kSubject);
  r.world->run_ticks(1);
  REQUIRE(r.exp->find(pop_of(kSubject)));
  CHECK(r.exp->find(pop_of(kSubject))->holders.empty());
  CHECK(r.exp->find(pop_of(kSubject))->stage == AbsenceStage::Ordinary);
}

TEST_CASE("a lost job strains the household that stood by, before any return (A1)") {
  EmployerPolicy no_pause;
  no_pause.pause_on_report = false;  // the count runs to the separation although the household reported the absence
  BondRig r(no_pause);
  r.begin(kSubject, AbsenceCause::Chose);
  r.world->jump(4 * kTicksPerGameDay);
  REQUIRE(r.exp->lost_job(pop_of(kSubject)));
  const auto strained = r.events(EventType::BondStrained);
  REQUIRE(strained.size() == 2);  // one per holder, at the separation
  for (const Event& e : strained) CHECK(e.b == 150);
  std::vector<EventType> types;
  for (const Event& e : r.world->log.cause_chain(strained[0].id)) types.push_back(e.type);
  REQUIRE(types.size() >= 2);
  CHECK(types[1] == EventType::Separated);
}

TEST_CASE("bonds jump as they step, and a save carries the strain and who left (A1)") {
  const auto drive = [](BondRig& r, bool jump) {
    for (int i = 0; i < 4; ++i) {
      r.begin(kSubject, AbsenceCause::Chose);
      if (jump) r.world->jump(r.world->clock.tick + kTicksPerGameDay);
      else r.world->run_ticks(kTicksPerGameDay);
      r.end(kSubject);
      if (jump) r.world->jump(r.world->clock.tick + 2 * kTicksPerGameDay);
      else r.world->run_ticks(2 * kTicksPerGameDay);
    }
  };
  BondRig stepped, jumped;
  drive(stepped, false);
  drive(jumped, true);
  CHECK(stepped.world->state_hash() == jumped.world->state_hash());
  CHECK(stepped.bonds->bonds() == jumped.bonds->bonds());
  REQUIRE(!stepped.events(EventType::BondStrained).empty());

  BondRig a, b;
  a.absence_until_reported(kSubject, AbsenceCause::Chose);
  b.world->load(a.world->save());
  CHECK(b.bonds->strain(pop_of(kHolderA), pop_of(kSubject), a.world->clock.tick) == 300);
  for (int i = 0; i < 3; ++i) a.absence_until_reported(kSubject, AbsenceCause::Chose);
  BondRig c;
  c.world->load(a.world->save());
  CHECK(c.bonds->left_household(pop_of(kHolderA)) == a.bonds->left_household(pop_of(kHolderA)));
  CHECK(c.world->state_hash() == a.world->state_hash());
}

TEST_CASE("in the one world, real absences strain the households that noticed (A1)" * doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 t = tier0::make_run(1);
  t.world->check_every_tick = false;
  REQUIRE(t.bonds);
  t.missing->set_rate_scale(6000);
  t.missing->redraw(*t.world, 0);
  t.world->jump(200 * kTicksPerGameDay);
  int strained = 0, left = 0;
  for (const Event& e : t.world->log.all()) {
    strained += e.type == EventType::BondStrained;
    left += e.type == EventType::LeftHousehold;
    if (e.type == EventType::BondStrained) CHECK(e.b > 0);
  }
  CHECK(strained > 0);
  CHECK(static_cast<int>(t.bonds->left().size()) == left);
  CHECK(t.bonds->bonds() > 0);
}
