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
double background_db(bool grid_alive, double hour);
// What a closed edge takes off a sound crossing it (dB); 0 when air passes.
double edge_loss_db(const SiteMap& map, const Assemblies& items, const EdgeRef& e);
// Level (dB) at every cell, sources added energetically. Index: level (in levels() order), then y, then x.
// Unreachable cells get kSilentDb.
inline constexpr double kSilentDb = -100;
std::vector<double> noise_field(const SiteMap& map, const Assemblies& items, const std::vector<NoiseSource>& sources);
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
