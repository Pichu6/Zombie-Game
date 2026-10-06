// Spec 28, dynamic groups: rosters whose members change, with events that pick members once.
#include <doctest.h>

#include <set>
#include <vector>

#include "zs/pop_registry.h"

using namespace zs;

namespace {
PopRegistry registry_with_pops(std::uint64_t seed = 21) {
  PopRegistry reg(seed);
  REQUIRE(reg.add_cohort({1, 0, 0, 2000}));
  return reg;
}
std::uint64_t pick(std::uint64_t a, std::uint64_t b, std::uint64_t n) { return keyed_u64(55, Purpose::Test, a, b) % n; }
}  // namespace

TEST_CASE("joining and leaving is by year; double joins and unknown pops are refused (A1)") {
  PopRegistry reg = registry_with_pops();
  REQUIRE(reg.add_roster(1, 0));
  CHECK_FALSE(reg.add_roster(1, 0));
  CHECK(reg.roster_join(1, 10, 1.0));
  CHECK_FALSE(reg.roster_join(1, 10, 1.5));          // already in
  CHECK_FALSE(reg.roster_join(1, 999999, 1.5));      // no such pop
  CHECK_FALSE(reg.roster_join(2, 11, 1.5));          // no such roster
  CHECK_FALSE(reg.roster_join(1, 11, 0.5));          // back in time
  CHECK(reg.roster_join(1, 11, 2.0));
  CHECK(reg.roster_leave(1, 10, 3.0));
  CHECK_FALSE(reg.roster_leave(1, 10, 4.0));         // not in
  const Roster& r = *reg.roster(1);
  CHECK(r.size(0.5) == 0);
  CHECK(r.size(1.5) == 1);
  CHECK(r.size(2.5) == 2);
  CHECK(r.size(3.5) == 1);
  CHECK(r.present(10, 2.5));
  CHECK_FALSE(r.present(10, 3.5));
  CHECK(reg.roster_join(1, 10, 5.0));                // can come back
  CHECK(r.size(5.5) == 2);
}

TEST_CASE("an event hits exactly k present members who were not already hit, never twice (A1)") {
  PopRegistry reg = registry_with_pops();
  REQUIRE(reg.add_roster(1, 0));
  for (PopId p = 100; p < 220; ++p) REQUIRE(reg.roster_join(1, p, 1.0));
  CHECK_FALSE(reg.roster_apply(1, EffectKind::LaidOff, 121, 2.0, 7));   // more than there are
  REQUIRE(reg.roster_apply(1, EffectKind::LaidOff, 40, 2.0, 7));
  REQUIRE(reg.roster_apply(1, EffectKind::LaidOff, 50, 3.0, 8));
  CHECK_FALSE(reg.roster_apply(1, EffectKind::LaidOff, 31, 4.0, 9));    // only 30 left to hit
  CHECK(reg.roster_apply(1, EffectKind::LaidOff, 30, 4.0, 9));
  const Roster& r = *reg.roster(1);
  CHECK(r.count(EffectKind::LaidOff, 4.5) == 120);
  const auto hit = r.in_state_members(EffectKind::LaidOff, 4.5);
  CHECK(std::set<PopId>(hit.begin(), hit.end()).size() == 120);
  std::uint64_t by7 = 0, by8 = 0, by9 = 0;
  for (PopId p = 100; p < 220; ++p) {
    const auto why = r.reason_of(p, EffectKind::LaidOff, 4.5);
    by7 += why == 7; by8 += why == 8; by9 += why == 9;
  }
  CHECK(by7 == 40);
  CHECK(by8 == 50);
  CHECK(by9 == 30);
}

TEST_CASE("giving back takes the newest hits first, and is exact at past years (A1)") {
  PopRegistry reg = registry_with_pops();
  REQUIRE(reg.add_roster(1, 0));
  for (PopId p = 0; p < 100; ++p) REQUIRE(reg.roster_join(1, p, 0.0));
  REQUIRE(reg.roster_apply(1, EffectKind::Ill, 30, 1.0, 11));
  REQUIRE(reg.roster_apply(1, EffectKind::Ill, 20, 2.0, 22));
  REQUIRE(reg.roster_apply(1, EffectKind::Ill, -25, 3.0, 33));   // 25 recover: the 20 newest and 5 of the first 30
  const Roster& r = *reg.roster(1);
  CHECK(r.count(EffectKind::Ill, 0.5) == 0);
  CHECK(r.count(EffectKind::Ill, 1.5) == 30);
  CHECK(r.count(EffectKind::Ill, 2.5) == 50);
  CHECK(r.count(EffectKind::Ill, 3.5) == 25);
  std::uint64_t by22 = 0;
  for (PopId p = 0; p < 100; ++p) by22 += r.reason_of(p, EffectKind::Ill, 3.5) == 22;
  CHECK(by22 == 0);                                               // all of the second event recovered
  CHECK_FALSE(reg.roster_apply(1, EffectKind::Ill, -26, 4.0, 44));  // only 25 left to give back
}

TEST_CASE("members who join later are not hit by earlier events; members who leave stop counting (A1)") {
  PopRegistry reg = registry_with_pops();
  REQUIRE(reg.add_roster(1, 0));
  for (PopId p = 0; p < 50; ++p) REQUIRE(reg.roster_join(1, p, 0.0));
  REQUIRE(reg.roster_apply(1, EffectKind::LaidOff, 20, 1.0, 5));
  for (PopId p = 50; p < 80; ++p) REQUIRE(reg.roster_join(1, p, 2.0));      // hired after the layoff
  const Roster& r = *reg.roster(1);
  for (PopId p = 50; p < 80; ++p) CHECK_FALSE(r.in_state(p, EffectKind::LaidOff, 2.5));
  CHECK(r.count(EffectKind::LaidOff, 2.5) == 20);
  // A laid-off member leaves the roster: they no longer count here.
  const PopId gone = r.in_state_members(EffectKind::LaidOff, 2.5).front();
  REQUIRE(reg.roster_leave(1, gone, 3.0));
  CHECK(r.count(EffectKind::LaidOff, 3.5) == 19);
  CHECK(r.count(EffectKind::LaidOff, 2.5) == 20);                          // the past is unchanged
  // A second event can hit the new hires.
  REQUIRE(reg.roster_apply(1, EffectKind::LaidOff, 40, 4.0, 6));
  CHECK(r.count(EffectKind::LaidOff, 4.5) == 59);
  CHECK(r.size(4.5) == 79);
}

TEST_CASE("the count equals the members found, through random hires, quits and events (A1)") {
  for (int trial = 0; trial < 40; ++trial) {
    PopRegistry reg = registry_with_pops(300 + trial);
    REQUIRE(reg.add_roster(1, 0));
    double year = 0.0;
    PopId next = 0;
    for (int step = 0; step < 40; ++step) {
      year += 0.1;
      const auto what = pick(trial, step, 4);
      if (what == 0) { for (int i = 0; i < 8 && next < 2000; ++i) reg.roster_join(1, next++, year); }
      else if (what == 1) { const auto m = reg.roster(1)->members(year); if (!m.empty()) reg.roster_leave(1, m[pick(trial, step + 100, m.size())], year); }
      else {
        const auto kind = static_cast<EffectKind>(pick(trial, step + 200, 3));
        const std::int64_t now = static_cast<std::int64_t>(reg.roster_count(1, kind, year));
        const std::int64_t room = static_cast<std::int64_t>(reg.roster(1)->size(year)) - now;
        std::int64_t d = static_cast<std::int64_t>(pick(trial, step + 300, 14)) - 5;
        d = std::min(d, room); d = std::max(d, -now);
        CHECK(reg.roster_apply(1, kind, d, year, static_cast<std::uint64_t>(step + 1)));
      }
    }
    const Roster& r = *reg.roster(1);
    for (int k = 0; k < 3; ++k) {
      const auto kind = static_cast<EffectKind>(k);
      std::uint64_t found = 0;
      for (PopId p = 0; p < next; ++p) {
        PopSnapshot s;
        REQUIRE(reg.snapshot(p, year, s));
        found += (s.roster_bits >> k) & 1u;
      }
      CHECK(found == r.count(kind, year));
      CHECK(found == r.in_state_members(kind, year).size());
    }
    CHECK(reg.roster_count_region(0, EffectKind::Ill, year) == r.count(EffectKind::Ill, year));
  }
}

TEST_CASE("born pops can be hired and hit like anyone (A1)") {
  PopRegistry reg(23);
  REQUIRE(reg.add_cohort({1, 0, 0, 400}));
  std::vector<PopId> kids;
  for (std::uint32_t slot = 0; slot < 400 && kids.size() < 6; ++slot) {
    PopSnapshot s;
    REQUIRE(reg.snapshot(census_pop_id(slot), 0.0, s));
    for (const Birth& b : births_of(23, census_pop_id(slot), s.base, 0.0)) kids.push_back(birth_pop_id(census_pop_id(slot), b.index));
  }
  REQUIRE(kids.size() >= 6);
  REQUIRE(reg.add_roster(1, 0));
  for (PopId k : kids) REQUIRE(reg.roster_join(1, k, 50.0));
  REQUIRE(reg.roster_apply(1, EffectKind::LaidOff, 3, 51.0, 1));
  std::uint64_t found = 0;
  for (PopId k : kids) { PopSnapshot s; REQUIRE(reg.snapshot(k, 52.0, s)); found += s.roster_bits & 1u; }
  CHECK(found == 3);
}

TEST_CASE("the same history picks the same people; save and load keep rosters (A1)") {
  const auto build = [] {
    PopRegistry reg(31);
    reg.add_cohort({1, 0, 0, 500});
    reg.add_roster(1, 0);
    reg.add_roster(2, 1);
    for (PopId p = 0; p < 100; ++p) reg.roster_join(1, p, 0.0);
    for (PopId p = 100; p < 160; ++p) reg.roster_join(2, p, 0.5);
    reg.roster_apply(1, EffectKind::LaidOff, 25, 1.0, 5);
    reg.roster_apply(2, EffectKind::Ill, 10, 1.5, 6);
    reg.roster_leave(1, 3, 2.0);
    reg.roster_apply(1, EffectKind::LaidOff, -4, 2.5, 7);
    reg.promote(7);
    return reg;
  };
  PopRegistry a = build(), b = build();
  CHECK(a.save() == b.save());
  CHECK(a.roster(1)->in_state_members(EffectKind::LaidOff, 3.0) == b.roster(1)->in_state_members(EffectKind::LaidOff, 3.0));
  PopRegistry other(31);
  other.load(a.save());
  CHECK(other.save() == a.save());
  CHECK(other.hash() == a.hash());
  for (PopId p : {3u, 7u, 20u, 120u}) {
    PopSnapshot x, y;
    REQUIRE(a.snapshot(p, 3.0, x));
    REQUIRE(other.snapshot(p, 3.0, y));
    CHECK(x.roster_bits == y.roster_bits);
  }
  CHECK(a.hash() == 7377591193803251036ull);
}
