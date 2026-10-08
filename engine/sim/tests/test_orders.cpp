// Spec 39: edicts applied through ordinary commands with the edict as cause; the trace from a transfer to its edict;
// beliefs from broadcasts and retellings; the lens; save and replay.
#include <doctest.h>

#include <memory>

#include "zs/orders_system.h"

using namespace zs;

namespace {

struct Ids {
  OwnerId country, region, store, farm1, farm2, mill;
  GoodId grain;
  std::uint32_t levy, tax, layoff;
  std::uint64_t mill_roster = 5;
};
Ids ids;

std::unique_ptr<World> make_world(std::uint64_t seed) {
  auto w = std::make_unique<World>(seed);
  auto pops = std::make_unique<PopSystem>(*w);
  PopRegistry& reg = pops->setup(*w);
  reg.add_cohort({1, 0, 0, 100});
  reg.add_roster(ids.mill_roster, 0, RosterKind::Workplace);
  for (PopId p = 0; p < 10; ++p) reg.roster_join(ids.mill_roster, p, 0.0);
  auto econ = std::make_unique<EconomySystem>(*w);
  Owners& o = econ->owners();
  Goods& g = econ->goods();
  ids.country = o.add(w->ledger, "country", Scope::Country);
  ids.region = o.add(w->ledger, "region", Scope::Region, ids.country);
  ids.store = o.add(w->ledger, "national store", Scope::Building, ids.country);
  ids.farm1 = o.add(w->ledger, "farm 1", Scope::Building, ids.region);
  ids.farm2 = o.add(w->ledger, "farm 2", Scope::Building, ids.region);
  ids.mill = o.add(w->ledger, "mill", Scope::Building, ids.region);
  ids.grain = g.add(w->ledger, "grain", "grain_g", "grain_sack", 50'000);
  w->ledger.mint(ids.farm1, g.def(ids.grain).bulk, 1'000'000);
  w->ledger.mint(ids.farm2, g.def(ids.grain).bulk, 500'000);
  w->ledger.mint(ids.farm1, econ->money().resource(), 100'000);
  w->ledger.mint(ids.mill, econ->money().resource(), 40'000);
  econ->start_lots(*w);
  econ->start_establishments(*w);
  econ->start_populations(*w);
  auto orders = std::make_unique<OrdersSystem>(econ.get(), pops.get());
  ids.levy = orders->edicts().add_def({"grain levy 10%", EdictKind::Levy, ids.region, ids.store, ids.grain, 100, 0, 0});
  ids.tax = orders->edicts().add_def({"business tax 5%", EdictKind::Tax, ids.region, ids.country, 0, 50, 0, 0});
  ids.layoff = orders->edicts().add_def({"lay off three", EdictKind::Staffing, ids.region, 0, 0, 0, 3, 0});
  std::vector<PopId> audience;
  for (PopId p = 0; p < 10; ++p) audience.push_back(p);
  orders->set_audience(audience);
  w->add_system(std::move(pops));
  w->add_system(std::move(econ));
  w->add_system(std::move(orders));
  w->close_setup();
  w->check_every_tick = true;
  return w;
}

void script(World& w) {
  const std::int64_t t = w.clock.tick, day = kTicksPerGameDay;
  if (t == 5) w.submit({0, EdictSign, ids.country, ids.levy, 0});       // edict 0: grain levy
  if (t == 6) w.submit({0, EdictSign, ids.country, 9, 0});              // refused: unknown def
  if (t == 1 * day + 1) w.submit({0, EdictSign, ids.country, ids.tax, 0});    // edict 1: tax
  if (t == 2 * day + 1) w.submit({0, SetComms, 0, 0, 0});               // the phones die
  if (t == 2 * day + 2) w.submit({0, EdictSign, ids.country, ids.layoff, 0}); // edict 2: layoffs, heard by nobody
  if (t == 2 * day + 3) w.submit({0, Tell, 0, 7, static_cast<std::int64_t>(edict_entity(0))});  // pop 0 tells pop 7 about the levy
  if (t == 2 * day + 4) w.submit({0, Tell, 7, 9, static_cast<std::int64_t>(edict_entity(0))});  // pop 7 retells to pop 9
  if (t == 2 * day + 5) w.submit({0, Tell, 3, 4, static_cast<std::int64_t>(edict_entity(2))});  // refused: 3 knows nothing of it
  if (t == 4 * day + 1) w.submit({0, EdictRepeal, 0, 0, 0});
  if (t == 4 * day + 2) w.submit({0, EdictRepeal, 0, 0, 0});            // refused: not in force
}

std::unique_ptr<World> run(std::uint64_t seed, std::int64_t ticks) {
  auto w = make_world(seed);
  for (std::int64_t i = 0; i < ticks; ++i) { script(*w); w->step(); }
  return w;
}

}  // namespace

TEST_CASE("an edict acts only through ordinary commands, each day, with the signing as root cause; its effects are sums (A1)") {
  auto w = run(1, 6 * kTicksPerGameDay);
  const auto* econ = w->system<EconomySystem>();
  const auto* orders = w->system<OrdersSystem>();
  REQUIRE(econ);
  REQUIRE(orders);
  const ResourceId grain = econ->goods().def(ids.grain).bulk;
  // The levy, signed on day 0, applied on days 1, 2, 3 and 4 (repealed at 4 d + 1, after that day's application):
  // 10 % of each farm's grain each day, compounding down: farm 1 keeps 0.9^4 of 1,000,000.
  CHECK(w->ledger.balance(ids.farm1, grain) == 656'100);
  CHECK(w->ledger.balance(ids.farm2, grain) == 328'050);
  CHECK(w->ledger.balance(ids.store, grain) == 1'500'000 - 656'100 - 328'050);
  const Edict& levy = orders->edicts().at(0);
  CHECK(!levy.in_force());
  CHECK(levy.applications == 4);
  CHECK(levy.moved == w->ledger.balance(ids.store, grain));
  // The tax, signed on day 1, applied on days 2 to 5: 5 % of each business's money a day to the country, as Tax.
  const Edict& tax = orders->edicts().at(1);
  CHECK(tax.in_force());
  CHECK(tax.applications == 4);
  CHECK(econ->money().received(ids.country, FlowKind::Tax) == tax.moved);
  CHECK(tax.moved > 0);
  CHECK(w->ledger.total(econ->money().resource()) == 140'000);
  // The trace: a ledger entry under the levy leads back to edict 0; one under the tax to edict 1.
  int traced_levy = 0, traced_tax = 0;
  for (const LedgerEntry& e : w->ledger.tail()) {
    const auto behind = orders->edicts().behind(w->log, e.cause);
    if (!behind) continue;
    if (e.resource_from == grain && *behind == 0) ++traced_levy;
    if (e.resource_from == econ->money().resource() && *behind == 1) ++traced_tax;
  }
  CHECK(traced_levy > 0);
  CHECK(traced_tax > 0);
  // The chain itself: transfer cause -> EdictApplied -> EdictSigned.
  const LedgerEntry& last = w->ledger.tail().back();
  const auto chain = Edicts::trace(w->log, last.cause);
  REQUIRE(chain.size() >= 2);
  CHECK(w->log.get(chain[0])->type == EventType::EdictApplied);
  CHECK(w->log.get(chain[1])->type == EventType::EdictSigned);
  // The staffing edict: three laid off at once on the mill's roster, with the edict as cause.
  const auto* pops = w->system<PopSystem>();
  REQUIRE(pops);
  CHECK(pops->registry().roster_count(ids.mill_roster, EffectKind::LaidOff, year_of_tick(w->clock.tick)) == 3);
  CHECK(orders->edicts().at(2).moved == 3);
  int refused = 0;
  for (const Event& e : w->log.all()) if (e.type == EventType::OrderRefused) ++refused;
  CHECK(refused == 3);  // unknown def, telling what one does not know, repealing twice
}

TEST_CASE("beliefs: a broadcast reaches everyone while communications are up, nobody after; retelling distorts; the lens shows belief, not truth (A1)") {
  auto w = run(2, 3 * kTicksPerGameDay);
  const auto* orders = w->system<OrdersSystem>();
  REQUIRE(orders);
  const Beliefs& b = orders->beliefs();
  const std::int64_t now = w->clock.tick;
  // Everyone heard the levy (100 per mille) and the tax; nobody heard the layoffs (comms were down).
  for (PopId p = 0; p < 10; ++p) {
    const auto levy = b.view(p, edict_entity(0), now);
    CHECK(levy.known);
    CHECK(levy.source == (p == 7 || p == 9 ? Source::Told : Source::Broadcast));
    CHECK(b.view(p, edict_entity(1), now).claim == 50);
    CHECK(!b.view(p, edict_entity(2), now).known);
  }
  CHECK(b.view(0, edict_entity(0), now).claim == 100);
  CHECK(b.view(0, edict_entity(0), now).age_ticks == now - 5);
  // Pop 7 was told by pop 0, pop 9 by pop 7: hops count, confidence falls, and the story drifts.
  const auto v7 = b.view(7, edict_entity(0), now), v9 = b.view(9, edict_entity(0), now);
  CHECK(v7.hops == 1);
  CHECK(v9.hops == 2);
  CHECK(v7.confidence == doctest::Approx(0.7));
  CHECK(v9.confidence == doctest::Approx(0.49));
  CHECK(b.about(7, edict_entity(0))->from == 0);
  CHECK(b.about(9, edict_entity(0))->from == 7);
  // The lens never reads the truth: the truth is 100 per mille; a retold claim is what was told, drifted.
  CHECK(orders->edicts().def(0).per_mille == 100);
  CHECK((v7.claim != 100 || v9.claim != 100));  // at 15 % per hop, at least one of the two drifted
  // Retelling is keyed: the same telling again gives the same story (one story, not two).
  Beliefs copy = b;
  copy.retell(w->rng.seed(), 0, 7, edict_entity(0), now, orders->distortion_per_hop());
  CHECK(copy.view(7, edict_entity(0), now).claim == v7.claim);
}

TEST_CASE("orders: save, load, save identical; replay and resume reproduce the hash (A1)") {
  const std::int64_t ticks = 5 * kTicksPerGameDay + 3;
  auto w = run(3, ticks);
  const auto bytes = w->save();
  auto candidate = make_world(3);
  candidate->load(bytes);
  CHECK(candidate->save() == bytes);
  CHECK(candidate->state_hash() == w->state_hash());
  CHECK(candidate->system<OrdersSystem>()->beliefs().pops() == w->system<OrdersSystem>()->beliefs().pops());
  CHECK(!candidate->system<OrdersSystem>()->comms_up());
  auto r = make_world(3);
  std::size_t next = 0;
  const auto& inputs = w->input_log();
  for (std::int64_t i = 0; i < ticks; ++i) {
    while (next < inputs.size() && inputs[next].tick == r->clock.tick) r->submit(inputs[next++]);
    r->step();
  }
  CHECK(r->state_hash() == w->state_hash());
  const std::int64_t half = 2 * kTicksPerGameDay + 2;
  auto first = run(3, half);
  auto resumed = make_world(3);
  resumed->load(first->save());
  for (std::int64_t i = half; i < ticks; ++i) { script(*resumed); resumed->step(); }
  CHECK(resumed->state_hash() == w->state_hash());
}
