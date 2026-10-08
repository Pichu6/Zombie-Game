// Spec 38: the encounter rule, its measured rates, the compartments against the entity-level run (A2, with the jump
// harness), the reproduction number and the design's claims about shamblers and runners.
#include <doctest.h>

#include <cmath>
#include <vector>

#include "jumps.h"
#include "zs/outbreak.h"

using namespace zs;
using namespace zs::outbreak;

namespace {

Micro shambler_warned() {
  Micro m;
  m.zombie_speed_mps = 1.0;
  m.warning = 0.8;
  return m;
}
Micro runner_unwarned() {
  Micro m;
  m.zombie_speed_mps = 4.0;
  m.warning = 0.0;
  return m;
}
Micro middling() {
  Micro m;
  m.zombie_speed_mps = 2.0;
  m.warning = 0.3;
  m.contacts_per_day = 1.5;
  return m;
}

// Final infected-ever counts of a town over `days`, aggregate and entity level, over many seeds.
struct Runs {
  jumps::Sample aggregate, entities;
  std::uint64_t took_off_aggregate = 0, took_off_entities = 0;
};

Runs run_town(const Micro& m, std::int64_t people, std::int64_t zombies, std::int64_t days, int agg_seeds, int ent_seeds, std::int64_t took_off_at) {
  const Rates r = measure(m, 99, 200'000);
  Runs out;
  for (int s = 1; s <= agg_seeds; ++s) {
    Compartments c;
    c.s = people;
    c.z = zombies;
    aggregate_days(c, r, m, static_cast<std::uint64_t>(s), 7, 0, days);
    out.aggregate.push_back(static_cast<double>(c.infected_ever() - zombies));
    if (c.infected_ever() - zombies >= took_off_at) ++out.took_off_aggregate;
  }
  for (int s = 1; s <= ent_seeds; ++s) {
    Entities e(people, zombies);
    for (std::int64_t d = 0; d < days; ++d) e.run_day(m, static_cast<std::uint64_t>(1000 + s), 7, d);
    const Compartments c = e.counts();
    out.entities.push_back(static_cast<double>(c.infected_ever() - zombies));
    if (c.infected_ever() - zombies >= took_off_at) ++out.took_off_entities;
  }
  return out;
}

}  // namespace

TEST_CASE("the encounter rule: keyed, and its measured rates match its closed-form chances (A1, A2)") {
  const Micro m = middling();
  CHECK(encounter(m, 1, 5, 3) == encounter(m, 1, 5, 3));
  bool differs = false;
  for (std::uint32_t k = 0; k < 50 && !differs; ++k) differs = encounter(m, 1, 5, k) != encounter(m, 2, 5, k);
  CHECK(differs);
  const Rates r = measure(m, 3, 200'000);
  CHECK(r.bite == doctest::Approx(bite_chance(m)).epsilon(0.02));
  CHECK(r.destroyed == doctest::Approx(kill_chance(m)).epsilon(0.02));
  CHECK(r.convert == doctest::Approx(0.9));
  // The chances follow the inputs: a faster zombie bites more; warned people are bitten less and destroy more.
  Micro fast = m; fast.zombie_speed_mps = 4.0;
  Micro warned = m; warned.warning = 0.9;
  CHECK(bite_chance(fast) > bite_chance(m));
  CHECK(bite_chance(warned) < bite_chance(m));
  CHECK(kill_chance(warned) > kill_chance(m));
  CHECK(kill_chance(runner_unwarned()) == 0);
}

TEST_CASE("keyed binomial: pure, in range, with the right mean and spread (A2)") {
  CHECK(keyed_binomial(1, 2, 3, 4, 100, 0.3) == keyed_binomial(1, 2, 3, 4, 100, 0.3));
  CHECK(keyed_binomial(1, 2, 3, 4, 0, 0.3) == 0);
  CHECK(keyed_binomial(1, 2, 3, 4, 10, 0.0) == 0);
  CHECK(keyed_binomial(1, 2, 3, 4, 10, 1.0) == 10);
  double sum = 0, ss = 0;
  const int n = 4000;
  for (int k = 0; k < n; ++k) {
    const auto x = keyed_binomial(5, 1, 9, static_cast<std::uint32_t>(k), 50, 0.2);
    CHECK(x >= 0);
    CHECK(x <= 50);
    sum += static_cast<double>(x);
    ss += static_cast<double>(x) * static_cast<double>(x);
  }
  const double mean = sum / n, var = ss / n - mean * mean;
  CHECK(mean == doctest::Approx(10.0).epsilon(0.03));
  CHECK(var == doctest::Approx(8.0).epsilon(0.12));
  // Large n p: the normal fallback stays in range and near the mean.
  const auto big = keyed_binomial(5, 1, 9, 1, 1'000'000, 0.5);
  CHECK(big > 495'000);
  CHECK(big < 505'000);
}

TEST_CASE("enumeration: the entity-level counts are sums and conserve people plus zombies (A1)") {
  Entities e(300, 3);
  const Micro m = middling();
  for (std::int64_t d = 0; d < 20; ++d) {
    e.run_day(m, 11, 1, d);
    const Compartments c = e.counts();
    CHECK(c.s + c.e + c.z + c.r == 303);
    std::int64_t by_hand = 0;
    for (const Entity& x : e.all()) if (x.state != State::Susceptible) ++by_hand;
    CHECK(by_hand == c.infected_ever());
  }
  Compartments c;
  c.s = 300;
  c.z = 3;
  const Rates r = measure(m, 99, 100'000);
  aggregate_days(c, r, m, 11, 1, 0, 20);
  CHECK(c.s + c.e + c.z + c.r == 303);
  CHECK(c.s >= 0);
  CHECK(c.z >= 0);
}

TEST_CASE("the aggregate and the entity-level run of the same town agree in distribution (A2, Anderson–Darling)") {
  // A middling outbreak in a town of 400 with 3 zombies over 30 days: neither a sure fizzle nor a sure wave, so the
  // distribution of infected has a shape to compare. 300 aggregate seeds against 60 entity-level ones.
  const Runs runs = run_town(middling(), 400, 3, 30, 300, 60, 40);
  const jumps::ADResult ad = jumps::anderson_darling_ksample({runs.aggregate, runs.entities});
  CHECK(!ad.reject_5pct);
  CHECK(jumps::means_agree(runs.aggregate, runs.entities, 4));
  CHECK(jumps::proportions_agree(runs.took_off_aggregate, 300, runs.took_off_entities, 60, 3));
  // The same for a sure-fizzle scenario, where the bulk of mass is at zero.
  const Runs fizzle = run_town(shambler_warned(), 400, 3, 30, 300, 60, 40);
  CHECK(jumps::means_agree(fizzle.aggregate, fizzle.entities, 4));
  CHECK(jumps::proportions_agree(fizzle.took_off_aggregate, 300, fizzle.took_off_entities, 60, 3));
}

TEST_CASE("the reproduction number is built from the measured rates: a warned shambler fizzles, an unwarned runner takes off (A3)") {
  const Rates slow = measure(shambler_warned(), 1, 100'000);
  const Rates fast = measure(runner_unwarned(), 1, 100'000);
  CHECK(slow.r0() < 1.0);
  CHECK(fast.r0() > 5.0);
  // The design's claim (world simulation §5): each shambler infects well under one person once people fight back.
  CHECK(slow.r0() < 0.5);
  // In a town of 400 over 40 days: the shambler almost always fizzles, the runner almost always takes off.
  const Runs s = run_town(shambler_warned(), 400, 3, 40, 200, 1, 40);
  const Runs f = run_town(runner_unwarned(), 400, 3, 40, 200, 1, 40);
  CHECK(static_cast<double>(s.took_off_aggregate) / 200 < 0.15);
  CHECK(static_cast<double>(f.took_off_aggregate) / 200 > 0.85);
  // A runner among the unwarned converts the whole town: nobody destroys it, so it stops only when nobody is left.
  double mean_f = 0;
  for (double x : f.aggregate) mean_f += x;
  CHECK(mean_f / 200 > 350);
}
