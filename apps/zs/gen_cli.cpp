// zs gen: the content generators from the command line (docs/design/content-tools.md section 6).
//   zs gen town   [--seed N] [--mix FILE] [--country ID]        town, streets, businesses and every pop's dossier
//   zs gen census [--seed N] [--mix FILE]          the pops as a data/tier0/census.toml-style table
//   zs gen names  --culture ID [--kind K] [--count N] [--seed N]
//                 kinds: person (default), street, town, province, country
//   zs gen houses [--seed N] [--mix FILE] [--country ID]        every household's dwelling: address, size, rooms (spec 51)
//   zs gen house  [--seed N] [--mix FILE] [--country ID] [--household N] [--out FILE]
//                                                  one house with its interior as a site map; prints its plan
//   zs gen stations [--seed N] [--people P] [--profile ISO3]   the town's police agency and its stations (spec 69)
//   zs gen station  [--seed N] [--people P] [--station K] [--out FILE]   one station as a site map
//   zs gen layout [--seed N] [--people P] [--country ID] [--bmp FILE]   the town's streets, blocks and plots (spec 71)
//   zs gen plot [--seed N] [--people P] [--plot N | --address A] [--out FILE]   what stands on a plot (spec 72)
//   zs gen routes [--seed N] [--freq F] [--bmp FILE]   roads, rivers and sea lanes between towns; the block's town's routes (spec 74)
//   zs gen food [--seed N] [--freq F] [--border X]   food made, eaten and sent between towns over the routes (spec 76)
//   zs gen shops [--seed N]                        the block's town's food: shops, distribution centre, pantries (spec 79)
//   zs gen cultures                                list the culture packs
//   zs gen world  [--seed N] [--freq F] [--land S] [--temp C] [--relief R] [--moisture M] [--fragmentation X] [--bmp FILE]
//                                                  world fields, land, deposits and countries: summary and a text map
//                                                  (specs 31, 52); --bmp draws rivers and borders, --political a colour per country
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "zs/gen/dwellings.h"
#include "zs/gen/stations.h"
#include "zs/gen/town_layout.h"
#include "zs/gen/plot_contents.h"
#include "zs/gen/keyed.h"
#include "zs/gen/names.h"
#include "zs/gen/pops.h"
#include "zs/gen/world_fields.h"
#include "zs/gen/world_deposits.h"
#include "zs/gen/world_land.h"
#include "zs/gen/world_people.h"
#include "zs/gen/world_routes.h"
#include "zs/gen/world_food.h"
#include "zs/gen/town_food.h"
#include "zs/gen/town_fuel.h"
#include "zs/gen/town_power.h"
#include "zs/gen/place.h"
#include "zs/tier0.h"
#include "modes.h"
#include "planet_raster.h"
#include "zs/gen/world_shapes.h"
#include "world_numbers.h"

namespace {

std::string arg(int argc, char** argv, const std::string& key, const std::string& fallback) {
  for (int i = 2; i + 1 < argc; ++i)
    if (argv[i] == key) return argv[i + 1];
  return fallback;
}

int usage() {
  std::cerr << "usage: zs gen town|census [--seed N] [--mix FILE] [--country ID]   ID: data/setup/nations/<ID>.toml, e.g. base_low\n"
               "       zs gen names --culture ID [--kind person|street|town|province|country] [--count N] [--seed N]\n"
               "       zs gen houses|house [--seed N] [--mix FILE] [--country ID] [--household N] [--out FILE]   (spec 51)\n"
               "       zs gen stations|station [--seed N] [--people P] [--profile ISO3] [--station K] [--out FILE]   (spec 69)\n"
               "       zs gen layout [--seed N] [--people P] [--country ID] [--bmp FILE]   (spec 71)\n"
               "       zs gen plot [--seed N] [--people P] [--country ID] [--plot N | --address A] [--out FILE]   (spec 72)\n"
               "       zs gen routes [--seed N] [--freq F] [--detour X] [--bmp FILE]   (spec 74)\n"
               "       zs gen food [--seed N] [--freq F] [--border X]   (spec 76)\n"
               "       zs gen cultures\n"
               "       zs gen world [--seed N] [--freq F] [--land S] [--temp C] [--relief R] [--moisture M] [--fragmentation X] [--radius KM] [--fuel X] [--metals X] [--fertility X] [--forest X] [--fish X] [--people X] [--continents N] [--preview] [--bmp FILE [--layer L | --political]] [--layers PREFIX]\n";
  return 2;
}

}  // namespace

// argv[0] is "gen": the command is argv[1], as when this was its own program.
int gen_main(int argc, char** argv) {
  if (argc < 2) return usage();
  const std::string cmd = argv[1];
  const std::string data = zs::tier0::data_dir();
  try {
    const zs::gen::Catalog cat = zs::gen::load_catalog(data + "/gen");
    const std::uint64_t seed = std::strtoull(arg(argc, argv, "--seed", "1").c_str(), nullptr, 10);
    if (cmd == "cultures") {
      for (const auto& c : cat.cultures) std::cout << c.id << "  " << c.label << " (" << c.country << ")\n";
      return 0;
    }
    if (cmd == "world") {
      // Spec 84: the world's settings as flags, generated by the one shared generator.
      zs::gen::WorldSettings ws;
      ws.land_share = std::atof(arg(argc, argv, "--land", "0.29").c_str());
      ws.global_temp_c = std::atof(arg(argc, argv, "--temp", "14").c_str());
      ws.relief = std::atof(arg(argc, argv, "--relief", "1").c_str());
      ws.moisture = std::atof(arg(argc, argv, "--moisture", "1").c_str());
      ws.fragmentation = std::atof(arg(argc, argv, "--fragmentation", "0.5").c_str());
      ws.radius_km = std::atof(arg(argc, argv, "--radius", "6371").c_str());
      ws.fuel_richness = std::atof(arg(argc, argv, "--fuel", "1").c_str());
      ws.metal_richness = std::atof(arg(argc, argv, "--metals", "1").c_str());
      ws.fertility = std::atof(arg(argc, argv, "--fertility", "1").c_str());
      ws.forest = std::atof(arg(argc, argv, "--forest", "1").c_str());
      ws.fish = std::atof(arg(argc, argv, "--fish", "1").c_str());
      ws.people = std::atof(arg(argc, argv, "--people", "1").c_str());
      ws.continents = std::atof(arg(argc, argv, "--continents", "8").c_str());  // spec 89
      const bool preview = std::any_of(argv, argv + argc, [](const char* a) { return std::string(a) == "--preview"; });
      int freq = std::atoi(arg(argc, argv, "--freq", "0").c_str());
      if (preview) freq = zs::gen::world_frequency(ws.radius_km, zs::gen::kPreviewCellKm2);
      const auto t0 = std::chrono::steady_clock::now();
      const zs::gen::GeneratedWorld gw = zs::gen::generate_world(seed, ws, freq);
      const double gen_s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
      const zs::gen::WorldGrid& grid = gw.grid;
      const zs::gen::WorldFields& f = gw.fields;
      if (preview) {
        std::cout << "preview (spec 84): " << grid.size() << " cells in " << gen_s << " s; key numbers against Earth:\n";
        for (const zs::atlas::PreviewNumber& n : zs::atlas::key_numbers(gw))
          std::cout << "  " << n.what << ": " << n.value << " " << n.unit << " (Earth " << n.earth << ", "
                    << (n.realistic ? "realistic" : "sandbox") << ")\n";
      }
      std::cout << "World seed " << seed << ": " << grid.size() << " cells, " << f.plate_count << " plates\n";
      double share[zs::gen::biome_count] = {};
      double land_area = 0;
      for (zs::gen::CellId i = 0; i < grid.size(); ++i) {
        share[static_cast<int>(f.biome[i])] += grid.area_km2(i);
        if (f.land(i)) land_area += grid.area_km2(i);
      }
      double above1000 = 0, depth = 0, sea_area = 0;
      for (zs::gen::CellId i = 0; i < grid.size(); ++i) {
        if (f.elevation_m[i] > 1000) above1000 += grid.area_km2(i);
        if (!f.land(i)) depth += -f.elevation_m[i] * grid.area_km2(i), sea_area += grid.area_km2(i);
      }
      std::cout << "land above 1000 m: " << 100.0 * above1000 / (land_area > 0 ? land_area : 1) << "% of land; mean ocean depth "
                << depth / (sea_area > 0 ? sea_area : 1) << " m\n";
      std::cout << "land " << 100.0 * land_area / grid.sphere_area_km2() << "% of the surface; biomes (% of land):\n";
      {
        // Spec 83 rule 7: beside Earth's share of non-ice land (WMR-1, Dinerstein et al. 2017, grouped to our biomes).
        static const double earth[zs::gen::biome_count] = {0, 0, 8.7, 11.4, 14.5, 7.8, 19.3, 16.7, 18.0, 3.6};
        const double ice = share[static_cast<int>(zs::gen::Biome::Ice)];
        std::cout << "  ice " << 100.0 * ice / land_area << "% of land\n  share of non-ice land (Earth, ratio):\n";
        for (int b = 2; b < zs::gen::biome_count; ++b) {
          const double ours = 100.0 * share[b] / std::max(1.0, land_area - ice);
          std::cout << "  " << zs::gen::biome_name(static_cast<zs::gen::Biome>(b)) << " " << ours << " (" << earth[b] << ", "
                    << ours / earth[b] << ")\n";
        }
      }
      {  // Spec 89: the land's shapes against Earth's (tools/data/earth_shapes.py, CC).
        const zs::gen::LandShapes ls = zs::gen::land_shapes(grid, f);
        std::cout << "shapes (spec 89): largest landmasses (% of land):";
        for (std::size_t i = 0; i < std::min<std::size_t>(6, ls.landmasses_km2.size()); ++i)
          std::cout << " " << 100 * ls.landmasses_km2[i] / ls.land_km2;
        std::cout << " (Earth 53.5 26.1 9.3 5.1 1.5 0.6); landmasses of 1M / 100k / 10k km2 " << ls.over_1m << " / " << ls.over_100k
                  << " / " << ls.over_10k << " (5 / 20 / 62); islands " << 100 * ls.islands << " % (4.4)\n";
        std::cout << "  land within 50 / 100 / 200 / 400 / 800 km of the sea " << 100 * ls.near_sea[0] << " / " << 100 * ls.near_sea[1]
                  << " / " << 100 * ls.near_sea[2] << " / " << 100 * ls.near_sea[3] << " / " << 100 * ls.near_sea[4]
                  << " % (Earth 8.5 / 17.0 / 29.3 / 47.6 / 72.6); median " << ls.median_sea_km << " km (431); thin-strip land "
                  << 100 * ls.thin << " %; beyond the polar circles N " << 100 * ls.arctic << " S " << 100 * ls.antarctic
                  << " % (5.4, 9.2); northern hemisphere " << 100 * ls.north << " % (67.3); sea ice " << 100 * ls.sea_ice << " % of the sea (about 6)\n";
        std::cout << "  land by latitude, -90 to +90 (% of land):";
        for (double x : ls.by_latitude) std::cout << " " << static_cast<int>(100 * x + 0.5);
        std::cout << " (Earth 3 6 1 0 1 3 6 6 7 7 8 10 10 11 10 9 2 0)\n";
      }
      // Spec 52: where the rain goes and what the land can feed.
      const zs::gen::WorldLand& land = gw.land;
      {  // the largest basins: a river's last land cell before the sea (or its closed depression)
        std::vector<std::pair<double, zs::gen::CellId>> outlets;
        for (zs::gen::CellId i = 0; i < grid.size(); ++i) {
          if (!f.land(i)) continue;
          const zs::gen::CellId d = land.downstream[i];
          if (d == zs::gen::kNoCell || !f.land(d)) outlets.push_back({land.catchment_km2[i], i});
        }
        std::sort(outlets.rbegin(), outlets.rend());
        // Each land cell's outlet, for the basins' mean latitude and rain.
        std::vector<zs::gen::CellId> outlet_of(grid.size(), zs::gen::kNoCell);
        for (zs::gen::CellId i = 0; i < grid.size(); ++i) {
          if (!f.land(i) || outlet_of[i] != zs::gen::kNoCell) continue;
          std::vector<zs::gen::CellId> path;
          zs::gen::CellId c = i;
          while (outlet_of[c] == zs::gen::kNoCell) {
            path.push_back(c);
            const zs::gen::CellId d = land.downstream[c];
            if (d == zs::gen::kNoCell || !f.land(d)) break;
            c = d;
          }
          const zs::gen::CellId out = outlet_of[c] != zs::gen::kNoCell ? outlet_of[c] : c;
          for (zs::gen::CellId x : path) outlet_of[x] = out;
        }
        std::map<zs::gen::CellId, std::array<double, 3>> basin;  // area, area x |lat|, area x rain
        for (zs::gen::CellId i = 0; i < grid.size(); ++i) {
          if (!f.land(i)) continue;
          auto& b = basin[outlet_of[i]];
          const double a = grid.area_km2(i);
          b[0] += a, b[1] += a * std::abs(std::asin(grid.sin_lat(i))) * 57.29578, b[2] += a * f.rain_mm[i];
        }
        std::cout << "basins: the largest";
        for (std::size_t k = 0; k < 6 && k < outlets.size(); ++k) {
          const auto& b = basin[outlets[k].second];
          std::cout << (k ? "," : "") << " " << outlets[k].first / 1e6 << " M km2 (" << static_cast<long long>(land.discharge_m3s[outlets[k].second])
                    << " m3/s, " << static_cast<int>(b[1] / b[0]) << " deg, " << static_cast<int>(b[2] / b[0]) << " mm)";
        }
        int over1m = 0, over10k = 0;
        for (const auto& o : outlets) over1m += o.first >= 1e6, over10k += land.discharge_m3s[o.second] >= 10000;

        std::cout << "; " << over1m << " over a million km2 (Earth: Amazon 6.3-7.0 M km2 and 209,000 m3/s, Congo 3.7 and 40,500, "
                  << "Nile 3.3, Mississippi 3.0, Ob 3.0, Yenisei 2.6; about 25 over a million)\n";
        // Rivers: land drained from 100,000 km2 or more, and the share of it carrying 100 m3/s (spec 52 rule 4); the
        // rivers' length (cell to its downstream cell) and the mouths' share of what reaches the sea.
        double land_a = 0, wide = 0, wide_wet = 0, len4 = 0, len_large = 0, to_sea = 0;
        std::vector<double> mouths;
        for (zs::gen::CellId i = 0; i < grid.size(); ++i) {
          if (!f.land(i)) continue;
          land_a += grid.area_km2(i);
          if (land.catchment_km2[i] >= 1e5) wide += grid.area_km2(i), wide_wet += land.discharge_m3s[i] >= 100 ? grid.area_km2(i) : 0.0;
          const zs::gen::CellId d = land.downstream[i];
          if (land.river[i] && d != zs::gen::kNoCell) (land.river[i] == 2 ? len_large : len4) += grid.distance_km(i, d);
          if (d != zs::gen::kNoCell && !f.land(d)) mouths.push_back(land.discharge_m3s[i]), to_sea += land.discharge_m3s[i];
        }
        len4 += len_large;
        std::sort(mouths.rbegin(), mouths.rend());
        double top1 = 0, top10 = 0;
        for (std::size_t k = 0; k < mouths.size() && k < 10; ++k) (k ? top10 : top1) += mouths[k];
        top10 += top1;
        std::cout << "  rivers: land drained from 100,000 km2 or more " << 100 * wide / land_a << " %, of it carrying 100 m3/s "
                  << 100 * wide_wet / std::max(1.0, wide) << " %; river length " << static_cast<long long>(len4) << " km, large "
                  << static_cast<long long>(len_large) << " km (Earth 209,000 and 58,000 at HydroRIVERS' finer detail, HYD-7); mouths of "
                  << "10,000 m3/s or more " << over10k << " (Earth 15); the largest and the ten largest carry " << 100 * top1 / std::max(1.0, to_sea)
                  << " / " << 100 * top10 / std::max(1.0, to_sea) << " % of what reaches the sea (Earth 15.7 / 34.1, HYD-6)\n";
        // Rain and runoff on land, area-weighted: the mean and where the wettest tenth starts.
        std::vector<std::pair<double, double>> rr;  // (rain, area)
        double area = 0, rain = 0, run = 0;
        for (zs::gen::CellId i = 0; i < grid.size(); ++i) {
          if (!f.land(i)) continue;
          const double a = grid.area_km2(i);
          rr.push_back({f.rain_mm[i], a});
          area += a, rain += a * f.rain_mm[i], run += a * land.runoff_mm[i];
        }
        std::sort(rr.begin(), rr.end());
        double acc = 0, p50 = 0, p90 = 0, p99 = 0;
        for (const auto& [r, a] : rr) {
          acc += a;
          if (!p50 && acc >= 0.5 * area) p50 = r;
          if (!p90 && acc >= 0.9 * area) p90 = r;
          if (!p99 && acc >= 0.99 * area) p99 = r;
        }
        std::cout << "  rain on land: mean " << static_cast<int>(rain / area) << " mm (Earth about 790 outside Antarctica, HYD-2), median "
                  << static_cast<int>(p50) << ", wettest tenth from " << static_cast<int>(p90) << ", wettest hundredth from " << static_cast<int>(p99)
                  << "; runoff mean " << static_cast<int>(run / area) << " mm (Earth about 280-320 outside the ice, HYD-1, HYD-8)\n";
        // Runoff by latitude, -90 to +90 in 10-degree bands (Earth from HydroRIVERS, HYD-8).
        std::array<double, 18> band_a{}, band_r{};
        for (zs::gen::CellId i = 0; i < grid.size(); ++i) {
          if (!f.land(i)) continue;
          const int b = std::clamp(static_cast<int>((std::asin(grid.sin_lat(i)) * 57.29578 + 90) / 10), 0, 17);
          band_a[static_cast<std::size_t>(b)] += grid.area_km2(i), band_r[static_cast<std::size_t>(b)] += grid.area_km2(i) * land.runoff_mm[i];
        }
        std::cout << "  runoff by latitude, -90 to +90 (mm):";
        for (std::size_t b = 0; b < 18; ++b) std::cout << " " << (band_a[b] > 0 ? static_cast<int>(band_r[b] / band_a[b]) : 0);
        std::cout << " (Earth 0 0 0 844 556 143 112 331 849 670 252 266 168 196 289 291 174 105, HYD-8)\n";
      }
      int rivers = 0, large = 0, lakes = 0;
      double arable = 0, capacity = 0, largest = 0, lake_area = 0, endorheic = 0, forest = 0;
      for (zs::gen::CellId i = 0; i < grid.size(); ++i) {
        if (!f.land(i)) continue;
        rivers += land.river[i] == 1;
        large += land.river[i] == 2;
        lakes += land.lake[i] != 0;
        if (land.lake[i] == zs::gen::kLakeOpen || land.lake[i] == zs::gen::kLakeTerminal) lake_area += grid.area_km2(i);
        if (land.endorheic[i] || land.arheic[i]) endorheic += grid.area_km2(i);  // drains nowhere: closed or dry
        forest += land.forest_share[i] * grid.area_km2(i);
        arable += land.arable_share[i] * grid.area_km2(i);
        capacity += land.capacity(i);
        largest = std::max(largest, land.discharge_m3s[i]);
      }
      std::cout << "land (spec 52): runoff " << 100.0 * land.runoff_km3 / land.land_rain_km3 << "% of land rain (" << land.runoff_km3
                << " km3/yr); river cells " << rivers << ", large " << large << ", largest " << largest << " m3/s; lake cells " << lakes
                << "; arable " << 100.0 * arable / land_area << "% of land; capacity " << capacity / 1e9 << " billion people\n";
      std::cout << "water (spec 83): lakes " << 100.0 * lake_area / land_area << "% of land (Earth 1.8, WMR-8); "
                << land.open_lakes << " open lakes, " << land.closed_basins << " closed basins; endorheic or arheic "
                << 100.0 * endorheic / land_area << "% of land (Earth 17-18, WMR-10); forest " << 100.0 * forest / land_area
                << "% of land (Earth 31, WMR-13)\n";
      // Spec 52 slice 3: deposits, with their totals.
      const zs::gen::WorldDeposits& deposits = gw.deposits;
      std::cout << "deposits (spec 52):";
      for (int k = 0; k < zs::gen::resource_count; ++k) {
        const auto r = static_cast<zs::gen::Resource>(k);
        int count = 0;
        for (const auto& d : deposits.deposits) count += d.kind == r;
        std::cout << " " << zs::gen::resource_name(r) << " " << count << " (" << deposits.total(r) / 1e9 << " billion "
                  << zs::gen::resource_unit(r) << ");";
      }
      std::cout << "\n";
      // Spec 52 slice 4: countries formed from the land.
      zs::gen::CountryParams cp;
      cp.fragmentation = ws.fragmentation;
      const zs::gen::WorldPeople& people = gw.people;
      {
        std::vector<std::size_t> order(people.countries.size());
        for (std::size_t k = 0; k < order.size(); ++k) order[k] = k;
        std::stable_sort(order.begin(), order.end(),
                         [&](std::size_t a, std::size_t b) { return people.countries[a].area_km2 > people.countries[b].area_km2; });
        std::size_t landlocked = 0, two = 0, million = 0, big89 = 0;
        double top10 = 0, held = 0;
        std::size_t held_cells = 0;
        for (const auto& s : people.formed.countries) held += s.area_km2, held_cells += s.cells.size();
        const double cell = held / static_cast<double>(std::max<std::size_t>(1, held_cells));
        for (std::size_t i = 0; i < order.size(); ++i) {
          const zs::gen::Country& c = people.countries[order[i]];
          landlocked += c.landlocked;
          two += c.area_km2 >= 1.5 * cell;
          million += c.area_km2 >= 1e6;
          big89 += c.area_km2 >= 89000;
          if (i < 10) top10 += c.area_km2;
        }
        std::cout << "countries (spec 52, fragmentation " << cp.fragmentation << "): " << people.countries.size() << " ("
                  << two << " of two cells or more, " << big89 << " of 89,000 km2 or more (Earth 110), " << million << " over 1 million km2, " << landlocked
                  << " landlocked); the ten largest hold " << 100.0 * top10 / std::max(1.0, held) << "% of the land held (ice is no one's); history: "
                  << people.formed.start_polities << " start polities, " << people.formed.events.size() << " events\n";
        if (!people.formed.entities.empty()) {  // spec 91
          int unions = 0, divisions = 0, members = 0;
          for (const auto& e : people.formed.entities)
            unions += e.origin == zs::gen::Origin::Union, divisions += e.origin == zs::gen::Origin::Division;
          for (const auto& a : people.formed.alliances) members += static_cast<int>(a.members.size());
          std::cout << "history (spec 91): " << people.formed.start_polities << " founders, " << unions << " unions, " << divisions
                    << " divisions, " << people.countries.size() << " countries today; " << people.formed.alliances.size()
                    << " alliances of " << members << " countries:";
          for (std::size_t i = 0; i < people.alliance_names.size(); ++i)
            std::cout << (i ? "; " : " ") << people.alliance_names[i] << " (" << people.formed.alliances[i].members.size() << ")";
          std::cout << "\n  born of the history:";
          int shown = 0;
          for (const auto& c : people.countries)
            if (c.origin != "founded" && !c.origin.empty() && shown++ < 8) std::cout << " " << c.name << " (" << c.origin << ");";
          std::cout << "\n";
        }
        for (std::size_t i = 0; i < std::min<std::size_t>(12, order.size()); ++i) {
          const zs::gen::Country& c = people.countries[order[i]];
          std::cout << "  " << (c.name.empty() ? "(unnamed)" : c.name) << ": " << c.area_km2 / 1e6 << " million km2, "
                    << c.people / 1e6 << " million people, " << zs::gen::kIncomeGroupIds[static_cast<std::size_t>(c.income)]
                    << ", endowment " << c.endowment << (c.landlocked ? ", landlocked" : "") << ", capital held "
                    << people.formed.countries[order[i]].capital_held << " of " << people.formed.periods << " periods\n";
        }
      }
      {  // Spec 86 rule 4: where people and towns are, against the real shares (PTP).
        const zs::gen::PlacementReport pr = zs::gen::placement_report(grid, f, land, people);
        std::cout << "people (spec 86): within 100/200/400 km of the sea " << 100 * pr.coast_100 << " / " << 100 * pr.coast_200 << " / "
                  << 100 * pr.coast_400 << " % (Earth 38 / 50 / 67, PTP-1); below 100 m " << 100 * pr.below_100m << " %, below 500 m "
                  << 100 * pr.below_500m << " % (33.5, 78), median height " << pr.median_m << " m (194, PTP-4); in river or lake cells "
                  << 100 * pr.water_people << " % of people on " << 100 * pr.water_land << " % of land; largest country "
                  << 100 * pr.largest_country << " %, two largest " << 100 * pr.two_largest << " %, ten largest " << 100 * pr.ten_largest
                  << " % (18, 35, 57, PTP-7, PTP-11)\n";
        std::cout << "people (spec 86, EM): below 100 / 200 / 500 m " << 100 * pr.below_100m << " / " << 100 * pr.below_200m << " / "
                  << 100 * pr.below_500m << " % (Earth 43.6 / 56.9 / 76.6, EM-2); river cells " << 100 * pr.river_land << " % of land with "
                  << 100 * pr.river_people << " % of people (10.0, 17.8, EM-4), large rivers " << 100 * pr.large_land << " % with "
                  << 100 * pr.large_people << " % (3.0, 8.2, EM-5); half of people on " << 100 * pr.half_cells << " % of land cells, 80 % on "
                  << 100 * pr.cells_80 << " % (3.1, 10.6, EM-7); land cells under 1 a km2 " << 100 * pr.empty_cells
                  << " % (48.5, EM-8); median land cell " << pr.median_cell_density << " a km2 (1.3), the median person's cell "
                  << pr.median_person_density << " (392)\n";
        std::cout << "towns (spec 86): " << pr.million_towns << " of a million or more, " << 100 * pr.million_coastal
                  << " % within 100 km of the sea (Earth 53, PTP-6); sites:";
        for (int k = 0; k < zs::gen::kTownSites; ++k)
          std::cout << " " << zs::gen::town_site_name(static_cast<zs::gen::TownSite>(k)) << " " << pr.sites[static_cast<std::size_t>(k)];
        std::cout << "\n";
      }
      {  // Spec 88 rule 6: the land and sea at work, against EZ.
        const zs::gen::WorldLandUse& u = gw.use;
        int mines = 0, fields_n = 0, offshore = 0;
        for (const zs::gen::WorkSite& s : u.sites) {
          (s.kind == zs::gen::SiteKind::Mine ? mines : fields_n) += 1;
          offshore += s.country < 0;
        }
        std::cout << "land use (spec 88): cropland " << 100 * u.crop_km2 / u.land_km2 << " % of land (Earth 12, EZ-1), pasture "
                  << 100 * u.pasture_km2 / u.land_km2 << " % (25), working forest " << 100 * u.forest_worked_km2 / std::max(1.0, u.forest_km2)
                  << " % of forest (30, EZ-2); fish landed " << u.fish_landed_t / 1e6 << " of " << u.fish_available_t / 1e6
                  << " million t (80, EZ-5); worked: " << mines << " mining districts, " << fields_n << " oil and gas fields ("
                  << offshore << " offshore)\n";
      }
      {  // spec 93: processing plants
        const zs::gen::WorldIndustry& ind = gw.industry;
        const double scale = people.total_people / 8.0e9;
        std::vector<int> count(ind.kinds.size(), 0), at_port(ind.kinds.size(), 0);
        std::vector<double> feed_km(ind.kinds.size(), 0.0);
        std::vector<int> feed_n(ind.kinds.size(), 0);
        for (const auto& pl : ind.plants) {
          const auto k = static_cast<std::size_t>(pl.kind);
          ++count[k];
          const auto site = people.towns[static_cast<std::size_t>(pl.town)].site;
          at_port[k] += site == zs::gen::TownSite::Port || site == zs::gen::TownSite::Mouth;
          if (!pl.feeds.empty()) feed_km[k] += grid.distance_km(pl.cell, gw.use.sites[static_cast<std::size_t>(pl.feeds.front())].cell), ++feed_n[k];
        }
        std::cout << "processing (spec 93): " << ind.plants.size() << " plants:";
        for (std::size_t k = 0; k < ind.kinds.size(); ++k) {
          std::cout << (k ? ";" : "") << " " << ind.kinds[k].label << " " << count[k] << " (Earth's at this many people "
                    << static_cast<long long>(ind.kinds[k].earth_plants * scale + 0.5) << ", at ports " << (count[k] ? 100 * at_port[k] / count[k] : 0) << " %";
          if (feed_n[k]) std::cout << ", input " << static_cast<int>(feed_km[k] / feed_n[k]) << " km away";
          std::cout << ")";
        }
        std::vector<std::size_t> order(ind.of_town.size());
        for (std::size_t t = 0; t < order.size(); ++t) order[t] = t;
        std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return ind.of_town[a].size() > ind.of_town[b].size(); });
        std::cout << "\n  most industrial towns:";
        for (std::size_t i = 0; i < 5 && i < order.size(); ++i) {
          const auto& town = people.towns[order[i]];
          std::cout << " " << ind.of_town[order[i]].size() << " plants in " << people.countries[static_cast<std::size_t>(town.country)].name
                    << "'s town of " << static_cast<int>(town.people / 1000) << " thousand;";
        }
        std::cout << "\n";
      }
      {  // spec 94: power plants and the grid
        const zs::gen::WorldEnergy& en = gw.energy;
        const zs::gen::EnergyParams ep;
        double all = 0;
        for (double x : en.world_twh) all += x;
        std::cout << "energy (spec 94): " << static_cast<long long>(all) << " TWh a year (Earth about 29,900 at 8 billion; ELS-1); mix:";
        for (int q = 0; q < zs::gen::kFuels; ++q)
          std::cout << (q ? "," : "") << " " << zs::gen::fuel_name(static_cast<zs::gen::Fuel>(q)) << " "
                    << (all > 0 ? 100 * en.world_twh[static_cast<std::size_t>(q)] / all : 0) << " % (" << 100 * ep.world_share[static_cast<std::size_t>(q)] << ")";
        std::array<double, zs::gen::kFuels> mw{};
        std::array<int, zs::gen::kFuels> count{};
        for (const auto& pl : en.plants) mw[static_cast<std::size_t>(pl.fuel)] += pl.mw, ++count[static_cast<std::size_t>(pl.fuel)];
        const double scale = people.total_people / 8.0e9;
        std::cout << "\n  plants: coal " << count[0] << " (" << static_cast<int>(mw[0] / 1000) << " GW; Earth about " << static_cast<int>(2175 * scale)
                  << " GW at these people, ELS-3), gas " << count[1] << " (" << static_cast<int>(mw[1] / 1000) << " GW), oil and other " << count[2]
                  << ", hydro dams " << count[3] << " (" << static_cast<int>(mw[3] / 1000) << " GW of " << static_cast<int>(en.hydro_potential_mw / 1000)
                  << " GW of sites; Earth " << static_cast<int>(1443 * scale) << ", ELS-4), nuclear " << count[4] << " (" << static_cast<int>(mw[4] / 1000)
                  << " GW; Earth " << static_cast<int>(377 * scale) << ", ELS-5), wind " << count[5] << ", solar " << count[6];
        double km = 0, inter = 0;
        for (const auto& l : en.lines) (l.country < 0 ? inter : km) += l.km;
        std::cout << "\n  grid: " << static_cast<long long>(km) << " km of high-voltage lines inside countries, " << static_cast<long long>(inter)
                  << " km between them\n";
      }
      const std::string bmp = arg(argc, argv, "--bmp", "");
      // Spec 83 rule 6: the map in layers.
      zs::atlas::MapInputs mi;
      mi.countries = &people.formed;
      mi.deposits = &deposits;
      mi.people = &people;
      mi.use = &gw.use;
      mi.industry = &gw.industry;
      mi.energy = &gw.energy;
      if (arg(argc, argv, "--projection", "") == "equal-earth") mi.projection = zs::atlas::Projection::EqualEarth;  // spec 89 amendment
      if (!bmp.empty()) {
        const bool political = std::any_of(argv, argv + argc, [](const char* a) { return std::string(a) == "--political"; });
        zs::atlas::MapLayer layer = political ? zs::atlas::MapLayer::Political : zs::atlas::MapLayer::Physical;
        if (const std::string name = arg(argc, argv, "--layer", ""); !name.empty() && !zs::atlas::parse_layer(name, layer)) {
          std::cerr << "unknown layer " << name << " (one of:";
          for (int k = 0; k < zs::atlas::kMapLayers; ++k) std::cerr << " " << zs::atlas::layer_name(static_cast<zs::atlas::MapLayer>(k));
          std::cerr << ")\n";
          return 2;
        }
        zs::atlas::write_bmp(bmp, zs::atlas::render_layer(grid, f, land, mi, layer));
        std::cout << "wrote " << bmp << " (" << zs::atlas::layer_legend(layer) << ")\n";
      }
      if (const std::string prefix = arg(argc, argv, "--layers", ""); !prefix.empty())
        for (int k = 0; k < zs::atlas::kMapLayers; ++k) {
          const auto layer = static_cast<zs::atlas::MapLayer>(k);
          const std::string path = prefix + "_" + zs::atlas::layer_name(layer) + ".bmp";
          zs::atlas::write_bmp(path, zs::atlas::render_layer(grid, f, land, mi, layer));
          std::cout << "wrote " << path << " (" << zs::atlas::layer_legend(layer) << ")\n";
        }
      static const char glyph[zs::gen::biome_count] = {' ', '*', ',', 'b', 'f', '"', ':', 's', 'T', '^'};
      for (int row = 0; row < 36; ++row) {
        const double lat = 87.5 - 5.0 * row;
        for (int col = 0; col < 96; ++col) {
          const auto c = grid.cell_at(lat, -180.0 + 3.75 * (col + 0.5));
          std::cout << (f.land(c) ? (land.river[c] == 2 ? '~' : glyph[static_cast<int>(f.biome[c])]) : (f.elevation_m[c] > -1500 ? '.' : ' '));
        }
        std::cout << "\n";
      }
      std::cout << "legend: '~' large river, ' ' deep sea, '.' shelf, '*' ice, ',' tundra, b boreal, f temperate forest, \" grassland,"
                   " ':' desert, s savanna, T tropical forest, ^ mountain\n";
      return 0;
    }
    if (cmd == "fuel") {  // spec 97: the block's town's filling stations and their depot
      const zs::gen::GeneratedWorld gw = zs::gen::generate_world(seed, {});
      const int town = zs::gen::choose_town(gw.fields, gw.people, seed);
      if (town < 0) throw std::runtime_error("no town for the block");
      const zs::Place place = zs::gen::place_of_town(gw.grid, gw.fields, gw.people, town, seed);
      const auto rec = zs::gen::town_record(place, seed);
      int income = 0;
      for (int g = 0; g < zs::gen::kIncomeGroups; ++g)
        if (place.income == zs::gen::kIncomeGroupIds[static_cast<std::size_t>(g)]) income = g;
      const auto fp = zs::gen::load_fuel_retail(data + "/gen");
      const auto depot = zs::gen::fuel_depot(gw.routes, gw.people, gw.industry, town);
      const auto fuel = zs::gen::plan_town_fuel(rec->layout, rec->spec.people, income, fp, seed, depot);
      std::cout << "The fuel of " << rec->spec.name << " (" << rec->spec.people << " people, " << place.income << " income, "
                << place.country_name << "), spec 97: " << fuel.stations.size() << " filling stations; the town's vehicles take "
                << fuel.litres_per_day << " L a day\n";
      for (const auto& s : fuel.stations)
        std::cout << "  - " << s.name << ": tanks " << s.tank_l << " L, holding " << s.stock_l << " L ("
                  << (fuel.litres_per_day > 0 ? s.stock_l / (fuel.litres_per_day * s.share) : 0.0) << " days)"
                  << (s.generator ? ", its own generator" : "") << "\n";
      if (depot.town >= 0) {
        const auto& d = gw.people.towns[static_cast<std::size_t>(depot.town)];
        std::cout << "depot: " << (depot.refinery ? "a refinery" : "a port terminal") << " in a town of " << d.people << " people, "
                  << depot.hours << " h away by the routes\n";
      } else {
        std::cout << "depot: none reachable by the routes\n";
      }
      return 0;
    }
    if (cmd == "power") {  // spec 96: the block's town's grid and its outage; the outage over every town
      const zs::gen::GeneratedWorld gw = zs::gen::generate_world(seed, {});
      const int town = zs::gen::choose_town(gw.fields, gw.people, seed);
      if (town < 0) throw std::runtime_error("no town for the block");
      const auto& t = gw.people.towns[static_cast<std::size_t>(town)];
      const zs::gen::TownFeed feed = zs::gen::town_feed(gw.grid, gw.people, gw.energy, town);
      const zs::gen::OutageTimeline out = zs::gen::outage_timeline(feed, gw.energy, {}, seed);
      std::cout << "The block's town (" << t.people << " people, " << gw.people.countries[static_cast<std::size_t>(t.country)].name
                << "), spec 96: " << zs::gen::describe_feed(feed, out) << "\n";
      std::vector<double> days;
      int causes[3] = {};
      for (std::size_t i = 0; i < gw.people.towns.size(); i += 25) {
        const zs::gen::TownFeed fi = zs::gen::town_feed(gw.grid, gw.people, gw.energy, static_cast<int>(i));
        const zs::gen::OutageTimeline oi = zs::gen::outage_timeline(fi, gw.energy, {}, seed);
        days.push_back(oi.dark_h / 24);
        ++causes[static_cast<int>(oi.cause)];
      }
      std::sort(days.begin(), days.end());
      auto at = [&](double q) { return days.empty() ? 0.0 : days[static_cast<std::size_t>(q * static_cast<double>(days.size() - 1))]; };
      std::cout << "every 25th town (" << days.size() << "): dark on day " << at(0) << " / " << at(0.1) << " / " << at(0.5) << " / "
                << at(0.9) << " / " << at(1) << " (least, tenth, median, ninth tenth, most); causes: plants " << causes[0] << ", lines "
                << causes[1] << ", feeder " << causes[2] << "\n";
      return 0;
    }
    if (cmd == "shops") {  // spec 79: the block's town's food: outlets, distribution centre, household stocks
      const zs::gen::WorldGrid grid(zs::gen::kOneWorldFrequency, 6371.0);
      zs::gen::WorldParams wp;
      wp.seed = seed;
      const auto f = zs::gen::generate_fields(grid, wp);
      const auto land = zs::gen::generate_land(grid, f);
      zs::gen::DepositParams dp;
      dp.seed = seed;
      const auto deposits = zs::gen::generate_deposits(grid, f, dp);
      const std::string tables = zs::tier0::data_dir();
      const auto people = zs::gen::populate_world(grid, f, land, deposits, seed, zs::gen::load_group_tables(tables),
                                                  zs::gen::load_real_profiles(tables));
      const auto routes = zs::gen::build_routes(grid, f, land, people);
      const int town = zs::gen::choose_town(f, people, seed);
      if (town < 0) throw std::runtime_error("no town for the block");
      const zs::Place place = zs::gen::place_of_town(grid, f, people, town, seed);
      const auto rec = zs::gen::town_record(place, seed);
      const auto params = zs::gen::load_food_retail(data + "/gen");
      int income = 0;
      for (int g = 0; g < zs::gen::kIncomeGroups; ++g)
        if (place.income == zs::gen::kIncomeGroupIds[static_cast<std::size_t>(g)]) income = g;
      const auto centre = zs::gen::serving_centre(grid, f, people, routes, town, params);
      const auto food = zs::gen::plan_town_food(rec->town, rec->layout, rec->spec.people, income, params, seed, centre);
      std::cout << "The food of " << rec->spec.name << " (" << rec->spec.people << " people, " << place.income << " income, "
                << place.country_name << "), spec 79\n";
      int counts[zs::gen::kOutletKinds] = {};
      double shares[zs::gen::kOutletKinds] = {};
      for (const auto& o : food.outlets) ++counts[static_cast<int>(o.kind)], shares[static_cast<int>(o.kind)] += o.share;
      for (int k = 0; k < zs::gen::kOutletKinds; ++k)
        std::cout << "  " << counts[k] << " " << zs::gen::outlet_kind_name(static_cast<zs::gen::OutletKind>(k)) << (counts[k] == 1 ? "" : "s")
                  << ", selling " << 100.0 * shares[k] << "% of the town's food\n";
      for (const auto& o : food.outlets)
        if (o.kind != zs::gen::OutletKind::Small)
          std::cout << "  - " << o.name << " (" << zs::gen::outlet_kind_name(o.kind) << ", " << o.floor_m2 << " m2, "
                    << o.share * food.people_days_per_day << " people-days a day, " << o.stock_days << " days of stock)\n";
      if (food.centre.town >= 0)
        std::cout << "distribution centre: in a town of " << people.towns[static_cast<std::size_t>(food.centre.town)].people << " people, "
                  << food.centre.km << " km and " << food.centre.hours << " h away; it holds " << params.dc_dry_days << " days of dry food and "
                  << params.dc_fresh_days << " of fresh\n";
      std::cout << "households: " << food.pantry_days << " days in the pantry, " << food.fridge_days << " in the fridge, "
                << food.trips_per_week << " shopping trips a week\n";
      std::cout << "digest " << zs::gen::town_food_digest(food) << "\n";
      return 0;
    }
    if (cmd == "towns") {  // data review: the planet's towns by size, the largest, primacy, the most populous countries
      const int freq = std::atoi(arg(argc, argv, "--freq", "101").c_str());
      const zs::gen::WorldGrid grid(freq, 6371.0);
      zs::gen::WorldParams wp;
      wp.seed = seed;
      const auto f = zs::gen::generate_fields(grid, wp);
      const auto land = zs::gen::generate_land(grid, f);
      zs::gen::DepositParams dp;
      dp.seed = seed;
      const auto deposits = zs::gen::generate_deposits(grid, f, dp);
      const std::string tables = zs::tier0::data_dir();
      const auto people = zs::gen::populate_world(grid, f, land, deposits, seed, zs::gen::load_group_tables(tables),
                                                  zs::gen::load_real_profiles(tables));
      const double bounds[] = {1e4, 5e4, 1e5, 5e5, 1e6, 5e6, 1e7, 1e12};
      const char* labels[] = {"10k-50k", "50k-100k", "100k-500k", "500k-1M", "1M-5M", "5M-10M", "10M+"};
      std::size_t count[7] = {};
      double urban = 0;
      std::vector<double> country_urban(people.countries.size(), 0);
      for (const auto& t : people.towns) {
        urban += t.people;
        country_urban[static_cast<std::size_t>(t.country)] += t.people;
        for (int b = 0; b < 7; ++b)
          if (t.people >= bounds[b] && t.people < bounds[b + 1]) ++count[b];
      }
      std::cout << "Towns, seed " << seed << ": " << people.towns.size() << " of 10,000 or more, holding " << urban / 1e9 << " billion ("
                << 100.0 * urban / people.total_people << "% of " << people.total_people / 1e9 << " billion)\n  by size:";
      for (int b = 0; b < 7; ++b) std::cout << " " << labels[b] << " " << count[b] << ";";
      std::cout << "\n";
      auto name_of = [&](int k) {
        const auto& c = people.countries[static_cast<std::size_t>(k)];
        return c.name.empty() ? std::string("(unnamed)") : c.name;
      };
      std::vector<std::size_t> order(people.towns.size());
      for (std::size_t i = 0; i < order.size(); ++i) order[i] = i;
      std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return people.towns[a].people > people.towns[b].people; });
      std::cout << "the largest towns:\n";
      for (std::size_t i = 0; i < std::min<std::size_t>(15, order.size()); ++i) {
        const auto& t = people.towns[order[i]];
        std::cout << "  " << t.people / 1e6 << " million, rank " << t.rank << " of " << name_of(t.country) << " ("
                  << people.countries[static_cast<std::size_t>(t.country)].people / 1e6 << " million people; "
                  << 100.0 * t.people / country_urban[static_cast<std::size_t>(t.country)] << "% of its urban)\n";
      }
      // Primacy: the largest town's share of its country's urban people.
      std::vector<double> primacy;
      for (const auto& t : people.towns)
        if (t.rank == 1) primacy.push_back(t.people / country_urban[static_cast<std::size_t>(t.country)]);
      std::sort(primacy.begin(), primacy.end());
      if (!primacy.empty())
        std::cout << "primacy (largest town over its country's urban), " << primacy.size() << " countries: median "
                  << 100.0 * primacy[primacy.size() / 2] << "%, 10th percentile " << 100.0 * primacy[primacy.size() / 10]
                  << "%, 90th " << 100.0 * primacy[primacy.size() * 9 / 10] << "%\n";
      std::vector<std::size_t> co(people.countries.size());
      for (std::size_t i = 0; i < co.size(); ++i) co[i] = i;
      std::stable_sort(co.begin(), co.end(), [&](std::size_t a, std::size_t b) { return people.countries[a].people > people.countries[b].people; });
      std::cout << "the most populous countries:\n";
      for (std::size_t i = 0; i < std::min<std::size_t>(10, co.size()); ++i) {
        const auto& c = people.countries[co[i]];
        std::size_t towns = 0;
        for (const auto& t : people.towns) towns += t.country == static_cast<int>(co[i]);
        std::cout << "  " << name_of(static_cast<int>(co[i])) << ": " << c.people / 1e6 << " million on " << c.area_km2 / 1e6
                  << " million km2, " << zs::gen::kIncomeGroupIds[static_cast<std::size_t>(c.income)] << ", urban "
                  << 100.0 * country_urban[co[i]] / c.people << "%, " << towns << " towns\n";
      }
      return 0;
    }
    if (cmd == "food") {  // spec 76
      const int freq = std::atoi(arg(argc, argv, "--freq", "101").c_str());
      const zs::gen::WorldGrid grid(freq, 6371.0);
      zs::gen::WorldParams wp;
      wp.seed = seed;
      const auto f = zs::gen::generate_fields(grid, wp);
      const auto land = zs::gen::generate_land(grid, f);
      zs::gen::DepositParams dp;
      dp.seed = seed;
      const auto deposits = zs::gen::generate_deposits(grid, f, dp);
      const std::string tables = zs::tier0::data_dir();
      const auto people = zs::gen::populate_world(grid, f, land, deposits, seed, zs::gen::load_group_tables(tables),
                                                  zs::gen::load_real_profiles(tables));
      const auto routes = zs::gen::build_routes(grid, f, land, people);
      const auto use = zs::gen::generate_land_use(grid, f, land, deposits, people);  // spec 88 rule 5
      zs::gen::FreightParams fp;
      fp.border_road_km_per_hour = std::atof(arg(argc, argv, "--border", std::to_string(fp.border_road_km_per_hour)).c_str());
      const auto food = zs::gen::food_flows(land, people, routes, fp, &use);
      std::cout << "Food (spec 76), seed " << seed << ", a border hour " << fp.border_road_km_per_hour << " road-km: production "
                << food.production / 1e9 << " billion people-years, demand " << food.demand / 1e9 << "; traded "
                << 100.0 * food.traded / food.production << "% of production, across borders " << 100.0 * food.international_share()
                << "% (WSR-44: about 23%); unmet " << 100.0 * food.unmet / food.demand << "% of demand; " << food.flows.size()
                << " flows in " << food.rounds << " rounds\n";
      if (const int town = zs::gen::choose_town(f, people, seed); town >= 0) {
        const int node = routes.node_of[people.towns[static_cast<std::size_t>(town)].cell];
        const auto& nf = food.nodes[static_cast<std::size_t>(node)];
        std::cout << "the block's town's region: makes " << nf.made << ", eats " << nf.demand << " (at home " << nf.eaten << ", sent "
                  << nf.sent << ", received " << nf.received << ", unmet " << nf.unmet << ")\n";
        for (const auto& fl : food.flows)
          if (fl.to == node || fl.from == node)
            std::cout << "  " << (fl.to == node ? "from" : "to") << " town cell " << (fl.to == node ? fl.from : fl.to) << ": "
                      << fl.amount << " people-years at " << fl.cost << " road-km" << (fl.international ? " (across a border)" : "")
                      << "\n";
      }
      // Spec 77: by kind.
      const auto diet = zs::gen::food_by_kind(f, land, people, routes, fp, {}, {}, &use);
      std::cout << "by kind (spec 77): across borders " << 100.0 * diet.international_share() << "% of production (one kind: "
                << 100.0 * food.international_share() << "%); traded " << 100.0 * diet.traded / diet.production << "%\n";
      for (int k = 0; k < zs::gen::kFoodKinds; ++k) {
        const auto& kf = diet.kinds[static_cast<std::size_t>(k)];
        std::cout << "  " << zs::gen::food_kind_name(static_cast<zs::gen::FoodKind>(k)) << ": " << 100.0 * kf.demand / diet.demand
                  << "% of the diet, traded " << 100.0 * kf.traded / kf.production << "%, across borders "
                  << 100.0 * kf.international_share() << "%\n";
      }
      std::cout << "digest " << zs::gen::food_digest(food) << ", by kind " << zs::gen::diet_digest(diet) << "\n";
      return 0;
    }
    if (cmd == "routes") {  // spec 74
      const int freq = std::atoi(arg(argc, argv, "--freq", "101").c_str());
      const zs::gen::WorldGrid grid(freq, 6371.0);
      zs::gen::WorldParams wp;
      wp.seed = seed;
      const auto f = zs::gen::generate_fields(grid, wp);
      const auto land = zs::gen::generate_land(grid, f);
      zs::gen::DepositParams dp;
      dp.seed = seed;
      const auto deposits = zs::gen::generate_deposits(grid, f, dp);
      const std::string tables = zs::tier0::data_dir();
      const auto people = zs::gen::populate_world(grid, f, land, deposits, seed, zs::gen::load_group_tables(tables),
                                                  zs::gen::load_real_profiles(tables));
      zs::gen::RouteParams rp;
      rp.loop_detour = std::atof(arg(argc, argv, "--detour", std::to_string(rp.loop_detour)).c_str());
      const auto routes = zs::gen::build_routes(grid, f, land, people, rp);
      std::size_t loops = 0, border = 0;
      double km[zs::gen::kRouteModes] = {};
      for (const auto& r : routes.routes) {
        loops += r.why == zs::gen::RouteWhy::Loop;
        border += r.crosses_border;
        for (int m = 0; m < zs::gen::kRouteModes; ++m) km[m] += r.km[static_cast<std::size_t>(m)];
      }
      std::cout << "Routes (spec 74), seed " << seed << ": " << people.towns.size() << " towns in " << routes.nodes.size() << " cells; "
                << routes.routes.size() << " routes (" << routes.routes.size() - loops << " tree, " << loops << " loops, " << border
                << " across a border); trunk km: road " << km[0] << ", river " << km[1] << ", sea " << km[2] << "\n";
      // The block's town (spec 48 rule 7): its routes and the time to its capital.
      if (const int town = zs::gen::choose_town(f, people, seed); town >= 0) {
        const auto& t = people.towns[static_cast<std::size_t>(town)];
        const auto& c = people.countries[static_cast<std::size_t>(t.country)];
        auto town_name = [&](int k) {
          const auto& s = people.towns[static_cast<std::size_t>(k)];
          return "town " + std::to_string(k) + " (" + std::to_string(static_cast<long long>(s.people)) + " people, rank " +
                 std::to_string(s.rank) + ")";
        };
        std::cout << "the block's town: " << town_name(town) << " in " << (c.name.empty() ? "(unnamed)" : c.name) << "\n";
        const int node = routes.node_of[t.cell];
        for (int ri : routes.of_node[static_cast<std::size_t>(node)]) {
          const auto& r = routes.routes[static_cast<std::size_t>(ri)];
          const int other = r.a == node ? r.b : r.a;
          std::cout << "  to " << town_name(routes.nodes[static_cast<std::size_t>(other)].towns.front()) << ": " << r.hours << " h, road "
                    << r.km[0] << " km, river " << r.km[1] << " km, sea " << r.km[2] << " km" << (r.crosses_border ? ", border" : "")
                    << (r.why == zs::gen::RouteWhy::Loop ? " (loop)" : "") << "\n";
        }
        for (std::size_t k = 0; k < people.towns.size(); ++k)
          if (people.towns[k].country == t.country && people.towns[k].rank == 1)
          {
            double road_km = 0;
            for (int ri : zs::gen::node_route_path(routes, node, routes.node_of[people.towns[k].cell])) {
              const auto& r = routes.routes[static_cast<std::size_t>(ri)];
              road_km += r.km[0] + r.km[1] + r.km[2];
            }
            const double hours = zs::gen::town_hours(grid, f, people, routes, town, static_cast<int>(k));
            std::cout << "  to the capital, " << town_name(static_cast<int>(k)) << ": " << hours << " h over " << road_km
                      << " km (" << grid.distance_km(t.cell, people.towns[k].cell) << " km straight; " << road_km / std::max(1e-9, hours)
                      << " km/h on average)\n";
          }
      }
      std::cout << "loop detour " << rp.loop_detour << ": mean routes per town cell "
                << 2.0 * static_cast<double>(routes.routes.size()) / static_cast<double>(std::max<std::size_t>(1, routes.nodes.size()))
                << "; road circuity (network km over straight km) 150-600 km apart " << zs::gen::road_circuity(grid, routes, 150, 400, seed)
                << ", 500-2000 km " << zs::gen::road_circuity(grid, routes, 500, 200, seed) << "\n";
      std::cout << "digest " << zs::gen::routes_digest(routes) << "\n";
      if (const std::string bmp = arg(argc, argv, "--bmp", ""); !bmp.empty()) {
        auto raster = zs::atlas::render_planet(grid, f, land, &people.formed, false);
        zs::atlas::draw_routes(raster, grid, f, land, routes);
        zs::atlas::write_bmp(bmp, raster);
        std::cout << "wrote " << bmp << "\n";
      }
      return 0;
    }
    if (cmd == "houses" || cmd == "house") {  // spec 51
      auto mix = zs::gen::load_mix(arg(argc, argv, "--mix", data + "/gen/populations/tier0_block.toml"));
      if (const std::string c = arg(argc, argv, "--country", ""); !c.empty()) zs::gen::set_country(mix, c, data);
      auto town = zs::gen::generate_town(cat, mix, seed);
      const auto rules = zs::gen::load_dwelling_rules(data + "/gen");
      const std::string income = zs::gen::income_group(mix);
      const auto dwellings = zs::gen::plan_dwellings(town, rules, income, seed);
      zs::gen::assign_addresses(town, dwellings);
      if (cmd == "houses") {
        std::cout << "Town: " << town.name << " (seed " << seed << "), " << income << " income, " << dwellings.size() << " dwellings\n\n";
        for (const auto& d : dwellings) {
          const auto& hh = town.households[static_cast<std::size_t>(d.household)];
          std::cout << "  " << d.address << ": " << hh.members.size() << (hh.members.size() == 1 ? " person, " : " people, ") << d.floor_area() << " m2 (" << d.width << " x "
                    << d.depth << "), rooms:";
          for (const auto& r : d.rooms) std::cout << " " << r.type << (r.occupants ? "(" + std::to_string(r.occupants) + ")" : "");
          std::cout << "; food " << (d.pantry_kj + d.fridge_kj) / 1000 << " MJ" << (d.fridge ? "" : ", no fridge")
                    << (d.bathroom ? "" : ", no bathroom") << "\n";
        }
        return 0;
      }
      const int h = std::atoi(arg(argc, argv, "--household", "1").c_str()) - 1;
      if (h < 0 || h >= static_cast<int>(dwellings.size())) {
        std::cerr << "household must be 1.." << dwellings.size() << "\n";
        return 2;
      }
      const std::string toml = zs::gen::house_toml(town, dwellings[static_cast<std::size_t>(h)], rules, seed);
      if (const std::string out = arg(argc, argv, "--out", ""); !out.empty()) {
        std::ofstream(out, std::ios::binary) << toml;
        std::cout << "wrote " << out << "\n";
      }
      std::cout << toml;
      return 0;
    }
    if (cmd == "layout" || cmd == "plot") {  // spec 71: the town's streets, blocks and plots; spec 72: what stands on one
      auto mix = zs::gen::load_mix(arg(argc, argv, "--mix", data + "/gen/populations/tier0_block.toml"));
      if (const std::string c = arg(argc, argv, "--country", ""); !c.empty()) zs::gen::set_country(mix, c, data);
      const auto town = zs::gen::generate_town(cat, mix, seed);
      zs::gen::TownSpec spec;
      spec.name = town.name;
      spec.culture = town.culture;
      spec.income = zs::gen::income_group(mix);
      spec.people = std::atof(arg(argc, argv, "--people", "17626").c_str());
      spec.mean_household_size = mix.nation ? zs::mean_household_size(*mix.nation) : 2.5;
      if (mix.nation) spec.household_shares = mix.nation->household_sizes;
      // Its police stations (spec 69) need plots of their own, and so does the Tier 0 block with its census households.
      const zs::AgencyParams ap = zs::load_agency_params(data + "/services/police.toml", "");
      const auto rules = zs::gen::load_dwelling_rules(data + "/gen");
      const auto stations = zs::gen::plan_stations(cat, town, zs::plan_agency(ap, spec.people, 6), ap.shifts,
                                                   zs::gen::load_station_rules(data + "/gen"), rules, seed, 20);
      for (const auto& s : stations) spec.lots.push_back({"police_station", s.name, s.plan.lot_width, s.plan.lot_depth});
      spec.lots.push_back({"tier0_block", "the Tier 0 block", 32, 28, zs::gen::LandUse::Residential, static_cast<int>(town.households.size())});
      const auto layout = zs::gen::lay_out_town(cat, zs::gen::load_urban_form(data + "/gen"), spec, seed);
      if (cmd == "plot") {
        const auto prules = zs::gen::load_plot_rules(data + "/gen");
        int pick = std::atoi(arg(argc, argv, "--plot", "0").c_str()) - 1;
        if (const std::string a = arg(argc, argv, "--address", ""); !a.empty())
          for (std::size_t i = 0; i < layout.plots.size(); ++i)
            if (layout.plots[i].address == a) pick = static_cast<int>(i);
        if (pick < 0) {  // no plot named: one of each kind, to try
          std::map<std::string, int> first;
          std::map<std::string, int> count;
          for (int i = 0; i < static_cast<int>(layout.plots.size()); ++i) {
            const std::string kind = zs::gen::plan_plot(spec, layout, i, rules, prules, seed).kind;
            first.try_emplace(kind, i);
            ++count[kind];
          }
          std::cout << spec.name << ": " << layout.plots.size() << " plots\n";
          for (const auto& [kind, i] : first)
            std::cout << "  " << kind << ": " << count[kind] << ", e.g. plot " << i + 1 << ", " << layout.plots[static_cast<std::size_t>(i)].address << "\n";
          std::cout << "zs gen plot --plot N (or --address \"...\") --out f.toml writes one as a site map\n";
          return 0;
        }
        if (pick >= static_cast<int>(layout.plots.size())) {
          std::cerr << "plot must be 1.." << layout.plots.size() << "\n";
          return 2;
        }
        const auto plan = zs::gen::plan_plot(spec, layout, pick, rules, prules, seed);
        const std::string toml = zs::gen::plot_toml(spec, layout, pick, rules, prules, seed);
        if (toml.empty()) {
          std::cout << layout.plots[static_cast<std::size_t>(pick)].address << ": " << plan.kind
                    << (plan.kind == "block" ? " (data/tier0/map/block.toml)" : " (zs gen station writes it)") << "\n";
          return 0;
        }
        if (const std::string out = arg(argc, argv, "--out", ""); !out.empty()) {
          std::ofstream(out, std::ios::binary) << toml;
          std::cout << "wrote " << out << "\n";
        }
        if (const std::string bmp = arg(argc, argv, "--bmp", ""); !bmp.empty()) {  // spec 92: the plot seen from above
          const zs::atlas::PlanetRaster img = zs::atlas::site_picture(toml, 8);
          zs::atlas::write_bmp(bmp, img);
          std::cout << "wrote " << bmp << " (" << img.w << " x " << img.h << " px, 8 a cell)\n";
          return 0;
        }
        std::cout << toml;
        return 0;
      }
      std::cout << zs::gen::layout_summary(spec, layout);
      if (const std::string bmp = arg(argc, argv, "--bmp", ""); !bmp.empty()) {
        // 2 m a pixel: streets grey, then each plot by its use, with its outline darker (spec 92 rule 5: one picture code).
        const zs::atlas::PlanetRaster img = zs::atlas::town_picture(layout).img;
        zs::atlas::write_bmp(bmp, img);
        std::cout << "wrote " << bmp << " (" << img.w << " x " << img.h << " px, 2 m a pixel)\n";
      }
      return 0;
    }
    if (cmd == "stations" || cmd == "station") {  // spec 69: the town's police agency and its stations
      auto mix = zs::gen::load_mix(arg(argc, argv, "--mix", data + "/gen/populations/tier0_block.toml"));
      if (const std::string c = arg(argc, argv, "--country", ""); !c.empty()) zs::gen::set_country(mix, c, data);
      const auto town = zs::gen::generate_town(cat, mix, seed);
      const double people = std::atof(arg(argc, argv, "--people", "50000").c_str());  // the town the agency serves
      const zs::AgencyParams ap = zs::load_agency_params(data + "/services/police.toml", arg(argc, argv, "--profile", ""));
      const zs::AgencyPlan agency = zs::plan_agency(ap, people, 6);
      const auto rules = zs::gen::load_dwelling_rules(data + "/gen");
      const auto stations = zs::gen::plan_stations(cat, town, agency, ap.shifts, zs::gen::load_station_rules(data + "/gen"), rules, seed);
      if (cmd == "stations") {
        std::cout << "Police of " << town.name << " (seed " << seed << "), serving " << people << " people: " << agency.sworn << " sworn ("
                  << agency.per_1000 << " per 1,000; " << agency.investigators << " investigators, " << agency.patrol << " patrol, "
                  << agency.women << " women), " << agency.civilians << " civilians, " << agency.stations
                  << (agency.stations == 1 ? " station, " : " stations, ") << agency.cells << " holding cells\n\n";
        for (const auto& s : stations) {
          std::cout << "  " << s.name << ", " << s.address << ": " << s.sworn << " sworn, " << s.civilians << " civilians, "
                    << s.plan.floor_area() << " m2 (" << s.plan.width << " x " << s.plan.depth << "), rooms:";
          for (const auto& r : s.plan.rooms) std::cout << " " << r.type << (r.occupants ? "(" + std::to_string(r.occupants) + ")" : "");
          std::cout << "\n";
        }
        return 0;
      }
      const int k = std::atoi(arg(argc, argv, "--station", "1").c_str()) - 1;
      if (k < 0 || k >= static_cast<int>(stations.size())) {
        std::cerr << "station must be 1.." << stations.size() << "\n";
        return 2;
      }
      const std::string toml = zs::gen::station_toml(town, stations[static_cast<std::size_t>(k)], rules, seed);
      if (const std::string out = arg(argc, argv, "--out", ""); !out.empty()) {
        std::ofstream(out, std::ios::binary) << toml;
        std::cout << "wrote " << out << "\n";
      }
      std::cout << toml;
      return 0;
    }
    if (cmd == "town" || cmd == "census") {
      auto mix = zs::gen::load_mix(arg(argc, argv, "--mix", data + "/gen/populations/tier0_block.toml"));
      if (const std::string c = arg(argc, argv, "--country", ""); !c.empty()) zs::gen::set_country(mix, c, data);  // spec 47
      const auto town = zs::gen::generate_town(cat, mix, seed);
      if (cmd == "census") {
        std::cout << zs::gen::census_toml(town);
        return 0;
      }
      std::cout << "Town: " << town.name << " (seed " << seed << ")\nStreets:";
      for (const auto& s : town.streets) std::cout << " " << s << ";";
      std::cout << "\nBusinesses:\n";
      for (const auto& b : town.businesses) std::cout << "  " << b.type << ": " << b.name << "\n";
      std::cout << "\n" << town.households.size() << " households, " << town.pops.size() << " people\n\n";
      for (const auto& p : town.pops) std::cout << zs::gen::dossier(town, p, cat.life.year);
      return 0;
    }
    if (cmd == "names") {
      const auto& c = cat.culture(arg(argc, argv, "--culture", "american"));
      const std::string kind = arg(argc, argv, "--kind", "person");
      const int count = std::atoi(arg(argc, argv, "--count", "10").c_str());
      const zs::gen::Keyed r(seed);
      for (int i = 0; i < count; ++i) {
        const std::uint64_t id = zs::gen::mix64(zs::gen::hash_str(c.id + kind), static_cast<std::uint64_t>(i));
        if (kind == "person") {
          const char sex = i % 2 ? 'f' : 'm';
          std::cout << zs::gen::full_name(c, zs::gen::person_name(c, r, id, sex), sex) << "\n";
        } else if (kind == "street") {
          std::cout << zs::gen::street_name(c, r, id) << "\n";
        } else {
          const auto k = kind == "town" ? zs::gen::PlaceKind::Town
                         : kind == "province" ? zs::gen::PlaceKind::Province
                                              : zs::gen::PlaceKind::Country;
          std::cout << zs::gen::place_name(c, r, id, k) << "\n";
        }
      }
      return 0;
    }
  } catch (const std::exception& e) {
    std::cerr << "zs gen: " << e.what() << "\n";
    return 1;
  }
  return usage();
}
