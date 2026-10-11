#include "planet_raster.h"

#include <algorithm>
#include <cstdint>
#include <fstream>

#include "zs/gen/keyed.h"

namespace zs::atlas {

PlanetRaster render_planet(const gen::WorldGrid& grid, const gen::WorldFields& f, const gen::WorldLand& land,
                           const gen::WorldCountries* countries, bool political, int W, int H) {
  static const unsigned char rgb[gen::biome_count][3] = {
      {30, 70, 140}, {240, 245, 250}, {170, 180, 150}, {60, 110, 80},  {40, 140, 60},
      {150, 190, 90}, {225, 200, 130}, {190, 180, 70}, {20, 100, 40},  {130, 120, 110}};
  PlanetRaster r;
  r.w = W;
  r.h = H;
  r.rgb.assign(static_cast<std::size_t>(W) * H * 3, 0);
  r.country.assign(static_cast<std::size_t>(W) * H, -1);
  std::vector<gen::CellId> at(static_cast<std::size_t>(W) * H);
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x)
      at[static_cast<std::size_t>(y) * W + x] = grid.cell_at(90.0 - 180.0 * (y + 0.5) / H, -180.0 + 360.0 * (x + 0.5) / W);
  auto country_at = [&](int x, int y) {
    return countries ? countries->country_of[at[static_cast<std::size_t>(y) * W + ((x + W) % W)]] : -1;
  };
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      const auto c = at[static_cast<std::size_t>(y) * W + x];
      // A border (spec 52 slice 4): land whose country differs from the land to its right or below.
      const int k = country_at(x, y);
      r.country[static_cast<std::size_t>(y) * W + x] = k;
      const bool border = k >= 0 && ((country_at(x + 1, y) >= 0 && country_at(x + 1, y) != k) ||
                                     (y + 1 < H && country_at(x, y + 1) >= 0 && country_at(x, y + 1) != k));
      double shade = 1.0;
      unsigned char col[3];
      const auto b = static_cast<int>(f.biome[c]);
      for (int q = 0; q < 3; ++q) col[q] = rgb[b][q];
      if (f.land(c)) shade = 0.85 + 0.15 * std::min(1.0, f.elevation_m[c] / 4000.0);
      else shade = 0.55 + 0.45 * std::max(0.0, 1.0 + f.elevation_m[c] / 6000.0);
      if (f.land(c) && (land.river[c] || land.lake[c])) {  // rivers and lakes in blue (spec 52)
        const bool big = land.river[c] == 2;
        col[0] = big ? 0 : 110;
        col[1] = big ? 150 : 215;
        col[2] = big ? 255 : 255;
      }
      if (political && k >= 0) {  // one colour per country, keyed by its index
        const std::uint64_t h = gen::mix64(static_cast<std::uint64_t>(k) + 1);
        for (int q = 0; q < 3; ++q) col[q] = static_cast<unsigned char>(90 + ((h >> (8 * q)) & 127));
        shade = 1.0;
      }
      if (border) col[0] = col[1] = col[2] = 40, shade = 1.0;
      for (int q = 0; q < 3; ++q)
        r.rgb[(static_cast<std::size_t>(y) * W + x) * 3 + q] = static_cast<unsigned char>(col[q] * shade);
    }
  return r;
}

void write_bmp(const std::string& path, const PlanetRaster& r) {
  std::ofstream o(path, std::ios::binary);
  auto w32 = [&](std::uint32_t v) { for (int i = 0; i < 4; ++i) o.put(static_cast<char>((v >> (8 * i)) & 255)); };
  auto w16 = [&](std::uint16_t v) { o.put(static_cast<char>(v & 255)); o.put(static_cast<char>(v >> 8)); };
  const int W = r.w, H = r.h;
  o.write("BM", 2); w32(54 + W * H * 3); w32(0); w32(54); w32(40); w32(W); w32(H); w16(1); w16(24);
  w32(0); w32(W * H * 3); w32(2835); w32(2835); w32(0); w32(0);
  for (int y = H - 1; y >= 0; --y)
    for (int x = 0; x < W; ++x) {
      const auto* p3 = &r.rgb[(static_cast<std::size_t>(y) * W + x) * 3];
      o.put(static_cast<char>(p3[2])); o.put(static_cast<char>(p3[1])); o.put(static_cast<char>(p3[0]));
    }
}

}  // namespace zs::atlas
