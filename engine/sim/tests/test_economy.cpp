// Spec 32: stocks, goods and money across levels, and decision 0051 (whose account a panel shows).
#include <doctest.h>
#include "property.h"

#include <vector>

#include "zs/clock.h"
#include "zs/economy.h"

using namespace zs;

namespace {

// A small world: one country, two regions, a farm and a shop in the first region, a household in the second,
// two pops, a decay sink. Grain in grams, sacks of 50 kg; water in millilitres, cans of 1 L.
struct Fixture {
  Ledger l;
  Owners owners;
  Goods goods;
  Money money{l};
  OwnerId decayed, country, region1, region2, farm, shop, house, ana, bo;
  GoodId grain, water;

  Fixture() {
    decayed = owners.add(l, "world.decayed", Scope::Sink);
    country = owners.add(l, "country", Scope::Country);
    region1 = owners.add(l, "region1", Scope::Region, country);
    region2 = owners.add(l, "region2", Scope::Region, country);
    farm = owners.add(l, "farm", Scope::Building, region1);
    shop = owners.add(l, "shop", Scope::Building, region1);
    house = owners.add(l, "house", Scope::Building, region2);
    ana = owners.add(l, "ana", Scope::Pop, house);
    bo = owners.add(l, "bo", Scope::Pop, farm);
    grain = goods.add(l, "grain", "grain_g", "grain_sack", 50'000, 0);
    water = goods.add(l, "water", "water_ml", "water_can", 1'000, 3.0);  // spoils: half life 3 days
    l.mint(farm, goods.def(grain).bulk, 1'234'567);
    l.mint(shop, goods.def(grain).bulk, 100'000);
    l.mint(region1, goods.def(grain).bulk, 7);
    l.mint(country, goods.def(water).bulk, 20'000);
    l.mint(country, money.resource(), 1'000'000);
    l.mint(farm, money.resource(), 50'000);
    l.mint(shop, money.resource(), 20'000);
    l.mint(ana, money.resource(), 3'000);
    l.close_setup();
  }
  std::int64_t grain_total() const { return l.total(goods.def(grain).bulk) + l.total(goods.def(grain).unit) * 50'000; }
};

}  // namespace

TEST_CASE("enumeration: an owner's aggregate is its own account plus its descendants' (A1)") {
  Fixture f;
  const ResourceId g = f.goods.def(f.grain).bulk;
  CHECK(f.owners.aggregate(f.l, f.region1, g) == 1'234'567 + 100'000 + 7);
  CHECK(f.owners.aggregate(f.l, f.region2, g) == 0);
  CHECK(f.owners.aggregate(f.l, f.country, g) == 1'234'567 + 100'000 + 7);
  const auto rows = f.owners.by_children(f.l, f.country, g);
  std::int64_t sum = 0;
  for (const auto& r : rows) sum += r.amount;
  CHECK(sum == f.owners.aggregate(f.l, f.country, g));
  CHECK(rows.size() == 3);  // two regions and the country's own account
  CHECK(rows.back().owner == f.country);
}

TEST_CASE("enumeration holds under random transfers inside the country; nothing leaves without a sink (A1)") {
  property("aggregate invariant", [](const std::vector<std::tuple<std::uint8_t, std::uint8_t, std::int32_t>>& ops) {
    Fixture f;
    const ResourceId g = f.goods.def(f.grain).bulk;
    const OwnerId inside[] = {f.country, f.region1, f.region2, f.farm, f.shop, f.house, f.ana, f.bo};
    const std::int64_t before = f.owners.aggregate(f.l, f.country, g);
    std::int64_t tick = 1;
    for (const auto& [a, b, amount] : ops) {
      f.l.transfer(tick++, inside[a % 8], inside[b % 8], g, amount, 0);
      RC_ASSERT(f.owners.aggregate(f.l, f.country, g) == before);
      RC_ASSERT(!f.l.check().has_value());
    }
  });
}

TEST_CASE("a visit makes whole units from bulk, leaving returns them; mass is conserved and the remainder stays (A1)") {
  Fixture f;
  const GoodDef& d = f.goods.def(f.grain);
  const std::int64_t total = f.grain_total();
  CHECK(f.goods.materialise(f.l, 1, f.farm, f.grain, 1'000, 1) == 24);  // 1,234,567 g = 24 sacks + 34,567 g
  CHECK(f.l.balance(f.farm, d.unit) == 24);
  CHECK(f.l.balance(f.farm, d.bulk) == 34'567);
  CHECK(f.grain_total() == total);
  CHECK(f.goods.materialise(f.l, 2, f.farm, f.grain, 1'000, 1) == 0);  // nothing left to pack
  CHECK(f.goods.dematerialise(f.l, 3, f.farm, f.grain, 1) == 24);
  CHECK(f.l.balance(f.farm, d.bulk) == 1'234'567);
  CHECK(f.l.balance(f.farm, d.unit) == 0);
  CHECK(f.grain_total() == total);
  CHECK(!f.l.check().has_value());
}

TEST_CASE("visit round trip is the identity for any bulk amount, and a capped visit shows only what is held (A1)") {
  property("round trip", [](std::uint32_t amount, std::uint8_t cap) {
    Ledger l;
    Owners owners;
    Goods goods;
    const OwnerId w = owners.add(l, "warehouse", Scope::Building);
    const GoodId g = goods.add(l, "grain", "grain_g", "grain_sack", 50'000);
    l.mint(w, goods.def(g).bulk, amount);
    l.close_setup();
    const std::int64_t units = goods.materialise(l, 1, w, g, cap, 1);
    RC_ASSERT(units <= cap);
    RC_ASSERT(units * 50'000 <= amount);
    RC_ASSERT(units == std::min<std::int64_t>(cap, amount / 50'000));
    RC_ASSERT(l.balance(w, goods.def(g).bulk) + units * 50'000 == amount);
    goods.dematerialise(l, 2, w, g, 1);
    RC_ASSERT(l.balance(w, goods.def(g).bulk) == amount);
    RC_ASSERT(l.balance(w, goods.def(g).unit) == 0);
    RC_ASSERT(!l.check().has_value());
  });
}

TEST_CASE("spoilage: stepping every hour and one jump move exactly the same amount to the sink (A1)") {
  Fixture hot, cold;
  const std::int64_t days = 5;
  Spoilage sh, sc;
  sh.reset(hot.l, hot.goods, hot.country, hot.water, 0);
  sc.reset(cold.l, cold.goods, cold.country, cold.water, 0);
  std::int64_t moved_hot = 0;
  for (std::int64_t t = kTicksPerGameHour; t <= days * kTicksPerGameDay; t += kTicksPerGameHour)
    moved_hot += sh.advance(hot.l, hot.goods, hot.decayed, t, 1);
  const std::int64_t moved_cold = sc.advance(cold.l, cold.goods, cold.decayed, days * kTicksPerGameDay, 1);
  CHECK(moved_hot == moved_cold);
  const ResourceId wb = hot.goods.def(hot.water).bulk;
  CHECK(hot.l.balance(hot.country, wb) == cold.l.balance(cold.country, wb));
  CHECK(hot.l.balance(hot.decayed, wb) == moved_hot);
  // Five days at a three-day half life leaves 2^(-5/3) of 20 L: between 6.2 and 6.4 litres.
  CHECK(hot.l.balance(hot.country, wb) > 6'200);
  CHECK(hot.l.balance(hot.country, wb) < 6'400);
  CHECK(!hot.l.check().has_value());
  CHECK(!cold.l.check().has_value());
}

TEST_CASE("spoilage: any partition of the interval gives the same result; remaining is monotone (A1)") {
  property("partition independence", [](std::vector<std::uint16_t> steps) {
    Fixture a, b;
    Spoilage sa, sb;
    sa.reset(a.l, a.goods, a.country, a.water, 0);
    sb.reset(b.l, b.goods, b.country, b.water, 0);
    std::int64_t t = 0, last = 20'000;
    const ResourceId wb = a.goods.def(a.water).bulk;
    for (std::uint16_t s : steps) {
      t += static_cast<std::int64_t>(s) + 1;
      sa.advance(a.l, a.goods, a.decayed, t, 1);
      const std::int64_t now = a.l.balance(a.country, wb);
      RC_ASSERT(now <= last);
      last = now;
    }
    sb.advance(b.l, b.goods, b.decayed, t, 1);
    RC_ASSERT(a.l.balance(a.country, wb) == b.l.balance(b.country, wb));
    RC_ASSERT(a.l.balance(a.decayed, wb) == b.l.balance(b.decayed, wb));
    RC_ASSERT(!a.l.check().has_value());
  });
  CHECK(Spoilage::remaining(1000, 0, 1'000'000) == 1000);  // no half life, no decay
  CHECK(Spoilage::remaining(1000, 1.0, kTicksPerGameDay) == 500);
  CHECK(Spoilage::remaining(0, 1.0, kTicksPerGameDay) == 0);
  CHECK(Spoilage::remaining(1, 1.0, 100 * kTicksPerGameDay) == 1);  // ceil: the last unit waits for the curve
}

TEST_CASE("a purchase moves goods one way and money the other, all or nothing per unit (A1)") {
  Fixture f;
  const GoodDef& d = f.goods.def(f.grain);
  f.goods.materialise(f.l, 1, f.shop, f.grain, 100, 1);  // the shop has 2 sacks
  const std::int64_t money_total = f.l.total(f.money.resource());
  // Ana has 30.00; a sack costs 12.50: she can afford 2 but wants 5 and the shop has 2.
  CHECK(f.money.purchase(f.l, 2, f.ana, f.shop, f.goods, f.grain, 1'250, 5, 2) == 2);
  CHECK(f.l.balance(f.ana, d.unit) == 2);
  CHECK(f.l.balance(f.ana, f.money.resource()) == 500);
  CHECK(f.l.balance(f.shop, f.money.resource()) == 22'500);
  CHECK(f.money.received(f.shop, FlowKind::Sale) == 2'500);
  CHECK(f.money.paid(f.ana, FlowKind::Sale) == 2'500);
  // Now she cannot afford one more even if the shop restocks.
  f.goods.materialise(f.l, 3, f.shop, f.grain, 100, 3);
  CHECK(f.l.balance(f.shop, d.unit) == 0);  // the shop's bulk was 100,000 g = exactly the 2 sacks already sold
  f.l.transfer(3, f.farm, f.shop, d.bulk, 200'000, 3);
  f.goods.materialise(f.l, 3, f.shop, f.grain, 100, 3);
  CHECK(f.money.purchase(f.l, 4, f.ana, f.shop, f.goods, f.grain, 1'250, 1, 4) == 0);
  CHECK(f.l.total(f.money.resource()) == money_total);
  CHECK(!f.l.check().has_value());
}

TEST_CASE("money is conserved across random purchases, wages and taxes; no hidden mint (A1)") {
  property("no mint", [](const std::vector<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint16_t>>& ops) {
    Fixture f;
    f.goods.materialise(f.l, 0, f.farm, f.grain, 1'000, 0);
    f.goods.materialise(f.l, 0, f.shop, f.grain, 1'000, 0);
    const OwnerId who[] = {f.country, f.region1, f.region2, f.farm, f.shop, f.house, f.ana, f.bo};
    const std::int64_t total = f.l.total(f.money.resource());
    const std::int64_t sacks = f.l.total(f.goods.def(f.grain).unit);
    std::int64_t tick = 1;
    for (const auto& [kind, a, b, amount] : ops) {
      const OwnerId x = who[a % 8], y = who[b % 8];
      switch (kind % 3) {
        case 0: f.money.purchase(f.l, tick, x, y, f.goods, f.grain, amount % 5'000, 1 + amount % 3, tick); break;
        case 1: f.money.pay(f.l, tick, x, y, amount, FlowKind::Wage, tick); break;
        default: f.money.pay(f.l, tick, x, y, amount, FlowKind::Tax, tick); break;
      }
      ++tick;
      RC_ASSERT(f.l.total(f.money.resource()) == total);
      RC_ASSERT(f.l.total(f.goods.def(f.grain).unit) == sacks);
      RC_ASSERT(!f.l.check().has_value());
    }
    // The tallies are sums of what moved: for every owner, received - paid = balance change.
    for (OwnerId o : who) {
      const std::int64_t start = o == f.country ? 1'000'000 : o == f.farm ? 50'000 : o == f.shop ? 20'000 : o == f.ana ? 3'000 : 0;
      RC_ASSERT(f.l.balance(o, f.money.resource()) == start + f.money.received(o) - f.money.paid(o));
    }
  });
}

TEST_CASE("the same operations in the same order give the same balances (A1, determinism)") {
  auto run = [] {
    Fixture f;
    Spoilage s;
    s.reset(f.l, f.goods, f.country, f.water, 0);
    f.goods.materialise(f.l, 1, f.farm, f.grain, 10, 1);
    f.money.pay(f.l, 1, f.farm, f.bo, 4'000, FlowKind::Wage, 1);
    f.money.purchase(f.l, 2, f.bo, f.farm, f.goods, f.grain, 1'250, 1, 2);
    f.money.pay(f.l, 3, f.bo, f.region1, 200, FlowKind::Tax, 3);
    f.money.pay(f.l, 3, f.region1, f.country, 150, FlowKind::Tax, 3);
    s.advance(f.l, f.goods, f.decayed, 2 * kTicksPerGameDay, 4);
    return f.l.balances();
  };
  CHECK(run() == run());
}

TEST_CASE("0051: Individual shows the pop; City the selection or else the region; Macro the country") {
  Fixture f;
  Context ctx{f.ana, f.region2, f.country, std::nullopt};
  CHECK(shown_owner(Layer::Individual, f.owners, ctx) == f.ana);
  CHECK(shown_owner(Layer::Macro, f.owners, ctx) == f.country);
  CHECK(shown_owner(Layer::City, f.owners, ctx) == f.region2);
  ctx.selected = f.house;
  CHECK(shown_owner(Layer::City, f.owners, ctx) == f.house);
  CHECK(shown_owner(Layer::Individual, f.owners, ctx) == f.ana);  // a selection changes nothing elsewhere
  CHECK(shown_owner(Layer::Macro, f.owners, ctx) == f.country);
  ctx.selected = f.bo;
  CHECK(shown_owner(Layer::City, f.owners, ctx) == f.bo);
  ctx.selected = f.region1;  // a region or a country is not a City selection
  CHECK(shown_owner(Layer::City, f.owners, ctx) == f.region2);
  ctx.selected = f.country;
  CHECK(shown_owner(Layer::City, f.owners, ctx) == f.region2);
}

TEST_CASE("0051: a building's panel never includes the region's stock, and Macro sums what the regions hold") {
  Fixture f;
  const ResourceId gb = f.goods.def(f.grain).bulk;
  Context ctx{f.bo, f.region1, f.country, f.shop};
  const Panel shop = panel(Layer::City, f.l, f.owners, f.goods, f.money, ctx);
  CHECK(shop.owner == f.shop);
  CHECK(shop.scope == Scope::Building);
  CHECK(!shop.aggregated);
  CHECK(shop.bulk[f.grain] == 100'000);
  CHECK(shop.money == 20'000);
  ctx.selected.reset();
  const Panel region = panel(Layer::City, f.l, f.owners, f.goods, f.money, ctx);
  CHECK(region.owner == f.region1);
  CHECK(region.bulk[f.grain] == 7);  // only what lies loose in the region's own account, not its buildings'
  CHECK(region.money == 0);
  const Panel pop = panel(Layer::Individual, f.l, f.owners, f.goods, f.money, ctx);
  CHECK(pop.owner == f.bo);
  CHECK(pop.scope == Scope::Pop);
  CHECK(pop.money == 0);
  const Panel country = panel(Layer::Macro, f.l, f.owners, f.goods, f.money, ctx);
  CHECK(country.aggregated);
  CHECK(country.bulk[f.grain] == f.owners.aggregate(f.l, f.country, gb));
  CHECK(country.money == 1'000'000 + 50'000 + 20'000 + 3'000);
  std::int64_t by_part = 0;
  for (const auto& r : country.money_by_part) by_part += r.amount;
  CHECK(by_part == country.money);
  CHECK(country.money_by_part.size() == 3);
}
