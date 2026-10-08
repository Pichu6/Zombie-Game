// Spec 33: recipes, establishments, the production jump, chains and Macro sums, on the ledger alone.
#include <doctest.h>
#include "property.h"

#include <vector>

#include "zs/clock.h"
#include "zs/establishment.h"

using namespace zs;

namespace {

// Grain -> flour at the mill (100 kg -> 95 kg, plus 1 kWh of "fuel" consumed), flour -> bread at the bakery
// (10 kg flour + 6 L water -> 14 kg bread). Labour: a batch of flour is 2 worker-hours, a batch of bread 1.
struct Fixture {
  Ledger l;
  Owners owners;
  Goods goods;
  Recipes recipes;
  Establishments estabs;
  OwnerId consumed, country, region, farm, mill, mill2, bakery, pop;
  GoodId grain, flour, fuel, water, bread;
  RecipeId milling, baking;

  Fixture() {
    consumed = owners.add(l, "world.consumed", Scope::Sink);
    country = owners.add(l, "country", Scope::Country);
    region = owners.add(l, "region", Scope::Region, country);
    farm = owners.add(l, "farm", Scope::Building, region);
    mill = owners.add(l, "mill", Scope::Building, region);
    mill2 = owners.add(l, "mill2", Scope::Building, region);
    bakery = owners.add(l, "bakery", Scope::Building, region);
    pop = owners.add(l, "pop", Scope::Pop, bakery);
    grain = goods.add(l, "grain", "grain_g", "grain_sack", 50'000);
    flour = goods.add(l, "flour", "flour_g", "flour_bag", 10'000);
    fuel = goods.add(l, "fuel", "fuel_wh", "fuel_can", 1'000);
    water = goods.add(l, "water", "water_ml", "water_can", 1'000);
    bread = goods.add(l, "bread", "bread_g", "bread_loaf", 500);
    milling = recipes.add(l, goods, {"milling", grain, 100'000, flour, 95'000, {{fuel, 1'000}}, 2 * kTicksPerGameHour, 5'000});
    baking = recipes.add(l, goods, {"baking", flour, 10'000, bread, 14'000, {{water, 6'000}}, 1 * kTicksPerGameHour, 0});
  }
  void stock_mill(OwnerId m, std::int64_t grain_g, std::int64_t fuel_wh) {
    l.mint(m, goods.def(grain).bulk, grain_g);
    l.mint(m, goods.def(fuel).bulk, fuel_wh);
  }
};

}  // namespace

TEST_CASE("a recipe runs whole batches on one account; the ledger stays clean; by hand is the same recipe (A1)") {
  Fixture f;
  f.stock_mill(f.mill, 1'000'000, 20'000);
  f.l.mint(f.pop, f.goods.def(f.grain).bulk, 250'000);
  f.l.mint(f.pop, f.goods.def(f.fuel).bulk, 2'000);
  f.l.close_setup();
  CHECK(f.recipes.affordable(f.l, f.goods, f.mill, f.milling) == 10);
  CHECK(f.recipes.run(f.l, f.goods, 1, f.mill, f.milling, 3, f.consumed, 1) == 3);
  CHECK(f.l.balance(f.mill, f.goods.def(f.grain).bulk) == 700'000);
  CHECK(f.l.balance(f.mill, f.goods.def(f.flour).bulk) == 285'000);
  CHECK(f.l.balance(f.mill, f.goods.def(f.fuel).bulk) == 17'000);
  CHECK(f.l.balance(f.consumed, f.goods.def(f.fuel).bulk) == 3'000);
  // The pop by hand: the fuel caps it at 2 batches although the grain allows 2 as well.
  CHECK(f.recipes.run(f.l, f.goods, 2, f.pop, f.milling, 10, f.consumed, 2) == 2);
  CHECK(f.l.balance(f.pop, f.goods.def(f.flour).bulk) == 190'000);
  CHECK(f.l.balance(f.pop, f.goods.def(f.grain).bulk) == 50'000);
  CHECK(f.recipes.run(f.l, f.goods, 3, f.pop, f.milling, 10, f.consumed, 3) == 0);
  CHECK(!f.l.check().has_value());
}

TEST_CASE("production jump: stepping every tick equals one jump, with inputs running out midway (A1)") {
  auto build = [](Fixture& f) {
    f.stock_mill(f.mill, 450'000, 100'000);  // 4 batches of grain, fuel for 100
    f.l.close_setup();
    f.estabs.add({f.mill, f.milling, 0, 3, 5'000});  // 3 workers, powered: a batch every 40 minutes
  };
  Fixture hot, cold;
  build(hot);
  build(cold);
  const std::int64_t span = 6 * kTicksPerGameHour;  // 9 batches' worth of labour, 4 batches of grain
  for (std::int64_t t = 1; t <= span; ++t) hot.estabs.advance(hot.l, hot.goods, hot.recipes, t, hot.consumed, 1);
  cold.estabs.advance(cold.l, cold.goods, cold.recipes, span, cold.consumed, 1);
  CHECK(hot.l.balances() == cold.l.balances());
  CHECK(hot.estabs.at(0).labour_bank == cold.estabs.at(0).labour_bank);
  CHECK(hot.estabs.at(0).produced == 4);
  CHECK(cold.estabs.at(0).produced == 4);
  CHECK(hot.l.balance(hot.mill, hot.goods.def(hot.grain).bulk) == 50'000);
  CHECK(hot.l.balance(hot.mill, hot.goods.def(hot.flour).bulk) == 380'000);
  CHECK(!hot.l.check().has_value());
}

TEST_CASE("production jump: any partition agrees with one jump, for any staff (A1)") {
  property("partition independence", [](std::vector<std::uint16_t> steps, std::uint8_t staff) {
    Fixture a, b;
    for (Fixture* f : {&a, &b}) {
      f->stock_mill(f->mill, 2'000'000, 1'000'000);
      f->l.close_setup();
      f->estabs.add({f->mill, f->milling, 0, static_cast<std::int64_t>(staff), 5'000});
    }
    std::int64_t t = 0;
    for (std::uint16_t s : steps) {
      t += static_cast<std::int64_t>(s) + 1;
      a.estabs.advance(a.l, a.goods, a.recipes, t, a.consumed, 1);
    }
    b.estabs.advance(b.l, b.goods, b.recipes, t, b.consumed, 1);
    RC_ASSERT(a.l.balances() == b.l.balances());
    RC_ASSERT(a.estabs.at(0).labour_bank == b.estabs.at(0).labour_bank);
    RC_ASSERT(a.estabs.at(0).produced == b.estabs.at(0).produced);
    RC_ASSERT(!a.l.check().has_value());
  });
}

TEST_CASE("no power, no staff, or no inputs: nothing is produced and idle labour is not banked") {
  Fixture f;
  f.stock_mill(f.mill, 1'000'000, 100'000);
  f.l.close_setup();
  const std::size_t i = f.estabs.add({f.mill, f.milling, 0, 4, 4'999});  // under the 5 kW the recipe needs
  f.estabs.advance(f.l, f.goods, f.recipes, kTicksPerGameDay, f.consumed, 1);
  CHECK(f.estabs.at(i).produced == 0);
  CHECK(f.estabs.at(i).labour_bank == 0);
  f.estabs.at(i).power_w = 5'000;
  f.estabs.at(i).staff = 0;
  f.estabs.advance(f.l, f.goods, f.recipes, 2 * kTicksPerGameDay, f.consumed, 1);
  CHECK(f.estabs.at(i).produced == 0);
  f.estabs.at(i).staff = 4;  // a batch every 30 minutes; grain for 10 batches
  f.estabs.advance(f.l, f.goods, f.recipes, 3 * kTicksPerGameDay, f.consumed, 1);
  CHECK(f.estabs.at(i).produced == 10);
  CHECK(f.estabs.at(i).labour_bank < 2 * kTicksPerGameHour);  // no stored hours for the day with nothing to mill
  CHECK(!f.l.check().has_value());
}

TEST_CASE("Macro: a region's output is the sum over its establishments; a chain is bounded by its slowest stage (A1)") {
  Fixture f;
  f.stock_mill(f.mill, 1'000'000, 100'000);
  f.stock_mill(f.mill2, 1'000'000, 100'000);
  f.l.mint(f.bakery, f.goods.def(f.water).bulk, 1'000'000);
  f.l.close_setup();
  f.estabs.add({f.mill, f.milling, 0, 2, 5'000});    // a batch an hour
  f.estabs.add({f.mill2, f.milling, 0, 1, 5'000});   // a batch every two hours
  f.estabs.add({f.bakery, f.baking, 0, 10, 0});      // ten batches an hour, if it has flour
  const std::int64_t hour = kTicksPerGameHour;
  for (std::int64_t h = 1; h <= 8; ++h) {
    f.estabs.advance(f.l, f.goods, f.recipes, h * hour, f.consumed, 1);
    // The chain: each hour the mills ship their flour to the bakery.
    for (OwnerId m : {f.mill, f.mill2})
      f.l.transfer(h * hour, m, f.bakery, f.goods.def(f.flour).bulk, f.l.balance(m, f.goods.def(f.flour).bulk), 2);
  }
  CHECK(f.estabs.produced_under(f.owners, f.region, f.milling) == 8 + 4);
  CHECK(f.estabs.produced_under(f.owners, f.country) == f.estabs.at(0).produced + f.estabs.at(1).produced + f.estabs.at(2).produced);
  CHECK(f.estabs.produced_under(f.owners, f.farm) == 0);
  // The mills make 12 batches of flour (1,140 kg) but the bakery, at 10 batches an hour, is the slowest stage: it
  // has flour from hour 2 on, so it bakes about 7 x 10 batches and flour piles up behind it.
  const std::int64_t flour_made = 12 * 95'000;
  const std::int64_t bread_batches = f.estabs.at(2).produced;
  CHECK(bread_batches * 10'000 <= flour_made);
  CHECK(bread_batches >= 7 * 10 - 1);
  CHECK(bread_batches <= 7 * 10 + 1);
  CHECK(f.l.balance(f.bakery, f.goods.def(f.flour).bulk) > 0);  // the chain is bound by the bakery, not by flour
  CHECK(f.l.balance(f.bakery, f.goods.def(f.bread).bulk) == bread_batches * 14'000);
  CHECK(!f.l.check().has_value());
}
