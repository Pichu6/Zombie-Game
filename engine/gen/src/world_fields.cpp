// World fields (spec 31): plates -> elevation -> sea level -> temperature -> rain -> biome.
// Keyed draws only; arithmetic is + - * / sqrt and detmath, so a seed gives one world everywhere.
#include "zs/gen/world_fields.h"
#include "zs/gen/world_seasons.h"

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

// Spec 83 rule 3: aridity cuts (rain over potential evapotranspiration), calibrated on seeds 1-5 against Earth's
// biome shares (WMR-1); they started at the UNEP aridity classes (0.2, 0.5) and 0.9.
namespace {
constexpr double kAiDesert = 0.31, kAiGrass = 0.55, kAiSavanna = 0.87;  // recalibrated with the hydrology fix (2026-10-15)
constexpr double kAiDesertWarm = 0.36;
constexpr double kTropicC = 20;  // temperature recalibration: the annual mean from which a land is tropical (was 18)  // a warm land is dry at a higher aridity index (Koppen's dry threshold rises with T)
}  // namespace

double pet_mm(double t) { return 58.93 * std::clamp(t, 0.0, 30.0); }

Biome classify_biome(bool land, double elev, double t, double rain, double range_c) {
  if (!land) return Biome::Ocean;
  if (elev >= 2500) return Biome::Mountain;  // second amendment: high glaciers are part of the range, in its country
  if (range_c >= 0) {  // Koppen's EF and ET by the warmest month (AMS glossary; Kottek et al. 2006)
    const double warmest = t + range_c / 2;
    if (warmest <= 0) return Biome::Ice;
    if (warmest < 10) return Biome::Tundra;
  } else {
    if (t < -15) return Biome::Ice;
    if (t < -3) return Biome::Tundra;
  }
  const double pet = pet_mm(t);
  const double ai = pet > 0 ? rain / pet : 1e9;
  if (t < 6) return ai >= kAiDesert ? Biome::BorealForest : Biome::Tundra;
  if (ai < (t >= kTropicC ? kAiDesertWarm : kAiDesert)) return Biome::Desert;
  if (t < kTropicC) return ai < kAiGrass ? Biome::Grassland : Biome::TemperateForest;
  return ai < kAiSavanna ? Biome::Savanna : Biome::TropicalForest;
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
// Spec 89 (estimates, calibrated in As built): cratons and their size spread; island arcs as chains of islands.
constexpr int kCratons = 8;
constexpr double kCratonZipf = 1.3;        // second amendment: 1.3 (1.5 gave the largest landmass 62 % against Earth's 53.5)
constexpr int kMicroCratons = 12;          // micro-continents: Earth's 100,000-1,000,000 km2 landmasses are mostly
constexpr double kMicroShare = 0.04;       // continental fragments (Madagascar, Borneo, New Zealand, Britain), about 4 % of land (CC-1)
// Second amendment: Earth's land is lopsided (CC-3): 67 % in the north, reaching 72 N (Siberia, Canada), while beyond
// Antarctica the south's land ends near 55 S (0.2 % of land at 50-60 S). One hemisphere, keyed, is the landed one.
// Craton centres are drawn by Earth's land by 10-degree band without Antarctica (CC-3), from 60 S to 70 N, the landed
// hemisphere taking Earth's north.
constexpr double kCratonBands[13] = {0.2, 0.7, 2.8, 6.3, 6.4, 7.1, 6.8, 7.6, 10.2, 10.4, 10.7, 9.8, 9.0};
constexpr int kCratonTries = 32;             // candidate centres per craton; the one farthest from the others wins
constexpr double kPolarSlowFrom = 0.883, kPolarSlowTo = 0.951;  // landed hemisphere: crust spreads slower from 62 to
constexpr double kPolarSlow = 0.0;           // 72 degrees by up to e^1 per km ...
constexpr double kCapDeg = 68;               // ... and no land beyond about 68 degrees (Earth: 2.4 % of land at 70-80 N, mostly islands, CC-3)
constexpr double kOtherSlowFrom = 0.766, kOtherSlowTo = 0.848;  // the other: slower from 50 to 58 degrees ...
constexpr double kCapDegOther = 58;          // ... no land beyond about 58 (Tierra del Fuego, 55 S)
constexpr double kCapWanderDeg = 4;          // the limit wanders by this much
constexpr double kCratonMerge = 0.1;          // the chance two cratons that meet join (Africa and Eurasia); else a strait        // the k-th craton's share of continental crust falls as k^-1.5 (CC)
constexpr double kArcWavelengthKm = 2000;  // arc islands come in a few groups along the arc (Japan, the Philippines)
constexpr double kArcM = 3500;             // an arc's uplift at full convergence (was the subduction's 5,000)
constexpr double kSeaIceC = -13.0;         // annual mean air temperature below which the sea is ice-covered (CC)
constexpr double kIceSheetPolar = 0.5;     // a landmass at least half beyond 60 degrees ...
constexpr double kIceSheetC = -5.0;        // ... and colder than this on average carries an ice sheet ...
constexpr double kIceSheetCellC = -6.0;    // ... over its cells colder than this; its milder fringe stays tundra (Greenland's coast)
constexpr int kIceShelfRings = 3;         // the sea this many cells out from an ice sheet freezes ...
constexpr double kIceShelfC = -3.0;        // ... where its annual mean is below this (east Greenland's coast, estimate)
constexpr double kSeaIceGyreC = 12.0;       // the edge's large bends: the Antarctic edge lies at 55-65 S (estimate)
constexpr double kSeaIceNoiseC = 2.0;      // the ice edge's small wander (estimate)
constexpr double kSideC = 6.0;             // second amendment: an ocean's western side is colder and its eastern side
                                           // warmer at high latitude, by up to this each way (Nain, Labrador, -3 C at
                                           // 56 N against Bergen 7.6 C at 60 N; the Labrador Current and the Norwegian
                                           // Atlantic Current)
constexpr double kSideKm = 1500;           // the side effect fades over this distance from the coast
constexpr double kCurrentC = 3.0;          // high-latitude anomaly from currents, both ways (estimate; Norway is about 10 C
                                           // warmer than its latitude's mean, Labrador several colder)
constexpr double kInlandSeaKm2 = 2.0e6;    // an enclosed sea is filled as a low basin (Earth has none larger than the Caspian, CC)
constexpr double kBasinM = 5;              // how far above the sea a filled basin's floor stands

struct PlateInfo {
  bool continental = false;
  double base = 0, weight = 1, age = 0, offset = 0;
  Vec3 omega;  // rotation axis; velocity at point c is omega x c
};

struct Source {
  double v = 0;
  BoundaryKind k = BoundaryKind::None;
};

// Piecewise-linear rain by sine of latitude (spec 31 rule 7 calibration): wet equator, dry subtropics,
// wetter mid-latitudes, dry poles. Spec 83 rule 2 splits it into the profile without the subtropical dip
// (`wet_rain`) and the dip itself, which each place takes by the side of its continent.
double piecewise(const double (*knots)[2], std::size_t n, double z) {
  const double a = z < 0 ? -z : z;
  for (std::size_t i = 1; i < n; ++i)
    if (a <= knots[i][0]) {
      const double f = (a - knots[i - 1][0]) / (knots[i][0] - knots[i - 1][0]);
      return knots[i - 1][1] + f * (knots[i][1] - knots[i - 1][1]);
    }
  return knots[n - 1][1];
}
double lat_rain(double z) {
  static const double knots[][2] = {{0, 2600}, {0.15, 2100}, {0.35, 700}, {0.5, 350},
                                    {0.7, 800}, {0.85, 650}, {1.0, 200}};
  return piecewise(knots, 7, z);
}
double wet_rain(double z) {  // the 15-35 degree knots raised to the line from 0.15 to 0.7
  static const double knots[][2] = {{0, 2600}, {0.15, 2100}, {0.7, 800}, {0.85, 650}, {1.0, 200}};
  return piecewise(knots, 5, z);
}

// Spec 83 rule 2 (estimates, tuned in As built): how much of the subtropical dip a west and an east coast keep;
// the reach of the moisture carry in the tropics and elsewhere (hydrology fix 2: 6,000 km in the tropics, HYD-8, HYD-9);
// the onshore reach on a mid-latitude east side.
constexpr double kDipWest = 1.3, kDipEast = 0.3;
constexpr double kCarryTropicsKm = 6000, kCarryKm = 1800;
constexpr double kEastShoreKm = 1200, kEastShoreWeight = 0.7;
// Hydrology fix 2 (estimates, tuned to HYD-8): the rise per cell the air climbs without losing moisture, and the rise
// over which it loses all it can (half).
constexpr double kRiseFreeM = 300, kRiseLossM = 3000;
// Temperature recalibration (2026-10-17, tuned to SR-6): a continent's interior colder far north; the annual range.
constexpr double kInlandKm = 1000, kInteriorColdC = 4.0;
constexpr double kContBase = 0.45, kContInland = 0.85;
constexpr double kRangeBase = 2.0, kRangeLat = 45.0, kRangeExp = 1.5;
constexpr double kFloor = 0.2;  // the share of the profile a cell keeps however far the wind has carried it
constexpr double kSideWalkKm = 8000;

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
      pi.offset = off;
    }
  }

  // ---- Spec 89 rule 1: continental crust from cratons, apart from the plates (a plate carries both kinds). The k-th
  // craton takes a share of the continental area falling as k^-1.5 with a keyed spread; each grows from a keyed point
  // as a noisy flood until it holds its share, so continents are blobs of many sizes, and cratons that meet merge.
  std::vector<std::uint8_t>& crust = f.crust;
  // Second amendment: the landed hemisphere (keyed) and each cell's polar limit: 72 degrees on the landed side, 58 on
  // the other, wandering by 4 degrees over 2,500 km so the high coasts are not a parallel. No land lies beyond it.
  const double landed = rnd.u01(0, field::CratonSize + 2000) < 0.5 ? 1.0 : -1.0;
  std::vector<double> cap_sin(N);
  {
    const noise::Fbm cap_noise{2500, 2, 0.5, 0};
    for (CellId i = 0; i < N; ++i) {
      const bool home = g.centre(i).z * landed >= 0;
      const double deg = (home ? kCapDeg : kCapDegOther) +
                         kCapWanderDeg * noise::fbm(prm.seed, field::NoiseCraton + 3000, g.centre(i), g.radius_km(), cap_noise);
      cap_sin[i] = detmath::sin(deg * 3.141592653589793 / 180.0);
    }
  }
  crust.assign(N, 0);
  {
    const int big = prm.cratons > 0 ? prm.cratons : kCratons;
    const int C = big + kMicroCratons;  // second amendment: micro-continents after the cratons (Madagascar, New Zealand)
    const double total = std::clamp(prm.land_share * 1.35, 0.02, 0.95) * g.sphere_area_km2();
    std::vector<double> budget(static_cast<std::size_t>(C));
    double sum = 0;
    for (int k = 0; k < big; ++k) {
      budget[static_cast<std::size_t>(k)] =
          detmath::pow(k + 1.0, -kCratonZipf) * (0.7 + 0.6 * rnd.u01(static_cast<std::uint64_t>(k), field::CratonSize));
      sum += budget[static_cast<std::size_t>(k)];
    }
    for (int k = 0; k < big; ++k) budget[static_cast<std::size_t>(k)] *= (1.0 - kMicroShare) * total / sum;
    for (int k = big; k < C; ++k)
      budget[static_cast<std::size_t>(k)] =
          kMicroShare * total / kMicroCratons * (0.4 + 1.2 * rnd.u01(static_cast<std::uint64_t>(k), field::CratonSize));
    std::vector<Vec3> at;
    for (int k = 0; k < C; ++k) {
      Vec3 best{0, 0, 1};
      double best_gap = -2;
      for (std::uint32_t t = 0; t < kCratonTries; ++t) {
        Vec3 c;
        {
          double total_w = 0;
          for (const double w : kCratonBands) total_w += w;
          double u = rnd.u01(static_cast<std::uint64_t>(k), field::CratonPoint, 2 * t) * total_w;
          int band = 0;
          while (band < 12 && u > kCratonBands[band]) u -= kCratonBands[band++];
          const double lo = (-60.0 + 10.0 * band) * 3.141592653589793 / 180.0, hi = lo + 10.0 * 3.141592653589793 / 180.0;
          const double v = rnd.u01(static_cast<std::uint64_t>(k), field::CratonPoint + 2000, 2 * t);
          const double z = landed * (detmath::sin(lo) + v * (detmath::sin(hi) - detmath::sin(lo)));  // area-uniform in the band
          const double phi = 6.283185307179586 * rnd.u01(static_cast<std::uint64_t>(k), field::CratonPoint, 2 * t + 1);
          const double r = detmath::sqrt(std::max(0.0, 1.0 - z * z));
          c = {r * detmath::cos(phi), r * detmath::sin(phi), z};
        }
        double nearest = 4;
        for (const Vec3& s : at) {
          const Vec3 d{c.x - s.x, c.y - s.y, c.z - s.z};
          nearest = std::min(nearest, dot(d, d));
        }
        if (nearest > best_gap) best_gap = nearest, best = c;
      }
      at.push_back(best);
    }
    const noise::Fbm craton_noise{1500, 3, 0.5, 0};
    using Item = std::tuple<double, CellId, int>;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> q;
    std::vector<double> got(static_cast<std::size_t>(C), 0.0), cost(N, 1e300);
    std::vector<int> owner(N, -1);
    // Second amendment (the user: continents too clustered): two cratons join only by a keyed chance; otherwise a
    // strait of at least one cell stays between them.
    auto joins = [&](int a, int b) {
      const int lo = std::min(a, b), hi = std::max(a, b);
      return rnd.u01(static_cast<std::uint64_t>(lo * 64 + hi), field::CratonSize + 1000) < kCratonMerge;
    };
    for (int k = 0; k < C; ++k) {
      const CellId s = nearest_cell(g, at[static_cast<std::size_t>(k)]);
      cost[s] = 0;
      q.push({0.0, s, k});
    }
    while (!q.empty()) {
      const auto [c, cell, k] = q.top();
      q.pop();
      if (crust[cell] || got[static_cast<std::size_t>(k)] >= budget[static_cast<std::size_t>(k)]) continue;
      bool apart = false;  // a strait two cells wide, so the shelves do not close it
      for (const CellId nb : g.neighbours(cell)) {
        if (owner[nb] >= 0 && owner[nb] != k && !joins(owner[nb], k)) apart = true;
        for (const CellId n2 : g.neighbours(nb))
          if (owner[n2] >= 0 && owner[n2] != k && !joins(owner[n2], k)) apart = true;
      }
      if (apart) continue;
      crust[cell] = 1;
      owner[cell] = k;
      got[static_cast<std::size_t>(k)] += g.area_km2(cell);
      for (const CellId nb : g.neighbours(cell)) {
        const bool home = g.centre(nb).z * landed >= 0;
        if (crust[nb] || std::abs(g.centre(nb).z) > cap_sin[nb]) continue;  // no continent reaches a pole
        const double n = noise::fbm(prm.seed, field::NoiseCraton, g.centre(nb), g.radius_km(), craton_noise);
        // Spread is slow toward the poles, so continents reach polar seas only at their edges (the user's call).
        // Second amendment: up to 72 degrees, as Siberia's and Canada's tundra coasts.
        const double az = std::abs(g.centre(nb).z);
        const double polar = home ? std::clamp((az - kPolarSlowFrom) / (kPolarSlowTo - kPolarSlowFrom), 0.0, 1.0)
                                  : std::clamp((az - kOtherSlowFrom) / (kOtherSlowTo - kOtherSlowFrom), 0.0, 1.0);
        const double step = g.distance_km(cell, nb) * detmath::exp(kCostContrast * n + kPolarSlow * polar);
        if (c + step < cost[nb]) cost[nb] = c + step, q.push({c + step, nb, k});
      }
    }
  }

  // ---- Boundaries (rule 3): per-cell source from the strongest convergence with a neighbouring plate. Spec 89: the
  // kind follows the crust on each side, not the plate.
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
        const bool ci = crust[i] != 0, cj = crust[j] != 0;
        if (ci && cj) s = {7000.0 * conv, BoundaryKind::Collision};
        else if (ci) s = {5000.0 * conv, BoundaryKind::Subduction};
        else if (cj) s = {-6000.0 * conv, BoundaryKind::Trench};
        else if (A.age >= B.age) s = {-6000.0 * conv, BoundaryKind::Trench};
        else s = {kArcM * conv, BoundaryKind::IslandArc};
      } else {
        const double a = -conv;
        if (!crust[i] && !crust[j]) s = {1500.0 * a, BoundaryKind::Ridge};
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
  // Spread over a few hundred km: strongest effect wins, fading each step (rule 3). An effect stays on the plate that
  // made it (spec 83 As built): a trench spreading into the overriding continent fought its uplift cell by cell and
  // left a saw of one-cell strips of land and sea along every subduction coast.
  const int reach = std::max(1, static_cast<int>(350.0 / spacing + 0.5));
  const double decay = detmath::pow(0.1, 1.0 / static_cast<double>(reach));
  for (int it = 0; it < reach; ++it) {
    std::vector<Source> next = src;
    for (CellId i = 0; i < N; ++i)
      for (const CellId j : g.neighbours(i)) {
        if (f.plate[j] != f.plate[i] || crust[j] != crust[i]) continue;  // spec 89: and on its own crust
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
        if (crust[i] != crust[j]) {
          margin[i] = 0.5 * g.distance_km(i, j);
          q.push({margin[i], i});
          break;
        }
    while (!q.empty()) {
      const auto [d, c] = q.top();
      q.pop();
      if (d > margin[c]) continue;
      for (const CellId j : g.neighbours(c)) {
        if (crust[j] != crust[c]) continue;
        const double nd = d + g.distance_km(c, j);
        if (nd < margin[j]) margin[j] = nd, q.push({nd, j});
      }
    }
  }
  auto margin_base = [&](CellId i) {
    const PlateInfo& pl = plates[f.plate[i]];
    const double d = std::min(margin[i], 1e6);
    // Spec 89: the crust's base (continental 0 m, oceanic -5,200 m, calibrated on seeds 1-5 to Earth's hypsometry and
    // ocean depth, MG-24, MG-25), the plate's offset.
    const double base = (crust[i] ? 0.0 : -5200.0) + pl.offset;
    if (crust[i]) return kShelfM + (base - kShelfM) * std::min(1.0, d / kPlainKm);
    return base + (kShelfM - base) * std::max(0.0, 1.0 - d / kSlopeKm);
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
      const double scale = (crust[at] ? 0.15 : 1.0) * (0.6 + 0.8 * rnd.u01(static_cast<std::uint64_t>(h), field::Hotspot, 9));
      for (int k = 0; k < kHotspotChain; ++k) {
        const double back = k * kHotspotStepKm / g.radius_km();
        const Vec3 p = unit({c0.x - v.x * back, c0.y - v.y * back, c0.z - v.z * back});
        const double height = scale * kHotspotM * detmath::pow(0.55, k);  // spec 89: older volcanoes sink fast (Hawaii's chain)
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
  const noise::Fbm arc_noise{kArcWavelengthKm, 2, 0.5, 0};
  std::vector<double> elev(N);
  for (CellId i = 0; i < N; ++i) {
    const Vec3& c = g.centre(i);
    const Vec3 w{noise::fbm(prm.seed, field::NoiseWarp * 4 + 0, c, g.radius_km(), warp_noise),
                 noise::fbm(prm.seed, field::NoiseWarp * 4 + 1, c, g.radius_km(), warp_noise),
                 noise::fbm(prm.seed, field::NoiseWarp * 4 + 2, c, g.radius_km(), warp_noise)};
    const Vec3 q{c.x + kWarp * w.x, c.y + kWarp * w.y, c.z + kWarp * w.z};
    const double n = noise::fbm(prm.seed, field::NoiseElev, q, g.radius_km(), elev_noise);
    const double inland = crust[i] ? std::min(margin[i], 1e6) : 0.0;
    const double amp = kNoiseM * (kNoiseInland + (1.0 - kNoiseInland) * detmath::exp(-inland / kNoiseFadeKm));
    const double shape = noise::fbm(prm.seed, field::NoiseElev + 1, q, g.radius_km(), shape_noise);
    double boundary = src[i].v;
    if (src[i].k == BoundaryKind::IslandArc) {  // spec 89 rule 3: islands that come and go along the arc, not a wall
      const double m = noise::fbm(prm.seed, field::NoiseArc, c, g.radius_km(), arc_noise);
      boundary *= std::clamp((m - 0.3) / 0.25, 0.0, 1.0);  // amendment: a few island groups, not a dotted line
    }
    elev[i] = margin_base(i) + kShapeM * shape + prm.relief * (boundary + amp * n + hot[i]);
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

  // Second amendment: beyond the polar limit the floor stays deep, whatever the plates raised there.
  for (CellId i = 0; i < N; ++i) {
    const double az = std::abs(g.centre(i).z);
    if (az > cap_sin[i]) elev[i] = std::min(elev[i], -1500.0 - 20000.0 * (az - cap_sin[i]));
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
    // Spec 89 rule 2: a sea shut inside the land and smaller than the Caspian (CC) is not sea but a low basin: it is
    // raised just above the sea and the level is found again, so the land share stays exact.
    for (int pass = 0; pass < 4; ++pass) {
      std::sort(order.begin(), order.end(), [&](CellId a, CellId b) {
        return elev[a] != elev[b] ? elev[a] > elev[b] : a < b;
      });
      double cum = 0;
      std::size_t m = 0;
      while (m + 1 < N && std::abs(cum + g.area_km2(order[m]) - target) < std::abs(cum - target)) cum += g.area_km2(order[m++]);
      f.sea_level_raw_m = elev[order[m]];
      std::vector<int> comp(N, -1);
      std::vector<double> area;
      std::vector<std::vector<CellId>> cells;
      for (CellId i = 0; i < N; ++i) {
        if (elev[i] > f.sea_level_raw_m || comp[i] >= 0) continue;
        const int k = static_cast<int>(area.size());
        area.push_back(0);
        cells.emplace_back();
        std::vector<CellId> stack{i};
        comp[i] = k;
        while (!stack.empty()) {
          const CellId x = stack.back();
          stack.pop_back();
          area.back() += g.area_km2(x);
          cells.back().push_back(x);
          for (const CellId nb : g.neighbours(x))
            if (elev[nb] <= f.sea_level_raw_m && comp[nb] < 0) comp[nb] = k, stack.push_back(nb);
        }
      }
      std::size_t ocean = 0;
      for (std::size_t k = 1; k < area.size(); ++k)
        if (area[k] > area[ocean]) ocean = k;
      bool raised = false;
      for (std::size_t k = 0; k < area.size(); ++k) {
        if (k == ocean || area[k] >= kInlandSeaKm2) continue;
        double low = elev[cells[k].front()];
        for (const CellId x : cells[k]) low = std::min(low, elev[x]);
        // keep the basin's own relief, lifted so its floor stands just above the sea: its water gathers at the floor
        for (const CellId x : cells[k])  // a tiny keyed tie-break: two basin floors must not stand at the same height
          elev[x] = f.sea_level_raw_m + kBasinM + 0.5 * (elev[x] - low) + 1e-6 * static_cast<double>(x % 9973);
        raised = true;
      }
      if (!raised) break;
    }
    // The level once more over the filled basins, so the land share is exact to a cell (rule 5).
    std::sort(order.begin(), order.end(), [&](CellId a, CellId b) { return elev[a] != elev[b] ? elev[a] > elev[b] : a < b; });
    {
      double cum = 0;
      std::size_t m = 0;
      while (m + 1 < N && std::abs(cum + g.area_km2(order[m]) - target) < std::abs(cum - target)) cum += g.area_km2(order[m++]);
      f.sea_level_raw_m = elev[order[m]];
    }
    for (CellId i = 0; i < N; ++i) f.elevation_m[i] = std::clamp(elev[i] - f.sea_level_raw_m, -11000.0, 9000.0);
  }
  std::vector<CellId> sea;
  for (CellId i = 0; i < N; ++i)
    if (!f.land(i)) sea.push_back(i);
  f.sea_cells = g.graph_distance(sea);

  // ---- Temperature (rule 6): a latitude profile shifted to the global mean, lapse with height.
  // The recalibration's land term by sine of latitude (C), and the annual range's terms.
  auto land_warmth = [](double a) {
    static const double knots[][2] = {{0, 2.0}, {0.17, 5.5}, {0.34, 7.0}, {0.5, 3.5}, {0.64, 2.0}, {0.77, 1.0}, {0.87, 0.0}, {1.0, 0.0}};
    for (std::size_t k = 1; k < sizeof(knots) / sizeof(knots[0]); ++k)
      if (a <= knots[k][0]) return knots[k - 1][1] + (a - knots[k - 1][0]) / (knots[k][0] - knots[k - 1][0]) * (knots[k][1] - knots[k - 1][1]);
    return 0.0;
  };
  // Zonal-mean sea-level temperature by sine of latitude (Earth: 26 at the equator, 0 at 60 degrees, -25 at the pole).
  auto profile = [](double z) {
    static const double knots[][2] = {{0, 26}, {0.5, 20}, {0.707, 8}, {0.866, -1}, {0.966, -15}, {1.0, -25}};
    const double a = z < 0 ? -z : z;
    for (std::size_t i = 1; i < sizeof(knots) / sizeof(knots[0]); ++i)
      if (a <= knots[i][0])
        return knots[i - 1][1] + (a - knots[i - 1][0]) / (knots[i][0] - knots[i - 1][0]) * (knots[i][1] - knots[i - 1][1]);
    return knots[5][1];
  };
  // ---- Spec 83 rule 1: which side of its continent each land cell is on, by walking along the parallel.
  f.west_km.assign(N, 0);
  f.east_km.assign(N, 0);
  for (CellId i = 0; i < N; ++i) {
    if (!f.land(i)) continue;
    for (const double dir : {-1.0, 1.0}) {
      CellId c = i;
      double km = 0;
      while (f.land(c) && km < kSideWalkKm) {
        const Vec3& p = g.centre(c);
        const double el = detmath::sqrt(p.x * p.x + p.y * p.y);
        if (el < 1e-6) {  // at a pole east is undefined: count the cell as mid-continent
          km = kSideWalkKm;
          break;
        }
        const Vec3 east{dir * -p.y / el, dir * p.x / el, 0};
        CellId best = c;
        double bd = -2;
        for (const CellId nb : g.neighbours(c)) {
          const Vec3& q = g.centre(nb);
          const double d = dot(unit(Vec3{q.x - p.x, q.y - p.y, q.z - p.z}), east);
          if (d > bd) bd = d, best = nb;
        }
        km += g.distance_km(c, best);
        c = best;
      }
      (dir < 0 ? f.west_km : f.east_km)[i] = std::min(km, kSideWalkKm);
    }
  }

  // Second amendment: the same walk over the sea, for how far a sea cell lies from the land to its west and east.
  std::vector<double> sea_west(N, kSideWalkKm), sea_east(N, kSideWalkKm);
  for (CellId i = 0; i < N; ++i) {
    if (f.land(i)) continue;
    for (const double dir : {-1.0, 1.0}) {
      CellId c = i;
      double km = 0;
      while (!f.land(c) && km < kSideWalkKm) {
        const Vec3& p = g.centre(c);
        const double el = detmath::sqrt(p.x * p.x + p.y * p.y);
        if (el < 1e-9) {
          km = kSideWalkKm;
          break;
        }
        const Vec3 east{dir * -p.y / el, dir * p.x / el, 0};
        CellId best = c;
        double bd = -2;
        for (const CellId nb : g.neighbours(c)) {
          const Vec3& q = g.centre(nb);
          const double d = dot(unit(Vec3{q.x - p.x, q.y - p.y, q.z - p.z}), east);
          if (d > bd) bd = d, best = nb;
        }
        km += g.distance_km(c, best);
        c = best;
      }
      (dir < 0 ? sea_west : sea_east)[i] = std::min(km, kSideWalkKm);
    }
  }
  const noise::Fbm current_noise{5000, 2, 0.5, 0};  // spec 89 amendment
  // The recalibration's land term, before the shift, so the planet's sea-level mean stays the setting's.
  std::vector<double> land_term(N, 0.0);
  for (CellId i = 0; i < N; ++i) {
    if (!f.land(i)) continue;
    const double az = g.sin_lat(i) < 0 ? -g.sin_lat(i) : g.sin_lat(i);
    const double inland = 1.0 - detmath::exp(-static_cast<double>(f.sea_cells[i]) * spacing / kInlandKm);
    land_term[i] = land_warmth(az) - kInteriorColdC * inland * std::clamp((az - 0.71) / 0.2, 0.0, 1.0);
  }
  double mean = 0;
  for (CellId i = 0; i < N; ++i) mean += (profile(g.sin_lat(i)) + land_term[i]) * g.area_km2(i);
  const double shift = prm.global_temp_c - mean / g.sphere_area_km2();
  for (CellId i = 0; i < N; ++i) {
    const double z = g.sin_lat(i), az = z < 0 ? -z : z;
    const double h = std::max(0.0, f.elevation_m[i]);
    // Spec 89 amendment: a large-scale anomaly at high latitudes, as ocean currents warm or chill a whole coast (the
    // Gulf Stream's Norway, the Labrador Current's Canada), applied to land and sea alike so the ice and the land beside
    // it agree.
    const double polar = std::clamp((az - 0.57) / 0.3, 0.0, 1.0);  // from about 35 degrees, full beyond about 62
    // Second amendment: the side of the ocean. Sea with land close to its west is an ocean's western side (cold
    // currents from the pole: Labrador, Oyashio); sea with land close to its east is its eastern side (warm currents
    // from the tropics: Norway, Alaska's panhandle). A coast takes its sea's side: a continent's east coast is cold,
    // its west coast mild.
    const double to_w = f.land(i) ? f.east_km[i] : sea_west[i];  // land: its sea lies east when the east coast is near
    const double to_e = f.land(i) ? f.west_km[i] : sea_east[i];
    const double side = detmath::exp(-to_e / kSideKm) - detmath::exp(-to_w / kSideKm);
    const double anomaly = polar * (kSideC * side +
                                    kCurrentC * noise::fbm(prm.seed, field::NoiseIce + 1000, g.centre(i), g.radius_km(), current_noise));
    // Temperature recalibration (2026-10-17, research snow-and-rivers SR-6): land is warmer than the sea at its latitude
    // in the tropics and subtropics (dry, clear skies: the Sahara, India) and a continent's interior colder far north;
    // the land term is tuned so the land's mean by band meets WorldClim's.
    const double inland_km = static_cast<double>(f.sea_cells[i]) * spacing;
    const double inland = 1.0 - detmath::exp(-inland_km / kInlandKm);
    f.temp_mean_c[i] = profile(z) + shift + anomaly + land_term[i] - 6.5 * h / 1000.0;
    const double continentality = f.land(i) ? kContBase + kContInland * inland : 0.35;
    f.temp_range_c[i] = (kRangeBase + kRangeLat * detmath::pow(az + 1e-9, kRangeExp)) * continentality;
  }

  // ---- Spec 89 rule 4: sea ice where the sea's annual mean is below the freezing of sea water.
  f.sea_ice.assign(N, 0);
  {
    const noise::Fbm ice_noise{1500, 2, 0.5, 0};
    const noise::Fbm ice_gyre{3000, 2, 0.5, 0};  // second amendment: gyres bend the edge (the Weddell and Ross seas)
    for (CellId i = 0; i < N; ++i)
      f.sea_ice[i] = !f.land(i) &&
                     f.temp_mean_c[i] < kSeaIceC + kSeaIceNoiseC * noise::fbm(prm.seed, field::NoiseIce, g.centre(i), g.radius_km(), ice_noise) +
                                            kSeaIceGyreC * noise::fbm(prm.seed, field::NoiseIce + 2000, g.centre(i), g.radius_km(), ice_gyre);
  }

  // ---- Rain (rule 7; spec 83 rule 2): the profile without its subtropical dip, the dip taken by side, times
  // moisture carried by the wind off the sea, with rain on rising ground and a shadow behind it. Gauss-Seidel in
  // id order, fixed pass count.
  std::vector<double> M(N, 1.0);
  std::vector<double> retain(N, 0.0);
  for (CellId i = 0; i < N; ++i) {
    const double az = g.sin_lat(i) < 0 ? -g.sin_lat(i) : g.sin_lat(i);
    const double t = std::clamp((az - 0.26) / 0.08, 0.0, 1.0);  // tropics below 15 degrees, blend to 20
    retain[i] = detmath::exp(-spacing / (kCarryTropicsKm + t * (kCarryKm - kCarryTropicsKm)));
  }
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
      // Hydrology fix 2 (2026-10-16): only a real climb wrings the air out (a range's windward side, not every hill of
      // the coarse terrain), so the moisture reaches a lowland basin's interior (HYD-9).
      M[i] = up_m * retain[i] * (1.0 - std::min(0.5, std::max(0.0, rise[i] - kRiseFreeM) / kRiseLossM));
    }
  }
  for (CellId i = 0; i < N; ++i) {
    const double z = g.sin_lat(i), az = z < 0 ? -z : z;
    double m = M[i];
    double profile = lat_rain(z);
    if (f.land(i)) {
      const double e = east_index(f, i);
      const double dip = wet_rain(z) - lat_rain(z);
      profile = std::max(0.0, wet_rain(z) - dip * (kDipWest + (kDipEast - kDipWest) * e));
      if (az >= 0.5 && az < 0.87)  // mid-latitude east sides: onshore summer winds off the eastern sea
        m = std::max(m, kEastShoreWeight * detmath::exp(-f.east_km[i] / kEastShoreKm));
    }
    const double base = profile * (kFloor + (1 - kFloor) * m);
    const double oro = f.land(i) ? 900.0 * (rise[i] / 1000.0) * M[i] : 0.0;
    f.rain_mm[i] = std::clamp((base + oro) * prm.moisture, 0.0, 4000.0);
  }

  // ---- Biome (rule 8).
  for (CellId i = 0; i < N; ++i)
    f.biome[i] = classify_biome(f.land(i), f.elevation_m[i], f.temp_mean_c[i], f.rain_mm[i], f.temp_range_c[i]);
  // Spec 89 amendment: a landmass mostly beyond 60 degrees and cold as a whole carries an ice sheet (Antarctica,
  // Greenland): its cold cells are ice, so no river, lake or settled country on them.
  {
    std::vector<int> comp(N, -1);
    for (CellId s = 0; s < N; ++s) {
      if (!f.land(s) || comp[s] >= 0) continue;
      std::vector<CellId> cells{s}, stack{s};
      comp[s] = static_cast<int>(s);
      while (!stack.empty()) {
        const CellId x = stack.back();
        stack.pop_back();
        for (const CellId nb : g.neighbours(x))
          if (f.land(nb) && comp[nb] < 0) comp[nb] = static_cast<int>(s), stack.push_back(nb), cells.push_back(nb);
      }
      double area = 0, polar = 0, temp = 0;
      for (const CellId x : cells) {
        const double a = g.area_km2(x), az = g.sin_lat(x) < 0 ? -g.sin_lat(x) : g.sin_lat(x);
        area += a;
        temp += a * f.temp_mean_c[x];
        if (az >= 0.866) polar += a;  // beyond 60 degrees
      }
      if (polar < kIceSheetPolar * area || temp / area >= kIceSheetC) {
        continue;
      }
      for (const CellId x : cells)
        if (f.temp_mean_c[x] < kIceSheetCellC) f.biome[x] = Biome::Ice;
    }
    // The sea along an ice sheet freezes too (ice shelves, fast ice), so the cap reads as one: land ice and sea ice
    // meet, as around Antarctica and east Greenland, instead of a landmass lying on the ice band.
    std::vector<int> dist(N, -1);
    std::vector<CellId> front;
    for (CellId i = 0; i < N; ++i)
      if (f.biome[i] == Biome::Ice) dist[i] = 0, front.push_back(i);
    for (int ring = 1; ring <= kIceShelfRings && !front.empty(); ++ring) {
      std::vector<CellId> next;
      for (const CellId x : front)
        for (const CellId nb : g.neighbours(x))
          if (!f.land(nb) && dist[nb] < 0 && f.temp_mean_c[nb] < kIceShelfC) {
            dist[nb] = ring, f.sea_ice[nb] = 1, next.push_back(nb);
          }
      front.swap(next);
    }
  }
  fill_rain_seasons(g, f);  // spec 100
  fill_winds(g, f);         // spec 101
  return f;
}

}  // namespace zs::gen
