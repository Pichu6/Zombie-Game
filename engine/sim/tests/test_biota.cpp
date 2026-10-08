// Spec 37: stocks that grow logistically with an exact jump, individuals from the stock on approach, kills that
// leave the others as they were, Macro readouts; and inside a world: the take command, save and replay.
#include <doctest.h>
#include "property.h"

#include <cmath>
#include <memory>

#include "zs/clock.h"
#include "zs/economy_system.h"

using namespace zs;

namespace {

// A forest cell of oaks (one tree 2,000 kg; K = 400 t; r = 0.03 a day, a placeholder for a fast test) and a herd
// of deer (one deer 60 kg; K = 6 t; r = 0.01 a day).
struct Fixture {
  Ledger l;
  Owners owners;
  Goods goods;
  Populations pops;
  OwnerId country, region, cell, hunter;
  GoodId oak, deer;
  std::size_t forest, herd;

  explicit Fixture(std::int64_t oak_kg = 100'000, std::int64_t deer_kg = 3'000) {
    country = owners.add(l, "country", Scope::Country);
    region = owners.add(l, "region", Scope::Region, country);
    cell = owners.add(l, "cell", Scope::Building, region);
    hunter = owners.add(l, "hunter", Scope::Pop, cell);
    oak = goods.add(l, "oak", "oak_kg", "oak_tree", 2'000);
    deer = goods.add(l, "deer", "deer_kg", "deer_head", 60);
    l.mint(cell, goods.def(oak).bulk, oak_kg);
    l.mint(cell, goods.def(deer).bulk, deer_kg);
    l.close_setup();
    forest = pops.add({cell, oak, 400'000, 0.03});
    herd = pops.add({cell, deer, 6'000, 0.01});
    pops.reset(l, goods, forest, 0);
    pops.reset(l, goods, herd, 0);
  }
  std::int64_t stock(std::size_t i) const { return l.balance(pops.at(i).owner, goods.def(pops.at(i).species).bulk); }
};

}  // namespace

TEST_CASE("logistic growth: stepping every hour equals one jump; the stock climbs toward K and never passes it (A1)") {
  Fixture hot, cold;
  const std::int64_t span = 200 * kTicksPerGameDay;
  for (std::int64_t t = kTicksPerGameHour; t <= span; t += kTicksPerGameHour) hot.pops.advance(hot.l, hot.goods, t, 1);
  cold.pops.advance(cold.l, cold.goods, span, 1);
  CHECK(hot.l.balances() == cold.l.balances());
  CHECK(hot.stock(hot.forest) > 100'000);
  CHECK(hot.stock(hot.forest) <= 400'000);
  // 200 days at r = 0.03 from a quarter of K: the closed form says K / (1 + 3 e^-6), about 99.3 % of K.
  CHECK(hot.stock(hot.forest) == doctest::Approx(400'000 / (1 + 3 * std::exp(-6.0))).epsilon(1e-4));
  CHECK(!hot.l.check().has_value());
  // Above K the stock declines to K; from nothing, nothing grows.
  CHECK(Populations::grown(500'000, 400'000, 0.03, 100 * kTicksPerGameDay) == doctest::Approx(400'000 / (1 - 0.2 * std::exp(-3.0))).epsilon(1e-4));
  CHECK(Populations::grown(0, 400'000, 0.03, 100 * kTicksPerGameDay) == 0);
  CHECK(Populations::grown(400'000, 400'000, 0.03, 100 * kTicksPerGameDay) == 400'000);
}

TEST_CASE("logistic growth: any partition agrees with one jump, with harvests between (A1)") {
  property("partition", [](std::vector<std::uint16_t> steps) {
    Fixture a, b;
    std::int64_t t = 0;
    for (std::uint16_t s : steps) {
      t += static_cast<std::int64_t>(s) * 11 + 1;
      a.pops.advance(a.l, a.goods, t, 1);
    }
    b.pops.advance(b.l, b.goods, t, 1);
    RC_ASSERT(a.l.balances() == b.l.balances());
    RC_ASSERT(!a.l.check().has_value());
  });
}

TEST_CASE("harvest at the sustainable yield holds the stock near K/2; past it the stock is depleted (A3)") {
  Fixture f(200'000, 3'000);  // the forest at K/2
  const double yield = Populations::sustainable_yield_per_day(f.pops.at(f.forest));
  CHECK(yield == doctest::Approx(3'000.0));
  for (int day = 1; day <= 365; ++day) {
    f.pops.advance(f.l, f.goods, day * kTicksPerGameDay, 1);
    f.l.transfer(day * kTicksPerGameDay, f.cell, f.hunter, f.goods.def(f.oak).bulk, 3'000, 2);
    f.pops.reset(f.l, f.goods, f.forest, day * kTicksPerGameDay);
  }
  CHECK(f.stock(f.forest) > 180'000);
  CHECK(f.stock(f.forest) < 220'000);
  CHECK(!f.pops.depleted(f.l, f.goods, f.forest));
  // Twice the yield: the stock falls and the flag rises within a few years.
  for (int day = 366; day <= 4 * 365; ++day) {
    f.pops.advance(f.l, f.goods, day * kTicksPerGameDay, 1);
    f.l.transfer(day * kTicksPerGameDay, f.cell, f.hunter, f.goods.def(f.oak).bulk, 6'000, 2);
    f.pops.reset(f.l, f.goods, f.forest, day * kTicksPerGameDay);
  }
  CHECK(f.pops.depleted(f.l, f.goods, f.forest));
  CHECK(!f.l.check().has_value());
}

TEST_CASE("approach: the same stock gives the same individuals in any order; a kill removes one and leaves the others (A1)") {
  Fixture f;
  const auto herd1 = f.pops.roster(7, f.l, f.goods, f.herd);
  const auto forest1 = f.pops.roster(7, f.l, f.goods, f.forest);
  const auto herd2 = f.pops.roster(7, f.l, f.goods, f.herd);
  REQUIRE(herd1.size() == 50);   // 3,000 kg / 60
  REQUIRE(forest1.size() == 50);  // 100,000 kg / 2,000
  CHECK(herd1.size() == herd2.size());
  for (std::size_t i = 0; i < herd1.size(); ++i) {
    CHECK(herd1[i].serial == herd2[i].serial);
    CHECK(herd1[i].mass_share == herd2[i].mass_share);
    CHECK(herd1[i].offset_x == herd2[i].offset_x);
  }
  CHECK(herd1[3].mass_share != forest1[3].mass_share);  // another species, other draws
  bool any_female = false, any_male = false;
  for (const auto& d : herd1) { any_female |= d.female; any_male |= !d.female; }
  CHECK(any_female);
  CHECK(any_male);
  // The hunter kills deer 17: its mass goes to the hunter, the herd is 49, and every other deer is the same.
  const std::int64_t moved = f.pops.take(f.l, f.goods, 10, f.herd, 17, f.hunter, 3, 7);
  CHECK(moved > 0);
  CHECK(moved == f.l.balance(f.hunter, f.goods.def(f.deer).bulk));
  CHECK(f.pops.take(f.l, f.goods, 11, f.herd, 17, f.hunter, 3, 7) == -1);   // already taken
  CHECK(f.pops.take(f.l, f.goods, 11, f.herd, 999, f.hunter, 3, 7) == -1);  // not on the roster
  // The roster is the stock in whole units: deer 17 weighed its keyed share of a unit, so the herd is 49 or 48 head
  // (spec 37 open point 2); whichever it is, every deer still on it is the same deer as before.
  const auto herd3 = f.pops.roster(7, f.l, f.goods, f.herd);
  const auto expected = static_cast<std::size_t>((3'000 - moved) / 60);
  REQUIRE(herd3.size() == expected);
  CHECK(expected >= 48);
  std::size_t j = 0;
  for (std::size_t i = 0; i < herd1.size() && j < herd3.size(); ++i) {
    if (herd1[i].serial == 17) continue;
    CHECK(herd3[j].serial == herd1[i].serial);
    CHECK(herd3[j].mass_share == herd1[i].mass_share);
    ++j;
  }
  // Growth after the kill appends new serials at the end; the ones shown stay.
  f.pops.advance(f.l, f.goods, 400 * kTicksPerGameDay, 4);
  const auto herd4 = f.pops.roster(7, f.l, f.goods, f.herd);
  REQUIRE(herd4.size() > herd3.size());
  for (std::size_t i = 0; i < herd3.size(); ++i) CHECK(herd4[i].serial == herd3[i].serial);
  CHECK(herd4[herd3.size()].serial == herd3.back().serial + 1);
  CHECK(!f.l.check().has_value());
}

TEST_CASE("the roster is a pure function: leaving and coming back shows the same forest (A1)") {
  property("round trip", [](std::uint32_t seed, std::uint16_t kills_raw) {
    Fixture f;
    const auto first = f.pops.approach(seed, f.l, f.goods, f.forest);
    const std::size_t kills = kills_raw % 10;
    for (std::size_t k = 0; k < kills; ++k) f.pops.take(f.l, f.goods, 1 + static_cast<std::int64_t>(k), f.forest, first[k * 3].serial, f.hunter, 1, seed);
    const auto again = f.pops.approach(seed, f.l, f.goods, f.forest);
    // Each cut removes one tree and its keyed mass (half to one and a half units), so the count in whole units falls
    // by between half and two per cut (a light tree cut can let a sapling's worth of bulk show as a new tree at the end).
    RC_ASSERT(again.size() <= first.size() - kills / 2);
    RC_ASSERT(again.size() + 2 * kills >= first.size());
    std::size_t j = 0;
    for (const auto& tree : first) {
      if (f.pops.at(f.forest).taken.contains(tree.serial)) continue;
      if (j >= again.size()) break;
      RC_ASSERT(again[j].serial == tree.serial);
      RC_ASSERT(again[j].age_share == tree.age_share);
      ++j;
    }
    RC_ASSERT(!f.l.check().has_value());
  });
}

// ---- In a world.

namespace {
struct Ids {
  OwnerId country, region, cell, hunter;
  GoodId deer;
  std::size_t herd;
};
Ids ids;

std::unique_ptr<World> make_world(std::uint64_t seed) {
  auto w = std::make_unique<World>(seed);
  auto econ = std::make_unique<EconomySystem>(*w);
  Owners& o = econ->owners();
  Goods& g = econ->goods();
  ids.country = o.add(w->ledger, "country", Scope::Country);
  ids.region = o.add(w->ledger, "region", Scope::Region, ids.country);
  ids.cell = o.add(w->ledger, "cell", Scope::Building, ids.region);
  ids.hunter = o.add(w->ledger, "hunter", Scope::Pop, ids.cell);
  ids.deer = g.add(w->ledger, "deer", "deer_kg", "deer_head", 60);
  w->ledger.mint(ids.cell, g.def(ids.deer).bulk, 1'200);
  ids.herd = econ->populations().add({ids.cell, ids.deer, 6'000, 0.02});
  econ->start_lots(*w);
  econ->start_establishments(*w);
  econ->start_populations(*w);
  w->add_system(std::move(econ));
  w->close_setup();
  w->check_every_tick = true;
  return w;
}

void script(World& w) {
  const std::int64_t t = w.clock.tick, day = kTicksPerGameDay;
  if (t == 3 * day) w.submit({0, BiotaTake, ids.hunter, static_cast<std::int64_t>(ids.herd), 4});
  if (t == 3 * day + 1) w.submit({0, BiotaTake, ids.hunter, static_cast<std::int64_t>(ids.herd), 4});   // refused: taken
  if (t == 5 * day) w.submit({0, BiotaTake, ids.hunter, static_cast<std::int64_t>(ids.herd), 12});
  if (t == 6 * day) w.submit({0, BiotaTake, ids.hunter, 9, 0});                                        // refused: no such population
}

std::unique_ptr<World> run(std::uint64_t seed, std::int64_t ticks) {
  auto w = make_world(seed);
  for (std::int64_t i = 0; i < ticks; ++i) { script(*w); w->step(); }
  return w;
}
}  // namespace

TEST_CASE("biota in a world: the herd grows by the hour, kills move mass to the hunter with reasons; save and replay (A1)") {
  const std::int64_t ticks = 10 * kTicksPerGameDay;
  auto w = run(3, ticks);
  const auto* econ = w->system<EconomySystem>();
  REQUIRE(econ);
  const ResourceId deer = econ->goods().def(ids.deer).bulk;
  CHECK(w->ledger.balance(ids.cell, deer) > 1'200 - 2 * 90);  // grew more than the two kills took
  CHECK(w->ledger.balance(ids.hunter, deer) > 0);
  CHECK(econ->populations().at(ids.herd).taken.size() == 2);
  int taken = 0, refused = 0;
  for (const Event& e : w->log.all()) {
    if (e.type == EventType::BiotaTaken) ++taken;
    if (e.type == EventType::EconRefused) ++refused;
  }
  CHECK(taken == 2);
  CHECK(refused == 2);
  CHECK(!w->ledger.check().has_value());
  const auto bytes = w->save();
  auto candidate = make_world(3);
  candidate->load(bytes);
  CHECK(candidate->save() == bytes);
  CHECK(candidate->state_hash() == w->state_hash());
  auto r = make_world(3);
  std::size_t next = 0;
  const auto& inputs = w->input_log();
  for (std::int64_t i = 0; i < ticks; ++i) {
    while (next < inputs.size() && inputs[next].tick == r->clock.tick) r->submit(inputs[next++]);
    r->step();
  }
  CHECK(r->state_hash() == w->state_hash());
  const std::int64_t half = 4 * kTicksPerGameDay + 9;
  auto first = run(3, half);
  auto resumed = make_world(3);
  resumed->load(first->save());
  for (std::int64_t i = half; i < ticks; ++i) { script(*resumed); resumed->step(); }
  CHECK(resumed->state_hash() == w->state_hash());
}
