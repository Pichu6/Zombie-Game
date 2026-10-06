// Spec 28 benchmark: what records, lazy pops and promotion cost. `zs_headless --pop-bench`.
// First run only measures: the targets in the spec are set from these numbers, never the other way.
#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "zs/pop_registry.h"
#include "pop_bench.h"

namespace {

using Clock = std::chrono::steady_clock;
double seconds_since(Clock::time_point t0) { return std::chrono::duration<double>(Clock::now() - t0).count(); }

// Resident memory in bytes, or 0 where we can't read it (only Linux here).
std::uint64_t resident_bytes() {
  std::ifstream f("/proc/self/statm");
  std::uint64_t pages = 0, resident = 0;
  if (!(f >> pages >> resident)) return 0;
  return resident * 4096;
}

zs::PopId random_pop(std::uint64_t salt, std::uint64_t i, std::uint64_t total) {
  return static_cast<zs::PopId>(zs::keyed_u64(7, zs::Purpose::Test, salt, i) % total);
}

// A region of `total` pops in cohorts of about 500 (a workplace or a street), with `effects` aggregate
// events spread over them.
zs::PopRegistry make_registry(std::uint64_t total, std::uint64_t effects) {
  zs::PopRegistry reg(1);
  const std::uint64_t cohort_size = 500;
  std::uint64_t id = 1;
  for (std::uint64_t first = 0; first < total; first += cohort_size, ++id)
    reg.add_cohort({id, static_cast<std::uint32_t>(id % 64), static_cast<std::uint32_t>(first), std::min(cohort_size, total - first)});
  const std::uint64_t cohorts = id - 1;
  for (std::uint64_t e = 0; e < effects; ++e) {
    const std::uint64_t cohort = 1 + zs::keyed_u64(7, zs::Purpose::Test, 1000, e) % cohorts;
    reg.apply({cohort, static_cast<zs::EffectKind>(e % 3), 1, static_cast<double>(e) * 1e-6, e + 1});
  }
  return reg;
}

void b1_records(std::uint64_t n) {
  const std::uint64_t before = resident_bytes();
  zs::PopRegistry reg = make_registry(n, 0);
  const std::uint64_t base = resident_bytes();
  const auto t0 = Clock::now();
  for (std::uint64_t i = 0; i < n; ++i) reg.promote(i);
  const double build = seconds_since(t0);
  const std::uint64_t after = resident_bytes();
  const std::size_t saved = reg.save().size();
  std::printf("B1 records %9llu: build %7.3f s (%5.2f us each), memory %8.1f MB (%5.1f bytes/record, process), save %6.1f MB (%4.1f bytes/record)%s\n",
              static_cast<unsigned long long>(n), build, build * 1e6 / static_cast<double>(n),
              static_cast<double>(after - base) / 1e6, after ? static_cast<double>(after - base) / static_cast<double>(n) : 0.0,
              static_cast<double>(saved) / 1e6, static_cast<double>(saved) / static_cast<double>(n), before ? "" : " (memory n/a)");
}

void b2_lazy(std::uint64_t total, std::uint64_t effects) {
  zs::PopRegistry reg = make_registry(total, effects);
  const std::uint64_t samples = 20000;
  zs::PopSnapshot snap;
  std::uint64_t sink = 0;
  auto t0 = Clock::now();
  for (std::uint64_t i = 0; i < samples; ++i) {
    reg.snapshot(random_pop(1, i, total), 1.0, snap);
    sink += snap.state_bits;
  }
  const double snapshot_us = seconds_since(t0) * 1e6 / static_cast<double>(samples);
  t0 = Clock::now();
  for (std::uint64_t i = 0; i < 200; ++i) sink += reg.count_region(static_cast<std::uint32_t>(i % 64), zs::EffectKind::LaidOff, 1.0);
  const double region_us = seconds_since(t0) * 1e6 / 200.0;
  std::printf("B2 lazy %9llu pops, %6llu effects: one snapshot %9.2f us, one region count %11.2f us%s\n",
              static_cast<unsigned long long>(total), static_cast<unsigned long long>(effects), snapshot_us, region_us,
              sink == 1 ? " " : "");
}

void b3_promotion(std::uint64_t total) {
  zs::PopRegistry reg = make_registry(total, 100);
  const std::uint64_t n = 100000;
  auto t0 = Clock::now();
  for (std::uint64_t i = 0; i < n; ++i) reg.promote(random_pop(2, i, total));
  const double up = seconds_since(t0) * 1e6 / static_cast<double>(n);
  const std::uint64_t made = reg.counts().records;
  t0 = Clock::now();
  for (std::uint64_t i = 0; i < n; ++i) reg.demote(random_pop(2, i, total));
  const double down = seconds_since(t0) * 1e6 / static_cast<double>(n);
  std::printf("B3 promotion over %llu pops: lazy->record %.2f us, record->lazy %.2f us (%llu records made)\n",
              static_cast<unsigned long long>(total), up, down, static_cast<unsigned long long>(made));
}

// B4 and B6: a long step over decades, no outbreak: births and deaths for everyone and their descendants.
void b4_long_step(std::uint64_t founders, double years) {
  zs::PopRegistry reg = make_registry(founders, 0);
  const auto t0 = Clock::now();
  const zs::DemographyCounts c = reg.project_demography(0.0, years);
  const double s = seconds_since(t0);
  const double per_k_year = 1000.0 / (static_cast<double>(c.alive_at_start) * years);
  std::printf("B4 %9llu founders over %2.0f years: %7.3f s (%6.1f game years per real second, %.2f us per person visited, %llu visited)\n",
              static_cast<unsigned long long>(founders), years, s, years / s, s * 1e6 / static_cast<double>(c.people_visited),
              static_cast<unsigned long long>(c.people_visited));
  std::printf("B6   population %llu -> %llu (x%.2f); births %.1f and deaths %.1f per 1000 per year; alive by generation:",
              static_cast<unsigned long long>(c.alive_at_start), static_cast<unsigned long long>(c.alive_at_end),
              static_cast<double>(c.alive_at_end) / static_cast<double>(c.alive_at_start),
              static_cast<double>(c.births) * per_k_year, static_cast<double>(c.deaths) * per_k_year);
  for (int g = 0; g < 8; ++g) std::printf(" %llu", static_cast<unsigned long long>(c.by_generation_alive[g]));
  std::puts("");
}

}  // namespace

int pop_bench() {
  std::puts("Spec 28 benchmark (first run: measures only; build type matters, use release)");
  for (std::uint64_t n : {10000ull, 100000ull, 1000000ull}) b1_records(n);
  for (std::uint64_t effects : {0ull, 100ull, 1000ull, 10000ull}) b2_lazy(1000000, effects);
  b2_lazy(10000000, 100);
  b3_promotion(1000000);
  for (double years : {10.0, 60.0, 150.0}) b4_long_step(100000, years);
  b4_long_step(1000000, 60.0);
  std::puts("B5 embodied: see `zs_headless --bench` (4 pops + 60 walkers for a game day)");
  std::puts("B6 is partial: the realism report against real data is not written yet");
  return 0;
}
