// Real time to game time: the speed ladder (0049, 0050), caps from what is attended (an embodied body in a fight
// forces 1x), alerts that pause, hysteresis so the speed does not flicker, and the accumulator that turns real
// seconds into whole ticks for World::jump. Spec: docs/tier0/specs/40-time.md
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "zs/archive.h"
#include "zs/clock.h"

namespace zs {

// The ladder of 0050: pause, slow, 1:1, 1 real hour = 1 game day (the start speed), faster, long steps.
enum class Pace : std::uint8_t {
  Pause = 0, Quarter = 1, Half = 2, Real = 3, X3 = 4, X10 = 5, DayPerHour = 6, X60 = 7, X360 = 8,
  DayPerSecond = 9, WeekPerSecond = 10, MonthPerSecond = 11, YearPerSecond = 12
};
inline constexpr Pace kStartPace = Pace::DayPerHour;  // 0050 item 1
double pace_factor(Pace s);  // game seconds per real second
const char* pace_name(Pace s);
Pace faster(Pace s);
Pace slower(Pace s);

class Pacer {
 public:
  // The player's choice.
  void set_requested(Pace s) { requested_ = s; }
  Pace requested() const { return requested_; }

  // A cap from a unit whose rules need it: an attended body in a fight caps at Real; a body walking at X3; a
  // watched project at anything. Caps are named so the time bar can say why it is slow. Releasing a cap keeps it
  // for `dwell_ticks` more (hysteresis).
  void cap(const std::string& who, Pace at_most, std::int64_t now);
  void release(const std::string& who, std::int64_t now);
  // An alert (vision: alerts and events slow or pause): pauses until acknowledged.
  void alert(const std::string& what, std::int64_t now);
  void acknowledge();
  bool alerted() const { return !alert_.empty(); }
  const std::string& alert_text() const { return alert_; }

  // The speed actually run: the requested one, capped by the tightest cap still in force, paused by an alert.
  Pace effective(std::int64_t now) const;
  const std::string* limiting(std::int64_t now) const;  // who caps it, or null

  // Real seconds in, whole ticks out (the fraction carries). This is what the app calls each frame, then World::jump.
  std::int64_t ticks_for(double real_seconds, std::int64_t now);

  void set_dwell_ticks(std::int64_t t) { dwell_ticks_ = t; }
  void save(ArchiveWriter& w) const;
  void load(ArchiveReader& r);

 private:
  struct Cap {
    Pace at_most;
    std::int64_t released_at = -1;  // -1 while held
  };
  Pace requested_ = kStartPace;
  std::map<std::string, Cap> caps_;
  std::string alert_;
  double carry_ = 0;  // fraction of a tick not yet run
  std::int64_t dwell_ticks_ = 10 * static_cast<std::int64_t>(1.0 / kTickSeconds);  // 10 game seconds
};

}  // namespace zs
