// zs gen: the content generators from the command line (docs/design/content-tools.md section 6).
//   zs gen town   [--seed N] [--mix FILE]          town, streets, businesses and every pop's dossier
//   zs gen census [--seed N] [--mix FILE]          the pops as a data/tier0/census.toml-style table
//   zs gen names  --culture ID [--kind K] [--count N] [--seed N]
//                 kinds: person (default), street, town, province, country
//   zs gen cultures                                list the culture packs
//   zs gen world  [--seed N] [--freq F] [--land S] [--temp C] [--relief R] [--moisture M]
//                                                  world fields: summary and a text map (spec 31)
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "zs/gen/names.h"
#include "zs/gen/pops.h"
#include "zs/gen/world_fields.h"
#include "modes.h"

namespace {

std::string arg(int argc, char** argv, const std::string& key, const std::string& fallback) {
  for (int i = 2; i + 1 < argc; ++i)
    if (argv[i] == key) return argv[i + 1];
  return fallback;
}

int usage() {
  std::cerr << "usage: zs gen town|census [--seed N] [--mix FILE]\n"
               "       zs gen names --culture ID [--kind person|street|town|province|country] [--count N] [--seed N]\n"
               "       zs gen cultures\n"
               "       zs gen world [--seed N] [--freq F] [--land S] [--temp C] [--relief R] [--moisture M]\n";
  return 2;
}

}  // namespace

// argv[0] is "gen": the command is argv[1], as when this was its own program.
int gen_main(int argc, char** argv) {
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
      const std::string bmp = arg(argc, argv, "--bmp", "");
      if (!bmp.empty()) {
        // 2:1 equirectangular map, 24-bit BMP: biome colour shaded by height or depth.
        static const unsigned char rgb[zs::gen::biome_count][3] = {
            {30, 70, 140}, {240, 245, 250}, {170, 180, 150}, {60, 110, 80},  {40, 140, 60},
            {150, 190, 90}, {225, 200, 130}, {190, 180, 70}, {20, 100, 40},  {130, 120, 110}};
        const int W = 1024, H = 512;
        std::vector<unsigned char> px(static_cast<std::size_t>(W) * H * 3);
        zs::gen::CellId hint = 0;
        for (int y = 0; y < H; ++y)
          for (int x = 0; x < W; ++x) {
            const double lat = 90.0 - 180.0 * (y + 0.5) / H, lon = -180.0 + 360.0 * (x + 0.5) / W;
            const auto c = grid.cell_at(lat, lon);
            hint = c;
            double shade = 1.0;
            unsigned char col[3];
            const auto b = static_cast<int>(f.biome[c]);
            for (int k = 0; k < 3; ++k) col[k] = rgb[b][k];
            if (f.land(c)) shade = 0.85 + 0.15 * std::min(1.0, f.elevation_m[c] / 4000.0);
            else shade = 0.55 + 0.45 * std::max(0.0, 1.0 + f.elevation_m[c] / 6000.0);
            for (int k = 0; k < 3; ++k)
              px[(static_cast<std::size_t>(y) * W + x) * 3 + k] = static_cast<unsigned char>(col[k] * shade);
          }
        (void)hint;
        std::ofstream o(bmp, std::ios::binary);
        auto w32 = [&](std::uint32_t v) { for (int i = 0; i < 4; ++i) o.put(static_cast<char>((v >> (8 * i)) & 255)); };
        auto w16 = [&](std::uint16_t v) { o.put(static_cast<char>(v & 255)); o.put(static_cast<char>(v >> 8)); };
        o.write("BM", 2); w32(54 + W * H * 3); w32(0); w32(54); w32(40); w32(W); w32(H); w16(1); w16(24);
        w32(0); w32(W * H * 3); w32(2835); w32(2835); w32(0); w32(0);
        for (int y = H - 1; y >= 0; --y)
          for (int x = 0; x < W; ++x) {
            const auto* p3 = &px[(static_cast<std::size_t>(y) * W + x) * 3];
            o.put(static_cast<char>(p3[2])); o.put(static_cast<char>(p3[1])); o.put(static_cast<char>(p3[0]));
          }
        std::cout << "wrote " << bmp << "\n";
      }
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
    std::cerr << "zs gen: " << e.what() << "\n";
    return 1;
  }
  return usage();
}
