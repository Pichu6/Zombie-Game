// Spec 57: the testing harness. Each seed twice, perturbed runs against plain stepping, and the negative controls.
#include <doctest.h>

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
std::vector<Config> configs(std::uint64_t seed) {
  return {
      {"stage 6 block", [seed] { return tier0::make_world(seed); }},
      {"stage 8 group", [seed] { return tier0::make_world(seed, true, 4); }},
      {"stage 9 one world", [seed] { return tier0::make_run(seed); }},
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
