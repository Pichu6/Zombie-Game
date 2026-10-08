// Spec: docs/tier0/specs/37-flora-and-fauna.md
#include "zs/biota.h"

#include <algorithm>
#include <stdexcept>

#include "zs/clock.h"
#include "zs/detmath.h"
#include "zs/fields.h"

namespace zs {

namespace {
enum BiotaField : std::uint32_t { Mass = 101, Age = 102, OffX = 103, OffY = 104, Sex = 105 };
std::uint64_t population_key(OwnerId owner, GoodId species) {
  return (static_cast<std::uint64_t>(owner) << 16) | species;
}
}  // namespace

std::size_t Populations::add(Population p) {
  if (p.capacity <= 0 || p.rate_per_day < 0) throw std::invalid_argument("Populations::add: capacity must be positive, rate not negative");
  if (find(p.owner, p.species)) throw std::invalid_argument("Populations::add: one population per owner and species");
  all_.push_back(std::move(p));
  return all_.size() - 1;
}

const Population* Populations::find(OwnerId owner, GoodId species) const {
  for (const Population& p : all_) if (p.owner == owner && p.species == species) return &p;
  return nullptr;
}

Population* Populations::find(OwnerId owner, GoodId species) {
  for (Population& p : all_) if (p.owner == owner && p.species == species) return &p;
  return nullptr;
}

// B(t) = K / (1 + (K / B0 - 1) e^(-r t)), rounded toward the start value's side so a stock never overshoots K from
// below nor undershoots it from above, and B0 = 0 stays 0 (nothing grows from nothing).
std::int64_t Populations::grown(std::int64_t start, std::int64_t capacity, double rate_per_day, std::int64_t elapsed_ticks) {
  if (start <= 0) return 0;
  if (elapsed_ticks <= 0 || rate_per_day <= 0 || start == capacity) return start;
  const double days = static_cast<double>(elapsed_ticks) / static_cast<double>(kTicksPerGameDay);
  const double k = static_cast<double>(capacity), b0 = static_cast<double>(start);
  const double b = k / (1.0 + (k / b0 - 1.0) * detmath::exp(-rate_per_day * days));
  const auto r = static_cast<std::int64_t>(b);  // floor
  if (start < capacity) return std::clamp<std::int64_t>(r, start, capacity);
  const std::int64_t up = static_cast<double>(r) < b ? r + 1 : r;  // ceil when declining
  return std::clamp<std::int64_t>(up, capacity, start);
}

void Populations::reset(const Ledger& l, const Goods& goods, std::size_t i, std::int64_t tick) {
  Population& p = all_.at(i);
  p.start_amount = l.balance(p.owner, goods.def(p.species).bulk);
  p.start_tick = tick;
  p.caught_up = tick;
}

std::int64_t Populations::advance(Ledger& l, const Goods& goods, std::int64_t to_tick, CauseId cause) {
  std::int64_t net = 0;
  for (Population& p : all_) {
    if (to_tick <= p.caught_up) continue;
    const ResourceId bulk = goods.def(p.species).bulk;
    const std::int64_t before = grown(p.start_amount, p.capacity, p.rate_per_day, p.caught_up - p.start_tick);
    const std::int64_t after = grown(p.start_amount, p.capacity, p.rate_per_day, to_tick - p.start_tick);
    if (after > before) {
      l.declare_inflow(to_tick, p.owner, bulk, after - before, cause);
      net += after - before;
    } else if (after < before) {
      const std::int64_t out = std::min(before - after, l.balance(p.owner, bulk));
      if (out > 0) l.declare_outflow(to_tick, p.owner, bulk, out, cause);
      net -= out;
    }
    p.caught_up = to_tick;
  }
  return net;
}

Individual Populations::individual(std::uint64_t seed, OwnerId owner, GoodId species, std::uint64_t serial) {
  const std::uint64_t id = population_key(owner, species);
  const auto k = static_cast<std::uint32_t>(serial);
  Individual ind;
  ind.serial = serial;
  ind.mass_share = std::clamp(1.0 + 0.2 * fields::keyed_normal(seed, id, Mass, k), 0.5, 1.5);
  ind.age_share = fields::keyed_unit(seed, id, Age, k);
  ind.offset_x = fields::keyed_unit(seed, id, OffX, k);
  ind.offset_y = fields::keyed_unit(seed, id, OffY, k);
  ind.female = fields::keyed_unit(seed, id, Sex, k) < 0.5;
  return ind;
}

std::vector<Individual> Populations::roster(std::uint64_t seed, const Ledger& l, const Goods& goods, std::size_t i) const {
  const Population& p = all_.at(i);
  const GoodDef& d = goods.def(p.species);
  const std::int64_t n = l.balance(p.owner, d.bulk) / d.bulk_per_unit;
  std::vector<Individual> out;
  out.reserve(static_cast<std::size_t>(std::max<std::int64_t>(n, 0)));
  for (std::uint64_t s = 0; static_cast<std::int64_t>(out.size()) < n; ++s) {
    if (p.taken.contains(s)) continue;
    out.push_back(individual(seed, p.owner, p.species, s));
  }
  return out;
}

std::vector<Individual> Populations::approach(std::uint64_t seed, const Ledger& l, const Goods& goods, std::size_t i) {
  std::vector<Individual> out = roster(seed, l, goods, i);
  Population& p = all_.at(i);
  if (!out.empty()) p.next_serial = std::max(p.next_serial, out.back().serial + 1);
  return out;
}

std::int64_t Populations::take(Ledger& l, const Goods& goods, std::int64_t tick, std::size_t i, std::uint64_t serial,
                               OwnerId taker, CauseId cause, std::uint64_t seed) {
  // Catch the stock up first, so what is taken is what is there now.
  advance(l, goods, tick, cause);
  Population& p = all_.at(i);
  const GoodDef& d = goods.def(p.species);
  const std::int64_t n = l.balance(p.owner, d.bulk) / d.bulk_per_unit;
  // Is this serial on the roster? It is if it is not taken and fewer than n untaken serials precede it.
  if (p.taken.contains(serial)) return -1;
  std::int64_t before = 0;
  for (std::uint64_t s = 0; s < serial; ++s) if (!p.taken.contains(s)) ++before;
  if (before >= n) return -1;
  const Individual ind = individual(seed, p.owner, p.species, serial);
  const auto mass = static_cast<std::int64_t>(static_cast<double>(d.bulk_per_unit) * ind.mass_share);
  const std::int64_t moved = l.transfer(tick, p.owner, taker, d.bulk, std::max<std::int64_t>(mass, 1), cause);
  p.taken.insert(serial);
  p.next_serial = std::max(p.next_serial, serial + 1);
  reset(l, goods, i, tick);
  return moved;
}

bool Populations::depleted(const Ledger& l, const Goods& goods, std::size_t i) const {
  const Population& p = all_.at(i);
  return l.balance(p.owner, goods.def(p.species).bulk) * 10 < p.capacity * 3;
}

void Populations::save(ArchiveWriter& w) const {
  w.u64(all_.size());
  for (const Population& p : all_) {
    w.u32(p.owner); w.u16(p.species); w.i64(p.capacity); w.f64(p.rate_per_day);
    w.i64(p.start_amount); w.i64(p.start_tick); w.i64(p.caught_up); w.u64(p.next_serial);
    w.u64(p.taken.size());
    for (std::uint64_t s : p.taken) w.u64(s);
  }
}

void Populations::load(ArchiveReader& r) {
  std::vector<Population> all(r.u64());
  for (Population& p : all) {
    p.owner = r.u32(); p.species = r.u16(); p.capacity = r.i64(); p.rate_per_day = r.f64();
    p.start_amount = r.i64(); p.start_tick = r.i64(); p.caught_up = r.i64(); p.next_serial = r.u64();
    const std::uint64_t n = r.u64();
    for (std::uint64_t k = 0; k < n; ++k) p.taken.insert(r.u64());
  }
  all_ = std::move(all);
}

}  // namespace zs
