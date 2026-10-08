// Spec: docs/tier0/specs/32-stocks-goods-and-money.md
#include "zs/economy.h"

#include <algorithm>
#include <stdexcept>

#include "zs/clock.h"
#include "zs/detmath.h"

namespace zs {

// ---- Owners

OwnerId Owners::add(Ledger& l, std::string name, Scope scope, OwnerId parent) {
  if (parent != kNoOwner && !info_.contains(parent)) throw std::invalid_argument("Owners::add: unknown parent");
  const OwnerId id = l.add_owner(name);
  info_[id] = Info{std::move(name), scope, parent};
  if (parent != kNoOwner) children_[parent].push_back(id);
  return id;
}

const std::vector<OwnerId>& Owners::children(OwnerId o) const {
  static const std::vector<OwnerId> none;
  auto it = children_.find(o);
  return it == children_.end() ? none : it->second;
}

bool Owners::under(OwnerId o, OwnerId root) const {
  for (OwnerId cur = o; cur != kNoOwner; cur = info_.at(cur).parent)
    if (cur == root) return true;
  return false;
}

std::int64_t Owners::aggregate(const Ledger& l, OwnerId o, ResourceId r) const {
  std::int64_t sum = l.balance(o, r);
  for (OwnerId c : children(o)) sum += aggregate(l, c, r);
  return sum;
}

std::vector<Owners::Row> Owners::by_children(const Ledger& l, OwnerId o, ResourceId r) const {
  std::vector<Row> rows;
  for (OwnerId c : children(o)) rows.push_back({c, aggregate(l, c, r)});
  rows.push_back({o, l.balance(o, r)});
  return rows;
}

// ---- Goods

GoodId Goods::add(Ledger& l, std::string name, std::string bulk_resource, std::string unit_resource,
                  std::int64_t bulk_per_unit, double half_life_days) {
  if (bulk_per_unit <= 0) throw std::invalid_argument("Goods::add: bulk_per_unit must be positive");
  if (half_life_days < 0) throw std::invalid_argument("Goods::add: half life must not be negative");
  GoodDef d;
  d.name = std::move(name);
  d.bulk = l.add_resource(std::move(bulk_resource));
  d.unit = l.add_resource(std::move(unit_resource));
  d.bulk_per_unit = bulk_per_unit;
  d.pack = l.add_conversion({d.bulk, d.unit, bulk_per_unit, 1});
  d.unpack = l.add_conversion({d.unit, d.bulk, 1, bulk_per_unit});
  d.half_life_days = half_life_days;
  defs_.push_back(std::move(d));
  return static_cast<GoodId>(defs_.size() - 1);
}

std::int64_t Goods::materialise(Ledger& l, std::int64_t tick, OwnerId owner, GoodId g, std::int64_t max_units,
                                CauseId cause) const {
  const GoodDef& d = defs_.at(g);
  return l.convert(tick, owner, owner, d.pack, max_units, cause);
}

std::int64_t Goods::dematerialise(Ledger& l, std::int64_t tick, OwnerId owner, GoodId g, CauseId cause) const {
  const GoodDef& d = defs_.at(g);
  const std::int64_t units = l.balance(owner, d.unit);
  return l.convert(tick, owner, owner, d.unpack, units, cause);
}

// ---- Spoilage

std::int64_t Spoilage::remaining(std::int64_t start_amount, double half_life_days, std::int64_t elapsed_ticks) {
  if (start_amount <= 0) return 0;
  if (half_life_days <= 0 || elapsed_ticks <= 0) return start_amount;
  const double half_life_ticks = half_life_days * static_cast<double>(kTicksPerGameDay);
  const double ln2 = 0.69314718055994530942;
  const double x = static_cast<double>(start_amount) *
                   detmath::exp(-ln2 * static_cast<double>(elapsed_ticks) / half_life_ticks);
  auto r = static_cast<std::int64_t>(x);
  if (static_cast<double>(r) < x) ++r;  // ceil: the last gram goes only when the curve says so
  return std::clamp<std::int64_t>(r, 0, start_amount);
}

void Spoilage::reset(const Ledger& l, const Goods& goods, OwnerId owner, GoodId g, std::int64_t tick) {
  const std::int64_t amount = l.balance(owner, goods.def(g).bulk);
  for (Lot& lot : lots_) {
    if (lot.owner == owner && lot.good == g) {
      lot = Lot{owner, g, amount, tick, tick};
      return;
    }
  }
  lots_.push_back(Lot{owner, g, amount, tick, tick});
}

std::int64_t Spoilage::advance(Ledger& l, const Goods& goods, OwnerId decayed, std::int64_t to_tick,
                               CauseId cause) {
  std::int64_t moved = 0;
  for (Lot& lot : lots_) {
    if (to_tick <= lot.caught_up) continue;
    const GoodDef& d = goods.def(lot.good);
    const std::int64_t before = remaining(lot.start_amount, d.half_life_days, lot.caught_up - lot.start_tick);
    const std::int64_t after = remaining(lot.start_amount, d.half_life_days, to_tick - lot.start_tick);
    if (before > after) moved += l.transfer(to_tick, lot.owner, decayed, d.bulk, before - after, cause);
    lot.caught_up = to_tick;
  }
  return moved;
}

// ---- Money

std::int64_t Money::purchase(Ledger& l, std::int64_t tick, OwnerId buyer, OwnerId seller, const Goods& goods,
                             GoodId g, std::int64_t price_cents, std::int64_t want, CauseId cause) {
  if (want <= 0 || price_cents < 0 || buyer == seller) return 0;
  const GoodDef& d = goods.def(g);
  std::int64_t n = std::min(want, l.balance(seller, d.unit));
  if (price_cents > 0) n = std::min(n, l.balance(buyer, money_) / price_cents);
  if (n <= 0) return 0;
  const std::int64_t goods_moved = l.transfer(tick, seller, buyer, d.unit, n, cause);
  const std::int64_t cents = goods_moved * price_cents;
  const std::int64_t money_moved = l.transfer(tick, buyer, seller, money_, cents, cause);
  if (money_moved != cents) throw std::logic_error("Money::purchase: the buyer's money changed under us");
  tally(buyer, seller, FlowKind::Sale, cents);
  return goods_moved;
}

std::int64_t Money::pay(Ledger& l, std::int64_t tick, OwnerId from, OwnerId to, std::int64_t cents, FlowKind kind,
                        CauseId cause) {
  if (cents <= 0 || from == to) return 0;
  const std::int64_t moved = l.transfer(tick, from, to, money_, cents, cause);
  tally(from, to, kind, moved);
  return moved;
}

void Money::tally(OwnerId from, OwnerId to, FlowKind k, std::int64_t cents) {
  if (cents <= 0) return;
  out_[{from, k}] += cents;
  in_[{to, k}] += cents;
}

std::int64_t Money::received(OwnerId o, FlowKind k) const {
  auto it = in_.find({o, k});
  return it == in_.end() ? 0 : it->second;
}

std::int64_t Money::paid(OwnerId o, FlowKind k) const {
  auto it = out_.find({o, k});
  return it == out_.end() ? 0 : it->second;
}

std::int64_t Money::received(OwnerId o) const {
  std::int64_t sum = 0;
  for (const auto& [key, v] : in_) if (key.first == o) sum += v;
  return sum;
}

std::int64_t Money::paid(OwnerId o) const {
  std::int64_t sum = 0;
  for (const auto& [key, v] : out_) if (key.first == o) sum += v;
  return sum;
}

// ---- Decision 0051

OwnerId shown_owner(Layer layer, const Owners& owners, const Context& ctx) {
  switch (layer) {
    case Layer::Individual: return ctx.played_pop;
    case Layer::Macro: return ctx.country;
    case Layer::City:
      if (ctx.selected && owners.known(*ctx.selected)) {
        const Scope s = owners.scope(*ctx.selected);
        if (s == Scope::Pop || s == Scope::Building) return *ctx.selected;
      }
      return ctx.region;
  }
  return ctx.region;
}

Panel panel(Layer layer, const Ledger& l, const Owners& owners, const Goods& goods, const Money& money,
            const Context& ctx) {
  Panel p;
  p.owner = shown_owner(layer, owners, ctx);
  p.scope = owners.scope(p.owner);
  p.aggregated = layer == Layer::Macro;
  const auto amount = [&](ResourceId r) {
    return p.aggregated ? owners.aggregate(l, p.owner, r) : l.balance(p.owner, r);
  };
  p.money = amount(money.resource());
  for (std::size_t g = 0; g < goods.size(); ++g) {
    const GoodDef& d = goods.def(static_cast<GoodId>(g));
    p.bulk.push_back(amount(d.bulk));
    p.units.push_back(amount(d.unit));
  }
  if (p.aggregated) p.money_by_part = owners.by_children(l, p.owner, money.resource());
  return p;
}

}  // namespace zs
