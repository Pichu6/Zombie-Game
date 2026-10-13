// Spec 75: travel inside the block's town. docs/tier0/specs/75-travel-time-in-the-one-world.md
#include "zs/gen/town_travel.h"

#include <cmath>
#include <cstdlib>
#include <stdexcept>

#include "toml.hpp"
#include "zs/gen/keyed.h"

namespace zs::gen {

TownTravel load_town_travel(const std::string& gen_dir) {
  const std::string path = gen_dir + "/town_travel.toml";
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  TownTravel v;
  v.walk_kmh = t["walk_kmh"].value_or(v.walk_kmh);
  v.drive_kmh = t["drive_kmh"].value_or(v.drive_kmh);
  v.car_overhead_min = t["car_overhead_min"].value_or(v.car_overhead_min);
  if (v.walk_kmh <= 0 || v.drive_kmh <= 0 || v.car_overhead_min < 0) throw std::runtime_error(path + ": speeds must be positive");
  return v;
}

int plot_of_level(const TownLayout& layout, std::size_t stations, const std::vector<zs::Places::Open>& opened, int level) {
  if (level < 0) return -1;
  if (level < 10) return layout.lot_plot.empty() ? -1 : layout.lot_plot.back();  // the block: the last lot (spec 72)
  if (level < zs::Places::kFirstLevel) {
    const std::size_t i = static_cast<std::size_t>(level / 10 - 1);
    return i < stations && i < layout.lot_plot.size() ? layout.lot_plot[i] : -1;
  }
  for (const zs::Places::Open& o : opened)
    if (level >= o.z && level < o.z + 10) return o.plot;
  return -1;
}

int home_plot(const TownLayout& layout, zs::PopId pop, std::size_t census_size, std::uint64_t seed) {
  if (!zs::is_born(pop) && zs::founder_slot_of(pop) < census_size) return layout.lot_plot.empty() ? -1 : layout.lot_plot.back();
  std::vector<int> homes;
  for (std::size_t i = 0; i < layout.plots.size(); ++i) {
    const Plot& p = layout.plots[i];
    if (p.use == LandUse::Residential && p.households > 0 && p.lot < 0) homes.push_back(static_cast<int>(i));
  }
  if (homes.empty()) return -1;
  return homes[static_cast<std::size_t>(mix64(seed ^ hash_str("home_plot"), static_cast<std::uint64_t>(pop)) % homes.size())];
}

std::int64_t plot_minutes(const TownLayout& layout, int from, int to, const TownTravel& travel) {
  const Plot& a = layout.plots[static_cast<std::size_t>(from)];
  const Plot& b = layout.plots[static_cast<std::size_t>(to)];
  const double dx = std::abs((2.0 * a.x + a.w) - (2.0 * b.x + b.w)) / 2.0, dy = std::abs((2.0 * a.y + a.h) - (2.0 * b.y + b.h)) / 2.0;
  const double km = (dx + dy) / 1000.0;
  const double walk = 60.0 * km / travel.walk_kmh, drive = 60.0 * km / travel.drive_kmh + travel.car_overhead_min;
  const double minutes = std::ceil(walk < drive ? walk : drive);
  return minutes < 1 ? 1 : static_cast<std::int64_t>(minutes);
}

}  // namespace zs::gen
