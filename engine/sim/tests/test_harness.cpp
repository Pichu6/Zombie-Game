// Spec 57: the testing harness. Each seed twice, perturbed runs against plain stepping, and the negative controls.
#include <doctest.h>

#include <array>
#include <string>

#include "harness.h"

using namespace zs;
using namespace zs::harness;

namespace {

struct Config {
  const char* name;
  Make make;
};

// The three worlds of the stages: the block alone, the block with the dead and four AI pops, the one world.
// `faults` turns the fault sites on, keyed by the seed: every copy of the world built by the setup sees the same faults.
std::vector<Config> configs(std::uint64_t seed, bool faults = false) {
  auto armed = [seed, faults](tier0::Tier0 t) {
    if (faults) t.world->buggify.seed = 0xFA17ull + seed;
    return t;
  };
  return {
      {"stage 6 block", [=] { return armed(tier0::make_world(seed)); }},
      {"stage 8 group", [=] { return armed(tier0::make_world(seed, true, 4)); }},
      {"stage 9 one world", [=] { return armed(tier0::make_run(seed)); }},
  };
}

constexpr int kHours = 24;

}  // namespace

TEST_CASE("each seed run twice gives the same system hashes every hour (A1)" * doctest::skip(World::kDebugBuild)) {
  for (std::uint64_t seed : {1, 2}) {
    for (const Config& c : configs(seed)) {
      const Oracle a = plain_run(c.make, kHours), b = plain_run(c.make, kHours);
      REQUIRE(a.hours.size() == b.hours.size());
      for (std::size_t h = 0; h < a.hours.size(); ++h)
        for (std::size_t i = 0; i < a.hours[h].size(); ++i)
          CHECK_MESSAGE(a.hours[h][i].hash == b.hours[h][i].hash,
                        c.name << " seed " << seed << " hour " << h + 1 << " system " << a.hours[h][i].name);
      CHECK(a.final_hash == b.final_hash);
    }
  }
}

TEST_CASE("perturbed runs (steps, jumps, save and load into a fresh setup) equal plain stepping (A1)" *
          doctest::skip(World::kDebugBuild)) {
  for (const Config& c : configs(3)) {
    const Oracle oracle = plain_run(c.make, kHours);
    for (std::uint64_t plan : {11, 12, 13, 14, 15, 16}) {
      const Report r = perturbed_run(c.make, oracle, plan, kHours);
      CHECK_MESSAGE(r.agree, c.name << ", plan " << plan << ": first difference at hour " << r.hour << " in "
                                    << r.system << " after: " << r.ops);
      CHECK(r.jumps + r.saves + r.steps > kHours);  // the plan did more than one operation per hour
    }
  }
}

TEST_CASE("the harness catches a tick too many and a corrupted save (negative controls)" *
          doctest::skip(World::kDebugBuild)) {
  const Config c = configs(4)[1];
  const Oracle oracle = plain_run(c.make, 2);
  const Report extra = perturbed_run(c.make, oracle, 21, 2, Fault::ExtraTick);
  CHECK_FALSE(extra.agree);
  CHECK(extra.system == "core");  // the clock moved: the core differs first
  CHECK(!extra.ops.empty());
  bool caught = false;
  for (std::uint64_t plan = 30; plan < 40 && !caught; ++plan)  // a plan with a save and load at or after op 3
    caught = !perturbed_run(c.make, oracle, plan, 2, Fault::CorruptSave).agree;
  CHECK(caught);
}

TEST_CASE("plans are reproducible: the same plan seed does the same operations") {
  const Config c = configs(5)[0];
  const Oracle oracle = plain_run(c.make, 1);
  const Report a = perturbed_run(c.make, oracle, 77, 1), b = perturbed_run(c.make, oracle, 77, 1);
  CHECK(a.agree);
  CHECK(a.steps == b.steps);
  CHECK(a.jumps == b.jumps);
  CHECK(a.saves == b.saves);
}

TEST_CASE("with fault sites on, the three promises still hold, over six days (A1)" * doctest::skip(World::kDebugBuild)) {
  constexpr int kDays = 6 * 24;  // the grid fails somewhere in days 2 to 5 (spec 17 rule 5): the generator branch runs
  std::array<std::uint64_t, static_cast<std::size_t>(FaultSite::kCount)> fired{};
  for (const Config& c : configs(6, true)) {
    const Oracle oracle = plain_run(c.make, kDays);
    for (std::size_t s = 0; s < fired.size(); ++s) fired[s] += oracle.hits[s];
    for (std::uint64_t plan : {51, 52, 53}) {
      const Report r = perturbed_run(c.make, oracle, plan, kDays);
      CHECK_MESSAGE(r.agree, c.name << ", plan " << plan << ": first difference at hour " << r.hour << " in "
                                    << r.system << " after: " << r.ops);
    }
  }
  // A site that never fires tests nothing: every site fired somewhere in the worlds.
  for (std::size_t s = 0; s < fired.size(); ++s) CHECK_MESSAGE(fired[s] > 0, "fault site " << s << " never fired");
}

TEST_CASE("fault sites change the run, and a world without a seed never fires one (A1)" * doctest::skip(World::kDebugBuild)) {
  const Config plain = configs(7, false)[1], faulty = configs(7, true)[1];
  const Oracle a = plain_run(plain.make, 72), b = plain_run(faulty.make, 72);
  CHECK(a.final_hash != b.final_hash);  // the faults changed what happened
  for (std::uint64_t n : a.hits) CHECK(n == 0);
  std::uint64_t total = 0;
  for (std::uint64_t n : b.hits) total += n;
  CHECK(total > 0);
  CHECK(b.final_hash == plain_run(faulty.make, 72).final_hash);  // and the same faults give the same run
}
