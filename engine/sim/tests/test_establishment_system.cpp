// Spec 33: establishments inside a world: staff from a workplace roster, commands, hourly production, save and replay.
#include <doctest.h>

#include <memory>

#include "zs/economy_system.h"

using namespace zs;

namespace {

struct Ids {
  OwnerId country, region, mill, bakery, ana;
  GoodId grain, flour, bread, water;
  RecipeId milling, baking;
  std::uint64_t mill_roster = 7;
};

Ids ids;

std::unique_ptr<World> make_world(std::uint64_t seed) {
  auto w = std::make_unique<World>(seed);
  auto pops = std::make_unique<PopSystem>(*w);
  PopRegistry& reg = pops->setup(*w);
  reg.add_cohort({1, 0, 0, 200});
  reg.add_roster(ids.mill_roster, 0, RosterKind::Workplace);
  for (PopId p = 0; p < 6; ++p) reg.roster_join(ids.mill_roster, p, 0.0);  // six millers
  auto econ = std::make_unique<EconomySystem>(*w);
  econ->attach_pops(*pops);
  Owners& o = econ->owners();
  Goods& g = econ->goods();
  ids.country = o.add(w->ledger, "country", Scope::Country);
  ids.region = o.add(w->ledger, "region", Scope::Region, ids.country);
  ids.mill = o.add(w->ledger, "mill", Scope::Building, ids.region);
  ids.bakery = o.add(w->ledger, "bakery", Scope::Building, ids.region);
  ids.ana = o.add(w->ledger, "ana", Scope::Pop, ids.bakery);
  ids.grain = g.add(w->ledger, "grain", "grain_g", "grain_sack", 50'000);
  ids.flour = g.add(w->ledger, "flour", "flour_g", "flour_bag", 10'000);
  ids.bread = g.add(w->ledger, "bread", "bread_g", "bread_loaf", 500);
  ids.water = g.add(w->ledger, "water", "water_ml", "water_can", 1'000);
  ids.milling = econ->recipes().add(w->ledger, g, {"milling", ids.grain, 100'000, ids.flour, 95'000, {}, 2 * kTicksPerGameHour, 0});
  ids.baking = econ->recipes().add(w->ledger, g, {"baking", ids.flour, 10'000, ids.bread, 14'000, {{ids.water, 6'000}}, kTicksPerGameHour, 3'000});
  econ->establishments().add({ids.mill, ids.milling, ids.mill_roster});       // staff from the roster
  econ->establishments().add({ids.bakery, ids.baking, 0, 2, 0});              // two bakers set by hand, no power yet
  w->ledger.mint(ids.mill, g.def(ids.grain).bulk, 50'000'000);  // 500 batches
  w->ledger.mint(ids.bakery, g.def(ids.water).bulk, 500'000);
  w->ledger.mint(ids.ana, g.def(ids.flour).bulk, 20'000);
  w->ledger.mint(ids.ana, g.def(ids.water).bulk, 12'000);
  econ->start_lots(*w);
  econ->start_establishments(*w);
  w->add_system(std::move(pops));
  w->add_system(std::move(econ));
  w->close_setup();
  w->check_every_tick = true;
  return w;
}

void script(World& w) {
  const std::int64_t t = w.clock.tick, day = kTicksPerGameDay, hour = kTicksPerGameHour;
  if (t == 6 * hour) w.submit({0, EconShip, ids.mill, ids.bakery, econ_ship_arg(ids.flour, 400'000)});
  if (t == 6 * hour + 1) w.submit({0, EstabSetPower, ids.bakery, 3'000, 0});
  if (t == 1 * day) w.submit({0, RosterApply, ids.mill_roster, static_cast<std::int64_t>(EffectKind::LaidOff), 3});
  if (t == 1 * day + 2) w.submit({0, EstabSetStaff, ids.mill, 10, 0});  // refused: the roster decides
  if (t == 1 * day + 3) w.submit({0, EconRunByHand, ids.ana, ids.baking, 5});  // Ana bakes two batches by hand
  if (t == 2 * day) w.submit({0, EstabSetStaff, ids.bakery, 0, 0});
}

std::unique_ptr<World> run(std::uint64_t seed, std::int64_t ticks) {
  auto w = make_world(seed);
  for (std::int64_t i = 0; i < ticks; ++i) { script(*w); w->step(); }
  return w;
}

}  // namespace

TEST_CASE("establishments: staff follows the roster, a layoff slows the mill, commands and by-hand work (A1)") {
  auto w = run(1, 3 * kTicksPerGameDay);
  const auto* econ = w->system<EconomySystem>();
  REQUIRE(econ);
  const Establishment* mill = econ->establishments().find(ids.mill);
  const Establishment* bakery = econ->establishments().find(ids.bakery);
  REQUIRE(mill);
  REQUIRE(bakery);
  // Six millers at 2 worker-hours a batch: 3 batches an hour for 23 hours (69). The layoff lands at the start of
  // tick 24 h and the hourly read sees it before the jump, so hour 24 already runs with three: 1 batch and half a
  // batch banked. The run stops before tick 72 h, so the last hourly jump is at 71 h: 47 hours at 1.5 an hour
  // from a half-batch bank (2, 1, 2, ... = 71). 69 + 1 + 71 = 141, with the bank empty at the end.
  CHECK(mill->staff == 3);
  CHECK(mill->produced == 141);
  CHECK(mill->labour_bank == 0);
  // The bakery: no power until 6 h; then two bakers make two batches an hour while flour lasts (40 batches shipped),
  // then nothing; staff set to 0 on day 2.
  CHECK(bakery->staff == 0);
  CHECK(bakery->produced == 40);
  CHECK(w->ledger.balance(ids.bakery, econ->goods().def(ids.bread).bulk) == 40 * 14'000);
  // Ana by hand: flour for 2 batches, water for 2.
  CHECK(w->ledger.balance(ids.ana, econ->goods().def(ids.bread).bulk) == 2 * 14'000);
  CHECK(w->ledger.balance(ids.ana, econ->goods().def(ids.flour).bulk) == 0);
  // Macro: the region's milling is the mill's; everything under the country sums the three producers.
  CHECK(econ->establishments().produced_under(econ->owners(), ids.region, ids.milling) == mill->produced);
  CHECK(econ->establishments().produced_under(econ->owners(), ids.country) == mill->produced + bakery->produced);
  int refused = 0, produced = 0;
  for (const Event& e : w->log.all()) {
    if (e.type == EventType::EconRefused) { ++refused; CHECK(e.b == static_cast<std::int64_t>(EconRefusal::StaffFromRoster)); }
    if (e.type == EventType::EconProduced) ++produced;
  }
  CHECK(refused == 1);
  CHECK(produced > 48);
  CHECK(!w->ledger.check().has_value());
}

TEST_CASE("establishments: save, load, save identical; replay and resume reproduce the hash (A1)") {
  const std::int64_t ticks = 2 * kTicksPerGameDay + 50;
  auto w = run(2, ticks);
  const auto bytes = w->save();
  auto candidate = make_world(2);
  candidate->load(bytes);
  CHECK(candidate->save() == bytes);
  CHECK(candidate->state_hash() == w->state_hash());
  auto r = make_world(2);
  std::size_t next = 0;
  const auto& inputs = w->input_log();
  for (std::int64_t i = 0; i < ticks; ++i) {
    while (next < inputs.size() && inputs[next].tick == r->clock.tick) r->submit(inputs[next++]);
    r->step();
  }
  CHECK(r->state_hash() == w->state_hash());
  const std::int64_t half = kTicksPerGameDay + 7;
  auto first = run(2, half);
  auto resumed = make_world(2);
  resumed->load(first->save());
  for (std::int64_t i = half; i < ticks; ++i) { script(*resumed); resumed->step(); }
  CHECK(resumed->state_hash() == w->state_hash());
}
