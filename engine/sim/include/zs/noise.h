// Noise field: how loud each sound is everywhere, through walls, doors and windows.
// Spec: docs/tier0/specs/18-noise-field.md
//
// Compressed model: a least-loss path over cells (Dijkstra; walls, doors and windows cost their
// transmission loss, stairs and open edges nothing) plus spherical spreading over the straight-line
// distance. Stateless: callers evaluate it when something changes.
#pragma once
#include <string>
#include <vector>

#include "zs/power.h"

namespace zs {

struct NoiseSource {
  Cell at;
  double db_1m = 0;
};

// Background at the listener: urban day/night while the grid lives; quiet after (spec 18 table).
// Decision 0064: two hearing thresholds on signal minus background. A sound is noticed at 0 dB (the listener turns
// toward it), and placed at +10 dB (the listener knows where it is).
inline constexpr double kNoticeDb = 0;
inline constexpr double kPlaceDb = 10;
// Background by hour: 55 dB by day and 45 at night while the grid lives; 35 and 27 once it has failed (decision 0064:
// 20 dB and 18 dB drops, so voices carry about ten times farther), night being 22:00 to 07:00.
double background_db(bool grid_alive, double hour);
// What a closed edge takes off a sound crossing it (dB); 0 when air passes.
double edge_loss_db(const SiteMap& map, const Assemblies& items, const EdgeRef& e);
// Level (dB) at every cell, sources added energetically. Index: level (in levels() order), then y, then x.
// Unreachable cells get kSilentDb.
inline constexpr double kSilentDb = -100;
std::vector<double> noise_field(const SiteMap& map, const Assemblies& items, const std::vector<NoiseSource>& sources);
// The least-loss tree from `from`: the loss to every cell and the previous cell on the way back (same indexing; npos at
// `from` and where unreachable). Same ties as path_loss.
struct PathTree {
  static constexpr std::size_t npos = static_cast<std::size_t>(-1);
  std::vector<double> loss;
  std::vector<std::size_t> prev;
};
PathTree path_tree(const SiteMap& map, const Assemblies& items, const Cell& from);
// The cells from `to` back to the tree's root, both included; empty when `to` is unreachable.
std::vector<Cell> path_to_root(const SiteMap& map, const PathTree& tree, const Cell& to);
// Least total edge loss (dB) from `from` to every cell (same indexing); 1e300 where unreachable.
std::vector<double> path_loss(const SiteMap& map, const Assemblies& items, const Cell& from);
std::size_t cell_index(const SiteMap& map, const Cell& c);
double distance_m(const Cell& a, const Cell& b);
double noise_at(const SiteMap& map, const std::vector<double>& field, const Cell& c);
// How far from its best way outdoors a source stays audible (signal - background > 3 dB), in m.
double audible_range_m(const SiteMap& map, const Assemblies& items, const NoiseSource& s, double background);
// The running generator and every pop's current noise.
std::vector<NoiseSource> noise_sources(const World& w, const Actors& actors, const Power& power);
// One level as text: '#' not walkable, '.' below background + 3 dB, '1'-'9' every 10 dB above it.
std::string noise_ascii(const SiteMap& map, const std::vector<double>& field, int z, double background);

}  // namespace zs
