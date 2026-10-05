// Headless runner: runs the simulation with no window. Spec: docs/tier0/specs/16-view-2d.md rule 2.
// Stage 1: runs the demo scenario and prints one line per game day.
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "zs/body_lab.h"
#include "zs/demo.h"
#include "zs/noise.h"
#include "zs/site_map.h"
#include "zs/tier0.h"

namespace {

void usage() {
  std::puts("usage: zs_headless [--seed N] [--days D] [--save FILE] [--load FILE]\n"
            "       zs_headless --map [FILE]   print the room graph of a map (default: the Tier 0 block)\n"
            "       zs_headless --body-lab     run the body calibration scenarios\n"
            "       zs_headless --bench        body cost: 4 pops and 60 walkers for one game day\n"
            "       zs_headless --scripted-pop [--ai]  stage 4: a pop eats, drinks, dresses a wound, salvages, barricades\n"
            "       zs_headless --stage6 [--seed N]  the grid fails; generator, fuel, fridge, noise and light");
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

// Stage 4: the scripted pop, driven as the player (default) or as the AI.
int scripted_pop(bool as_ai) {
  auto t = zs::tier0::make_world(1);
  zs::tier0::wound_player(t);
  zs::tier0::ScriptedPop script(as_ai);
  std::size_t seen = 0;
  const std::int64_t limit = 4 * zs::kTicksPerGameHour;
  while (t.world->clock.tick < limit && script.drive(t)) {
    t.world->step();
    const auto& log = t.world->log.all();
    for (; seen < log.size(); ++seen) {
      const zs::Event& e = log[seen];
      const auto minute = static_cast<double>(e.tick) / (zs::kTicksPerGameHour / 60);
      const char* proc = e.a >= 0 && static_cast<std::size_t>(e.a) < t.actors->processes().size()
                             ? t.actors->processes()[static_cast<std::size_t>(e.a)].name.c_str() : "move";
      if (e.type == zs::EventType::ActionDone) std::printf("%6.1f min  done: %s\n", minute, proc);
      else if (e.type == zs::EventType::ActionRefused)
        std::printf("%6.1f min  refused: %s (%s)\n", minute, proc, zs::to_string(static_cast<zs::Reason>(e.b)));
      else if (e.type == zs::EventType::ActionInterrupted)
        std::printf("%6.1f min  interrupted: %s (%s)\n", minute, proc, zs::to_string(static_cast<zs::Reason>(e.b)));
    }
  }
  const auto& body = t.bodies->get(t.player);
  const auto& wd = body.wounds.front();
  std::printf("finished at %.1f min, world hash %016llx\n", static_cast<double>(t.world->clock.tick) / 120.0,
              static_cast<unsigned long long>(t.world->world_hash()));
  std::printf("wound: cleaned %d, dressed %d; blood loss %.3f; carried %.1f kg\n", static_cast<int>(wd.cleaned),
              wd.dressed ? 1 : 0, t.bodies->blood_loss_fraction(body), t.actors->actor(t.player).carried_kg);
  for (const auto& [id, ref] : t.actors->edges()) {
    const zs::Edge& ed = t.map->edge(ref);
    if (ed.barricade == 0) continue;
    int boards = 0;
    for (zs::EntityId p : t.items->get(ed.barricade).slots) boards += p != 0;
    std::printf("barricade at (%d,%d,%d): %d parts, Q%d, works %d\n", ref.z, ref.x, ref.y, boards,
                t.items->get(ed.barricade).quality, t.items->works(ed.barricade) ? 1 : 0);
  }
  return 0;
}

// Stage 6: the generator problem, as a timeline plus a night snapshot of noise and light.
std::string clock_text(std::int64_t tick) {
  zs::Clock c;
  c.tick = tick;
  const double h = c.hour();
  char buf[32];
  std::snprintf(buf, sizeof buf, "day %lld %02d:%02d", static_cast<long long>(c.calendar_day()), static_cast<int>(h),
                static_cast<int>((h - static_cast<int>(h)) * 60));
  return buf;
}

void night_snapshot(const zs::tier0::Tier0& t) {
  using namespace zs;
  const World& w = *t.world;
  const Power& p = *t.power;
  const double bg = background_db(p.mains_alive(), w.clock.hour());
  std::printf("\n--- snapshot %s ---\n", clock_text(w.clock.tick).c_str());
  std::printf("power: mains %s, panel on %s, generator %s, load %.0f W, fuel %.2f L (%.2f L/h)\n",
              p.mains_alive() ? "alive" : "dead",
              p.transfer() == Transfer::Generator ? "generator" : p.transfer() == Transfer::Mains ? "mains" : "off",
              p.generator_running() ? "running" : "stopped", p.load_w(), static_cast<double>(p.fuel_ul(w)) / 1e6,
              p.fuel_lph());
  for (const Receiver& r : p.receivers())
    std::printf("  %-10s at (%d,%d,%d): %s%s\n", p.receiver_defs()[r.def].kind.c_str(), r.at.z, r.at.x, r.at.y,
                r.on ? "on" : "off", r.powered ? ", powered" : "");
  std::printf("fridge %.1f C, spoil %.0f %%\n", p.fridge_temp_c(), p.spoil_progress() * 100);
  const NoiseSource gen{p.generator_cell(), p.generator().noise_db_1m};
  std::printf("noise: background %.0f dB; the generator is heard %.0f m away (with the grid alive at night: %.0f m)\n", bg,
              audible_range_m(*t.map, *t.items, gen, bg), audible_range_m(*t.map, *t.items, gen, background_db(true, 23)));
  const auto field = noise_field(*t.map, *t.items, noise_sources(w, *t.actors, p));
  std::printf("noise map, ground floor ('.' inaudible, 1-9 = every 10 dB above hearing):\n%s",
              noise_ascii(*t.map, field, 0, bg).c_str());
  std::printf("light: outdoors %.2f lux; lit windows:\n", Power::ambient_lux(w.clock.hour()));
  for (const WindowLeak& l : p.window_leaks(w))
    if (l.room_lux > 0)
      std::printf("  window (%d,%d,%d%s): room %.1f lux, cover %.0f %%, leak %.2f lux, seen from %.0f m\n", l.edge.z,
                  l.edge.x, l.edge.y, l.edge.vertical ? ",v" : "", l.room_lux, l.cover * 100, l.leak_lux, l.seen_m);
  std::printf("---\n\n");
}

int stage6(std::uint64_t seed) {
  using namespace zs;
  auto t = tier0::make_world(seed);
  tier0::GeneratorScenario script(false);
  std::printf("grid failure drawn for %s\n", clock_text(t.power->grid_failure_tick()).c_str());
  std::size_t seen = 0;
  bool snap = false;
  while (script.drive(t)) {
    t.world->step();
    const auto& log = t.world->log.all();
    for (; seen < log.size(); ++seen) {
      const Event& e = log[seen];
      const auto& procs = t.actors->processes();
      const char* proc = e.a >= 0 && static_cast<std::size_t>(e.a) < procs.size() ? procs[static_cast<std::size_t>(e.a)].name.c_str() : "?";
      const std::string at = clock_text(e.tick);
      switch (e.type) {
        case EventType::ActionDone: std::printf("%s  done: %s\n", at.c_str(), proc); break;
        case EventType::ActionRefused:
        case EventType::ActionInterrupted:
          std::printf("%s  %s: %s (%s)\n", at.c_str(), e.type == EventType::ActionRefused ? "refused" : "interrupted", proc,
                      to_string(static_cast<Reason>(e.b)));
          break;
        case EventType::GridFailed: std::printf("%s  GRID FAILED\n", at.c_str()); break;
        case EventType::GeneratorStalled: std::printf("%s  generator stalled\n", at.c_str()); break;
        case EventType::GeneratorOutOfFuel: std::printf("%s  generator out of fuel\n", at.c_str()); break;
        case EventType::BreakerTripped: std::printf("%s  breaker tripped: %s\n", at.c_str(), t.power->circuits()[static_cast<std::size_t>(e.a)].id.c_str()); break;
        case EventType::FoodSpoiled: std::printf("%s  fridge food spoiled (%.0f kJ)\n", at.c_str(), static_cast<double>(e.a) / 1000); break;
        case EventType::Death: std::printf("%s  the pop died\n", at.c_str()); break;
        default: break;
      }
    }
    if (!snap && t.power->generator_running() && t.world->clock.hour() >= 23 &&
        t.world->clock.tick % kTicksPerGameHour == 0 && t.actors->covered(*t.world, *tier0::GeneratorScenario::front_window(t))) {
      snap = true;
      night_snapshot(t);
    }
  }
  std::printf("ended %s: refueled %s, fridge %.1f C, world hash %016llx\n", clock_text(t.world->clock.tick).c_str(),
              script.refueled() ? "yes" : "no", t.power->fridge_temp_c(),
              static_cast<unsigned long long>(t.world->world_hash()));
  return 0;
}

int main(int argc, char** argv) {
  if (argc >= 2 && !std::strcmp(argv[1], "--stage6"))
    return stage6(argc >= 4 && !std::strcmp(argv[2], "--seed") ? std::strtoull(argv[3], nullptr, 10) : 1);
  if (argc >= 2 && !std::strcmp(argv[1], "--scripted-pop"))
    return scripted_pop(argc >= 3 && !std::strcmp(argv[2], "--ai"));
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
