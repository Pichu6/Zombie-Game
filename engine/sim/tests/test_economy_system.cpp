// Spec 32: the economy inside a world: commands, hourly spoilage, refusals, save, hash and replay.
#include <doctest.h>

#include <memory>

#include "zs/economy_system.h"

using namespace zs;

namespace {

struct Ids {
  OwnerId country, region, farm, shop, ana;
  GoodId grain, milk;
};

Ids ids;  // the same setup every time, so ids are stable

std::unique_ptr<World> make_world(std::uint64_t seed) {
  auto w = std::make_unique<World>(seed);
  auto econ = std::make_unique<EconomySystem>(*w);
  Owners& o = econ->owners();
  Goods& g = econ->goods();
  ids.country = o.add(w->ledger, "country", Scope::Country);
  ids.region = o.add(w->ledger, "region", Scope::Region, ids.country);
  ids.farm = o.add(w->ledger, "farm", Scope::Building, ids.region);
  ids.shop = o.add(w->ledger, "shop", Scope::Building, ids.region);
  ids.ana = o.add(w->ledger, "ana", Scope::Pop, ids.shop);
  ids.grain = g.add(w->ledger, "grain", "grain_g", "grain_sack", 50'000, 0);
  ids.milk = g.add(w->ledger, "milk", "milk_ml", "milk_bottle", 1'000, 2.0);
  w->ledger.mint(ids.farm, g.def(ids.grain).bulk, 2'000'000);
  w->ledger.mint(ids.farm, g.def(ids.milk).bulk, 400'000);
  w->ledger.mint(ids.country, econ->money().resource(), 5'000'000);
  w->ledger.mint(ids.shop, econ->money().resource(), 80'000);
  w->ledger.mint(ids.ana, econ->money().resource(), 9'000);
  econ->start_lots(*w);
  w->add_system(std::move(econ));
  w->close_setup();
  w->check_every_tick = true;
  return w;
}

// A scripted player over four game days: the farm ships to the shop, the shop puts goods on the shelf, Ana is paid
// and buys, taxes flow up, the shop packs the shelf away again.
void script(World& w) {
  const std::int64_t t = w.clock.tick, day = kTicksPerGameDay, hour = kTicksPerGameHour;
  if (t == 1 * hour) w.submit({0, EconShip, ids.farm, ids.shop, econ_ship_arg(ids.grain, 600'000)});
  if (t == 1 * hour + 1) w.submit({0, EconShip, ids.farm, ids.shop, econ_ship_arg(ids.milk, 100'000)});
  if (t == 2 * hour) w.submit({0, EconVisit, ids.shop, ids.grain, 100});
  if (t == 2 * hour + 1) w.submit({0, EconVisit, ids.shop, ids.milk, 40});
  if (t == 1 * day) w.submit({0, EconPay, ids.shop, ids.ana, econ_pay_arg(FlowKind::Wage, 12'000)});
  if (t == 1 * day + 5) w.submit({0, EconPurchase, ids.ana, ids.shop, econ_purchase_arg(ids.grain, 2, 3'500)});
  if (t == 1 * day + 6) w.submit({0, EconPurchase, ids.ana, ids.shop, econ_purchase_arg(ids.milk, 3, 150)});
  if (t == 2 * day) w.submit({0, EconPay, ids.ana, ids.region, econ_pay_arg(FlowKind::Tax, 1'000)});
  if (t == 2 * day + 1) w.submit({0, EconPay, ids.region, ids.country, econ_pay_arg(FlowKind::Tax, 800)});
  if (t == 3 * day) w.submit({0, EconLeave, ids.shop, ids.grain, 0});
  if (t == 3 * day + 7) w.submit({0, EconPurchase, ids.ana, ids.shop, econ_purchase_arg(ids.grain, 1, 3'500)});  // refused: shelf empty
}

std::unique_ptr<World> run(std::uint64_t seed, std::int64_t ticks) {
  auto w = make_world(seed);
  for (std::int64_t i = 0; i < ticks; ++i) { script(*w); w->step(); }
  return w;
}

int count(const World& w, EventType type) {
  int n = 0;
  for (const Event& e : w.log.all()) if (e.type == type) ++n;
  return n;
}

}  // namespace

TEST_CASE("economy: the script moves goods and money as expected, and totals hold (A1)") {
  auto w = run(1, 4 * kTicksPerGameDay);
  const auto* econ = w->system<EconomySystem>();
  REQUIRE(econ);
  const Ledger& l = w->ledger;
  const ResourceId money = econ->money().resource();
  const GoodDef& grain = econ->goods().def(ids.grain);
  const GoodDef& milk = econ->goods().def(ids.milk);
  // Grain does not spoil: 2,000,000 g stay in play, as grams or sacks.
  CHECK(l.total(grain.bulk) + 50'000 * l.total(grain.unit) == 2'000'000);
  CHECK(l.balance(ids.ana, grain.unit) == 2);
  CHECK(l.balance(ids.shop, grain.unit) == 0);  // the shelf was packed away on day 3
  CHECK(l.balance(ids.shop, grain.bulk) == 600'000 - 2 * 50'000);
  // Milk spoils: what left play went to the decay sink and nowhere else.
  CHECK(l.total(milk.bulk) + 1'000 * l.total(milk.unit) == 400'000);
  CHECK(l.balance(econ->decay_sink(), milk.bulk) > 0);
  CHECK(l.balance(ids.ana, milk.unit) == 3);
  // Money: no mint. Ana: 9,000 + 12,000 wage - 7,000 grain - 450 milk - 1,000 tax.
  CHECK(l.total(money) == 5'000'000 + 80'000 + 9'000);
  CHECK(l.balance(ids.ana, money) == 9'000 + 12'000 - 7'000 - 450 - 1'000);
  CHECK(econ->money().received(ids.shop, FlowKind::Sale) == 7'450);
  CHECK(econ->money().paid(ids.shop, FlowKind::Wage) == 12'000);
  CHECK(econ->money().received(ids.country, FlowKind::Tax) == 800);
  CHECK(count(*w, EventType::EconTraded) == 2);
  CHECK(count(*w, EventType::EconPaid) == 3);
  CHECK(count(*w, EventType::EconRefused) == 1);
  CHECK(count(*w, EventType::EconSpoiled) > 0);
}

TEST_CASE("economy: refusals name their reason and change nothing") {
  auto w = make_world(2);
  const auto hash = w->state_hash();
  w->submit({0, EconPurchase, 999, ids.shop, econ_purchase_arg(ids.grain, 1, 100)});  // unknown buyer
  w->submit({0, EconVisit, ids.farm, 77, 10});                                        // unknown good
  w->submit({0, EconPay, ids.ana, ids.ana, econ_pay_arg(FlowKind::Grant, 100)});       // to itself
  w->submit({0, EconPurchase, ids.ana, ids.shop, econ_purchase_arg(ids.grain, 1, 100)});  // empty shelf
  w->submit({0, EconShip, ids.farm, w->system<EconomySystem>()->decay_sink(), econ_ship_arg(ids.grain, 5)});  // a sink is not a destination
  w->step();
  const auto& log = w->log.all();
  std::vector<std::int64_t> reasons;
  for (const Event& e : log) if (e.type == EventType::EconRefused) reasons.push_back(e.b);
  REQUIRE(reasons.size() == 5);
  CHECK(reasons[0] == static_cast<std::int64_t>(EconRefusal::UnknownOwner));
  CHECK(reasons[1] == static_cast<std::int64_t>(EconRefusal::UnknownGood));
  CHECK(reasons[2] == static_cast<std::int64_t>(EconRefusal::Nothing));
  CHECK(reasons[3] == static_cast<std::int64_t>(EconRefusal::Nothing));
  CHECK(reasons[4] == static_cast<std::int64_t>(EconRefusal::UnknownOwner));
  CHECK(w->ledger.balances() == make_world(2)->ledger.balances());
  CHECK(hash != w->state_hash());  // the refusals are in the log, as every event is
}

TEST_CASE("economy: save, load, save gives identical bytes; the hash includes lots and tallies (A1)") {
  auto w = run(3, 2 * kTicksPerGameDay + 100);
  const auto bytes = w->save();
  auto candidate = make_world(3);
  candidate->load(bytes);
  CHECK(candidate->save() == bytes);
  CHECK(candidate->state_hash() == w->state_hash());
  const auto* a = w->system<EconomySystem>();
  const auto* b = candidate->system<EconomySystem>();
  CHECK(a->spoilage().lots().size() == b->spoilage().lots().size());
  CHECK(a->money().received_tallies() == b->money().received_tallies());
  const auto before = w->state_hash();
  w->submit({0, EconPay, ids.country, ids.region, econ_pay_arg(FlowKind::Grant, 10)});
  w->step();
  CHECK(w->state_hash() != before);
}

TEST_CASE("economy: replaying the input log reproduces the hash (A1)") {
  const std::int64_t ticks = 4 * kTicksPerGameDay;
  auto w = run(4, ticks);
  auto r = make_world(4);
  std::size_t next = 0;
  const auto& inputs = w->input_log();
  for (std::int64_t i = 0; i < ticks; ++i) {
    while (next < inputs.size() && inputs[next].tick == r->clock.tick) r->submit(inputs[next++]);
    r->step();
  }
  CHECK(r->state_hash() == w->state_hash());
}

TEST_CASE("economy: a load into a continuing world matches never having stopped, spoilage included (A1)") {
  const std::int64_t half = 2 * kTicksPerGameDay + 3, whole = 4 * kTicksPerGameDay;
  auto full = run(5, whole);
  auto first = run(5, half);
  auto resumed = make_world(5);
  resumed->load(first->save());
  for (std::int64_t i = half; i < whole; ++i) { script(*resumed); resumed->step(); }
  CHECK(resumed->state_hash() == full->state_hash());
}

TEST_CASE("economy: 0051 panels read from the world") {
  auto w = run(6, 3 * kTicksPerGameDay);  // past the taxes on day 2
  const auto* econ = w->system<EconomySystem>();
  Context ctx{ids.ana, ids.region, ids.country, std::nullopt};
  const Panel mine = econ->panel(*w, Layer::Individual, ctx);
  CHECK(mine.owner == ids.ana);
  CHECK(mine.units[ids.grain] == 2);
  CHECK(mine.money == w->ledger.balance(ids.ana, econ->money().resource()));
  ctx.selected = ids.shop;
  const Panel shop = econ->panel(*w, Layer::City, ctx);
  CHECK(shop.owner == ids.shop);
  CHECK(!shop.aggregated);
  ctx.selected.reset();
  const Panel region = econ->panel(*w, Layer::City, ctx);
  CHECK(region.owner == ids.region);
  CHECK(region.money == 1'000 - 800);  // the tax it kept, nothing of its buildings'
  const Panel country = econ->panel(*w, Layer::Macro, ctx);
  CHECK(country.aggregated);
  CHECK(country.money == 5'000'000 + 80'000 + 9'000);  // everything in the country, by enumeration
}
