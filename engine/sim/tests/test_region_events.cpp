// Spec 28: region-wide events split down to rosters; the exact hypergeometric draw behind them.
#include <doctest.h>

#include <cmath>
#include <map>
#include <vector>

#include "zs/hypergeometric.h"
#include "zs/pop_system.h"

using namespace zs;

namespace {
double u_at(std::uint64_t a, std::uint64_t b) { return keyed_uniform01(77, Purpose::Test, a, b); }

// A region with workplaces of different sizes: rosters 1..n in region `region`.
PopRegistry make_region(std::uint64_t seed, std::uint32_t rosters, PopId& next, std::uint32_t region = 0) {
  PopRegistry reg(seed);
  reg.add_cohort({1, 0, 0, 100000});
  for (std::uint32_t r = 1; r <= rosters; ++r) {
    reg.add_roster(r, region);
    const std::uint64_t size = 20 + (r * 37) % 180;
    for (std::uint64_t i = 0; i < size; ++i) reg.roster_join(r, next++, 0.0);
  }
  return reg;
}
double exact_pmf(std::uint64_t N, std::uint64_t K, std::uint64_t n, std::uint64_t k) {
  return std::exp(log_factorial(K) - log_factorial(k) - log_factorial(K - k) + log_factorial(N - K) - log_factorial(n - k) -
                  log_factorial(N - K - (n - k)) - (log_factorial(N) - log_factorial(n) - log_factorial(N - n)));
}
}  // namespace

TEST_CASE("log factorial: table and Stirling agree across the join, and match exact small values (A1)") {
  CHECK(log_factorial(0) == 0.0);
  CHECK(log_factorial(1) == 0.0);
  CHECK(log_factorial(5) == doctest::Approx(std::log(120.0)).epsilon(1e-12));
  CHECK(log_factorial(20) == doctest::Approx(std::log(2432902008176640000.0)).epsilon(1e-12));
  // The step from the table to the series is the log of the next integer.
  CHECK(log_factorial(1025) - log_factorial(1024) == doctest::Approx(std::log(1025.0)).epsilon(1e-9));
  CHECK(log_factorial(100000) - log_factorial(99999) == doctest::Approx(std::log(100000.0)).epsilon(1e-9));
}

TEST_CASE("hypergeometric: bounds, edge cases and purity (A1)") {
  for (int i = 0; i < 2000; ++i) {
    const std::uint64_t N = 1 + static_cast<std::uint64_t>(u_at(1, i) * 500);
    const std::uint64_t K = static_cast<std::uint64_t>(u_at(2, i) * (N + 1)) % (N + 1);
    const std::uint64_t n = static_cast<std::uint64_t>(u_at(3, i) * (N + 1)) % (N + 1);
    const std::uint64_t k = hypergeometric(u_at(4, i), N, K, n);
    CHECK(k <= std::min(n, K));
    CHECK(k + (N - K) >= n);
    CHECK(k == hypergeometric(u_at(4, i), N, K, n));
  }
  CHECK(hypergeometric(0.5, 100, 0, 30) == 0);      // nothing marked
  CHECK(hypergeometric(0.5, 100, 40, 0) == 0);      // nothing drawn
  CHECK(hypergeometric(0.5, 100, 40, 100) == 40);   // everything drawn
  CHECK(hypergeometric(0.5, 100, 100, 30) == 30);   // everything marked
  CHECK(hypergeometric(0.99999, 10, 4, 10) == 4);
}

TEST_CASE("hypergeometric: the distribution matches the exact probabilities (A2)") {
  const std::uint64_t N = 50, K = 20, n = 15;
  const int samples = 200000;
  std::map<std::uint64_t, int> seen;
  for (int i = 0; i < samples; ++i) ++seen[hypergeometric(u_at(9, i), N, K, n)];
  for (std::uint64_t k = 0; k <= std::min(n, K); ++k) {
    const double p = exact_pmf(N, K, n, k);
    const double expect = p * samples, sd = std::sqrt(samples * p * (1 - p));
    CHECK(std::fabs(seen[k] - expect) < 5 * sd + 3);
  }
}

TEST_CASE("hypergeometric: mean and spread are right for big populations (A2)") {
  const std::uint64_t N = 10000000, K = 3000000, n = 1000000;
  const int samples = 2000;
  double sum = 0, sum2 = 0;
  for (int i = 0; i < samples; ++i) { const double k = static_cast<double>(hypergeometric(u_at(11, i), N, K, n)); sum += k; sum2 += k * k; }
  const double mean = sum / samples, var = sum2 / samples - mean * mean;
  const double want_mean = static_cast<double>(n) * K / N;
  const double want_var = want_mean * (1.0 - static_cast<double>(K) / N) * static_cast<double>(N - n) / static_cast<double>(N - 1);
  CHECK(std::fabs(mean - want_mean) < 5 * std::sqrt(want_var / samples));
  CHECK(var > 0.85 * want_var);
  CHECK(var < 1.15 * want_var);
}

TEST_CASE("a region event hits exactly the count asked, across its rosters, and nobody else (A1)") {
  PopId next = 0;
  PopRegistry reg = make_region(5, 30, next);
  REQUIRE(reg.add_roster(100, 1));  // another region
  for (PopId p = next; p < next + 50; ++p) REQUIRE(reg.roster_join(100, p, 0.0));
  const std::uint64_t eligible = [&] { std::uint64_t n = 0; for (std::uint32_t r = 1; r <= 30; ++r) n += reg.roster(r)->size(0.5); return n; }();
  const std::int64_t k = static_cast<std::int64_t>(eligible * 3 / 20);  // 15%
  REQUIRE(reg.region_apply(0, EffectKind::LaidOff, k, 1.0, 9));
  CHECK(reg.roster_count_region(0, EffectKind::LaidOff, 1.5) == static_cast<std::uint64_t>(k));
  CHECK(reg.roster_count_region(1, EffectKind::LaidOff, 1.5) == 0);
  std::uint64_t found = 0;
  for (PopId p = 0; p < next + 50; ++p) { PopSnapshot s; REQUIRE(reg.snapshot(p, 1.5, s)); found += s.roster_bits & 1u; }
  CHECK(found == static_cast<std::uint64_t>(k));
  for (std::uint32_t r = 1; r <= 30; ++r) CHECK(reg.roster(r)->count(EffectKind::LaidOff, 1.5) <= reg.roster(r)->size(1.5));
  CHECK(reg.audit().has_value() == false);
}

TEST_CASE("a region event never hits anyone twice: two halves, then nothing is left (A1)") {
  PopId next = 0;
  PopRegistry reg = make_region(6, 12, next);
  std::uint64_t eligible = 0;
  for (std::uint32_t r = 1; r <= 12; ++r) eligible += reg.roster(r)->size(0.5);
  const std::int64_t half = static_cast<std::int64_t>(eligible / 2);
  REQUIRE(reg.region_apply(0, EffectKind::Ill, half, 1.0, 1));
  REQUIRE(reg.region_apply(0, EffectKind::Ill, static_cast<std::int64_t>(eligible) - half, 2.0, 2));
  CHECK(reg.roster_count_region(0, EffectKind::Ill, 2.5) == eligible);
  CHECK_FALSE(reg.region_apply(0, EffectKind::Ill, 1, 3.0, 3));  // everyone is already in the state
  for (std::uint32_t r = 1; r <= 12; ++r) CHECK(reg.roster(r)->count(EffectKind::Ill, 2.5) == reg.roster(r)->size(2.5));
}

TEST_CASE("a region give-back frees exactly that many and keeps every count consistent (A1)") {
  PopId next = 0;
  PopRegistry reg = make_region(7, 20, next);
  REQUIRE(reg.region_apply(0, EffectKind::Moved, 800, 1.0, 1));
  REQUIRE(reg.region_apply(0, EffectKind::Moved, -300, 2.0, 2));
  CHECK(reg.roster_count_region(0, EffectKind::Moved, 2.5) == 500);
  CHECK(reg.roster_count_region(0, EffectKind::Moved, 1.5) == 800);  // the past is unchanged
  CHECK_FALSE(reg.region_apply(0, EffectKind::Moved, -501, 3.0, 3));
  CHECK(reg.region_apply(0, EffectKind::Moved, -500, 3.0, 3));
  CHECK(reg.roster_count_region(0, EffectKind::Moved, 3.5) == 0);
}

TEST_CASE("refusals change nothing: too many, no such region, going back in time, zero (A1)") {
  PopId next = 0;
  PopRegistry reg = make_region(8, 10, next);
  REQUIRE(reg.region_apply(0, EffectKind::LaidOff, 100, 2.0, 1));
  const auto before = reg.save();
  CHECK_FALSE(reg.region_apply(0, EffectKind::LaidOff, 100000, 3.0, 2));   // more than there are
  CHECK_FALSE(reg.region_apply(9, EffectKind::LaidOff, 1, 3.0, 2));        // no rosters there
  CHECK_FALSE(reg.region_apply(0, EffectKind::LaidOff, 1, 1.0, 2));        // before a roster's last year
  CHECK_FALSE(reg.region_apply(0, EffectKind::LaidOff, 0, 3.0, 2));        // nothing
  CHECK_FALSE(reg.region_apply(0, EffectKind::LaidOff, -101, 3.0, 2));     // more than are in the state
  CHECK(reg.save() == before);
}

TEST_CASE("the split is proportional to what each roster has, on average (A3)") {
  // Two workplaces, one three times the size of the other: over many events it gets about three times the hits.
  std::uint64_t big = 0, small = 0;
  for (int trial = 0; trial < 300; ++trial) {
    PopRegistry reg(1000 + trial);
    reg.add_cohort({1, 0, 0, 1000});
    reg.add_roster(1, 0); reg.add_roster(2, 0);
    for (PopId p = 0; p < 300; ++p) reg.roster_join(1, p, 0.0);
    for (PopId p = 300; p < 400; ++p) reg.roster_join(2, p, 0.0);
    REQUIRE(reg.region_apply(0, EffectKind::LaidOff, 40, 1.0, 1));
    big += reg.roster(1)->count(EffectKind::LaidOff, 1.5);
    small += reg.roster(2)->count(EffectKind::LaidOff, 1.5);
  }
  CHECK(big + small == 300u * 40u);
  CHECK(static_cast<double>(big) / static_cast<double>(small) > 2.7);
  CHECK(static_cast<double>(big) / static_cast<double>(small) < 3.3);
}

TEST_CASE("the same history gives the same split; successive events split differently; a save keeps the counter (A1)") {
  const auto build = [] {
    PopId next = 0;
    PopRegistry reg = make_region(9, 15, next);
    reg.region_apply(0, EffectKind::LaidOff, 200, 1.0, 1);
    reg.region_apply(0, EffectKind::LaidOff, 200, 2.0, 2);
    return reg;
  };
  PopRegistry a = build(), b = build();
  CHECK(a.save() == b.save());
  // Event 1 and event 2 split their 200 differently across rosters (the counter is part of the key).
  bool differs = false;
  for (std::uint32_t r = 1; r <= 15; ++r)
    differs = differs || a.roster(r)->count(EffectKind::LaidOff, 1.5) * 2 != a.roster(r)->count(EffectKind::LaidOff, 2.5);
  CHECK(differs);
  PopRegistry loaded(9);
  loaded.load(a.save());
  CHECK(loaded.save() == a.save());
  // After a load, the next event splits as it would have without the pause.
  a.region_apply(0, EffectKind::Ill, 100, 3.0, 3);
  loaded.region_apply(0, EffectKind::Ill, 100, 3.0, 3);
  CHECK(loaded.save() == a.save());
}

TEST_CASE("a region event is a world command: its reason is the order, and replay reproduces it (A1)") {
  const auto make = [] {
    auto w = std::make_unique<World>(12);
    auto pops = std::make_unique<PopSystem>(*w);
    PopRegistry& reg = pops->setup(*w);
    reg.add_cohort({1, 0, 0, 5000});
    PopId next = 0;
    for (std::uint32_t r = 1; r <= 25; ++r) {
      reg.add_roster(r, 3);
      for (std::uint64_t i = 0; i < 40; ++i) reg.roster_join(r, next++, 0.0);
    }
    w->add_system(std::move(pops));
    w->close_setup();
    w->check_every_tick = true;
    return w;
  };
  auto w = make();
  w->run_ticks(kTicksPerGameDay);
  w->submit({0, RegionApply, 3, static_cast<std::int64_t>(EffectKind::LaidOff), 120});
  w->run_ticks(kTicksPerGameDay);
  const auto& reg = w->system<PopSystem>()->registry();
  CHECK(reg.roster_count_region(3, EffectKind::LaidOff, 1.0) == 120);
  std::uint64_t reason = 0;
  for (std::uint32_t r = 1; r <= 25 && !reason; ++r)
    for (PopId p : reg.roster(r)->in_state_members(EffectKind::LaidOff, 1.0)) { reason = reg.roster(r)->reason_of(p, EffectKind::LaidOff, 1.0); break; }
  REQUIRE(reason != 0);
  CHECK(w->log.get(reason)->type == EventType::Intent);
  CHECK(w->log.get(reason)->a == RegionApply);
  // Replay.
  auto r = make();
  std::size_t next = 0;
  for (std::int64_t i = 0; i < 2 * kTicksPerGameDay; ++i) {
    while (next < w->input_log().size() && w->input_log()[next].tick == r->clock.tick) r->submit(w->input_log()[next++]);
    r->step();
  }
  CHECK(r->state_hash() == w->state_hash());
}
