// Stocks, transfers, conversions and instance locations. Spec: docs/tier0/specs/03-stock-transfer.md
// Conserved amounts are integers held by exactly one owner. They move; they are never recomputed.
#pragma once
#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "zs/draw.h"  // EntityId

namespace zs {

using OwnerId = std::uint32_t;
using ResourceId = std::uint16_t;
using ConversionId = std::uint16_t;
using CauseId = std::uint64_t;  // the event that caused a movement (0 = setup)

enum class EntryKind : std::uint8_t { Mint, Transfer, Conversion, Inflow, Outflow };

struct LedgerEntry {
  std::int64_t tick;
  EntryKind kind;
  OwnerId from;
  OwnerId to;
  ResourceId resource_from;
  ResourceId resource_to;
  std::int64_t amount_from;
  std::int64_t amount_to;
  CauseId cause;
};

// A declared conversion: one batch turns `from_per_batch` of one resource into `to_per_batch`
// of another (e.g. 1 g glycogen -> 17 kJ spent). Only declared pairs can convert.
struct ConversionDef {
  ResourceId from;
  ResourceId to;
  std::int64_t from_per_batch;
  std::int64_t to_per_batch;
};

class Ledger {
 public:
  ResourceId add_resource(std::string name);
  OwnerId add_owner(std::string name);
  ConversionId add_conversion(ConversionDef def);

  // Creates amounts out of nothing. Only allowed during setup (before close_setup()).
  void mint(OwnerId owner, ResourceId resource, std::int64_t amount, CauseId cause = 0);
  void close_setup() { setup_open_ = false; }

  // Declared flows (spec 29): amounts that enter or leave what the ledger tracks after setup, for example a
  // person's tissue when a body is embodied or given back. Unlike a silent mint they are allowed at any time,
  // carry their cause, change the expected total by exactly their amount, and leave the check intact.
  // An outflow takes exactly `amount` (it throws if the owner holds less: the caller must know what it removes).
  void declare_inflow(std::int64_t tick, OwnerId owner, ResourceId resource, std::int64_t amount, CauseId cause);
  void declare_outflow(std::int64_t tick, OwnerId owner, ResourceId resource, std::int64_t amount, CauseId cause);

  // Moves up to `amount` (clamped to what `from` holds). Returns what moved.
  std::int64_t transfer(std::int64_t tick, OwnerId from, OwnerId to, ResourceId resource, std::int64_t amount,
                        CauseId cause);

  // Converts up to `batches` whole batches (clamped to what `from` holds). Returns batches converted.
  std::int64_t convert(std::int64_t tick, OwnerId from, OwnerId to, ConversionId conversion, std::int64_t batches,
                       CauseId cause);

  std::int64_t balance(OwnerId owner, ResourceId resource) const;
  std::int64_t total(ResourceId resource) const;  // sum over all owners, sinks included

  // Checks conservation and non-negativity. Returns a description of the first violation.
  std::optional<std::string> check() const;

  const std::deque<LedgerEntry>& tail() const { return tail_; }  // last entries, for debugging
  const std::vector<std::string>& resource_names() const { return resources_; }
  const std::vector<std::string>& owner_names() const { return owners_; }
  const std::vector<ConversionDef>& conversions() const { return conversions_; }

  // Persistence (the definitions are rebuilt by setup code; saves hold balances and expected totals).
  const std::map<std::pair<OwnerId, ResourceId>, std::int64_t>& balances() const { return balances_; }
  const std::vector<std::int64_t>& expected_totals() const { return expected_; }
  bool setup_open() const { return setup_open_; }
  void restore(std::map<std::pair<OwnerId, ResourceId>, std::int64_t> balances, std::vector<std::int64_t> expected,
               bool setup_open);

 private:
  void record(const LedgerEntry& e);

  std::vector<std::string> resources_;
  std::vector<std::string> owners_;
  std::vector<ConversionDef> conversions_;
  std::map<std::pair<OwnerId, ResourceId>, std::int64_t> balances_;
  std::vector<std::int64_t> expected_;  // per resource: minted + converted in - converted out
  std::deque<LedgerEntry> tail_;
  bool setup_open_ = true;
};

// Where each instance with identity is. Locations form a tree rooted at 0 (the world).
class Locations {
 public:
  static constexpr EntityId kWorld = 0;

  // Returns false (and changes nothing) if the move would create a cycle.
  bool place(EntityId entity, EntityId parent);
  EntityId parent_of(EntityId entity) const;  // kWorld if unknown
  bool contains(EntityId entity) const { return parent_.contains(entity); }
  const std::map<EntityId, EntityId>& all() const { return parent_; }
  const std::set<EntityId>& children(EntityId parent) const;  // derived index, in id order
  void restore(std::map<EntityId, EntityId> parents);

 private:
  std::map<EntityId, EntityId> parent_;
  std::map<EntityId, std::set<EntityId>> children_;
};

}  // namespace zs
