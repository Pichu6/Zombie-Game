// The Tier 0 world builder. Spec: docs/tier0/specs/05-clock.md rule 1 (update order).
#include "zs/tier0.h"

#include <algorithm>
#include <cstdio>

namespace zs::tier0 {

Tier0 make_world(std::uint64_t seed, bool with_dead) {
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
  if (with_dead) {
    census = std::make_unique<Census>(w, *map, *items, *bodies, *power);
    walkers = std::make_unique<Walkers>(w, *map, *items, *bodies, *actors, *power, *census);
  }
  t.items = items.get();
  t.map = map.get();
  t.bodies = bodies.get();
  t.actors = actors.get();
  t.power = power.get();
  t.census = census.get();
  t.walkers = walkers.get();
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
  const std::string data = ZS_DATA_DIR;
  load_item_defs(*t.items, data + "/tier0/items.toml");
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
  w.close_setup();
  return t;
}

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
      if (t.bodies->get(id).alive && z.pos.z == me.z && std::abs(z.pos.x - me.x) + std::abs(z.pos.y - me.y) <= 1) near = id;
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

}  // namespace zs::tier0
