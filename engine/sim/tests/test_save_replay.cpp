// Spec 06: save, load, replay and the event log.
#include <doctest.h>

#include "zs/demo.h"

using namespace zs;

namespace {
std::unique_ptr<World> run(std::uint64_t seed, std::int64_t ticks) {
  auto w = demo::make_world(seed);
  w->check_every_tick = true;
  for (std::int64_t i = 0; i < ticks; ++i) {
    demo::scripted_player(*w);
    w->step();
  }
  return w;
}
// Replays an input log into a fresh world: start state + seed + inputs.
std::unique_ptr<World> replay(std::uint64_t seed, const std::vector<Command>& inputs, std::int64_t ticks) {
  auto w = demo::make_world(seed);
  std::size_t next = 0;
  for (std::int64_t i = 0; i < ticks; ++i) {
    while (next < inputs.size() && inputs[next].tick == w->clock.tick) w->submit(inputs[next++]);
    w->step();
  }
  return w;
}
}  // namespace

TEST_CASE("save -> load -> save gives identical bytes (A1)") {
  auto w = run(3, 2 * kTicksPerGameDay + 123);
  const auto bytes = w->save();
  auto candidate = demo::make_world(3);
  candidate->load(bytes);
  CHECK(candidate->save() == bytes);
}

TEST_CASE("replaying the input log reproduces the per-day hashes (A1)") {
  auto w = run(4, 4 * kTicksPerGameDay);
  auto r = replay(4, w->input_log(), 4 * kTicksPerGameDay);
  CHECK(r->state_hash() == w->state_hash());
}

TEST_CASE("load at day 2 and play on matches the uninterrupted run (A1)") {
  auto straight = run(6, 4 * kTicksPerGameDay);
  auto first = run(6, 2 * kTicksPerGameDay);
  auto resumed = demo::make_world(6);
  resumed->load(first->save());
  for (std::int64_t i = 0; i < 2 * kTicksPerGameDay; ++i) {
    demo::scripted_player(*resumed);
    resumed->step();
  }
  CHECK(resumed->state_hash() == straight->state_hash());
}

TEST_CASE("golden hash of the scripted demo run (A1; pinned on Linux, must match on Windows)") {
  auto w = run(1, 14 * kTicksPerGameDay);
  CHECK(w->state_hash() == 0xdb28323507a4a3aeULL);
}

TEST_CASE("every saved format version still loads (A1)") {
  // fixtures/save_vN.zsav: demo seed 1, one game day, written by zs --days 1 --save
  // with the build of that format version.
  for (const char* file : {"/save_v1.zsav", "/save_v2.zsav", "/save_v3.zsav"}) {
    CAPTURE(file);
    auto w = demo::make_world(1);
    w->load(read_file(std::string(ZS_FIXTURES_DIR) + file));
    CHECK(w->clock.tick == kTicksPerGameDay);
    CHECK(w->state_hash() == run(1, kTicksPerGameDay)->state_hash());
  }
}

TEST_CASE("a corrupt or unknown save is refused") {
  auto bytes = run(2, 100)->save();
  auto w = demo::make_world(2);
  auto bad = bytes;
  bad.back() ^= 0xff;
  CHECK_THROWS(w->load(bad));
  auto future = bytes;
  future[4] = 99;  // format version
  CHECK_THROWS(demo::make_world(2)->load(future));
}

TEST_CASE("the ledger check catches a violation") {
  auto w = demo::make_world(1);
  w->check_every_tick = true;
  w->ledger.restore({{{0, 0}, -5}}, {0, 0}, false);
  CHECK_THROWS(w->step());
  REQUIRE(w->last_violation.has_value());
}
