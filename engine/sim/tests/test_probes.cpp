// Spec 56: per-system hashes and timers, and the first-divergence comparison.
#include <doctest.h>

#include <memory>
#include <string>
#include <vector>

#include "zs/clock.h"
#include "zs/tier0.h"
#include "zs/world.h"

using namespace zs;

namespace {

// The first (tick, system) at which two worlds' system hashes differ; empty name = none.
std::string first_divergence(const World& a, const World& b) {
  const auto ha = a.system_hashes(), hb = b.system_hashes();
  if (ha.size() != hb.size()) return "system count";
  for (std::size_t i = 0; i < ha.size(); ++i)
    if (ha[i].name != hb[i].name) return "system order: " + ha[i].name + " vs " + hb[i].name;
    else if (ha[i].hash != hb[i].hash) return ha[i].name;
  return "";
}

}  // namespace

TEST_CASE("system hashes: core first, then every system in update order; equal worlds agree, a stepped one differs (A1)") {
  auto a = tier0::make_world(3, true, 4);  // stage 8: the block with the dead and four AI pops
  auto b = tier0::make_world(3, true, 4);
  const auto hashes = a.world->system_hashes();
  REQUIRE(hashes.size() == a.world->systems().size() + 1);
  CHECK(hashes[0].name == "core");
  for (std::size_t i = 0; i < a.world->systems().size(); ++i) CHECK(hashes[i + 1].name == a.world->systems()[i]->name());
  CHECK(first_divergence(*a.world, *b.world).empty());
  a.world->run_ticks(10);
  CHECK(first_divergence(*a.world, *b.world) == "core");  // the clock moved: the core differs first
  b.world->run_ticks(10);
  CHECK(first_divergence(*a.world, *b.world).empty());
}

TEST_CASE("stepping and jumping agree system by system every hour of a day (A1)") {
  auto stepped = tier0::make_world(5, true, 4);
  auto jumped = tier0::make_world(5, true, 4);
  stepped.world->check_every_tick = jumped.world->check_every_tick = false;
  for (int h = 1; h <= 24; ++h) {
    stepped.world->run_ticks(kTicksPerGameHour);
    jumped.world->jump(static_cast<std::int64_t>(h) * kTicksPerGameHour);
    const std::string where = first_divergence(*stepped.world, *jumped.world);
    CHECK_MESSAGE(where.empty(), "hour " << h << ": " << where);
    if (!where.empty()) break;
  }
}

TEST_CASE("save, load into a fresh setup, and the system hashes are the same (A1)") {
  auto a = tier0::make_world(7, true, 4);
  a.world->check_every_tick = false;
  a.world->run_ticks(3 * kTicksPerGameHour);
  const auto bytes = a.world->save();
  auto b = tier0::make_world(7, true, 4);
  b.world->load(bytes);
  CHECK(first_divergence(*a.world, *b.world).empty());
}

TEST_CASE("probes count every update, and time them only when asked; they are not in the hash") {
  auto a = tier0::make_world(9, true, 4);
  a.world->check_every_tick = false;
  const std::uint64_t h0 = a.world->state_hash();
  a.world->run_ticks(100);
  REQUIRE(a.world->probes.size() == a.world->systems().size());
  for (const auto& p : a.world->probes) {
    CHECK(p.updates == 100);
    CHECK(p.ns == 0);
  }
  auto b = tier0::make_world(9, true, 4);
  b.world->check_every_tick = false;
  b.world->probe_timing = true;
  b.world->run_ticks(100);
  std::uint64_t total = 0;
  for (const auto& p : b.world->probes) total += p.ns;
  CHECK(total > 0);
  CHECK(a.world->state_hash() == b.world->state_hash());  // timing changed nothing
  (void)h0;
}
