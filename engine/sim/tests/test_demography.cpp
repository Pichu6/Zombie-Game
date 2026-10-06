// Spec 28 P5: births, the jump, and its agreement with stepping.
#include <doctest.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "zs/demography.h"
#include "zs/detmath.h"
#include "zs/pop_registry.h"

using namespace zs;

namespace {
std::vector<Founder> founders(int n, int first_slot = 0) {
  std::vector<Founder> out;
  for (int i = 0; i < n; ++i) {
    const double age = 18.0 + (i * 7) % 47;
    out.push_back({census_pop_id(static_cast<std::uint32_t>(first_slot + i)), -age, age});
  }
  return out;
}

// An independent week-by-week simulation of the same rates, with its own draws (Purpose::Test keys).
struct Person { bool female; double birth_year; bool childless; bool first_done; double next_ok_age; };
struct Stepped { std::uint64_t births = 0, deaths = 0, alive = 0; };

Stepped stepped_run(std::uint64_t run, const std::vector<Founder>& fs, double to) {
  const Fertility f;
  const double dt = 1.0 / 52.0;
  std::uint64_t counter = 0;
  const auto u = [&] { return keyed_uniform01(4242, Purpose::Test, run, counter++); };
  std::vector<Person> people;
  const auto new_person = [&](double birth_year) {
    Person p;
    p.female = u() >= 0.512;
    p.birth_year = birth_year;
    p.childless = u() < f.childless_share;
    p.first_done = false;
    p.next_ok_age = f.first_min_age;
    return p;
  };
  const auto fertility_step = [&](Person& p, double age) {
    if (!p.female || p.childless || age < p.next_ok_age || age >= f.end_age) return false;
    const double mean = p.first_done ? f.gap_mean_extra : f.first_mean_delay;
    if (u() < 1.0 - detmath::exp(-dt / mean)) {
      p.first_done = true;
      p.next_ok_age = age + f.gap_min;
      return true;
    }
    return false;
  };
  Stepped out;
  std::vector<char> alive;
  for (const Founder& fo : fs) {
    Person p = new_person(fo.birth_year);
    // Founders' history before the epoch: run the fertility process silently up to their age.
    for (double a = f.first_min_age; a < fo.known_alive_age; a += dt) fertility_step(p, a);
    people.push_back(p);
    alive.push_back(1);
  }
  for (double t = 0.0; t < to; t += dt) {
    const std::size_t n = people.size();
    for (std::size_t i = 0; i < n; ++i) {
      if (!alive[i]) continue;
      Person& p = people[i];
      const double age = t - p.birth_year;
      const Mortality m = mortality_for(p.female ? Sex::Female : Sex::Male);
      const double h = m.constant_hazard + m.gompertz_b * detmath::exp(m.gompertz_c * age);
      if (u() < 1.0 - detmath::exp(-h * dt)) { alive[i] = 0; ++out.deaths; continue; }
      if (fertility_step(p, age)) {
        ++out.births;
        people.push_back(new_person(t));
        alive.push_back(1);
      }
    }
  }
  for (char a : alive) out.alive += a ? 1 : 0;
  return out;
}

double mean(const std::vector<double>& v) { double s = 0; for (double x : v) s += x; return s / static_cast<double>(v.size()); }
double variance(const std::vector<double>& v) { const double m = mean(v); double s = 0; for (double x : v) s += (x - m) * (x - m); return s / static_cast<double>(v.size() - 1); }
}  // namespace

TEST_CASE("births are a pure function of the mother: same inputs, same births, in order (A1)") {
  const Fertility f;
  for (std::uint32_t slot = 0; slot < 200; ++slot) {
    const PopId id = census_pop_id(slot);
    const PopBase base = derive_base(8, id, 0.0, 0.0);
    const auto a = births_of(8, id, base, -1e9, f);
    const auto b = births_of(8, id, base, -1e9, f);
    CHECK(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
      CHECK(a[i].year == b[i].year);
      CHECK(a[i].index == b[i].index);
      if (i) CHECK(a[i].year >= a[i - 1].year + f.gap_min);
      CHECK(a[i].year >= f.first_min_age);
      CHECK(a[i].year < f.end_age);
      CHECK(a[i].year < base.death_year);
    }
    if (base.sex == Sex::Male) CHECK(a.empty());
    // Asking only for the later births gives the tail of the same list.
    const auto tail = births_of(8, id, base, 30.0, f);
    std::size_t j = 0;
    for (const Birth& x : a) if (x.year > 30.0) { REQUIRE(j < tail.size()); CHECK(tail[j].index == x.index); CHECK(tail[j].year == x.year); ++j; }
    CHECK(j == tail.size());
  }
}

TEST_CASE("births per woman who lives to 45 are near 1.8 (A3, placeholder fertility)") {
  const Fertility f;
  double births = 0;
  int women = 0;
  for (std::uint32_t slot = 0; slot < 40000; ++slot) {
    PopBase base = derive_base(2, census_pop_id(slot), 0.0, 0.0);
    if (base.sex != Sex::Female) continue;
    base.death_year = 100.0;  // everyone lives
    births += static_cast<double>(births_of(2, census_pop_id(slot), base, -1e9, f).size());
    ++women;
  }
  CHECK(births / women > 1.6);
  CHECK(births / women < 2.0);
}

TEST_CASE("the jump is pure and splits cleanly: births and deaths of two windows add up (A1)") {
  const auto fs = founders(300);
  const auto whole = jump(5, fs, 0.0, 60.0);
  const auto again = jump(5, fs, 0.0, 60.0);
  CHECK(whole.births == again.births);
  CHECK(whole.deaths == again.deaths);
  CHECK(whole.alive_at_end == again.alive_at_end);
  const auto first = jump(5, fs, 0.0, 25.0);
  const auto second = jump(5, fs, 25.0, 60.0);
  CHECK(first.births + second.births == whole.births);
  CHECK(first.deaths + second.deaths == whole.deaths);
  CHECK(second.alive_at_start == first.alive_at_end);
  CHECK(second.alive_at_end == whole.alive_at_end);
}

TEST_CASE("people are conserved by the jump: alive at the end = alive at the start + births - deaths (A1)") {
  const auto fs = founders(500);
  for (double to : {5.0, 20.0, 45.0, 80.0}) {
    const auto c = jump(6, fs, 0.0, to);
    CHECK(c.alive_at_start == 500);
    CHECK(c.alive_at_end == c.alive_at_start + c.births - c.deaths);
    std::uint64_t by_gen = 0;
    for (std::uint64_t g : c.by_generation_alive) by_gen += g;
    CHECK(by_gen == c.alive_at_end);
  }
}

TEST_CASE("one jump agrees with week-by-week stepping in distribution (A2, 30 runs each)") {
  const auto fs = founders(400);
  const double to = 40.0;
  std::vector<double> jb, jd, ja, sb, sd, sa;
  for (std::uint64_t run = 0; run < 30; ++run) {
    const auto c = jump(1000 + run, fs, 0.0, to);
    jb.push_back(static_cast<double>(c.births));
    jd.push_back(static_cast<double>(c.deaths));
    ja.push_back(static_cast<double>(c.alive_at_end));
    const auto s = stepped_run(run, fs, to);
    sb.push_back(static_cast<double>(s.births));
    sd.push_back(static_cast<double>(s.deaths));
    sa.push_back(static_cast<double>(s.alive));
  }
  const auto close = [](const std::vector<double>& a, const std::vector<double>& b) {
    const double se = std::sqrt(variance(a) / static_cast<double>(a.size()) + variance(b) / static_cast<double>(b.size()));
    MESSAGE("jump mean " << mean(a) << ", stepped mean " << mean(b) << ", 4 standard errors " << 4 * se);
    return std::fabs(mean(a) - mean(b)) < 4 * se;
  };
  CHECK(close(jb, sb));
  CHECK(close(jd, sd));
  CHECK(close(ja, sa));
}

TEST_CASE("a born pop is found by its id, and a mother's early death removes the children she would have had (A1)") {
  PopRegistry reg(9);
  REQUIRE(reg.add_cohort({1, 0, 0, 400}));
  // Find a founder with at least two births after the epoch.
  PopId mother = kNoPop;
  std::vector<Birth> births;
  for (std::uint32_t slot = 0; slot < 400 && mother == kNoPop; ++slot) {
    PopSnapshot s;
    REQUIRE(reg.snapshot(census_pop_id(slot), 0.0, s));
    const auto b = births_of(9, census_pop_id(slot), s.base, 0.0);
    if (b.size() >= 2) { mother = census_pop_id(slot); births = b; }
  }
  REQUIRE(mother != kNoPop);
  const PopId first = birth_pop_id(mother, births[0].index);
  const PopId second = birth_pop_id(mother, births[1].index);
  PopSnapshot a, b;
  REQUIRE(reg.snapshot(first, 0.0, a));
  REQUIRE(reg.snapshot(second, 0.0, b));
  CHECK(a.base.birth_year == births[0].year);
  CHECK(b.base.birth_year == births[1].year);
  CHECK(mother_of(first) == mother);
  CHECK(a.base.death_year > a.base.birth_year);
  // The same pop, promoted, is the same pop.
  REQUIRE(reg.promote(first));
  PopSnapshot a2;
  REQUIRE(reg.snapshot(first, 0.0, a2));
  CHECK(a2.base.birth_year == a.base.birth_year);
  CHECK(a2.base.death_year == a.base.death_year);
  CHECK(a2.level == Level::Record);
  // An id that was never born is not a pop.
  PopSnapshot none;
  CHECK_FALSE(reg.snapshot(birth_pop_id(mother, 14), 0.0, none));
  // She dies before the second birth: the second child never exists, the first still does.
  REQUIRE(reg.set_death_year(mother, births[1].year - 0.01));
  CHECK(reg.snapshot(first, 0.0, a));
  CHECK_FALSE(reg.snapshot(second, 0.0, b));
}

TEST_CASE("the registry's projection counts the same people as the jump over its founders, and respects exceptions (A1)") {
  PopRegistry reg(11);
  REQUIRE(reg.add_cohort({1, 0, 0, 250}));
  REQUIRE(reg.add_cohort({2, 1, 1000, 150}));
  std::vector<Founder> fs;
  for (const std::uint32_t slot : [] { std::vector<std::uint32_t> v; for (std::uint32_t i = 0; i < 250; ++i) v.push_back(i); for (std::uint32_t i = 1000; i < 1150; ++i) v.push_back(i); return v; }()) {
    PopSnapshot s;
    REQUIRE(reg.snapshot(census_pop_id(slot), 0.0, s));
    const double age = -s.base.birth_year;
    fs.push_back({census_pop_id(slot), s.base.birth_year, age});
  }
  const auto direct = jump(11, fs, 0.0, 50.0);
  const auto via = reg.project_demography(0.0, 50.0);
  CHECK(via.births == direct.births);
  CHECK(via.deaths == direct.deaths);
  CHECK(via.alive_at_end == direct.alive_at_end);
  for (std::uint32_t slot = 0; slot < 250; ++slot) reg.set_death_year(census_pop_id(slot), 1.0);  // a plague of founders
  const auto after = reg.project_demography(0.0, 50.0);
  CHECK(after.births < via.births);
  CHECK(after.deaths >= 250);
}

TEST_CASE("pinned known answers for births and the jump, the same on GCC and Clang (A1 across compilers)") {
  const PopBase base = derive_base(8, census_pop_id(3), 0.0, 0.0);
  const auto births = births_of(8, census_pop_id(3), base, -1e9);
  const auto c = jump(5, founders(100), 0.0, 40.0);
  MESSAGE("sex " << static_cast<int>(base.sex) << " births " << births.size() << " | jump births " << c.births << " deaths " << c.deaths << " alive " << c.alive_at_end << " visited " << c.people_visited);
  CHECK(base.sex == Sex::Female);
  CHECK(births.size() == 3);
  CHECK(c.births == 18);
  CHECK(c.deaths == 51);
  CHECK(c.alive_at_end == 67);
  CHECK(c.people_visited == 118);
  CHECK(c.alive_at_end == c.alive_at_start + c.births - c.deaths);
}
