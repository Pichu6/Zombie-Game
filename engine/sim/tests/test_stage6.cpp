// Stage 6: the generator problem (specs 17, 18, 19). Power, fuel, the fridge, noise and light.
#include <doctest.h>

#include <algorithm>

#include "zs/noise.h"
#include "zs/tier0.h"

using namespace zs;
using namespace zs::tier0;

namespace {

int count(const World& w, EventType type) {
  int n = 0;
  for (const Event& e : w.log.all()) n += e.type == type;
  return n;
}

EntityId object(const Tier0& t, const char* kind, const char* id = "") { return t.actors->find_object(kind, id).value(); }

EntityId item_of(const Tier0& t, const char* def_id) {
  for (const auto& [id, inst] : t.items->all())
    if (t.items->def(inst.def).id == def_id) return id;
  return 0;
}

// Grid down, panel on the generator, generator running.
void on_generator(Tier0& t) {
  t.power->fail_grid(*t.world, 0);
  t.power->set_transfer(*t.world, Transfer::Generator, 0);
  REQUIRE(t.power->start_generator(*t.world, 0));
}

void all_lamps(Tier0& t, bool on) {
  for (const Receiver& r : t.power->receivers())
    if (t.power->receiver_defs()[r.def].kind == "lamp") t.power->set_on(*t.world, r.object, on, 0);
}

// The kitchen lamp's walled space and a window of it.
EntityId kitchen_window(const Tier0& t) {
  const int space = t.power->space_of(*t.map->find_object("lamp", "kitchen"));
  for (const auto& [id, ref] : t.actors->edges()) {
    if (t.items->def(t.items->get(id).def).family != "window") continue;
    const Cell inner{ref.z, ref.x, ref.y};
    const Cell outer = ref.vertical ? Cell{ref.z, ref.x - 1, ref.y} : Cell{ref.z, ref.x, ref.y - 1};
    if (t.power->space_of(inner) == space || t.power->space_of(outer) == space) return id;
  }
  return 0;
}

double seen_m(const Tier0& t, EntityId window) {
  for (const WindowLeak& l : t.power->window_leaks(*t.world))
    if (l.window == window) return l.seen_m;
  return -1;
}

constexpr std::int64_t kNight = 15 * kTicksPerGameHour;  // 23:00 on day 0

}  // namespace

TEST_CASE("stage 6: the 3 kW generator runs fridge, lamps and hot plate; the saw stalls it (spec 17, A1)") {
  Tier0 t = make_world(1);
  on_generator(t);
  all_lamps(t, true);
  t.power->set_on(*t.world, object(t, "hot_plate"), true, 0);
  CHECK(t.power->generator_running());
  CHECK(t.power->powered(object(t, "fridge")));
  CHECK(t.power->powered(object(t, "hot_plate")));
  CHECK(t.power->load_w() == doctest::Approx(150 + 50 + 1200));
  t.power->set_on(*t.world, object(t, "saw"), true, 0);
  CHECK_FALSE(t.power->generator_running());
  CHECK(count(*t.world, EventType::GeneratorStalled) == 1);
  for (const Receiver& r : t.power->receivers()) CHECK_FALSE(r.powered);
}

TEST_CASE("stage 6: radio and saw together trip their breaker; reset brings it back (spec 17)") {
  Tier0 t = make_world(1);
  t.power->set_on(*t.world, object(t, "radio"), true, 0);
  t.power->set_on(*t.world, object(t, "saw"), true, 0);
  CHECK(count(*t.world, EventType::BreakerTripped) == 1);
  CHECK_FALSE(t.power->powered(object(t, "radio")));
  CHECK(t.power->powered(object(t, "fridge")));  // other circuits keep going
  t.power->set_on(*t.world, object(t, "saw"), false, 0);
  CHECK(t.power->reset_breakers(*t.world, 0));
  CHECK(t.power->powered(object(t, "radio")));
}

TEST_CASE("stage 6: fuel at ~50 % load over 10 h lands in the spec-sheet band (spec 17, A3)") {
  Tier0 t = make_world(1);
  t.world->check_every_tick = false;
  World& w = *t.world;
  // Fill the tank from the can.
  const OwnerId tank = *t.actors->store_of(t.power->generator_object());
  const OwnerId can = *t.actors->store_of(item_of(t, "fuel_can"));
  const auto gas = static_cast<ResourceId>(std::find(w.ledger.resource_names().begin(), w.ledger.resource_names().end(),
                                                     std::string("gasoline_ul")) - w.ledger.resource_names().begin());
  w.ledger.transfer(0, can, tank, gas, t.power->tank_ul() - w.ledger.balance(tank, gas), 0);
  CHECK(t.power->fuel_ul(w) == t.power->tank_ul());
  on_generator(t);
  all_lamps(t, true);
  t.power->set_on(w, object(t, "hot_plate"), true, 0);
  t.power->set_on(w, object(t, "radio"), true, 0);
  const double load = t.power->load_w() / t.power->generator().capacity_w;
  CHECK(load == doctest::Approx(0.47).epsilon(0.02));
  w.run_ticks(10 * kTicksPerGameHour);
  CHECK(t.power->generator_running());
  const double used_l = static_cast<double>(t.power->tank_ul() - t.power->fuel_ul(w)) / 1e6;
  CHECK(used_l > 10.0);  // ~12 h on a 15 L tank at 50 % (spec sheets)
  CHECK(used_l < 15.0);
  CHECK(used_l == doctest::Approx(10 * t.power->fuel_lph()).epsilon(0.01));
  CHECK_FALSE(w.ledger.check().has_value());
}

TEST_CASE("stage 6: grid failure is drawn in [day 2, day 5) and cuts every mains receiver in one solve (spec 17, A1)") {
  Tier0 t = make_world(7);
  const std::int64_t at = t.power->grid_failure_tick();
  CHECK(at >= 2 * kTicksPerGameDay);
  CHECK(at < 5 * kTicksPerGameDay);
  bool scheduled = false;
  for (const auto& [key, e] : t.world->scheduler.pending()) scheduled |= e.type == EventType::GridFailed && e.tick == at;
  CHECK(scheduled);
  all_lamps(t, true);
  t.power->set_on(*t.world, object(t, "radio"), true, 0);
  int lit = 0;
  for (const Receiver& r : t.power->receivers()) lit += r.powered;
  CHECK(lit == 7);  // fridge, radio, five lamps
  t.power->fail_grid(*t.world, 0);
  for (const Receiver& r : t.power->receivers()) CHECK_FALSE(r.powered);
  CHECK(count(*t.world, EventType::GridFailed) == 1);
}

TEST_CASE("stage 6: the generator is heard 200 m away at night only once the grid is down (spec 18, A3)") {
  Tier0 t = make_world(1);
  const NoiseSource gen{t.power->generator_cell(), t.power->generator().noise_db_1m};
  const double before = audible_range_m(*t.map, *t.items, gen, background_db(true, 23));
  const double after = audible_range_m(*t.map, *t.items, gen, background_db(false, 23));
  CHECK(before < 200);
  CHECK(before == doctest::Approx(112).epsilon(0.02));
  CHECK(after > 200);
}

TEST_CASE("stage 6: closing a door lowers the level beyond it by the door's loss (spec 18, A1)") {
  Tier0 t = make_world(1);
  // A basement door between the two basement rooms.
  std::optional<EdgeRef> door;
  for (const auto& [id, ref] : t.actors->edges())
    if (ref.z == -1 && t.items->def(t.items->get(id).def).family == "door") door = ref;
  REQUIRE(door);
  const Cell a{door->z, door->x, door->y};
  const Cell b = door->vertical ? Cell{door->z, door->x - 1, door->y} : Cell{door->z, door->x, door->y - 1};
  const std::vector<NoiseSource> src{{a, 80}};
  t.map->set_open(*door, true);
  const double open = noise_at(*t.map, noise_field(*t.map, *t.items, src), b);
  t.map->set_open(*door, false);
  const double closed = noise_at(*t.map, noise_field(*t.map, *t.items, src), b);
  const double loss = t.items->def(t.items->get(t.map->edge(*door).assembly).def).sound_loss_db;
  CHECK(loss > 0);
  CHECK(open - closed == doctest::Approx(loss));
}

TEST_CASE("stage 6: hammering carries farther than footsteps (spec 18, A3)") {
  Tier0 t = make_world(1);
  const Cell at = t.actors->actor(t.player).pos;
  const Body& b = t.bodies->get(t.player);
  const double steps = t.bodies->footstep_db(b, t.bodies->walk_speed(b, 0), 0);
  const double hammer = primitive_info(Primitive::Strike).noise_db;
  const double bg = background_db(false, 23);
  CHECK(audible_range_m(*t.map, *t.items, {at, hammer}, bg) > 3 * audible_range_m(*t.map, *t.items, {at, steps}, bg));
}

TEST_CASE("stage 6: a lit window shows at 50 m at night; curtained only close; boarded not at all (spec 19, A3)") {
  Tier0 t = make_world(1);
  t.world->clock.tick = kNight;
  const EntityId window = kitchen_window(t);
  REQUIRE(window != 0);
  CHECK(seen_m(t, window) == 0);  // dark kitchen
  t.power->set_on(*t.world, object(t, "lamp", "kitchen"), true, 0);
  CHECK(seen_m(t, window) > 50);
  // In daylight the same window never stands out.
  t.world->clock.tick = 4 * kTicksPerGameHour;
  CHECK(seen_m(t, window) == 0);
  t.world->clock.tick = kNight;
  // A blanket over it.
  const EntityId blanket = item_of(t, "blanket");
  t.world->locations.place(blanket, window);
  const double curtained = seen_m(t, window);
  CHECK(curtained > 0);
  CHECK(curtained < 20);
  t.world->locations.place(blanket, t.actors->floor_at(t.actors->actor(t.player).pos));
  // Three boards nailed over it.
  Assemblies& items = *t.items;
  const EntityId bar = items.create(*t.world, *items.find_def("barricade"), 50, window);
  t.map->set_barricade(t.actors->edges().at(window), bar);
  for (std::size_t slot = 1; slot <= 3; ++slot) {
    const EntityId plank = items.create(*t.world, *items.find_def("plank"), 50, t.actors->floor_at({0, 1, 1}));
    REQUIRE(items.mount(*t.world, bar, slot, plank));
  }
  CHECK(seen_m(t, window) == 0);
}

TEST_CASE("stage 6: work in the dark is slower than in daylight (spec 19, A3)") {
  Tier0 t = make_world(1);
  const std::size_t salvage = *t.actors->find_process("salvage_furniture");
  const EntityId chair = item_of(t, "chair");
  t.world->locations.place(item_of(t, "crowbar"), t.player);
  const std::size_t eat = *t.actors->find_process("eat");
  const double day = t.actors->preview(*t.world, t.player, salvage, chair).time_s;
  const double eat_day = t.actors->preview(*t.world, t.player, eat, object(t, "pantry")).time_s;
  t.world->clock.tick = kNight;
  const double night = t.actors->preview(*t.world, t.player, salvage, chair).time_s;
  CHECK(night == doctest::Approx(2 * day));
  // Eating doesn't need light.
  CHECK(t.actors->preview(*t.world, t.player, eat, object(t, "pantry")).time_s == doctest::Approx(eat_day));
}

TEST_CASE("stage 6: an unpowered fridge spoils its food within 8-36 h; a cold one keeps it 5+ days (spec 17)") {
  {
    Tier0 t = make_world(1);
    t.world->check_every_tick = false;
    t.power->fail_grid(*t.world, 0);
    while (count(*t.world, EventType::FoodSpoiled) == 0 && t.world->clock.tick < 3 * kTicksPerGameDay)
      t.world->run_ticks(kTicksPerGameHour);
    const double hours = static_cast<double>(t.world->clock.tick) / kTicksPerGameHour;
    CHECK(hours > 8);
    CHECK(hours < 36);
  }
  {
    Tier0 t = make_world(1);
    t.world->check_every_tick = false;
    t.world->run_ticks(kTicksPerGameDay);  // the grid fails no earlier than day 2
    CHECK(t.power->fridge_temp_c() == doctest::Approx(3).epsilon(0.01));
    CHECK(5 * t.power->spoil_progress() < 1);
  }
}

TEST_CASE("stage 6: the scenario through actions: refuels once, runs dry, the food spoils; save/load keeps the hash") {
  Tier0 t = make_world(1);
  t.world->check_every_tick = false;
  GeneratorScenario script(false);
  std::vector<std::uint8_t> mid;
  GeneratorScenario at_mid = script;
  while (script.drive(t)) {
    t.world->step();
    if (mid.empty() && t.power->generator_running()) {
      mid = t.world->save();
      at_mid = script;
    }
  }
  CHECK(script.refueled());
  CHECK(count(*t.world, EventType::GridFailed) == 1);
  CHECK(count(*t.world, EventType::GeneratorOutOfFuel) == 1);
  CHECK(count(*t.world, EventType::FoodSpoiled) == 1);
  CHECK(count(*t.world, EventType::ActionRefused) == 0);
  CHECK(count(*t.world, EventType::ActionInterrupted) == 0);
  CHECK(t.actors->covered(*t.world, *GeneratorScenario::front_window(t)));
  CHECK_FALSE(t.world->ledger.check().has_value());
  // Replaying from the save reaches the same end.
  Tier0 u = make_world(1);
  u.world->check_every_tick = false;
  u.world->load(mid);
  while (at_mid.drive(u)) u.world->step();
  CHECK(u.world->world_hash() == t.world->world_hash());
  const std::uint64_t golden = 2846288229969204348ULL;
  CHECK(t.world->world_hash() == golden);
  MESSAGE("stage 6 golden world hash: " << t.world->world_hash());
}
