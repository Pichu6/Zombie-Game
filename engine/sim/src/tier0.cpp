// The Tier 0 world builder. Spec: docs/tier0/specs/05-clock.md rule 1 (update order).
#include "zs/tier0.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

#include "toml.hpp"

namespace zs::tier0 {

namespace {
Tier0 build(std::uint64_t seed, bool with_dead, int group, bool run, const Place& place = default_place());
}

Tier0 make_world(std::uint64_t seed, bool with_dead, int group) { return build(seed, with_dead, group, false); }

Tier0 make_run(std::uint64_t seed, const Place& place) { return build(seed, true, 1, true, place); }

namespace {
void setup_run(Tier0& t, const std::string& path) {
  World& w = *t.world;
  const toml::table tt = toml::parse_file(path);
  const auto& r = *tt["run"].as_table();
  auto range = [&](const char* key) {
    const auto* a = r[key].as_array();
    return std::pair<int, int>{(*a)[0].value<int>().value(), (*a)[1].value<int>().value()};
  };
  const EntityId radio = *t.actors->find_object("radio");
  std::int64_t stranger_tick = 0;
  bool bitten = false;
  t.ai->set_run(t.run);
  if (const auto* found = tt["found"].as_array())
    for (const auto& node : *found) {
      const auto& f = *node.as_table();
      const std::string role_name = f["role"].value<std::string>().value();
      const std::string who = f["person"].value<std::string>().value();
      std::optional<std::size_t> person;
      for (std::size_t p : t.census->survivors())
        if (t.census->people()[p].name == who) person = p;
      if (!person) throw std::runtime_error("run: no survivor slot named " + who);
      const auto at = t.map->find_object("found", role_name);
      if (!at) throw std::runtime_error("run: the map has no found object " + role_name);
      const EntityId body = t.census->embody(w, *person);
      t.actors->add_actor(w, body, *at);
      Actor& a = t.actors->actor(body);
      for (std::size_t k = 0; k < kTradeCount; ++k)
        if (auto v = f[to_string(static_cast<Trade>(k))].value<int>()) a.skill[k] = *v;
      Body& b = t.bodies->get(body);
      if (const double short_share = f["water_short"].value_or(0.0); short_share > 0) {
        const auto& L = t.bodies->ids();
        w.ledger.transfer(0, b.owner, L.excreted, L.water_ul,
                          static_cast<std::int64_t>(short_share * static_cast<double>(b.water_normal_ul)), 0);
      }
      Role role = Role::Neighbour;
      if (role_name == "technician") role = Role::Technician;
      if (role_name == "stranger") {
        role = Role::Stranger;
        b.away = true;  // still out on the streets (spec 27: cold)
        const auto [d0, d1] = range("stranger_day");
        const double day = d0 + std::floor(w.rng.uniform01(body, Purpose::Run) * (d1 - d0 + 1));
        const double hour = 8 + 10 * w.rng.uniform01(body, Purpose::Run);
        stranger_tick = static_cast<std::int64_t>(day * kTicksPerGameDay + (hour - 8) * kTicksPerGameHour);
        bitten = w.rng.uniform01(body, Purpose::Run) < r["stranger_bite_p"].value_or(0.5);
      }
      t.run->add_found(role, body);
      t.group.push_back(body);
    }
  const auto [b0, b1] = range("broadcast_day");
  const double bday = b0 + std::floor(w.rng.uniform01(radio, Purpose::Run) * (b1 - b0 + 1));
  const std::int64_t broadcast = static_cast<std::int64_t>(bday * kTicksPerGameDay + 4 * kTicksPerGameHour);  // 12:00
  const std::int64_t end = r["days"].value_or(14) * kTicksPerGameDay;
  t.run->start(w, t.player, radio, stranger_tick, bitten, broadcast, end);
}

Tier0 build(std::uint64_t seed, bool with_dead, int group, bool is_run, const Place& place) {
  if (group > 0 && !with_dead) throw std::invalid_argument("make_world: a group needs the census");
  Tier0 t;
  t.world = std::make_unique<World>(seed);
  World& w = *t.world;
  auto items = std::make_unique<Assemblies>();
  auto map = std::make_unique<SiteMap>();
  auto bodies = std::make_unique<Bodies>(w);  // registers its ledger resources first
  auto actors = std::make_unique<Actors>(w, *items, *map, *bodies);
  auto power = std::make_unique<Power>(w, *map, *items, *actors);
  std::unique_ptr<Census> census;
  std::unique_ptr<Walkers> walkers;
  std::unique_ptr<PopAI> ai;
  if (with_dead) {
    census = std::make_unique<Census>(w, *map, *items, *bodies, *power);
    walkers = std::make_unique<Walkers>(w, *map, *items, *bodies, *actors, *power, *census);
  }
  if (group > 0) ai = std::make_unique<PopAI>(w, *map, *items, *bodies, *actors, *power, *walkers);
  std::unique_ptr<Run> run;
  if (is_run) run = std::make_unique<Run>(w, *bodies, *actors, *power, *census, *walkers, *ai);
  t.items = items.get();
  t.map = map.get();
  t.bodies = bodies.get();
  t.actors = actors.get();
  t.power = power.get();
  t.census = census.get();
  t.walkers = walkers.get();
  t.ai = ai.get();
  t.run = run.get();
  // Order: things and places, the executor and movement, power, the dead, then the body.
  w.add_system(std::move(items));
  w.add_system(std::move(map));
  w.add_system(std::move(actors));
  w.add_system(std::move(power));
  if (with_dead) {
    w.add_system(std::move(census));
    w.add_system(std::move(walkers));
  }
  w.add_system(std::move(bodies));
  if (ai) w.add_system(std::move(ai));  // decides on this tick's state, acts next tick
  if (run) w.add_system(std::move(run));  // last: arrivals, control and the end, after everything moved
  const std::string data = data_dir();
  load_item_defs(*t.items, data + "/tier0/items.toml");
  if (is_run) t.map->set_scenario("run");
  t.map->load_file(w, *t.items, data + "/tier0/map/block.toml");
  t.actors->load_processes(data + "/tier0/processes.toml");
  t.actors->populate(w);
  t.power->load_file(data + "/tier0/power.toml");
  t.power->setup(w);
  BodySpec spec;  // an average adult (spec 11 defaults)
  t.player = t.bodies->create(w, spec, "player");
  t.actors->add_actor(w, t.player, *t.map->find_object("pop_start", "player"));
  if (with_dead) {
    t.walkers->setup(w);
    t.census->load_file(w, data + "/tier0/census.toml", *t.walkers);
    t.census->add_pop(w, t.player, "You");
  }
  t.group.push_back(t.player);
  if (group > 0) {
    // Survivors start in the cells nearest the player start (breadth-first over walkable cells).
    const Cell start = t.actors->actor(t.player).pos;
    std::vector<Cell> cells{start};
    for (std::size_t i = 0; i < cells.size() && static_cast<int>(cells.size()) < 4 * group; ++i)
      for (const Cell& nb : {Cell{start.z, cells[i].x + 1, cells[i].y}, Cell{start.z, cells[i].x - 1, cells[i].y},
                             Cell{start.z, cells[i].x, cells[i].y + 1}, Cell{start.z, cells[i].x, cells[i].y - 1}})
        if (t.map->in_bounds(nb) && t.map->walkable(nb) && t.map->can_walk(SiteMap::edge_between(cells[i], nb)) &&
            std::find(cells.begin(), cells.end(), nb) == cells.end())
          cells.push_back(nb);
    const auto slots = t.census->survivors();
    for (int k = 1; k < group && k - 1 < static_cast<int>(slots.size()); ++k) {
      const std::size_t person = slots[static_cast<std::size_t>(k - 1)];
      const EntityId body = t.census->embody(w, person);
      t.actors->add_actor(w, body, cells.at(static_cast<std::size_t>(k)));
      t.census->set_fate(w, person, Fate::Pop);
      t.group.push_back(body);
    }
    for (EntityId b : t.group) t.ai->add_pop(w, b);
  }
  if (is_run) {
    setup_run(t, data + "/tier0/run.toml");
    add_world_systems(t, place);
  }
  w.close_setup();
  return t;
}
}  // namespace

}  // namespace zs::tier0

namespace zs::tier0 {
namespace {

std::optional<EntityId> loose_item(const Tier0& t, const std::string& def_id) {
  for (const auto& [id, inst] : t.items->all()) {
    if (t.items->def(inst.def).id != def_id) continue;
    const EntityId parent = t.world->locations.parent_of(id);
    if (t.items->exists(parent) || parent == t.player) continue;  // mounted, or already carried
    if (parent < Actors::kFloorBase || parent >= World::kFirstEntity) continue;  // on a wound, etc.
    return id;
  }
  return std::nullopt;
}

std::optional<EntityId> nearest_window(const Tier0& t) {
  const Cell from = t.actors->actor(t.player).pos;
  std::optional<EntityId> best;
  std::size_t best_len = 0;
  for (const auto& [id, ref] : t.actors->edges()) {
    if (ref.z != from.z || t.items->def(t.items->get(id).def).family != "window") continue;
    const Cell inner{ref.z, ref.x, ref.y};
    const Cell outer = ref.vertical ? Cell{ref.z, ref.x - 1, ref.y} : Cell{ref.z, ref.x, ref.y - 1};
    for (const Cell& c : {inner, outer}) {
      if (!t.map->in_bounds(c) || !t.map->walkable(c) || t.map->room_of(c) < 0) continue;
      if (!t.map->rooms()[static_cast<std::size_t>(t.map->room_of(c))].indoor) continue;  // work from inside
      const auto path = t.map->find_path(from, c);
      if (!path.empty() && (!best || path.size() < best_len)) {
        best = id;
        best_len = path.size();
      }
    }
  }
  return best;
}

constexpr const char* kSteps[] = {
    "drink at the sink", "eat from the pantry", "pick up the bandage", "pick up the disinfectant",
    "disinfect the wound", "dress the wound",   "pick up the hammer",  "pick up the nails",
    "break the chair apart", "pick up a plank", "pick up a plank",     "board the window",
    "board the window",   "done"};
constexpr int kStepCount = 13;

}  // namespace

const char* ScriptedPop::step_name(int step) { return kSteps[std::clamp(step, 0, kStepCount)]; }

EntityId wound_player(Tier0& t) {
  Wound w;
  w.region = Region::ForearmL;
  w.type = WoundType::Laceration;
  w.depth = Depth::Fat;
  w.length_mm = 40;
  w.contamination = 0.8;
  return t.bodies->add_wound(*t.world, t.player, w, 0);
}

bool ScriptedPop::drive(Tier0& t) {
  if (!t.actors->idle(t.player)) return true;
  if (step_ >= kStepCount || !t.bodies->get(t.player).alive) return false;
  Actors& ax = *t.actors;
  auto use = [&](const char* process, std::optional<EntityId> target) {
    Command c;
    c.type = static_cast<std::uint16_t>(CommandType::Use);
    c.subject = t.player;
    c.a = static_cast<std::int64_t>(ax.find_process(process).value());
    c.b = static_cast<std::int64_t>(target.value_or(0));
    if (as_ai_) t.world->queue_intent(c);
    else t.world->submit(c);
  };
  auto wound = [&]() -> std::optional<EntityId> {
    const auto& ws = t.bodies->get(t.player).wounds;
    if (ws.empty()) return std::nullopt;
    return ws.front().id;
  };
  switch (step_) {
    case 0: use("drink_tap", ax.find_object("sink")); break;
    case 1: use("eat", ax.find_object("pantry")); break;
    case 2: use("take", loose_item(t, "bandage")); break;
    case 3: use("take", loose_item(t, "disinfectant_bottle")); break;
    case 4: use("disinfect_wound", wound()); break;
    case 5: use("dress_wound", wound()); break;
    case 6: use("take", loose_item(t, "hammer")); break;
    case 7: use("take", loose_item(t, "nail_set")); break;
    case 8: use("salvage_furniture", loose_item(t, "chair")); break;
    case 9:
    case 10: use("take", loose_item(t, "plank")); break;
    case 11:
    case 12: use("barricade", nearest_window(t)); break;
  }
  ++step_;
  return true;
}

// ---- Stage 6 scenario ------------------------------------------------------------------------------

std::optional<EntityId> GeneratorScenario::front_window(const Tier0& t) {
  const auto lamp = t.actors->find_object("lamp", "front_room");
  if (!lamp) return std::nullopt;
  const int space = t.power->space_of(*t.map->find_object("lamp", "front_room"));
  for (const auto& [id, ref] : t.actors->edges()) {
    if (t.items->def(t.items->get(id).def).family != "window") continue;
    const Cell inner{ref.z, ref.x, ref.y};
    const Cell outer = ref.vertical ? Cell{ref.z, ref.x - 1, ref.y} : Cell{ref.z, ref.x, ref.y - 1};
    if (t.power->space_of(inner) == space || t.power->space_of(outer) == space) return id;
  }
  return std::nullopt;
}

void GeneratorScenario::use(Tier0& t, const char* process, std::optional<EntityId> target) {
  Command c;
  c.type = static_cast<std::uint16_t>(CommandType::Use);
  c.subject = t.player;
  c.a = static_cast<std::int64_t>(t.actors->find_process(process).value());
  c.b = static_cast<std::int64_t>(target.value_or(0));
  if (as_ai_) t.world->queue_intent(c);
  else t.world->submit(c);
}

bool GeneratorScenario::drive(Tier0& t) {
  World& w = *t.world;
  Power& p = *t.power;
  const Body& b = t.bodies->get(t.player);
  if (!b.alive || b.spec.walker || w.clock.day() >= 10) return false;
  for (EventId id : w.events_this_tick())
    if (w.log.get(id)->type == EventType::FoodSpoiled && out_twice_) return false;
  if (t.walkers && !b.activity.asleep) {
    if (!armed_) {
      armed_ = true;
      if (auto bar = loose_item(t, "crowbar")) todo_.push_front({"take", *bar});
    }
    // Fight back: the nearest walker within reach gets the crowbar (a stand-in for the pop AI, stage 8).
    const Cell me = t.actors->actor(t.player).pos;
    EntityId near = 0;
    for (const auto& [id, z] : t.walkers->walkers())
      if (t.bodies->get(id).alive && t.walkers->in_reach(me, z.pos)) near = id;
    if (near) {
      const Action& act = t.actors->actor(t.player).action;
      const auto strike = t.actors->find_process("strike_walker");
      if (!t.actors->idle(t.player) && act.process != static_cast<int>(*strike)) {
        Command c;
        c.type = static_cast<std::uint16_t>(CommandType::Cancel);
        c.subject = t.player;
        if (as_ai_) w.queue_intent(c);
        else w.submit(c);
      } else if (t.actors->idle(t.player)) {
        use(t, "strike_walker", near);
      }
      return true;
    }
  }
  if (!t.actors->idle(t.player) || b.activity.asleep) return true;
  if (!todo_.empty()) {
    const auto [process, target] = todo_.front();
    todo_.pop_front();
    use(t, process, target);
    return true;
  }
  Actors& ax = *t.actors;
  const EntityId gen = p.generator_object();
  if (w.clock.tick >= next_meal_) {
    next_meal_ = w.clock.tick + 8 * kTicksPerGameHour;
    todo_.push_back({"drink_tap", *ax.find_object("sink")});
    todo_.push_back({"eat", *ax.find_object("pantry")});
  } else if (!p.mains_alive() && !set_up_) {
    set_up_ = true;
    todo_.push_back({"take", *loose_item(t, "fuel_can")});
    todo_.push_back({"take", *loose_item(t, "blanket")});
    todo_.push_back({"switch_to_generator", *ax.find_object("power_panel")});
    todo_.push_back({"start_generator", gen});
    todo_.push_back({"switch_on", *ax.find_object("lamp", "front_room")});
    todo_.push_back({"hang_curtain", *front_window(t)});
  } else if (set_up_ && !refueled_ && p.fuel_ul(w) < 1'500'000) {
    refueled_ = true;
    todo_.push_back({"refuel_generator", gen});
    if (!p.generator_running()) todo_.push_back({"start_generator", gen});
  } else if (refueled_ && !p.generator_running() && p.fuel_ul(w) == 0) {
    out_twice_ = true;  // now wait for the fridge
  } else {
    const double h = w.clock.hour();
    // Days run 08:00 to 08:00, so one night is one day().
    if ((h >= 23 || h < 6) && slept_day_ != w.clock.day()) {
      slept_day_ = w.clock.day();
      todo_.push_back({"sleep", t.player});
    }
  }
  return true;
}

std::string data_dir() {
  if (const char* env = std::getenv("ZS_DATA")) return env;
  if (std::filesystem::exists(ZS_DATA_DIR)) return ZS_DATA_DIR;  // a build next to its source
  return "data";  // a packaged build: the data folder sits beside the program
}

std::string time_text(std::int64_t tick) {
  Clock c;
  c.tick = tick;
  const double h = c.hour();
  char buf[32];
  std::snprintf(buf, sizeof buf, "day %lld %02d:%02d", static_cast<long long>(c.calendar_day()), static_cast<int>(h),
                static_cast<int>((h - static_cast<int>(h)) * 60));
  return buf;
}

std::string death_sentence(const Tier0& t, const Event& death) {
  auto name = [&](EntityId b) { return t.census ? t.census->name_of(b) : std::string("someone"); };
  const Body& b = t.bodies->get(death.subject);
  std::string s = name(death.subject) + ", " + std::to_string(static_cast<int>(b.spec.age)) +
                  (death.b ? ", walker, was put down " : ", died ") + time_text(death.tick) + ": " +
                  to_string(static_cast<DeathCause>(death.a)) + ".";
  // Walk the cause chain to the first bite or strike.
  for (EventId id = death.cause; id != 0;) {
    const Event* e = t.world->log.get(id);
    if (!e) break;
    if (e->type == EventType::Bite) {
      s += " Bitten on the " + std::string(to_string(static_cast<Region>(e->b))) + " by " + name(e->subject) + ", " +
           time_text(e->tick) + ".";
      break;
    }
    if (e->type == EventType::Hit) {
      s += " Struck by " + name(e->subject) + ".";
      break;
    }
    id = e->cause;
  }
  return s;
}


namespace {

const Event* first_bite(const Tier0& t, const Event& death) {
  for (EventId id = death.cause; id != 0;) {
    const Event* e = t.world->log.get(id);
    if (!e) break;
    if (e->type == EventType::Bite) return e;
    id = e->cause;
  }
  return nullptr;
}

// Was the generator running at `tick`? Read from the log (start actions; stops, stalls, dry tank).
bool generator_ran_at(const Tier0& t, std::int64_t tick) {
  const auto start = t.actors->find_process("start_generator"), stop = t.actors->find_process("stop_generator");
  bool on = false;
  for (const Event& e : t.world->log.all()) {
    if (e.tick > tick) break;
    if (e.type == EventType::ActionDone && start && e.a == static_cast<std::int64_t>(*start)) on = true;
    if ((e.type == EventType::ActionDone && stop && e.a == static_cast<std::int64_t>(*stop)) ||
        e.type == EventType::GeneratorStalled || e.type == EventType::GeneratorOutOfFuel)
      on = false;
  }
  return on;
}

// Bitten, lived long enough to say so (a few decision steps), and never did.
bool hid_bite(const Tier0& t, const Event& death) {
  constexpr std::int64_t kTimeToTell = kTicksPerGameHour / 12;  // 5 minutes
  std::int64_t first = -1;
  for (const Event& e : t.world->log.all()) {
    if (e.type == EventType::BiteReported && e.subject == death.subject) return false;
    if (first < 0 && e.type == EventType::Bite && static_cast<EntityId>(e.a) == death.subject) first = e.tick;
  }
  return first >= 0 && death.tick - first >= kTimeToTell;
}

const Event* arrival_of(const Tier0& t, EntityId walker, std::int64_t before) {
  const Event* found = nullptr;
  for (const Event& e : t.world->log.all()) {
    if (e.tick > before) break;
    if (e.type == EventType::WalkerArrived && e.subject == walker) found = &e;
  }
  return found;
}

}  // namespace

std::vector<std::string> danger_tags(const Tier0& t, const Event& death) {
  std::vector<std::string> tags;
  const auto cause = static_cast<DeathCause>(death.a);
  const Event* bite = first_bite(t, death);
  if (bite) tags.push_back("walker attack");
  if (bite && hid_bite(t, death)) tags.push_back("hidden bite");
  if (cause == DeathCause::Exsanguination) tags.push_back("bleeding");
  if (cause == DeathCause::Sepsis) tags.push_back("infected wound");
  if (cause == DeathCause::Dehydration) tags.push_back("dehydration");
  if (cause == DeathCause::Starvation) tags.push_back("starvation");
  if (bite)
    if (const Event* came = arrival_of(t, bite->subject, bite->tick); came && generator_ran_at(t, came->tick))
      tags.push_back("noise");
  return tags;
}

std::vector<std::string> missed_counters(const Tier0& t, const Event& death) {
  std::vector<std::string> out;
  const Event* bite = first_bite(t, death);
  if (bite && hid_bite(t, death)) out.push_back("the bite was never told to the group");
  const Body& b = t.bodies->get(death.subject);
  const auto cause = static_cast<DeathCause>(death.a);
  if (cause == DeathCause::Sepsis || cause == DeathCause::Exsanguination)
    for (const Wound& wd : b.wounds)
      if (wd.cause == death.cause || (bite && wd.cause == bite->id)) {
        if (cause == DeathCause::Sepsis && wd.cleaned == Cleaned::No) out.push_back("the wound was never cleaned");
        if (cause == DeathCause::Exsanguination && !wd.dressed && !wd.tourniquet) out.push_back("the bleeding was never dressed");
        break;
      }
  if (bite)
    if (const Event* came = arrival_of(t, bite->subject, bite->tick); came && generator_ran_at(t, came->tick))
      out.push_back("the generator was running when " + (t.census ? t.census->name_of(bite->subject) : std::string("it")) + " arrived");
  return out;
}

std::optional<EventLine> event_line(const Tier0& t, const Event& e) {
  auto name = [&](EntityId b) { return t.census ? t.census->name_of(b) : std::string(b == t.player ? "You" : "someone"); };
  std::string line;
  const char* kind = "alert";
  switch (e.type) {
    case EventType::GridFailed: line = "The grid failed"; break;
    case EventType::GeneratorOutOfFuel: line = "The generator ran out of fuel"; break;
    case EventType::Joined: line = name(e.subject) + " joined the group"; break;
    case EventType::StrangerArrived: line = name(e.subject) + " is at the yard gate"; break;
    case EventType::Searched: line = name(e.subject) + " searched a house"; break;
    case EventType::RadioRepaired: line = name(e.subject) + " repaired the radio"; break;
    case EventType::RadioStatic: line = name(e.subject) + " listened to the radio: static"; kind = "body"; break;
    case EventType::RadioHeard: line = name(e.subject) + " heard the evacuation broadcast: it names the pickup"; break;
    case EventType::ControlPassed: line = "Control passes to " + name(e.subject); break;
    case EventType::RunEnded: line = std::string("The run ends: ") + to_string(static_cast<RunOutcome>(e.a)); break;
    case EventType::FoodSpoiled: line = "The fridge food spoiled"; break;
    case EventType::Death: line = death_sentence(t, e); kind = "dead"; break;
    case EventType::WalkerArrived: line = "A walker (" + name(e.subject) + ") arrived"; kind = "dead"; break;
    case EventType::Bite:
      line = name(e.subject) + " bit " + name(static_cast<EntityId>(e.a)) + " (" + to_string(static_cast<Region>(e.b)) + ")";
      break;
    case EventType::Reanimated: line = name(e.subject) + " rose as a walker"; break;
    case EventType::EdgeBreached: line = "The " + t.items->def(t.items->get(e.subject).def).id + " was broken in"; break;
    case EventType::PopRefused:
      line = name(e.subject) + " refused (" + to_string(static_cast<Doing>(e.b)) + "): " + to_string(static_cast<Refusal>(e.a));
      kind = "body";
      break;
    case EventType::ActionRefused:
      if (e.subject != t.player && !(t.run && e.subject == t.run->controlled())) return std::nullopt;
      line = "Can't " + (e.a >= 0 && static_cast<std::size_t>(e.a) < t.actors->processes().size()
                             ? t.actors->processes()[static_cast<std::size_t>(e.a)].name : std::string("do that")) +
             ": " + to_string(static_cast<Reason>(e.b));
      kind = "body";
      break;
    case EventType::Panicked: line = name(e.subject) + " panicked and ran"; kind = "body"; break;
    case EventType::BiteReported: line = name(e.subject) + " told the others: \"I've been bitten\""; break;
    case EventType::Hit: {
      const EntityId v = static_cast<EntityId>(e.a);
      if (!e.b || t.bodies->get(v).spec.walker) return std::nullopt;  // walkers put down have their own line
      line = name(e.subject) + " made sure " + (v == t.player ? std::string("you stay") : name(v) + " stays") + " down";
      kind = "dead";
      break;
    }
    case EventType::ActionDone: {
      const auto& procs = t.actors->processes();
      if (e.a < 0 || static_cast<std::size_t>(e.a) >= procs.size()) return std::nullopt;
      const std::string& id = procs[static_cast<std::size_t>(e.a)].id;
      if (id != "barricade" && id != "refuel_generator" && id != "start_generator" && id != "switch_to_generator" &&
          id != "shout" && id != "dress_wound")
        return std::nullopt;
      line = name(e.subject) + ": " + procs[static_cast<std::size_t>(e.a)].name;
      kind = "done";
      break;
    }
    default: return std::nullopt;
  }
  return EventLine{line, kind};
}

std::vector<std::string> day_summary(const Tier0& t, int day) {
  auto name = [&](EntityId b) { return t.census ? t.census->name_of(b) : std::string("someone"); };
  struct Line { int score; std::int64_t tick; std::string text; };
  std::vector<Line> lines;
  // Calendar days: tick 0 is 08:00 on day 0.
  const std::int64_t from = std::max<std::int64_t>(0, day * kTicksPerGameDay - kStartTimeOfDayTicks),
                     to = (day + 1) * kTicksPerGameDay - kStartTimeOfDayTicks;
  for (const Event& e : t.world->log.all()) {
    if (e.tick < from) continue;
    if (e.tick >= to) break;
    switch (e.type) {
      case EventType::Death:
        lines.push_back({e.b ? 1 : 10, e.tick, e.b ? name(e.subject) + " was put down" : death_sentence(t, e)});
        break;
      case EventType::Joined: lines.push_back({6, e.tick, name(e.subject) + " joined the group"}); break;
      case EventType::RadioHeard: lines.push_back({8, e.tick, name(e.subject) + " heard where the pickup is"}); break;
      case EventType::RadioRepaired: lines.push_back({5, e.tick, name(e.subject) + " repaired the radio"}); break;
      case EventType::StrangerArrived: lines.push_back({5, e.tick, name(e.subject) + " came to the yard gate"}); break;
      case EventType::EdgeBreached: lines.push_back({4, e.tick, "a door or window was broken in"}); break;
      case EventType::GridFailed: lines.push_back({5, e.tick, "the grid failed"}); break;
      case EventType::GeneratorOutOfFuel: lines.push_back({5, e.tick, "the generator ran out of fuel"}); break;
      case EventType::Bite:
        lines.push_back({3, e.tick, name(e.subject) + " bit " + name(static_cast<EntityId>(e.a))});
        break;
      default: break;
    }
  }
  std::stable_sort(lines.begin(), lines.end(), [](const Line& a, const Line& b) { return a.score > b.score; });
  if (lines.size() > 5) lines.resize(5);
  std::stable_sort(lines.begin(), lines.end(), [](const Line& a, const Line& b) { return a.tick < b.tick; });
  std::vector<std::string> out;
  for (const Line& l : lines) out.push_back(time_text(l.tick).substr(time_text(l.tick).find(' ', 4) + 1) + "  " + l.text);
  return out;
}

}  // namespace zs::tier0
