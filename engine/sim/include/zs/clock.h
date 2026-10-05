// One global clock, speeds and event slow-down. Spec: docs/tier0/specs/05-clock.md
#pragma once
#include <cstdint>

#include "zs/events.h"

namespace zs {

inline constexpr double kTickSeconds = 0.5;                  // game seconds per tick
inline constexpr std::int64_t kBodyStepTicks = 120;          // slow body step: 60 game s
inline constexpr std::int64_t kTicksPerGameHour = 7200;
inline constexpr std::int64_t kTicksPerGameDay = 24 * kTicksPerGameHour;
inline constexpr double kGameSecondsPerRealSecondAt1x = 24.0;  // 1 game day per real hour (Decided)

// Tick 0 is 08:00 on day 0 (a morning start). day() counts whole days since the start.
inline constexpr std::int64_t kStartTimeOfDayTicks = 8 * kTicksPerGameHour;

enum class Speed : std::uint8_t { Paused, X1, X3, X10, RestUntilEvent };

// Ticks per real second at a speed; RestUntilEvent returns 0, meaning "as fast as possible".
double ticks_per_real_second(Speed speed);

// Simulation time. Part of the saved state.
struct Clock {
  std::int64_t tick = 0;

  bool is_body_step() const { return tick % kBodyStepTicks == 0; }
  std::int64_t day() const { return tick / kTicksPerGameDay; }
  // Calendar day for display: day 0 is the start day, the next starts at midnight.
  std::int64_t calendar_day() const { return (tick + kStartTimeOfDayTicks) / kTicksPerGameDay; }
  // Hour of the day in [0, 24).
  double hour() const {
    return static_cast<double>((tick + kStartTimeOfDayTicks) % kTicksPerGameDay) / static_cast<double>(kTicksPerGameHour);
  }
};

// How fast the app runs the simulation. Not simulation state: speed never changes what a tick does,
// so it is kept outside the world and outside the state hash.
struct TimeControl {
  Speed speed = Speed::X1;
  bool pause_on_events = true;  // setting: slow-down events also pause

  // RimWorld rule: these events drop to 1x (and pause if the setting is on). Seeing a zombie does not.
  static bool is_slow_down_event(EventType type);
  // Applies the slow-down rule. Returns true if the speed changed.
  bool on_event(EventType type);
};

}  // namespace zs
