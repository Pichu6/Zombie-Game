// zs headless modes: the simulation with no window. Spec: docs/tier0/specs/16-view-2d.md rule 2.
// Stage 1: runs the demo scenario and prints one line per game day.
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <string>

#include "zs/body_lab.h"
#include "zs/demo.h"
#include "zs/noise.h"
#include "zs/site_map.h"
#include "zs/gen/place.h"
#include "zs/tier0.h"
#include "modes.h"
#include "pop_bench.h"
#include "town_demo.h"

namespace {

void usage() {
  std::puts("usage: zs [--seed N] [--watch]   the window (no headless flag)\n"
            "       zs gen ...       the content generators (zs gen with no command lists them)\n"
            "       zs --days D [--seed N] [--save FILE] [--load FILE]  stage 1 demo, one line per game day\n"
            "       zs --run [--seed N] [--seeds N] [--careless] [--trace FILE]  the 14-day run, AI only, with the chronicle\n"
            "       zs --group [--seed N] [--days D] [--seeds N] [--trace FILE]  stage 8: 4 AI pops with the dead\n"
            "       zs --stage7 [--seed N] [--trace FILE]  stage 6 with the dead, until the pop rises\n"
            "       zs --map [FILE]   print the room graph of a map (default: the Tier 0 block)\n"
            "       zs --body-lab     run the body calibration scenarios\n"
            "       zs --bench        body cost: 4 pops and 60 walkers for one game day\n"
            "       zs --town [--seed N] [--people N] [--years Y]  a generated town as pops: a layoff, then years\n"
            "       zs --pop-bench    spec 28: what records, lazy pops and promotion cost\n"
            "       zs --scripted-pop [--ai]  stage 4: a pop eats, drinks, dresses a wound, salvages, barricades\n"
            "       zs --stage6 [--seed N] [--trace FILE]  the grid fails; generator, fuel, fridge, noise and light\n"
            "       zs --day [--seed N] [--country C] [--people N] [--date YYYY-MM-DD]  spec 54: a town's day hour by hour\n"
            "       zs --diverge [--seed N] [--hours H] [--every TICKS] [--mode jump|saveload|both]  spec 56: the first divergent tick and system, and the time per system\n"
            "                  (--trace writes a JSON replay for the web viewer, tools/viewer)");
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

// A replay of the stage 6 run for the web viewer: the map once, a frame per game minute, the noise
// and light fields every game hour, and the notable events. Plain JSON, written by hand.
struct Trace {
  std::string frames, fields, events;
  std::vector<std::string> labels;
  std::map<std::string, int> label_ids;

  int label(const std::string& s) {
    auto [it, fresh] = label_ids.emplace(s, static_cast<int>(labels.size()));
    if (fresh) labels.push_back(s);
    return it->second;
  }
  static std::string quote(const std::string& s) {
    std::string o = "\"";
    for (char c : s) o += c == '"' || c == '\\' ? std::string("\\") + c : std::string(1, c);
    return o + "\"";
  }

  void frame(const zs::tier0::Tier0& t) {
    using namespace zs;
    const World& w = *t.world;
    const Power& p = *t.power;
    const Actor& a = t.actors->actor(t.player);
    const Body& b = t.bodies->get(t.player);
    std::string doing = "idle";
    if (b.spec.walker) doing = "risen as a walker";
    else if (!b.alive) doing = "dead";
    else if (t.walkers && t.walkers->held(t.player)) doing = "held by a walker";
    else if (b.activity.asleep) doing = "sleeping";
    else if (a.action.phase == Phase::Approach) doing = a.action.process >= 0 ? "walking to: " + t.actors->processes()[static_cast<std::size_t>(a.action.process)].name : "walking";
    else if (a.action.phase == Phase::Work && a.action.process >= 0) doing = t.actors->processes()[static_cast<std::size_t>(a.action.process)].name;
    long long powered = 0;
    for (std::size_t i = 0; i < p.receivers().size(); ++i) powered |= (p.receivers()[i].powered ? 1LL : 0LL) << i;
    char buf[256];
    std::snprintf(buf, sizeof buf, "%s[%lld,%d,%d,%d,%d,%lld,%.0f,%d,%d,%d,%.1f,%.0f,%lld,%.0f,%.0f,%.0f]",
                  frames.empty() ? "" : ",", static_cast<long long>(w.clock.tick), a.pos.z, a.pos.x, a.pos.y, label(doing),
                  static_cast<long long>(p.fuel_ul(w) / 1000), p.load_w(), p.generator_running() ? 1 : 0,
                  static_cast<int>(p.transfer()), p.mains_alive() ? 1 : 0, p.fridge_temp_c(), p.spoil_progress() * 100, powered,
                  t.bodies->needs(b).thirst * 100, t.bodies->needs(b).hunger * 100,
                  t.bodies->needs(b).tiredness * 100);
    frames += buf;
    // Walkers on the map: [z, x, y, mode, name label], in body order.
    frames.pop_back();
    frames += ",[";
    if (t.walkers) {
      bool first = true;
      for (const auto& [id, z] : t.walkers->walkers()) {
        if (!t.bodies->get(id).alive) continue;
        std::snprintf(buf, sizeof buf, "%s[%d,%d,%d,%d,%d]", first ? "" : ",", z.pos.z, z.pos.x, z.pos.y,
                      static_cast<int>(z.mode), label(t.census->name_of(id)));
        frames += buf;
        first = false;
      }
    }
    frames += "],[";
    // The group: [z, x, y, doing label, name label, alive], player first.
    bool first_pop = true;
    for (EntityId id : t.group) {
      const Body& pb = t.bodies->get(id);
      const Cell c = t.actors->actor(id).pos;
      std::string what = pb.spec.walker ? "risen as a walker" : !pb.alive ? "dead" : pb.activity.asleep ? "sleeping" : "idle";
      if (pb.alive && !pb.spec.walker && !pb.activity.asleep && t.ai) {
        if (t.ai->minds().contains(id)) what = to_string(t.ai->mind(id).doing);
        else what = pb.away ? "not here yet" : "waiting to be found";
      }
      std::snprintf(buf, sizeof buf, "%s[%d,%d,%d,%d,%d,%d]", first_pop ? "" : ",", c.z, c.x, c.y, label(what),
                    label(t.census ? t.census->name_of(id) : "You"), pb.alive && !pb.spec.walker ? 1 : 0);
      frames += buf;
      first_pop = false;
    }
    frames += "]]";
  }

  // True when some walker is doing more than standing about (frames then come often, like walking).
  static bool walkers_busy(const zs::tier0::Tier0& t) {
    if (!t.walkers) return false;
    for (const auto& [id, z] : t.walkers->walkers())
      if (z.mode != zs::Mode::Idle && t.bodies->get(id).alive) return true;
    return false;
  }

  void field(const zs::tier0::Tier0& t) {
    using namespace zs;
    const World& w = *t.world;
    const Power& p = *t.power;
    const double bg = background_db(p.mains_alive(), w.clock.hour());
    const auto noise = noise_field(*t.map, *t.items, noise_sources(w, *t.actors, p));
    std::string n, l;
    for (int z : t.map->levels())
      for (int y = 0; y < t.map->height(); ++y)
        for (int x = 0; x < t.map->width(); ++x) {
          const Cell c{z, x, y};
          const int margin = static_cast<int>(noise_at(*t.map, noise, c) - bg - 3);
          n += std::to_string(std::max(-1, std::min(90, margin))) + ",";
          const double lux = t.map->walkable(c) ? p.lux_at(w, c) : 0;
          l += std::to_string(lux > 0 ? static_cast<int>(10 * std::log10(lux)) : -99) + ",";
        }
    n.pop_back();
    l.pop_back();
    std::string win;
    for (const WindowLeak& k : p.window_leaks(w)) {
      char buf[128];
      std::snprintf(buf, sizeof buf, "%s[%d,%d,%d,%d,%.0f,%.2f,%.1f]", win.empty() ? "" : ",", k.edge.z, k.edge.x, k.edge.y,
                    k.edge.vertical ? 1 : 0, k.seen_m, k.cover, k.room_lux);
      win += buf;
    }
    const NoiseSource gen{p.generator_cell(), p.generator().noise_db_1m};
    char head[160];
    std::snprintf(head, sizeof head, "%s{\"tick\":%lld,\"bg\":%.0f,\"ambient\":%.3f,\"gen_range\":%.0f,", fields.empty() ? "" : ",",
                  static_cast<long long>(w.clock.tick), bg, Power::ambient_lux(w.clock.hour()),
                  p.generator_running() ? audible_range_m(*t.map, *t.items, gen, bg) : 0.0);
    fields += std::string(head) + "\"noise\":[" + n + "],\"light\":[" + l + "],\"windows\":[" + win + "]}";
  }

  void event(std::int64_t tick, const char* kind, const std::string& text, const zs::EdgeRef* at = nullptr) {
    events += (events.empty() ? "" : ",") + std::string("[") + std::to_string(tick) + "," + quote(kind) + "," + quote(text);
    if (at) events += "," + std::to_string(at->z) + "," + std::to_string(at->x) + "," + std::to_string(at->y) + "," + (at->vertical ? "1" : "0");
    events += "]";
  }

  void write(const zs::tier0::Tier0& t, std::uint64_t seed, const std::string& path) {
    using namespace zs;
    const SiteMap& m = *t.map;
    std::string floors, edges, objects, receivers;
    for (int z : m.levels()) {
      std::string rows;
      for (int y = 0; y < m.height(); ++y) {
        std::string row;
        for (int x = 0; x < m.width(); ++x) {
          const Floor f = m.floor({z, x, y});
          row += f == Floor::None ? ' ' : f == Floor::Wood ? '.' : f == Floor::Concrete ? ',' : f == Floor::Grass ? '"' : f == Floor::Asphalt ? '_' : ':';
        }
        rows += (rows.empty() ? "" : ",") + quote(row);
      }
      floors += (floors.empty() ? "" : ",") + std::string("[") + rows + "]";
    }
    for (int z : m.levels())
      for (int v = 0; v < 2; ++v)
        for (int y = 0; y <= m.height() - v; ++y)
          for (int x = 0; x <= m.width() - (v ? 0 : 1); ++x) {
            const EdgeRef r{z, x, y, v == 1};
            const Edge& e = m.edge(r);
            const char* k = "";
            switch (e.kind) {
              case EdgeKind::WallInterior: k = "wall"; break;
              case EdgeKind::WallExterior: k = "outer"; break;
              case EdgeKind::DoorInterior: k = "door"; break;
              case EdgeKind::DoorExterior: k = "outer_door"; break;
              case EdgeKind::Window: k = "window"; break;
              case EdgeKind::Fence: k = "fence"; break;
              case EdgeKind::Gate: k = "gate"; break;
              default: continue;
            }
            edges += (edges.empty() ? "" : ",") + std::string("[") + std::to_string(z) + "," + std::to_string(x) + "," +
                     std::to_string(y) + "," + std::to_string(v) + "," + quote(k) + "," + (e.barricade ? "1" : "0") + "]";
          }
    for (const MapObject& o : m.objects())
      objects += (objects.empty() ? "" : ",") + std::string("[") + quote(o.kind) + "," + quote(o.id) + "," +
                 std::to_string(o.at.z) + "," + std::to_string(o.at.x) + "," + std::to_string(o.at.y) + "]";
    for (const Receiver& r : t.power->receivers())
      receivers += (receivers.empty() ? "" : ",") + std::string("[") + quote(t.power->receiver_defs()[r.def].kind) + "," +
                   std::to_string(r.at.z) + "," + std::to_string(r.at.x) + "," + std::to_string(r.at.y) + "]";
    std::string labels_json;
    for (const std::string& s : labels) labels_json += (labels_json.empty() ? "" : ",") + quote(s);
    std::string levels;
    for (int z : m.levels()) levels += (levels.empty() ? "" : ",") + std::to_string(z);
    std::ofstream out(path);
    out << "{\"seed\":" << seed << ",\"dead\":" << (t.walkers ? 1 : 0) << ",\"width\":" << m.width() << ",\"height\":" << m.height() << ",\"levels\":[" << levels
        << "],\"ticks_per_hour\":" << kTicksPerGameHour << ",\"start_hour\":8,\"tank_ml\":" << t.power->tank_ul() / 1000
        << ",\"floors\":[" << floors << "],\"edges\":[" << edges << "],\"objects\":[" << objects << "],\"receivers\":["
        << receivers << "],\"labels\":[" << labels_json << "],\"frames\":[" << frames << "],\"fields\":[" << fields
        << "],\"events\":[" << events << "]}";
  }
};

int stage6(std::uint64_t seed, const std::string& trace_path = "", bool with_dead = false) {
  using namespace zs;
  auto t = tier0::make_world(seed, with_dead);
  tier0::GeneratorScenario script(false);
  std::printf("grid failure drawn for %s\n", clock_text(t.power->grid_failure_tick()).c_str());
  std::size_t seen = 0;
  bool snap = false;
  Trace trace;
  const bool tracing = !trace_path.empty();
  // With the dead, the run goes on after the pop is gone, until it rises (or day 10).
  std::int64_t stop_at = -1;
  auto running = [&]() {
    if (script.drive(t)) return true;
    if (!with_dead || t.world->clock.day() >= 10) return false;
    if (stop_at < 0) stop_at = t.world->clock.tick + 3 * kTicksPerGameHour;
    for (const auto& tr : t.walkers->turnings())
      if (tr.body == t.player && !tr.risen) stop_at = std::max(stop_at, t.world->clock.tick + 1);
    return t.world->clock.tick < stop_at;
  };
  while (running()) {
    t.world->step();
    // A frame every game minute, and every 5 game s while the pop walks so the replay can follow it.
    const bool walking = t.actors->actor(t.player).action.phase == Phase::Approach || Trace::walkers_busy(t);
    if (tracing && t.world->clock.tick % (walking ? 10 : kBodyStepTicks) == 0) trace.frame(t);
    if (tracing && t.world->clock.tick % kTicksPerGameHour == 0) trace.field(t);
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
        case EventType::Death:
          std::printf("%s  %s\n", at.c_str(), zs::tier0::death_sentence(t, e).c_str());
          break;
        case EventType::WalkerArrived: std::printf("%s  A walker (%s) arrived\n", at.c_str(), t.census->name_of(e.subject).c_str()); break;
        case EventType::Grabbed: std::printf("%s  %s grabbed %s\n", at.c_str(), t.census->name_of(e.subject).c_str(), t.census->name_of(static_cast<EntityId>(e.a)).c_str()); break;
        case EventType::Bite: std::printf("%s  %s bit %s (%s)\n", at.c_str(), t.census->name_of(e.subject).c_str(), t.census->name_of(static_cast<EntityId>(e.a)).c_str(), to_string(static_cast<Region>(e.b))); break;
        case EventType::EdgeBreached: std::printf("%s  BREACHED: %s at (%d,%d,%d)\n", at.c_str(), t.items->def(t.items->get(e.subject).def).id.c_str(), t.actors->edges().at(e.subject).z, t.actors->edges().at(e.subject).x, t.actors->edges().at(e.subject).y); break;
        case EventType::Reanimated: std::printf("%s  %s rose as a walker\n", at.c_str(), t.census->name_of(e.subject).c_str()); break;
        default: break;
      }
      if (tracing) {
        switch (e.type) {
          case EventType::ActionDone: trace.event(e.tick, "done", proc); break;
          case EventType::ActionRefused: trace.event(e.tick, "refused", std::string(proc) + " (" + to_string(static_cast<Reason>(e.b)) + ")"); break;
          case EventType::ActionInterrupted: trace.event(e.tick, "interrupted", std::string(proc) + " (" + to_string(static_cast<Reason>(e.b)) + ")"); break;
          case EventType::GridFailed: trace.event(e.tick, "alert", "The grid failed"); break;
          case EventType::GeneratorStalled: trace.event(e.tick, "alert", "The generator stalled"); break;
          case EventType::GeneratorOutOfFuel: trace.event(e.tick, "alert", "The generator ran out of fuel"); break;
          case EventType::BreakerTripped: trace.event(e.tick, "alert", "Breaker tripped: " + t.power->circuits()[static_cast<std::size_t>(e.a)].id); break;
          case EventType::FoodSpoiled: trace.event(e.tick, "alert", "The fridge food spoiled"); break;
          case EventType::FellAsleep: trace.event(e.tick, "body", "Fell asleep"); break;
          case EventType::WokeUp: trace.event(e.tick, "body", "Woke up"); break;
          case EventType::Death: trace.event(e.tick, "dead", tier0::death_sentence(t, e)); break;
          case EventType::WalkerArrived: trace.event(e.tick, "dead", "A walker (" + t.census->name_of(e.subject) + ") arrived"); break;
          case EventType::Grabbed: trace.event(e.tick, "dead", t.census->name_of(e.subject) + " grabbed " + t.census->name_of(static_cast<EntityId>(e.a))); break;
          case EventType::Bite: trace.event(e.tick, "alert", t.census->name_of(e.subject) + " bit " + t.census->name_of(static_cast<EntityId>(e.a)) + " (" + to_string(static_cast<Region>(e.b)) + ")"); break;
          case EventType::Reanimated: trace.event(e.tick, "alert", t.census->name_of(e.subject) + " rose as a walker"); break;
          case EventType::EdgeBreached: {
            const EdgeRef r = t.actors->edges().at(e.subject);
            trace.event(e.tick, "alert", "The " + t.items->def(t.items->get(e.subject).def).id + " was broken in", &r);
            break;
          }
          default: break;
        }
      }
    }
    if (!snap && t.power->generator_running() && t.world->clock.hour() >= 23 &&
        t.world->clock.tick % kTicksPerGameHour == 0 && t.actors->covered(*t.world, *tier0::GeneratorScenario::front_window(t))) {
      snap = true;
      night_snapshot(t);
    }
  }
  if (tracing) {
    trace.write(t, seed, trace_path);
    std::printf("trace written to %s\n", trace_path.c_str());
  }
  std::printf("ended %s: refueled %s, fridge %.1f C, world hash %016llx\n", clock_text(t.world->clock.tick).c_str(),
              script.refueled() ? "yes" : "no", t.power->fridge_temp_c(),
              static_cast<unsigned long long>(t.world->world_hash()));
  return 0;
}

// Stage 8: a group of AI pops with nobody in control. Prints the story (or only the summary).
struct GroupResult {
  std::int64_t ended = 0;
  int alive = 0;
  std::vector<std::string> deaths;
  // The run (stage 9).
  zs::RunOutcome outcome = zs::RunOutcome::Running;
  int joined = 0;
  double crunch_day = -1;  // the generator ran dry with no fuel left anywhere
  double grid_day = -1;
  double fuel_end_l = 0;
};

// Gasoline still in the tank and in every can (stores and containers), in litres.
double fuel_left_l(const zs::tier0::Tier0& t) {
  const auto& names = t.world->ledger.resource_names();
  const auto gas = std::find(names.begin(), names.end(), "gasoline_ul");
  if (gas == names.end()) return 0;
  const auto res = static_cast<zs::ResourceId>(gas - names.begin());
  std::int64_t ul = 0;
  for (const auto& [key, v] : t.world->ledger.balances()) {
    if (key.second != res) continue;
    const std::string& owner = t.world->ledger.owner_names()[key.first];
    if (owner.rfind("store:", 0) == 0 || owner.rfind("container:", 0) == 0) ul += v;
  }
  return static_cast<double>(ul) / 1e6;
}

GroupResult group_run(std::uint64_t seed, int days, const std::string& trace_path, bool verbose, bool is_run = false,
                      bool careless = false) {
  using namespace zs;
  auto t = is_run ? zs::gen::make_world(seed) : tier0::make_world(seed, true, 4);  // --run plays the one world
  if (careless) t.ai->set_careless(true);
  t.world->check_every_tick = false;
  Trace trace;
  const bool tracing = !trace_path.empty();
  std::size_t seen = 0;
  GroupResult out;
  auto name = [&](EntityId b) { return t.census->name_of(b); };
  auto anyone_alive = [&] {
    for (EntityId b : t.group)
      if (t.bodies->get(b).alive && !t.bodies->get(b).spec.walker) return true;
    return false;
  };
  if (verbose) {
    std::printf("group:");
    for (EntityId b : t.group) {
      if (t.ai->minds().contains(b))
        std::printf(" %s (%d, %s, %s)", name(b).c_str(), static_cast<int>(t.bodies->get(b).spec.age),
                    t.ai->mind(b).brave ? "brave" : "timid", t.ai->mind(b).honest ? "honest" : "secretive");
      else
        for (const Found& f : t.run->found())
          if (f.body == b) std::printf(" | %s (%d, the %s, waiting)", name(b).c_str(), static_cast<int>(t.bodies->get(b).spec.age), to_string(f.role));
    }
    std::printf("\ngrid failure drawn for %s\n", clock_text(t.power->grid_failure_tick()).c_str());
    if (t.run)
      std::printf("stranger at the gate %s%s; broadcast from %s\n", clock_text(t.run->stranger_tick()).c_str(),
                  t.run->stranger_bitten() ? " (bitten, hiding it)" : "", clock_text(t.run->broadcast_tick()).c_str());
  }
  std::int64_t all_dead_at = -1;
  std::map<std::pair<EntityId, std::int64_t>, std::int64_t> last_refusal;
  while (t.world->clock.tick < days * kTicksPerGameDay) {
    t.world->step();
    const std::int64_t now = t.world->clock.tick;
    if (tracing) {
      bool busy = Trace::walkers_busy(t);
      for (EntityId b : t.group) busy |= t.actors->actor(b).action.phase == Phase::Approach;
      if (now % (busy ? 10 : kBodyStepTicks) == 0) trace.frame(t);
      if (now % kTicksPerGameHour == 0) trace.field(t);
    }
    const auto& log = t.world->log.all();
    for (; seen < log.size(); ++seen) {
      const Event& e = log[seen];
      const auto el = tier0::event_line(t, e);
      if (!el) continue;
      std::string line = el->text;
      const char* kind = el->kind;
      switch (e.type) {
        case EventType::GridFailed: out.grid_day = static_cast<double>(e.tick) / kTicksPerGameDay; break;
        case EventType::GeneratorOutOfFuel:
          if (out.crunch_day < 0 && fuel_left_l(t) < 1) {
            out.crunch_day = static_cast<double>(e.tick) / kTicksPerGameDay;
            line += ", and there is none left";
          }
          break;
        case EventType::Joined: ++out.joined; break;
        case EventType::ActionRefused: continue;  // the AI's own tries
        case EventType::Death:
          if (!e.b) out.deaths.push_back(line);
          break;
        case EventType::EdgeBreached: {
          const EdgeRef r = t.actors->edges().at(e.subject);
          if (tracing) trace.event(e.tick, kind, line, &r);
          if (verbose) std::printf("%s  %s\n", clock_text(e.tick).c_str(), line.c_str());
          continue;
        }
        case EventType::PopRefused: {
          // One line per pop and reason an hour: the AI retries every few minutes.
          auto& last = last_refusal[{e.subject, e.a}];
          if (last && e.tick - last < kTicksPerGameHour) continue;
          last = e.tick;
          break;
        }
        default: break;
      }
      if (tracing) trace.event(e.tick, kind, line);
      if (verbose) std::printf("%s  %s\n", clock_text(e.tick).c_str(), line.c_str());
    }
    if (t.run && t.run->outcome() != RunOutcome::Running) break;
    if (!anyone_alive()) {
      if (all_dead_at < 0) all_dead_at = now;
      if (now >= all_dead_at + 2 * kTicksPerGameHour) break;  // long enough to see them rise
    }
  }
  out.ended = t.world->clock.tick;
  if (t.run) {
    out.outcome = t.run->outcome();
    out.fuel_end_l = fuel_left_l(t);
    out.alive = 0;
    for (EntityId b : t.run->group()) out.alive += t.bodies->get(b).alive && !t.bodies->get(b).spec.walker;
  }
  if (!t.run)
    for (EntityId b : t.group) out.alive += t.bodies->get(b).alive && !t.bodies->get(b).spec.walker;
  if (tracing) {
    trace.write(t, seed, trace_path);
    std::printf("trace written to %s\n", trace_path.c_str());
  }
  if (verbose && t.run) {
    // Spec 25: the death screen for each of the group, then the days in five lines each.
    std::printf("\n== The dead of the group ==\n");
    for (const Event& e : t.world->log.all()) {
      if (e.type != EventType::Death || e.b || std::find(t.group.begin(), t.group.end(), e.subject) == t.group.end()) continue;
      std::printf("%s\n", tier0::death_sentence(t, e).c_str());
      std::string tags;
      for (const std::string& g : tier0::danger_tags(t, e)) tags += (tags.empty() ? "" : ", ") + g;
      std::printf("  tags: %s\n", tags.empty() ? "none" : tags.c_str());
      for (const std::string& m : tier0::missed_counters(t, e)) std::printf("  missed: %s\n", m.c_str());
    }
    std::printf("\n== Day by day ==\n");
    for (int d = 0; d * kTicksPerGameDay <= out.ended; ++d) {
      const auto lines = tier0::day_summary(t, d);
      std::printf("Day %d%s\n", d, lines.empty() ? ": quiet" : "");
      for (const std::string& l : lines) std::printf("  %s\n", l.c_str());
    }
  }
  if (verbose)
    std::printf("ended %s: %d of %zu alive, world hash %016llx\n", clock_text(out.ended).c_str(), out.alive,
                t.group.size(), static_cast<unsigned long long>(t.world->world_hash()));
  return out;
}

int headless_main(int argc, char** argv) {
  if (argc >= 2 && !std::strcmp(argv[1], "--run")) {
    std::uint64_t seed = 1, seeds = 0;
    std::string trace;
    bool careless = false;
    for (int i = 2; i < argc; ++i) {
      if (!std::strcmp(argv[i], "--careless")) careless = true;
      else if (i + 1 >= argc) break;
      else if (!std::strcmp(argv[i], "--seed")) seed = std::strtoull(argv[++i], nullptr, 10);
      else if (!std::strcmp(argv[i], "--seeds")) seeds = std::strtoull(argv[++i], nullptr, 10);
      else if (!std::strcmp(argv[i], "--trace")) trace = argv[++i];
    }
    if (!seeds) {
      group_run(seed, 15, trace, true, true, careless);
      return 0;
    }
    // Batch (spec 26 tests): outcomes, when the fuel runs out, who joined, what kills.
    std::map<std::string, int> causes, outcomes;
    std::vector<double> crunch;
    int joined = 0;
    for (std::uint64_t s = seed; s < seed + seeds; ++s) {
      const GroupResult r = group_run(s, 15, "", false, true, careless);
      outcomes[to_string(r.outcome)]++;
      joined += r.joined;
      if (r.crunch_day >= 0) crunch.push_back(r.crunch_day);
      for (const std::string& d : r.deaths) {
        const auto colon = d.find(": ");
        const auto dot = d.find('.', colon == std::string::npos ? 0 : colon);
        causes[colon == std::string::npos ? d : d.substr(colon + 2, dot - colon - 2)]++;
      }
      std::printf("seed %llu: %s at %s; %d alive, %d joined; grid failed day %.1f; fuel gone %s (%.1f L left)\n",
                  static_cast<unsigned long long>(s), to_string(r.outcome), clock_text(r.ended).c_str(), r.alive, r.joined,
                  r.grid_day, r.crunch_day < 0 ? "never" : ("day " + std::to_string(r.crunch_day).substr(0, 4)).c_str(), r.fuel_end_l);
    }
    int in_band = 0;
    for (double d : crunch) in_band += d >= 8 && d < 12;
    std::printf("\n%llu runs: ", static_cast<unsigned long long>(seeds));
    for (const auto& [o, n] : outcomes) std::printf("%d %s; ", n, o.c_str());
    std::printf("\nfound survivors joined: %d of %llu; fuel ran out in %zu runs, between day 8 and 12 in %d\n", joined,
                static_cast<unsigned long long>(3 * seeds), crunch.size(), in_band);
    for (const auto& [c, n] : causes) std::printf("  %3d died: %s\n", n, c.c_str());
    return 0;
  }
  if (argc >= 2 && !std::strcmp(argv[1], "--group")) {
    std::uint64_t seed = 1, seeds = 0;
    int days = 14;
    std::string trace;
    for (int i = 2; i + 1 < argc; i += 2) {
      if (!std::strcmp(argv[i], "--seed")) seed = std::strtoull(argv[i + 1], nullptr, 10);
      else if (!std::strcmp(argv[i], "--seeds")) seeds = std::strtoull(argv[i + 1], nullptr, 10);
      else if (!std::strcmp(argv[i], "--days")) days = std::atoi(argv[i + 1]);
      else if (!std::strcmp(argv[i], "--trace")) trace = argv[i + 1];
    }
    if (!seeds) {
      group_run(seed, days, trace, true);
      return 0;
    }
    // Batch (spec 24 test): how many live, and what kills them.
    std::map<std::string, int> causes;
    int alive = 0, survived_runs = 0;
    for (std::uint64_t s = 1; s <= seeds; ++s) {
      const GroupResult r = group_run(s, days, "", false);
      alive += r.alive;
      survived_runs += r.alive > 0;
      for (const std::string& d : r.deaths) {
        const auto colon = d.find(": ");
        const auto dot = d.find('.', colon == std::string::npos ? 0 : colon);
        causes[colon == std::string::npos ? d : d.substr(colon + 2, dot - colon - 2)]++;
      }
      std::printf("seed %llu: %d alive at %s\n", static_cast<unsigned long long>(s), r.alive, clock_text(r.ended).c_str());
    }
    std::printf("\n%llu runs of 4 pops, %d days: %d of %llu pops alive at the end; at least one alive in %d runs\n",
                static_cast<unsigned long long>(seeds), days, alive, static_cast<unsigned long long>(4 * seeds), survived_runs);
    for (const auto& [c, n] : causes) std::printf("  %3d died: %s\n", n, c.c_str());
    return 0;
  }
  if (argc >= 2 && !std::strcmp(argv[1], "--stage6"))
  {
    std::uint64_t seed = 1;
    std::string trace;
    for (int i = 2; i + 1 < argc; i += 2) {
      if (!std::strcmp(argv[i], "--seed")) seed = std::strtoull(argv[i + 1], nullptr, 10);
      else if (!std::strcmp(argv[i], "--trace")) trace = argv[i + 1];
    }
    return stage6(seed, trace, false);
  }
  if (argc >= 2 && !std::strcmp(argv[1], "--stage7")) {
    std::uint64_t seed = 1;
    std::string trace;
    for (int i = 2; i + 1 < argc; i += 2) {
      if (!std::strcmp(argv[i], "--seed")) seed = std::strtoull(argv[i + 1], nullptr, 10);
      else if (!std::strcmp(argv[i], "--trace")) trace = argv[i + 1];
    }
    return stage6(seed, trace, true);
  }
  if (argc >= 2 && !std::strcmp(argv[1], "--scripted-pop"))
    return scripted_pop(argc >= 3 && !std::strcmp(argv[2], "--ai"));
  if (argc >= 2 && !std::strcmp(argv[1], "--body-lab")) return body_lab();
  if (argc >= 2 && !std::strcmp(argv[1], "--bench")) return bench();
  if (argc >= 2 && !std::strcmp(argv[1], "--pop-bench")) return pop_bench();
  if (argc >= 2 && !std::strcmp(argv[1], "--town")) {
    std::uint64_t seed = 1;
    int people = 3000;
    double years = 60;
    for (int i = 2; i + 1 < argc; i += 2) {
      if (!std::strcmp(argv[i], "--seed")) seed = std::strtoull(argv[i + 1], nullptr, 10);
      else if (!std::strcmp(argv[i], "--people")) people = std::atoi(argv[i + 1]);
      else if (!std::strcmp(argv[i], "--years")) years = std::atof(argv[i + 1]);
    }
    try {
      return town_demo(seed, people, years);
    } catch (const std::exception& e) {
      std::fprintf(stderr, "error: %s\n", e.what());
      return 1;
    }
  }
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
