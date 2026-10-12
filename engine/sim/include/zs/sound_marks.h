// Sound marks: what one listener hears of the noise sources, as the view shows it. A reader of the world's noise: it
// never feeds the simulation. Spec: docs/tier0/specs/58-sound-marks.md; design: docs/design/sensors-and-soundscape.md.
//
// Decision 0064: a sound is noticed at 0 dB over the background (the listener turns toward it) and placed at +10 dB
// (the listener knows where it is). A noticed sound has only a direction, with an error; a placed one has a place, with
// a smaller error. Through walls the sound seems to come from the last opening on its way to the listener, so a sound
// behind a closed door is heard at the door, not at the source.
#pragma once
#include <cstdint>
#include <utility>
#include <vector>

#include "zs/noise.h"

namespace zs {

struct SoundMark {
  enum class Kind { Noticed, Placed };
  Kind kind = Kind::Noticed;
  double over_db = 0;       // signal minus background at the listener
  double bearing = 0;       // radians, 0 east, pi/2 north (north is toward smaller y, up on the map); toward where it seems to come from
  Cell apparent{};          // where it seems to come from: the opening, or the source in the open; Noticed: same, but only the bearing is meant
  double distance_m = 0;    // to the apparent place, in steps of 5 m (a placed sound only; 0 when noticed)
  std::size_t source = 0;   // index into the sources given
};

// The marks one listener hears, one per audible source (not summed). `key` makes the error: the same key gives the same
// marks, so a mark does not jitter every frame; the view changes it every few game seconds. A source on the listener's
// own cell (its own noise) is not a mark.
std::vector<SoundMark> sound_marks(const SiteMap& map, const Assemblies& items, const Cell& listener,
                                   const std::vector<NoiseSource>& sources, double background_db, std::uint64_t key);


// A short memory of what was heard, so a mark does not vanish the instant its sound stops: it stays as an echo that
// fades over `kFadeTicks`. A reader like sound_marks: never fed back to the simulation, never saved.
struct SoundEcho {
  SoundMark mark;
  std::int64_t last_heard = 0;
  // 1 just after it was last heard, 0 when it is gone.
  double strength(std::int64_t now) const;
};

class SoundMemory {
 public:
  static constexpr std::int64_t kFadeTicks = 60;  // 30 game seconds at 2 ticks a second

  // Call with the marks heard now each time they are recomputed, even when there are none: what is in `marks` is heard
  // now; what was heard before and is not (the same source index) becomes an echo.
  void hear(const std::vector<SoundMark>& marks, std::int64_t tick);
  // The echoes at `now`: not heard now, not yet faded, one per source, loudest memory first (the most recent).
  std::vector<SoundEcho> echoes(std::int64_t now) const;
  void clear();

 private:
  struct Entry {
    SoundMark mark;
    std::int64_t last_heard = 0;
  };
  std::vector<std::pair<std::size_t, Entry>> entries_;  // by source index
  std::int64_t latest_ = -1;                             // the tick of the last hear()
};

// Compass word for a bearing, for text (north is toward smaller y, up on the map).
const char* compass_word(double bearing);

}  // namespace zs
