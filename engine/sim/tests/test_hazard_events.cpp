// Spec 04: hazards, exit sets, events and the schedule.
#include <doctest.h>

#include <array>

#include "zs/events.h"
#include "zs/hazard.h"

using namespace zs;

namespace {
// Mean firing time of a hazard with `rate` simulated in steps of `dt`.
double mean_firing_time(double rate, double dt, int runs) {
  Rng r(11);
  double sum = 0;
  for (int i = 0; i < runs; ++i) {
    double t = 0;
    while (!hazard_fires(r, static_cast<EntityId>(i), rate, dt)) t += dt;
    sum += t + dt;
  }
  return sum / runs;
}
}  // namespace

TEST_CASE("halving the step keeps the firing-time distribution (A2)") {
  const double rate = 1.0 / 30.0;  // mean 30 s
  CHECK(mean_firing_time(rate, 1.0, 20000) == doctest::Approx(30.0).epsilon(0.03));
  CHECK(mean_firing_time(rate, 0.5, 20000) == doctest::Approx(30.0).epsilon(0.03));
}

TEST_CASE("a zeroed hazard never fires and draws nothing (A1, metamorphic)") {
  Rng r(1);
  for (int i = 0; i < 1000; ++i) CHECK_FALSE(hazard_fires(r, 1, 0.0, 0.5));
  CHECK(r.counters().empty());
}

TEST_CASE("exit set: each hazard wins in proportion to its rate (A2)") {
  Rng r(3);
  const std::array<double, 2> rates = {1.0, 3.0};
  int wins0 = 0, wins1 = 0;
  for (int i = 0; i < 100000; ++i) {
    if (auto which = resolve_exit_set(r, static_cast<EntityId>(i), rates, 10.0)) (*which == 0 ? wins0 : wins1)++;
  }
  CHECK(static_cast<double>(wins0) / (wins0 + wins1) == doctest::Approx(0.25).epsilon(0.02));
}

TEST_CASE("events keep their causes; the chain walks back to the root (A1)") {
  EventLog log;
  const EventId root = log.append(0, EventType::RunStart, 0, 0);
  const EventId wound = log.append(5, EventType::NewWound, 7, root);
  const EventId symptom = log.append(9, EventType::NewSymptom, 7, wound);
  const auto chain = log.cause_chain(symptom);
  REQUIRE(chain.size() == 3);
  CHECK(chain[0].id == symptom);
  CHECK(chain[2].id == root);
}

TEST_CASE("the schedule pops due events in (tick, insertion) order") {
  Scheduler s;
  s.schedule({20, EventType::Scheduled, 1, 0, 0, 0});
  s.schedule({10, EventType::Scheduled, 2, 0, 0, 0});
  s.schedule({10, EventType::Scheduled, 3, 0, 0, 0});
  CHECK(s.pop_due(5).empty());
  const auto due = s.pop_due(15);
  REQUIRE(due.size() == 2);
  CHECK(due[0].subject == 2);
  CHECK(due[1].subject == 3);
  CHECK(s.pending().size() == 1);
}
