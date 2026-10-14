#include "world_numbers.h"

#include <algorithm>

namespace zs::atlas {

namespace {

PreviewNumber number(const std::string& what, double value, double earth, const std::string& unit, double lo, double hi) {
  return {what, value, earth, unit, value >= lo && value <= hi};
}

}  // namespace

std::vector<PreviewNumber> key_numbers(const gen::GeneratedWorld& w) {
  using gen::Biome;
  const gen::WorldGrid& g = w.grid;
  double land = 0, ice = 0, lakes = 0, endorheic = 0, forest = 0, arable = 0;
  double biome[gen::biome_count] = {};
  for (gen::CellId c = 0; c < g.size(); ++c) {
    if (!w.fields.land(c)) continue;
    const double a = g.area_km2(c);
    land += a;
    biome[static_cast<int>(w.fields.biome[c])] += a;
    if (w.fields.biome[c] == Biome::Ice) ice += a;
    if (w.land.lake[c] == gen::kLakeOpen || w.land.lake[c] == gen::kLakeTerminal) lakes += a;
    if (w.land.endorheic[c]) endorheic += a;
    forest += w.land.forest_share[c] * a;
    arable += w.land.arable_share[c] * a;
  }
  const double non_ice = std::max(1.0, land - ice), l = std::max(1.0, land);
  std::vector<PreviewNumber> out;
  // Earth's values: land 29 % and 148.9 million km2 (spec 31, WNC-1); biomes WMR-1; lakes WMR-8; closed basins WMR-10;
  // forest WMR-13; arable WMR-15; people WPP 2024; countries of 89,000 km2 or more, agglomerations DR-1, DR-2.
  out.push_back(number("Land share", 100 * land / g.sphere_area_km2(), 29.2, "%", 15, 45));
  out.push_back(number("Land", land / 1e6, 148.9, "million km2", 74, 300));
  struct B { Biome b; double earth; };
  for (const B& b : {B{Biome::TropicalForest, 18.0}, B{Biome::Savanna, 16.7}, B{Biome::Desert, 19.3}, B{Biome::Grassland, 7.8},
                     B{Biome::TemperateForest, 14.5}, B{Biome::BorealForest, 11.4}, B{Biome::Tundra, 8.7}})
    out.push_back(number(std::string(gen::biome_name(b.b)) + " (of land without ice)", 100 * biome[static_cast<int>(b.b)] / non_ice,
                         b.earth, "%", 0.6 * b.earth, 1.5 * b.earth));
  out.push_back(number("Lakes", 100 * lakes / l, 1.8, "% of land", 0.5, 3.0));
  out.push_back(number("Closed (endorheic) basins", 100 * endorheic / l, 17.5, "% of land", 8, 30));
  out.push_back(number("Forest", 100 * forest / l, 31, "% of land", 20, 42));
  out.push_back(number("Arable", 100 * arable / l, 11, "% of land", 5.5, 22));
  out.push_back(number("People", w.people.total_people / 1e9, 8.09, "billion", 4, 12));
  int countries = 0;
  for (const gen::Country& c : w.people.countries) countries += c.area_km2 >= 89000;
  out.push_back(number("Countries of 89,000 km2 or more", countries, 110, "", 55, 220));
  double largest = 0;
  int million = 0;
  for (const gen::Settlement& t : w.people.towns) largest = std::max(largest, t.people), million += t.people >= 1e6;
  out.push_back(number("Largest town", largest / 1e6, 40, "million", 20, 45));
  out.push_back(number("Towns of a million or more", million, 548, "", 270, 1100));
  return out;
}

}  // namespace zs::atlas
