// Headless runner: runs the simulation with no window. Spec: docs/tier0/specs/16-view-2d.md rule 2.
// Stage 1: runs the demo scenario and prints one line per game day.
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "zs/body_lab.h"
#include "zs/demo.h"
#include "zs/site_map.h"

namespace {

void usage() {
  std::puts("usage: zs_headless [--seed N] [--days D] [--save FILE] [--load FILE]\n"
            "       zs_headless --map [FILE]   print the room graph of a map (default: the Tier 0 block)\n"
            "       zs_headless --body-lab     run the body calibration scenarios\n"
            "       zs_headless --bench        body cost: 4 pops and 60 walkers for one game day");
}

}  // namespace

// Stage 2: loads a map and prints its rooms and portals.
int print_map(const std::string& path) {
  zs::World world(1);
  auto items = std::make_unique<zs::Assemblies>();
  auto map = std::make_unique<zs::SiteMap>();
  auto* items_ptr = items.get();
  auto* map_ptr = map.get();
  world.add_system(std::move(items));
  world.add_system(std::move(map));
  zs::load_item_defs(*items_ptr, std::string(ZS_DATA_DIR) + "/tier0/items.toml");
  map_ptr->load_file(world, *items_ptr, path);
  const auto& rooms = map_ptr->rooms();
  std::printf("%zu rooms, %zu portals, %zu assemblies\n", rooms.size(), map_ptr->portals().size(),
              items_ptr->all().size());
  for (std::size_t i = 0; i < rooms.size(); ++i) {
    int open = 0, closed = 0;
    for (const auto& p : map_ptr->portals())
      if (p.room_a == static_cast<int>(i) || p.room_b == static_cast<int>(i)) (p.open ? open : closed)++;
    std::printf("room %2zu  level %2d  %4d m2  %-7s%s  portals: %d open, %d closed\n", i, rooms[i].z, rooms[i].cells,
                rooms[i].indoor ? "indoor" : "outdoor", rooms[i].touches_border ? "  border" : "", open, closed);
  }
  return 0;
}

// Stage 3: the body lab prints each calibration scenario against its band.
int body_lab() {
  int failed = 0;
  for (const auto& r : zs::body_lab::run_all(1)) {
    std::printf("%-4s %-62s %8.3f %-26s band: %s\n", r.pass ? "ok" : "FAIL", r.name.c_str(), r.value, r.unit.c_str(),
                r.band.c_str());
    failed += r.pass ? 0 : 1;
  }
  return failed == 0 ? 0 : 1;
}

// Stage 3: how fast one game day runs with 4 pops and 60 walkers (rest-until-event runs flat out).
int bench() {
  std::vector<zs::BodySpec> specs(64);
  for (std::size_t i = 4; i < specs.size(); ++i) specs[i].walker = true;
  auto lab = zs::body_lab::make_lab(1, specs);
  std::vector<zs::EntityId> ids;
  for (const auto& [id, b] : lab.bodies->all()) ids.push_back(id);
  zs::Wound cut;
  cut.region = zs::Region::ForearmL;
  cut.depth = zs::Depth::Fat;
  lab.bodies->add_wound(*lab.world, ids[0], cut, 0);
  for (std::size_t i = 4; i < ids.size(); ++i) lab.bodies->get(ids[i]).activity.speed_mps = 0.4;
  lab.world->check_every_tick = false;
  const auto t0 = std::chrono::steady_clock::now();
  for (int h = 0; h < 24; ++h) {
    for (int k = 0; k < 4; ++k) {
      zs::body_lab::daily_routine(lab, ids[k], zs::Activity{});
      zs::body_lab::keep_supplied(lab, ids[k], true, true);
    }
    lab.world->run_ticks(zs::kTicksPerGameHour);
  }
  const double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  std::printf("1 game day (%lld ticks), 4 pops + 60 walkers: %.3f s real, %.0fx faster than 1x speed\n",
              static_cast<long long>(zs::kTicksPerGameDay), s, 3600.0 / s);
  return 0;
}

int main(int argc, char** argv) {
  if (argc >= 2 && !std::strcmp(argv[1], "--body-lab")) return body_lab();
  if (argc >= 2 && !std::strcmp(argv[1], "--bench")) return bench();
  if (argc >= 2 && !std::strcmp(argv[1], "--map")) {
    try {
      return print_map(argc >= 3 ? argv[2] : std::string(ZS_DATA_DIR) + "/tier0/map/block.toml");
    } catch (const std::exception& e) {
      std::fprintf(stderr, "error: %s\n", e.what());
      return 1;
    }
  }
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
