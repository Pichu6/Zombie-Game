#include "planet_raster.h"

#include "zs/gen/world_seasons.h"

#include "toml.hpp"

#include <cstdio>
#include <map>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>

#include "zs/gen/keyed.h"

namespace zs::atlas {

namespace {

using RGB = std::array<double, 3>;

constexpr double kPi = 3.14159265358979323846;

// Spec 83 rule 5: an atlas palette for the biomes (ocean unused: the sea is shaded by depth).
const RGB kBiome[gen::biome_count] = {
    {30, 70, 140},   {244, 247, 250}, {186, 192, 168}, {78, 112, 82},  {72, 136, 74},
    {196, 212, 136}, {238, 220, 172}, {172, 178, 86},  {34, 112, 56},  {158, 148, 132}};
const RGB kLakeOpen{70, 130, 200}, kLakeSalt{160, 205, 220}, kLakeDry{216, 206, 186};
const RGB kRiver{40, 95, 195};
const RGB kForest{38, 108, 50}, kArable{232, 200, 64}, kPasture{160, 202, 112}, kBare{226, 216, 192};
const RGB kDeposit[gen::resource_count] = {{20, 20, 20}, {96, 56, 24}, {240, 140, 30}, {200, 40, 40}, {20, 150, 150}, {230, 110, 170}};

RGB mix(const RGB& a, const RGB& b, double t) { return {a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t, a[2] + (b[2] - a[2]) * t}; }

// A colour ramp through evenly spaced stops, t in [0, 1].
RGB ramp(const std::vector<RGB>& stops, double t) {
  t = std::clamp(t, 0.0, 1.0) * (static_cast<double>(stops.size()) - 1);
  const std::size_t i = std::min(stops.size() - 2, static_cast<std::size_t>(t));
  return mix(stops[i], stops[i + 1], t - static_cast<double>(i));
}

// Value noise on a 3D lattice, smoothstep-interpolated, in [-1, 1]; only for the picture (spec 83 rule 5: the map is
// drawn with detail below the cell, the world itself is not changed).
double lattice(std::int64_t x, std::int64_t y, std::int64_t z, std::uint64_t salt) {
  const std::uint64_t h = gen::mix64(gen::mix64(gen::mix64(salt, static_cast<std::uint64_t>(x)), static_cast<std::uint64_t>(y)),
                                     static_cast<std::uint64_t>(z));
  return static_cast<double>(h >> 11) / static_cast<double>(1ull << 53) * 2.0 - 1.0;
}
double value_noise(double x, double y, double z, std::uint64_t salt) {
  const double fx = std::floor(x), fy = std::floor(y), fz = std::floor(z);
  const auto ix = static_cast<std::int64_t>(fx), iy = static_cast<std::int64_t>(fy), iz = static_cast<std::int64_t>(fz);
  auto s = [](double t) { return t * t * (3 - 2 * t); };
  const double tx = s(x - fx), ty = s(y - fy), tz = s(z - fz);
  double v[2][2][2];
  for (int a = 0; a < 2; ++a)
    for (int b = 0; b < 2; ++b)
      for (int c = 0; c < 2; ++c) v[a][b][c] = lattice(ix + a, iy + b, iz + c, salt);
  auto l = [](double p, double q, double t) { return p + (q - p) * t; };
  return l(l(l(v[0][0][0], v[1][0][0], tx), l(v[0][1][0], v[1][1][0], tx), ty),
           l(l(v[0][0][1], v[1][0][1], tx), l(v[0][1][1], v[1][1][1], tx), ty), tz);
}
double fbm(const gen::Vec3& p, double freq, int octaves, std::uint64_t salt) {
  double sum = 0, amp = 1, norm = 0;
  for (int o = 0; o < octaves; ++o) {
    sum += amp * value_noise(p.x * freq, p.y * freq, p.z * freq, salt + static_cast<std::uint64_t>(o) * 7919);
    norm += amp;
    amp *= 0.5;
    freq *= 2.03;
  }
  return sum / norm;
}

gen::Vec3 normalize(gen::Vec3 v) {
  const double l = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  return {v.x / l, v.y / l, v.z / l};
}

// The picture's warp: a pixel looks up the world at a point moved by noise of about a cell, so cell edges become
// natural lines (coasts, biome edges, lakes) instead of hexagons and saw teeth. Returns the moved point.
struct Warp {
  double amp, freq;
  gen::Vec3 apply(const gen::Vec3& p) const {
    return normalize({p.x + amp * fbm(p, freq, 4, 101), p.y + amp * fbm(p, freq, 4, 202), p.z + amp * fbm(p, freq, 4, 303)});
  }
  gen::Vec3 undo(const gen::Vec3& c) const {  // where a world point shows in the picture, to first order
    return normalize({c.x - amp * fbm(c, freq, 4, 101), c.y - amp * fbm(c, freq, 4, 202), c.z - amp * fbm(c, freq, 4, 303)});
  }
};

struct Canvas {
  int w, h;
  std::vector<RGB> px;
  RGB& at(int x, int y) { return px[static_cast<std::size_t>(y) * w + x]; }
  void disc(double cx, double cy, double r, const RGB& c, double alpha = 1.0) {
    const int x0 = static_cast<int>(std::floor(cx - r)), x1 = static_cast<int>(std::ceil(cx + r));
    const int y0 = std::max(0, static_cast<int>(std::floor(cy - r))), y1 = std::min(h - 1, static_cast<int>(std::ceil(cy + r)));
    for (int y = y0; y <= y1; ++y)
      for (int x = x0; x <= x1; ++x) {
        const double dx = x + 0.5 - cx, dy = y + 0.5 - cy, d = std::sqrt(dx * dx + dy * dy);
        if (d > r + 0.5) continue;
        const double edge = std::clamp(r + 0.5 - d, 0.0, 1.0);  // anti-aliased rim
        RGB& p = at((x % w + w) % w, y);
        p = mix(p, c, alpha * edge);
      }
  }
  void line(double x0, double y0, double x1, double y1, double width, const RGB& c, double alpha = 1.0) {
    const double len = std::max(std::abs(x1 - x0), std::abs(y1 - y0));
    const int steps = std::max(1, static_cast<int>(std::ceil(len * 2)));
    for (int s = 0; s <= steps; ++s) {
      const double t = static_cast<double>(s) / steps;
      disc(x0 + (x1 - x0) * t, y0 + (y1 - y0) * t, width / 2, c, alpha);
    }
  }
};

// Equal Earth (Savric, Patterson and Jenny 2018): forward and inverse on radians, fitted to the picture with a margin.
constexpr double kA1 = 1.340264, kA2 = -0.081106, kA3 = 0.000893, kA4 = 0.003796, kM = 0.8660254037844386;  // sqrt(3)/2
constexpr double kEeX = 2.7066299836891155, kEeY = 1.3173627591574133;  // the projection's half-width and half-height
void ee_forward(double lat, double lon, double& x, double& y) {
  const double t = std::asin(kM * std::sin(lat)), t2 = t * t, t6 = t2 * t2 * t2;
  x = lon * std::cos(t) / (kM * (kA1 + 3 * kA2 * t2 + t6 * (7 * kA3 + 9 * kA4 * t2)));
  y = t * (kA1 + kA2 * t2 + t6 * (kA3 + kA4 * t2));
}
bool ee_inverse(double x, double y, double& lat, double& lon) {
  if (std::abs(y) > kEeY) return false;
  double t = y / kA1;
  for (int k = 0; k < 12; ++k) {  // Newton on y(t)
    const double t2 = t * t, t6 = t2 * t2 * t2;
    const double f = t * (kA1 + kA2 * t2 + t6 * (kA3 + kA4 * t2)) - y;
    const double d = kA1 + 3 * kA2 * t2 + t6 * (7 * kA3 + 9 * kA4 * t2);
    t -= f / d;
  }
  const double t2 = t * t, t6 = t2 * t2 * t2;
  lon = kM * x * (kA1 + 3 * kA2 * t2 + t6 * (7 * kA3 + 9 * kA4 * t2)) / std::cos(t);
  if (std::abs(lon) > kPi) return false;
  lat = std::asin(std::clamp(std::sin(t) / kM, -1.0, 1.0));
  return true;
}
// The picture's scale for Equal Earth: fit the outline with a 2 % margin, centred.
void ee_frame(int W, int H, double& s, double& cx, double& cy) {
  s = 0.98 * std::min(W / (2 * kEeX), H / (2 * kEeY));
  cx = W / 2.0, cy = H / 2.0;
}

// A pixel's latitude and longitude (radians); false outside the projection's outline.
bool pixel_latlon(Projection proj, int W, int H, double px, double py, double& lat, double& lon) {
  if (proj == Projection::Equirectangular) {
    lat = (90.0 - 180.0 * py / H) * kPi / 180.0;
    lon = (-180.0 + 360.0 * px / W) * kPi / 180.0;
    return true;
  }
  double s, cx, cy;
  ee_frame(W, H, s, cx, cy);
  return ee_inverse((px - cx) / s, (cy - py) / s, lat, lon);
}

Projection g_projection = Projection::Equirectangular;  // the projection of the picture being drawn (one at a time)

void to_pixel(const gen::Vec3& v, int W, int H, double& x, double& y) {
  const double lat = std::asin(std::clamp(v.z, -1.0, 1.0));
  const double lon = std::atan2(v.y, v.x);
  if (g_projection == Projection::Equirectangular) {
    x = (lon * 180.0 / kPi + 180.0) / 360.0 * W;
    y = (90.0 - lat * 180.0 / kPi) / 180.0 * H;
    return;
  }
  double s, cx, cy, ex, ey;
  ee_frame(W, H, s, cx, cy);
  ee_forward(lat, lon, ex, ey);
  x = cx + ex * s;
  y = cy - ey * s;
}

// Köppen-like main group from the fields (spec 83 rule 5's climate layer): A tropical, B arid (BW desert, BS steppe),
// C temperate, D continental, E polar; the coldest and warmest months from the mean and the range.
// Spec 100: Koppen-Geiger classes with their rain seasons, in the colours of the usual Koppen maps (Beck et al.'s).
RGB koppen_colour(const std::string& k) {
  static const std::map<std::string, RGB> c = {
      {"Af", {0, 0, 255}},     {"Am", {0, 120, 255}},   {"Aw", {70, 170, 250}},  {"BW", {255, 0, 0}},     {"BS", {245, 165, 0}},
      {"Cs", {255, 255, 0}},   {"Cw", {150, 255, 150}}, {"Cf", {100, 200, 50}},  {"Ds", {255, 0, 255}},   {"Dw", {170, 175, 255}},
      {"Df", {0, 220, 220}},   {"ET", {178, 178, 178}}, {"EF", {102, 102, 102}}};
  const auto it = c.find(k);
  return it == c.end() ? RGB{0, 0, 0} : it->second;
}

const RGB kBoundary[7] = {{0, 0, 0}, {200, 30, 30}, {240, 130, 30}, {40, 40, 80}, {150, 60, 170}, {250, 220, 40}, {40, 200, 220}};

}  // namespace

const char* layer_name(MapLayer l) {
  static const char* names[kMapLayers] = {"physical", "political", "topographic", "temperature", "rain",   "climate",
                                          "wind",     "geology",   "water",       "cover",       "resources", "people",
                                          "alliances", "industry", "energy"};
  return names[static_cast<int>(l)];
}

bool parse_layer(const std::string& name, MapLayer& out) {
  for (int k = 0; k < kMapLayers; ++k)
    if (name == layer_name(static_cast<MapLayer>(k))) return out = static_cast<MapLayer>(k), true;
  return false;
}

std::string layer_legend(MapLayer l) {
  switch (l) {
    case MapLayer::Physical:
      return "physical: biome colour (ice white, tundra grey-green, boreal dark green, temperate forest green, grassland pale "
             "green, desert sand, savanna yellow-olive, tropical forest deep green, mountain grey-brown) shaded by relief from the "
             "north-west; sea ice pale blue-white; open lakes blue, terminal (salt) lakes pale blue, dry basins pale beige; rivers as blue curves, wider "
             "with discharge; borders thin grey";
    case MapLayer::Political: return "political: a colour per country (a custom country's own); rivers and lakes as in physical; borders dark";
    case MapLayer::Alliances: return "alliances (spec 91): a colour per alliance, countries in none grey; country borders thin";
    case MapLayer::Energy:
      return "energy (spec 94): physical faded; high-voltage lines dark (between countries dashed red), power plants by fuel sized "
             "by MW: coal black, gas orange, oil and other brown, hydro blue, nuclear yellow ringed, wind white, solar gold";
    case MapLayer::Industry:
      return "industry (spec 93): physical faded; plants at their towns, a dot by kind sized by how many: refineries brown, steel "
             "red, alumina and aluminium pink, copper teal, cement grey, flour and sugar mills yellow, meat plants purple, sawmills "
             "green, fish plants cyan";
    case MapLayer::Topographic:
      return "topographic: height tint (green lowland, yellow, brown, grey, white above 4,000 m), contours every 500 m, sea "
             "depth from light to dark blue, the shelf (above -200 m) lightest";
    case MapLayer::Temperature: return "temperature: the annual mean, one colour every 10 C: -30 deep blue, -20 blue, -10 pale blue, 0 white, 10 pale yellow, 20 orange, 30 dark red (sea a little greyer)";
    case MapLayer::Rain: return "rain: the annual total on a log scale, 50 mm beige to 4,000 mm dark blue";
    case MapLayer::Climate:
      return "climate (Koppen-Geiger from the monthly normals, spec 100): rainforest Af blue, monsoon Am mid blue, savanna Aw "
             "light blue (dry winter); desert BW red, steppe BS orange; temperate with dry summer Cs yellow (Mediterranean), dry "
             "winter Cw pale green, no dry season Cf green; cold with dry summer Ds magenta, dry winter Dw lavender, no dry season "
             "Df cyan; tundra ET grey, ice EF dark grey";
    case MapLayer::Wind:
      return "wind: the prevailing wind the rain rule uses (trade easterlies under 30 degrees, westerlies 30-60, polar "
             "easterlies beyond) as arrows over the faded physical map; speeds are not modelled";
    case MapLayer::Geology:
      return "geology: a shade per plate (continental warm, oceanic cool); boundaries: collision red, subduction orange, "
             "trench dark blue, island arc purple, ridge yellow, rift cyan";
    case MapLayer::Water:
      return "water: land shaded by runoff, closed (endorheic) basins hatched brown, rivers by discharge, lakes open blue, "
             "salt pale blue, dry beige";
    case MapLayer::Cover:
      return "cover: the land in use (spec 88): working forest dark green, cropland yellow, pasture light green, the rest beige, each by its share of the cell; "
             "water as in physical";
    case MapLayer::Resources:
      return "resources: physical faded; deposits of three times the median or more, as discs sized by amount (solid when worked, faint when not): coal black, oil brown, gas orange, iron red, copper "
             "teal, bauxite pink; fishing grounds shaded cyan by catch";
    case MapLayer::People:
      return "people: density from pale yellow (under 1 a km2) to dark red (over 1,000); towns as black dots sized by people, "
             "capitals ringed white";
  }
  return "";
}

PlanetRaster render_layer(const gen::WorldGrid& grid, const gen::WorldFields& f, const gen::WorldLand& land,
                          const MapInputs& in, MapLayer layer, int W, int H) {
  PlanetRaster r;
  r.w = W;
  r.h = H;
  r.rgb.assign(static_cast<std::size_t>(W) * H * 3, 0);
  r.country.assign(static_cast<std::size_t>(W) * H, -1);
  const double spacing = std::sqrt(grid.sphere_area_km2() / static_cast<double>(grid.size())) / grid.radius_km();  // radians
  const Warp warp{0.7 * spacing, 1.0 / (5.0 * spacing)};
  const double sigma2 = (0.7 * spacing) * (0.7 * spacing);

  // Sampling: per pixel, the nearest cell to the warped point, and land, lake and colour weights over it and its
  // neighbours (a soft edge a few pixels wide).
  std::vector<gen::CellId> at(static_cast<std::size_t>(W) * H);
  std::vector<float> elev(at.size());
  std::vector<std::uint8_t> water(at.size());  // 0 land, 1 sea, 2 open lake, 3 salt lake, 4 dry basin
  Canvas cv{W, H, std::vector<RGB>(at.size())};
  gen::CellId hint = 0;
  std::vector<gen::CellId> near;
  std::vector<double> wgt;
  g_projection = in.projection;
  std::vector<std::uint8_t> outside(at.size(), 0);  // spec 89 amendment: pixels beyond the projection's outline
  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      double lat = 0, lon = 0;
      if (!pixel_latlon(in.projection, W, H, x + 0.5, y + 0.5, lat, lon)) {
        const std::size_t i = static_cast<std::size_t>(y) * W + x;
        outside[i] = 1;
        at[i] = hint;
        water[i] = 1;
        elev[i] = -4000;
        continue;
      }
      const gen::Vec3 p{std::cos(lat) * std::cos(lon), std::cos(lat) * std::sin(lon), std::sin(lat)};
      const gen::Vec3 q = warp.apply(p);
      const gen::CellId c = grid.nearest_cell(q, hint);
      hint = c;
      const std::size_t i = static_cast<std::size_t>(y) * W + x;
      at[i] = c;
      near.assign(1, c);
      for (const gen::CellId nb : grid.neighbours(c)) near.push_back(nb);
      wgt.resize(near.size());
      double ws = 0, e = 0, lake[4] = {}, wland = 0;
      for (std::size_t k = 0; k < near.size(); ++k) {
        const gen::Vec3& cc = grid.centre(near[k]);
        const double dx = cc.x - q.x, dy = cc.y - q.y, dz = cc.z - q.z;
        wgt[k] = std::exp(-(dx * dx + dy * dy + dz * dz) / sigma2);
        ws += wgt[k];
        e += wgt[k] * f.elevation_m[near[k]];
        if (f.land(near[k])) wland += wgt[k], lake[land.lake[near[k]]] += wgt[k];
      }
      for (double& v : wgt) v /= ws;
      e /= ws;
      // Below-cell detail on the coast: small noise where the interpolated height is near sea level.
      e += 120.0 * fbm(q, warp.freq * 1.5, 2, 404) * std::exp(-std::abs(e) / 250.0);
      elev[i] = static_cast<float>(e);
      std::uint8_t wtr = e > 0 ? 0 : 1;
      if (wtr == 0 && wland > 0) {
        const double best = std::max({lake[1], lake[2], lake[3]});
        if (best > 0.5 * wland) wtr = best == lake[1] ? 2 : best == lake[2] ? 3 : 4;
      }
      water[i] = wtr;
      r.country[i] = in.countries && wtr != 1 ? in.countries->country_of[c] : -1;
      if (in.countries && wtr != 1 && r.country[i] < 0)  // a pixel of land drawn over a sea cell: its land neighbour's country
        for (const gen::CellId nb : grid.neighbours(c))
          if (in.countries->country_of[nb] >= 0) { r.country[i] = in.countries->country_of[nb]; break; }
      // The land colour, blended over the land cells around; the sea by depth.
      RGB col{0, 0, 0};
      if (wtr == 1) {
        const double shade = 0.55 + 0.45 * std::max(0.0, 1.0 + e / 6000.0);
        col = {30 * shade, 70 * shade, 140 * shade};
      } else {
        double wl = 0;
        for (std::size_t k = 0; k < near.size(); ++k) {
          if (!f.land(near[k])) continue;
          const RGB& b = kBiome[static_cast<int>(f.biome[near[k]])];
          for (int t = 0; t < 3; ++t) col[t] += wgt[k] * b[t];
          wl += wgt[k];
        }
        if (wl > 0) for (double& v : col) v /= wl;
        else col = kBiome[static_cast<int>(gen::Biome::Grassland)];
        if (e > 1500) col = mix(col, RGB{170, 150, 125}, std::min(0.5, (e - 1500) / 5000));
      }
      cv.px[i] = col;
    }
  }

  // Relief: the interpolated land height, box-blurred a little, lit from the north-west.
  const int R = std::max(1, W / 1400);
  std::vector<float> sm(elev.size()), tmp(elev.size());
  for (std::size_t i = 0; i < elev.size(); ++i) sm[i] = std::max(0.0f, elev[i]);
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      double s = 0;
      for (int k = -R; k <= R; ++k) s += sm[static_cast<std::size_t>(y) * W + ((x + k) % W + W) % W];
      tmp[static_cast<std::size_t>(y) * W + x] = static_cast<float>(s / (2 * R + 1));
    }
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      double s = 0;
      for (int k = -R; k <= R; ++k) s += tmp[static_cast<std::size_t>(std::clamp(y + k, 0, H - 1)) * W + x];
      sm[static_cast<std::size_t>(y) * W + x] = static_cast<float>(s / (2 * R + 1));
    }
  // The sea's depth, blurred wide, so ridges and trenches read as soft shading, not lines (spec 89 amendment).
  std::vector<float> deep(elev.size());
  {
    const int RS = std::max(2, W / 300);
    std::vector<float> t1(elev.size());
    for (std::size_t i = 0; i < elev.size(); ++i) deep[i] = std::min(0.0f, elev[i]);
    for (int pass = 0; pass < 2; ++pass) {
      for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
          double s = 0;
          for (int k = -RS; k <= RS; ++k) s += deep[static_cast<std::size_t>(y) * W + ((x + k) % W + W) % W];
          t1[static_cast<std::size_t>(y) * W + x] = static_cast<float>(s / (2 * RS + 1));
        }
      for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
          double s = 0;
          for (int k = -RS; k <= RS; ++k) s += t1[static_cast<std::size_t>(std::clamp(y + k, 0, H - 1)) * W + x];
          deep[static_cast<std::size_t>(y) * W + x] = static_cast<float>(s / (2 * RS + 1));
        }
    }
  }
  const double km_per_px = 2 * kPi * grid.radius_km() / W;
  auto hillshade = [&](int x, int y) {
    const double nw = sm[static_cast<std::size_t>(std::clamp(y - 1, 0, H - 1)) * W + ((x - 1) % W + W) % W];
    const double se = sm[static_cast<std::size_t>(std::clamp(y + 1, 0, H - 1)) * W + ((x + 1) % W + W) % W];
    const double slope = (se - nw) / (2.83 * km_per_px * 1000.0);
    return std::clamp(1.0 + 25.0 * slope, 0.55, 1.4);
  };

  double max_catch = 0, max_runoff = 0;
  for (gen::CellId c = 0; c < grid.size(); ++c) {
    max_catch = std::max(max_catch, land.fish_catch_t[c] / grid.area_km2(c));
    max_runoff = std::max(max_runoff, land.runoff_mm[c]);
  }

  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      const std::size_t i = static_cast<std::size_t>(y) * W + x;
      const gen::CellId c = at[i];
      const double e = elev[i];
      const std::uint8_t wtr = water[i];
      RGB col = cv.px[i];
      const RGB physical = col;
      const bool sea = wtr == 1;
      if (sea) {  // shelf light, deep sea dark, from the blurred depth; the political map keeps only a hint of it
        const double d = std::clamp(-static_cast<double>(deep[i]) / 5500.0, 0.0, 1.0);
        col = mix(RGB{62, 118, 178}, RGB{22, 52, 112}, layer == MapLayer::Political ? 0.55 + 0.15 * d : std::sqrt(d));
      }
      switch (layer) {
        case MapLayer::Physical:
        case MapLayer::Resources:
        case MapLayer::Energy:
        case MapLayer::Industry:
        case MapLayer::Wind: break;
        case MapLayer::Political:
          if (!sea) {
            const int k = r.country[i];
            const std::string* own = in.people && k >= 0 && k < static_cast<int>(in.people->countries.size()) &&
                                             in.people->countries[static_cast<std::size_t>(k)].color.size() == 7
                                         ? &in.people->countries[static_cast<std::size_t>(k)].color
                                         : nullptr;
            unsigned v = 0;
            if (own && std::sscanf(own->c_str() + 1, "%x", &v) == 1) {  // spec 92: a custom country's colour
              col = {static_cast<double>((v >> 16) & 255), static_cast<double>((v >> 8) & 255), static_cast<double>(v & 255)};
            } else if (k >= 0) {
              const std::uint64_t h = gen::mix64(static_cast<std::uint64_t>(k) + 1);
              col = {90.0 + ((h >> 0) & 127), 90.0 + ((h >> 8) & 127), 90.0 + ((h >> 16) & 127)};
            } else {
              col = {205, 205, 205};
            }
          }
          break;
        case MapLayer::Alliances:
          if (!sea) {
            const int k = r.country[i];
            const int al = in.people && k >= 0 && k < static_cast<int>(in.people->countries.size())
                               ? in.people->countries[static_cast<std::size_t>(k)].alliance
                               : -1;
            if (al >= 0) {
              const std::uint64_t h = gen::mix64(static_cast<std::uint64_t>(al) * 7919 + 17);
              col = {60.0 + ((h >> 0) & 175), 60.0 + ((h >> 8) & 175), 60.0 + ((h >> 16) & 175)};
            } else {
              col = k >= 0 ? RGB{200, 200, 200} : RGB{225, 225, 225};
            }
          }
          break;
        case MapLayer::Topographic:
          if (sea) {
            col = e > -200 ? RGB{170, 210, 235} : ramp({{120, 175, 220}, {60, 110, 180}, {20, 45, 110}}, -e / 6500.0);
          } else {
            col = ramp({{90, 160, 90}, {190, 200, 110}, {220, 190, 110}, {170, 120, 80}, {150, 140, 135}, {250, 250, 250}}, e / 5000.0);
            const int band = static_cast<int>(std::floor(e / 500.0));
            const auto lvl = [&](int xx, int yy) { return static_cast<int>(std::floor(elev[static_cast<std::size_t>(std::clamp(yy, 0, H - 1)) * W + ((xx % W) + W) % W] / 500.0)); };
            if (lvl(x + 1, y) != band || lvl(x, y + 1) != band) col = mix(col, RGB{90, 60, 40}, 0.6);
          }
          break;
        case MapLayer::Temperature: {
          const double t = f.temp_mean_c[c];
          // One stop every 10 C, white at 0 (the six-stop ramp put white at -6 C and red at 18 C, so temperate land read hot).
          col = ramp({{20, 40, 140}, {60, 110, 200}, {150, 195, 235}, {245, 245, 245}, {245, 225, 140}, {235, 140, 50}, {150, 20, 20}},
                     (t + 30) / 60.0);
          if (sea) col = mix(col, RGB{90, 100, 120}, 0.2);
          break;
        }
        case MapLayer::Rain:
          if (!sea) col = ramp({{235, 220, 180}, {200, 220, 160}, {100, 190, 140}, {40, 130, 190}, {20, 50, 140}},
                               (std::log10(std::max(50.0, f.rain_mm[c])) - std::log10(50.0)) / (std::log10(4000.0) - std::log10(50.0)));
          break;
        case MapLayer::Climate:
          if (!sea) col = koppen_colour(gen::koppen_class(gen::rain_months_mm(f, c), gen::temp_months_c(grid, f, c), grid.sin_lat(c) < 0));
          break;
        case MapLayer::Geology: {
          const std::uint64_t h = gen::mix64(static_cast<std::uint64_t>(f.plate[c]) + 77);
          const bool cont = f.elevation_m[c] > -1500;  // a stand-in for crust type: shelf and land read as continental
          const double v = 0.75 + 0.25 * static_cast<double>(h & 255) / 255.0;
          col = cont ? RGB{200 * v, 170 * v, 130 * v} : RGB{110 * v, 140 * v, 180 * v};
          if (f.boundary_kind[c] != gen::BoundaryKind::None && f.boundary_cells[c] <= 1)
            col = mix(col, kBoundary[static_cast<int>(f.boundary_kind[c])], 0.8);
          break;
        }
        case MapLayer::Water:
          if (!sea) {
            col = ramp({{235, 225, 200}, {170, 210, 200}, {90, 160, 200}}, std::sqrt(land.runoff_mm[c] / std::max(1.0, max_runoff)));
            if (land.endorheic[c] && ((x + y) / 3) % 2 == 0) col = mix(col, RGB{150, 110, 70}, 0.5);
          }
          break;
        case MapLayer::Cover:
          if (!sea) {
            const double fo = in.use ? in.use->forest[c] : land.forest_share[c], ar = in.use ? in.use->crop[c] : land.arable_share[c],
                         pa = in.use ? in.use->pasture[c] : land.pasture_share[c];
            const double sum = fo + ar + pa, bare = std::max(0.0, 1.0 - sum), n = std::max(1.0, sum + bare);
            for (int q = 0; q < 3; ++q) col[q] = (kForest[q] * fo + kArable[q] * ar + kPasture[q] * pa + kBare[q] * bare) / n;
            if (f.biome[c] == gen::Biome::Ice) col = kBiome[static_cast<int>(gen::Biome::Ice)];
          }
          break;
        case MapLayer::People:
          if (!sea) {
            const double d = in.people ? in.people->people[c] / grid.area_km2(c) : 0;
            const double t = std::clamp(std::log10(std::max(d, 0.01)) / 3.0, 0.0, 1.0);
            col = d < 0.01 ? RGB{235, 235, 230} : ramp({{250, 240, 170}, {240, 150, 60}, {140, 20, 20}}, t);
          }
          break;
      }
      // Spec 89 rule 4: sea ice, on the maps that show the surface.
      if (sea && !f.sea_ice.empty() && f.sea_ice[c] &&
          (layer == MapLayer::Physical || layer == MapLayer::Topographic || layer == MapLayer::Cover ||  // political maps leave it out
           layer == MapLayer::Wind || layer == MapLayer::Water))
        col = RGB{222, 233, 242};
      // Spec 89 amendment: an ice sheet is no one's and draws as ice, one cap with the sea ice around it.
      if (!sea && f.biome[c] == gen::Biome::Ice &&
          (layer == MapLayer::Political || layer == MapLayer::Wind || layer == MapLayer::Water || layer == MapLayer::Cover))
        col = RGB{236, 241, 246};
      if (layer == MapLayer::Resources && sea && max_catch > 0) {
        const double t = in.use ? in.use->fish_t[c] : land.fish_catch_t[c];  // spec 88: what is landed
        if (t > 0) col = mix(col, RGB{60, 200, 210}, 0.7 * std::sqrt(t / grid.area_km2(c) / max_catch));
      }
      const bool shows_water = layer != MapLayer::Temperature && layer != MapLayer::Climate && layer != MapLayer::Geology &&
                               layer != MapLayer::People && layer != MapLayer::Rain;
      if (shows_water && wtr == 2) col = kLakeOpen;
      if (shows_water && wtr == 3) col = kLakeSalt;
      if (shows_water && wtr == 4 && layer != MapLayer::Water) col = kLakeDry;
      const bool shaded = layer == MapLayer::Physical || layer == MapLayer::Resources || layer == MapLayer::Industry || layer == MapLayer::Energy || layer == MapLayer::Cover ||
                          layer == MapLayer::Wind || layer == MapLayer::Topographic || layer == MapLayer::Political;
      if (shaded && wtr == 0) for (double& v : col) v *= layer == MapLayer::Political ? 0.85 + 0.15 * hillshade(x, y) : hillshade(x, y);
      if (layer == MapLayer::Resources || layer == MapLayer::Industry || layer == MapLayer::Energy || layer == MapLayer::Wind)
        col = mix(col, RGB{200, 200, 200}, layer == MapLayer::Wind ? 0.25 : (layer == MapLayer::Industry || layer == MapLayer::Energy) ? 0.55 : 0.4);
      (void)physical;
      // A border (spec 52 slice 4): land whose country differs from the land to its right or below.
      const int k = r.country[i];
      const int kr = r.country[static_cast<std::size_t>(y) * W + (x + 1) % W];
      const int kd = y + 1 < H ? r.country[static_cast<std::size_t>(y + 1) * W + x] : -1;
      const bool border = k >= 0 && ((kr >= 0 && kr != k) || (kd >= 0 && kd != k));
      if (border && layer != MapLayer::Geology && layer != MapLayer::Temperature)
        col = layer == MapLayer::Political ? RGB{40, 40, 40} : mix(col, RGB{60, 60, 60}, 0.45);
      cv.px[i] = col;
    }

  for (std::size_t i = 0; i < outside.size(); ++i)
    if (outside[i]) cv.px[i] = RGB{18, 20, 26}, r.country[i] = -1;  // beyond the outline

  // Rivers as smooth curves from source to mouth (Catmull-Rom through the cells' centres as the picture shows them),
  // wider downstream with discharge: one pixel at the river threshold, four at 100,000 m3/s.
  const bool draws_rivers = layer == MapLayer::Physical || layer == MapLayer::Political || layer == MapLayer::Topographic ||
                            layer == MapLayer::Water || layer == MapLayer::Cover || layer == MapLayer::Resources || layer == MapLayer::Industry || layer == MapLayer::Energy ||
                            layer == MapLayer::Wind;
  if (draws_rivers) {
    const double scale = std::max(0.5, static_cast<double>(W) / 2048.0);
    std::vector<std::uint8_t> fed(grid.size(), 0), drawn(grid.size(), 0);
    for (gen::CellId c = 0; c < grid.size(); ++c)
      if (f.land(c) && land.river[c] && land.downstream[c] != gen::kNoCell) fed[land.downstream[c]] = 1;
    auto pix = [&](gen::CellId c, double& px, double& py) { to_pixel(warp.undo(grid.centre(c)), W, H, px, py); };
    for (gen::CellId s = 0; s < grid.size(); ++s) {
      if (!f.land(s) || !land.river[s] || fed[s]) continue;  // a source: a river cell no river flows into
      std::vector<gen::CellId> chain{s};
      for (gen::CellId c = s; drawn[c] == 0;) {
        drawn[c] = 1;
        const gen::CellId d = land.downstream[c];
        if (d == gen::kNoCell) break;
        chain.push_back(d);
        if (!f.land(d)) break;
        c = d;
      }
      std::vector<double> xs, ys, ws;
      for (const gen::CellId c : chain) {
        double px, py;
        pix(c, px, py);
        if (!xs.empty() && std::abs(px - xs.back()) > W / 2.0) break;  // across the date line
        xs.push_back(px), ys.push_back(py);
        const double q = std::max(f.land(c) ? land.discharge_m3s[c] : land.discharge_m3s[chain[chain.size() > 1 ? chain.size() - 2 : 0]], 100.0);
        ws.push_back((0.7 + 3.3 * std::clamp(std::log10(q / 100.0) / 3.0, 0.0, 1.0)) * scale);
      }
      // Smooth the path: corner cutting (Chaikin, three rounds), then a gentle meander across it from noise.
      for (int round = 0; round < 3 && xs.size() >= 3; ++round) {
        std::vector<double> nx{xs.front()}, ny{ys.front()}, nw{ws.front()};
        for (std::size_t k = 0; k + 1 < xs.size(); ++k) {
          for (const double t : {0.25, 0.75}) {
            nx.push_back(xs[k] + (xs[k + 1] - xs[k]) * t);
            ny.push_back(ys[k] + (ys[k + 1] - ys[k]) * t);
            nw.push_back(ws[k] + (ws[k + 1] - ws[k]) * t);
          }
        }
        nx.push_back(xs.back()), ny.push_back(ys.back()), nw.push_back(ws.back());
        xs.swap(nx), ys.swap(ny), ws.swap(nw);
      }
      const double cell_px = spacing / (2 * kPi) * W;
      for (std::size_t k = 1; k + 1 < xs.size(); ++k) {
        const double tx = xs[k + 1] - xs[k - 1], ty = ys[k + 1] - ys[k - 1], tl = std::sqrt(tx * tx + ty * ty);
        if (tl <= 0) continue;
        const double m = 0.22 * cell_px * value_noise(xs[k] / cell_px * 1.7, ys[k] / cell_px * 1.7, 0.5, 505);
        xs[k] += -ty / tl * m, ys[k] += tx / tl * m;
      }
      for (std::size_t k = 0; k + 1 < xs.size(); ++k) {
        const int ix = ((static_cast<int>(xs[k + 1]) % W) + W) % W, iy = std::clamp(static_cast<int>(ys[k + 1]), 0, H - 1);
        const double wd = (ws[k] + ws[k + 1]) / 2;
        cv.line(xs[k], ys[k], xs[k + 1], ys[k + 1], wd, kRiver, std::clamp(0.45 + 0.35 * wd / scale, 0.0, 1.0));
        if (water[static_cast<std::size_t>(iy) * W + ix] == 1) break;  // the mouth
      }
    }
  }

  if (layer == MapLayer::Wind) {  // arrows every few degrees: the prevailing wind's direction by latitude band
    const int step = std::max(16, W / 40);
    for (int y = step / 2; y < H; y += step)
      for (int x = step / 2; x < W; x += step) {
        const double lat = 90.0 - 180.0 * (y + 0.5) / H, az = std::abs(lat);
        const double dir = (az >= 30 && az < 60) ? 1.0 : -1.0;  // westerlies blow toward the east
        const double tilt = (az < 30 ? (lat > 0 ? 0.35 : -0.35) : az < 60 ? (lat > 0 ? -0.25 : 0.25) : (lat > 0 ? 0.3 : -0.3));
        const double len = step * 0.4;
        const double x1 = x + dir * len, y1 = y + tilt * len;
        cv.line(x - dir * len, y - tilt * len, x1, y1, 1.2 * std::max(0.6, W / 2048.0), RGB{30, 30, 60});
        cv.disc(x1, y1, 1.8 * std::max(0.6, W / 2048.0), RGB{30, 30, 60});
      }
  }

  if (layer == MapLayer::Resources && in.deposits) {
    double median[gen::resource_count] = {};
    for (int k = 0; k < gen::resource_count; ++k) {
      std::vector<double> a;
      for (const gen::Deposit& d : in.deposits->deposits)
        if (static_cast<int>(d.kind) == k) a.push_back(d.amount);
      if (a.empty()) continue;
      std::nth_element(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(a.size() / 2), a.end());
      median[k] = a[a.size() / 2];
    }
    std::vector<char> worked(in.deposits->deposits.size(), in.use ? 0 : 1);
    if (in.use)
      for (const gen::WorkSite& s : in.use->sites) worked[static_cast<std::size_t>(s.deposit)] = 1;
    for (std::size_t di = 0; di < in.deposits->deposits.size(); ++di) {
      const gen::Deposit& d = in.deposits->deposits[di];
      const int k = static_cast<int>(d.kind);
      double x, y;
      to_pixel(warp.undo(grid.centre(d.cell)), W, H, x, y);
      const double rel = median[k] > 0 ? d.amount / median[k] : 1;
      if (rel < 3) continue;  // the large deposits only: about one in seven
      const double rad = std::clamp(0.6 + 1.6 * std::log10(rel), 1.0, 4.5) * (W / 2048.0 + 0.5);
      const double off = (k - 2.5) * 0.9 * (W / 2048.0);
      cv.disc(x + off, y + off * 0.5, rad, kDeposit[k], worked[di] ? 0.95 : 0.3);  // spec 88: unworked deposits faint
    }
  }

  if (layer == MapLayer::Energy && in.energy) {  // spec 94: lines first, plants over them
    const gen::WorldEnergy& en = *in.energy;
    for (const gen::GridLine& l : en.lines) {
      double x0, y0, x1, y1;
      to_pixel(warp.undo(grid.centre(l.a)), W, H, x0, y0);
      to_pixel(warp.undo(grid.centre(l.b)), W, H, x1, y1);
      if (std::abs(x1 - x0) > W / 2.0) continue;  // a line across the map's seam
      const int steps = std::max(2, static_cast<int>(std::hypot(x1 - x0, y1 - y0)));
      for (int s = 0; s <= steps; ++s) {
        if (l.country < 0 && (s / 3) % 2) continue;  // between countries: dashed
        const double t = static_cast<double>(s) / steps;
        cv.disc(x0 + (x1 - x0) * t, y0 + (y1 - y0) * t, 0.45 * (W / 2048.0 + 0.5), l.country < 0 ? RGB{170, 30, 30} : RGB{40, 40, 50}, 0.8);
      }
    }
    static const RGB kFuel[gen::kFuels] = {{20, 20, 20}, {240, 140, 30}, {120, 80, 40}, {30, 90, 220}, {250, 220, 40}, {250, 250, 250}, {230, 180, 30}};
    for (const gen::PowerPlant& pl : en.plants) {
      double x, y;
      to_pixel(warp.undo(grid.centre(pl.cell)), W, H, x, y);
      const auto q = static_cast<std::size_t>(pl.fuel);
      const bool small = pl.fuel == gen::Fuel::Wind || pl.fuel == gen::Fuel::Solar || pl.fuel == gen::Fuel::Oil;
      const double rad = (small ? 0.7 : std::clamp(0.6 + 0.5 * std::log2(1.0 + pl.mw / 200.0), 1.0, 3.5)) * (W / 2048.0 + 0.4);
      if (pl.fuel == gen::Fuel::Nuclear) cv.disc(x, y, rad + 0.8, RGB{30, 30, 30});
      cv.disc(x, y, rad, kFuel[q], small ? 0.6 : 0.95);
    }
  }

  if (layer == MapLayer::Industry && in.industry) {  // spec 93: each town's plants by kind, a dot sized by the count
    static const std::map<std::string, RGB> kColour = {
        {"oil_refinery", {120, 70, 30}},    {"steel_works", {200, 30, 30}},  {"alumina_refinery", {230, 120, 180}},
        {"aluminium_smelter", {200, 60, 160}}, {"copper_smelter", {20, 150, 150}}, {"cement_plant", {110, 110, 110}},
        {"flour_mill", {230, 200, 40}},     {"sugar_mill", {250, 230, 120}}, {"meat_plant", {130, 60, 160}},
        {"sawmill", {40, 130, 40}},         {"fish_plant", {40, 190, 220}}};
    const gen::WorldIndustry& ind = *in.industry;
    // Light industry first (many, small), heavy industry over it.
    for (int pass = 0; pass < 2; ++pass)
      for (std::size_t t = 0; t < ind.of_town.size(); ++t) {
        std::map<int, int> count;
        for (int p : ind.of_town[t]) ++count[ind.plants[static_cast<std::size_t>(p)].kind];
        if (count.empty()) continue;
        double x, y;
        to_pixel(warp.undo(grid.centre(in.people->towns[t].cell)), W, H, x, y);
        int slot = 0;
        for (const auto& [k, n] : count) {
          const gen::PlantKind& K = ind.kinds[static_cast<std::size_t>(k)];
          const bool heavy = K.rule != gen::PlantRule::Market && K.id != "sawmill" && K.id != "fish_plant" && K.id != "sugar_mill";
          if (heavy != (pass == 1)) continue;
          auto it = kColour.find(K.id);
          const RGB c = it != kColour.end() ? it->second : RGB{0, 0, 0};
          // Heavy industry large and solid; light industry (mills, meat plants, sawmills) small and faint beneath it.
          const double rad = heavy ? std::clamp(1.2 + 0.7 * std::log2(1.0 + n), 1.4, 4.5) * (W / 2048.0 + 0.4)
                                   : std::clamp(0.4 + 0.25 * std::log2(1.0 + n), 0.5, 1.2) * (W / 2048.0 + 0.4);
          const double off = (slot++ % 4 - 1.5) * 1.2 * (W / 2048.0 + 0.4);
          cv.disc(x + off, y + (heavy ? -1.0 : 1.0) * (W / 2048.0 + 0.4), rad, c, heavy ? 0.95 : 0.35);
        }
      }
  }

  if (layer == MapLayer::People && in.people) {
    for (const gen::Settlement& t : in.people->towns) {
      double x, y;
      to_pixel(warp.undo(grid.centre(t.cell)), W, H, x, y);
      const double rad = std::clamp(0.4 * (std::log10(t.people) - 3.5), 0.6, 3.5) * (W / 2048.0 + 0.3);
      if (t.rank == 1) cv.disc(x, y, rad + 1.6, RGB{255, 255, 255});
      cv.disc(x, y, rad, RGB{15, 15, 15});
    }
  }

  for (std::size_t i = 0; i < cv.px.size(); ++i)
    for (int q = 0; q < 3; ++q) r.rgb[i * 3 + q] = static_cast<unsigned char>(std::clamp(cv.px[i][q], 0.0, 255.0));
  return r;
}

PlanetRaster render_planet(const gen::WorldGrid& grid, const gen::WorldFields& f, const gen::WorldLand& land,
                           const gen::WorldCountries* countries, bool political, int W, int H) {
  MapInputs in;
  in.countries = countries;
  return render_layer(grid, f, land, in, political ? MapLayer::Political : MapLayer::Physical, W, H);
}

void draw_routes(PlanetRaster& r, const gen::WorldGrid& grid, const gen::WorldFields& f, const gen::WorldLand& land,
                 const gen::WorldRoutes& routes) {
  auto pixel = [&](gen::CellId c, int& x, int& y) {
    const gen::Vec3& v = grid.centre(c);
    const double lat = std::asin(std::clamp(v.z, -1.0, 1.0)) * 180.0 / 3.14159265358979323846;
    const double lon = std::atan2(v.y, v.x) * 180.0 / 3.14159265358979323846;
    x = std::clamp(static_cast<int>((lon + 180.0) / 360.0 * r.w), 0, r.w - 1);
    y = std::clamp(static_cast<int>((90.0 - lat) / 180.0 * r.h), 0, r.h - 1);
  };
  auto put = [&](int x, int y, const unsigned char* col) {
    for (int q = 0; q < 3; ++q) r.rgb[(static_cast<std::size_t>(y) * r.w + x) * 3 + q] = col[q];
  };
  static const unsigned char road[3] = {150, 20, 20}, river[3] = {10, 40, 160}, sea[3] = {235, 235, 235}, town[3] = {0, 0, 0};
  for (const gen::Route& route : routes.routes)
    for (std::size_t i = 1; i < route.path.size(); ++i) {
      int x0, y0, x1, y1;
      pixel(route.path[i - 1], x0, y0);
      pixel(route.path[i], x1, y1);
      if (std::abs(x1 - x0) > r.w / 2) continue;  // across the date line
      const bool wet = !f.land(route.path[i - 1]) || !f.land(route.path[i]);
      const bool on_river = !wet && land.river[route.path[i - 1]] == 2 && land.river[route.path[i]] == 2 &&
                            (land.downstream[route.path[i - 1]] == route.path[i] || land.downstream[route.path[i]] == route.path[i - 1]);
      const unsigned char* col = wet ? sea : on_river ? river : road;
      const int steps = std::max(std::abs(x1 - x0), std::abs(y1 - y0));
      for (int s = 0; s <= steps; ++s) {
        const int x = steps ? x0 + (x1 - x0) * s / steps : x0, y = steps ? y0 + (y1 - y0) * s / steps : y0;
        put(x, y, col);
      }
    }
  for (const gen::RouteNode& n : routes.nodes) {
    int x, y;
    pixel(n.cell, x, y);
    put(x, y, town);
  }
}

void write_bmp(const std::string& path, const PlanetRaster& r) {
  std::ofstream o(path, std::ios::binary);
  auto w32 = [&](std::uint32_t v) { for (int i = 0; i < 4; ++i) o.put(static_cast<char>((v >> (8 * i)) & 255)); };
  auto w16 = [&](std::uint16_t v) { o.put(static_cast<char>(v & 255)); o.put(static_cast<char>(v >> 8)); };
  const int W = r.w, H = r.h;
  const int pad = (4 - (W * 3) % 4) % 4;  // every row is padded to a multiple of 4 bytes
  const std::uint32_t size = static_cast<std::uint32_t>((W * 3 + pad) * H);
  o.write("BM", 2); w32(54 + size); w32(0); w32(54); w32(40); w32(W); w32(H); w16(1); w16(24);
  w32(0); w32(size); w32(2835); w32(2835); w32(0); w32(0);
  for (int y = H - 1; y >= 0; --y) {
    for (int x = 0; x < W; ++x) {
      const auto* p3 = &r.rgb[(static_cast<std::size_t>(y) * W + x) * 3];
      o.put(static_cast<char>(p3[2])); o.put(static_cast<char>(p3[1])); o.put(static_cast<char>(p3[0]));
    }
    for (int k = 0; k < pad; ++k) o.put(0);
  }
}

TownPicture town_picture(const gen::TownLayout& t, int scale) {
  TownPicture out;
  out.m_per_px = scale;
  PlanetRaster& img = out.img;
  img.w = std::max(1, t.width_m / scale);
  img.h = std::max(1, t.height_m / scale);
  img.rgb.assign(static_cast<std::size_t>(img.w) * img.h * 3, 110);
  out.plot.assign(static_cast<std::size_t>(img.w) * img.h, -1);
  static const unsigned char colour[5][3] = {{236, 222, 196}, {242, 168, 92}, {170, 140, 200}, {120, 190, 110}, {90, 140, 230}};
  for (std::size_t k = 0; k < t.plots.size(); ++k) {
    const auto& p = t.plots[k];
    const bool empty = p.use == gen::LandUse::Residential && p.households == 0;
    for (int y = p.y / scale; y < (p.y + p.h) / scale && y < img.h; ++y)
      for (int x = p.x / scale; x < (p.x + p.w) / scale && x < img.w; ++x) {
        const bool edge = x == p.x / scale || y == p.y / scale;
        const auto* c = colour[static_cast<int>(p.use)];
        const std::size_t i = static_cast<std::size_t>(y) * img.w + x;
        for (int q = 0; q < 3; ++q)
          img.rgb[i * 3 + static_cast<std::size_t>(q)] =
              static_cast<unsigned char>(edge ? c[q] * 0.6 : empty ? 250 : (p.units > 1 ? c[q] * 0.85 : c[q]));
        out.plot[i] = static_cast<int>(k);
      }
  }
  return out;
}

PlanetRaster site_picture(const std::string& site_toml, int px) {
  PlanetRaster img;
  std::string grid;
  struct Thing {
    std::string kind;
    int x = 0, y = 0;
  };
  std::vector<Thing> things;  // the ground level's furniture and fittings
  try {
    const toml::table t = toml::parse(site_toml);
    if (const toml::array* levels = t["level"].as_array())
      for (const auto& l : *levels)
        if (const toml::table* lt = l.as_table(); lt && (*lt)["z"].value_or(0) == 0) grid = (*lt)["grid"].value_or(std::string{});
    if (const toml::array* objs = t["object"].as_array())
      for (const auto& o : *objs)
        if (const toml::table* ot = o.as_table())
          if (const toml::array* at = (*ot)["at"].as_array(); at && at->size() == 3 && (*at)[0].value_or(-1) == 0)
            things.push_back({(*ot)["kind"].value_or(std::string{}), static_cast<int>((*at)[1].value_or(0)), static_cast<int>((*at)[2].value_or(0))});
  } catch (const std::exception&) {
    return img;
  }
  // The grid: edge rows ("+" corners, the edge above each cell) and cell rows (the edge left of each cell, then the cell).
  std::vector<std::string> rows;
  {
    std::string line;
    for (char c : grid) {
      if (c == '\n') {
        if (!line.empty()) rows.push_back(line);
        line.clear();
      } else if (c != '\r') {
        line += c;
      }
    }
    if (!line.empty()) rows.push_back(line);
  }
  if (rows.size() < 2) return img;
  const int H = static_cast<int>(rows.size() / 2), W = static_cast<int>(rows[1].size() / 2);
  img.w = W * px;
  img.h = H * px;
  img.rgb.assign(static_cast<std::size_t>(img.w) * img.h * 3, 0);
  auto fill = [&](int x0, int y0, int w, int h, const unsigned char c[3]) {
    for (int y = std::max(0, y0); y < std::min(img.h, y0 + h); ++y)
      for (int x = std::max(0, x0); x < std::min(img.w, x0 + w); ++x)
        for (int q = 0; q < 3; ++q) img.rgb[(static_cast<std::size_t>(y) * img.w + x) * 3 + static_cast<std::size_t>(q)] = c[q];
  };
  auto ground = [](char c) -> const unsigned char* {
    static const unsigned char street[3] = {95, 95, 100}, pave[3] = {175, 175, 170}, grass[3] = {120, 175, 95},
                               floor[3] = {222, 205, 170}, path[3] = {200, 185, 150}, other[3] = {160, 150, 140};
    switch (c) {
      case ' ': return street;
      case '_': return pave;
      case '"': return grass;
      case '.': return floor;
      case ':': return path;
      default: return other;
    }
  };
  auto edge = [](char c) -> const unsigned char* {
    static const unsigned char wall[3] = {45, 40, 38}, inner[3] = {110, 95, 80}, window[3] = {120, 190, 235}, door[3] = {150, 90, 40},
                               fence[3] = {90, 80, 60};
    switch (c) {
      case '#': return wall;
      case '-':
      case '|': return inner;
      case 'W': return window;
      case 'D':
      case 'E': return door;
      case '=': return fence;
      default: return nullptr;
    }
  };
  const int t = std::max(1, px / 4);  // a wall's thickness in pixels
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      const std::string& cell = rows[static_cast<std::size_t>(2 * y + 1)];
      const char g = static_cast<std::size_t>(2 * x + 1) < cell.size() ? cell[static_cast<std::size_t>(2 * x + 1)] : ' ';
      fill(x * px, y * px, px, px, ground(g));
    }
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      const std::string& above = rows[static_cast<std::size_t>(2 * y)];
      const std::string& cell = rows[static_cast<std::size_t>(2 * y + 1)];
      if (const char e = static_cast<std::size_t>(2 * x + 1) < above.size() ? above[static_cast<std::size_t>(2 * x + 1)] : ' '; edge(e))
        fill(x * px, y * px - t / 2, px, t, edge(e));
      if (const char e = static_cast<std::size_t>(2 * x) < cell.size() ? cell[static_cast<std::size_t>(2 * x)] : ' '; edge(e))
        fill(x * px - t / 2, y * px, t, px, edge(e));
    }
  // Furniture and fittings as small squares by kind: beds blue, seats and tables brown, the kitchen white, the bathroom
  // pale blue, storage orange, lamps yellow.
  for (const Thing& th : things) {
    static const unsigned char bed[3] = {70, 110, 190}, wood[3] = {130, 85, 45}, kitchen[3] = {245, 245, 240}, bath[3] = {160, 210, 230},
                               store[3] = {225, 140, 50}, lamp[3] = {245, 215, 70}, other[3] = {90, 90, 90};
    const unsigned char* c = other;
    if (th.kind == "bed") c = bed;
    else if (th.kind == "sofa" || th.kind == "table" || th.kind == "wardrobe" || th.kind == "bench") c = wood;
    else if (th.kind == "fridge" || th.kind == "stove" || th.kind == "hot_plate") c = kitchen;
    else if (th.kind == "toilet" || th.kind == "bathtub" || th.kind == "sink") c = bath;
    else if (th.kind == "pantry" || th.kind == "stockpile" || th.kind == "shelf") c = store;
    else if (th.kind == "lamp") c = lamp;
    else if (th.kind == "plot" || th.kind == "tree") continue;
    const int m = std::max(1, px / 5);
    fill(th.x * px + m, th.y * px + m, px - 2 * m, px - 2 * m, c);
  }
  return img;
}

}  // namespace zs::atlas
