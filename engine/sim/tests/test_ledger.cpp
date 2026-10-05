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
