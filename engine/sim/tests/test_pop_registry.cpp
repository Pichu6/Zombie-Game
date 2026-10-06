// Spec 28 P2 and P3: cohort effects, aggregate agreement, records, promotion, save.
#include <doctest.h>

#include <set>
#include <vector>

#include "zs/pop_registry.h"

using namespace zs;

namespace {
PopRegistry make_registry(std::uint64_t seed = 12) {
  PopRegistry reg(seed);
  REQUIRE(reg.add_cohort({1, 0, 0, 120}));       // a workplace in region 0
  REQUIRE(reg.add_cohort({2, 0, 1000, 800}));    // a district in region 0
  REQUIRE(reg.add_cohort({3, 1, 5000, 3000}));   // a district in region 1
  return reg;
}
std::uint64_t pick(std::uint64_t k, std::uint64_t a, std::uint64_t b, std::uint64_t n) {
  return keyed_u64(99, Purpose::Test, a, b) % n + k;
}
}  // namespace

TEST_CASE("cohorts cannot overlap or repeat an id (A1)") {
  PopRegistry reg(1);
  CHECK(reg.add_cohort({1, 0, 100, 50}));
  CHECK_FALSE(reg.add_cohort({1, 0, 500, 50}));   // same id
  CHECK_FALSE(reg.add_cohort({2, 0, 120, 50}));   // slots overlap
  CHECK_FALSE(reg.add_cohort({3, 0, 60, 50}));    // overlaps from the left
  CHECK(reg.add_cohort({4, 0, 150, 10}));
  CHECK_FALSE(reg.add_cohort({5, 0, 0, 0}));      // empty
}

TEST_CASE("effects that cannot happen are refused and change nothing (A1)") {
  PopRegistry reg = make_registry();
  const auto before = reg.save();
  CHECK_FALSE(reg.apply({1, EffectKind::LaidOff, 121, 0.0, 7}));   // more than the cohort has
  CHECK_FALSE(reg.apply({1, EffectKind::LaidOff, -1, 0.0, 7}));    // nothing to give back
  CHECK_FALSE(reg.apply({9, EffectKind::LaidOff, 1, 0.0, 7}));     // no such cohort
  CHECK(reg.save() == before);
  CHECK(reg.apply({1, EffectKind::LaidOff, 100, 1.0, 7}));
  CHECK_FALSE(reg.apply({1, EffectKind::LaidOff, 21, 1.0, 7}));    // 100 + 21 > 120
  CHECK_FALSE(reg.apply({1, EffectKind::LaidOff, 1, 0.5, 7}));     // earlier than the last effect
}

TEST_CASE("the aggregate count equals the members found in the state, for random event sequences (A1)") {
  for (int trial = 0; trial < 60; ++trial) {
    PopRegistry reg = make_registry(100 + trial);
    double year = 0.0;
    for (int step = 0; step < 12; ++step) {
      const std::uint64_t cohort = 1 + pick(0, trial, step, 3);
      const auto kind = static_cast<EffectKind>(pick(0, trial, step + 100, 3));
      const std::int64_t now = static_cast<std::int64_t>(reg.count(cohort, kind, year));
      const std::int64_t room = static_cast<std::int64_t>(reg.cohort(cohort)->size) - now;
      std::int64_t delta = static_cast<std::int64_t>(pick(0, trial, step + 200, 60)) - 20;  // -20 .. 39
      delta = std::min(delta, room);
      delta = std::max(delta, -now);
      CHECK(reg.apply({cohort, kind, delta, year, static_cast<std::uint64_t>(step + 1)}));
      year += 0.1;
    }
    std::uint64_t region0 = 0, region1 = 0;
    for (std::uint64_t cid = 1; cid <= 3; ++cid)
      for (int k = 0; k < 3; ++k) {
        const auto kind = static_cast<EffectKind>(k);
        std::uint64_t found = 0;
        const Cohort& c = *reg.cohort(cid);
        for (std::uint64_t m = 0; m < c.size; ++m) found += reg.member_in(cid, m, kind, year) ? 1 : 0;
        CHECK(found == reg.count(cid, kind, year));
      }
    for (int k = 0; k < 3; ++k) {
      const auto kind = static_cast<EffectKind>(k);
      region0 += reg.count_region(0, kind, year);
      region1 += reg.count_region(1, kind, year);
      CHECK(reg.count_region(0, kind, year) == reg.count(1, kind, year) + reg.count(2, kind, year));
      CHECK(reg.count_region(1, kind, year) == reg.count(3, kind, year));
    }
  }
}

TEST_CASE("which order hit a member is answered from its rank, and give-backs take the newest first (A1)") {
  PopRegistry reg = make_registry();
  REQUIRE(reg.apply({1, EffectKind::LaidOff, 30, 1.0, 11}));   // order 11 hits 30
  REQUIRE(reg.apply({1, EffectKind::LaidOff, 20, 2.0, 22}));   // order 22 hits 20 more
  REQUIRE(reg.apply({1, EffectKind::LaidOff, -10, 3.0, 33}));  // 10 are rehired: the newest first
  const CohortPermutation perm(12, 1 * 64 + static_cast<std::uint64_t>(EffectKind::LaidOff), 120);
  std::uint64_t by11 = 0, by22 = 0, none = 0;
  for (std::uint64_t m = 0; m < 120; ++m) {
    const std::uint64_t r = perm.forward(m);
    const std::uint64_t why = reg.reason_of(1, m, EffectKind::LaidOff, 3.5);
    if (r < 30) { CHECK(why == 11); ++by11; }
    else if (r < 40) { CHECK(why == 22); ++by22; }
    else { CHECK(why == 0); ++none; }
  }
  CHECK(by11 == 30);
  CHECK(by22 == 10);
  CHECK(none == 80);
  CHECK(reg.count(1, EffectKind::LaidOff, 3.5) == 40);
  // The same cohort at an earlier year, before the give-back.
  CHECK(reg.count(1, EffectKind::LaidOff, 2.5) == 50);
  CHECK(reg.count(1, EffectKind::LaidOff, 0.5) == 0);
}

TEST_CASE("a pop's snapshot is the same whether or not it is promoted (A1)") {
  PopRegistry reg = make_registry();
  REQUIRE(reg.apply({2, EffectKind::Moved, 200, 1.0, 5}));
  for (PopId id = 1000; id < 1800; id += 7) {
    PopSnapshot lazy, rec;
    REQUIRE(reg.snapshot(id, 2.0, lazy));
    REQUIRE(reg.promote(id));
    REQUIRE(reg.snapshot(id, 2.0, rec));
    CHECK(rec.level == Level::Record);
    CHECK(lazy.level == Level::Lazy);
    CHECK(rec.state_bits == lazy.state_bits);
    CHECK(rec.base.sex == lazy.base.sex);
    CHECK(rec.base.birth_year == lazy.base.birth_year);
    CHECK(rec.base.death_year == lazy.base.death_year);
  }
  PopSnapshot none;
  CHECK_FALSE(reg.snapshot(50000, 0.0, none));  // not in any cohort
}

TEST_CASE("the aggregate matches what promoted pops report, for any promoted subset (A1)") {
  PopRegistry reg = make_registry();
  REQUIRE(reg.apply({3, EffectKind::LaidOff, 450, 1.0, 5}));
  REQUIRE(reg.apply({3, EffectKind::LaidOff, -50, 2.0, 6}));
  std::uint64_t reported = 0;
  for (PopId id = 5000; id < 8000; ++id) {
    reg.promote(id);
    PopSnapshot s;
    REQUIRE(reg.snapshot(id, 3.0, s));
    reported += (s.state_bits & 1u) ? 1 : 0;
  }
  CHECK(reg.counts().records == 3000);
  CHECK(reported == reg.count(3, EffectKind::LaidOff, 3.0));
  CHECK(reported == 400);
}

TEST_CASE("demotion keeps what the pop did, and refuses officeholders and followed pops (A1)") {
  PopRegistry reg = make_registry();
  const PopId id = 1234;
  PopSnapshot before, mid, after, again;
  REQUIRE(reg.snapshot(id, 0.0, before));
  REQUIRE(reg.promote(id));
  CHECK_FALSE(reg.promote(id));                   // already a record
  REQUIRE(reg.set_death_year(id, before.base.death_year - 3.0));  // something that happened to them
  REQUIRE(reg.snapshot(id, 0.0, mid));
  CHECK(mid.base.death_year == before.base.death_year - 3.0);
  REQUIRE(reg.demote(id));
  CHECK(reg.level(id) == Level::Lazy);
  REQUIRE(reg.snapshot(id, 0.0, after));
  CHECK(after.base.death_year == mid.base.death_year);   // the exception survived
  CHECK(after.level == Level::Lazy);
  REQUIRE(reg.promote(id));
  REQUIRE(reg.snapshot(id, 0.0, again));
  CHECK(again.base.death_year == mid.base.death_year);
  CHECK(again.base.birth_year == before.base.birth_year);
  CHECK(again.base.sex == before.base.sex);

  REQUIRE(reg.set_office(id, 4));
  CHECK_FALSE(reg.demote(id));                    // an officeholder is never lazy
  REQUIRE(reg.set_office(id, 0));
  REQUIRE(reg.set_followed(id, true));
  CHECK_FALSE(reg.demote(id));                    // nor is a followed pop
  REQUIRE(reg.set_followed(id, false));
  CHECK(reg.demote(id));
}

TEST_CASE("people are conserved through any promotion and demotion (A1)") {
  PopRegistry reg = make_registry();
  const PopCounts start = reg.counts();
  CHECK(start.total == 120 + 800 + 3000);
  CHECK(start.records == 0);
  for (int i = 0; i < 2000; ++i) {
    const PopId id = pick(0, i, 0, 2) == 0 ? pick(0, i, 1, 120) : 1000 + pick(0, i, 2, 800);
    if (pick(0, i, 3, 2) == 0) reg.promote(id); else reg.demote(id);
    const PopCounts c = reg.counts();
    CHECK(c.total == start.total);
    CHECK(c.records + c.lazy == c.total);
  }
}

TEST_CASE("save, load and save again gives the same bytes and hash (A1)") {
  PopRegistry reg = make_registry();
  REQUIRE(reg.apply({1, EffectKind::LaidOff, 40, 1.0, 5}));
  REQUIRE(reg.apply({2, EffectKind::Ill, 60, 1.5, 6}));
  for (PopId id : {3u, 9u, 1100u, 1500u}) REQUIRE(reg.promote(id));
  REQUIRE(reg.set_office(9, 2));
  REQUIRE(reg.set_death_year(3, 41.5));
  REQUIRE(reg.set_death_year(1700, 52.0));  // a lazy pop with an exception
  const auto bytes = reg.save();
  PopRegistry other(12);
  other.load(bytes);
  CHECK(other.save() == bytes);
  CHECK(other.hash() == reg.hash());
  for (PopId id : {3u, 9u, 20u, 1100u, 1700u}) {
    PopSnapshot a, b;
    REQUIRE(reg.snapshot(id, 2.0, a));
    REQUIRE(other.snapshot(id, 2.0, b));
    CHECK(a.base.death_year == b.base.death_year);
    CHECK(a.state_bits == b.state_bits);
    CHECK(a.office == b.office);
    CHECK(a.level == b.level);
  }
  PopRegistry wrong_seed(13);
  CHECK_THROWS(wrong_seed.load(bytes));
}

TEST_CASE("promoting and demoting a pop with nothing to remember leaves the save unchanged (A1)") {
  PopRegistry reg = make_registry();
  REQUIRE(reg.apply({3, EffectKind::Moved, 100, 1.0, 5}));
  const auto before = reg.save();
  for (PopId id = 5000; id < 5100; ++id) {
    REQUIRE(reg.promote(id));
    REQUIRE(reg.demote(id));
  }
  CHECK(reg.save() == before);
}

TEST_CASE("pinned known answer for a registry hash, the same on GCC and Clang (A1 across compilers)") {
  PopRegistry reg = make_registry();
  reg.apply({1, EffectKind::LaidOff, 40, 1.0, 5});
  reg.apply({3, EffectKind::Ill, 700, 1.5, 6});
  reg.promote(7);
  reg.set_death_year(7, 44.25);
  CHECK(reg.hash() == 10031871636624098524ull);
}
