// zs_gen: the content generators from the command line (docs/design/content-tools.md section 6).
//   zs_gen town   [--seed N] [--mix FILE]          town, streets, businesses and every pop's dossier
//   zs_gen census [--seed N] [--mix FILE]          the pops as a data/tier0/census.toml-style table
//   zs_gen names  --culture ID [--kind K] [--count N] [--seed N]
//                 kinds: person (default), street, town, province, country
//   zs_gen cultures                                list the culture packs
#include <cstdlib>
#include <iostream>
#include <string>

#include "zs/gen/names.h"
#include "zs/gen/pops.h"

namespace {

std::string arg(int argc, char** argv, const std::string& key, const std::string& fallback) {
  for (int i = 2; i + 1 < argc; ++i)
    if (argv[i] == key) return argv[i + 1];
  return fallback;
}

int usage() {
  std::cerr << "usage: zs_gen town|census [--seed N] [--mix FILE]\n"
               "       zs_gen names --culture ID [--kind person|street|town|province|country] [--count N] [--seed N]\n"
               "       zs_gen cultures\n";
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
