// Establishments and processing chains: a recipe is a declared conversion plus labour and power; an establishment
// runs one recipe with the staff it has, with an exact jump form; a chain is recipes fed by shipments; Macro output
// is the sum over establishments. Spec: docs/tier0/specs/33-establishments-and-chains.md
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "zs/economy.h"

namespace zs {

using RecipeId = std::uint16_t;

// One batch turns `input_per_batch` of the primary input into `output_per_batch` of the output (the ledger's declared
// conversion), consumes the secondary inputs (transfers to the consumed sink), and costs `labour_per_batch`
// worker-ticks. While it runs it needs `power_w` watts (0 = none). The same recipe serves a plant and a pair of hands.
struct RecipeDef {
  std::string name;
  GoodId input;
  std::int64_t input_per_batch;
  GoodId output;
  std::int64_t output_per_batch;
  std::vector<std::pair<GoodId, std::int64_t>> consumed;
  std::int64_t labour_per_batch;
  std::int64_t power_w = 0;
  ConversionId conversion = 0;  // filled by Recipes::add
};

class Recipes {
 public:
  RecipeId add(Ledger& l, const Goods& goods, RecipeDef def);
  const RecipeDef& def(RecipeId r) const { return defs_.at(r); }
  std::size_t size() const { return defs_.size(); }

  // Whole batches the owner's own stock can pay for.
  std::int64_t affordable(const Ledger& l, const Goods& goods, OwnerId owner, RecipeId r) const;
  // Runs up to `batches` whole batches on the owner's account: inputs from it, output to it, secondary inputs to
  // the consumed sink. Returns batches run. Labour is the caller's business (an establishment's bank or an action).
  std::int64_t run(Ledger& l, const Goods& goods, std::int64_t tick, OwnerId owner, RecipeId r, std::int64_t batches,
                   OwnerId consumed_sink, CauseId cause) const;

 private:
  std::vector<RecipeDef> defs_;
};

struct Establishment {
  OwnerId owner;              // its account: inputs, output, money (Scope::Building)
  RecipeId recipe;
  std::uint64_t roster = 0;   // the workplace roster whose present members are its staff (0 = staff is set by command)
  std::int64_t staff = 0;     // workers at work now
  std::int64_t power_w = 0;   // power available now
  std::int64_t labour_bank = 0;  // worker-ticks accrued and not yet spent on a batch
  std::int64_t caught_up = 0;
  std::int64_t produced = 0;  // batches, ever
};

class Establishments {
 public:
  std::size_t add(Establishment e);
  std::size_t size() const { return all_.size(); }
  const Establishment& at(std::size_t i) const { return all_.at(i); }
  Establishment& at(std::size_t i) { return all_.at(i); }
  const Establishment* find(OwnerId owner) const;
  Establishment* find(OwnerId owner);
  const std::vector<Establishment>& all() const { return all_; }
  void restore(std::vector<Establishment> all) { all_ = std::move(all); }

  struct Produced {
    std::size_t index;
    std::int64_t batches;
  };
  // Jump form (spec 33 rule 4): every establishment runs from its caught-up tick to `to_tick`. Labour accrues as
  // staff x ticks while powered; whole batches are run as far as inputs allow; labour that found no inputs is lost.
  // Stepping every tick and one jump give the same balances and banks. Returns what each establishment produced.
  std::vector<Produced> advance(Ledger& l, const Goods& goods, const Recipes& recipes, std::int64_t to_tick,
                                OwnerId consumed_sink, CauseId cause);

  // Macro (enumeration): batches produced, ever, by the establishments under `root` in the owner tree, all recipes or one.
  std::int64_t produced_under(const Owners& owners, OwnerId root, std::optional<RecipeId> recipe = std::nullopt) const;

 private:
  std::vector<Establishment> all_;
};

}  // namespace zs
