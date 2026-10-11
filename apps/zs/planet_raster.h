// The planet as a 2:1 equirectangular picture: biome shaded by height, rivers in blue, borders dark, or one colour per
// country. Shared by `zs gen world --bmp` and the Atlas (spec 55), so the screen shows what the file shows.
#pragma once
#include <string>
#include <vector>

#include "zs/gen/world_countries.h"
#include "zs/gen/world_fields.h"
#include "zs/gen/world_grid.h"
#include "zs/gen/world_land.h"

namespace zs::atlas {

struct PlanetRaster {
  int w = 0, h = 0;
  std::vector<unsigned char> rgb;  // w * h * 3, row 0 at the north
  std::vector<int> country;        // w * h, the country index under each pixel, -1 at sea or unruled
};

PlanetRaster render_planet(const gen::WorldGrid& grid, const gen::WorldFields& fields, const gen::WorldLand& land,
                           const gen::WorldCountries* countries, bool political, int w = 2048, int h = 1024);
// 24-bit BMP, bottom row first.
void write_bmp(const std::string& path, const PlanetRaster& r);

}  // namespace zs::atlas
