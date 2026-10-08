// Stocks, goods and money as a world system: commands for visits, purchases, payments and shipments, spoilage
// advanced by the hour, lots and tallies saved with the world. Spec: docs/tier0/specs/32-stocks-goods-and-money.md
#pragma once
#include <cstdint>

#include "zs/biota.h"
#include "zs/economy.h"
#include "zs/establishment.h"
#include "zs/pop_system.h"
#include "zs/project.h"
#include "zs/world.h"

namespace zs {

// Command types for the economy. A high range of its own, apart from the pop commands (0x1000).
enum EconCommand : std::uint16_t {
  EconVisit = 0x2001,     // subject = owner, a = good, b = max units: bulk -> units
  EconLeave = 0x2002,     // subject = owner, a = good: units -> bulk
  EconPurchase = 0x2003,  // subject = buyer, a = seller, b = econ_purchase_arg(good, want, price)
  EconPay = 0x2004,       // subject = payer, a = receiver, b = econ_pay_arg(kind, cents)
  EconShip = 0x2005,      // subject = from, a = to, b = econ_ship_arg(good, bulk amount): a transfer of bulk between owners
  EstabSetStaff = 0x2006, // subject = establishment owner, a = workers (refused if its staff comes from a roster)
  EstabSetPower = 0x2007, // subject = establishment owner, a = watts available
  EconRunByHand = 0x2008, // subject = a pop's owner, a = recipe, b = max batches: the same recipe as a plant, labour supplied by the action
  ProjectOrder = 0x2009,  // subject = site owner, a = plan, b = orderer owner (pays the budget into the site's escrow)
  ProjectCancel = 0x200a, // subject = site owner: the escrow left goes back to the orderer; materials stay on site
  BiotaTake = 0x200b,     // subject = taker owner, a = population index, b = serial: a kill or a cut; the mass goes to the taker
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
  NoEstablishment = 4,
  UnknownRecipe = 5,
  StaffFromRoster = 6,  // the roster decides its staff; lay people off there
  UnknownPlan = 7,
  SiteBusy = 8,         // the site already has a project
  NoProject = 9,
  NoBudget = 10,        // the orderer cannot escrow the whole budget
  NoPopulation = 11,
  NotOnRoster = 12,     // that serial is taken already, or beyond the stock
};

class EconomySystem : public System {
 public:
  explicit EconomySystem(World& w);
  const char* name() const override { return "economy"; }

  // Setup access: the owner tree, the goods and the money live here; add owners and goods while the world's setup
  // is open, mint through the ledger. The same setup must make the same calls (definitions are not saved).
  Owners& owners() { return owners_; }
  Goods& goods() { return goods_; }
  Recipes& recipes() { return recipes_; }
  Establishments& establishments() { return estabs_; }
  Projects& projects() { return projects_; }
  Populations& populations() { return populations_; }
  const Owners& owners() const { return owners_; }
  const Recipes& recipes() const { return recipes_; }
  const Establishments& establishments() const { return estabs_; }
  const Projects& projects() const { return projects_; }
  const Populations& populations() const { return populations_; }
  const Goods& goods() const { return goods_; }
  const Money& money() const { return money_; }
  const Spoilage& spoilage() const { return spoilage_; }
  OwnerId decay_sink() const { return decayed_; }
  OwnerId consumed_sink() const { return consumed_; }

  // Spec 33 rule 3: an establishment with a roster takes its staff from the roster's present members who are not laid off
  // or ill, read from the pop system. Setup only, the same call in every setup (not saved).
  void attach_pops(const PopSystem& pops) { pops_ = &pops; }

  // Every owner's bulk of every spoiling good starts decaying now. Call once at the end of setup, after minting.
  void start_lots(const World& w);
  // Establishments start their clocks now. Call once at the end of setup, with start_lots.
  void start_establishments(const World& w);
  // Populations start their lots now. Call once at the end of setup, after minting.
  void start_populations(const World& w);

  // Decision 0051: the panel for a layer and a context, read from this world.
  Panel panel(const World& w, Layer layer, const Context& ctx) const;

  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

 private:
  void refuse(World& w, const Command& c, EventId cause, EconRefusal why);
  bool known_good(std::int64_t g) const { return g >= 0 && static_cast<std::size_t>(g) < goods_.size(); }
  void read_staff(const World& w);
  void catch_up(World& w, CauseId cause);

  Owners owners_;
  Goods goods_;
  Money money_;
  Spoilage spoilage_;
  Recipes recipes_;
  Establishments estabs_;
  Projects projects_;
  Populations populations_;
  const PopSystem* pops_ = nullptr;
  OwnerId decayed_;
  OwnerId consumed_;
};

}  // namespace zs
