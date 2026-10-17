// Living stocks in the town's region. Spec: docs/tier0/specs/103-living-stocks-in-the-region.md.
#include "zs/living_stocks.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "zs/anomalies.h"
#include "zs/biota.h"
#include "zs/calendar.h"
#include "zs/clock.h"
#include "zs/detmath.h"
#include "zs/events.h"

namespace zs {

namespace {
constexpr std::array<int, 12> kMonthDays{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
constexpr double kDaysPerMonth = 365.0 / 12.0;
// Rule 3 (estimates under LS-7): grass and crops' own rates, grass's dormant share of its peak.
constexpr double kPastureRate = 0.08, kCropRate = 0.07, kPastureDormant = 0.4;
constexpr double kHarvestShare = 0.9;  // rule 5: the tenth left is seed and stubble
constexpr double kGrowMinC = 5.0;      // LS-8
constexpr double kSoilStoreMm = 100;   // LS-8: the soil water a crop can draw on after the rains
enum LivingRefusal : int { NoStock = 1, NoTaker = 2 };

double monthly_share(double year_share) {
  if (year_share <= 0) return 0;
  if (year_share >= 1) return 1;
  return 1.0 - detmath::pow(1.0 - year_share, 1.0 / 12.0);
}
int month_index(std::int64_t month_abs) { return static_cast<int>(((month_abs % 12) + 12) % 12); }
}  // namespace

const char* living_kind_name(LivingKind k) {
  switch (k) {
    case LivingKind::Game: return "game";
    case LivingKind::Wood: return "wood";
    case LivingKind::Pasture: return "pasture";
    case LivingKind::Crops: return "crops";
    case LivingKind::Fish: return "fish";
  }
  return "?";
}
const char* living_unit_name(LivingKind k) { return k == LivingKind::Wood ? "dm3" : "kg"; }

namespace living {

double normal_temp_c(const fields::Climate& c, int month) {
  // Spec 36's seasonal cosine (coldest about day 20 in the north) at the month's middle.
  int day = 0;
  for (int m = 0; m < month; ++m) day += kMonthDays[static_cast<std::size_t>(m)];
  const double mid = day + kMonthDays[static_cast<std::size_t>(month)] / 2.0;
  const double phase = 2.0 * detmath::pi * (mid - 20.0) / fields::kDaysPerYear + (c.south ? detmath::pi : 0.0);
  return c.temp_mean_c - 0.5 * c.temp_range_c * detmath::cos(phase);
}

double normal_rain_mm(const fields::Climate& c, int month) { return c.rain_mm * fields::rain_month_share(c, month); }

std::array<bool, 12> growing_months(const fields::Climate& c) {
  // FAO's growing period (LS-8): warm enough, and rain over half the potential evaporation or water left in a 100 mm soil
  // store filled by earlier months' surplus. PET by month from spec 52's line (RV-5: 200 + 40 T mm a year, at least 100).
  std::array<double, 12> t{}, p{}, pet{};
  for (int m = 0; m < 12; ++m) {
    t[static_cast<std::size_t>(m)] = normal_temp_c(c, m);
    p[static_cast<std::size_t>(m)] = normal_rain_mm(c, m);
    pet[static_cast<std::size_t>(m)] = std::max(100.0, 200.0 + 40.0 * t[static_cast<std::size_t>(m)]) / 12.0;
  }
  std::array<bool, 12> g{};
  double store = 0;
  for (int pass = 0; pass < 3; ++pass)  // the store's cycle settles in two years
    for (int m = 0; m < 12; ++m) {
      const auto i = static_cast<std::size_t>(m);
      const double wet = p[i] - (t[i] > 0 ? pet[i] : 0.0);
      const bool water = p[i] >= 0.5 * pet[i] || store > 0;
      store = std::clamp(store + wet, 0.0, kSoilStoreMm);
      if (pass == 2) g[i] = t[i] >= kGrowMinC && water;
    }
  return g;
}

std::array<bool, 12> harvest_months(const std::array<bool, 12>& growing) {
  std::array<bool, 12> h{};
  int n = 0;
  for (bool b : growing) n += b ? 1 : 0;
  if (n == 12) {
    h[0] = h[6] = true;
    return h;
  }
  // The month after the end of the longest run of growing months (the year's main season; a shorter run before a dry gap
  // keeps growing into it). Ties: the earliest end.
  int best_len = 0, best_end = -1;
  for (int end = 0; end < 12; ++end) {
    if (!growing[static_cast<std::size_t>(end)] || growing[static_cast<std::size_t>((end + 1) % 12)]) continue;
    int len = 0;
    while (len < 12 && growing[static_cast<std::size_t>((end - len + 12) % 12)]) ++len;
    if (len > best_len) best_len = len, best_end = end;
  }
  if (best_end >= 0) h[static_cast<std::size_t>((best_end + 1) % 12)] = true;
  return h;
}

std::array<bool, 12> months_of(const LivingStock& s) {
  std::array<bool, 12> g{};
  if (s.kind == LivingKind::Pasture || s.kind == LivingKind::Crops) return growing_months(s.climate);
  if (s.kind == LivingKind::Fish) {
    g.fill(true);
    return g;
  }
  bool any = false;
  for (int m = 0; m < 12; ++m) any |= g[static_cast<std::size_t>(m)] = normal_temp_c(s.climate, m) >= kGrowMinC;
  if (!any) g.fill(true);
  return g;
}

Month month_of(const LivingStock& s, const std::array<bool, 12>& growing, std::uint64_t seed, std::int64_t month_abs, bool weather) {
  const int m = month_index(month_abs);
  const bool grows = growing[static_cast<std::size_t>(m)];
  int n = 0;
  for (bool b : growing) n += b ? 1 : 0;
  // Rule 4: grass and crops follow the rain of this month and the last.
  double w = 1.0;
  if (weather && grows && s.kind == LivingKind::Pasture) {
    const double f = 0.5 * (fields::rain_month_factor(seed, s.climate, month_abs) + fields::rain_month_factor(seed, s.climate, month_abs - 1));
    w = std::clamp(0.5 + 0.5 * f, 0.3, 1.15);
  } else if (weather && grows && s.kind == LivingKind::Crops) {
    // A crop's yield is its season's: the mean rain factor of the season's months so far (rule 4).
    double f = 0;
    int k = 0;
    for (std::int64_t a = month_abs; k < 12 && growing[static_cast<std::size_t>(month_index(a))]; --a, ++k)
      f += 0.5 * (fields::rain_month_factor(seed, s.climate, a) + fields::rain_month_factor(seed, s.climate, a - 1));
    w = std::clamp(0.5 + 0.5 * f / k, 0.3, 1.15);
  }
  Month out;
  switch (s.kind) {
    case LivingKind::Game:
    case LivingKind::Wood:
      out.k = s.peak;
      out.r_per_day = grows && n > 0 ? s.rate_year / (n * kDaysPerMonth) : 0.0;
      out.take_share = monthly_share(s.take_year);
      break;
    case LivingKind::Fish:
      out.k = s.peak;
      out.r_per_day = s.rate_year / fields::kDaysPerYear;
      out.take_share = monthly_share(s.take_year);
      break;
    case LivingKind::Pasture:
      out.k = grows ? s.peak * w : s.peak * kPastureDormant;
      out.r_per_day = grows ? kPastureRate : 0.0;
      out.take_kg = grows && n > 0 ? s.take_year * s.peak / n : 0.0;
      break;
    case LivingKind::Crops:
      out.k = s.peak * w;
      out.r_per_day = grows ? kCropRate : 0.0;
      out.take_share = harvest_months(growing)[static_cast<std::size_t>(m)] ? kHarvestShare : 0.0;
      break;
  }
  return out;
}

double logistic(double b0, double k, double r_per_day, double days) {
  if (b0 <= 0 || k <= 0) return b0 <= 0 ? 0.0 : (r_per_day > 0 ? 0.0 : b0);
  if (r_per_day <= 0 || days <= 0) return b0 > k && r_per_day > 0 ? k : b0;
  return k / (1.0 + (k / b0 - 1.0) * detmath::exp(-r_per_day * days));
}

std::array<double, 12> equilibrium(const LivingStock& s) {
  const std::array<bool, 12> g = months_of(s);
  std::array<double, 12> at{};
  // From the continuous harvest's equilibrium K (1 - q / r) for game, wood and fish, so only the seasons are left to
  // settle; until the year repeats, at most 3,000 years (a slow forest from far off needs centuries).
  double b = s.kind == LivingKind::Pasture ? s.peak : s.kind == LivingKind::Crops ? 0.1 * s.peak : 0.5 * s.peak;
  if ((s.kind == LivingKind::Game || s.kind == LivingKind::Wood || s.kind == LivingKind::Fish) && s.rate_year > 0) {
    const double q = s.take_year < 1 ? -detmath::log(1.0 - s.take_year) : 10.0;
    b = s.peak * std::clamp(1.0 - q / s.rate_year, 0.01, 1.0);
  }
  for (int year = 0; year < 3000; ++year) {
    const double was = b;
    for (int m = 0; m < 12; ++m) {
      const Month mo = month_of(s, g, 0, m, false);
      b -= std::min(b, b * mo.take_share + mo.take_kg);  // the month's start: the take, then dormant grass dies back
      if (s.kind == LivingKind::Pasture && mo.r_per_day <= 0 && b > mo.k) b = mo.k;
      at[static_cast<std::size_t>(m)] = b;
      b = logistic(b, mo.k, mo.r_per_day, kMonthDays[static_cast<std::size_t>(m)]);
    }
    if (year > 2 && std::abs(b - was) <= 1e-8 * s.peak) break;
  }
  return at;
}

double equilibrium_mean(const LivingStock& s) {
  double sum = 0;
  for (double v : equilibrium(s)) sum += v;
  return sum / 12.0;
}

double equilibrium_take_year(const LivingStock& s) {
  const std::array<bool, 12> g = months_of(s);
  const std::array<double, 12> at = equilibrium(s);
  double taken = 0;
  for (int m = 0; m < 12; ++m) {
    // The stock just before month m's take is month m-1's start value grown through month m-1.
    const int p = (m + 11) % 12;
    const Month prev = month_of(s, g, 0, p, false), mo = month_of(s, g, 0, m, false);
    const double b = logistic(at[static_cast<std::size_t>(p)], prev.k, prev.r_per_day, kMonthDays[static_cast<std::size_t>(p)]);
    taken += std::min(b, b * mo.take_share + mo.take_kg);
  }
  return taken;
}

}  // namespace living

LivingSystem::LivingSystem(LivingSetup setup, std::int64_t start_day) : setup_(std::move(setup)), start_day_(start_day) {
  for (const LivingStock& s : setup_.stocks) growing_.push_back(living::months_of(s));
}

std::int64_t LivingSystem::month_start_tick(std::int64_t month_abs) const {
  const auto y = static_cast<int>(month_abs >= 0 ? month_abs / 12 : (month_abs - 11) / 12);
  const int m = month_index(month_abs) + 1;
  return (days_from_civil(y, m, 1) - start_day_) * kTicksPerGameDay - 8 * kTicksPerGameHour;
}

std::int64_t LivingSystem::month_at(std::int64_t tick) const {
  const CivilTime t = civil_time(start_day_, tick);
  return static_cast<std::int64_t>(t.date.year) * 12 + (t.date.month - 1);
}

void LivingSystem::setup(World& w) {
  Ledger& l = w.ledger;
  for (int k = 0; k < kLivingKinds; ++k) {
    const auto kind = static_cast<LivingKind>(k);
    resources_[static_cast<std::size_t>(k)] = l.add_resource(std::string("living_") + living_kind_name(kind) + "_" + living_unit_name(kind));
    takers_[static_cast<std::size_t>(k)] = l.add_owner(std::string("living:") + living_kind_name(kind) + " taken");
  }
  // Rule 6: each stock at its equilibrium for the start month, grown to tick 0 by the month's own law.
  const std::int64_t now = w.clock.tick;
  const std::int64_t month = month_at(now);
  for (std::size_t i = 0; i < setup_.stocks.size(); ++i) {
    const LivingStock& s = setup_.stocks[i];
    owners_.push_back(l.add_owner(std::string("living:") + living_kind_name(s.kind) + ":" + std::to_string(s.cell)));
    const living::Month mo = living::month_of(s, growing_[i], setup_.world_seed, month, true);
    const double start = living::equilibrium(s)[static_cast<std::size_t>(month_index(month))];
    const double days = static_cast<double>(now - month_start_tick(month)) / static_cast<double>(kTicksPerGameDay);
    const auto amount = static_cast<std::int64_t>(std::llround(living::logistic(start, mo.k, mo.r_per_day, days)));
    if (amount > 0) l.mint(owners_.back(), resource(s.kind), amount);
    Lot lot;
    lot.month = month;
    lot.k = static_cast<std::int64_t>(std::llround(mo.k));
    lot.r = mo.r_per_day;
    lot.start_amount = amount;
    lot.start_tick = lot.caught_up = now;
    lots_.push_back(lot);
  }
  next_midnight_ = now;  // the first update catches up and finds the next midnight
}

std::int64_t LivingSystem::amount(const World& w, std::size_t i) const { return w.ledger.balance(owners_[i], resource(setup_.stocks[i].kind)); }

std::int64_t LivingSystem::taken(const World& w, LivingKind k) const { return w.ledger.balance(taker(k), resource(k)); }

void LivingSystem::grow_to(World& w, std::size_t i, std::int64_t to) {
  Lot& lot = lots_[i];
  if (to <= lot.caught_up) return;
  const ResourceId res = resource(setup_.stocks[i].kind);
  // Spec 37 rule 1 with the month's K and r; a K below the stock with no growth is the dormant die-back (rule 3).
  auto at = [&](std::int64_t t) { return Populations::grown(lot.start_amount, lot.k, lot.r, t - lot.start_tick); };
  const std::int64_t before = at(lot.caught_up), after = at(to);
  if (after > before) {
    w.ledger.declare_inflow(to, owners_[i], res, after - before, 0);
  } else if (after < before) {
    const std::int64_t out = std::min(before - after, w.ledger.balance(owners_[i], res));
    if (out > 0) w.ledger.declare_outflow(to, owners_[i], res, out, 0);
  }
  lot.caught_up = to;
}

void LivingSystem::open_month(World& w, std::size_t i, std::int64_t month_abs, std::int64_t at) {
  const LivingStock& s = setup_.stocks[i];
  const living::Month mo = living::month_of(s, growing_[i], setup_.world_seed, month_abs, true);
  const ResourceId res = resource(s.kind);
  const std::int64_t held = w.ledger.balance(owners_[i], res);
  const auto take = std::min(held, static_cast<std::int64_t>(std::llround(static_cast<double>(held) * mo.take_share + mo.take_kg)));
  if (take > 0) w.ledger.transfer(at, owners_[i], taker(s.kind), res, take, 0);
  // Rule 3: dormant grass above its winter share dies back at once.
  const auto k = static_cast<std::int64_t>(std::llround(mo.k));
  if (s.kind == LivingKind::Pasture && mo.r_per_day <= 0 && w.ledger.balance(owners_[i], res) > k)
    w.ledger.declare_outflow(at, owners_[i], res, w.ledger.balance(owners_[i], res) - k, 0);
  Lot& lot = lots_[i];
  lot.month = month_abs;
  lot.k = static_cast<std::int64_t>(std::llround(mo.k));
  lot.r = mo.r_per_day;
  lot.start_amount = w.ledger.balance(owners_[i], res);
  lot.start_tick = lot.caught_up = at;
}

void LivingSystem::advance(World& w, std::int64_t to) {
  for (std::size_t i = 0; i < lots_.size(); ++i) {
    while (lots_[i].caught_up < to) {
      const std::int64_t next = month_start_tick(lots_[i].month + 1);
      if (next > to) {
        grow_to(w, i, to);
        break;
      }
      grow_to(w, i, next);
      open_month(w, i, lots_[i].month + 1, next);
    }
  }
}

void LivingSystem::update(World& w) {
  const std::int64_t now = w.clock.tick;
  if (now < next_midnight_) return;
  advance(w, now);
  // Midnight is 16 hours after tick 0 (08:00), then every day.
  const std::int64_t base = 16 * kTicksPerGameHour;
  next_midnight_ = now < base ? base : base + ((now - base) / kTicksPerGameDay + 1) * kTicksPerGameDay;
}

std::int64_t LivingSystem::next_wake(const World&, std::int64_t now) const { return lots_.empty() ? kNever : std::max(now, next_midnight_); }

void LivingSystem::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type != LivingTake) return;
  if (c.a < 0 || static_cast<std::size_t>(c.a) >= lots_.size()) {
    w.emit(EventType::LivingTaken, c.subject, cause, c.a, -NoStock);
    return;
  }
  const auto i = static_cast<std::size_t>(c.a);
  const auto taker = static_cast<OwnerId>(c.subject);
  if (taker >= w.ledger.owner_names().size()) {
    w.emit(EventType::LivingTaken, c.subject, cause, c.a, -NoTaker);
    return;
  }
  advance(w, w.clock.tick);
  const ResourceId res = resource(setup_.stocks[i].kind);
  const std::int64_t moved = w.ledger.transfer(w.clock.tick, owners_[i], taker, res, std::max<std::int64_t>(0, c.b), cause);
  Lot& lot = lots_[i];  // spec 37 rule 1: a take starts the lot again from what the stock holds
  lot.start_amount = w.ledger.balance(owners_[i], res);
  lot.start_tick = lot.caught_up = w.clock.tick;
  w.emit(EventType::LivingTaken, c.subject, cause, c.a, moved);
}

void LivingSystem::save(ArchiveWriter& a) const {
  a.i64(next_midnight_);
  a.u32(static_cast<std::uint32_t>(lots_.size()));
  for (const Lot& l : lots_) {
    a.i64(l.start_amount);
    a.i64(l.start_tick);
    a.i64(l.caught_up);
    a.i64(l.month);
    a.i64(l.k);
    a.f64(l.r);
  }
}

void LivingSystem::load(ArchiveReader& a) {
  next_midnight_ = a.i64();
  const std::uint32_t n = a.u32();
  if (n != lots_.size()) throw std::runtime_error("living_stocks: the save's stocks are not the setup's");
  for (Lot& l : lots_) {
    l.start_amount = a.i64();
    l.start_tick = a.i64();
    l.caught_up = a.i64();
    l.month = a.i64();
    l.k = a.i64();
    l.r = a.f64();
  }
}

}  // namespace zs
