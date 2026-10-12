// Noise field. Spec: docs/tier0/specs/18-noise-field.md
#include "zs/noise.h"

#include <algorithm>
#include <queue>
#include <tuple>

#include "zs/detmath.h"
#include "zs/world.h"

namespace zs {
namespace {

constexpr double kHearingThresholdDb = kNoticeDb;  // spec 18 rule 2, decision 0064: noticed at 0 dB over the background
constexpr double kBoardLossDb = 3;         // per board nailed over an opening (estimate)
constexpr double kStoreyM = 3;             // distance between levels

std::size_t index_of(const SiteMap& m, const Cell& c) {
  const auto& lv = m.levels();
  const auto li = static_cast<std::size_t>(std::find(lv.begin(), lv.end(), c.z) - lv.begin());
  return (li * static_cast<std::size_t>(m.height()) + static_cast<std::size_t>(c.y)) * static_cast<std::size_t>(m.width()) +
         static_cast<std::size_t>(c.x);
}

Cell cell_at(const SiteMap& m, std::size_t i) {
  const auto w = static_cast<std::size_t>(m.width()), h = static_cast<std::size_t>(m.height());
  return {m.levels()[i / (w * h)], static_cast<int>(i % w), static_cast<int>((i / w) % h)};
}

// Least total edge loss from `from` to every cell; ties broken by steps, then cell index.
}  // namespace

std::vector<double> path_loss(const SiteMap& map, const Assemblies& items, const Cell& from) {
  const std::size_t n = map.levels().size() * static_cast<std::size_t>(map.width() * map.height());
  std::vector<double> best(n, 1e300);
  using Key = std::tuple<double, int, std::size_t>;
  std::priority_queue<Key, std::vector<Key>, std::greater<Key>> open;
  if (!map.in_bounds(from) || !map.walkable(from)) return best;
  best[index_of(map, from)] = 0;
  open.emplace(0.0, 0, index_of(map, from));
  while (!open.empty()) {
    const auto [loss, steps, i] = open.top();
    open.pop();
    if (loss > best[i]) continue;
    const Cell c = cell_at(map, i);
    auto relax = [&](const Cell& nb, double cost) {
      if (!map.in_bounds(nb) || !map.walkable(nb)) return;
      const std::size_t j = index_of(map, nb);
      if (loss + cost < best[j]) {
        best[j] = loss + cost;
        open.emplace(best[j], steps + 1, j);
      }
    };
    for (const Cell& nb : {Cell{c.z, c.x + 1, c.y}, Cell{c.z, c.x - 1, c.y}, Cell{c.z, c.x, c.y + 1}, Cell{c.z, c.x, c.y - 1}})
      if (map.in_bounds(nb)) relax(nb, edge_loss_db(map, items, SiteMap::edge_between(c, nb)));
    for (const Stairs& s : map.stairs()) {
      if (s.a == c) relax(s.b, 0);
      if (s.b == c) relax(s.a, 0);
    }
  }
  return best;
}

double distance_m(const Cell& a, const Cell& b) {
  const double dx = a.x - b.x, dy = a.y - b.y, dz = (a.z - b.z) * kStoreyM;
  return detmath::sqrt(dx * dx + dy * dy + dz * dz);
}

std::size_t cell_index(const SiteMap& m, const Cell& c) { return index_of(m, c); }

double background_db(bool grid_alive, double hour) {
  const bool night = hour < 7 || hour >= 22;  // WHO night period (approx.)
  if (grid_alive) return night ? 45 : 55;
  return night ? 27 : 35;  // decision 0064
}

double edge_loss_db(const SiteMap& map, const Assemblies& items, const EdgeRef& e) {
  if (map.air_passes(e)) return 0;
  const Edge& ed = map.edge(e);
  double loss = ed.assembly && items.exists(ed.assembly) ? items.def(items.get(ed.assembly).def).sound_loss_db : 30;
  if (ed.barricade && items.exists(ed.barricade))
    for (EntityId part : items.get(ed.barricade).slots)
      if (part != 0 && items.def(items.get(part).def).family == "plank") loss += kBoardLossDb;
  return loss;
}

std::vector<double> noise_field(const SiteMap& map, const Assemblies& items, const std::vector<NoiseSource>& sources) {
  const std::size_t n = map.levels().size() * static_cast<std::size_t>(map.width() * map.height());
  std::vector<double> energy(n, 0);
  for (const NoiseSource& s : sources) {
    const std::vector<double> loss = path_loss(map, items, s.at);
    for (std::size_t i = 0; i < n; ++i) {
      if (loss[i] >= 1e299) continue;
      const double level = s.db_1m - 20 * detmath::log10(std::max(1.0, distance_m(s.at, cell_at(map, i)))) - loss[i];
      energy[i] += detmath::pow(10.0, level / 10);
    }
  }
  std::vector<double> out(n, kSilentDb);
  for (std::size_t i = 0; i < n; ++i)
    if (energy[i] > 0) out[i] = 10 * detmath::log10(energy[i]);
  return out;
}

double noise_at(const SiteMap& map, const std::vector<double>& field, const Cell& c) {
  return map.in_bounds(c) ? field[index_of(map, c)] : kSilentDb;
}

double audible_range_m(const SiteMap& map, const Assemblies& items, const NoiseSource& s, double background) {
  const std::vector<double> loss = path_loss(map, items, s.at);
  double exit = 1e300;
  for (std::size_t i = 0; i < loss.size(); ++i)
    if (loss[i] < exit && !is_indoor(map.floor(cell_at(map, i))) && map.walkable(cell_at(map, i))) exit = loss[i];
  if (exit >= 1e299) return 0;
  const double margin = s.db_1m - exit - background - kHearingThresholdDb;
  return margin > 0 ? detmath::pow(10.0, margin / 20) : 0;
}

std::vector<NoiseSource> noise_sources(const World&, const Actors& actors, const Power& power) {
  std::vector<NoiseSource> out;
  if (power.generator_running()) out.push_back({power.generator_cell(), power.generator().noise_db_1m});
  for (const auto& [id, a] : actors.actors())
    if (a.noise_db > 0) out.push_back({a.pos, a.noise_db});
  return out;
}

std::string noise_ascii(const SiteMap& map, const std::vector<double>& field, int z, double background) {
  std::string out;
  for (int y = 0; y < map.height(); ++y) {
    for (int x = 0; x < map.width(); ++x) {
      const Cell c{z, x, y};
      if (!map.walkable(c)) {
        out += '#';
        continue;
      }
      const double margin = noise_at(map, field, c) - background - kHearingThresholdDb;
      out += margin <= 0 ? '.' : static_cast<char>('1' + std::min(8, static_cast<int>(margin / 10)));
    }
    out += '\n';
  }
  return out;
}

}  // namespace zs
