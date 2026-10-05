// The Tier 0 world builder. Spec: docs/tier0/specs/05-clock.md rule 1 (update order).
#include "zs/tier0.h"

#include <algorithm>

namespace zs::tier0 {

Tier0 make_world(std::uint64_t seed) {
  Tier0 t;
  t.world = std::make_unique<World>(seed);
  World& w = *t.world;
  auto items = std::make_unique<Assemblies>();
  auto map = std::make_unique<SiteMap>();
  auto bodies = std::make_unique<Bodies>(w);  // registers its ledger resources first
  auto actors = std::make_unique<Actors>(w, *items, *map, *bodies);
  auto power = std::make_unique<Power>(w, *map, *items, *actors);
  t.items = items.get();
  t.map = map.get();
  t.bodies = bodies.get();
  t.actors = actors.get();
  t.power = power.get();
  // Order: things and places, then the executor and movement, power, then the body.
  w.add_system(std::move(items));
  w.add_system(std::move(map));
  w.add_system(std::move(actors));
  w.add_system(std::move(power));
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
  if (!b.alive || w.clock.day() >= 10) return false;
  for (EventId id : w.events_this_tick())
    if (w.log.get(id)->type == EventType::FoodSpoiled && out_twice_) return false;
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

}  // namespace zs::tier0
