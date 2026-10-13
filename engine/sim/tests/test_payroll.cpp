// Spec 62: payroll. A shift worked is paid, a shift missed is not, a separation ends the pay and takes the pop off the
// employer's workplace roster; money is conserved.
#include <doctest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "zs/economy_system.h"
#include "zs/expectations.h"
#include "zs/payroll.h"
#include "zs/pop_system.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr std::int64_t kMin = kTicksPerGameHour / 60;
constexpr std::uint64_t kRoster = 77;
constexpr std::int64_t kWage = 1000;  // cents an hour: an 8 hour shift pays 8,000

// A shift every day at 09:00 (tick 0 is 08:00), eight hours, a private employer, nobody to phone.
std::optional<Commitment> daily_shift(PopId, std::int64_t from) {
  const std::int64_t due = ((from + kTicksPerGameDay - 1) / kTicksPerGameDay) * kTicksPerGameDay + 60 * kMin;
  Commitment c{CommitmentKind::ShiftStart, due, {}, EmployerKind::Private};
  c.minutes = 480;
  return c;
}

struct PayRig {
  std::unique_ptr<World> world;
  PopSystem* pops = nullptr;
  EconomySystem* econ = nullptr;
  Expectations* exp = nullptr;
  Payroll* pay = nullptr;
  OwnerId employer = 0;
  std::vector<OwnerId> wallets;  // by slot
  ResourceId money = 0;
  std::int64_t minted = 0;

  PayRig(int workers, std::int64_t employer_cents = 100'000'000, std::uint64_t seed = 1) {
    world = std::make_unique<World>(seed);
    world->check_every_tick = true;
    auto p = std::make_unique<PopSystem>(*world);
    p->setup(*world).add_cohort({1, 1, 0, static_cast<std::uint64_t>(workers + 2)});
    p->setup(*world).add_roster(kRoster, 1, RosterKind::Workplace);
    for (int i = 0; i < workers; ++i) p->setup(*world).roster_join(kRoster, census_pop_id(static_cast<std::uint32_t>(i)), 0.0);
    auto e = std::make_unique<EconomySystem>(*world);
    Owners& o = e->owners();
    const OwnerId country = o.add(world->ledger, "country", Scope::Country);
    const OwnerId region = o.add(world->ledger, "region", Scope::Region, country);
    employer = o.add(world->ledger, "employer", Scope::Building, region);
    for (int i = 0; i < workers + 2; ++i) wallets.push_back(o.add(world->ledger, "pop" + std::to_string(i), Scope::Pop, region));
    money = e->money().resource();
    world->ledger.mint(employer, money, employer_cents);
    e->start_lots(*world);
    e->start_establishments(*world);
    e->start_populations(*world);
    auto x = std::make_unique<Expectations>(load_expectation_params(tier0::data_dir() + "/life/expectations.toml"), daily_shift);
    WageParams wp;
    wp.cents_per_hour = kWage;
    auto pr = std::make_unique<Payroll>(wp, x.get(), daily_shift, employer, kRoster,
                                        [this](PopId id) { return wallets[founder_slot_of(id)]; });
    for (int i = 0; i < workers; ++i) pr->track(census_pop_id(static_cast<std::uint32_t>(i)));
    pops = p.get();
    econ = e.get();
    exp = x.get();
    pay = pr.get();
    world->add_system(std::move(p));
    world->add_system(std::move(e));
    world->add_system(std::move(x));
    world->add_system(std::move(pr));
    world->close_setup();
    minted = world->ledger.total(money);
  }
  PopId pop(int slot) const { return census_pop_id(static_cast<std::uint32_t>(slot)); }
  std::int64_t cash(int slot) const { return world->ledger.balance(wallets[static_cast<std::size_t>(slot)], money); }
  std::int64_t employer_cash() const { return world->ledger.balance(employer, money); }
  bool on_roster(int slot) const { return pops->registry().roster(kRoster)->present(pop(slot), year_of_tick(world->clock.tick)); }
  void begin(int slot, bool phoned_in = false) {
    world->submit({0, AbsenceBegin, pop(slot), static_cast<std::int64_t>(AbsenceCause::Chose), phoned_in ? 1 : 0});
  }
  void end(int slot) { world->submit({0, AbsenceEnd, pop(slot), 0, 0}); }
  std::vector<Event> events(EventType type, int slot = -1) const {
    std::vector<Event> out;
    for (const Event& e : world->log.all())
      if (e.type == type && (slot < 0 || e.subject == pop(slot))) out.push_back(e);
    return out;
  }
};

constexpr std::int64_t kShiftPay = 8 * kWage;  // 8 hours

}  // namespace

TEST_CASE("the wage tables load per country and income group, and a bad file is refused (A1)") {
  const std::string path = tier0::data_dir() + "/employment/wages.toml";
  CHECK(load_wage_params(path, "", "high").cents_per_hour == doctest::Approx(1328));  // DRL-1: the group's median
  CHECK(load_wage_params(path, "", "low").cents_per_hour == doctest::Approx(101));
  CHECK(load_wage_params(path, "", "high").cents_per_hour > load_wage_params(path, "", "upper_middle").cents_per_hour);
  CHECK(load_wage_params(path, "USA", "low").cents_per_hour == doctest::Approx(4349));  // a table of its own wins (DRL-1)
  CHECK_THROWS(load_wage_params(tier0::data_dir() + "/employment/no_such.toml", "", "high"));
  CHECK_THROWS(load_wage_params(path, "", "no_such_group"));
  const std::string bad_path = (std::filesystem::temp_directory_path() / "zs_bad_wages.toml").string();
  {
    std::ofstream bad(bad_path);
    bad << "[t]\ncents_per_hour = 0\n[index]\nhigh = \"t\"\n";
  }
  CHECK_THROWS(load_wage_params(bad_path, "t", "high"));  // a wage must be positive
  std::remove(bad_path.c_str());
}

TEST_CASE("each shift worked is paid by the employer when it starts, and money is conserved (A1)") {
  PayRig r(3);
  r.world->jump(3 * kTicksPerGameDay);  // shifts on days 0, 1 and 2
  for (int i = 0; i < 3; ++i) CHECK(r.cash(i) == 3 * kShiftPay);
  CHECK(r.employer_cash() == 100'000'000 - 9 * kShiftPay);
  CHECK(r.world->ledger.total(r.money) == r.minted);
  CHECK(r.events(EventType::EconPaid).size() == 9);
  CHECK(r.pay->shifts_paid_total() == 9);
  CHECK(r.pay->wages_ordered() == 9 * kShiftPay);
  CHECK(r.pay->headcount() == 3);
  CHECK(r.cash(3) == 0);  // a pop with no job is never paid
}

TEST_CASE("a shift missed during an absence is withheld, and pay resumes on return (A1)") {
  PayRig r(2);
  r.begin(0);  // pop 0 is away from tick 0 (a phone call would not change the pay)
  r.world->jump(kTicksPerGameDay + 3 * kTicksPerGameHour);  // shifts at day 0 and 1: both missed by pop 0 (two, so no separation)
  CHECK(r.cash(0) == 0);
  CHECK(r.cash(1) == 2 * kShiftPay);
  const auto unpaid = r.events(EventType::ShiftUnpaid, 0);
  REQUIRE(unpaid.size() == 2);
  CHECK(unpaid[0].a == kShiftPay);  // cents withheld
  CHECK(unpaid[0].b == 480);
  r.end(0);
  r.world->jump(2 * kTicksPerGameDay + 3 * kTicksPerGameHour);  // back for the third shift
  CHECK(r.cash(0) == kShiftPay);
  CHECK(r.pay->find(r.pop(0))->shifts_unpaid == 2);
  CHECK(r.pay->find(r.pop(0))->shifts_paid == 1);
  CHECK(r.world->ledger.total(r.money) == r.minted);
}

TEST_CASE("a separation ends the pay and takes the pop off the employer's roster; labour is one fewer (A1)") {
  PayRig r(3);
  r.begin(0);  // nobody to phone, so nobody reports: three missed shifts and the employer separates
  r.world->jump(6 * kTicksPerGameDay);
  REQUIRE(r.exp->lost_job(r.pop(0)));
  CHECK_FALSE(r.on_roster(0));
  CHECK(r.on_roster(1));
  CHECK(r.on_roster(2));
  CHECK(r.pay->headcount() == 2);
  CHECK(r.pay->find(r.pop(0))->active == false);
  const auto left = r.events(EventType::LeftEmployer, 0);
  REQUIRE(left.size() == 1);
  CHECK(left[0].a == static_cast<std::int64_t>(r.employer));
  // The separation fell on day 2's shift (the third missed): the roster change follows at once, not at the next shift.
  const auto sep = r.events(EventType::Separated, 0);
  REQUIRE(sep.size() == 1);
  CHECK(left[0].tick == sep[0].tick);
  // Back at last: the job is gone, so nothing more is paid; the others kept earning.
  r.end(0);
  r.world->jump(9 * kTicksPerGameDay);
  CHECK(r.cash(0) == 0);
  CHECK(r.cash(1) == 9 * kShiftPay);
  CHECK(r.world->ledger.total(r.money) == r.minted);
}

TEST_CASE("an employer that cannot pay the whole wage pays what it has and nothing is made from thin air (A1)") {
  PayRig r(2, 12'000);  // enough for one and a half shifts
  r.world->jump(2 * kTicksPerGameDay);
  CHECK(r.employer_cash() == 0);
  CHECK(r.cash(0) + r.cash(1) == 12'000);
  CHECK(r.world->ledger.total(r.money) == r.minted);
}

TEST_CASE("payroll jumps as it steps, and a save carries the count and the lost job (A1)") {
  const auto drive = [](PayRig& r, bool jump) {
    r.begin(0);
    const std::int64_t end = 8 * kTicksPerGameDay;
    if (jump) r.world->jump(end);
    else r.world->run_ticks(end);
  };
  PayRig stepped(3), jumped(3);
  stepped.world->check_every_tick = jumped.world->check_every_tick = false;
  drive(stepped, false);
  drive(jumped, true);
  CHECK(stepped.world->state_hash() == jumped.world->state_hash());
  CHECK(stepped.world->log.all().size() == jumped.world->log.all().size());

  PayRig a(3), b(3);
  a.begin(0);
  a.world->jump(kTicksPerGameDay + 5 * kTicksPerGameHour);  // mid-count
  b.world->load(a.world->save());
  CHECK(b.pay->entries().size() == 3);
  a.world->jump(8 * kTicksPerGameDay);
  b.world->jump(8 * kTicksPerGameDay);
  CHECK(a.world->state_hash() == b.world->state_hash());
  CHECK(b.pay->headcount() == 2);
}

TEST_CASE("in the one world, wages flow to workers' wallets and a separation leaves the roster (A1)" * doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 t = tier0::make_run(1);
  t.world->check_every_tick = false;
  REQUIRE(t.payroll);
  CHECK(t.payroll->headcount() > 0);
  const ResourceId money = t.econ->money().resource();
  const std::int64_t total = t.world->ledger.total(money);
  const std::int64_t employer_before = t.world->ledger.balance(t.owners.employers, money);
  t.world->jump(14 * kTicksPerGameDay);
  CHECK(t.payroll->wages_ordered() > 0);
  CHECK(t.world->ledger.balance(t.owners.employers, money) < employer_before);  // wages left the employer
  CHECK(t.world->ledger.total(money) == total);
  // Some worker has been paid into a wallet above its starting balance.
  bool richer = false;
  for (const Payroll::Entry& e : t.payroll->entries())
    if (e.shifts_paid > 0) {
      const OwnerId wallet = t.owners.pops[founder_slot_of(e.pop)];
      richer = richer || t.world->ledger.balance(wallet, money) > 50'000;
    }
  CHECK(richer);
}
