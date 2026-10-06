// Stage 8: the group (spec 24). Pop AI wants and jobs, refusals, orders, bite reports, possession,
// putting down the dead, and the AI-only run.
#include <doctest.h>

#include <algorithm>

#include "zs/tier0.h"

using namespace zs;
using namespace zs::tier0;

namespace {

int count(const World& w, EventType type, EntityId subject = 0) {
  int n = 0;
  for (const Event& e : w.log.all()) n += e.type == type && (!subject || e.subject == subject);
  return n;
}

EntityId item_of(const Tier0& t, const char* def_id, int skip = 0) {
  for (const auto& [id, inst] : t.items->all())
    if (t.items->def(inst.def).id == def_id && skip-- == 0) return id;
  return 0;
}

void run_hours(Tier0& t, double h) {
  const auto end = t.world->clock.tick + static_cast<std::int64_t>(h * kTicksPerGameHour);
  while (t.world->clock.tick < end) t.world->step();
}

Tier0 group_world(std::uint64_t seed) {
  Tier0 t = make_world(seed, true, 4);
  t.world->check_every_tick = false;
  return t;
}

std::vector<EntityId> off_map(const Tier0& t) {
  std::vector<EntityId> out;
  for (const Person& p : t.census->people())
    if (p.fate == Fate::WalkerOffMap) out.push_back(p.body);
  return out;
}

bool alive_pop(const Tier0& t, EntityId b) { return t.bodies->get(b).alive && !t.bodies->get(b).spec.walker; }

}  // namespace

TEST_CASE("stage 8: a group starts embodied next to the player, every pop with a mind") {
  Tier0 t = group_world(1);
  REQUIRE(t.group.size() == 4);
  CHECK(t.group.front() == t.player);
  for (EntityId b : t.group) {
    CHECK(t.ai->minds().contains(b));
    CHECK(t.census->people()[*t.census->person_of(b)].fate == Fate::Pop);
    CHECK(is_indoor(t.map->floor(t.actors->actor(b).pos)));
  }
}

TEST_CASE("stage 8: a dark room is refused with its reason; an insisted order goes through at low fear (spec 24 rule 4, A1)") {
  bool tested = false;
  for (std::uint64_t seed = 1; seed < 10 && !tested; ++seed) {
    Tier0 t = group_world(seed);
    for (EntityId b : t.group) t.ai->set_priority(b, Job::Refuel, 0);  // nobody starts the generator
    t.power->fail_grid(*t.world, 0);
    run_hours(t, 14);  // 22:00, no power
    for (EntityId b : t.group) {
      if (!alive_pop(t, b) || t.bodies->get(b).activity.asleep || !t.walkers->visible_to(*t.world, b).empty()) continue;
      const Cell from = t.actors->actor(b).pos;
      std::optional<Cell> dark;
      for (int y = 0; y < t.map->height() && !dark; ++y)
        for (int x = 0; x < t.map->width() && !dark; ++x) {
          const Cell c{from.z, x, y};
          if (is_indoor(t.map->floor(c)) && t.map->room_of(c) != t.map->room_of(from) && t.power->lux_at(*t.world, c) < 1 &&
              !t.map->find_path(from, c).empty())
            dark = c;
        }
      if (!dark) continue;
      CHECK(t.ai->refusal(*t.world, b, *dark) == Refusal::Dark);
      const int before = count(*t.world, EventType::PopRefused, b);
      CHECK(t.ai->give_order(*t.world, b, Order{OrderType::Go, *dark, 0, false}) == Refusal::Dark);
      CHECK(count(*t.world, EventType::PopRefused, b) == before + 1);
      CHECK(t.ai->mind(b).order.type == OrderType::None);
      if (t.bodies->get(b).fear < 0.5) {
        CHECK(t.ai->give_order(*t.world, b, Order{OrderType::Go, *dark, 0, true}) == Refusal::None);
        CHECK(t.ai->mind(b).order.type == OrderType::Go);
      }
      tested = true;
      break;
    }
  }
  CHECK(tested);
}

TEST_CASE("stage 8: open ground with a walker in sight is refused (spec 24 rule 4, A1)") {
  Tier0 t = group_world(1);
  const EntityId pop = t.group[1];
  const Cell from = t.actors->actor(pop).pos;
  std::optional<Cell> out;
  for (int y = 0; y < t.map->height() && !out; ++y)
    for (int x = 0; x < t.map->width() && !out; ++x) {
      const Cell c{from.z, x, y};
      if (t.map->floor(c) == Floor::None || is_indoor(t.map->floor(c)) || t.map->find_path(from, c).empty()) continue;
      out = c;
    }
  REQUIRE(out);
  CHECK(t.ai->refusal(*t.world, pop, *out) == Refusal::None);  // nothing in sight yet
  // A walker where the pop can see it.
  const EntityId z = off_map(t).front();
  bool seen = false;
  for (int y = 0; y < t.map->height() && !seen; ++y)
    for (int x = 0; x < t.map->width() && !seen; ++x) {
      const Cell c{from.z, x, y};
      if (t.map->floor(c) == Floor::None || is_indoor(t.map->floor(c)) || distance_m(c, from) < 6) continue;
      t.walkers->spawn(*t.world, z, c);
      const auto v = t.walkers->visible_to(*t.world, pop);
      seen = std::find(v.begin(), v.end(), z) != v.end();
    }
  REQUIRE(seen);
  CHECK(t.ai->refusal(*t.world, pop, *out) == Refusal::OpenGround);
  CHECK(t.ai->give_order(*t.world, pop, Order{OrderType::Go, *out, 0, false}) == Refusal::OpenGround);
}

TEST_CASE("stage 8: honest pops report a bite, secretive ones keep it to themselves (spec 24 rule 7, A1)") {
  int honest = 0, secretive = 0;
  for (std::uint64_t seed = 1; seed <= 4; ++seed) {
    Tier0 t = group_world(seed);
    const EntityId biter = off_map(t).front();
    for (EntityId b : t.group) t.walkers->bite(*t.world, biter, b, 0);
    for (int i = 0; i < 20; ++i) t.world->step();
    for (EntityId b : t.group) {
      const PopMind& m = t.ai->mind(b);
      CHECK(t.ai->says_bitten(b) == m.honest);
      CHECK(count(*t.world, EventType::BiteReported, b) == (m.honest ? 1 : 0));
      (m.honest ? honest : secretive)++;
    }
  }
  MESSAGE("pops bitten: " << honest << " honest, " << secretive << " secretive");
  CHECK(honest > 0);
  CHECK(secretive > 0);
}

TEST_CASE("stage 8: possess, act, release: the AI picks up the same body and state (spec 24 rule 8, A1)") {
  Tier0 t = group_world(2);
  run_hours(t, 0.5);
  const EntityId pop = t.group[1];
  REQUIRE(alive_pop(t, pop));
  t.ai->set_priority(pop, Job::Guard, 1);
  const PopMind before = t.ai->mind(pop);
  t.ai->possess(pop);
  // While possessed the AI sends nothing: the pop drinks because the player said so.
  const std::size_t drink = *t.actors->find_process("drink_tap");
  const EntityId sink = *t.actors->find_object("sink");
  Command c;
  c.type = static_cast<std::uint16_t>(CommandType::Use);
  c.subject = pop;
  c.a = static_cast<std::int64_t>(drink);
  c.b = static_cast<std::int64_t>(sink);
  t.world->submit(c);
  const auto decide_before = t.ai->mind(pop).next_decide;
  run_hours(t, 0.25);
  CHECK(t.ai->mind(pop).next_decide == decide_before);
  bool drank = false;
  for (const Event& e : t.world->log.all())
    drank |= e.type == EventType::ActionDone && e.subject == pop && e.a == static_cast<std::int64_t>(drink);
  CHECK(drank);
  t.ai->release();
  run_hours(t, 0.05);
  const PopMind& after = t.ai->mind(pop);
  CHECK(after.body == pop);
  CHECK(after.priority == before.priority);
  CHECK(after.brave == before.brave);
  CHECK(after.honest == before.honest);
  CHECK(after.next_decide > decide_before);  // deciding again
}

TEST_CASE("stage 8: weak blows add up; a plank in weak hands puts a walker down in time (spec 20)") {
  Tier0 t = group_world(1);
  const EntityId pop = t.group[1];
  const EntityId plank = item_of(t, "plank");
  REQUIRE(plank);
  t.world->locations.place(plank, pop);
  const EntityId z = off_map(t).front();
  t.walkers->spawn(*t.world, z, t.actors->actor(pop).pos);
  int strikes = 0;
  while (t.walkers->walkers().contains(z) && strikes < 200) {
    t.walkers->strike(*t.world, pop, z, plank, 0);
    ++strikes;
  }
  MESSAGE("strikes with a plank to put a walker down: " << strikes);
  CHECK_FALSE(t.walkers->walkers().contains(z));
  CHECK(strikes <= 20);
}

TEST_CASE("stage 8: the group puts down its dead before they rise (spec 21, 24)") {
  Tier0 t = group_world(3);
  const EntityId victim = t.group[1], helper = t.group[2];
  // Bite until the infection takes, give the helper a crowbar, then let the infection kill.
  const EntityId biter = off_map(t).front();
  for (int i = 0; i < 20 && !t.walkers->turning_of(victim); ++i) t.walkers->bite(*t.world, biter, victim, 0);
  REQUIRE(t.walkers->turning_of(victim));
  t.world->locations.place(item_of(t, "crowbar"), helper);
  t.bodies->kill(*t.world, victim, DeathCause::Pathogen, 0);
  run_hours(t, 1.5);
  CHECK(t.walkers->put_down(victim));
  CHECK(count(*t.world, EventType::Reanimated, victim) == 0);
  CHECK_FALSE(t.bodies->get(victim).spec.walker);
}

TEST_CASE("stage 8: the AI-only group outlives a few days on its own in some runs (spec 24, A3)") {
  int alive = 0, runs_alive = 0;
  for (std::uint64_t seed = 1; seed <= 4; ++seed) {
    Tier0 t = group_world(seed);
    run_hours(t, 48);
    int n = 0;
    for (EntityId b : t.group) n += alive_pop(t, b);
    alive += n;
    runs_alive += n > 0;
  }
  MESSAGE("AI-only, 2 days, 4 runs: " << alive << " of 16 alive; someone alive in " << runs_alive << " runs");
  CHECK(runs_alive >= 2);
}

TEST_CASE("stage 8: a group run: save/load keeps the hash; golden") {
  Tier0 t = group_world(1);
  run_hours(t, 6);
  const auto mid = t.world->save();
  run_hours(t, 6);
  Tier0 u = group_world(1);
  u.world->load(mid);
  run_hours(u, 6);
  CHECK(u.world->world_hash() == t.world->world_hash());
  const std::uint64_t golden = 9545267489536325917ULL;
  CHECK(t.world->world_hash() == golden);
  MESSAGE("stage 8 golden world hash: " << t.world->world_hash());
}
