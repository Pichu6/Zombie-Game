// Spec 103: living stocks in the town's region. Wild game, fish, forest wood, pasture grass and crops in the field, one
// stock per cell and kind, growing by spec 37's logistic with the month's capacity and rate (seasons and the year's rain,
// specs 100-101), taken every month by hunting, fishing, logging, grazing and the harvest through the ledger.
// docs/tier0/specs/103-living-stocks-in-the-region.md.
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "zs/fields.h"
#include "zs/world.h"

namespace zs {

enum class LivingKind : std::uint8_t { Game, Wood, Pasture, Crops, Fish };
inline constexpr int kLivingKinds = 5;
const char* living_kind_name(LivingKind k);  // "game", "wood", "pasture", "crops", "fish"
const char* living_unit_name(LivingKind k);  // "kg" or "dm3" (wood)

// One stock as zs::gen::make_world gives it (rule 1); never saved, it comes back from the seed and settings.
struct LivingStock {
  std::uint64_t cell = 0;
  LivingKind kind = LivingKind::Game;
  fields::Climate climate{0, 0, 0};
  double peak = 0;       // K: game, wood and fish their capacity; pasture and crops their peak in a normal growing month
  double rate_year = 0;  // game, wood, fish: the logistic rate a year (pasture and crops have their own, rule 3)
  double take_year = 0;  // game, wood, fish: the share taken a year; pasture: livestock's share of the peak (rule 5)
};

struct LivingSetup {
  std::uint64_t world_seed = 0;
  std::vector<LivingStock> stocks;
};

namespace living {
// Rule 2: growing months (mean 5 C or more and rain at least twice the mean) from a climate's normals.
std::array<bool, 12> growing_months(const fields::Climate& c);
double normal_temp_c(const fields::Climate& c, int month);   // the month's normal mean
double normal_rain_mm(const fields::Climate& c, int month);  // the month's normal rain
// The months a stock grows in: grass and crops the growing months; game and wood the warm ones (5 C or more; every month
// where none is); fish every month.
std::array<bool, 12> months_of(const LivingStock& s);
// Rule 5: the harvest months (the first month after a growing season; January and July where every month grows).
std::array<bool, 12> harvest_months(const std::array<bool, 12>& growing);

// The month's capacity, daily rate and take (rules 3-5). `month_abs` is year * 12 + month (0 = January); `weather`
// off gives the normal year (rule 6).
struct Month {
  double k = 0, r_per_day = 0;
  double take_share = 0;  // of the stock, at the month's start
  double take_kg = 0;     // an amount at the month's start (grazing), clamped to the stock
};
Month month_of(const LivingStock& s, const std::array<bool, 12>& growing, std::uint64_t seed, std::int64_t month_abs, bool weather);

// Rule 6: the equilibrium under the normal weather, the stock at each month's start after its take (60 years by the
// month from half the peak), and the mean of the twelve.
std::array<double, 12> equilibrium(const LivingStock& s);
double equilibrium_mean(const LivingStock& s);
// What people take of it in a year at that equilibrium.
double equilibrium_take_year(const LivingStock& s);
// The stock after `days` of a month at constant K and r (spec 37's logistic in doubles).
double logistic(double b0, double k, double r_per_day, double days);
}  // namespace living

inline constexpr std::uint16_t LivingTake = 0x2103;  // subject = the taker (an owner), a = the stock, b = the amount

class LivingSystem : public System {
 public:
  LivingSystem(LivingSetup setup, std::int64_t start_day);
  const char* name() const override { return "living_stocks"; }
  void setup(World& w);  // resources, owners, the stocks minted at their equilibrium
  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;
  std::int64_t next_wake(const World& w, std::int64_t now) const override;
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& a) override;

  const LivingSetup& living() const { return setup_; }
  std::size_t size() const { return setup_.stocks.size(); }
  std::int64_t amount(const World& w, std::size_t i) const;  // in the ledger now
  OwnerId owner(std::size_t i) const { return owners_[i]; }
  OwnerId taker(LivingKind k) const { return takers_[static_cast<std::size_t>(k)]; }
  ResourceId resource(LivingKind k) const { return resources_[static_cast<std::size_t>(k)]; }
  std::int64_t taken(const World& w, LivingKind k) const;  // everything taken of a kind so far
  // Catches every stock up to `to` (growth, month starts and their takes); update calls it at each midnight.
  void advance(World& w, std::int64_t to);

  // The tick a civil month starts (midnight of its first day); tick 0 is 08:00 of the start day.
  std::int64_t month_start_tick(std::int64_t month_abs) const;
  std::int64_t month_at(std::int64_t tick) const;  // year * 12 + month of the civil date at a tick

 private:
  struct Lot {
    std::int64_t start_amount = 0, start_tick = 0, caught_up = 0;
    std::int64_t month = 0;  // month_abs whose K and r the lot follows
    std::int64_t k = 0;
    double r = 0;
  };
  void open_month(World& w, std::size_t i, std::int64_t month_abs, std::int64_t at);  // the take, then the month's lot
  void grow_to(World& w, std::size_t i, std::int64_t to);  // within the lot's month

  LivingSetup setup_;
  std::int64_t start_day_;
  std::vector<std::array<bool, 12>> growing_;
  std::array<ResourceId, kLivingKinds> resources_{};
  std::array<OwnerId, kLivingKinds> takers_{};
  std::vector<OwnerId> owners_;
  std::vector<Lot> lots_;
  std::int64_t next_midnight_ = 0;
};

}  // namespace zs
