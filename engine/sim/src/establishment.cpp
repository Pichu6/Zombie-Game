// Spec: docs/tier0/specs/33-establishments-and-chains.md
#include "zs/establishment.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace zs {

// ---- Recipes

RecipeId Recipes::add(Ledger& l, const Goods& goods, RecipeDef def) {
  if (def.input_per_batch <= 0 || def.output_per_batch <= 0) throw std::invalid_argument("Recipes::add: batch amounts must be positive");
  if (def.labour_per_batch <= 0) throw std::invalid_argument("Recipes::add: labour per batch must be positive");
  if (def.power_w < 0) throw std::invalid_argument("Recipes::add: power must not be negative");
  for (const auto& [g, amount] : def.consumed)
    if (amount <= 0 || g >= goods.size()) throw std::invalid_argument("Recipes::add: bad secondary input");
  def.conversion = l.add_conversion({goods.def(def.input).bulk, goods.def(def.output).bulk, def.input_per_batch, def.output_per_batch});
  defs_.push_back(std::move(def));
  return static_cast<RecipeId>(defs_.size() - 1);
}

std::int64_t Recipes::affordable(const Ledger& l, const Goods& goods, OwnerId owner, RecipeId r) const {
  const RecipeDef& d = defs_.at(r);
  std::int64_t n = l.balance(owner, goods.def(d.input).bulk) / d.input_per_batch;
  for (const auto& [g, amount] : d.consumed) n = std::min(n, l.balance(owner, goods.def(g).bulk) / amount);
  return std::max<std::int64_t>(n, 0);
}

std::int64_t Recipes::run(Ledger& l, const Goods& goods, std::int64_t tick, OwnerId owner, RecipeId r,
                          std::int64_t batches, OwnerId consumed_sink, CauseId cause) const {
  const RecipeDef& d = defs_.at(r);
  const std::int64_t n = std::min(batches, affordable(l, goods, owner, r));
  if (n <= 0) return 0;
  for (const auto& [g, amount] : d.consumed) {
    const std::int64_t moved = l.transfer(tick, owner, consumed_sink, goods.def(g).bulk, n * amount, cause);
    if (moved != n * amount) throw std::logic_error("Recipes::run: a secondary input changed under us");
  }
  const std::int64_t done = l.convert(tick, owner, owner, d.conversion, n, cause);
  if (done != n) throw std::logic_error("Recipes::run: the primary input changed under us");
  return n;
}

// ---- Establishments

std::size_t Establishments::add(Establishment e) {
  if (find(e.owner)) throw std::invalid_argument("Establishments::add: one establishment per owner");
  all_.push_back(e);
  return all_.size() - 1;
}

const Establishment* Establishments::find(OwnerId owner) const {
  for (const Establishment& e : all_) if (e.owner == owner) return &e;
  return nullptr;
}

Establishment* Establishments::find(OwnerId owner) {
  for (Establishment& e : all_) if (e.owner == owner) return &e;
  return nullptr;
}

std::vector<Establishments::Produced> Establishments::advance(Ledger& l, const Goods& goods, const Recipes& recipes,
                                                             std::int64_t to_tick, OwnerId consumed_sink, CauseId cause) {
  std::vector<Produced> out;
  for (std::size_t i = 0; i < all_.size(); ++i) {
    Establishment& e = all_[i];
    const std::int64_t dt = to_tick - e.caught_up;
    if (dt <= 0) continue;
    const RecipeDef& d = recipes.def(e.recipe);
    const bool powered = d.power_w == 0 || e.power_w >= d.power_w;
    if (powered && e.staff > 0) {
      if (e.staff > std::numeric_limits<std::int64_t>::max() / dt) throw std::overflow_error("Establishments::advance: labour overflow");
      e.labour_bank += e.staff * dt;
    }
    const std::int64_t by_labour = e.labour_bank / d.labour_per_batch;
    const std::int64_t n = std::min(by_labour, recipes.affordable(l, goods, e.owner, e.recipe));
    if (n > 0) {
      recipes.run(l, goods, to_tick, e.owner, e.recipe, n, consumed_sink, cause);
      e.labour_bank -= n * d.labour_per_batch;
      e.produced += n;
      out.push_back({i, n});
    }
    // Labour that found no inputs (or no power, or no staff) is lost: workers do not bank idle hours.
    if (e.labour_bank >= d.labour_per_batch) e.labour_bank %= d.labour_per_batch;
    e.caught_up = to_tick;
  }
  return out;
}

std::int64_t Establishments::produced_under(const Owners& owners, OwnerId root, std::optional<RecipeId> recipe) const {
  std::int64_t sum = 0;
  for (const Establishment& e : all_) {
    if (recipe && e.recipe != *recipe) continue;
    if (owners.under(e.owner, root)) sum += e.produced;
  }
  return sum;
}

}  // namespace zs
