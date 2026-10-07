// World fields (spec 31): plates -> elevation -> sea level -> temperature -> rain -> biome.
// Keyed draws only; arithmetic is + - * / sqrt and detmath, so a seed gives one world everywhere.
#include "zs/gen/world_fields.h"

#include <algorithm>
#include <cstddef>
#include <queue>
#include <tuple>

#include "fields.h"
#include "zs/detmath.h"
#include "zs/gen/keyed.h"

namespace zs::gen {

const char* biome_name(Biome b) {
  static const char* names[biome_count] = {"ocean",     "ice",     "tundra",  "boreal forest", "temperate forest",
                                           "grassland", "desert",  "savanna", "tropical forest", "mountain"};
  return names[static_cast<int>(b)];
}

const char* boundary_name(BoundaryKind k) {
  static const char* names[] = {"none", "collision", "subduction", "trench", "island arc", "ridge", "rift"};
  return names[static_cast<int>(k)];
}

Biome classify_biome(bool land, double elev, double t, double rain) {
  if (!land) return Biome::Ocean;
  if (t < -15) return Biome::Ice;
  if (elev >= 2500) return Biome::Mountain;
  if (t < -3) return Biome::Tundra;
  if (t < 6) return rain >= 250 ? Biome::BorealForest : Biome::Tundra;
  if (t < 18) {
    if (rain < 250) return Biome::Desert;
    return rain < 600 ? Biome::Grassland : Biome::TemperateForest;
  }
  if (rain < 400) return Biome::Desert;
  return rain < 1100 ? Biome::Savanna : Biome::TropicalForest;
}

namespace {

struct PlateInfo {
  bool continental = false;
  double base = 0, weight = 1, age = 0;
  Vec3 omega;  // rotation axis; velocity at point c is omega x c
};

struct Source {
  double v = 0;
  BoundaryKind k = BoundaryKind::None;
};

// Piecewise-linear rain by sine of latitude (spec 31 rule 7 calibration): wet equator, dry subtropics,
// wetter mid-latitudes, dry poles.
double lat_rain(double z) {
  static const double knots[][2] = {{0, 2200}, {0.15, 1900}, {0.35, 700}, {0.5, 350},
                                    {0.7, 1000}, {0.85, 850}, {1.0, 200}};
  const double a = z < 0 ? -z : z;
  for (std::size_t i = 1; i < sizeof(knots) / sizeof(knots[0]); ++i)
    if (a <= knots[i][0]) {
      const double f = (a - knots[i - 1][0]) / (knots[i][0] - knots[i - 1][0]);
      return knots[i - 1][1] + f * (knots[i][1] - knots[i - 1][1]);
    }
  return knots[6][1];
}

Vec3 unit(Vec3 v) {
  const double l = detmath::sqrt(dot(v, v));
  return {v.x / l, v.y / l, v.z / l};
}

}  // namespace

WorldFields generate_fields(const WorldGrid& g, const WorldParams& prm) {
  const std::size_t N = g.size();
  const Keyed rnd(prm.seed);
  const double spacing = detmath::sqrt(g.sphere_area_km2() / static_cast<double>(N));
  WorldFields f;
  f.plate.assign(N, 0);
  f.elevation_m.assign(N, 0);
  f.temp_mean_c.assign(N, 0);
  f.temp_range_c.assign(N, 0);
  f.rain_mm.assign(N, 0);
  f.biome.assign(N, Biome::Ocean);
  f.boundary_kind.assign(N, BoundaryKind::None);

  // ---- Plates (rule 2): best-candidate seeds, then a weighted flood so every plate is connected.
  int P = prm.plate_count;
  if (P <= 0) {
    // Plates are geophysics, not resolution: about 14 at Earth's area, never fewer than 40 cells each.
    const double earth_area = 510.1e6;
    P = std::clamp(static_cast<int>(14.0 * g.sphere_area_km2() / earth_area + 0.5), 6, 40);
  }
  P = std::max(1, std::min<int>(P, static_cast<int>(N / 40)));
  f.plate_count = P;
  std::vector<CellId> seeds;
  for (int p = 0; p < P; ++p) {
    CellId best = 0;
    double best_gap = -2;
    for (std::uint32_t k = 0; k < 8; ++k) {
      const CellId c = static_cast<CellId>(rnd.below(static_cast<std::uint64_t>(p), field::PlateSeed, N, k));
      double nearest = 2;  // smallest chord to an existing seed; larger is better spread
      for (const CellId s : seeds) {
        const Vec3 d{g.centre(c).x - g.centre(s).x, g.centre(c).y - g.centre(s).y, g.centre(c).z - g.centre(s).z};
        nearest = std::min(nearest, dot(d, d));
      }
      if (seeds.empty()) nearest = 1;
      if (nearest > best_gap) best_gap = nearest, best = c;
    }
    seeds.push_back(best);
  }
  std::vector<PlateInfo> plates(static_cast<std::size_t>(P));
  for (int p = 0; p < P; ++p) {
    PlateInfo& pi = plates[static_cast<std::size_t>(p)];
    const auto id = static_cast<std::uint64_t>(p);
    pi.weight = 0.5 + rnd.u01(id, field::PlateWeight);
    pi.age = rnd.u01(id, field::PlateAge);
    pi.omega = unit({rnd.bell(id, field::PlateOmega, 0) + 1e-3, rnd.bell(id, field::PlateOmega, 1),
                     rnd.bell(id, field::PlateOmega, 2)});
  }
  {
    using Item = std::tuple<double, CellId, int>;  // cost, cell, plate; ties by id
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> q;
    std::vector<double> cost(N, 1e300);
    std::vector<int> owner(N, -1);
    for (int p = 0; p < P; ++p) {
      cost[seeds[static_cast<std::size_t>(p)]] = 0;
      q.push({0.0, seeds[static_cast<std::size_t>(p)], p});
    }
    while (!q.empty()) {
      const auto [c, cell, p] = q.top();
      q.pop();
      if (owner[cell] >= 0) continue;
      owner[cell] = p;
      for (const CellId nb : g.neighbours(cell)) {
        if (owner[nb] >= 0) continue;
        const double step = (1.0 + 0.8 * rnd.u01(nb, field::CellCost)) / plates[static_cast<std::size_t>(p)].weight;
        if (c + step < cost[nb]) cost[nb] = c + step, q.push({c + step, nb, p});
      }
    }
    for (CellId i = 0; i < N; ++i) f.plate[i] = static_cast<std::uint16_t>(owner[i]);
  }

  // Plate types (rule 2): in keyed random order, plates turn continental until their area covers
  // about 1.35 times the land share (Earth: continental crust about 40% against 29% land).
  {
    std::vector<double> parea(static_cast<std::size_t>(P), 0.0);
    for (CellId i = 0; i < N; ++i) parea[f.plate[i]] += g.area_km2(i);
    std::vector<int> order(static_cast<std::size_t>(P));
    for (int p = 0; p < P; ++p) order[static_cast<std::size_t>(p)] = p;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
      const double ua = rnd.u01(static_cast<std::uint64_t>(a), field::PlateType);
      const double ub = rnd.u01(static_cast<std::uint64_t>(b), field::PlateType);
      return ua != ub ? ua < ub : a < b;
    });
    const double want = std::clamp(prm.land_share * 1.35, 0.05, 0.95) * g.sphere_area_km2();
    double have = 0;
    for (std::size_t k = 0; k < order.size(); ++k) {
      if (have >= want && k > 0) break;
      if (k + 1 == order.size() && P >= 2) break;  // keep at least one oceanic plate
      plates[static_cast<std::size_t>(order[k])].continental = true;
      have += parea[static_cast<std::size_t>(order[k])];
    }
    for (int p = 0; p < P; ++p) {
      PlateInfo& pi = plates[static_cast<std::size_t>(p)];
      const double off = (rnd.u01(static_cast<std::uint64_t>(p), field::PlateOffset) - 0.5) * 600.0;
      pi.base = (pi.continental ? 400.0 : -4000.0) + off;
    }
  }

  // ---- Boundaries (rule 3): per-cell source from the strongest convergence with a neighbouring plate.
  auto velocity = [&](int p, const Vec3& c) { return cross(plates[static_cast<std::size_t>(p)].omega, c); };
  std::vector<Source> src(N);
  std::vector<std::uint8_t> on_boundary(N, 0);
  for (CellId i = 0; i < N; ++i) {
    const int pi = f.plate[i];
    const PlateInfo& A = plates[static_cast<std::size_t>(pi)];
    for (const CellId j : g.neighbours(i)) {
      const int pj = f.plate[j];
      if (pj == pi) continue;
      on_boundary[i] = 1;
      const PlateInfo& B = plates[static_cast<std::size_t>(pj)];
      Vec3 d{g.centre(j).x - g.centre(i).x, g.centre(j).y - g.centre(i).y, g.centre(j).z - g.centre(i).z};
      d = unit(d);
      const Vec3 va = velocity(pi, g.centre(i)), vb = velocity(pj, g.centre(j));
      const double conv = (dot(va, d) - dot(vb, d)) / 2.0;  // [-1, 1]; positive = closing
      Source s;
      if (conv > 0) {
        if (A.continental && B.continental) s = {7000.0 * conv, BoundaryKind::Collision};
        else if (A.continental) s = {5000.0 * conv, BoundaryKind::Subduction};
        else if (B.continental) s = {-6000.0 * conv, BoundaryKind::Trench};
        else if (A.age >= B.age) s = {-6000.0 * conv, BoundaryKind::Trench};
        else s = {3500.0 * conv, BoundaryKind::IslandArc};
      } else {
        const double a = -conv;
        if (!A.continental && !B.continental) s = {1500.0 * a, BoundaryKind::Ridge};
        else s = {-700.0 * a, BoundaryKind::Rift};
      }
      if (std::abs(s.v) > std::abs(src[i].v)) src[i] = s;
    }
  }
  {
    std::vector<CellId> bsrc;
    for (CellId i = 0; i < N; ++i)
      if (on_boundary[i]) bsrc.push_back(i);
    f.boundary_cells = g.graph_distance(bsrc);
  }
  // Spread over a few hundred km: strongest effect wins, fading each step (rule 3).
  const int reach = std::max(1, static_cast<int>(350.0 / spacing + 0.5));
  const double decay = detmath::pow(0.1, 1.0 / static_cast<double>(reach));
  for (int it = 0; it < reach; ++it) {
    std::vector<Source> next = src;
    for (CellId i = 0; i < N; ++i)
      for (const CellId j : g.neighbours(i)) {
        const double cand = src[j].v * decay;
        if (std::abs(cand) > std::abs(next[i].v)) next[i] = {cand, src[j].k};
      }
    src.swap(next);
  }

  // ---- Elevation (rules 3 and 4): plate base + boundary + roughness, then a little smoothing.
  std::vector<double> rough(N);
  for (CellId i = 0; i < N; ++i) rough[i] = rnd.bell(i, field::Rough);
  for (int it = 0; it < 2; ++it) {
    std::vector<double> next(N);
    for (CellId i = 0; i < N; ++i) {
      double s = rough[i];
      for (const CellId j : g.neighbours(i)) s += rough[j];
      next[i] = s / static_cast<double>(1 + g.neighbours(i).size());
    }
    rough.swap(next);
  }
  double rms = 0;
  for (const double r : rough) rms += r * r;
  rms = detmath::sqrt(rms / static_cast<double>(N));
  std::vector<double> elev(N);
  for (CellId i = 0; i < N; ++i) {
    const double rg = rms > 0 ? rough[i] / rms : 0.0;
    elev[i] = plates[f.plate[i]].base + prm.relief * (src[i].v + 350.0 * rg);
    f.boundary_kind[i] = src[i].k;
  }
  for (int it = 0; it < 2; ++it) {
    std::vector<double> next(N);
    for (CellId i = 0; i < N; ++i) {
      double s = 0;
      for (const CellId j : g.neighbours(i)) s += elev[j];
      next[i] = 0.5 * elev[i] + 0.5 * s / static_cast<double>(g.neighbours(i).size());
    }
    elev.swap(next);
  }

  // ---- Sea level (rule 5): the elevation that makes the area-weighted land share exact to a cell.
  {
    std::vector<CellId> order(N);
    for (CellId i = 0; i < N; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](CellId a, CellId b) {
      return elev[a] != elev[b] ? elev[a] > elev[b] : a < b;
    });
    const double share = std::clamp(prm.land_share, 0.01, 0.99);
    const double target = share * g.sphere_area_km2();
    double cum = 0;
    std::size_t m = 0;
    while (m + 1 < N && std::abs(cum + g.area_km2(order[m]) - target) < std::abs(cum - target)) cum += g.area_km2(order[m++]);
    f.sea_level_raw_m = elev[order[m]];
    for (CellId i = 0; i < N; ++i) f.elevation_m[i] = std::clamp(elev[i] - f.sea_level_raw_m, -11000.0, 9000.0);
  }
  std::vector<CellId> sea;
  for (CellId i = 0; i < N; ++i)
    if (!f.land(i)) sea.push_back(i);
  f.sea_cells = g.graph_distance(sea);

  // ---- Temperature (rule 6): a latitude profile shifted to the global mean, lapse with height.
  // Zonal-mean sea-level temperature by sine of latitude (Earth: 26 at the equator, 0 at 60 degrees, -25 at the pole).
  auto profile = [](double z) {
    static const double knots[][2] = {{0, 26}, {0.5, 20}, {0.707, 8}, {0.866, -1}, {0.966, -15}, {1.0, -25}};
    const double a = z < 0 ? -z : z;
    for (std::size_t i = 1; i < sizeof(knots) / sizeof(knots[0]); ++i)
      if (a <= knots[i][0])
        return knots[i - 1][1] + (a - knots[i - 1][0]) / (knots[i][0] - knots[i - 1][0]) * (knots[i][1] - knots[i - 1][1]);
    return knots[5][1];
  };
  double mean = 0;
  for (CellId i = 0; i < N; ++i) mean += profile(g.sin_lat(i)) * g.area_km2(i);
  const double shift = prm.global_temp_c - mean / g.sphere_area_km2();
  for (CellId i = 0; i < N; ++i) {
    const double z = g.sin_lat(i), az = z < 0 ? -z : z;
    const double h = std::max(0.0, f.elevation_m[i]);
    f.temp_mean_c[i] = profile(z) + shift - 6.5 * h / 1000.0;
    const double inland_km = static_cast<double>(f.sea_cells[i]) * spacing;
    const double continentality = f.land(i) ? 0.5 + 0.8 * (1.0 - detmath::exp(-inland_km / 1200.0)) : 0.35;
    f.temp_range_c[i] = (3.0 + 32.0 * detmath::pow(az + 1e-9, 1.2)) * continentality;
  }

  // ---- Rain (rule 7): latitude profile times moisture carried by the wind off the sea, with
  // rain on rising ground and a shadow behind it. Gauss-Seidel in id order, fixed pass count.
  std::vector<double> M(N, 1.0);
  const double retain = detmath::exp(-spacing / 1800.0);
  const int passes = static_cast<int>(4000.0 / spacing) + 2;
  std::vector<Vec3> wind(N);
  for (CellId i = 0; i < N; ++i) {
    const Vec3& c = g.centre(i);
    const double az = c.z < 0 ? -c.z : c.z;
    const double ex = -c.y, ey = c.x, el = detmath::sqrt(ex * ex + ey * ey);
    const double dir = (az >= 0.5 && az < 0.87) ? 1.0 : -1.0;  // westerlies mid-latitude, easterlies else
    wind[i] = el > 1e-9 ? Vec3{dir * ex / el, dir * ey / el, 0} : Vec3{0, 0, 0};
  }
  std::vector<double> rise(N, 0.0);  // metres gained over upwind ground, for orographic rain
  for (int pass = 0; pass < passes; ++pass) {
    for (CellId i = 0; i < N; ++i) {
      if (!f.land(i)) {
        M[i] = 1.0;
        continue;
      }
      double wsum = 0, msum = 0, esum = 0;
      for (const CellId j : g.neighbours(i)) {
        Vec3 d{g.centre(j).x - g.centre(i).x, g.centre(j).y - g.centre(i).y, g.centre(j).z - g.centre(i).z};
        d = unit(d);
        double w = -dot(wind[i], d);
        if (wind[i].x == 0 && wind[i].y == 0) w = 1.0;
        if (w <= 0) continue;
        wsum += w;
        msum += w * M[j];
        esum += w * std::max(0.0, f.elevation_m[j]);
      }
      if (wsum <= 0) continue;
      const double up_m = msum / wsum, up_e = esum / wsum;
      rise[i] = std::max(0.0, f.elevation_m[i] - up_e);
      M[i] = up_m * retain * (1.0 - std::min(0.5, rise[i] / 2000.0));
    }
  }
  for (CellId i = 0; i < N; ++i) {
    const double base = lat_rain(g.sin_lat(i)) * (0.35 + 0.65 * M[i]);
    const double oro = f.land(i) ? 900.0 * (rise[i] / 1000.0) * M[i] : 0.0;
    f.rain_mm[i] = std::clamp((base + oro) * prm.moisture, 0.0, 4000.0);
  }

  // ---- Biome (rule 8).
  for (CellId i = 0; i < N; ++i)
    f.biome[i] = classify_biome(f.land(i), f.elevation_m[i], f.temp_mean_c[i], f.rain_mm[i]);
  return f;
}

}  // namespace zs::gen
