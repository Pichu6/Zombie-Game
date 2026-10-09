// World fields (spec 31): plates -> elevation -> sea level -> temperature -> rain -> biome.
// Keyed draws only; arithmetic is + - * / sqrt and detmath, so a seed gives one world everywhere.
#include "zs/gen/world_fields.h"

#include <algorithm>
#include <cstddef>
#include <queue>
#include <tuple>

#include "fields.h"
#include "noise.h"
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

// Rules 3b, 3c and 4 (estimates; tuned against the checks in the spec's As built).
constexpr double kShelfM = -500;          // base height at the continent-ocean contact
constexpr double kPlainKm = 900;          // from the contact to the continental plate's base height
constexpr double kSlopeKm = 900;          // from the contact down to the ocean floor
constexpr double kNoiseM = 1400;          // fractal noise amplitude near the coast
constexpr double kNoiseInland = 0.25;     // share of it left deep inland
constexpr double kNoiseFadeKm = 900;
constexpr double kShapeM = 800;           // the longest waves, which carve continents across plates
constexpr double kCostContrast = 4.0;     // rule 2: a plate step costs exp(4 n) times its length, n the cost noise
constexpr double kWarp = 0.25;            // domain warp, in unit-sphere lengths (about 1,600 km on Earth)
constexpr double kHotspotsAtEarth = 45;   // Earth: about 40-50 hotspots
constexpr int kHotspotChain = 6;
constexpr double kHotspotStepKm = 350, kHotspotSigmaKm = 150, kHotspotM = 8000;

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

// A keyed point uniform on the unit sphere.
Vec3 random_point(const Keyed& rnd, std::uint64_t id, std::uint32_t fld, std::uint32_t k) {
  const double z = 2.0 * rnd.u01(id, fld, 2 * k) - 1.0;
  const double phi = 6.283185307179586 * rnd.u01(id, fld, 2 * k + 1);
  const double r = detmath::sqrt(std::max(0.0, 1.0 - z * z));
  return {r * detmath::cos(phi), r * detmath::sin(phi), z};
}

CellId nearest_cell(const WorldGrid& g, const Vec3& p) {  // ties by the lower id
  CellId best = 0;
  double bd = -2;
  for (CellId i = 0; i < g.size(); ++i) {
    const double d = dot(g.centre(i), p);
    if (d > bd) bd = d, best = i;
  }
  return best;
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
  // Seeds are keyed points on the sphere (best of 8 candidates for spread), each in the cell nearest it, so the
  // same seed gives the same plates on any grid.
  std::vector<Vec3> seed_at;
  for (int p = 0; p < P; ++p) {
    Vec3 best{0, 0, 1};
    double best_gap = -2;
    for (std::uint32_t k = 0; k < 8; ++k) {
      const Vec3 c = random_point(rnd, static_cast<std::uint64_t>(p), field::PlatePoint, k);
      double nearest = 4;  // smallest squared chord to an earlier seed; larger is better spread
      for (const Vec3& s : seed_at) {
        const Vec3 d{c.x - s.x, c.y - s.y, c.z - s.z};
        nearest = std::min(nearest, dot(d, d));
      }
      if (nearest > best_gap) best_gap = nearest, best = c;
    }
    seed_at.push_back(best);
  }
  std::vector<CellId> seeds;
  for (const Vec3& s : seed_at) seeds.push_back(nearest_cell(g, s));
  const noise::Fbm cost_noise{2000, 3, 0.5, 0};  // all three octaves on every grid: plates must not depend on it
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
        // A step costs its length times a noise factor in [0.2, 1.8] (rule 2), so borders wander at any resolution.
        const double n = noise::fbm(prm.seed, field::NoiseCost, g.centre(nb), g.radius_km(), cost_noise);
        const double step = g.distance_km(cell, nb) * detmath::exp(kCostContrast * n) / plates[static_cast<std::size_t>(p)].weight;
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
      pi.base = (pi.continental ? 250.0 : -4000.0) + off;
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
        else s = {5000.0 * conv, BoundaryKind::IslandArc};
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

  // ---- Continental margin (rule 3b): signed distance in km to the contact between continental and oceanic crust,
  // positive inside a continent; the base height rises from the ocean floor over a slope to a shelf at the contact,
  // then over a coastal plain to the plate's base inland.
  std::vector<double> margin(N, 1e300);
  {
    using Item = std::pair<double, CellId>;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> q;
    for (CellId i = 0; i < N; ++i)
      for (const CellId j : g.neighbours(i))
        if (plates[f.plate[i]].continental != plates[f.plate[j]].continental) {
          margin[i] = 0.5 * g.distance_km(i, j);
          q.push({margin[i], i});
          break;
        }
    while (!q.empty()) {
      const auto [d, c] = q.top();
      q.pop();
      if (d > margin[c]) continue;
      for (const CellId j : g.neighbours(c)) {
        if (plates[f.plate[j]].continental != plates[f.plate[c]].continental) continue;
        const double nd = d + g.distance_km(c, j);
        if (nd < margin[j]) margin[j] = nd, q.push({nd, j});
      }
    }
  }
  auto margin_base = [&](CellId i) {
    const PlateInfo& pl = plates[f.plate[i]];
    const double d = std::min(margin[i], 1e6);
    if (pl.continental) return kShelfM + (pl.base - kShelfM) * std::min(1.0, d / kPlainKm);
    return pl.base + (kShelfM - pl.base) * std::max(0.0, 1.0 - d / kSlopeKm);
  };

  // ---- Hotspots (rule 3c): keyed points on the sphere; each leaves a chain of volcanic bumps trailing against its
  // plate's motion (Hawaii), full height on oceanic plates, less on continents.
  std::vector<double> hot(N, 0.0);
  {
    const int H = static_cast<int>(kHotspotsAtEarth * g.sphere_area_km2() / 510.1e6 + 0.5);
    const double sigma = std::max(kHotspotSigmaKm, 0.7 * spacing) / g.radius_km();  // at least one cell wide
    for (int h = 0; h < H; ++h) {
      const Vec3 c0 = random_point(rnd, static_cast<std::uint64_t>(h), field::Hotspot, 0);
      const CellId at = nearest_cell(g, c0);
      const PlateInfo& pl = plates[f.plate[at]];
      Vec3 v = cross(pl.omega, c0);
      const double vl = detmath::sqrt(dot(v, v));
      v = vl > 1e-12 ? Vec3{v.x / vl, v.y / vl, v.z / vl} : Vec3{0, 0, 0};
      const double scale = (pl.continental ? 0.15 : 1.0) * (0.6 + 0.8 * rnd.u01(static_cast<std::uint64_t>(h), field::Hotspot, 9));
      for (int k = 0; k < kHotspotChain; ++k) {
        const double back = k * kHotspotStepKm / g.radius_km();
        const Vec3 p = unit({c0.x - v.x * back, c0.y - v.y * back, c0.z - v.z * back});
        const double height = scale * kHotspotM * detmath::pow(0.7, k);
        for (CellId i = 0; i < N; ++i) {
          const Vec3 d{g.centre(i).x - p.x, g.centre(i).y - p.y, g.centre(i).z - p.z};
          const double r2 = dot(d, d) / (sigma * sigma);
          if (r2 < 9.0) hot[i] += height * detmath::exp(-r2);
        }
      }
    }
  }

  // ---- Elevation (rules 3 and 4): margin base + boundary + warped fractal noise + hotspots, then a little smoothing.
  const noise::Fbm warp_noise{4000, 3, 0.5, 2 * spacing};
  const noise::Fbm elev_noise{3000, 8, 0.55, 2 * spacing};
  const noise::Fbm shape_noise{9000, 2, 0.5, 2 * spacing};
  std::vector<double> elev(N);
  for (CellId i = 0; i < N; ++i) {
    const Vec3& c = g.centre(i);
    const Vec3 w{noise::fbm(prm.seed, field::NoiseWarp * 4 + 0, c, g.radius_km(), warp_noise),
                 noise::fbm(prm.seed, field::NoiseWarp * 4 + 1, c, g.radius_km(), warp_noise),
                 noise::fbm(prm.seed, field::NoiseWarp * 4 + 2, c, g.radius_km(), warp_noise)};
    const Vec3 q{c.x + kWarp * w.x, c.y + kWarp * w.y, c.z + kWarp * w.z};
    const double n = noise::fbm(prm.seed, field::NoiseElev, q, g.radius_km(), elev_noise);
    const double inland = plates[f.plate[i]].continental ? std::min(margin[i], 1e6) : 0.0;
    const double amp = kNoiseM * (kNoiseInland + (1.0 - kNoiseInland) * detmath::exp(-inland / kNoiseFadeKm));
    const double shape = noise::fbm(prm.seed, field::NoiseElev + 1, q, g.radius_km(), shape_noise);
    elev[i] = margin_base(i) + kShapeM * shape + prm.relief * (src[i].v + amp * n + hot[i]);
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
