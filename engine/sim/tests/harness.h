// The testing harness (spec 57, ranked item 4): a seeded plan of perturbations (step, jump, save and load into a fresh
// setup, probe timing on and off) applied to a run, compared system by system at every hour against the same run
// stepped tick by tick. A perturbed run must equal plain stepping; when it does not, the report names the first hour, the
// first system and the operations since the last agreement, and the seed that makes the plan.
// Research: docs/research/hordes-sound-sky-and-testing.md HST-104..HST-118 (FoundationDB, TigerBeetle).
#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <exception>
#include <sstream>
#include <string>
#include <vector>

#include "zs/clock.h"
#include "zs/tier0.h"
#include "zs/world.h"

namespace zs::harness {

using Make = std::function<tier0::Tier0()>;  // builds the world under test, the same every call

// SplitMix64: the plan's own generator, so a plan is the same on every compiler.
struct PlanRng {
  std::uint64_t s;
  std::uint64_t next() {
    std::uint64_t z = (s += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
  }
  std::int64_t below(std::int64_t n) { return static_cast<std::int64_t>(next() % static_cast<std::uint64_t>(n)); }
};

inline std::string first_divergence(const World& a, const World& b) {
  const auto ha = a.system_hashes(), hb = b.system_hashes();
  if (ha.size() != hb.size()) return "system count";
  for (std::size_t i = 0; i < ha.size(); ++i) {
    if (ha[i].name != hb[i].name) return "system order: " + ha[i].name + " vs " + hb[i].name;
    if (ha[i].hash != hb[i].hash) return ha[i].name;
  }
  return "";
}

// The oracle: the world stepped tick by tick, its system hashes recorded at every hour boundary.
struct Oracle {
  std::vector<std::vector<World::SystemHash>> hours;  // [h - 1] = the hashes after h hours
  std::uint64_t final_hash = 0;
  std::array<std::uint64_t, static_cast<std::size_t>(FaultSite::kCount)> hits{};  // fault sites fired
};

inline Oracle plain_run(const Make& make, int hours) {
  auto t = make();
  t.world->check_every_tick = false;
  Oracle o;
  for (int h = 1; h <= hours; ++h) {
    t.world->run_ticks(kTicksPerGameHour);
    o.hours.push_back(t.world->system_hashes());
  }
  o.final_hash = t.world->state_hash();
  o.hits = t.world->buggify.hits;
  return o;
}

enum class Fault { None, ExtraTick, CorruptSave };  // negative controls: the harness must catch these

struct Report {
  bool agree = true;
  int hour = 0;                 // the first hour at which the perturbed run differed
  std::string system;           // and the first system that did
  std::string ops;              // the operations since the last agreement, for the failure message
  int saves = 0, jumps = 0, steps = 0;  // what the plan did
  std::array<std::uint64_t, static_cast<std::size_t>(FaultSite::kCount)> hits{};  // fault sites fired, over every world of the run
};

// A perturbed run of `hours` game hours under the plan keyed by `plan_seed`. Segments never cross an hour boundary,
// so the comparison with the oracle is at the same tick. `fault` injects a known bug at the third operation.
inline Report perturbed_run(const Make& make, const Oracle& oracle, std::uint64_t plan_seed, int hours,
                            Fault fault = Fault::None) {
  Report r;
  PlanRng rng{plan_seed};
  auto t = make();
  t.world->check_every_tick = false;
  std::ostringstream ops;
  int op_count = 0;
  for (int h = 1; h <= hours; ++h) {
    const std::int64_t hour_end = static_cast<std::int64_t>(h) * kTicksPerGameHour;
    while (t.world->clock.tick < hour_end) {
      const std::int64_t left = hour_end - t.world->clock.tick;
      const std::int64_t n = 1 + rng.below(left < 4000 ? left : 4000);
      ++op_count;
      switch (rng.below(5)) {
        case 0:  // step
        case 1:
          ops << "step " << n << "; ";
          t.world->run_ticks(n);
          if (fault == Fault::ExtraTick && op_count >= 3) t.world->run_ticks(1);  // a tick too many
          ++r.steps;
          break;
        case 2:  // jump
        case 3:
          ops << "jump to +" << n << "; ";
          t.world->jump(t.world->clock.tick + n);
          ++r.jumps;
          break;
        default: {  // save, then load into a fresh setup, and carry on in the fresh one
          ops << "save+load; ";
          const auto bytes = t.world->save();
          auto fresh = make();
          fresh.world->check_every_tick = false;
          if (fault == Fault::CorruptSave && op_count >= 3) {  // one bit flipped in the middle of the save
            auto bad = bytes;
            bad[bad.size() / 2] ^= 0x10;
            try {
              fresh.world->load(bad);
            } catch (const std::exception&) {  // a rejected load is also a catch
              r.agree = false;
              r.hour = h;
              r.system = "load rejected";
              r.ops = ops.str();
              return r;
            }
          } else {
            fresh.world->load(bytes);
          }
          for (std::size_t s = 0; s < r.hits.size(); ++s) r.hits[s] += t.world->buggify.hits[s];
          t = std::move(fresh);
          ++r.saves;
          break;
        }
      }
    }
    // Jumps and loads leave the world exactly at the hour boundary, as the oracle's hours do.
    const auto got = t.world->system_hashes();
    const auto& want = oracle.hours[static_cast<std::size_t>(h - 1)];
    for (std::size_t i = 0; i < got.size() && i < want.size(); ++i)
      if (got[i].hash != want[i].hash) {
        r.agree = false;
        r.hour = h;
        r.system = got[i].name;
        r.ops = ops.str();
        return r;
      }
    ops.str("");
  }
  for (std::size_t s = 0; s < r.hits.size(); ++s) r.hits[s] += t.world->buggify.hits[s];
  if (t.world->state_hash() != oracle.final_hash) {
    r.agree = false;
    r.hour = hours;
    r.system = "state hash";
    r.ops = ops.str();
  }
  return r;
}

}  // namespace zs::harness
