// Stage 3: the body (specs 11, 12, 13). The calibration bands run through the body lab.
#include <doctest.h>

#include "property.h"
#include "zs/body_lab.h"

using namespace zs;
using namespace zs::body_lab;

namespace {
void check(const Result& r) {
  INFO(r.name << ": " << r.value << " " << r.unit << " (band: " << r.band << ")");
  CHECK(r.pass);
}

// Two pops with wounds, one walker; fed and watered by the routine.
Lab scripted_lab(std::uint64_t seed) {
  BodySpec walker;
  walker.walker = true;
  BodySpec woman;
  woman.sex = Sex::Female;
  woman.mass_kg = 62;
  woman.height_m = 1.65;
  woman.fat_fraction = 0.26;
  Lab lab = make_lab(seed, {BodySpec{}, woman, walker});
  return lab;
}

void script_hour(Lab& lab, int hour) {
  std::vector<EntityId> ids;
  for (const auto& [id, b] : lab.bodies->all()) ids.push_back(id);
  if (hour == 9) {
    Wound w;
    w.region = Region::ForearmL;
    w.type = WoundType::Bite;
    w.depth = Depth::Muscle;
    w.length_mm = 30;
    w.contamination = 1.0;
    lab.bodies->add_wound(*lab.world, ids[0], w, 0);
  }
  Activity work;
  work.met = 2.5;
  daily_routine(lab, ids[0], work);
  daily_routine(lab, ids[1], Activity{});
  keep_supplied(lab, ids[0], true, true);
  keep_supplied(lab, ids[1], true, false);
  lab.bodies->get(ids[2]).activity.speed_mps = 0.4;
}
}  // namespace

TEST_CASE("body lab: starvation and semi-starvation bands (A3)") {
  check(total_fast_14d());
  check(semi_starvation_14d());
}

TEST_CASE("body lab: dehydration band (A3)") { check(no_water_critical_day()); }

TEST_CASE("body lab: bleeding, untreated and dressed (A3)") {
  check(thigh_bleed_untreated());
  check(thigh_bleed_dressed());
}

TEST_CASE("body lab: infection rates, and cleaning lowers them (A3)") {
  const Result dirty = infection_rate(Cleaned::No);
  const Result water = infection_rate(Cleaned::Water);
  const Result disinfected = infection_rate(Cleaned::Disinfectant);
  check(dirty);
  check(water);
  check(disinfected);
  CHECK(water.value < dirty.value);
}

TEST_CASE("body lab: wakefulness, haul, the wall (A3)") {
  check(awake_40h());
  check(heavy_haul());
  check(the_wall());
}

TEST_CASE("body lab: capabilities (A1, A3)") {
  check(load_on_light_vs_strong());
  check(fracture_vs_hand());
  check(lean_outruns_heavy());
}

TEST_CASE("body: without water every body dies of dehydration, fat or lean (review fix)") {
  BodySpec woman;
  woman.sex = Sex::Female;
  woman.mass_kg = 62;
  woman.height_m = 1.65;
  woman.fat_fraction = 0.26;
  Lab lab = make_lab(1, {BodySpec{}, woman, BodySpec{Sex::Male, 30, 1.75, 110, 1 - 54.0 / 110, 23, false}});
  std::vector<EntityId> ids;
  for (const auto& [id, b] : lab.bodies->all()) ids.push_back(id);
  Activity light;
  light.met = 2.0;
  for (int h = 0; h < 12 * 24; ++h) {
    for (EntityId id : ids) {
      daily_routine(lab, id, light);
      keep_supplied(lab, id, true, false);
    }
    advance(lab, kTicksPerGameHour);
  }
  for (EntityId id : ids) {
    const Body& b = lab.bodies->get(id);
    CHECK_FALSE(b.alive);
    CHECK(b.death_cause == DeathCause::Dehydration);
  }
}

TEST_CASE("body: a small muscle-deep cut in the thigh clots by itself (review fix)") {
  Lab lab = make_lab(1, {BodySpec{}});
  const EntityId id = lab.bodies->all().begin()->first;
  Wound w;
  w.region = Region::ThighL;
  w.depth = Depth::Muscle;
  w.length_mm = 20;
  lab.bodies->add_wound(*lab.world, id, w, 0);
  advance(lab, 3 * kTicksPerGameHour);
  const Body& b = lab.bodies->get(id);
  CHECK(lab.bodies->bleed_ml_per_min(b) < 0.05);  // 6 time constants: almost closed
  CHECK(lab.bodies->blood_loss_fraction(b) < 0.05);
}

TEST_CASE("body: creation conserves the spec mass and mints only in setup") {
  Lab lab = make_lab(1, {BodySpec{}});
  const Body& b = lab.bodies->all().begin()->second;
  CHECK(lab.bodies->mass_kg(b) == doctest::Approx(75).epsilon(1e-6));
  CHECK(lab.bodies->fat_fraction(b) == doctest::Approx(0.18).epsilon(1e-6));
  CHECK_THROWS(lab.bodies->create(*lab.world, BodySpec{}, "late"));
  BodySpec impossible;
  impossible.fat_fraction = 0.95;
  World w(1);
  Bodies bodies(w);
  CHECK_THROWS(bodies.create(w, impossible, "x"));
}

TEST_CASE("body: capabilities stay in [0, 1] and speeds stay >= 0 for any wounds (A1)") {
  property("capability bounds", [](std::vector<std::uint8_t> picks) {
    Lab lab = make_lab(1, {BodySpec{}});
    const EntityId id = lab.bodies->all().begin()->first;
    for (std::size_t i = 0; i + 2 < picks.size() && i < 60; i += 3) {
      Wound w;
      w.region = static_cast<Region>(picks[i] % kRegionCount);
      w.type = static_cast<WoundType>(picks[i + 1] % 6);
      w.depth = static_cast<Depth>(picks[i + 2] % 4);
      w.splinted = picks[i] & 0x80;
      lab.bodies->add_wound(*lab.world, id, w, 0);
    }
    lab.world->run_ticks(240);
    const Body& b = lab.bodies->get(id);
    const Capabilities c = lab.bodies->capabilities(b, lab.world->clock.tick);
    for (double v : {c.consciousness, c.moving, c.manipulation_l, c.manipulation_r, c.sight, c.hearing}) {
      RC_ASSERT(v >= 0.0);
      RC_ASSERT(v <= 1.0);
    }
    RC_ASSERT(lab.bodies->walk_speed(b, lab.world->clock.tick) >= 0.0);
    RC_ASSERT(lab.bodies->run_speed(b, lab.world->clock.tick) >= 0.0);
    const double p = lab.bodies->pain(b, lab.world->clock.tick);
    RC_ASSERT(p >= 0.0);
    RC_ASSERT(p <= 1.0);
    // Readouts are pure: reading twice gives the same value.
    RC_ASSERT(lab.bodies->capabilities(b, lab.world->clock.tick).moving == c.moving);
  });
}

TEST_CASE("body: every death names its cause and emits one Death event") {
  Lab lab = make_lab(1, {BodySpec{}});
  const EntityId id = lab.bodies->all().begin()->first;
  Wound w;
  w.region = Region::Neck;
  w.depth = Depth::Deep;
  w.length_mm = 60;
  lab.bodies->add_wound(*lab.world, id, w, 0);
  lab.world->run_ticks(6 * kTicksPerGameHour);
  const Body& b = lab.bodies->get(id);
  REQUIRE_FALSE(b.alive);
  CHECK(b.death_cause == DeathCause::Exsanguination);
  int deaths = 0;
  for (const Event& e : lab.world->log.all())
    if (e.type == EventType::Death) {
      ++deaths;
      CHECK(e.subject == id);
      CHECK(e.a == static_cast<std::int64_t>(DeathCause::Exsanguination));
    }
  CHECK(deaths == 1);
}

TEST_CASE("body: save mid-run and load replays to the same hash; ledger checked every tick") {
  Lab a = scripted_lab(7);
  a.world->check_every_tick = true;
  for (int h = 0; h < 30; ++h) {
    script_hour(a, h % 24);
    a.world->run_ticks(kTicksPerGameHour);
  }
  const auto bytes = a.world->save();
  Lab b = scripted_lab(7);
  b.world->load(bytes);
  CHECK(b.world->state_hash() == a.world->state_hash());
  for (int h = 30; h < 60; ++h) {
    script_hour(a, h % 24);
    script_hour(b, h % 24);
    a.world->run_ticks(kTicksPerGameHour);
    b.world->run_ticks(kTicksPerGameHour);
  }
  CHECK(b.world->state_hash() == a.world->state_hash());
}

TEST_CASE("body: golden hash (same on every compiler)") {
  Lab lab = scripted_lab(3);
  for (int h = 0; h < 48; ++h) {
    script_hour(lab, h % 24);
    lab.world->run_ticks(kTicksPerGameHour);
  }
  const std::uint64_t golden = 17635968491440591384ULL;  // re-pinned on MSVC for decision 0065 (ids per creator; the draws are keyed by id), 2026-10-11
  CHECK(lab.world->state_hash() == golden);
  MESSAGE("body golden hash: " << lab.world->state_hash());
}
