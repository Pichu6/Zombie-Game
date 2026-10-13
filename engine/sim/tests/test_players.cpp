// Spec 66: players and possession (decisions 0066, 0068, 0070). A player joins with a pop (its home) or as a free camera;
// possessing a pop that is not its home takes it from its life (an absence the household, the employer and the police see);
// leaving it sends it home; calling in before a report makes the absence ordinary; pins only show.
#include <doctest.h>

#include <memory>
#include <set>
#include <vector>

#include "zs/expectations.h"
#include "zs/missing_persons.h"
#include "zs/players.h"
#include "zs/police_staff.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr std::int64_t kMin = kTicksPerGameHour / 60;
constexpr PopId kAda = 1, kOfficer = 2, kBaker = 3, kHermit = 4;
constexpr PopId kSupervisor = 10, kSpouse = 11;

// The officer is due at work an hour in, every day, held by the supervisor then the spouse; the others are expected nowhere.
std::optional<Commitment> schedule(PopId pop, std::int64_t from) {
  if (pop != kOfficer) return std::nullopt;
  std::int64_t due = 60 * kMin;
  while (due < from) due += kTicksPerGameDay;
  return Commitment{CommitmentKind::ShiftStart, due, {kSupervisor, kSpouse}, EmployerKind::Police, 480};
}

struct Rig {
  std::unique_ptr<World> world;
  Expectations* exp = nullptr;
  Players* players = nullptr;
  std::set<PopId> dead;
  std::int64_t return_min = 0;
  explicit Rig(std::uint64_t seed = 1) {
    world = std::make_unique<World>(seed);
    world->check_every_tick = false;
    auto e = std::make_unique<Expectations>(load_expectation_params(tier0::data_dir() + "/life/expectations.toml"), schedule,
                                            load_employer_policy(tier0::data_dir() + "/employment/absence_policy.toml", "", "upper_middle"));
    exp = e.get();
    const PossessionParams params = load_possession_params(tier0::data_dir() + "/life/possession.toml");
    return_min = params.return_home_min;
    auto p = std::make_unique<Players>(params, exp, [this](PopId id, std::int64_t) { return id >= 1 && id <= 20 && !dead.count(id); });
    players = p.get();
    world->add_system(std::move(e));
    world->add_system(std::move(p));
    world->close_setup();
  }
  void cmd(std::uint16_t type, std::uint64_t player, std::int64_t a = 0, std::int64_t b = 0) {
    world->submit({0, type, player, a, b});
    world->run_ticks(1);
  }
  void join(PopId pop, DetachLevel level = DetachLevel::Free) { cmd(PlayerJoin, 0, static_cast<std::int64_t>(pop), static_cast<std::int64_t>(level)); }
  std::vector<Event> events(EventType type) const {
    std::vector<Event> out;
    for (const Event& e : world->log.all())
      if (e.type == type) out.push_back(e);
    return out;
  }
  PlayerRefusal last_refusal() const {
    const auto r = events(EventType::PlayerRefused);
    return r.empty() ? PlayerRefusal{0} : static_cast<PlayerRefusal>(r.back().b);
  }
  std::size_t refusals() const { return events(EventType::PlayerRefused).size(); }
};

}  // namespace

TEST_CASE("the getting-home time loads from data, and a bad file is refused (A1)") {
  const PossessionParams p = load_possession_params(tier0::data_dir() + "/life/possession.toml");
  CHECK(p.return_home_min == 120);
  CHECK_THROWS(load_possession_params(tier0::data_dir() + "/life/no_such_file.toml"));
}

TEST_CASE("joining with a pop makes it home, first among the pins; free cam needs the free level (A1)") {
  Rig r;
  r.join(kAda, DetachLevel::Knowledge);
  REQUIRE(r.players->players().size() == 1);
  const Players::Player& p = r.players->players()[0];
  CHECK(p.id == 1);
  CHECK(p.mode == PlayerMode::Embodied);
  CHECK(p.home == kAda);
  CHECK(p.in == kAda);
  CHECK(p.pins == std::vector<PopId>{kAda});
  CHECK(r.exp->cases().empty());  // the home is the player's own life: no absence
  r.join(kNoPop, DetachLevel::Knowledge);  // 0068 rule 5: no side to know through
  CHECK(r.last_refusal() == PlayerRefusal::Level);
  r.join(kAda);  // another player's home
  CHECK(r.last_refusal() == PlayerRefusal::Reserved);
  r.dead.insert(kBaker);
  r.join(kBaker);
  CHECK(r.last_refusal() == PlayerRefusal::NotAlive);
  r.join(kNoPop, DetachLevel::Free);
  REQUIRE(r.players->players().size() == 2);
  CHECK(r.players->players()[1].mode == PlayerMode::Observer);
  CHECK(r.players->players()[1].home == kNoPop);
  CHECK(r.players->players()[1].pins.empty());
}

TEST_CASE("possessing another pop begins an absence caused by the possession; the home stays home (A1)") {
  Rig r;
  r.join(kAda);
  r.cmd(PlayerPossess, 1, kOfficer);
  const Players::Player& p = r.players->players()[0];
  CHECK(p.in == kOfficer);
  CHECK(p.home == kAda);
  CHECK(p.pins.front() == kAda);
  REQUIRE(r.exp->find(kOfficer));
  CHECK(r.exp->find(kOfficer)->cause == AbsenceCause::Possessed);
  CHECK(r.exp->find(kOfficer)->stage == AbsenceStage::Waiting);
  const auto began = r.events(EventType::AbsenceBegan);
  REQUIRE(began.size() == 1);
  const Event* why = r.world->log.get(began[0].cause);
  REQUIRE(why);
  CHECK(why->type == EventType::PlayerPossessed);
  CHECK(why->b == 1);
  CHECK(r.exp->find(kAda) == nullptr);  // the home left behind is not absent: it goes on living
}

TEST_CASE("knowledge and off players possess only their home; a free player anyone free (A1)") {
  Rig r;
  r.join(kAda, DetachLevel::Knowledge);
  r.cmd(PlayerPossess, 1, kOfficer);
  CHECK(r.last_refusal() == PlayerRefusal::Level);
  CHECK(r.players->players()[0].in == kAda);
  r.cmd(PlayerPossess, 9, kOfficer);
  CHECK(r.last_refusal() == PlayerRefusal::NoSuchPlayer);
}

TEST_CASE("a home or a possessed pop is reserved to its player (A1)") {
  Rig r;
  r.join(kAda);
  r.join(kBaker);
  r.cmd(PlayerPossess, 2, kAda);
  CHECK(r.last_refusal() == PlayerRefusal::Reserved);
  r.cmd(PlayerPossess, 1, kOfficer);
  r.cmd(PlayerPossess, 2, kOfficer);
  CHECK(r.last_refusal() == PlayerRefusal::Reserved);
  r.cmd(PlayerDetach, 1);  // player 1 is out of the body; its home stays reserved
  CHECK(r.players->players()[0].mode == PlayerMode::Detached);
  r.cmd(PlayerPossess, 2, kAda);
  CHECK(r.last_refusal() == PlayerRefusal::Reserved);
}

TEST_CASE("leaving a possessed pop sends it home: the absence ends after the getting-home time, traced to the possession (A1)") {
  Rig r;
  r.join(kAda);
  r.cmd(PlayerPossess, 1, kHermit);
  r.world->jump(3 * kTicksPerGameHour);
  r.cmd(PlayerDetach, 1);
  const std::int64_t released = r.world->clock.tick - 1;
  REQUIRE(r.players->returns().size() == 1);
  CHECK(r.players->returns()[0].at == released + r.return_min * kMin);
  r.world->jump(released + r.return_min * kMin - 1);
  CHECK(r.exp->find(kHermit));
  r.world->jump(released + r.return_min * kMin + 1);
  CHECK(r.exp->find(kHermit) == nullptr);
  CHECK(r.players->returns().empty());
  const auto ended = r.events(EventType::AbsenceEnded);
  REQUIRE(ended.size() == 1);
  CHECK(ended[0].tick == released + r.return_min * kMin);
  bool from_possession = false;
  for (const Event& e : r.world->log.cause_chain(ended[0].id)) from_possession |= e.type == EventType::PlayerPossessed;
  CHECK(from_possession);
}

TEST_CASE("off cannot leave the body; return goes home and releases the pop it was in (A1)") {
  Rig r;
  r.join(kAda, DetachLevel::Off);
  r.cmd(PlayerDetach, 1);
  CHECK(r.last_refusal() == PlayerRefusal::Level);
  Rig f;
  f.join(kAda);
  f.cmd(PlayerPossess, 1, kHermit);
  f.cmd(PlayerReturn, 1);
  CHECK(f.players->players()[0].in == kAda);
  CHECK(f.players->returns().size() == 1);  // the hermit heads home
  f.cmd(PlayerReturn, 1);
  CHECK(f.last_refusal() == PlayerRefusal::AlreadyThere);
}

TEST_CASE("calling in before the commitment breaks leaves no ladder and no shift counted (A1)") {
  Rig r;
  r.join(kAda);
  r.cmd(PlayerPossess, 1, kOfficer);
  r.cmd(PlayerCallIn, 1);
  REQUIRE(r.events(EventType::CalledIn).size() == 1);
  CHECK(r.exp->find(kOfficer)->stage == AbsenceStage::Ordinary);
  r.world->jump(5 * kTicksPerGameDay);
  CHECK(r.events(EventType::CommitmentBroken).empty());
  CHECK(r.events(EventType::ShiftMissed).empty());
  CHECK(r.events(EventType::MissingReported).empty());
  r.cmd(PlayerCallIn, 1);
  CHECK(r.last_refusal() == PlayerRefusal::NothingToCall);
}

TEST_CASE("after a report calling in is too late: only a return closes the case (A1)") {
  Rig r;
  r.join(kAda);
  r.cmd(PlayerPossess, 1, kOfficer);
  r.world->jump(kTicksPerGameDay / 2);
  REQUIRE(r.events(EventType::MissingReported).size() == 1);
  r.cmd(PlayerCallIn, 1);
  CHECK(r.last_refusal() == PlayerRefusal::TooLate);
  r.cmd(PlayerDetach, 1);
  r.world->jump(r.world->clock.tick + r.return_min * kMin + 1);
  CHECK(r.events(EventType::CaseClosed).size() == 1);
}

TEST_CASE("calling in after the first call stops the ladder where it stands (A1)") {
  Rig r;
  r.join(kAda);
  r.cmd(PlayerPossess, 1, kOfficer);
  r.world->jump(100 * kMin);  // due at 60, tolerance 30: the supervisor has phoned
  REQUIRE(r.events(EventType::HolderPhoned).size() == 1);
  r.cmd(PlayerCallIn, 1);
  CHECK(r.events(EventType::CalledIn).size() == 1);
  CHECK(r.events(EventType::CalledIn)[0].a == static_cast<std::int64_t>(AbsenceStage::Broken));
  r.world->jump(3 * kTicksPerGameDay);
  CHECK(r.events(EventType::HolderAskedAround).empty());
  CHECK(r.events(EventType::MissingReported).empty());
}

TEST_CASE("a pop taken again on its way home stays absent: one absence, no second beginning (A1)") {
  Rig r;
  r.join(kAda);
  r.join(kBaker);
  r.cmd(PlayerPossess, 1, kHermit);
  r.cmd(PlayerDetach, 1);
  r.cmd(PlayerPossess, 2, kHermit);
  CHECK(r.players->returns().empty());
  CHECK(r.players->players()[1].in == kHermit);
  r.world->jump(r.world->clock.tick + r.return_min * kMin + 10);
  CHECK(r.exp->find(kHermit));
  CHECK(r.events(EventType::AbsenceBegan).size() == 1);
}

TEST_CASE("a pop away for its own reasons cannot be taken (A1)") {
  Rig r;
  r.join(kAda);
  r.world->submit({0, AbsenceBegin, kHermit, static_cast<std::int64_t>(AbsenceCause::Chose), 0});
  r.world->run_ticks(1);
  r.cmd(PlayerPossess, 1, kHermit);
  CHECK(r.last_refusal() == PlayerRefusal::Away);
}

TEST_CASE("pins show and change nothing; the home cannot be unpinned while it lives (A1)") {
  Rig r;
  r.join(kAda);
  r.cmd(PlayerPin, 1, kBaker);
  r.cmd(PlayerPin, 1, kBaker);
  CHECK(r.last_refusal() == PlayerRefusal::AlreadyThere);
  CHECK(r.players->players()[0].pins == std::vector<PopId>{kAda, kBaker});
  r.cmd(PlayerUnpin, 1, kAda);
  CHECK(r.last_refusal() == PlayerRefusal::HomeLocked);
  r.cmd(PlayerUnpin, 1, kBaker);
  CHECK(r.players->players()[0].pins == std::vector<PopId>{kAda});
  CHECK(r.exp->cases().empty());  // pinning took nobody from their life
}

TEST_CASE("make home moves home to the pop the player is in; the old home stays an ordinary pin (A1)") {
  Rig r;
  r.join(kAda);
  r.cmd(PlayerPossess, 1, kBaker);
  r.cmd(PlayerMakeHome, 1);
  const Players::Player& p = r.players->players()[0];
  CHECK(p.home == kBaker);
  CHECK(p.pins == std::vector<PopId>{kBaker, kAda});
  r.cmd(PlayerUnpin, 1, kAda);
  CHECK(r.players->players()[0].pins == std::vector<PopId>{kBaker});
}

TEST_CASE("when the home dies the player is left out of the body and chooses a new home at any level (A1)") {
  Rig r;
  r.join(kAda, DetachLevel::Knowledge);
  r.dead.insert(kAda);
  r.world->jump(2 * kTicksPerGameHour);  // the hourly look at the bodies
  const Players::Player& p = r.players->players()[0];
  CHECK(p.home_dead);
  CHECK(p.in == kNoPop);
  CHECK(p.mode == PlayerMode::Detached);
  REQUIRE(r.events(EventType::PlayerBodyDied).size() == 1);
  CHECK(r.events(EventType::PlayerBodyDied)[0].b == 1);
  CHECK(p.pins.front() == kAda);  // the pin stays
  r.cmd(PlayerReturn, 1);
  CHECK(r.last_refusal() == PlayerRefusal::NoHome);
  r.cmd(PlayerPossess, 1, kBaker);  // knowledge, but homeless: choosing a new home is open
  r.cmd(PlayerMakeHome, 1);
  CHECK(r.players->players()[0].home == kBaker);
  CHECK(!r.players->players()[0].home_dead);
  r.cmd(PlayerUnpin, 1, kAda);  // the dead home can go now
  CHECK(r.players->players()[0].pins == std::vector<PopId>{kBaker});
}

TEST_CASE("raising the level marks the save; lowering does not; off needs a body (A1)") {
  Rig r;
  r.join(kAda, DetachLevel::Knowledge);
  r.cmd(PlayerSetLevel, 1, static_cast<std::int64_t>(DetachLevel::Off));
  CHECK(!r.players->players()[0].marked);
  r.cmd(PlayerSetLevel, 1, static_cast<std::int64_t>(DetachLevel::Free));
  CHECK(r.players->players()[0].marked);
  CHECK(r.events(EventType::PlayerLevelSet).back().b == 1);
  r.cmd(PlayerDetach, 1);
  r.cmd(PlayerSetLevel, 1, static_cast<std::int64_t>(DetachLevel::Off));
  CHECK(r.last_refusal() == PlayerRefusal::NotEmbodied);
}

TEST_CASE("nobody playing costs nothing: the system never wakes (A1)") {
  Rig r;
  CHECK(r.players->next_wake(*r.world, 0) == System::kNever);
  r.join(kNoPop);
  CHECK(r.players->next_wake(*r.world, r.world->clock.tick) == System::kNever);  // a free camera has no body to look at
}

TEST_CASE("a possession jumps as it steps, and a save carries players, pins and returns (A1)") {
  const auto drive = [](Rig& r) {
    r.join(kAda);
    r.cmd(PlayerPin, 1, kBaker);
    r.cmd(PlayerPossess, 1, kOfficer);
  };
  Rig a, b;
  drive(a);
  drive(b);
  const std::int64_t mid = 30 * kTicksPerGameHour;
  a.world->jump(mid);
  for (std::int64_t t = b.world->clock.tick; t < mid; ++t) b.world->step();
  CHECK(a.world->state_hash() == b.world->state_hash());
  a.cmd(PlayerDetach, 1);
  b.cmd(PlayerDetach, 1);
  Rig c;
  c.world->load(a.world->save());
  REQUIRE(c.players->players().size() == 1);
  CHECK(c.players->players()[0].pins == std::vector<PopId>{kAda, kBaker});
  CHECK(c.players->returns().size() == 1);
  const std::int64_t end = mid + 3 * kTicksPerGameDay;
  a.world->jump(end);
  c.world->jump(end);
  for (std::int64_t t = b.world->clock.tick; t < end; ++t) b.world->step();
  CHECK(a.world->state_hash() == b.world->state_hash());
  CHECK(a.world->state_hash() == c.world->state_hash());
}

TEST_CASE("in the one world, an officer taken from his shift is missed, counted and met with a conduct case (A1)" *
          doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 t = tier0::make_run(1);
  t.world->check_every_tick = false;
  REQUIRE(t.players);
  REQUIRE(t.force.get() != nullptr);
  const PopId officer = t.force->officers().front();
  t.world->submit({0, PlayerJoin, 0, -1, static_cast<std::int64_t>(DetachLevel::Free)});
  t.world->submit({0, PlayerPossess, 1, static_cast<std::int64_t>(officer), 0});
  t.world->jump(4 * kTicksPerGameDay);
  REQUIRE(t.expectations->find(officer));
  CHECK(t.expectations->find(officer)->cause == AbsenceCause::Possessed);
  int missed = 0;
  for (const Event& e : t.world->log.all()) missed += e.type == EventType::ShiftMissed && e.subject == officer;
  CHECK(missed >= 3);
  t.world->submit({0, PlayerDetach, 1, 0, 0});
  t.world->jump(t.world->clock.tick + 3 * kTicksPerGameHour);
  CHECK(t.expectations->find(officer) == nullptr);
  int conduct = 0;
  for (const Event& e : t.world->log.all()) conduct += e.type == EventType::ConductCaseOpened && e.subject == officer;
  CHECK(conduct == 1);
}
