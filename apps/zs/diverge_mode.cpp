// zs --diverge (spec 56): two copies of the one world that should agree, compared system by system at every check
// tick; prints the first divergent tick and the system, or that they agree. Modes: jump (one steps, one jumps to the
// same ticks), saveload (one steps; the other is the first saved and loaded into a fresh setup at every check), both.
//   zs --diverge [--seed N] [--hours H] [--every TICKS] [--mode jump|saveload|both]
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "zs/clock.h"
#include "zs/gen/place.h"
#include "zs/tier0.h"
#include "zs/world.h"
#include "diverge_mode.h"

namespace zs::atlas {

namespace {

std::string first_divergence(const World& a, const World& b) {
  const auto ha = a.system_hashes(), hb = b.system_hashes();
  if (ha.size() != hb.size()) return "system count";
  for (std::size_t i = 0; i < ha.size(); ++i)
    if (ha[i].name != hb[i].name) return "system order";
    else if (ha[i].hash != hb[i].hash) return ha[i].name;
  return "";
}

std::string hex(std::uint64_t h) {
  char buf[24];
  std::snprintf(buf, sizeof buf, "%016llx", static_cast<unsigned long long>(h));
  return buf;
}

}  // namespace

int diverge_main(int argc, char** argv) {
  std::uint64_t seed = 1;
  int hours = 24;
  std::int64_t every = kTicksPerGameHour;
  std::string mode = "both";
  for (int i = 1; i + 1 < argc; ++i) {
    if (!std::strcmp(argv[i], "--seed")) seed = std::strtoull(argv[i + 1], nullptr, 10);
    else if (!std::strcmp(argv[i], "--hours")) hours = std::atoi(argv[i + 1]);
    else if (!std::strcmp(argv[i], "--every")) every = std::strtoll(argv[i + 1], nullptr, 10);
    else if (!std::strcmp(argv[i], "--mode")) mode = argv[i + 1];
  }
  const bool do_jump = mode == "jump" || mode == "both", do_saveload = mode == "saveload" || mode == "both";
  try {
    const auto t0 = std::chrono::steady_clock::now();
    std::printf("the one world, seed %llu: stepping %d hours, checking every %lld ticks (%s)\n", static_cast<unsigned long long>(seed), hours,
                static_cast<long long>(every), mode.c_str());
    auto stepped = zs::gen::make_world(seed);
    stepped.world->check_every_tick = false;
    stepped.world->probe_timing = true;
    std::unique_ptr<zs::tier0::Tier0> jumped;
    if (do_jump) {
      jumped = std::make_unique<zs::tier0::Tier0>(zs::gen::make_world(seed));
      jumped->world->check_every_tick = false;
    }
    const std::int64_t end = static_cast<std::int64_t>(hours) * kTicksPerGameHour;
    int checks = 0;
    bool diverged = false;
    for (std::int64_t t = every; t <= end && !diverged; t += every) {
      stepped.world->run_ticks(t - stepped.world->clock.tick);
      ++checks;
      if (do_jump) {
        jumped->world->jump(t);
        const std::string where = first_divergence(*stepped.world, *jumped->world);
        if (!where.empty()) {
          std::printf("DIVERGED (step vs jump) at tick %lld (%s): system %s\n", static_cast<long long>(t), zs::tier0::time_text(t).c_str(), where.c_str());
          diverged = true;
        }
      }
      if (do_saveload && !diverged) {
        const auto bytes = stepped.world->save();
        auto fresh = zs::gen::make_world(seed);
        fresh.world->load(bytes);
        const std::string where = first_divergence(*stepped.world, *fresh.world);
        if (!where.empty()) {
          std::printf("DIVERGED (save and load) at tick %lld (%s): system %s\n", static_cast<long long>(t), zs::tier0::time_text(t).c_str(), where.c_str());
          diverged = true;
        }
      }
    }
    const double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    if (!diverged) std::printf("agree: %d checks over %lld ticks, every system, in %.1f s\n", checks, static_cast<long long>(stepped.world->clock.tick), s);
    // The probes: where the time went.
    std::uint64_t total = 0;
    for (const auto& p : stepped.world->probes) total += p.ns;
    std::printf("\nsystem           updates        ms   share   hash\n");
    const auto hashes = stepped.world->system_hashes();
    for (std::size_t i = 0; i < stepped.world->systems().size(); ++i) {
      const auto& p = stepped.world->probes[i];
      std::printf("%-14s %9llu %9.1f %6.1f%%   %s\n", stepped.world->systems()[i]->name(), static_cast<unsigned long long>(p.updates), p.ns / 1e6,
                  total ? 100.0 * p.ns / total : 0.0, hex(hashes[i + 1].hash).c_str());
    }
    std::printf("%-14s %9s %9.1f %6s   %s\n", "core", "", 0.0, "", hex(hashes[0].hash).c_str());
    return diverged ? 1 : 0;
  } catch (const std::exception& ex) {
    std::fprintf(stderr, "zs --diverge: %s\n", ex.what());
    return 2;
  }
}

}  // namespace zs::atlas
