#include "zs/ledger.h"

#include <algorithm>
#include <stdexcept>

namespace zs {
namespace {
constexpr std::size_t kTailSize = 1024;
}

ResourceId Ledger::add_resource(std::string name) {
  resources_.push_back(std::move(name));
  expected_.push_back(0);
  return static_cast<ResourceId>(resources_.size() - 1);
}

OwnerId Ledger::add_owner(std::string name) {
  owners_.push_back(std::move(name));
  return static_cast<OwnerId>(owners_.size() - 1);
}

ConversionId Ledger::add_conversion(ConversionDef def) {
  if (def.from_per_batch <= 0 || def.to_per_batch < 0) throw std::invalid_argument("bad conversion");
  conversions_.push_back(def);
  return static_cast<ConversionId>(conversions_.size() - 1);
}

void Ledger::record(const LedgerEntry& e) {
  tail_.push_back(e);
  if (tail_.size() > kTailSize) tail_.pop_front();
}

void Ledger::mint(OwnerId owner, ResourceId resource, std::int64_t amount, CauseId cause) {
  if (!setup_open_) throw std::logic_error("mint after setup: amounts may only move");
  if (amount < 0) throw std::invalid_argument("negative mint");
  balances_[{owner, resource}] += amount;
  expected_[resource] += amount;
  record({0, EntryKind::Mint, owner, owner, resource, resource, amount, amount, cause});
}

std::int64_t Ledger::transfer(std::int64_t tick, OwnerId from, OwnerId to, ResourceId resource, std::int64_t amount,
                              CauseId cause) {
  if (amount <= 0 || from == to) return 0;
  std::int64_t& src = balances_[{from, resource}];
  const std::int64_t moved = std::min(amount, src);
  if (moved <= 0) return 0;
  src -= moved;
  balances_[{to, resource}] += moved;
  record({tick, EntryKind::Transfer, from, to, resource, resource, moved, moved, cause});
  return moved;
}

std::int64_t Ledger::convert(std::int64_t tick, OwnerId from, OwnerId to, ConversionId conversion,
                             std::int64_t batches, CauseId cause) {
  if (batches <= 0) return 0;
  const ConversionDef& c = conversions_.at(conversion);
  std::int64_t& src = balances_[{from, c.from}];
  const std::int64_t done = std::min(batches, src / c.from_per_batch);
  if (done <= 0) return 0;
  const std::int64_t out = done * c.from_per_batch;
  const std::int64_t in = done * c.to_per_batch;
  src -= out;
  balances_[{to, c.to}] += in;
  expected_[c.from] -= out;
  expected_[c.to] += in;
  record({tick, EntryKind::Conversion, from, to, c.from, c.to, out, in, cause});
  return done;
}

std::int64_t Ledger::balance(OwnerId owner, ResourceId resource) const {
  auto it = balances_.find({owner, resource});
  return it == balances_.end() ? 0 : it->second;
}

std::int64_t Ledger::total(ResourceId resource) const {
  std::int64_t sum = 0;
  for (const auto& [key, value] : balances_)
    if (key.second == resource) sum += value;
  return sum;
}

std::optional<std::string> Ledger::check() const {
  for (const auto& [key, value] : balances_)
    if (value < 0)
      return "negative stock: " + owners_.at(key.first) + " holds " + std::to_string(value) + " " +
             resources_.at(key.second);
  for (ResourceId r = 0; r < resources_.size(); ++r)
    if (total(r) != expected_[r])
      return "not conserved: " + resources_[r] + " total " + std::to_string(total(r)) + ", expected " +
             std::to_string(expected_[r]);
  return std::nullopt;
}

void Ledger::restore(std::map<std::pair<OwnerId, ResourceId>, std::int64_t> balances, std::vector<std::int64_t> expected,
                     bool setup_open) {
  balances_ = std::move(balances);
  expected_ = std::move(expected);
  setup_open_ = setup_open;
  tail_.clear();
}

bool Locations::place(EntityId entity, EntityId parent) {
  if (entity == kWorld) return false;
  for (EntityId p = parent; p != kWorld; p = parent_of(p))
    if (p == entity) return false;  // would create a cycle
  parent_[entity] = parent;
  return true;
}

EntityId Locations::parent_of(EntityId entity) const {
  auto it = parent_.find(entity);
  return it == parent_.end() ? kWorld : it->second;
}

}  // namespace zs
