// Spec 34: projects on the ledger alone (order, escrow, stages, stalls, finish, cancel, exact jump) and inside a world.
#include <doctest.h>

#include <memory>

#include "zs/clock.h"
#include "zs/economy_system.h"
#include "zs/project.h"

using namespace zs;

namespace {

// A clinic of 20 stages: a stage takes 1,000 kg concrete and 100 kg steel and 8 worker-days; the plan costs 20.00
// a stage from a budget of 500.00; finished, the clinic runs a "care" recipe (bandages from cloth).
struct Fixture {
  Ledger l;
  Owners owners;
  Goods goods;
  Recipes recipes;
  Establishments estabs;
  Projects projects;
  Money money{l};
  OwnerId consumed, country, region, state, builder, plot, depot;
  GoodId concrete, steel, clinic, cloth, bandage;
  RecipeId build, care;
  PlanId plan;

  Fixture() {
    consumed = owners.add(l, "world.consumed", Scope::Sink);
    country = owners.add(l, "country", Scope::Country);
    region = owners.add(l, "region", Scope::Region, country);
    state = owners.add(l, "state", Scope::Building, country);  // the treasury's account
    builder = owners.add(l, "builder", Scope::Building, region);
    plot = owners.add(l, "plot", Scope::Building, region);
    depot = owners.add(l, "depot", Scope::Building, region);
    concrete = goods.add(l, "concrete", "concrete_kg", "concrete_t", 1'000);
    steel = goods.add(l, "steel", "steel_kg", "steel_t", 1'000);
    clinic = goods.add(l, "clinic", "clinic_stage", "clinic", 20);
    cloth = goods.add(l, "cloth", "cloth_m2", "cloth_bolt", 50);
    bandage = goods.add(l, "bandage", "bandage_m2", "bandage_roll", 2);
    build = recipes.add(l, goods, {"build clinic stage", concrete, 1'000, clinic, 1, {{steel, 100}}, 8 * kTicksPerGameDay, 0});
    care = recipes.add(l, goods, {"make bandages", cloth, 10, bandage, 9, {}, kTicksPerGameHour, 0});
    plan = projects.add_plan(goods, recipes, {"clinic", build, clinic, 50'000, 2'000, builder, care});
    estabs.add({plot, build, 0, 0, 0});
    l.mint(state, money.resource(), 100'000);
    l.mint(depot, goods.def(concrete).bulk, 50'000);
    l.mint(depot, goods.def(steel).bulk, 5'000);
    l.mint(plot, goods.def(cloth).bulk, 1'000);  // for the clinic once it runs
  }
  void ship(std::int64_t tick, GoodId g, std::int64_t amount) { l.transfer(tick, depot, plot, goods.def(g).bulk, amount, 9); }
  std::vector<Projects::Change> run_to(std::int64_t tick) {
    projects.before_advance(l, goods, money, estabs);
    estabs.advance(l, goods, recipes, tick, consumed, 1);
    return projects.after_advance(l, goods, recipes, money, estabs, tick, 1);
  }
};

}  // namespace

TEST_CASE("order escrows the whole budget or is refused; cancel returns what is left (A1)") {
  Fixture f;
  f.l.close_setup();
  CHECK(f.projects.order(f.l, f.goods, f.money, f.estabs, 1, f.depot, f.plan, f.state, 1) == Projects::OrderResult::NoSite);
  CHECK(f.projects.order(f.l, f.goods, f.money, f.estabs, 1, f.plot, f.plan, f.builder, 1) == Projects::OrderResult::NoBudget);
  CHECK(f.projects.order(f.l, f.goods, f.money, f.estabs, 1, f.plot, f.plan, f.state, 1) == Projects::OrderResult::Ok);
  CHECK(f.l.balance(f.plot, f.money.resource()) == 50'000);
  CHECK(f.l.balance(f.state, f.money.resource()) == 50'000);
  CHECK(f.projects.order(f.l, f.goods, f.money, f.estabs, 2, f.plot, f.plan, f.state, 2) == Projects::OrderResult::SiteBusy);
  CHECK(f.projects.cancel(f.l, f.money, f.estabs, 3, f.plot, 3) == 50'000);
  CHECK(f.projects.cancel(f.l, f.money, f.estabs, 3, f.plot, 3) == -1);
  CHECK(f.l.balance(f.state, f.money.resource()) == 100'000);
  CHECK(f.projects.find(f.plot)->state == ProjectState::Cancelled);
  CHECK(!f.l.check().has_value());
}

TEST_CASE("a project is built by stages as materials arrive; stalls name their reason; it finishes exactly and then runs its recipe (A1)") {
  Fixture f;
  f.l.close_setup();
  const std::int64_t day = kTicksPerGameDay;
  REQUIRE(f.projects.order(f.l, f.goods, f.money, f.estabs, 0, f.plot, f.plan, f.state, 1) == Projects::OrderResult::Ok);
  // No staff: stalled.
  auto ch = f.run_to(1 * day);
  REQUIRE(ch.size() == 1);
  CHECK(ch[0].what == Projects::Change::Stalled);
  CHECK(ch[0].stall == StallReason::NoStaff);
  // Eight workers (a stage a day), but no materials.
  f.estabs.find(f.plot)->staff = 8;
  ch = f.run_to(2 * day);
  REQUIRE(ch.size() == 1);
  CHECK(ch[0].stall == StallReason::NoMaterials);
  // Materials for 5 stages arrive: five days of work, then stalled again.
  f.ship(2 * day, f.concrete, 5'000);
  f.ship(2 * day, f.steel, 500);
  ch = f.run_to(3 * day);
  REQUIRE(ch.size() == 1);
  CHECK(ch[0].what == Projects::Change::Resumed);
  ch = f.run_to(7 * day);  // the fifth stage is built and the materials are gone: stalled at the end of the jump
  CHECK(f.projects.view(f.l, f.goods, f.money, f.plot).stages_done == 5);
  CHECK(f.l.balance(f.builder, f.money.resource()) == 5 * 2'000);
  REQUIRE(ch.size() == 1);
  CHECK(ch[0].stall == StallReason::NoMaterials);
  ch = f.run_to(8 * day);
  CHECK(ch.empty());  // still stalled, nothing new to say
  CHECK(f.projects.view(f.l, f.goods, f.money, f.plot).progress == doctest::Approx(0.25));
  // The rest arrives with some to spare; the project finishes at exactly 20 stages and the spare stays on site.
  f.ship(8 * day, f.concrete, 20'000);
  f.ship(8 * day, f.steel, 2'000);
  ch = f.run_to(9 * day);
  CHECK(ch.size() == 1);  // resumed
  ch = f.run_to(30 * day);
  REQUIRE(ch.size() == 1);
  CHECK(ch[0].what == Projects::Change::Finished);
  CHECK(ch[0].tick == 30 * day);
  CHECK(f.l.balance(f.plot, f.goods.def(f.clinic).unit) == 1);
  CHECK(f.l.balance(f.plot, f.goods.def(f.clinic).bulk) == 0);
  CHECK(f.l.balance(f.plot, f.goods.def(f.concrete).bulk) == 5'000);  // 25 t came, 20 t went into the building
  CHECK(f.l.balance(f.builder, f.money.resource()) == 20 * 2'000);
  CHECK(f.l.balance(f.plot, f.money.resource()) == 50'000 - 40'000);  // escrow left on site
  CHECK(f.l.total(f.money.resource()) == 100'000);
  const auto v = f.projects.view(f.l, f.goods, f.money, f.plot);
  CHECK(v.state == ProjectState::Finished);
  CHECK(v.progress == 1.0);
  // The clinic now runs its own recipe with the same staff: cloth in, bandages out.
  CHECK(f.estabs.find(f.plot)->recipe == f.care);
  f.run_to(31 * day);
  CHECK(f.l.balance(f.plot, f.goods.def(f.bandage).bulk) > 0);
  CHECK(!f.l.check().has_value());
}

TEST_CASE("a project stops when the escrow cannot pay the next stage; stepping and one jump agree (A1)") {
  auto build = [](Fixture& f) {
    f.l.close_setup();
    f.projects.order(f.l, f.goods, f.money, f.estabs, 0, f.plot, f.plan, f.state, 1);
    f.estabs.find(f.plot)->staff = 16;  // two stages a day
    f.ship(0, f.concrete, 30'000);
    f.ship(0, f.steel, 3'000);
    // Spend most of the escrow first: only 7 stages can be paid.
    f.money.pay(f.l, 0, f.plot, f.state, 50'000 - 7 * 2'000, FlowKind::Grant, 1);
  };
  Fixture hot, cold;
  build(hot);
  build(cold);
  const std::int64_t span = 10 * kTicksPerGameDay;
  std::vector<Projects::Change> hot_changes;
  for (std::int64_t t = 1; t <= span; ++t) for (const auto& c : hot.run_to(t)) hot_changes.push_back(c);
  const auto cold_changes = cold.run_to(span);
  CHECK(hot.l.balances() == cold.l.balances());
  CHECK(hot.estabs.at(0).labour_bank == cold.estabs.at(0).labour_bank);
  CHECK(hot.projects.view(hot.l, hot.goods, hot.money, hot.plot).stages_done == 7);
  CHECK(hot.l.balance(hot.plot, hot.money.resource()) == 0);
  REQUIRE(cold_changes.size() == 1);
  CHECK(cold_changes[0].stall == StallReason::NoMoney);
  REQUIRE(!hot_changes.empty());
  CHECK(hot_changes.back().stall == StallReason::NoMoney);
  CHECK(!hot.l.check().has_value());
}

// ---- In a world.

namespace {
struct Ids {
  OwnerId country, region, state, builder, plot, depot;
  GoodId concrete, steel, clinic;
  RecipeId build;
  PlanId plan;
};
Ids ids;

std::unique_ptr<World> make_world(std::uint64_t seed) {
  auto w = std::make_unique<World>(seed);
  auto econ = std::make_unique<EconomySystem>(*w);
  Owners& o = econ->owners();
  Goods& g = econ->goods();
  ids.country = o.add(w->ledger, "country", Scope::Country);
  ids.region = o.add(w->ledger, "region", Scope::Region, ids.country);
  ids.state = o.add(w->ledger, "state", Scope::Building, ids.country);
  ids.builder = o.add(w->ledger, "builder", Scope::Building, ids.region);
  ids.plot = o.add(w->ledger, "plot", Scope::Building, ids.region);
  ids.depot = o.add(w->ledger, "depot", Scope::Building, ids.region);
  ids.concrete = g.add(w->ledger, "concrete", "concrete_kg", "concrete_t", 1'000);
  ids.steel = g.add(w->ledger, "steel", "steel_kg", "steel_t", 1'000);
  ids.clinic = g.add(w->ledger, "clinic", "clinic_stage", "clinic", 10);
  ids.build = econ->recipes().add(w->ledger, g, {"build", ids.concrete, 1'000, ids.clinic, 1, {{ids.steel, 100}}, 8 * kTicksPerGameHour, 0});
  ids.plan = econ->projects().add_plan(g, econ->recipes(), {"clinic", ids.build, ids.clinic, 30'000, 2'000, ids.builder, std::nullopt});
  econ->establishments().add({ids.plot, ids.build, 0, 8, 0});
  w->ledger.mint(ids.state, econ->money().resource(), 100'000);
  w->ledger.mint(ids.depot, g.def(ids.concrete).bulk, 20'000);
  w->ledger.mint(ids.depot, g.def(ids.steel).bulk, 2'000);
  econ->start_lots(*w);
  econ->start_establishments(*w);
  w->add_system(std::move(econ));
  w->close_setup();
  w->check_every_tick = true;
  return w;
}

void script(World& w) {
  const std::int64_t t = w.clock.tick, day = kTicksPerGameDay;
  if (t == 10) w.submit({0, ProjectOrder, ids.plot, ids.plan, ids.state});
  if (t == 11) w.submit({0, ProjectOrder, ids.plot, ids.plan, ids.state});                 // refused: site busy
  if (t == 2 * day) w.submit({0, EconShip, ids.depot, ids.plot, econ_ship_arg(ids.concrete, 4'000)});
  if (t == 2 * day + 1) w.submit({0, EconShip, ids.depot, ids.plot, econ_ship_arg(ids.steel, 400)});
  if (t == 7 * day) w.submit({0, EconShip, ids.depot, ids.plot, econ_ship_arg(ids.concrete, 6'000)});
  if (t == 7 * day + 1) w.submit({0, EconShip, ids.depot, ids.plot, econ_ship_arg(ids.steel, 600)});
}

std::unique_ptr<World> run(std::uint64_t seed, std::int64_t ticks) {
  auto w = make_world(seed);
  for (std::int64_t i = 0; i < ticks; ++i) { script(*w); w->step(); }
  return w;
}
}  // namespace

TEST_CASE("projects in a world: ordered, stalled for materials, resumed, finished; events in order; money conserved (A1)") {
  auto w = run(1, 16 * kTicksPerGameDay);
  const auto* econ = w->system<EconomySystem>();
  REQUIRE(econ);
  std::vector<EventType> seq;
  for (const Event& e : w->log.all())
    if (e.type >= EventType::ProjectOrdered && e.type <= EventType::ProjectCancelled) seq.push_back(e.type);
  REQUIRE(seq.size() >= 4);
  CHECK(seq[0] == EventType::ProjectOrdered);
  CHECK(seq[1] == EventType::ProjectStalled);  // no materials for two days
  CHECK(seq[2] == EventType::ProjectResumed);
  CHECK(seq.back() == EventType::ProjectFinished);
  int refused = 0;
  for (const Event& e : w->log.all()) if (e.type == EventType::EconRefused) { ++refused; CHECK(e.b == static_cast<std::int64_t>(EconRefusal::SiteBusy)); }
  CHECK(refused == 1);
  CHECK(w->ledger.balance(ids.plot, econ->goods().def(ids.clinic).unit) == 1);
  CHECK(w->ledger.balance(ids.builder, econ->money().resource()) == 10 * 2'000);
  CHECK(w->ledger.total(econ->money().resource()) == 100'000);
  CHECK(econ->owners().aggregate(w->ledger, ids.country, econ->goods().def(ids.clinic).unit) == 1);  // Macro: the building stock
  const auto v = econ->projects().view(w->ledger, econ->goods(), econ->money(), ids.plot);
  CHECK(v.state == ProjectState::Finished);
  CHECK(v.money_left == 30'000 - 20'000);
}

TEST_CASE("projects in a world: save, load, save identical; replay and resume reproduce the hash (A1)") {
  const std::int64_t ticks = 9 * kTicksPerGameDay + 5;
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
  const std::int64_t half = 4 * kTicksPerGameDay + 3;
  auto first = run(2, half);
  auto resumed = make_world(2);
  resumed->load(first->save());
  for (std::int64_t i = half; i < ticks; ++i) { script(*resumed); resumed->step(); }
  CHECK(resumed->state_hash() == w->state_hash());
}
