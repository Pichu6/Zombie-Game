// Sound marks. Spec: docs/tier0/specs/58-sound-marks.md
#include "zs/sound_marks.h"

#include <algorithm>
#include <cmath>

#include "zs/detmath.h"
#include "zs/world.h"

namespace zs {
namespace {

constexpr double kBearingErrorRad = 0.5;   // a noticed sound: up to about 29 degrees either way
constexpr double kPlacedErrorBase = 1.0;   // a placed sound: at least this many cells...
constexpr double kPlacedErrorPer20M = 1.0; // ...plus one cell per 20 m of distance
constexpr double kPlacedErrorMax = 4.0;

// A value in [-1, 1] from the key, the source and which draw: SplitMix64, the same on every compiler.
double unit(std::uint64_t key, std::size_t source, int which) {
  std::uint64_t z = key + 0x9E3779B97F4A7C15ull * (static_cast<std::uint64_t>(source) * 2 + static_cast<std::uint64_t>(which) + 1);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
  z ^= z >> 31;
  return static_cast<double>(z >> 11) / static_cast<double>(1ull << 52) - 1.0;  // [-1, 1)
}

}  // namespace

std::vector<SoundMark> sound_marks(const SiteMap& map, const Assemblies& items, const Cell& listener,
                                   const std::vector<NoiseSource>& sources, double background, std::uint64_t key) {
  std::vector<SoundMark> out;
  if (!map.in_bounds(listener) || !map.walkable(listener)) return out;
  const PathTree tree = path_tree(map, items, listener);  // loss is symmetric: one tree serves every source
  for (std::size_t s = 0; s < sources.size(); ++s) {
    const NoiseSource& src = sources[s];
    if (src.at == listener || !map.in_bounds(src.at)) continue;  // the listener's own noise
    const std::size_t i = cell_index(map, src.at);
    if (tree.loss[i] >= 1e299) continue;
    const double level = src.db_1m - 20 * detmath::log10(std::max(1.0, distance_m(src.at, listener))) - tree.loss[i];
    const double over = level - background;
    if (over < kNoticeDb) continue;

    // The apparent place: the last opening on the way to the listener, else the source itself.
    const std::vector<Cell> way = path_to_root(map, tree, src.at);  // source ... listener
    Cell apparent = src.at;
    for (std::size_t k = way.size(); k-- > 1;)  // from the listener's end back toward the source
      if (edge_loss_db(map, items, SiteMap::edge_between(way[k - 1], way[k])) > 0) {
        apparent = way[k - 1];
        break;
      }

    SoundMark m;
    m.over_db = over;
    m.source = s;
    m.kind = over >= kPlaceDb ? SoundMark::Kind::Placed : SoundMark::Kind::Noticed;
    const double dist = distance_m(apparent, listener);
    if (m.kind == SoundMark::Kind::Placed) {
      const double radius = std::min(kPlacedErrorMax, kPlacedErrorBase + kPlacedErrorPer20M * dist / 20.0);
      Cell shown{apparent.z, apparent.x + static_cast<int>(std::lround(unit(key, s, 0) * radius)),
                 apparent.y + static_cast<int>(std::lround(unit(key, s, 1) * radius))};
      if (!map.in_bounds(shown) || !map.walkable(shown)) shown = apparent;  // keep it on a cell people can stand on
      m.apparent = shown;
      m.distance_m = 5.0 * std::round(distance_m(shown, listener) / 5.0);
      m.bearing = std::atan2(static_cast<double>(listener.y - shown.y), static_cast<double>(shown.x - listener.x));
    } else {
      m.apparent = apparent;
      const double truth = std::atan2(static_cast<double>(listener.y - apparent.y), static_cast<double>(apparent.x - listener.x));
      m.bearing = truth + unit(key, s, 0) * kBearingErrorRad;
    }
    out.push_back(m);
  }
  return out;
}

const char* compass_word(double bearing) {
  static const char* const words[8] = {"east", "north-east", "north", "north-west", "west", "south-west", "south", "south-east"};
  double a = std::fmod(bearing, 2 * 3.14159265358979323846);
  if (a < 0) a += 2 * 3.14159265358979323846;
  return words[static_cast<int>(std::lround(a / (3.14159265358979323846 / 4))) % 8];
}

}  // namespace zs
