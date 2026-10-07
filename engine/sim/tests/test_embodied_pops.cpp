// Spec 29: embodied pops, the registry and the body pool kept in step by commands.
#include <doctest.h>

#include <fstream>
#include <iterator>
#include <memory>
#include <random>

#include "zs/pop_body.h"
#include "zs/pop_system.h"

using namespace zs;

namespace {
constexpr std::int64_t kDwell = kTicksPerGameHour;

struct Setup {
  std::unique_ptr<World> world;
  Bodies* bodies = nullptr;
  PopSystem* pops = nullptr;
};

Setup make(std::size_t slots, std::uint64_t seed = 11) {
  Setup s;
  s.world = std::make_unique<World>(seed);
  auto b = std::make_unique<Bodies>(*s.world);
  s.bodies = b.get();
  s.world->add_system(std::move(b));
  auto p = std::make_unique<PopSystem>(*s.world);
  s.pops = p.get();
  p->setup(*s.world).add_cohort({1, 0, 0, 400});
  s.bodies->reserve_slots(*s.world, slots);
  p->attach_bodies(*s.world, *s.bodies);
  s.world->add_system(std::move(p));
  s.world->close_setup();
  s.world->check_every_tick = true;
  return s;
}

int refusals(const World& w, Refusal why) {
  int n = 0;
  for (const Event& e : w.log.all()) n += e.type == EventType::PopCommandRefused && e.b == static_cast<std::int64_t>(why);
  return n;
}

void send(World& w, std::uint16_t type, PopId pop) {
  w.submit({0, type, pop, 0, 0});
  w.step();
}

std::int64_t tissue(const Setup& s) {
  const BodyLedger& L = s.bodies->ids();
  std::int64_t t = 0;
  for (ResourceId r : {L.fat_mg, L.muscle_mg, L.structure_mg, L.glycogen_mg, L.water_ul, L.blood_ul})
    t += s.world->ledger.total(r);
  return t;
}

// What the pool's slots hold (a body's metabolism sends mass on to sinks, so the world total is the wrong thing to
// compare after time has passed).
std::int64_t slots_held(const Setup& s) {
  const BodyLedger& L = s.bodies->ids();
  std::int64_t t = 0;
  for (EntityId id : s.bodies->slots())
    for (ResourceId r : {L.fat_mg, L.muscle_mg, L.structure_mg, L.glycogen_mg, L.water_ul, L.blood_ul, L.food_j})
      t += s.world->ledger.balance(s.bodies->get(id).owner, r);
  return t;
}
}  // namespace

TEST_CASE("embodied: the same pop gets the same body, in any order (A1)") {
  auto x = make(4), y = make(4);
  // Same tick (a pop's age is its age now), opposite order.
  x.world->submit({0, PopEmbody, 10, 0, 0});
  x.world->submit({0, PopEmbody, 20, 0, 0});
  x.world->step();
  y.world->submit({0, PopEmbody, 20, 0, 0});
  y.world->submit({0, PopEmbody, 10, 0, 0});
  y.world->step();
  for (PopId pop : {PopId{10}, PopId{20}}) {
    const BodySpec& a = x.bodies->get(x.pops->registry().embodied_body(pop)).spec;
    const BodySpec& b = y.bodies->get(y.pops->registry().embodied_body(pop)).spec;
    CHECK(a.height_m == b.height_m);
    CHECK(a.mass_kg == b.mass_kg);
    CHECK(a.vo2max == b.vo2max);
    CHECK(a.sex == b.sex);
  }
  CHECK(x.pops->registry().level(10) == Level::Embodied);
  CHECK(x.pops->registry().counts().embodied == 2);
}

TEST_CASE("embodied: a body spec is a pure function of (seed, pop, age, sex) and differs between pops") {
  const BodySpec a = body_spec_for(1, 5, 30, PopSex::Male), b = body_spec_for(1, 5, 30, PopSex::Male);
  CHECK(a.height_m == b.height_m);
  CHECK(a.mass_kg == b.mass_kg);
  CHECK(body_spec_for(1, 6, 30, PopSex::Male).height_m != a.height_m);
  CHECK(body_spec_for(2, 5, 30, PopSex::Male).height_m != a.height_m);
  CHECK_FALSE(a.walker);
  CHECK(body_spec_for(1, 5, 9, PopSex::Female).height_m < 1.6);
}

TEST_CASE("embodied: round trips with no time between keep the body, the ledger and the registry (A1)") {
  auto s = make(2);
  s.pops->set_dwell_ticks(0);
  const auto cycle = [&] {
    s.world->submit({0, PopEmbody, 30, 0, 0});
    s.world->submit({0, PopDisembody, 30, 0, 0});
    s.world->step();  // both commands in one tick: embodied and demoted before any body rule runs
  };
  cycle();
  REQUIRE(s.pops->registry().saved_body(30) != nullptr);
  // The saved blob ends with the demotion tick (spec 29 rule 6), which moves one tick per cycle; the state before it must not.
  const auto state_part = [](const std::vector<std::uint8_t>& blob) { return std::vector<std::uint8_t>(blob.begin(), blob.end() - 8); };
  const auto saved = state_part(*s.pops->registry().saved_body(30));
  const auto total = tissue(s);
  for (int i = 0; i < 20; ++i) {
    cycle();
    CHECK(state_part(*s.pops->registry().saved_body(30)) == saved);
    CHECK(tissue(s) == total);
    CHECK(slots_held(s) == 0);
    CHECK(s.bodies->free_slots() == 2);
  }
  CHECK(refusals(*s.world, Refusal::None) == 0);
}

TEST_CASE("embodied: the pop's body mass enters the ledger as a declared inflow and leaves again (A1)") {
  auto s = make(2);
  send(*s.world, PopEmbody, 31);
  const std::int64_t in = slots_held(s);
  CHECK(in > 0);
  s.world->run_ticks(kDwell);
  send(*s.world, PopDisembody, 31);
  CHECK(slots_held(s) == 0);
}

TEST_CASE("embodied: embodying twice, demoting a lazy pop, a dead pop and a missing pop are refused and change nothing (A1)") {
  auto s = make(2);
  send(*s.world, PopEmbody, 40);
  const auto hash = s.pops->registry().hash();
  send(*s.world, PopEmbody, 40);
  CHECK(refusals(*s.world, Refusal::AlreadyEmbodied) == 1);
  send(*s.world, PopDisembody, 41);
  CHECK(refusals(*s.world, Refusal::NotEmbodied) == 1);
  send(*s.world, PopEmbody, 999999);
  CHECK(refusals(*s.world, Refusal::NoSuchPop) == 1);
  CHECK(s.pops->registry().hash() == hash);
  CHECK(s.bodies->free_slots() == 1);
}

TEST_CASE("embodied: the pool is the cap and the request beyond it is refused, never queued (A1)") {
  auto s = make(8);
  for (PopId p = 100; p < 108; ++p) send(*s.world, PopEmbody, p);
  CHECK(s.bodies->free_slots() == 0);
  send(*s.world, PopEmbody, 108);
  CHECK(refusals(*s.world, Refusal::PoolFull) == 1);
  CHECK(s.pops->registry().counts().embodied == 8);
  CHECK(s.pops->registry().level(108) == Level::Lazy);
}

TEST_CASE("embodied: demoting before the dwell time is refused (A1)") {
  auto s = make(2);
  send(*s.world, PopEmbody, 50);
  send(*s.world, PopDisembody, 50);
  CHECK(refusals(*s.world, Refusal::Dwell) == 1);
  CHECK(s.pops->registry().level(50) == Level::Embodied);
  s.world->run_ticks(kDwell);
  send(*s.world, PopDisembody, 50);
  CHECK(s.pops->registry().level(50) == Level::Record);
  // Pushed back and forth: embodied again resets the clock.
  send(*s.world, PopEmbody, 50);
  send(*s.world, PopDisembody, 50);
  CHECK(refusals(*s.world, Refusal::Dwell) == 2);
}

TEST_CASE("embodied: a pop whose body dies while embodied is dead and its slot is freed (A1)") {
  auto s = make(2);
  send(*s.world, PopEmbody, 60);
  const EntityId body = s.pops->registry().embodied_body(60);
  s.bodies->kill(*s.world, body, DeathCause::Pathogen, 0);
  s.world->step();
  CHECK(s.bodies->free_slots() == 2);
  CHECK(s.pops->registry().embodied_body(60) == 0);
  CHECK(s.pops->registry().counts().embodied == 0);
  PopSnapshot snap;
  REQUIRE(s.pops->registry().snapshot(60, year_of_tick(s.world->clock.tick) + 0.01, snap));
  CHECK(snap.base.death_year <= year_of_tick(s.world->clock.tick));
  CHECK(slots_held(s) == 0);
  send(*s.world, PopEmbody, 60);
  CHECK(refusals(*s.world, Refusal::Dead) == 1);
}

TEST_CASE("embodied: a demoted pop's body comes back advanced over the gap by its jump form (spec 29 rule 6, A1)") {
  auto s = make(2);
  send(*s.world, PopEmbody, 70);
  const EntityId body = s.pops->registry().embodied_body(70);
  s.world->run_ticks(kDwell + 20 * kTicksPerGameHour);  // nearly a day embodied without food or water
  const Body before = s.bodies->get(body);
  const double glycogen_before = s.bodies->grams(before, s.bodies->ids().glycogen_mg);
  CHECK(glycogen_before < static_cast<double>(before.glycogen_max_mg) / 1000.0 * 0.8);
  send(*s.world, PopDisembody, 70);
  s.world->run_ticks(2 * kTicksPerGameDay);  // two days pass while the pop is a record and lives its fed life
  send(*s.world, PopEmbody, 70);
  const Body& after = s.bodies->get(s.pops->registry().embodied_body(70));
  // The jump form restored a fed, rested body: glycogen back near full, sleep pressure at the rested baseline.
  CHECK(s.bodies->grams(after, s.bodies->ids().glycogen_mg) > static_cast<double>(after.glycogen_max_mg) / 1000.0 * 0.9);
  CHECK(after.sleep_pressure < 0.35);
  // Masses are untouched by the gap, and the ledger holds exactly what the state says (declared inflow).
  CHECK(s.world->ledger.balance(after.owner, s.bodies->ids().fat_mg) == s.world->ledger.balance(before.owner, s.bodies->ids().fat_mg));
  CHECK(s.world->ledger.check() == std::nullopt);
}

TEST_CASE("embodied: random commands keep the registry and the pool in step (property)") {
  std::mt19937_64 rng(99);
  auto s = make(5);
  for (int i = 0; i < 400; ++i) {
    const PopId pop = 200 + rng() % 12;
    const bool in = rng() % 2;
    s.world->submit({0, static_cast<std::uint16_t>(in ? PopEmbody : PopDisembody), pop, 0, 0});
    s.world->run_ticks(1 + rng() % (kDwell / 2));  // check_every_tick audits the two truths each tick and throws
    CHECK(s.pops->registry().counts().embodied == 5 - s.bodies->free_slots());
    CHECK(s.pops->registry().counts().embodied <= 5);
  }
}

TEST_CASE("embodied: save, load and replay with embodied pops reproduce the world (A1)") {
  auto run = [](std::int64_t ticks) {
    auto s = make(3, 12);
    for (std::int64_t i = 0; i < ticks; ++i) {
      const std::int64_t t = s.world->clock.tick;
      if (t == 5) s.world->submit({0, PopEmbody, 300, 0, 0});
      if (t == 9) s.world->submit({0, PopEmbody, 301, 0, 0});
      if (t == 9 + kDwell) s.world->submit({0, PopDisembody, 300, 0, 0});
      s.world->step();
    }
    return s;
  };
  auto s = run(2 * kDwell + 100);
  const auto bytes = s.world->save();
  auto candidate = make(3, 12);
  candidate.world->load(bytes);
  CHECK(candidate.world->save() == bytes);
  CHECK(candidate.world->state_hash() == s.world->state_hash());
  CHECK(candidate.pops->registry().level(301) == Level::Embodied);
  CHECK(candidate.pops->registry().level(300) == Level::Record);
  CHECK(candidate.pops->registry().saved_body(300) != nullptr);
  // Replay the input log into a fresh world.
  auto r = make(3, 12);
  std::size_t next = 0;
  const auto& inputs = s.world->input_log();
  for (std::int64_t i = 0; i < 2 * kDwell + 100; ++i) {
    while (next < inputs.size() && inputs[next].tick == r.world->clock.tick) r.world->submit(inputs[next++]);
    r.world->step();
  }
  CHECK(r.world->state_hash() == s.world->state_hash());
}

TEST_CASE("embodied: a registry that never embodied anyone still saves as version 4 and loads (A1)") {
  PopRegistry reg(3);
  reg.add_cohort({1, 0, 0, 50});
  reg.promote(4);
  const auto bytes = reg.save();
  CHECK(bytes[8] == 4);  // magic is 8 bytes, then the u32 version, little endian
  PopRegistry loaded(3);
  loaded.load(bytes);
  CHECK(loaded.save() == bytes);
  reg.begin_embodied(4, 99, 7);
  CHECK(reg.save()[8] == 5);
  PopRegistry again(3);
  again.load(reg.save());
  CHECK(again.embodied_body(4) == 99);
  CHECK(again.embodied_since(4) == 7);
  CHECK_FALSE(again.demote(4));
}

namespace {
PopRegistry fixture_registry() {
  PopRegistry reg(3);
  reg.add_cohort({1, 0, 0, 50});
  reg.begin_embodied(4, 99, 7);                       // embodied now
  reg.begin_embodied(6, 98, 3);
  reg.end_embodied(6, {1, 2, 3, 4, 5});               // demoted, with a saved state
  reg.set_death_year(9, 61.5);                        // a lazy exception beside them
  return reg;
}
}  // namespace

TEST_CASE("embodied: the version-5 fixture still loads and re-saves to the same bytes (A1)") {
  // fixtures/popreg_v5.bin was written from fixture_registry() when version 5 shipped. Never regenerate it:
  // if this fails, the format changed and needs a new version and a new reader beside this one.
  std::ifstream in(std::string(ZS_FIXTURES_DIR) + "/popreg_v5.bin", std::ios::binary);
  REQUIRE(in);
  const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  PopRegistry loaded(3);
  loaded.load(bytes);
  CHECK(loaded.save() == bytes);
  CHECK(loaded.save() == fixture_registry().save());
  CHECK(loaded.embodied_body(4) == 99);
  CHECK(loaded.embodied_since(4) == 7);
  CHECK(loaded.level(6) == Level::Record);
  REQUIRE(loaded.saved_body(6) != nullptr);
  CHECK(*loaded.saved_body(6) == std::vector<std::uint8_t>{1, 2, 3, 4, 5});
  CHECK_FALSE(loaded.audit().has_value());
}
