// Spec 103: living stocks in the town's region: months, the exact jump across months, equilibrium, seasons, the harvest
// following the rain, the take command, save and load.
#include <doctest.h>

#include <cmath>
#include <memory>
#include <vector>

#include "zs/anomalies.h"
#include "zs/calendar.h"
#include "zs/clock.h"
#include "zs/living_stocks.h"

using namespace zs;

namespace {

// A temperate cell (10 C, 20 C between the coldest and warmest month, 800 mm), with or without the year's anomalies.
fields::Climate temperate(bool anomalies) {
  fields::Climate c{10.0, 20.0, 800.0};
  c.anomalies = anomalies;
  c.lat_deg = 45, c.lon_deg = 10;
  return c;
}

LivingSetup region(bool anomalies) {
  LivingSetup s;
  s.world_seed = 7;
  const fields::Climate c = temperate(anomalies);
  s.stocks.push_back({1, LivingKind::Game, c, 400'000, 0.25, 0.1});
  s.stocks.push_back({1, LivingKind::Wood, c, 3'000'000'000, 0.04, 0.01});
  s.stocks.push_back({1, LivingKind::Pasture, c, 50'000'000, 0, 0.25});
  s.stocks.push_back({1, LivingKind::Crops, c, 80'000'000, 0, 0});
  s.stocks.push_back({2, LivingKind::Fish, c, 2'000'000, 0.5, 0.25});
  return s;
}

struct Made {
  std::unique_ptr<World> w;
  LivingSystem* sys = nullptr;
};

Made make(bool anomalies = true) {
  Made m;
  m.w = std::make_unique<World>(11);
  auto sys = std::make_unique<LivingSystem>(region(anomalies), kDefaultStartDay);
  sys->setup(*m.w);
  m.sys = sys.get();
  m.w->add_system(std::move(sys));
  m.w->close_setup();
  return m;
}

std::vector<std::int64_t> balances(const Made& m) {
  std::vector<std::int64_t> b;
  for (std::size_t i = 0; i < m.sys->size(); ++i) b.push_back(m.sys->amount(*m.w, i));
  for (int k = 0; k < kLivingKinds; ++k) b.push_back(m.sys->taken(*m.w, static_cast<LivingKind>(k)));
  return b;
}

}  // namespace

TEST_CASE("living stocks: growing and harvest months from a climate's normals (rule 2)") {
  const auto g = living::growing_months(temperate(false));
  CHECK(!g[0]);   // January about 0 C
  CHECK(g[6]);    // July about 20 C with 67 mm
  CHECK(!g[11]);
  const auto h = living::harvest_months(g);
  int harvests = 0;
  for (int m = 0; m < 12; ++m) harvests += h[static_cast<std::size_t>(m)] ? 1 : 0;
  CHECK(harvests == 1);
  fields::Climate dry{25.0, 10.0, 200.0};  // 17 mm a month under 2 x 25: nothing grows
  for (bool b : living::growing_months(dry)) CHECK(!b);
  std::array<bool, 12> all{};
  all.fill(true);
  const auto h2 = living::harvest_months(all);
  CHECK((h2[0] && h2[6]));
}

TEST_CASE("living stocks: stepping by the day and one jump across months give identical balances (rule 3)") {
  Made a = make(), b = make();
  const std::int64_t to = 400 * kTicksPerGameDay + 1234;
  a.sys->advance(*a.w, to);
  for (std::int64_t t = 0; t < to; t += kTicksPerGameDay - 77) b.sys->advance(*b.w, std::min(t, to));
  b.sys->advance(*b.w, to);
  CHECK(balances(a) == balances(b));
  CHECK(!a.w->ledger.check().has_value());
  for (int k = 0; k < kLivingKinds; ++k) CHECK(a.sys->taken(*a.w, static_cast<LivingKind>(k)) > 0);
}

TEST_CASE("living stocks: the normal year returns to its equilibrium; grass by the season; one harvest a year (rules 3, 5, 6)") {
  Made m = make(false);
  const std::vector<std::int64_t> start = balances(m);
  const std::int64_t twenty = m.sys->month_start_tick(m.sys->month_at(0) + 20 * 12) + (0 - m.sys->month_start_tick(m.sys->month_at(0)));
  m.sys->advance(*m.w, twenty);
  for (std::size_t i = 0; i < m.sys->size(); ++i) {
    const double s = static_cast<double>(start[i]), now = static_cast<double>(m.sys->amount(*m.w, i));
    CHECK_MESSAGE(std::abs(now - s) <= 0.01 * s + 2, living_kind_name(m.sys->living().stocks[i].kind));
  }
  const auto pasture = living::equilibrium(region(false).stocks[2]);
  CHECK(pasture[6] > 1.5 * pasture[0]);
  // The crop: everything taken in a year is one harvest, 90 % of the standing crop that month.
  Made c = make(false);
  const std::int64_t jan = c.sys->month_at(0) + 12;
  c.sys->advance(*c.w, c.sys->month_start_tick(jan));
  int months_with_harvest = 0;
  std::int64_t harvest = 0, standing = 0;
  for (std::int64_t mo = jan; mo < jan + 12; ++mo) {
    const std::int64_t t0 = c.sys->taken(*c.w, LivingKind::Crops);
    const std::int64_t at = c.sys->month_start_tick(mo + 1);
    c.sys->advance(*c.w, at - 1);
    const std::int64_t held = c.sys->amount(*c.w, 3);
    c.sys->advance(*c.w, at);
    const std::int64_t got = c.sys->taken(*c.w, LivingKind::Crops) - t0;
    if (got > 0) {
      ++months_with_harvest;
      harvest = got;
      standing = held;
    }
  }
  CHECK(months_with_harvest == 1);
  CHECK(std::abs(static_cast<double>(harvest) - 0.9 * static_cast<double>(standing)) <= 0.001 * static_cast<double>(standing) + 2);
}

TEST_CASE("living stocks: the harvest follows the growing season's rain (rule 4)") {
  Made m = make(true);
  const fields::Climate c = temperate(true);
  const auto g = living::growing_months(c);
  const std::int64_t first = m.sys->month_at(0) / 12 + 1;
  std::vector<double> rain, crop;
  m.sys->advance(*m.w, m.sys->month_start_tick(first * 12));
  for (std::int64_t y = first; y < first + 40; ++y) {
    const std::int64_t t0 = m.sys->taken(*m.w, LivingKind::Crops);
    m.sys->advance(*m.w, m.sys->month_start_tick((y + 1) * 12));
    crop.push_back(static_cast<double>(m.sys->taken(*m.w, LivingKind::Crops) - t0));
    double f = 0;
    int n = 0;
    for (int mo = 0; mo < 12; ++mo)
      if (g[static_cast<std::size_t>(mo)]) {
        const std::int64_t abs = y * 12 + mo;
        f += 0.5 * (fields::rain_month_factor(7, c, abs) + fields::rain_month_factor(7, c, abs - 1)), ++n;
      }
    rain.push_back(f / n);
  }
  auto mean = [](const std::vector<double>& v) {
    double s = 0;
    for (double x : v) s += x;
    return s / static_cast<double>(v.size());
  };
  const double mr = mean(rain), mc = mean(crop);
  double sxy = 0, sxx = 0, syy = 0;
  for (std::size_t i = 0; i < rain.size(); ++i) {
    sxy += (rain[i] - mr) * (crop[i] - mc);
    sxx += (rain[i] - mr) * (rain[i] - mr);
    syy += (crop[i] - mc) * (crop[i] - mc);
  }
  const double corr = sxy / std::sqrt(sxx * syy);
  MESSAGE("rain-harvest correlation " << corr);
  CHECK(corr > 0.5);
}

TEST_CASE("living stocks: a take moves at most what a stock holds; refusals; save and load continue the same (rule 7)") {
  Made m = make();
  const OwnerId hunter = m.w->ledger.add_owner("hunter");
  m.w->jump(10 * kTicksPerGameDay);
  const std::int64_t held = m.sys->amount(*m.w, 0);
  m.w->submit({m.w->clock.tick, LivingTake, hunter, 0, held * 10});
  m.w->submit({m.w->clock.tick, LivingTake, hunter, 99, 5});
  m.w->step();
  CHECK(m.w->ledger.balance(hunter, m.sys->resource(LivingKind::Game)) >= held);
  int taken = 0, refused = 0;
  for (const Event& e : m.w->log.all())
    if (e.type == EventType::LivingTaken) (e.b < 0 ? refused : taken)++;
  CHECK(taken == 1);
  CHECK(refused == 1);
  CHECK(!m.w->ledger.check().has_value());

  Made a = make();
  a.w->jump(100 * kTicksPerGameDay + 5);
  const auto bytes = a.w->save();
  Made b = make();
  b.w->load(bytes);
  CHECK(b.w->save() == bytes);
  a.w->jump(300 * kTicksPerGameDay);
  b.w->jump(300 * kTicksPerGameDay);
  CHECK(a.w->state_hash() == b.w->state_hash());
  CHECK(balances(a) == balances(b));
}
