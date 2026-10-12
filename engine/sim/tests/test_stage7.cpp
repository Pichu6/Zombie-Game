// Stage 7: the dead (specs 10, 20, 21, 22, 23). Census, walkers, pushes, bites, turning, strikes.
#include <doctest.h>

#include <algorithm>
#include <set>

#include "zs/tier0.h"

using namespace zs;
using namespace zs::tier0;

namespace {

int count(const World& w, EventType type) {
  int n = 0;
  for (const Event& e : w.log.all()) n += e.type == type;
  return n;
}

EntityId item_of(const Tier0& t, const char* def_id) {
  for (const auto& [id, inst] : t.items->all())
    if (t.items->def(inst.def).id == def_id) return id;
  return 0;
}

Cell player_cell(const Tier0& t) { return t.actors->actor(t.player).pos; }

// Puts an embodied census person on the map as a walker at `at` (an off-map one arrives).
void place(Tier0& t, EntityId body, const Cell& at) {
  const std::size_t p = *t.census->person_of(body);
  if (t.census->people()[p].fate == Fate::WalkerOffMap) t.census->set_fate(*t.world, p, Fate::WalkerOnMap);
  t.walkers->spawn(*t.world, body, at);
}

// The bodies of embodied census people still off the map (walkers that have not arrived).
std::vector<EntityId> off_map(const Tier0& t) {
  std::vector<EntityId> out;
  for (const Person& p : t.census->people())
    if (p.fate == Fate::WalkerOffMap) out.push_back(p.body);
  return out;
}

// Embodied adults, sorted by a key.
template <class Key>
std::vector<EntityId> adults_by(const Tier0& t, Key key) {
  std::vector<EntityId> out;
  for (const Person& p : t.census->people())
    if (p.body && p.age >= 18 && p.fate != Fate::Pop) out.push_back(p.body);
  std::sort(out.begin(), out.end(), [&](EntityId a, EntityId b) { return key(a) < key(b); });
  return out;
}

std::optional<EdgeRef> edge_of_family(const Tier0& t, const char* family) {
  for (const auto& [id, ref] : t.actors->edges())
    if (t.items->def(t.items->get(id).def).family == family) return ref;
  return std::nullopt;
}

// Seconds of a steady push until the edge breaches (capped).
double breach_s(Tier0& t, const EdgeRef& e, double load_n) {
  double s = 0;
  while (s < 1e5 && !t.walkers->push_edge(*t.world, e, load_n, 1.0, 0)) s += 1;
  return s;
}

void run_hours(Tier0& t, double h) {
  const auto end = t.world->clock.tick + static_cast<std::int64_t>(h * kTicksPerGameHour);
  while (t.world->clock.tick < end) t.world->step();
}

}  // namespace

TEST_CASE("stage 7: the census conserves people through arrivals, deaths and rising (spec 22, A1)") {
  Tier0 t = make_world(4, true);  // seed 4 since decision 0065: the draws are keyed by entity id, and seed 1's scenario no longer bites in the window
  t.world->check_every_tick = false;
  GeneratorScenario script(false);
  while (script.drive(t)) t.world->step();
  run_hours(t, 2);  // the pop died of its bite; it rises within the hour
  CHECK(count(*t.world, EventType::WalkerArrived) > 0);
  CHECK(count(*t.world, EventType::Reanimated) == 1);
  std::int64_t total = 0;
  for (std::size_t f = 0; f < kFateCount; ++f) {
    const auto fate = static_cast<Fate>(f);
    const auto n = std::count_if(t.census->people().begin(), t.census->people().end(),
                                 [&](const Person& p) { return p.fate == fate; });
    CHECK(t.census->count(*t.world, fate) == n);
    total += t.census->count(*t.world, fate);
  }
  CHECK(total == static_cast<std::int64_t>(t.census->people().size()));
  CHECK_FALSE(t.world->ledger.check().has_value());
}

TEST_CASE("stage 7: generator hours raise attention and arrivals (spec 22 rule 3, A3)") {
  double att_quiet = 0, att_gen = 0;
  int arr_quiet = 0, arr_gen = 0;
  for (std::uint64_t seed = 1; seed <= 6; ++seed)
    for (bool gen : {false, true}) {
      Tier0 t = make_world(seed, true);
      t.world->check_every_tick = false;
      t.power->fail_grid(*t.world, 0);
      if (gen) {
        t.power->set_transfer(*t.world, Transfer::Generator, 0);
        REQUIRE(t.power->start_generator(*t.world, 0));
      }
      run_hours(t, 24);
      (gen ? att_gen : att_quiet) += t.census->attention();
      (gen ? arr_gen : arr_quiet) += count(*t.world, EventType::WalkerArrived);
    }
  MESSAGE("attention quiet " << att_quiet << " generator " << att_gen << "; arrivals " << arr_quiet << " vs " << arr_gen);
  CHECK(att_gen > att_quiet);
  CHECK(arr_gen > arr_quiet);
}

TEST_CASE("stage 7: more walkers on a door breach it sooner (spec 10, A1)") {
  double s[3];
  const double one = 400;  // N, an adult's steady shove
  for (int i = 0; i < 3; ++i) {
    Tier0 t = make_world(1, true);
    s[i] = breach_s(t, *edge_of_family(t, "door"), one * (1 << i));
  }
  MESSAGE("door breach s with 1/2/4 walkers: " << s[0] << " " << s[1] << " " << s[2]);
  CHECK(s[2] < s[1]);
  CHECK(s[1] < s[0]);
}

TEST_CASE("stage 7: a boarded window outlasts a bare one (spec 10, A1)") {
  Tier0 bare = make_world(1, true);
  Tier0 boarded = make_world(1, true);
  wound_player(boarded);
  ScriptedPop script(false);
  while (boarded.world->clock.tick < 4 * kTicksPerGameHour && script.drive(boarded)) boarded.world->step();
  std::optional<EdgeRef> window;
  for (const auto& [id, ref] : boarded.actors->edges())
    if (boarded.map->edge(ref).barricade) window = ref;
  REQUIRE(window);
  const double b = breach_s(boarded, *window, 800), n = breach_s(bare, *window, 800);
  MESSAGE("window breach s: bare " << n << ", boarded " << b);
  CHECK(b > n);
}

TEST_CASE("stage 7: a crowbar in strong hands puts walkers down more often than in weak ones (spec 20, A3)") {
  int strong_n = 0, weak_n = 0;
  for (std::uint64_t seed = 1; seed <= 50; ++seed) {
    Tier0 t = make_world(seed, true);
    const auto now = t.world->clock.tick;
    const auto by_force = adults_by(t, [&](EntityId b) { return t.bodies->push_force_n(t.bodies->get(b), now); });
    const EntityId weak = by_force.front(), strong = by_force.back();
    const EntityId crowbar = item_of(t, "crowbar");
    if (seed == 1) CHECK(t.walkers->strike_energy_j(*t.world, strong, crowbar) > t.walkers->strike_energy_j(*t.world, weak, crowbar));
    std::vector<EntityId> targets = off_map(t);
    std::erase(targets, weak);
    std::erase(targets, strong);
    REQUIRE(targets.size() >= 2);
    const Cell at = player_cell(t);
    for (EntityId b : {weak, strong, targets[0], targets[1]}) place(t, b, at);
    strong_n += t.walkers->strike(*t.world, strong, targets[0], crowbar, 0);
    weak_n += t.walkers->strike(*t.world, weak, targets[1], crowbar, 0);
  }
  MESSAGE("put down in 50 strikes: strong " << strong_n << ", weak " << weak_n);
  CHECK(strong_n > weak_n);
}

TEST_CASE("stage 7: bites establish the infection by depth; prodrome, death and rising fall in band (spec 21, A3)") {
  int established = 0;
  for (std::uint64_t seed = 1; seed <= 100; ++seed) {
    Tier0 t = make_world(seed, true);
    t.walkers->bite(*t.world, off_map(t).front(), t.player, 0);
    const auto turning = t.walkers->turning_of(t.player);
    if (!turning) continue;
    ++established;
    const double h = static_cast<double>(kTicksPerGameHour);
    CHECK(turning->prodrome_tick / h >= 6);
    CHECK(turning->prodrome_tick / h <= 12);
    CHECK(turning->death_tick / h >= 24);
    CHECK(turning->death_tick / h <= 48);
    CHECK(turning->rise_after / h * 60 >= 10);
    CHECK(turning->rise_after / h * 60 <= 60);
  }
  MESSAGE("infection established in " << established << " of 100 bites");
  CHECK(established >= 65);  // default preset: 0.5 skin-deep, 0.9 deeper, ~0.78 expected
  CHECK(established <= 92);
}

TEST_CASE("stage 7: a bitten pop dies and rises with its wounds and items (spec 21)") {
  for (std::uint64_t seed = 1;; ++seed) {
    REQUIRE(seed < 20);
    Tier0 t = make_world(seed, true);
    t.world->check_every_tick = false;
    const EntityId biter = off_map(t).front();
    t.walkers->bite(*t.world, biter, t.player, 0);
    if (!t.walkers->turning_of(t.player)) continue;
    t.world->locations.place(item_of(t, "crowbar"), t.player);
    auto items_on = [&] {  // the pop's items (its wounds are children too)
      std::set<EntityId> out;
      for (EntityId c : t.world->locations.children(t.player))
        if (t.items->exists(c)) out.insert(c);
      return out;
    };
    const std::set<EntityId> items = items_on();
    std::set<EntityId> wounds;
    for (const Wound& wd : t.bodies->get(t.player).wounds) wounds.insert(wd.id);
    while (!t.walkers->turning_of(t.player)->risen && t.world->clock.tick < 60 * kTicksPerGameHour) t.world->step();
    REQUIRE(t.walkers->turning_of(t.player)->risen);
    const Body& b = t.bodies->get(t.player);
    CHECK(b.alive);
    CHECK(b.spec.walker);
    // Whatever killed it (the infection, or bleeding from more bites while it stood there), it rose.
    CHECK(b.death_cause != DeathCause::BrainDestroyed);
    for (EntityId wd : wounds)
      CHECK(std::any_of(b.wounds.begin(), b.wounds.end(), [&](const Wound& x) { return x.id == wd; }));
    CHECK(items_on() == items);
    CHECK(t.walkers->cell_of(t.player) == player_cell(t));
    CHECK(t.census->people()[*t.census->person_of(t.player)].fate == Fate::WalkerOnMap);
    for (const Event& e : t.world->log.all())
      if (e.type == EventType::Death && e.subject == t.player)
        CHECK(death_sentence(t, e).find("Bitten on the") != std::string::npos);
    break;
  }
}

TEST_CASE("stage 7: walkers move at their body's pace: lean faster than fat, a broken leg slowest (spec 23, A3)") {
  Tier0 t = make_world(1, true);
  const auto now = t.world->clock.tick;
  const auto by_fat = adults_by(t, [&](EntityId b) {
    const Body& x = t.bodies->get(b);
    return x.spec.fat_fraction + (x.spec.age > 60 ? 1 : 0);  // keep the old out of the lean end
  });
  const EntityId lean = by_fat.front();
  EntityId fat = by_fat.front();
  for (EntityId b : by_fat)
    if (t.bodies->get(b).spec.age <= 60) fat = b;
  const double v_lean = t.bodies->walk_speed(t.bodies->get(lean), now);
  const double v_fat = t.bodies->walk_speed(t.bodies->get(fat), now);
  Wound broken;
  broken.region = Region::ShinL;
  broken.type = WoundType::Fracture;
  broken.depth = Depth::Muscle;
  t.bodies->add_wound(*t.world, lean, broken, 0);
  const double v_broken = t.bodies->walk_speed(t.bodies->get(lean), now);
  MESSAGE("walker body walk speed m/s: lean " << v_lean << ", fat " << v_fat << ", lean with a broken shin " << v_broken);
  CHECK(v_lean > v_fat);
  CHECK(v_fat > v_broken);
}

TEST_CASE("stage 7: a generator at night draws the walkers on the map toward it (spec 23, A3)") {
  auto near_gen = [](std::uint64_t seed, bool gen) {
    Tier0 t = make_world(seed, true);
    t.world->check_every_tick = false;
    t.power->fail_grid(*t.world, 0);
    run_hours(t, 13);  // 21:00
    if (gen) {
      t.power->set_transfer(*t.world, Transfer::Generator, 0);
      REQUIRE(t.power->start_generator(*t.world, 0));
    }
    run_hours(t, 4);
    const Cell g = t.power->generator_cell();
    int n = 0;
    for (const auto& [id, z] : t.walkers->walkers()) n += z.pos.z == g.z && distance_m(z.pos, g) <= 12;
    return n;
  };
  int quiet = 0, loud = 0;
  for (std::uint64_t seed = 1; seed <= 4; ++seed) {
    quiet += near_gen(seed, false);
    loud += near_gen(seed, true);
  }
  MESSAGE("walkers within 12 m of the generator after 4 night hours: off " << quiet << ", running " << loud);
  CHECK(loud > quiet);
}

TEST_CASE("stage 7: the scenario with the dead: save/load keeps the hash; golden") {
  Tier0 t = make_world(4, true);  // seed 4 since decision 0065 (see the census test above)
  t.world->check_every_tick = false;
  GeneratorScenario script(false);
  std::vector<std::uint8_t> mid;
  GeneratorScenario at_mid = script;
  while (script.drive(t)) {
    t.world->step();
    if (mid.empty() && count(*t.world, EventType::WalkerArrived) > 0) {
      mid = t.world->save();
      at_mid = script;
    }
  }
  REQUIRE_FALSE(mid.empty());
  run_hours(t, 2);
  CHECK(count(*t.world, EventType::Bite) > 0);
  CHECK(count(*t.world, EventType::Reanimated) == 1);
  Tier0 u = make_world(4, true);
  u.world->check_every_tick = false;
  u.world->load(mid);
  while (at_mid.drive(u)) u.world->step();
  run_hours(u, 2);
  CHECK(u.world->world_hash() == t.world->world_hash());
  const std::uint64_t golden = 6581081184344293996ULL;  // re-pinned on MSVC for decision 0064 (background 35/27, two thresholds, moans, generator 85 dB), 2026-10-11
  CHECK(t.world->world_hash() == golden);
  MESSAGE("stage 7 golden world hash: " << t.world->world_hash());
}

TEST_CASE("stage 7 review: nothing reaches through a shut door or a wall; a dead pop goes quiet") {
  Tier0 t = make_world(1, true);
  t.world->check_every_tick = false;
  // A closed door between the pop and a walker: every swing misses and no hand gets through.
  std::optional<EdgeRef> door;
  for (const auto& [id, ref] : t.actors->edges())
    if (t.items->def(t.items->get(id).def).family == "door" && !t.map->edge(ref).open) door = ref;
  REQUIRE(door);
  const Cell in{door->z, door->x, door->y};
  const Cell out = door->vertical ? Cell{in.z, in.x - 1, in.y} : Cell{in.z, in.x, in.y - 1};
  CHECK_FALSE(t.walkers->in_reach(in, out));
  t.actors->actor(t.player).pos = in;
  const EntityId walker = off_map(t).front();
  place(t, walker, out);
  const EntityId crowbar = item_of(t, "crowbar");
  for (int i = 0; i < 20; ++i) CHECK_FALSE(t.walkers->strike(*t.world, t.player, walker, crowbar, 0));
  CHECK(t.bodies->get(walker).wounds.empty());
  // A corpse makes no noise, whatever it was doing when it died.
  t.actors->actor(t.player).noise_db = 50;
  t.bodies->kill(*t.world, t.player, DeathCause::Exsanguination, 0);
  t.world->step();
  CHECK(t.actors->actor(t.player).noise_db == 0);
}

TEST_CASE("decision 0064: a walker that pursues moans, and a second walker nearby hears the moan and turns to it (spec 18, A3)") {
  Tier0 t = make_world(4, true);
  t.world->check_every_tick = false;
  t.world->clock.tick = 15 * kTicksPerGameHour;  // 23:00: night background (45 dB with the grid alive)
  const auto bodies = off_map(t);
  REQUIRE(bodies.size() >= 2);
  const Cell me = player_cell(t);
  // The first walker next to the player: it sees and pursues, and so moans.
  const Cell beside{me.z, me.x + 1, me.y};
  place(t, bodies[0], beside);
  // The second six cells farther along the same row (3 m): the moan, 65 dB at 1 m, is about 55 dB there, 10 dB over the
  // night background, so it is placed, not merely noticed.
  const Cell farther{me.z, me.x + 7, me.y};
  place(t, bodies[1], farther);
  t.world->run_ticks(4 * kBodyStepTicks);
  const auto& first = t.walkers->walkers().at(bodies[0]);
  CHECK((first.mode == Mode::Pursue || first.mode == Mode::Attack));
  bool moans = false;
  for (const NoiseSource& s : t.walkers->sources(*t.world))
    if (s.at == first.pos && s.db_1m >= 60) moans = true;
  CHECK(moans);
  const auto& second = t.walkers->walkers().at(bodies[1]);
  CHECK(second.mode != Mode::Idle);
  CHECK(std::abs(second.goal.x - me.x) <= std::abs(farther.x - me.x));  // its goal is toward the player, not away
}
