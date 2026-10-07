// zs_gen: the content generators from the command line (docs/design/content-tools.md section 6).
//   zs_gen town   [--seed N] [--mix FILE]          town, streets, businesses and every pop's dossier
//   zs_gen census [--seed N] [--mix FILE]          the pops as a data/tier0/census.toml-style table
//   zs_gen names  --culture ID [--kind K] [--count N] [--seed N]
//                 kinds: person (default), street, town, province, country
//   zs_gen cultures                                list the culture packs
//   zs_gen world  [--seed N] [--freq F] [--land S] [--temp C] [--relief R] [--moisture M]
//                                                  world fields: summary and a text map (spec 31)
#include <cstdlib>
#include <iostream>
#include <string>

#include "zs/gen/names.h"
#include "zs/gen/pops.h"
#include "zs/gen/world_fields.h"

namespace {

std::string arg(int argc, char** argv, const std::string& key, const std::string& fallback) {
  for (int i = 2; i + 1 < argc; ++i)
    if (argv[i] == key) return argv[i + 1];
  return fallback;
}

int usage() {
  std::cerr << "usage: zs_gen town|census [--seed N] [--mix FILE]\n"
               "       zs_gen names --culture ID [--kind person|street|town|province|country] [--count N] [--seed N]\n"
               "       zs_gen cultures\n"
               "       zs_gen world [--seed N] [--freq F] [--land S] [--temp C] [--relief R] [--moisture M]\n";
  return 2;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) return usage();
  const std::string cmd = argv[1];
  const std::string data = ZS_DATA_DIR;
  try {
    const zs::gen::Catalog cat = zs::gen::load_catalog(data + "/gen");
    const std::uint64_t seed = std::strtoull(arg(argc, argv, "--seed", "1").c_str(), nullptr, 10);
    if (cmd == "cultures") {
      for (const auto& c : cat.cultures) std::cout << c.id << "  " << c.label << " (" << c.country << ")\n";
      return 0;
    }
    if (cmd == "world") {
      const int freq = std::atoi(arg(argc, argv, "--freq", "24").c_str());
      const zs::gen::WorldGrid grid(freq, 6371.0);
      zs::gen::WorldParams wp;
      wp.seed = seed;
      wp.land_share = std::atof(arg(argc, argv, "--land", "0.29").c_str());
      wp.global_temp_c = std::atof(arg(argc, argv, "--temp", "14").c_str());
      wp.relief = std::atof(arg(argc, argv, "--relief", "1").c_str());
      wp.moisture = std::atof(arg(argc, argv, "--moisture", "1").c_str());
      const auto f = zs::gen::generate_fields(grid, wp);
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
      for (int b = 1; b < zs::gen::biome_count; ++b)
        std::cout << "  " << zs::gen::biome_name(static_cast<zs::gen::Biome>(b)) << " " << 100.0 * share[b] / land_area << "\n";
      static const char glyph[zs::gen::biome_count] = {' ', '*', ',', 'b', 'f', '"', ':', 's', 'T', '^'};
      for (int row = 0; row < 36; ++row) {
        const double lat = 87.5 - 5.0 * row;
        for (int col = 0; col < 96; ++col) {
          const auto c = grid.cell_at(lat, -180.0 + 3.75 * (col + 0.5));
          std::cout << (f.land(c) ? glyph[static_cast<int>(f.biome[c])] : (f.elevation_m[c] > -1500 ? '.' : ' '));
        }
        std::cout << "\n";
      }
      std::cout << "legend: ' ' deep sea, '.' shelf, '*' ice, ',' tundra, b boreal, f temperate forest, \" grassland,"
                   " ':' desert, s savanna, T tropical forest, ^ mountain\n";
      return 0;
    }
    if (cmd == "town" || cmd == "census") {
      const auto mix = zs::gen::load_mix(arg(argc, argv, "--mix", data + "/gen/populations/tier0_block.toml"));
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
    std::cerr << "zs_gen: " << e.what() << "\n";
    return 1;
  }
  return usage();
}
