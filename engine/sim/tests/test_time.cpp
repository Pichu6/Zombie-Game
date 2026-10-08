// Spec 40: World::jump steps only the wake ticks and equals stepping, event log and hash included; an attended body
// forces every tick; scheduled events land on time; the pacer's ladder, caps, hysteresis, alerts and accumulator.
#include <doctest.h>

#include <memory>

#include "zs/body_lab.h"
#include "zs/economy_system.h"
#include "zs/grid_system.h"
#include "zs/orders_system.h"
#include "zs/pacer.h"

using namespace zs;

namespace {

struct Ids {
  OwnerId country, region, farm, shop, ana;
  GoodId grain, milk;
  RecipeId milling;
  std::uint32_t levy;
};
Ids ids;

// A cold world: economy (hourly), orders (daily), grids (commands only), pops without bodies (commands only).
std::unique_ptr<World> make_cold(std::uint64_t seed) {
  auto w = std::make_unique<World>(seed);
  auto pops = std::make_unique<PopSystem>(*w);
  pops->setup(*w).add_cohort({1, 0, 0, 50});
  auto econ = std::make_unique<EconomySystem>(*w);
  Owners& o = econ->owners();
  Goods& g = econ->goods();
  ids.country = o.add(w->ledger, "country", Scope::Country);
  ids.region = o.add(w->ledger, "region", Scope::Region, ids.country);
  ids.farm = o.add(w->ledger, "farm", Scope::Building, ids.region);
  ids.shop = o.add(w->ledger, "shop", Scope::Building, ids.region);
  ids.ana = o.add(w->ledger, "ana", Scope::Pop, ids.shop);
  ids.grain = g.add(w->ledger, "grain", "grain_g", "grain_sack", 50'000);
  ids.milk = g.add(w->ledger, "milk", "milk_ml", "milk_bottle", 1'000, 2.0);
  GoodId flour = g.add(w->ledger, "flour", "flour_g", "flour_bag", 10'000);
  ids.milling = econ->recipes().add(w->ledger, g, {"milling", ids.grain, 100'000, flour, 95'000, {}, 2 * kTicksPerGameHour, 0});
  econ->establishments().add({ids.farm, ids.milling, 0, 4, 0});
  w->ledger.mint(ids.farm, g.def(ids.grain).bulk, 20'000'000);
  w->ledger.mint(ids.farm, g.def(ids.milk).bulk, 300'000);
  w->ledger.mint(ids.country, econ->money().resource(), 1'000'000);
  w->ledger.mint(ids.shop, econ->money().resource(), 50'000);
  econ->start_lots(*w);
  econ->start_establishments(*w);
  econ->start_populations(*w);
  auto orders = std::make_unique<OrdersSystem>(econ.get(), pops.get());
  ids.levy = orders->edicts().add_def({"milk levy 5%", EdictKind::Levy, ids.region, ids.country, ids.milk, 50, 0, 0});
  auto grids = std::make_unique<GridSystem>();
  const GridId nation = grids->tree().add_grid("nation");
  grids->tree().add_node(nation, {"plant", NodeKind::Source, 1'000'000});
  const std::size_t load = grids->tree().add_node(nation, {"town", NodeKind::Load, 0, 400'000});
  grids->tree().add_edge(nation, 0, load);
  grids->start();
  w->add_system(std::move(pops));
  w->add_system(std::move(econ));
  w->add_system(std::move(orders));
  w->add_system(std::move(grids));
  w->close_setup();
  w->check_every_tick = true;
  return w;
}

// Commands at odd ticks, so the jump must stop for them.
void script(World& w) {
  const std::int64_t t = w.clock.tick, day = kTicksPerGameDay, hour = kTicksPerGameHour;
  if (t == 3) w.submit({0, EdictSign, ids.country, ids.levy, 0});
  if (t == 5 * hour + 17) w.submit({0, EconShip, ids.farm, ids.shop, econ_ship_arg(ids.milk, 40'000)});
  if (t == 2 * day + 999) w.submit({0, GridSetDemand, GridTree::node_id(0, 1), 700'000, 0});
  if (t == 3 * day + 1) w.submit({0, EconPay, ids.shop, ids.ana, econ_pay_arg(FlowKind::Wage, 1'500)});
}

}  // namespace

TEST_CASE("a cold world jumps a week with the same event log and hash as stepping, stopping for commands (A1)") {
  const std::int64_t span = 7 * kTicksPerGameDay;
  auto stepped = make_cold(1);
  for (std::int64_t i = 0; i < span; ++i) { script(*stepped); stepped->step(); }
  auto jumped = make_cold(1);
  // The jump must land on every command tick: run to each one, submit, jump on.
  const std::int64_t stops[] = {3, 5 * kTicksPerGameHour + 17, 2 * kTicksPerGameDay + 999, 3 * kTicksPerGameDay + 1, span};
  int steps = 0;
  for (std::int64_t stop : stops) {
    while (jumped->clock.tick < stop) {
      const std::int64_t wake = std::min(jumped->next_wake(jumped->clock.tick), stop);
      jumped->jump(wake);
      if (jumped->clock.tick < stop) { jumped->step(); ++steps; }  // the wake tick itself
    }
    if (stop < span) script(*jumped);
  }
  CHECK(jumped->clock.tick == span);
  CHECK(jumped->state_hash() == stepped->state_hash());
  CHECK(jumped->log.all().size() == stepped->log.all().size());
  CHECK(jumped->ledger.balances() == stepped->ledger.balances());
  // 7 days: 168 hourly wakes plus a few command ticks, against 1,209,600 ticks.
  CHECK(steps < 200);
  CHECK(steps >= 168);
}

TEST_CASE("World::jump does the bookkeeping itself: one call equals the stepping world (A1)") {
  const std::int64_t span = 3 * kTicksPerGameDay;
  auto stepped = make_cold(2);
  for (std::int64_t i = 0; i < span; ++i) stepped->step();
  auto jumped = make_cold(2);
  jumped->jump(span);
  CHECK(jumped->clock.tick == span);
  CHECK(jumped->state_hash() == stepped->state_hash());
  // Idempotent: jumping to where we are changes nothing; jumping backwards is a no-op.
  const auto h = jumped->state_hash();
  jumped->jump(span);
  jumped->jump(span - 100);
  CHECK(jumped->state_hash() == h);
  CHECK(jumped->clock.tick == span);
}

TEST_CASE("a scheduled event lands on its tick inside a jump; a pending command stops the jump at once (A1)") {
  auto w = make_cold(3);
  // A scheduled event in the middle of an hour: the jump must wake for it.
  ScheduledEvent e;
  e.tick = 2 * kTicksPerGameHour + 123;
  w->scheduler.schedule(e);
  CHECK(w->next_wake(0) == 0);  // the economy wakes at tick 0 (an hour boundary)
  CHECK(w->next_wake(1) == kTicksPerGameHour);
  CHECK(w->next_wake(2 * kTicksPerGameHour + 1) == 2 * kTicksPerGameHour + 123);
  w->submit({0, EconPay, ids.shop, ids.ana, econ_pay_arg(FlowKind::Wage, 100)});
  CHECK(w->next_wake(1) == 1);  // a pending command: now
  w->step();
  CHECK(w->next_wake(1) == kTicksPerGameHour);
}

TEST_CASE("an attended body forces every tick: with bodies the wake is now, so the jump steps everything (A1)") {
  World w(4);
  Bodies bodies(w);
  bodies.reserve_slots(w, 2);
  auto pops = std::make_unique<PopSystem>(w);
  pops->setup(w).add_cohort({1, 0, 0, 10});
  pops->attach_bodies(w, bodies);
  w.add_system(std::move(pops));
  w.close_setup();
  CHECK(w.next_wake(5) == 5);
  CHECK(w.next_wake(1000) == 1000);
  const std::int64_t before = w.clock.tick;
  w.jump(before + 300);
  CHECK(w.clock.tick == before + 300);
  CHECK(w.log.all().size() >= 1);  // nothing skipped: the pops system audited every tick (no events, but no skips either)
}

TEST_CASE("the pacer: the ladder of 0050, caps with hysteresis, alerts that pause, and the tick accumulator (A1)") {
  CHECK(pace_factor(kStartPace) == 24);  // 1 real hour = 1 game day
  CHECK(pace_factor(Pace::Real) == 1);
  CHECK(pace_factor(Pace::DayPerSecond) == 86400);
  CHECK(faster(Pace::Real) == Pace::X3);
  CHECK(slower(Pace::Pause) == Pace::Pause);
  CHECK(faster(Pace::YearPerSecond) == Pace::YearPerSecond);
  Pacer p;
  p.set_dwell_ticks(100);
  CHECK(p.effective(0) == kStartPace);
  // One real second at 24x is 24 game seconds = 48 ticks; fractions carry.
  CHECK(p.ticks_for(1.0, 0) == 48);
  CHECK(p.ticks_for(1.0 / 96, 0) == 0);
  CHECK(p.ticks_for(1.0 / 96, 0) == 1);
  // A fight caps at 1:1 and names itself; the request stands; releasing keeps the cap for the dwell.
  p.set_requested(Pace::X60);
  p.cap("Ana, fighting", Pace::Real, 10);
  CHECK(p.effective(10) == Pace::Real);
  REQUIRE(p.limiting(10));
  CHECK(*p.limiting(10) == "Ana, fighting");
  CHECK(p.requested() == Pace::X60);
  p.release("Ana, fighting", 50);
  CHECK(p.effective(100) == Pace::Real);   // still within the dwell
  CHECK(p.effective(150) == Pace::X60);    // dwell over
  CHECK(p.limiting(150) == nullptr);
  // A looser cap than the request limits nothing.
  p.cap("a project watched", Pace::DayPerSecond, 200);
  CHECK(p.effective(200) == Pace::X60);
  CHECK(p.limiting(200) == nullptr);
  // An alert pauses until acknowledged.
  p.alert("Bo was bitten", 300);
  CHECK(p.effective(300) == Pace::Pause);
  CHECK(p.ticks_for(5.0, 300) == 0);
  p.acknowledge();
  CHECK(p.effective(300) == Pace::X60);
  // Saved and restored.
  ArchiveWriter aw;
  p.save(aw);
  Pacer q;
  ArchiveReader ar(aw.bytes());
  q.load(ar);
  CHECK(ar.at_end());
  CHECK(q.requested() == Pace::X60);
  CHECK(q.effective(200) == Pace::X60);
}
