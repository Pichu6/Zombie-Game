// Spec 71: a town's streets, blocks and plots (decision 0077 item 3.2; research city-layout-generation, CL). From the town's
// record (people, households, income group, culture, its police stations) a land budget, a grid of named streets spaced by
// the region's intersection density, districts (a commercial centre, residential rings, industry at the edge, parks),
// blocks cut into plots that each front a street, and every household and station given a plot and an address. All
// geometry is in whole metres; a pure function of its inputs and a seed.
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "zs/gen/catalog.h"

namespace zs::gen {

struct UrbanGroup {
  double density_per_ha = 100, intersections_per_km2 = 100, frontage_m = 12;
};
struct UrbanForm {
  double street_share = 0.30, open_share = 0.15, commercial_share = 0.10, industrial_share = 0.06, vacancy = 0.05;
  int arterial_every = 3;
  double arterial_width_factor = 1.5;
  int max_units_per_plot = 12;
  double commercial_frontage_m = 15, industrial_frontage_m = 40;
  std::map<std::string, UrbanGroup> groups;  // by income group
};
// data/gen/urban_form.toml from `gen_dir`.
UrbanForm load_urban_form(const std::string& gen_dir);

// What the layout must hold (the town's record).
struct TownSpec {
  std::string name, culture, income = "upper_middle";
  double people = 0;
  double mean_household_size = 3;
  struct Lot {  // a building that needs its own plot (spec 69: a police station's lot)
    std::string kind, name;
    int width_m = 0, depth_m = 0;
  };
  std::vector<Lot> lots;
};

enum class LandUse : std::uint8_t { Residential = 0, Commercial = 1, Industrial = 2, Park = 3, Civic = 4 };
const char* land_use_name(LandUse u);

struct Street {
  std::string name;
  bool vertical = false;  // runs north-south
  int at_m = 0;           // its centre line (x for a vertical street, y for a horizontal one)
  int width_m = 0;        // its reserve
  bool arterial = false;
};

struct Block {
  int x = 0, y = 0, w = 0, h = 0;  // its land inside the street reserves (metres)
  int bx = 0, by = 0;              // its place in the grid
  LandUse use = LandUse::Residential;
};

struct Plot {
  int block = 0;
  int x = 0, y = 0, w = 0, h = 0;
  LandUse use = LandUse::Residential;
  int street = 0;       // the street it fronts
  int number = 0;       // its number on that street
  std::string address;
  int units = 0;        // dwelling units (residential: 1 a house, more an apartment house)
  int households = 0;   // units taken by the town's households
  int lot = -1;         // the TownSpec lot it holds (a station), -1 if none
};

struct TownLayout {
  int cell_m = 0;             // street spacing, centre to centre
  int width_m = 0, height_m = 0;
  std::vector<Street> streets;
  std::vector<Block> blocks;
  std::vector<Plot> plots;
  int households = 0;         // the record's
  std::vector<int> lot_plot;  // per TownSpec lot, its plot (-1 if it found none)
  // Measures for the audit (CL bands).
  double built_up_ha() const { return width_m * static_cast<double>(height_m) / 10000.0; }
  double street_share() const;
  double intersections_per_km2() const;
  double street_km() const;
};

// Rules 1-7.
TownLayout lay_out_town(const Catalog& cat, const UrbanForm& form, const TownSpec& spec, std::uint64_t seed);
// A few lines for zs gen layout.
std::string layout_summary(const TownSpec& spec, const TownLayout& t);

}  // namespace zs::gen
