// The living stocks of a town's region. Spec: docs/tier0/specs/103-living-stocks-in-the-region.md.
#include "zs/gen/region_stocks.h"

#include <algorithm>
#include <cmath>

#include "zs/detmath.h"
#include "zs/fields.h"

namespace zs::gen {

namespace {
constexpr std::uint32_t kOverfishedField = 10301;

// Rule 6: the capacity whose equilibrium's yearly mean is `target` (the equilibrium scales with the capacity).
bool calibrate(LivingStock& s, double target) {
  s.peak = 1.0;
  const double unit = living::equilibrium_mean(s);
  if (target <= 0 || unit <= 1e-9) return false;
  s.peak = target / unit;
  return true;
}
}  // namespace

std::vector<CellId> region_cells(const GeneratedWorld& world, int town) {
  std::vector<CellId> cells;
  if (town < 0) return cells;
  const CellId home = world.people.towns.at(static_cast<std::size_t>(town)).cell;
  // Spec 95's region: the cells whose nearest route node in time is the town's (the area its food flows sum over).
  const WorldRoutes& r = world.routes;
  const int node = r.node_of.empty() ? -1 : r.node_of[home];
  auto in_region = [&](CellId c) {
    if (c == home) return true;
    if (node >= 0 && !r.region_of.empty()) return r.region_of[c] == node;
    return world.use.market[c] == town;  // no routes: the cells marketed to it (spec 88)
  };
  for (CellId c = 0; c < static_cast<CellId>(world.grid.size()); ++c) {
    const bool land = world.fields.land(c);
    if (land ? in_region(c) : (!world.use.fish_port.empty() && world.use.fish_t[c] > 0 && world.use.fish_port[c] >= 0 &&
                               in_region(world.people.towns[static_cast<std::size_t>(world.use.fish_port[c])].cell)))
      cells.push_back(c);
  }
  return cells;
}

std::vector<LivingStock> cell_stocks(const GeneratedWorld& world, CellId c, std::uint64_t seed, const RegionStockParams& p) {
  std::vector<LivingStock> out;
  const WorldLand& land = world.land;
  const WorldLandUse& use = world.use;
  const fields::Climate clim = place_of(world.grid, world.fields, c, seed).climate;
  const double km2 = world.grid.area_km2(c);
  if (!world.fields.land(c)) {
    // Fish (LS-3): the take that lands spec 88's catch; a keyed share on the overfished root, the same catch from less.
    if (use.fish_t.empty() || use.fish_t[c] <= 0 || land.fish_catch_t[c] <= 0) return out;
    const double r = LandParams{}.fish_r;
    const double x = std::min(1.0, use.fish_t[c] / land.fish_catch_t[c]);
    const bool over = fields::keyed_unit(seed, c, kOverfishedField, 0) < p.overfished;
    const double q = 0.5 * r * (1.0 + (over ? 1.0 : -1.0) * std::sqrt(1.0 - x));
    out.push_back({c, LivingKind::Fish, clim, land.fish_k_t[c] * 1000.0, r, 1.0 - detmath::exp(-q)});
    return out;
  }
  // Game (LS-6, estimates): hunted near the town, less with distance; never more than 80 % of its growth.
  if (land.game_kg[c] > 0) {
    const double r = LandParams{}.game_r;
    const double km = use.market[c] >= 0 ? use.market_km[c] : 1e9;
    LivingStock s{c, LivingKind::Game, clim, 0, r, std::min(0.8 * r, p.hunt_share * detmath::exp(-km / p.hunt_km))};
    if (calibrate(s, land.game_kg[c])) out.push_back(s);
  }
  // Wood (LS-1): the worked share of the forest cut at half its growth rate; in dm3.
  if (land.wood_m3[c] > 0 && land.wood_r[c] > 0) {
    const double worked = land.forest_share[c] > 0 ? std::min(1.0, use.forest[c] / land.forest_share[c]) : 0.0;
    LivingStock s{c, LivingKind::Wood, clim, 0, land.wood_r[c], worked * p.log_of_rate * land.wood_r[c]};
    if (calibrate(s, land.wood_m3[c] * 1000.0)) out.push_back(s);
  }
  // Pasture (LS-4, LS-7): the year's grass on the pasture in use, a quarter eaten.
  const auto grows = living::growing_months(clim);
  const bool any = std::any_of(grows.begin(), grows.end(), [](bool b) { return b; });
  if (any && use.pasture[c] > 0) {
    const double g_m2 = std::max(0.0, p.grass_g_per_mm * std::min(clim.rain_mm, p.grass_rain_cap_mm) - p.grass_less_g);
    const double peak = g_m2 * use.pasture[c] * km2 * 1e6 / 1000.0;
    if (peak > 0) out.push_back({c, LivingKind::Pasture, clim, peak, 0, p.graze_share});
  }
  // Crops: spec 52's yield on the crops in use.
  if (any && use.crop[c] > 0) {
    const LandParams lp;
    const double peak = lp.yield_t_per_ha * land.fertility[c] * 1000.0 * use.crop[c] * km2 * 100.0;
    if (peak > 0) out.push_back({c, LivingKind::Crops, clim, peak, 0, 0});
  }
  return out;
}

LivingSetup region_stocks(const GeneratedWorld& world, int town, std::uint64_t seed, const RegionStockParams& p) {
  LivingSetup setup;
  setup.world_seed = seed;
  for (CellId c : region_cells(world, town))
    for (LivingStock& s : cell_stocks(world, c, seed, p)) setup.stocks.push_back(s);
  return setup;
}

LivingWorldChecks living_world_checks(const GeneratedWorld& world, std::uint64_t seed, const RegionStockParams& p) {
  LivingWorldChecks k;
  for (CellId c = 0; c < static_cast<CellId>(world.grid.size()); ++c) {
    const bool land = world.fields.land(c);
    if (land && world.land.wood_m3[c] <= 0 && world.use.pasture[c] <= 0) continue;
    if (!land && (world.use.fish_t.empty() || world.use.fish_t[c] <= 0)) continue;
    for (const LivingStock& s : cell_stocks(world, c, seed, p)) {
      switch (s.kind) {
        case LivingKind::Wood:
          k.wood_stock_m3 += living::equilibrium_mean(s) / 1000.0;
          k.wood_removed_m3 += living::equilibrium_take_year(s) / 1000.0;
          break;
        case LivingKind::Fish:
          ++k.fish_cells;
          k.fish_overfished += living::equilibrium_mean(s) < 0.5 * s.peak ? 1 : 0;
          break;
        case LivingKind::Pasture:
          k.grass_peak_kg += s.peak;
          k.grass_eaten_kg += living::equilibrium_take_year(s);
          break;
        default: break;
      }
    }
  }
  return k;
}

}  // namespace zs::gen
