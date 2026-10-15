// The planet as a 2:1 equirectangular picture in layers (spec 83 rule 5). Each pixel looks up the world through a small
// noise warp and blends the cells around it, so coasts, biome edges and lakes read as natural lines, not hexagons;
// rivers are smooth curves from source to mouth. Shared by
// `zs gen world --bmp` and the Atlas (spec 55), so the screen shows what the file shows.
#pragma once
#include "zs/gen/town_layout.h"
#include "zs/gen/world_industry.h"
#include <string>
#include <vector>

#include "zs/gen/world_countries.h"
#include "zs/gen/world_deposits.h"
#include "zs/gen/world_fields.h"
#include "zs/gen/world_grid.h"
#include "zs/gen/world_land.h"
#include "zs/gen/world_land_use.h"
#include "zs/gen/world_people.h"
#include "zs/gen/world_routes.h"

namespace zs::atlas {

struct PlanetRaster {
  int w = 0, h = 0;
  std::vector<unsigned char> rgb;  // w * h * 3, row 0 at the north
  std::vector<int> country;        // w * h, the country index under each pixel, -1 at sea or unruled
};

// Spec 83 rule 5 (amended 2026-10-14, the user: "different overlays: political, physical, topographic, climatic,
// geological, urban ... resources, wind", after Cities: Skylines' info views).
enum class MapLayer { Physical, Political, Topographic, Temperature, Rain, Climate, Wind, Geology, Water, Cover, Resources, People,
                      Alliances, Industry };
inline constexpr int kMapLayers = 14;
const char* layer_name(MapLayer l);
bool parse_layer(const std::string& name, MapLayer& out);
// The legend of a layer, one line per colour, for printing beside the picture.
std::string layer_legend(MapLayer l);

// Spec 89 amendment: how the sphere is laid flat. Equirectangular stretches each pole into a whole row (polar caps read
// as bands); Equal Earth (Savric, Patterson and Jenny 2018) keeps areas and draws the poles as short curved edges.
enum class Projection { Equirectangular, EqualEarth };

// What a layer may draw; any pointer may be null (the layer then leaves that part out).
struct MapInputs {
  Projection projection = Projection::Equirectangular;
  const gen::WorldCountries* countries = nullptr;
  const gen::WorldDeposits* deposits = nullptr;
  const gen::WorldPeople* people = nullptr;
  const gen::WorldLandUse* use = nullptr;  // spec 88: cover shows the land in use; resources the worked sites
  const gen::WorldIndustry* industry = nullptr;  // spec 93: the industry layer's plants
};
PlanetRaster render_layer(const gen::WorldGrid& grid, const gen::WorldFields& fields, const gen::WorldLand& land,
                          const MapInputs& in, MapLayer layer, int w = 2048, int h = 1024);

// The physical layer (or political when `political`), as before spec 83's layers.
PlanetRaster render_planet(const gen::WorldGrid& grid, const gen::WorldFields& fields, const gen::WorldLand& land,
                           const gen::WorldCountries* countries, bool political, int w = 2048, int h = 1024);
// Spec 74: the routes over a picture: roads dark red, rivers dark blue, sea lanes white, town cells black.
void draw_routes(PlanetRaster& r, const gen::WorldGrid& grid, const gen::WorldFields& f, const gen::WorldLand& land,
                 const gen::WorldRoutes& routes);
// 24-bit BMP, bottom row first.
void write_bmp(const std::string& path, const PlanetRaster& r);

// Spec 92 rule 5: a town's layout as a picture (spec 71): streets grey, plots by use with their outlines, empty houses
// pale, apartment houses darker; `plot` gives each pixel's plot index (-1 a street). Used by the Atlas and `zs gen layout`.
struct TownPicture {
  PlanetRaster img;
  std::vector<int> plot;
  int m_per_px = 2;
};
TownPicture town_picture(const gen::TownLayout& t, int m_per_px = 2);
// Spec 92 rule 4: a plot's site map (spec 72's TOML) seen from above, its ground level: street, pavement, grass, floors,
// walls, windows and doors, `px` pixels a cell.
PlanetRaster site_picture(const std::string& site_toml, int px = 8);

}  // namespace zs::atlas
