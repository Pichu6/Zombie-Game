// The jump test harness (layer jumps §5 step 2) checked against itself, and the body's time while demoted
// (spec 29 rule 6) checked against stepping with it.
#include <doctest.h>

#include <vector>

#include "jumps.h"
#include "zs/body_lab.h"

using namespace zs;
using namespace zs::jumps;

namespace {

// A BodyState taken from a live body (what disembody() would save), without a pool.
BodyState state_of(const Bodies& bodies, const World& w, EntityId id) {
  const Body& b = bodies.get(id);
  const BodyLedger& L = bodies.ids();
  BodyState s;
  s.spec = b.spec;
  s.label = b.label;
  s.blood_normal_ul = b.blood_normal_ul;
  s.water_normal_ul = b.water_normal_ul;
  s.muscle_start_mg = b.muscle_start_mg;
  s.glycogen_max_mg = b.glycogen_max_mg;
  s.posture = b.posture;
  s.activity = b.activity;
  s.sleep_pressure = b.sleep_pressure;
  s.exertion = b.exertion;
  s.vo2max = b.vo2max;
  s.fear = b.fear;
  s.morale = b.morale;
  s.threat = b.threat;
  s.active_minutes_today = b.active_minutes_today;
  s.heavy_minutes_today = b.heavy_minutes_today;
  s.carry = b.carry;
  s.wounds = b.wounds;
  s.fat_mg = w.ledger.balance(b.owner, L.fat_mg);
  s.muscle_mg = w.ledger.balance(b.owner, L.muscle_mg);
  s.structure_mg = w.ledger.balance(b.owner, L.structure_mg);
  s.glycogen_mg = w.ledger.balance(b.owner, L.glycogen_mg);
  s.water_ul = w.ledger.balance(b.owner, L.water_ul);
  s.blood_ul = w.ledger.balance(b.owner, L.blood_ul);
  s.food_j = w.ledger.balance(b.owner, L.food_j);
  return s;
}

Wound dirty_cut(EntityId id) {
  Wound wd;
  wd.id = id;
  wd.region = Region::ForearmL;
  wd.type = WoundType::Laceration;
  wd.depth = Depth::Fat;
  wd.length_mm = 40;
  wd.contamination = 0.9;
  return wd;
}

bool infected_ever(const Wound& w) { return w.infection != Infection::None; }

}  // namespace

TEST_CASE("harness: Anderson–Darling tells the same distribution from a shifted one; energy distance too") {
  Sample a, b, c;
  std::uint64_t x = 7;
  auto u = [&] { return static_cast<double>(jumps::detail::splitmix(x) >> 11) / 9007199254740992.0; };
  for (int i = 0; i < 200; ++i) {
    a.push_back(std::floor(u() * 20));  // integers with ties, like counts
    b.push_back(std::floor(u() * 20));
    c.push_back(std::floor(u() * 20) + 3);
  }
  const ADResult same = anderson_darling_ksample({a, b});
  const ADResult shifted = anderson_darling_ksample({a, c});
  CHECK(!same.reject_5pct);
  CHECK(shifted.reject_5pct);
  CHECK(shifted.p_approx < same.p_approx);
  Vectors va, vb, vc;
  for (int i = 0; i < 60; ++i) {
    va.push_back({a[i], b[i]});
    vb.push_back({a[i + 60], b[i + 60]});
    vc.push_back({c[i], a[i] + 5});
  }
  CHECK(!energy_distance(va, vb, 100).reject_5pct);
  CHECK(energy_distance(va, vc, 100).reject_5pct);
  CHECK(band_share(a, b) > 0.8);
  CHECK(proportions_agree(10, 100, 12, 100));
  CHECK(!proportions_agree(10, 100, 40, 100));
}

TEST_CASE("body jump: determinism and the no-time identity (A1)") {
  std::vector<BodySpec> specs(1);
  auto lab = body_lab::make_lab(3, specs);
  const EntityId id = lab.bodies->all().begin()->first;
  lab.bodies->add_wound(*lab.world, id, dirty_cut(0), 0);
  const BodyState s = state_of(*lab.bodies, *lab.world, id);
  Rng r1(99), r2(99);
  const AdvancedBody a = advance_demoted(s, 1000, 1000 + 2 * kTicksPerGameDay, r1);
  const AdvancedBody b = advance_demoted(s, 1000, 1000 + 2 * kTicksPerGameDay, r2);
  CHECK(a.state.glycogen_mg == b.state.glycogen_mg);
  CHECK(a.state.wounds.size() == b.state.wounds.size());
  CHECK(a.death.has_value() == b.death.has_value());
  const AdvancedBody none = advance_demoted(s, 1000, 1000, r1);
  CHECK(none.state.glycogen_mg == s.glycogen_mg);
  CHECK(none.state.sleep_pressure == s.sleep_pressure);
  CHECK(none.state.wounds.size() == s.wounds.size());
}

TEST_CASE("body jump: a fed life brings stocks back toward normal and settles the mind (spec 29 rule 6)") {
  std::vector<BodySpec> specs(1);
  auto lab = body_lab::make_lab(4, specs);
  const EntityId id = lab.bodies->all().begin()->first;
  lab.world->run_ticks(36 * kTicksPerGameHour);  // a day and a half without food or water, stepped
  BodyState s = state_of(*lab.bodies, *lab.world, id);
  s.fear = 0.9;
  s.sleep_pressure = 0.95;
  CHECK(s.glycogen_mg < s.glycogen_max_mg / 2);
  Rng r(5);
  const AdvancedBody a = advance_demoted(s, 0, 2 * kTicksPerGameDay, r);
  CHECK(a.state.glycogen_mg > s.glycogen_max_mg * 9 / 10);
  CHECK(std::llabs(a.state.water_ul - s.water_normal_ul) < s.water_normal_ul / 50);
  CHECK(a.state.sleep_pressure < 0.35);
  CHECK(a.state.fear < 0.01);
  CHECK(a.state.exertion == 1.0);
  CHECK(!a.state.activity.asleep);
  CHECK(a.state.fat_mg + a.state.muscle_mg + a.state.structure_mg == s.fat_mg + s.muscle_mg + s.structure_mg);
}

TEST_CASE("body jump: infection of a dirty cut agrees with stepping in proportion (A2); healing by the clock (A1)") {
  // Hot: 60 bodies, each with the same dirty cut, stepped two days.
  const int n_hot = 60;
  std::vector<BodySpec> specs(static_cast<std::size_t>(n_hot));
  auto lab = body_lab::make_lab(6, specs);
  lab.world->check_every_tick = false;
  std::vector<EntityId> ids;
  for (const auto& [id, b] : lab.bodies->all()) ids.push_back(id);
  for (std::size_t i = 0; i < ids.size(); ++i) lab.bodies->add_wound(*lab.world, ids[i], dirty_cut(0), 0);
  const BodyState start = state_of(*lab.bodies, *lab.world, ids[0]);
  const std::int64_t span = 2 * kTicksPerGameDay;
  lab.world->run_ticks(span);
  std::uint64_t hot_infected = 0;
  for (EntityId id : ids)
    for (const Wound& w : lab.bodies->get(id).wounds) hot_infected += infected_ever(w);
  // Cold: 3000 jumps of the same start, each wound on its own draw stream.
  const int n_cold = 3000;
  Rng r(6);
  std::uint64_t cold_infected = 0, cold_resolved = 0;
  for (int i = 0; i < n_cold; ++i) {
    BodyState s = start;
    s.wounds.front().id = static_cast<EntityId>(100000 + i);
    const AdvancedBody a = advance_demoted(s, 0, span, r);
    for (const Wound& w : a.state.wounds) {
      cold_infected += infected_ever(w);
      cold_resolved += w.infection == Infection::Resolved;
    }
    if (a.death) ++cold_infected;  // sepsis counts as infected
  }
  MESSAGE("infected after two days: stepped " << hot_infected << " of " << n_hot << ", jumped " << cold_infected << " of " << n_cold
          << " (" << cold_resolved << " resolved)");
  CHECK(proportions_agree(hot_infected, n_hot, cold_infected, n_cold));
  // Healing: a clean cut gone by its heal tick, in one jump, exactly as the clock says.
  BodyState clean = start;
  clean.wounds.front().contamination = 0;
  clean.wounds.front().cleaned = Cleaned::Disinfectant;
  const std::int64_t heal = clean.wounds.front().heal_tick;
  CHECK(advance_demoted(clean, 0, heal - 1, r).state.wounds.size() == 1);
  CHECK(advance_demoted(clean, 0, heal, r).state.wounds.empty());
}
