#include "zs/clock.h"

namespace zs {

double ticks_per_real_second(Speed speed) {
  const double at1x = kGameSecondsPerRealSecondAt1x / kTickSeconds;  // 48
  switch (speed) {
    case Speed::Paused: return 0.0;
    case Speed::X1: return at1x;
    case Speed::X3: return 3.0 * at1x;
    case Speed::X10: return 10.0 * at1x;
    case Speed::RestUntilEvent: return 0.0;
  }
  return 0.0;
}

bool TimeControl::is_slow_down_event(EventType type) {
  switch (type) {
    case EventType::Alarm:
    case EventType::AttackOnPop:
    case EventType::NewWound:
    case EventType::Breakdown:
    case EventType::NewSymptom:
    case EventType::SurvivorFound:
    case EventType::Death:
    case EventType::InfectionVisible:
    case EventType::InfectionWorse:
      return true;
    default:
      return false;
  }
}

bool TimeControl::on_event(EventType type) {
  if (!is_slow_down_event(type)) return false;
  const Speed target = pause_on_events ? Speed::Paused : Speed::X1;
  if (speed == target || speed == Speed::Paused) return false;
  speed = target;
  return true;
}

}  // namespace zs
