// Spec 60: real people go missing without any player: rates by age class, return times, causes, determinism, and a
// possession's absence not being disturbed.
#include <doctest.h>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "zs/missing_persons.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr std::int64_t kYear = 365 * kTicksPerGameDay;
constexpr std::int64_t kMin = kTicksPerGameHour / 60;

std::string data_file() { return tier0::data_dir() + "/life/missing.toml"; }

std::optional<Commitment> everyone_is_expected(PopId pop, std::int64_t from) {
  return Commitment{CommitmentKind::ShiftStart, from + 8 * kTicksPerGameHour, {pop + 1'000'000}};
}

struct Rig {
  std::unique_ptr<World> world;
  Expectations* exp = nullptr;
  MissingPersons* missing = nullptr;
  // `adults` pops of age 30 with ids 1..adults, `children` of age 10 after them; scale multiplies the rates.
  Rig(std::uint64_t seed, int adults, int children, double scale, std::function<bool(PopId)> eligible = nullptr, const std::string& income = "high") {
    world = std::make_unique<World>(seed);
    world->check_every_tick = false;
    auto e = std::make_unique<Expectations>(load_expectation_params(tier0::data_dir() + "/life/expectations.toml"), everyone_is_expected);
    exp = e.get();
    MissingParams params = load_missing_params(data_file(), "", income);
    params.rate_scale = scale;
    auto m = std::make_unique<MissingPersons>(params, exp, std::move(eligible));
    missing = m.get();
    world->add_system(std::move(e));
    world->add_system(std::move(m));
    for (int i = 1; i <= adults + children; ++i) missing->track(*world, static_cast<PopId>(i), i <= adults ? 30 : 10);
    world->close_setup();
  }
  std::vector<Event> events(EventType type) const {
    std::vector<Event> out;
    for (const Event& e : world->log.all())
      if (e.type == type) out.push_back(e);
    return out;
  }
};

}  // namespace

TEST_CASE("the tables load, an income group reads its index, and a bad table is refused (A1)") {
  const MissingParams high = load_missing_params(data_file(), "", "high");
  CHECK(high.adult_per_1000_year == doctest::Approx(1.8));
  CHECK(high.child_per_1000_year == doctest::Approx(5.6));
  CHECK(high.child_per_1000_year > high.adult_per_1000_year);
  const MissingParams middle = load_missing_params(data_file(), "", "upper_middle");
  CHECK(middle.adult_per_1000_year == doctest::Approx(0.9));
  CHECK(load_missing_params(data_file(), "gb_ew", "low").adult_per_1000_year == doctest::Approx(1.8));  // a table of its own wins
  REQUIRE(high.adult_return.size() == 2);
  CHECK(high.adult_return[0].first == 48);
  CHECK_THROWS(load_missing_params(tier0::data_dir() + "/life/no_such.toml", "", "high"));
  CHECK_THROWS(load_missing_params(data_file(), "", "no_such_group"));
  const std::string bad_path = (std::filesystem::temp_directory_path() / "zs_bad_missing.toml").string();
  {
    std::ofstream bad(bad_path);
    bad << "[t]\nadult_per_1000_year=1\nchild_per_1000_year=1\nchild_age_max=17\nadult_return_cdf=[[48,0.9],[24,0.95]]\n"
           "child_return_cdf=[[24,0.5]]\nadult_chose=0\nadult_wandered=0\nchild_chose=0\nchild_wandered=0\n[index]\nhigh=\"t\"\n";
  }
  CHECK_THROWS(load_missing_params(bad_path, "t", "high"));  // hours go backwards
  std::remove(bad_path.c_str());
}

TEST_CASE("absences begin at the table's rate per head, children more often than adults (A1)") {
  Rig r(1, 4000, 4000, 40.0);  // 40 times the real rate, so a year has hundreds
  r.world->jump(kYear);
  int adults = 0, children = 0;
  for (const Event& e : r.events(EventType::AbsenceBegan)) {
    CHECK(e.b == 0);  // real absences are never phoned in
    (e.subject <= 4000 ? adults : children)++;
  }
  const double want_adults = 4000 * 1.8 / 1000 * 40, want_children = 4000 * 5.6 / 1000 * 40;  // 288 and 896
  CHECK(adults == doctest::Approx(want_adults).epsilon(0.12));
  CHECK(children == doctest::Approx(want_children).epsilon(0.08));
  CHECK(children > 2.5 * adults);
}

TEST_CASE("adults come back as the table says: 85 % within 48 h, 98.8 % within 28 days, a few never (A1)") {
  Rig r(2, 6000, 0, 300.0);
  r.world->jump(kYear);
  const auto ended = r.events(EventType::AbsenceEnded);
  const auto began = r.events(EventType::AbsenceBegan);
  REQUIRE(began.size() > 3000);
  int within48 = 0, within28d = 0;
  for (const Event& e : ended) {
    within48 += e.b <= 48 * kTicksPerGameHour;
    within28d += e.b <= 672 * kTicksPerGameHour;
  }
  // Of the absences that began early enough to have resolved (before the last 28 days), shares that returned by then.
  int old_began = 0;
  for (const Event& e : began) old_began += e.tick < kYear - 28 * kTicksPerGameDay;
  REQUIRE(old_began > 2500);
  CHECK(static_cast<double>(within48) / static_cast<double>(began.size()) == doctest::Approx(0.85).epsilon(0.04));
  CHECK(static_cast<double>(within28d) / static_cast<double>(began.size()) == doctest::Approx(0.988).epsilon(0.01));
  const double never = 1.0 - static_cast<double>(ended.size()) / static_cast<double>(began.size());  // still open at the end
  CHECK(never < 0.04);  // 1.2 % long-term, plus the ones that began in the last weeks
  CHECK(never > 0.005);
  for (const Event& e : ended) CHECK(e.b > 0);
}

TEST_CASE("the causes follow the table: adults 16 % chose, 8.6 % wandered, the rest unknown (A1)") {
  Rig r(3, 6000, 0, 300.0);
  r.world->jump(kYear);
  const auto began = r.events(EventType::AbsenceBegan);
  REQUIRE(began.size() > 3000);
  int chose = 0, wandered = 0, unknown = 0;
  for (const Event& e : began) {
    chose += e.a == static_cast<std::int64_t>(AbsenceCause::Chose);
    wandered += e.a == static_cast<std::int64_t>(AbsenceCause::Wandered);
    unknown += e.a == static_cast<std::int64_t>(AbsenceCause::Unknown);
  }
  CHECK(chose + wandered + unknown == static_cast<int>(began.size()));
  CHECK(static_cast<double>(chose) / began.size() == doctest::Approx(0.16).epsilon(0.12));
  CHECK(static_cast<double>(wandered) / began.size() == doctest::Approx(0.086).epsilon(0.2));
}

TEST_CASE("the same seed gives the same absences, jumped or stepped, and a save carries them (A1)") {
  Rig a(4, 60, 20, 800.0), b(4, 60, 20, 800.0), c(5, 60, 20, 800.0);
  a.world->jump(20 * kTicksPerGameDay);
  b.world->run_ticks(20 * kTicksPerGameDay);
  c.world->jump(20 * kTicksPerGameDay);
  CHECK(a.world->state_hash() == b.world->state_hash());
  CHECK(a.world->log.all().size() == b.world->log.all().size());
  CHECK(a.world->state_hash() != c.world->state_hash());
  REQUIRE(!a.events(EventType::AbsenceBegan).empty());

  Rig x(6, 60, 20, 800.0), y(6, 60, 20, 800.0);
  x.world->jump(7 * kTicksPerGameDay);
  y.world->load(x.world->save());
  x.world->jump(20 * kTicksPerGameDay);
  y.world->jump(20 * kTicksPerGameDay);
  CHECK(x.world->state_hash() == y.world->state_hash());
}

TEST_CASE("a pop already absent (a possession) is not disturbed; an ineligible pop never goes missing (A1)") {
  Rig r(7, 40, 0, 2000.0, [](PopId id) { return id % 2 == 0; });
  r.world->submit({0, AbsenceBegin, 2, static_cast<std::int64_t>(AbsenceCause::Possessed), 0});  // pop 2 is possessed all year
  r.world->jump(kYear);
  for (const Event& e : r.events(EventType::AbsenceBegan))
    if (e.subject != 2) CHECK(e.subject % 2 == 0);  // odd pops are ineligible
  CHECK(!r.events(EventType::AbsenceBegan).empty());
  CHECK(r.events(EventType::ExpectationRefused).empty());  // the system never begins a second absence or ends one it did not begin
  const Expectations::Case* c = r.exp->find(2);
  REQUIRE(c);
  CHECK(c->cause == AbsenceCause::Possessed);
  CHECK(c->began == 0);
  int pop2_began = 0;
  for (const Event& e : r.events(EventType::AbsenceBegan)) pop2_began += e.subject == 2;
  CHECK(pop2_began == 1);  // only the player's
}

TEST_CASE("a rate of zero never wakes the system (A1)") {
  Rig r(8, 100, 100, 0.0);
  CHECK(r.missing->next_wake(*r.world, 0) >= System::kNever / 8);
  r.world->jump(kYear);
  CHECK(r.events(EventType::AbsenceBegan).empty());
}

TEST_CASE("in the one world, real absences are noticed by the household on the ladder (A1)" * doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 t = tier0::make_run(1);
  t.world->check_every_tick = false;
  REQUIRE(t.missing);
  CHECK(!t.missing->entries().empty());
  for (const MissingPersons::Entry& e : t.missing->entries()) CHECK(e.next_start > 0);  // at the real rates nobody goes missing on day one
  t.missing->set_rate_scale(4000);  // a sandbox option: thousands of times the real rate, so a month has several
  t.missing->redraw(*t.world, 0);
  t.world->jump(60 * kTicksPerGameDay);
  int real = 0, broken = 0;
  for (const Event& e : t.world->log.all()) {
    if (e.type == EventType::AbsenceBegan) {
      ++real;
      CHECK(e.b == 0);
      CHECK(e.a != static_cast<std::int64_t>(AbsenceCause::Possessed));
    }
    broken += e.type == EventType::CommitmentBroken;
  }
  CHECK(real > 5);
  CHECK(broken > 0);  // somebody at home expected one of them and the ladder ran
}
