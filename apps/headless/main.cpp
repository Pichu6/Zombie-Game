// Headless runner: runs the simulation with no window. Spec: docs/tier0/specs/16-view-2d.md rule 2.
// Stage 1: runs the demo scenario and prints one line per game day.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "zs/demo.h"

namespace {

void usage() {
  std::puts("usage: zs_headless [--seed N] [--days D] [--save FILE] [--load FILE]");
}

}  // namespace

int main(int argc, char** argv) {
  std::uint64_t seed = 1;
  std::int64_t days = 14;
  std::string save_path, load_path;
  for (int i = 1; i < argc; ++i) {
    const bool has_value = i + 1 < argc;
    if (!std::strcmp(argv[i], "--seed") && has_value) seed = std::strtoull(argv[++i], nullptr, 10);
    else if (!std::strcmp(argv[i], "--days") && has_value) days = std::strtoll(argv[++i], nullptr, 10);
    else if (!std::strcmp(argv[i], "--save") && has_value) save_path = argv[++i];
    else if (!std::strcmp(argv[i], "--load") && has_value) load_path = argv[++i];
    else { usage(); return 2; }
  }

  try {
    auto world = zs::demo::make_world(seed);
    if (!load_path.empty()) world->load(zs::read_file(load_path));
    const auto ids = zs::demo::ids_of(*world);
    const std::int64_t end = world->clock.tick + days * zs::kTicksPerGameDay;
    while (world->clock.tick < end) {
      zs::demo::scripted_player(*world);
      world->step();
      if (world->clock.tick % zs::kTicksPerGameDay == 0) {
        std::printf("day %2lld  hash %016llx  tank %6lld mL  cans %6lld mL  delivered %8lld kJ  events %zu\n",
                    static_cast<long long>(world->clock.day()),
                    static_cast<unsigned long long>(world->state_hash()),
                    static_cast<long long>(world->ledger.balance(ids.tank, ids.fuel_ml)),
                    static_cast<long long>(world->ledger.balance(ids.cans, ids.fuel_ml)),
                    static_cast<long long>(world->ledger.balance(ids.house, ids.electric_kj)),
                    world->log.all().size());
      }
    }
    if (!save_path.empty()) zs::write_file(save_path, world->save());
  } catch (const std::exception& e) {
    std::fprintf(stderr, "error: %s\n", e.what());
    return 1;
  }
  return 0;
}
