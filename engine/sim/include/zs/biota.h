// Flora and fauna across levels: a stock per place and species on the ledger (a good whose unit is one tree or
// one animal), logistic growth with an exact jump form, harvest as transfers, and individuals generated from the
// stock on approach by keyed serials, so a kill removes one and leaves the others as they were.
// Spec: docs/tier0/specs/37-flora-and-fauna.md
#pragma once
#include <cstdint>
#include <set>
#include <vector>

#include "zs/archive.h"
#include "zs/economy.h"

namespace zs {

// A living stock: the owner's bulk of a species good (kg of biomass or heads x unit mass) grows logistically toward
// its carrying capacity. Growth enters the ledger as a declared inflow (nothing is minted silently); decline above
// capacity leaves as a declared outflow. Any transfer of the bulk (a harvest, a kill) must reset the lot.
struct Population {
  OwnerId owner;           // the place: a cell's or a region's account
  GoodId species;          // bulk = biomass (kg), unit = one individual (bulk_per_unit = its mass)
  std::int64_t capacity;   // K, in bulk units
  double rate_per_day;     // r: intrinsic growth per day
  std::int64_t start_amount = 0;  // the lot: the stock when last touched
  std::int64_t start_tick = 0;
  std::int64_t caught_up = 0;
  std::uint64_t next_serial = 0;  // individuals ever shown, so a new one never reuses an old identity
  std::set<std::uint64_t> taken;  // serials removed (killed, cut); the roster skips them
};

// One individual, generated on approach from (seed, population, serial): the same serial is the same animal or
// tree whenever it is asked for.
struct Individual {
  std::uint64_t serial;
  double mass_share;   // its mass relative to the unit mass (0.5..1.5)
  double age_share;    // 0..1 of the species' life
  double offset_x, offset_y;  // position inside the place, 0..1
  bool female;
};

class Populations {
 public:
  std::size_t add(Population p);
  std::size_t size() const { return all_.size(); }
  const Population& at(std::size_t i) const { return all_.at(i); }
  Population& at(std::size_t i) { return all_.at(i); }
  const Population* find(OwnerId owner, GoodId species) const;
  Population* find(OwnerId owner, GoodId species);
  const std::vector<Population>& all() const { return all_; }
  void restore(std::vector<Population> all) { all_ = std::move(all); }

  // Logistic closed form in integers: the stock after `elapsed` ticks from `start` toward `capacity` at `rate`.
  static std::int64_t grown(std::int64_t start, std::int64_t capacity, double rate_per_day, std::int64_t elapsed_ticks);

  // The lot restarts from what the owner holds now (after a harvest, a kill, a release).
  void reset(const Ledger& l, const Goods& goods, std::size_t i, std::int64_t tick);
  // Jump form: every population grows (or declines) from its caught-up tick to `to_tick`; returns the net inflow.
  std::int64_t advance(Ledger& l, const Goods& goods, std::int64_t to_tick, CauseId cause);

  // On approach: the individuals alive now, the first n serials not taken, n = stock / unit mass; the remainder
  // stays bulk (a stock of saplings, not a tree). Pure in (seed, population, stock, taken).
  std::vector<Individual> roster(std::uint64_t seed, const Ledger& l, const Goods& goods, std::size_t i) const;
  static Individual individual(std::uint64_t seed, OwnerId owner, GoodId species, std::uint64_t serial);
  // Shows the roster and remembers how many serials have been shown (so growth after this appends new ones).
  std::vector<Individual> approach(std::uint64_t seed, const Ledger& l, const Goods& goods, std::size_t i);

  // A kill or a cut: the individual leaves the stock and its mass goes to the taker's account. Returns the mass
  // moved, or -1 if that serial is not on the roster. The lot is reset.
  std::int64_t take(Ledger& l, const Goods& goods, std::int64_t tick, std::size_t i, std::uint64_t serial,
                    OwnerId taker, CauseId cause, std::uint64_t seed);

  // Macro readouts: the sustainable yield r K / 4 per day in bulk units, and whether the stock is below 30 % of K.
  static double sustainable_yield_per_day(const Population& p) { return p.rate_per_day * static_cast<double>(p.capacity) / 4.0; }
  bool depleted(const Ledger& l, const Goods& goods, std::size_t i) const;

  void save(ArchiveWriter& w) const;
  void load(ArchiveReader& r);

 private:
  std::vector<Population> all_;
};

}  // namespace zs
