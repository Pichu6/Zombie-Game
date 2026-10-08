// Stage 9: the run (specs 25, 26). Found survivors, the stranger, the radio, control after death,
// the end of the run, and the chronicle's tags and day summaries.
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

void run_hours(Tier0& t, double h) {
  const auto end = t.world->clock.tick + static_cast<std::int64_t>(h * kTicksPerGameHour);
  while (t.world->clock.tick < end) t.world->step();
}

Tier0 quiet_run(std::uint64_t seed) {
  Tier0 t = make_run(seed);
  t.world->check_every_tick = false;
  t.ai->possess(t.player);  // the test drives the player; nobody else is in the group yet
  return t;
}

EntityId found_body(const Tier0& t, Role role) {
  for (const Found& f : t.run->found())
    if (f.role == role) return f.body;
  return 0;
}

void use(Tier0& t, EntityId body, const char* process, EntityId target) {
  Command c;
  c.type = static_cast<std::uint16_t>(CommandType::Use);
  c.subject = body;
  c.a = static_cast<std::int64_t>(*t.actors->find_process(process));
  c.b = static_cast<std::int64_t>(target);
  t.world->submit(c);
}

bool done(const Tier0& t, EntityId body, const char* process) {
  const auto p = static_cast<std::int64_t>(*t.actors->find_process(process));
  for (const Event& e : t.world->log.all())
    if (e.type == EventType::ActionDone && e.subject == body && e.a == p) return true;
  return false;
}

}  // namespace

TEST_CASE("stage 9: the run starts with the player alone and three found survivors placed (spec 26 rules 1-2)") {
  Tier0 t = make_run(1);
  REQUIRE(t.run->found().size() == 3);
  CHECK(t.run->group() == std::vector<EntityId>{t.player});
  CHECK(t.ai->minds().size() == 1);
  const EntityId neighbour = found_body(t, Role::Neighbour), tech = found_body(t, Role::Technician),
                 stranger = found_body(t, Role::Stranger);
  CHECK(t.run->waiting(neighbour));
  CHECK(t.run->waiting(tech));
  CHECK_FALSE(t.run->waiting(stranger));  // still away
  CHECK(t.bodies->get(stranger).away);
  CHECK(t.bodies->water_deficit_fraction(t.bodies->get(neighbour)) > 0.25);
  const auto elec = static_cast<std::size_t>(Trade::Electronics);
  CHECK(t.actors->actor(tech).skill[elec] == 70);
  CHECK(t.actors->actor(t.player).skill[elec] < 0);
  CHECK(t.actors->actor(neighbour).skill[static_cast<std::size_t>(Trade::FirstAid)] == 70);
  CHECK(t.run->stranger_tick() >= 3 * kTicksPerGameDay);
  CHECK(t.run->stranger_tick() < 6 * kTicksPerGameDay);
  CHECK(t.run->broadcast_tick() >= 9 * kTicksPerGameDay);
}

TEST_CASE("stage 9: invite brings a found survivor into the group with a mind (spec 26 rule 3)") {
  Tier0 t = quiet_run(1);
  const EntityId neighbour = found_body(t, Role::Neighbour);
  use(t, t.player, "invite", neighbour);
  run_hours(t, 0.25);
  CHECK(count(*t.world, EventType::Joined, neighbour) == 1);
  CHECK(t.run->group().size() == 2);
  CHECK(t.ai->minds().contains(neighbour));
  CHECK(t.census->people()[*t.census->person_of(neighbour)].fate == Fate::Pop);
  // Asking again is refused: already one of us.
  const auto p = *t.actors->find_process("invite");
  CHECK(t.actors->preview(*t.world, t.player, p, neighbour).reason != Reason::Ok);
}

TEST_CASE("stage 9: only the technician can repair the radio; it needs power; static before the broadcast (spec 26 rule 5)") {
  Tier0 t = quiet_run(1);
  const EntityId tech = found_body(t, Role::Technician);
  const EntityId radio = t.run->radio();
  const auto repair = *t.actors->find_process("repair_radio"), listen = *t.actors->find_process("listen_radio");
  CHECK(t.actors->preview(*t.world, t.player, repair, radio).reason == Reason::UnknownProcess);
  CHECK(t.actors->preview(*t.world, t.player, listen, radio).reason == Reason::Broken);
  // The technician takes the screwdriver and fixes it (driven directly: not in the group yet).
  EntityId screwdriver = 0;
  for (const auto& [id, inst] : t.items->all())
    if (t.items->def(inst.def).id == "screwdriver") screwdriver = id;
  use(t, tech, "take", screwdriver);
  use(t, tech, "repair_radio", radio);
  run_hours(t, 1.5);
  REQUIRE(t.run->radio_repaired());
  CHECK(count(*t.world, EventType::RadioRepaired, tech) == 1);
  CHECK(t.actors->preview(*t.world, t.player, listen, radio).reason == Reason::NoPower);  // switched off
  use(t, t.player, "switch_on", radio);
  use(t, t.player, "listen_radio", radio);
  run_hours(t, 0.5);
  CHECK(done(t, t.player, "listen_radio"));
  CHECK(count(*t.world, EventType::RadioStatic, t.player) == 1);
  CHECK_FALSE(t.run->heard());
}

TEST_CASE("stage 9: the stranger reaches the gate at the drawn time, bitten and hiding it when drawn so (spec 26 rule 2)") {
  int bitten = 0, clean = 0;
  for (std::uint64_t seed = 1; seed <= 12 && (bitten == 0 || clean == 0); ++seed) {
    Tier0 t = make_run(seed);  // the AI keeps the group going until the stranger comes
    t.world->check_every_tick = false;
    const EntityId stranger = found_body(t, Role::Stranger);
    while (t.world->clock.tick <= t.run->stranger_tick()) t.world->step();
    t.world->step();
    if (t.run->outcome() != RunOutcome::Running) continue;  // nobody left to open the gate
    t.ai->possess(t.run->controlled());
    CHECK_FALSE(t.bodies->get(stranger).away);
    CHECK(count(*t.world, EventType::StrangerArrived, stranger) == 1);
    CHECK(t.run->waiting(stranger));
    if (t.run->stranger_bitten()) {
      ++bitten;
      CHECK(t.walkers->turning_of(stranger).has_value());
      if (count(*t.world, EventType::Joined, stranger) == 0) {
        use(t, t.run->controlled(), "invite", stranger);
        run_hours(t, 0.5);
      }
      if (count(*t.world, EventType::Joined, stranger) == 1) {
        CHECK_FALSE(t.ai->mind(stranger).honest);
        run_hours(t, 0.1);
        CHECK_FALSE(t.ai->says_bitten(stranger));
      }
    } else {
      ++clean;
      CHECK_FALSE(t.walkers->turning_of(stranger).has_value());
    }
  }
  CHECK(bitten > 0);
  CHECK(clean > 0);
}

TEST_CASE("stage 9: control passes to whoever joined first; the run ends when nobody is left (spec 26 rules 6-7)") {
  Tier0 t = quiet_run(1);
  const EntityId neighbour = found_body(t, Role::Neighbour), tech = found_body(t, Role::Technician);
  use(t, t.player, "invite", neighbour);
  use(t, t.player, "invite", tech);
  run_hours(t, 0.5);
  REQUIRE(t.run->group().size() == 3);
  t.ai->release();
  t.bodies->kill(*t.world, t.player, DeathCause::Exsanguination, 0);
  t.world->step();
  t.world->step();
  CHECK(t.run->controlled() == neighbour);
  CHECK(count(*t.world, EventType::ControlPassed, neighbour) == 1);
  CHECK(t.run->outcome() == RunOutcome::Running);
  t.bodies->kill(*t.world, neighbour, DeathCause::Exsanguination, 0);
  t.bodies->kill(*t.world, tech, DeathCause::Exsanguination, 0);
  t.world->step();
  t.world->step();
  CHECK(t.run->outcome() == RunOutcome::AllDead);
  CHECK(count(*t.world, EventType::RunEnded) == 1);
}

TEST_CASE("stage 9: the chronicle tags a hidden bite and says what was missed (spec 25)") {
  Tier0 t = quiet_run(1);
  const EntityId neighbour = found_body(t, Role::Neighbour);
  use(t, t.player, "invite", neighbour);
  run_hours(t, 0.25);
  REQUIRE(t.ai->minds().contains(neighbour));
  t.ai->set_honest(neighbour, false);
  std::vector<EntityId> off;
  for (const Person& p : t.census->people())
    if (p.fate == Fate::WalkerOffMap) off.push_back(p.body);
  for (int i = 0; i < 20 && !t.walkers->turning_of(neighbour); ++i) t.walkers->bite(*t.world, off.front(), neighbour, 0);
  REQUIRE(t.walkers->turning_of(neighbour));
  run_hours(t, 0.25);  // long enough to have told the group
  t.bodies->kill(*t.world, neighbour, DeathCause::Pathogen, t.walkers->turning_of(neighbour)->cause);
  const Event* death = nullptr;
  for (const Event& e : t.world->log.all())
    if (e.type == EventType::Death && e.subject == neighbour) death = &e;
  REQUIRE(death);
  const auto tags = danger_tags(t, *death);
  CHECK(std::find(tags.begin(), tags.end(), "walker attack") != tags.end());
  CHECK(std::find(tags.begin(), tags.end(), "hidden bite") != tags.end());
  const auto missed = missed_counters(t, *death);
  CHECK(std::find(missed.begin(), missed.end(), "the bite was never told to the group") != missed.end());
  // The honest technician, bitten the same way, tells the group: no hidden-bite tag.
  const EntityId tech = found_body(t, Role::Technician);
  use(t, t.player, "invite", tech);
  run_hours(t, 0.25);
  REQUIRE(t.ai->minds().contains(tech));
  t.ai->set_honest(tech, true);
  for (int i = 0; i < 20 && !t.walkers->turning_of(tech); ++i) t.walkers->bite(*t.world, off.front(), tech, 0);
  REQUIRE(t.walkers->turning_of(tech));
  run_hours(t, 0.25);
  t.bodies->kill(*t.world, tech, DeathCause::Pathogen, t.walkers->turning_of(tech)->cause);
  const Event* told = nullptr;
  for (const Event& e : t.world->log.all())
    if (e.type == EventType::Death && e.subject == tech) told = &e;
  REQUIRE(told);
  const auto honest_tags = danger_tags(t, *told);
  CHECK(std::find(honest_tags.begin(), honest_tags.end(), "hidden bite") == honest_tags.end());
  const auto day = day_summary(t, 0);
  CHECK(!day.empty());
  CHECK(day.size() <= 5);
  CHECK(std::any_of(day.begin(), day.end(), [](const std::string& l) { return l.find("joined the group") != std::string::npos; }));
}

TEST_CASE("stage 9: a run: save/load keeps the hash across a join; golden") {
  Tier0 t = make_run(1);
  t.world->check_every_tick = false;
  run_hours(t, 0.02);
  REQUIRE(t.run->group().size() == 1);
  const auto mid = t.world->save();
  run_hours(t, 6);
  CHECK(t.run->group().size() > 1);  // someone joined after the save
  Tier0 u = make_run(1);
  u.world->check_every_tick = false;
  u.world->load(mid);
  run_hours(u, 6);
  CHECK(u.world->world_hash() == t.world->world_hash());
  CHECK(u.run->group() == t.run->group());
  const std::uint64_t golden = 15585671806141295154ULL;  // re-pinned on MSVC for spec 41 (the one world), 2026-10-08
  CHECK(t.world->world_hash() == golden);
  MESSAGE("stage 9 golden world hash: " << t.world->world_hash());
}
