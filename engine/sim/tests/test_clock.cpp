// Spec 05: clock, speeds and slow-down.
#include <doctest.h>

#include "zs/clock.h"
#include "zs/demo.h"

using namespace zs;

TEST_CASE("1x is one game day per real hour: 48 ticks per real second") {
  CHECK(ticks_per_real_second(Speed::X1) == 48.0);
  CHECK(ticks_per_real_second(Speed::X10) == 480.0);
  CHECK(kTicksPerGameDay * kTickSeconds == 86400.0);
}

TEST_CASE("slow-down events drop to 1x or pause; others do not (A1)") {
  for (EventType t : {EventType::Alarm, EventType::AttackOnPop, EventType::NewWound, EventType::Breakdown,
                      EventType::NewSymptom, EventType::SurvivorFound}) {
    TimeControl tc{Speed::X10, false};
    CHECK(tc.on_event(t));
    CHECK(tc.speed == Speed::X1);
    TimeControl paused{Speed::RestUntilEvent, true};
    CHECK(paused.on_event(t));
    CHECK(paused.speed == Speed::Paused);
  }
  TimeControl tc{Speed::X10, false};
  CHECK_FALSE(tc.on_event(EventType::PlayerCommand));
  CHECK(tc.speed == Speed::X10);
}

TEST_CASE("speed never changes results: same hash whatever the app's speed (A1)") {
  // The world has no speed; an app at 1x simply calls step() fewer times per real second.
  auto a = demo::make_world(5);
  auto b = demo::make_world(5);
  TimeControl fast{Speed::RestUntilEvent, true};
  for (std::int64_t i = 0; i < 3 * kTicksPerGameDay; ++i) {
    demo::scripted_player(*a);
    a->step();
    demo::scripted_player(*b);
    b->step();
    for (EventId id : b->events_this_tick()) fast.on_event(b->log.get(id)->type);
    fast.speed = Speed::RestUntilEvent;  // the player keeps resuming
  }
  CHECK(a->state_hash() == b->state_hash());
}
