// The generated town as pops: founders with their generated age and sex, households and businesses as rosters.
#include <doctest.h>

#include <map>

#include "zs/gen/registry_bridge.h"
#include "zs/pop_system.h"

using namespace zs::gen;
using zs::EffectKind;
using zs::PopId;
using zs::PopSnapshot;

namespace {
const Catalog& catalog() {
  static const Catalog cat = load_catalog(std::string(ZS_DATA_DIR) + "/gen");
  return cat;
}
Town make_town(int people, std::uint64_t seed) {
  Mix mix = load_mix(std::string(ZS_DATA_DIR) + "/gen/populations/tier0_block.toml");
  mix.people = people;
  mix.on_map_homes.clear();
  return generate_town(catalog(), mix, seed);
}
}  // namespace

TEST_CASE("a generated town becomes founders with their generated age and sex (A1)") {
  const Town town = make_town(300, 4);
  zs::PopRegistry reg(4);
  const auto map = populate_registry(town, reg, 0, 1);
  REQUIRE(map.pop_ids.size() == town.pops.size());
  CHECK(reg.counts().total == town.pops.size());
  for (std::size_t i = 0; i < town.pops.size(); ++i) {
    PopSnapshot s;
    REQUIRE(reg.snapshot(map.pop_ids[i], 0.0, s));
    CHECK(s.base.sex == (town.pops[i].sex == 'f' ? zs::PopSex::Female : zs::PopSex::Male));
    CHECK(static_cast<int>(-s.base.birth_year) == town.pops[i].age);
    CHECK(s.base.death_year > 0.0);
  }
}

TEST_CASE("households and businesses become rosters with the right members (A1)") {
  const Town town = make_town(400, 5);
  zs::PopRegistry reg(5);
  const auto map = populate_registry(town, reg, 0, 1);
  for (std::size_t h = 0; h < town.households.size(); ++h) {
    const zs::Roster* r = reg.roster(household_roster(h));
    REQUIRE(r);
    CHECK(r->size(0.5) == town.households[h].members.size());
    for (std::size_t m : town.households[h].members) CHECK(r->present(map.pop_ids[m], 0.5));
  }
  std::map<int, std::size_t> staff, pupils;
  for (std::size_t i = 0; i < town.pops.size(); ++i)
    if (map.business_of_pop[i] >= 0) ++(map.student[i] ? pupils : staff)[map.business_of_pop[i]];
  std::size_t total_staff = 0, total_pupils = 0;
  for (std::size_t b = 0; b < town.businesses.size(); ++b) {
    const zs::Roster* w = reg.roster(workplace_roster(b));
    const zs::Roster* s = reg.roster(school_roster(b));
    REQUIRE(w);
    REQUIRE(s);
    CHECK(w->kind() == zs::RosterKind::Workplace);
    CHECK(s->kind() == zs::RosterKind::School);
    CHECK(w->size(0.5) == staff[static_cast<int>(b)]);
    CHECK(s->size(0.5) == pupils[static_cast<int>(b)]);
    total_staff += w->size(0.5);
    total_pupils += s->size(0.5);
  }
  CHECK(total_staff == map.employed);
  CHECK(total_pupils == map.students);
  CHECK(map.employed > 0);
  CHECK(map.students > 0);
  for (std::size_t i = 0; i < town.pops.size(); ++i)  // a student is under a school roster, never a job
    if (map.student[i]) CHECK(town.pops[i].job_id.empty());
  CHECK(map.unmatched_workplaces == 0);  // every workplace a pop names is in the town's directory
  CHECK_FALSE(reg.audit().has_value());
}

TEST_CASE("a layoff aimed at workplaces lands only on workplace rosters; households are untouched (A1)") {
  const Town town = make_town(500, 6);
  zs::PopRegistry reg(6);
  const auto map = populate_registry(town, reg, 0, 1);
  std::uint64_t jobs = 0;
  for (std::size_t b = 0; b < town.businesses.size(); ++b) jobs += reg.roster(workplace_roster(b))->size(0.5);
  REQUIRE(jobs > 30);
  const auto k = static_cast<std::int64_t>(jobs / 10);
  REQUIRE(reg.region_apply(0, EffectKind::LaidOff, k, 1.0, 7, zs::RosterKind::Workplace));
  std::uint64_t at_work = 0, at_home = 0;
  for (std::size_t b = 0; b < town.businesses.size(); ++b) at_work += reg.roster(workplace_roster(b))->count(EffectKind::LaidOff, 1.5);
  for (std::size_t h = 0; h < town.households.size(); ++h) at_home += reg.roster(household_roster(h))->count(EffectKind::LaidOff, 1.5);
  CHECK(at_work == static_cast<std::uint64_t>(k));
  CHECK(at_home == 0);
  // A pop on the list really reports it, and its dossier is the generated one.
  std::size_t found = 0;
  for (std::size_t i = 0; i < town.pops.size(); ++i) {
    PopSnapshot s;
    REQUIRE(reg.snapshot(map.pop_ids[i], 1.5, s));
    if (s.roster_bits & 1u) { ++found; CHECK(map.business_of_pop[i] >= 0); CHECK_FALSE(map.student[i]); }  // no pupil is ever laid off
  }
  CHECK(found == static_cast<std::size_t>(k));
  CHECK(reg.region_apply(0, EffectKind::Ill, 25, 2.0, 8, zs::RosterKind::Household));   // illness can be aimed at homes
  CHECK_FALSE(reg.region_apply(0, EffectKind::Ill, 1, 3.0, 9, zs::RosterKind::Office)); // there are no offices
}

TEST_CASE("the same town gives the same registry bytes, and a town can run a few generations (A1)") {
  const Town town = make_town(300, 7);
  zs::PopRegistry a(7), b(7);
  populate_registry(town, a, 0, 1);
  populate_registry(town, b, 0, 1);
  CHECK(a.save() == b.save());
  const auto jump = a.project_demography(0.0, 80.0);
  CHECK(jump.alive_at_start > 250);
  CHECK(jump.births > 0);
  CHECK(jump.alive_at_end == jump.alive_at_start + jump.births - jump.deaths);
}

TEST_CASE("the generated town works as a world system with commands and replay (A1)") {
  const Town town = make_town(300, 8);
  const auto make = [&] {
    auto w = std::make_unique<zs::World>(8);
    auto pops = std::make_unique<zs::PopSystem>(*w);
    populate_registry(town, pops->setup(*w), 0, 1);
    w->add_system(std::move(pops));
    w->close_setup();
    w->check_every_tick = true;
    return w;
  };
  auto w = make();
  w->run_ticks(zs::kTicksPerGameDay);
  w->submit({0, zs::RegionApply, 0, static_cast<std::int64_t>(EffectKind::Ill), 50});  // all rosters
  w->run_ticks(zs::kTicksPerGameDay);
  CHECK(w->system<zs::PopSystem>()->registry().roster_count_region(0, EffectKind::Ill, zs::year_of_tick(2 * zs::kTicksPerGameDay)) == 50);
  auto r = make();
  std::size_t next = 0;
  for (std::int64_t i = 0; i < 2 * zs::kTicksPerGameDay; ++i) {
    while (next < w->input_log().size() && w->input_log()[next].tick == r->clock.tick) r->submit(w->input_log()[next++]);
    r->step();
  }
  CHECK(r->state_hash() == w->state_hash());
}
