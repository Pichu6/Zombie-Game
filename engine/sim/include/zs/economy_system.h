// Stocks, goods and money as a world system: commands for visits, purchases, payments and shipments, spoilage
// advanced by the hour, lots and tallies saved with the world. Spec: docs/tier0/specs/32-stocks-goods-and-money.md
#pragma once
#include <cstdint>

#include "zs/economy.h"
#include "zs/world.h"

namespace zs {

// Command types for the economy. A high range of its own, apart from the pop commands (0x1000).
enum EconCommand : std::uint16_t {
  EconVisit = 0x2001,     // subject = owner, a = good, b = max units: bulk -> units
  EconLeave = 0x2002,     // subject = owner, a = good: units -> bulk
  EconPurchase = 0x2003,  // subject = buyer, a = seller, b = econ_purchase_arg(good, want, price)
  EconPay = 0x2004,       // subject = payer, a = receiver, b = econ_pay_arg(kind, cents)
  EconShip = 0x2005,      // subject = from, a = to, b = econ_ship_arg(good, bulk amount): a transfer of bulk between owners
};

// Packing of the `b` field. A purchase: good in the low 16 bits, units wanted in the next 16, price in cents in the
// upper 32. A payment: kind in the top byte, cents below. A shipment: good in the low 16 bits, amount above.
inline std::int64_t econ_purchase_arg(GoodId good, std::uint16_t want, std::uint32_t price_cents) {
  return static_cast<std::int64_t>(good) | (static_cast<std::int64_t>(want) << 16) |
         (static_cast<std::int64_t>(price_cents) << 32);
}
inline std::int64_t econ_pay_arg(FlowKind kind, std::int64_t cents) {
  return (static_cast<std::int64_t>(kind) << 56) | (cents & ((std::int64_t{1} << 56) - 1));
}
inline std::int64_t econ_ship_arg(GoodId good, std::int64_t bulk_amount) {
  return static_cast<std::int64_t>(good) | (bulk_amount << 16);
}

// Why an economy command was refused: carried in the EconRefused event's `b`.
enum class EconRefusal : std::int64_t {
  None = 0,
  UnknownOwner = 1,  // not in the owner tree (a body slot, a typo, a sink as buyer)
  UnknownGood = 2,
  Nothing = 3,       // the command would move nothing: no stock, no money, zero amount, same owner both sides
};

class EconomySystem : public System {
 public:
  explicit EconomySystem(World& w);
  const char* name() const override { return "economy"; }

  // Setup access: the owner tree, the goods and the money live here; add owners and goods while the world's setup
  // is open, mint through the ledger. The same setup must make the same calls (definitions are not saved).
  Owners& owners() { return owners_; }
  Goods& goods() { return goods_; }
  const Owners& owners() const { return owners_; }
  const Goods& goods() const { return goods_; }
  const Money& money() const { return money_; }
  const Spoilage& spoilage() const { return spoilage_; }
  OwnerId decay_sink() const { return decayed_; }

  // Every owner's bulk of every spoiling good starts decaying now. Call once at the end of setup, after minting.
  void start_lots(const World& w);

  // Decision 0051: the panel for a layer and a context, read from this world.
  Panel panel(const World& w, Layer layer, const Context& ctx) const;

  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

 private:
  void refuse(World& w, const Command& c, EventId cause, EconRefusal why);
  bool known_good(std::int64_t g) const { return g >= 0 && static_cast<std::size_t>(g) < goods_.size(); }

  Owners owners_;
  Goods goods_;
  Money money_;
  Spoilage spoilage_;
  OwnerId decayed_;
};

}  // namespace zs
