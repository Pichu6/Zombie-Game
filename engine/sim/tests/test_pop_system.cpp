// Spec 28: the pop registry inside a world: save, hash, replay, commands and reasons.
#include <doctest.h>

#include <memory>

#include "zs/pop_system.h"

using namespace zs;

namespace {
std::unique_ptr<World> make_world(std::uint64_t seed) {
  auto w = std::make_unique<World>(seed);
  auto pops = std::make_unique<PopSystem>(*w);
  PopRegistry& reg = pops->setup(*w);
  reg.add_cohort({1, 0, 0, 400});
  reg.add_cohort({2, 1, 1000, 300});
  reg.add_roster(1, 0);
  for (PopId p = 0; p < 80; ++p) reg.roster_join(1, p, 0.0);
  w->add_system(std::move(pops));
  w->close_setup();
  w->check_every_tick = true;
  return w;
}

// A scripted player: a few commands at known ticks over a few game days.
void script(World& w) {
  const std::int64_t t = w.clock.tick;
  const std::int64_t day = kTicksPerGameDay;
  if (t == 1 * day) w.submit({0, RosterApply, 1, static_cast<std::int64_t>(EffectKind::LaidOff), 20});
  if (t == 2 * day) w.submit({0, CohortApply, 1, static_cast<std::int64_t>(EffectKind::Ill), 50});
  if (t == 2 * day + 5) w.submit({0, PopSetOffice, 7, 3, 0});
  if (t == 3 * day) w.submit({0, RosterLeave, 12, 1, 0});
  if (t == 3 * day + 9) w.submit({0, RosterApply, 1, static_cast<std::int64_t>(EffectKind::LaidOff), -5});
  if (t == 4 * day) w.submit({0, PopPromote, 1100, 0, 0});
}

std::unique_ptr<World> run(std::uint64_t seed, std::int64_t ticks) {
  auto w = make_world(seed);
  for (std::int64_t i = 0; i < ticks; ++i) { script(*w); w->step(); }
  return w;
}
}  // namespace

TEST_CASE("pops: save, load, save gives identical bytes, and the world hash includes the registry (A1)") {
  auto w = run(5, 3 * kTicksPerGameDay + 100);
  const auto bytes = w->save();
  auto candidate = make_world(5);
  candidate->load(bytes);
  CHECK(candidate->save() == bytes);
  CHECK(candidate->state_hash() == w->state_hash());
  // Changing a pop changes the world hash.
  const auto before = w->state_hash();
  w->submit({0, PopPromote, 200, 0, 0});
  w->step();
  CHECK(w->state_hash() != before);
}

TEST_CASE("pops: replaying the input log reproduces the per-day hashes (A1)") {
  auto w = run(6, 5 * kTicksPerGameDay);
  auto r = make_world(6);
  std::size_t next = 0;
  const auto& inputs = w->input_log();
  for (std::int64_t i = 0; i < 5 * kTicksPerGameDay; ++i) {
    while (next < inputs.size() && inputs[next].tick == r->clock.tick) r->submit(inputs[next++]);
    r->step();
  }
  CHECK(r->state_hash() == w->state_hash());
}

TEST_CASE("pops: a load into a continuing world matches never having stopped (A1)") {
  auto straight = run(7, 5 * kTicksPerGameDay);
  auto first = run(7, 2 * kTicksPerGameDay + 50);
  auto resumed = make_world(7);
  resumed->load(first->save());
  for (std::int64_t i = 2 * kTicksPerGameDay + 50; i < 5 * kTicksPerGameDay; ++i) { script(*resumed); resumed->step(); }
  CHECK(resumed->world_hash() == straight->world_hash());
}

TEST_CASE("pops: every effect carries the order that caused it, and refusals are logged and change nothing (A1)") {
  auto w = make_world(8);
  const auto* pops = w->system<PopSystem>();
  w->submit({0, RosterApply, 1, static_cast<std::int64_t>(EffectKind::Ill), 30});
  w->step();
  // The reason on the roster's hits is the Intent event of that command.
  std::uint64_t reason = 0;
  for (PopId p = 0; p < 80 && !reason; ++p) reason = pops->registry().roster(1)->reason_of(p, EffectKind::Ill, 1.0);
  REQUIRE(reason != 0);
  const Event* e = w->log.get(reason);
  REQUIRE(e);
  CHECK(e->type == EventType::Intent);
  CHECK(e->a == RosterApply);
  const auto before = pops->registry().save();
  w->submit({0, RosterApply, 1, static_cast<std::int64_t>(EffectKind::Ill), 500});  // more than the roster has
  w->submit({0, PopPromote, 999999, 0, 0});                                          // not a pop
  w->step();
  CHECK(pops->registry().save() == before);
  int refused = 0, effects = 0;
  for (const Event& ev : w->log.all()) { refused += ev.type == EventType::PopCommandRefused; effects += ev.type == EventType::PopEffect; }
  CHECK(refused == 2);
  CHECK(effects == 1);
}

TEST_CASE("pops: setup is closed after close_setup, so pops change only by command (A1)") {
  auto w = make_world(9);
  CHECK_THROWS(w->system<PopSystem>()->setup(*w));
}

TEST_CASE("pops: the registry audit stays clean through a scripted run (A1)") {
  auto w = run(10, 5 * kTicksPerGameDay);
  CHECK_FALSE(w->system<PopSystem>()->registry().audit().has_value());
  CHECK(w->system<PopSystem>()->registry().counts().total == 700);
}

TEST_CASE("pops: game years run 365 days, and a command lands in the right year (A1)") {
  CHECK(year_of_tick(0) == 0.0);
  CHECK(year_of_tick(365 * kTicksPerGameDay) == 1.0);
  auto w = make_world(11);
  w->run_ticks(2 * kTicksPerGameDay);
  w->submit({0, RosterApply, 1, static_cast<std::int64_t>(EffectKind::Moved), 10});
  w->step();
  const auto* roster = w->system<PopSystem>()->registry().roster(1);
  CHECK(roster->count(EffectKind::Moved, year_of_tick(2 * kTicksPerGameDay) - 1e-9) == 0);
  CHECK(roster->count(EffectKind::Moved, year_of_tick(2 * kTicksPerGameDay) + 1e-9) == 10);
}
