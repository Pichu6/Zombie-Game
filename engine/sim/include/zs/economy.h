// Stocks, goods and money across levels: owners with a scope and a parent, goods that are bulk at a distance and
// units up close, spoilage with an exact jump form, money as one more conserved resource, and the rule for which
// owner's account a panel shows. Spec: docs/tier0/specs/32-stocks-goods-and-money.md. Decision 0051.
#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "zs/ledger.h"

namespace zs {

// Who holds things, by layer: the country (Macro), a region (City with nothing selected), a building (an
// establishment, a household, a warehouse) and a pop (its wallet and what it carries). Sinks hold what left play.
enum class Scope : std::uint8_t { Sink, Country, Region, Building, Pop };

inline constexpr OwnerId kNoOwner = ~OwnerId{0};

// The owner tree over the ledger's owners. Every owner that holds goods or money is registered here with its scope
// and parent; owners the body model adds for itself may stay out (they hold tissue, not goods).
class Owners {
 public:
  struct Info {
    std::string name;
    Scope scope;
    OwnerId parent;
  };
  struct Row {
    OwnerId owner;
    std::int64_t amount;
  };

  // Adds an owner to the ledger and to the tree. Roots and sinks pass kNoOwner as parent.
  OwnerId add(Ledger& l, std::string name, Scope scope, OwnerId parent = kNoOwner);

  bool known(OwnerId o) const { return info_.contains(o); }
  const Info& info(OwnerId o) const { return info_.at(o); }
  Scope scope(OwnerId o) const { return info_.at(o).scope; }
  OwnerId parent(OwnerId o) const { return info_.at(o).parent; }
  const std::vector<OwnerId>& children(OwnerId o) const;
  std::size_t size() const { return info_.size(); }
  // True if `o` is `root` or lies under it in the tree.
  bool under(OwnerId o, OwnerId root) const;
  // Every owner under `root` (not `root` itself), depth first in id order.
  std::vector<OwnerId> descendants(OwnerId root) const;

  // Enumeration (layer jumps §5 harness property 2): an owner's own account plus every descendant's, so a
  // region's stock is what its buildings and people hold plus what lies loose, and a country's is its regions'.
  std::int64_t aggregate(const Ledger& l, OwnerId o, ResourceId r) const;
  // The same split one level down: a row per direct child with that child's aggregate, then a row for the owner's
  // own account (owner = o). The rows sum to aggregate(o).
  std::vector<Row> by_children(const Ledger& l, OwnerId o, ResourceId r) const;

 private:
  std::map<OwnerId, Info> info_;
  std::map<OwnerId, std::vector<OwnerId>> children_;
};

// A good is one conserved bulk amount (grams, millilitres, a count) that a visit shows as units (sacks, cans, crates):
// a stock of units with no identity, as spec 15 rule 4 says, at a rate declared in data. Spoilage is a half-life.
using GoodId = std::uint16_t;

struct GoodDef {
  std::string name;
  ResourceId bulk;
  ResourceId unit;
  std::int64_t bulk_per_unit;
  ConversionId pack;    // bulk -> unit
  ConversionId unpack;  // unit -> bulk
  double half_life_days;  // 0 = does not spoil
};

class Goods {
 public:
  GoodId add(Ledger& l, std::string name, std::string bulk_resource, std::string unit_resource,
             std::int64_t bulk_per_unit, double half_life_days = 0);
  const GoodDef& def(GoodId g) const { return defs_.at(g); }
  std::size_t size() const { return defs_.size(); }

  // A visit: up to `max_units` whole units are made from the owner's bulk; the remainder stays bulk. Returns units made.
  std::int64_t materialise(Ledger& l, std::int64_t tick, OwnerId owner, GoodId g, std::int64_t max_units,
                           CauseId cause) const;
  // Leaving: every unit the owner holds goes back to bulk. Returns units returned.
  std::int64_t dematerialise(Ledger& l, std::int64_t tick, OwnerId owner, GoodId g, CauseId cause) const;

 private:
  std::vector<GoodDef> defs_;
};

// Spoilage as a rate with an exact jump form (spec 27 rule 2). A lot is an owner's bulk of one good from the tick
// it was last touched; what remains at any later tick is a closed form of (start amount, elapsed), so stepping every
// tick and one jump move exactly the same amount to the sink (A1). A transfer of the bulk must reset the lot.
class Spoilage {
 public:
  struct Lot {
    OwnerId owner;
    GoodId good;
    std::int64_t start_amount;
    std::int64_t start_tick;
    std::int64_t caught_up;
  };

  // ceil(start * 2^(-elapsed / half life)); start when the good does not spoil.
  static std::int64_t remaining(std::int64_t start_amount, double half_life_days, std::int64_t elapsed_ticks);

  // The owner's bulk of `g` as it is now starts (or restarts) decaying from `tick`.
  void reset(const Ledger& l, const Goods& goods, OwnerId owner, GoodId g, std::int64_t tick);
  // Jump form: moves what spoiled between each lot's caught-up tick and `to_tick` to `decayed`. Returns the total moved.
  std::int64_t advance(Ledger& l, const Goods& goods, OwnerId decayed, std::int64_t to_tick, CauseId cause);

  const std::vector<Lot>& lots() const { return lots_; }
  void restore(std::vector<Lot> lots) { lots_ = std::move(lots); }

 private:
  std::vector<Lot> lots_;
};

// Why money moved; aggregates (revenue, wages paid, tax taken) are sums of these, never a separate number.
enum class FlowKind : std::uint8_t { Sale, Wage, Tax, Grant, Other };

class Money {
 public:
  explicit Money(Ledger& l) : money_(l.add_resource("money_cents")) {}
  ResourceId resource() const { return money_; }

  // All or nothing per unit: n = min(want, the seller's units, the buyer's money / price). The goods and the money
  // move in the same tick with the same cause. Returns units bought.
  std::int64_t purchase(Ledger& l, std::int64_t tick, OwnerId buyer, OwnerId seller, const Goods& goods, GoodId g,
                        std::int64_t price_cents, std::int64_t want, CauseId cause);
  // A wage, a tax, a grant: a transfer clamped to what the payer holds. Returns what moved.
  std::int64_t pay(Ledger& l, std::int64_t tick, OwnerId from, OwnerId to, std::int64_t cents, FlowKind kind,
                   CauseId cause);

  std::int64_t received(OwnerId o, FlowKind k) const;
  std::int64_t paid(OwnerId o, FlowKind k) const;
  std::int64_t received(OwnerId o) const;  // all kinds
  std::int64_t paid(OwnerId o) const;

  // Persistence: the tallies (the balances themselves are the ledger's).
  using Tallies = std::map<std::pair<OwnerId, FlowKind>, std::int64_t>;
  const Tallies& received_tallies() const { return in_; }
  const Tallies& paid_tallies() const { return out_; }
  void restore(Tallies in, Tallies out) { in_ = std::move(in); out_ = std::move(out); }

 private:
  void tally(OwnerId from, OwnerId to, FlowKind k, std::int64_t cents);

  ResourceId money_;
  Tallies in_;
  Tallies out_;
};

// Decision 0051: whose money and resources a panel shows.
enum class Layer : std::uint8_t { Individual, City, Macro };

struct Context {
  OwnerId played_pop;               // Individual: always this one
  OwnerId region;                   // City with nothing selected
  OwnerId country;                  // Macro: always this one
  std::optional<OwnerId> selected;  // City: a pop or a building, if one is selected
};

// Individual -> the played pop. City -> the selected pop or building, else the region. Macro -> the country.
OwnerId shown_owner(Layer layer, const Owners& owners, const Context& ctx);

struct Panel {
  OwnerId owner;
  Scope scope;
  bool aggregated;  // Macro sums the country's parts; the other layers show the owner's own account
  std::int64_t money;
  std::vector<std::int64_t> bulk;  // per good, in bulk units
  std::vector<std::int64_t> units;
  std::vector<Owners::Row> money_by_part;  // Macro only: one row per region plus the country's own
};

Panel panel(Layer layer, const Ledger& l, const Owners& owners, const Goods& goods, const Money& money,
            const Context& ctx);

}  // namespace zs
