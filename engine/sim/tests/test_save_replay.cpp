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
  CHECK(w->state_hash() == 16535670403649575214ULL);  // re-pinned on MSVC for decision 0065 (ids per creator; the draws are keyed by id), 2026-10-11
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

// Spec 06 rules 16 and 17: format version 6 carries the codec and the uncompressed size; the state is zstd by default.
TEST_CASE("version 6: both codecs load to the same state, zstd is smaller, hashes are over the uncompressed state (A1)") {
  auto w = run(3, 2 * kTicksPerGameDay);
  const auto raw = w->save(SaveCodec::None);
  const auto packed = w->save();  // zstd level 3
  CHECK(packed.size() < raw.size());
  CHECK(packed == w->save());  // the same state gives the same compressed bytes
  for (const auto* bytes : {&raw, &packed}) {
    auto c = demo::make_world(3);
    c->load(*bytes);
    CHECK(c->state_hash() == w->state_hash());
    CHECK(c->save() == packed);
  }
  for (int level : {1, 3, 19}) {
    auto c = demo::make_world(3);
    c->load(w->save(SaveCodec::Zstd, level));
    CHECK(c->state_hash() == w->state_hash());
  }
}

TEST_CASE("version 6: a save damaged anywhere, or lying about its size or codec, is refused (A1)") {
  auto w = run(2, 3000);
  const auto good = w->save();
  auto refuses = [&](std::vector<std::uint8_t> bytes) {
    auto c = demo::make_world(2);
    try {
      c->load(bytes);
    } catch (const std::exception&) {
      return true;
    }
    return false;
  };
  CHECK_FALSE(refuses(good));
  for (std::size_t i : {std::size_t{20}, std::size_t{28}, good.size() / 2, good.size() - 1}) {  // header hash, frame start, middle, checksum
    CAPTURE(i);
    auto bad = good;
    bad[i] ^= 0x01;
    CHECK(refuses(bad));
  }
  auto huge = good;  // the uncompressed size claims 2^40 bytes: refused before any allocation
  huge[19] = 0x01;
  CHECK(refuses(huge));
  auto wrong_size = good;  // the size differs from the frame's own content size
  wrong_size[12] ^= 0x01;
  CHECK(refuses(wrong_size));
  auto codec = good;
  codec[8] = 7;  // an unknown codec
  CHECK(refuses(codec));
  CHECK(refuses({good.begin(), good.begin() + 20}));  // cut inside the header
  CHECK(refuses({good.begin(), good.end() - 3}));      // cut inside the frame
}

TEST_CASE("the version 5 save still loads (A1)") {
  // fixtures/save_v5.zsav: demo seed 1, one game day, written by zs --days 1 --save with the build of version 5.
  auto w = demo::make_world(1);
  w->load(read_file(std::string(ZS_FIXTURES_DIR) + "/save_v5.zsav"));
  CHECK(w->clock.tick == kTicksPerGameDay);
  CHECK(w->state_hash() == run(1, kTicksPerGameDay)->state_hash());
}
