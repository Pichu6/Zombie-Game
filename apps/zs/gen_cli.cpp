// zs gen: the content generators from the command line (docs/design/content-tools.md section 6).
//   zs gen town   [--seed N] [--mix FILE] [--country ID]        town, streets, businesses and every pop's dossier
//   zs gen census [--seed N] [--mix FILE]          the pops as a data/tier0/census.toml-style table
//   zs gen names  --culture ID [--kind K] [--count N] [--seed N]
//                 kinds: person (default), street, town, province, country
//   zs gen houses [--seed N] [--mix FILE] [--country ID]        every household's dwelling: address, size, rooms (spec 51)
//   zs gen house  [--seed N] [--mix FILE] [--country ID] [--household N] [--out FILE]
//                                                  one house with its interior as a site map; prints its plan
//   zs gen cultures                                list the culture packs
//   zs gen world  [--seed N] [--freq F] [--land S] [--temp C] [--relief R] [--moisture M] [--fragmentation X] [--bmp FILE]
//                                                  world fields, land, deposits and countries: summary and a text map
//                                                  (specs 31, 52); --bmp draws rivers and borders, --political a colour per country
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "zs/gen/dwellings.h"
#include "zs/gen/keyed.h"
#include "zs/gen/names.h"
#include "zs/gen/pops.h"
#include "zs/gen/world_fields.h"
#include "zs/gen/world_deposits.h"
#include "zs/gen/world_land.h"
#include "zs/gen/world_people.h"
#include "zs/tier0.h"
#include "modes.h"

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
               "       zs gen cultures\n"
               "       zs gen world [--seed N] [--freq F] [--land S] [--temp C] [--relief R] [--moisture M] [--fragmentation X] [--bmp FILE [--political]]\n";
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
      const int freq = std::atoi(arg(argc, argv, "--freq", "101").c_str());
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
      // Spec 52: where the rain goes and what the land can feed.
      const zs::gen::WorldLand land = zs::gen::generate_land(grid, f);
      int rivers = 0, large = 0, lakes = 0;
      double arable = 0, capacity = 0, largest = 0;
      for (zs::gen::CellId i = 0; i < grid.size(); ++i) {
        if (!f.land(i)) continue;
        rivers += land.river[i] == 1;
        large += land.river[i] == 2;
        lakes += land.lake[i];
        arable += land.arable_share[i] * grid.area_km2(i);
        capacity += land.capacity(i);
        largest = std::max(largest, land.discharge_m3s[i]);
      }
      std::cout << "land (spec 52): runoff " << 100.0 * land.runoff_km3 / land.land_rain_km3 << "% of land rain (" << land.runoff_km3
                << " km3/yr); river cells " << rivers << ", large " << large << ", largest " << largest << " m3/s; lake cells " << lakes
                << "; arable " << 100.0 * arable / land_area << "% of land; capacity " << capacity / 1e9 << " billion people\n";
      // Spec 52 slice 3: deposits, with their totals.
      zs::gen::DepositParams dp;
      dp.seed = seed;
      const zs::gen::WorldDeposits deposits = zs::gen::generate_deposits(grid, f, dp);
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
      cp.fragmentation = std::atof(arg(argc, argv, "--fragmentation", "0.5").c_str());
      const std::string data = zs::tier0::data_dir();
      const zs::gen::WorldPeople people = zs::gen::populate_world(grid, f, land, deposits, seed, zs::gen::load_group_tables(data),
                                                                   zs::gen::load_real_profiles(data), cp);
      {
        std::vector<std::size_t> order(people.countries.size());
        for (std::size_t k = 0; k < order.size(); ++k) order[k] = k;
        std::stable_sort(order.begin(), order.end(),
                         [&](std::size_t a, std::size_t b) { return people.countries[a].area_km2 > people.countries[b].area_km2; });
        std::size_t landlocked = 0, two = 0, million = 0;
        double top10 = 0, held = 0;
        std::size_t held_cells = 0;
        for (const auto& s : people.formed.countries) held += s.area_km2, held_cells += s.cells.size();
        const double cell = held / static_cast<double>(std::max<std::size_t>(1, held_cells));
        for (std::size_t i = 0; i < order.size(); ++i) {
          const zs::gen::Country& c = people.countries[order[i]];
          landlocked += c.landlocked;
          two += c.area_km2 >= 1.5 * cell;
          million += c.area_km2 >= 1e6;
          if (i < 10) top10 += c.area_km2;
        }
        std::cout << "countries (spec 52, fragmentation " << cp.fragmentation << "): " << people.countries.size() << " ("
                  << two << " of two cells or more, " << million << " over 1 million km2, " << landlocked
                  << " landlocked); the ten largest hold " << 100.0 * top10 / std::max(1.0, held) << "% of the land held (ice is no one's); history: "
                  << people.formed.start_polities << " start polities, " << people.formed.events.size() << " events\n";
        for (std::size_t i = 0; i < std::min<std::size_t>(12, order.size()); ++i) {
          const zs::gen::Country& c = people.countries[order[i]];
          std::cout << "  " << (c.name.empty() ? "(unnamed)" : c.name) << ": " << c.area_km2 / 1e6 << " million km2, "
                    << c.people / 1e6 << " million people, " << zs::gen::kIncomeGroupIds[static_cast<std::size_t>(c.income)]
                    << ", endowment " << c.endowment << (c.landlocked ? ", landlocked" : "") << ", capital held "
                    << people.formed.countries[order[i]].capital_held << " of " << people.formed.periods << " periods\n";
        }
      }
      const std::string bmp = arg(argc, argv, "--bmp", "");
      if (!bmp.empty()) {
        // 2:1 equirectangular map, 24-bit BMP: biome colour shaded by height or depth.
        static const unsigned char rgb[zs::gen::biome_count][3] = {
            {30, 70, 140}, {240, 245, 250}, {170, 180, 150}, {60, 110, 80},  {40, 140, 60},
            {150, 190, 90}, {225, 200, 130}, {190, 180, 70}, {20, 100, 40},  {130, 120, 110}};
        const int W = 2048, H = 1024;
        std::vector<unsigned char> px(static_cast<std::size_t>(W) * H * 3);
        std::vector<zs::gen::CellId> at(static_cast<std::size_t>(W) * H);
        for (int y = 0; y < H; ++y)
          for (int x = 0; x < W; ++x)
            at[static_cast<std::size_t>(y) * W + x] = grid.cell_at(90.0 - 180.0 * (y + 0.5) / H, -180.0 + 360.0 * (x + 0.5) / W);
        const bool political = std::any_of(argv, argv + argc, [](const char* a) { return std::string(a) == "--political"; });
        auto country_at = [&](int x, int y) { return people.formed.country_of[at[static_cast<std::size_t>(y) * W + ((x + W) % W)]]; };
        for (int y = 0; y < H; ++y)
          for (int x = 0; x < W; ++x) {
            const auto c = at[static_cast<std::size_t>(y) * W + x];
            // A border (spec 52 slice 4): land whose country differs from the land to its right or below.
            const int k = country_at(x, y);
            const bool border = k >= 0 && ((country_at(x + 1, y) >= 0 && country_at(x + 1, y) != k) ||
                                           (y + 1 < H && country_at(x, y + 1) >= 0 && country_at(x, y + 1) != k));
            double shade = 1.0;
            unsigned char col[3];
            const auto b = static_cast<int>(f.biome[c]);
            for (int k = 0; k < 3; ++k) col[k] = rgb[b][k];
            if (f.land(c)) shade = 0.85 + 0.15 * std::min(1.0, f.elevation_m[c] / 4000.0);
            else shade = 0.55 + 0.45 * std::max(0.0, 1.0 + f.elevation_m[c] / 6000.0);
            if (f.land(c) && (land.river[c] || land.lake[c])) {  // rivers and lakes in blue (spec 52)
              const bool big = land.river[c] == 2;
              col[0] = big ? 0 : 110;
              col[1] = big ? 150 : 215;
              col[2] = big ? 255 : 255;
            }
            if (political && k >= 0) {  // one colour per country, keyed by its index
              const std::uint64_t h = zs::gen::mix64(static_cast<std::uint64_t>(k) + 1);
              for (int q = 0; q < 3; ++q) col[q] = static_cast<unsigned char>(90 + ((h >> (8 * q)) & 127));
              shade = 1.0;
            }
            if (border) col[0] = col[1] = col[2] = 40, shade = 1.0;
            for (int k = 0; k < 3; ++k)
              px[(static_cast<std::size_t>(y) * W + x) * 3 + k] = static_cast<unsigned char>(col[k] * shade);
          }
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
          std::cout << (f.land(c) ? (land.river[c] == 2 ? '~' : glyph[static_cast<int>(f.biome[c])]) : (f.elevation_m[c] > -1500 ? '.' : ' '));
        }
        std::cout << "\n";
      }
      std::cout << "legend: '~' large river, ' ' deep sea, '.' shelf, '*' ice, ',' tundra, b boreal, f temperate forest, \" grassland,"
                   " ':' desert, s savanna, T tropical forest, ^ mountain\n";
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
