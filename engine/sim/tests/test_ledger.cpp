// Spec 03: stocks, transfers, conversions and locations.
#include <doctest.h>
#include "property.h"

#include "zs/ledger.h"

using namespace zs;

TEST_CASE("random transfers and conversions conserve totals and never go negative (A1)") {
  property("property", [](const std::vector<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::int32_t>>& ops) {
    Ledger l;
    const ResourceId fuel = l.add_resource("fuel");
    const ResourceId energy = l.add_resource("energy");
    for (int i = 0; i < 4; ++i) l.add_owner("owner" + std::to_string(i));
    const ConversionId burn = l.add_conversion({fuel, energy, 3, 20});
    l.mint(0, fuel, 1000);
    l.mint(1, fuel, 500);
    l.close_setup();
    std::int64_t tick = 0;
    for (const auto& [kind, from, to, amount] : ops) {
      if (kind % 2 == 0) l.transfer(tick++, from % 4, to % 4, fuel, amount, 0);
      else l.convert(tick++, from % 4, to % 4, burn, amount, 0);
      RC_ASSERT(!l.check().has_value());
    }
  });
}

TEST_CASE("transfers clamp to what the owner holds and report what moved") {
  Ledger l;
  const ResourceId r = l.add_resource("nails");
  l.add_owner("a");
  l.add_owner("b");
  l.mint(0, r, 10);
  l.close_setup();
  CHECK(l.transfer(1, 0, 1, r, 25, 0) == 10);
  CHECK(l.balance(0, r) == 0);
  CHECK(l.balance(1, r) == 10);
  CHECK(l.transfer(2, 0, 1, r, 5, 0) == 0);
  CHECK_THROWS(l.mint(0, r, 1));  // nothing appears after setup
}

TEST_CASE("conversions only move whole batches") {
  Ledger l;
  const ResourceId g = l.add_resource("glycogen_g");
  const ResourceId kj = l.add_resource("spent_kj");
  l.add_owner("body");
  l.add_owner("world.metabolised");
  const ConversionId c = l.add_conversion({g, kj, 1, 17});
  l.mint(0, g, 5);
  l.close_setup();
  CHECK(l.convert(1, 0, 1, c, 9, 0) == 5);
  CHECK(l.balance(1, kj) == 85);
  CHECK(!l.check());
}

TEST_CASE("locations form a tree: a move that would create a cycle is refused (A1)") {
  property("property", [](const std::vector<std::pair<std::uint8_t, std::uint8_t>>& moves) {
    Locations loc;
    for (const auto& [e, p] : moves) {
      loc.place(e % 16 + 1, p % 17);
      for (EntityId x = 1; x <= 16; ++x) {  // walking up from any entity reaches the world
        EntityId cur = x;
        int steps = 0;
        while (cur != Locations::kWorld && steps < 32) { cur = loc.parent_of(cur); ++steps; }
        RC_ASSERT(cur == Locations::kWorld);
      }
    }
  });
}

TEST_CASE("declared flows enter and leave after setup, keep the check intact and carry their cause (A1)") {
  Ledger l;
  const ResourceId mass = l.add_resource("mass");
  const OwnerId a = l.add_owner("a"), b = l.add_owner("b");
  l.mint(a, mass, 100);
  l.close_setup();
  CHECK_THROWS(l.mint(a, mass, 1));                  // a silent mint is still refused after setup
  l.declare_inflow(5, b, mass, 40, 77);
  CHECK(l.balance(b, mass) == 40);
  CHECK(l.total(mass) == 140);
  CHECK_FALSE(l.check().has_value());                // the expected total moved by exactly the inflow
  CHECK(l.tail().back().kind == EntryKind::Inflow);
  CHECK(l.tail().back().cause == 77);
  CHECK(l.tail().back().tick == 5);
  l.transfer(6, b, a, mass, 15, 78);
  l.declare_outflow(7, b, mass, 25, 79);
  CHECK(l.balance(b, mass) == 0);
  CHECK(l.total(mass) == 115);
  CHECK_FALSE(l.check().has_value());
  CHECK(l.tail().back().kind == EntryKind::Outflow);
  CHECK_THROWS(l.declare_outflow(8, b, mass, 1, 0));  // an outflow takes exactly what is named, never more than held
  CHECK_THROWS(l.declare_inflow(8, b, mass, -1, 0));
  CHECK_THROWS(l.declare_outflow(8, a, mass, -1, 0));
  l.declare_inflow(9, a, mass, 0, 0);                 // nothing is nothing
  CHECK(l.total(mass) == 115);
}

TEST_CASE("random declared flows and transfers keep the ledger consistent (A1)") {
  property("property", [](const std::vector<std::tuple<std::uint8_t, std::uint8_t, std::uint16_t>>& ops) {
    Ledger l;
    const ResourceId r = l.add_resource("tissue");
    for (int i = 0; i < 3; ++i) l.add_owner("o" + std::to_string(i));
    l.mint(0, r, 500);
    l.close_setup();
    std::int64_t tick = 0;
    for (const auto& [kind, owner, amount] : ops) {
      const OwnerId o = owner % 3;
      if (kind % 3 == 0) l.declare_inflow(tick++, o, r, amount, 1);
      else if (kind % 3 == 1) { const std::int64_t held = l.balance(o, r); if (held > 0) l.declare_outflow(tick++, o, r, std::min<std::int64_t>(amount, held), 2); }
      else l.transfer(tick++, o, (o + 1) % 3, r, amount, 3);
      RC_ASSERT(!l.check().has_value());
    }
  });
}
