// Spec 51: one dwelling per household with a real address, floor area from members and income, and a house plan
// with its interior that loads as a site map, every free cell reachable, beds for everyone and the household's food.
#include <doctest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <set>

#include "zs/gen/dwellings.h"
#include "zs/gen/keyed.h"
#include "zs/site_map.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

const Catalog& catalog() {
  static const Catalog cat = load_catalog(std::string(ZS_DATA_DIR) + "/gen");
  return cat;
}

const DwellingRules& rules() {
  static const DwellingRules r = load_dwelling_rules(std::string(ZS_DATA_DIR) + "/gen");
  return r;
}

Town town_of(int people, std::uint64_t seed, const std::string& country = "") {
  Mix mix = load_mix(std::string(ZS_DATA_DIR) + "/gen/populations/tier0_block.toml");
  mix.people = people;
  if (!country.empty()) set_country(mix, country, ZS_DATA_DIR);
  return generate_town(catalog(), mix, seed);
}

// A generated house, loaded as the simulation loads a map.
struct Loaded {
  zs::World world{1};
  zs::Assemblies* items;
  zs::SiteMap* map;
  explicit Loaded(const std::string& toml_text) {
    auto a = std::make_unique<zs::Assemblies>();
    auto m = std::make_unique<zs::SiteMap>();
    items = a.get();
    map = m.get();
    world.add_system(std::move(a));
    world.add_system(std::move(m));
    zs::load_item_defs(*items, std::string(ZS_DATA_DIR) + "/tier0/items.toml");
    const std::string path = (std::filesystem::temp_directory_path() / "zs_spec51_house.toml").string();
    {
      std::ofstream(path, std::ios::binary) << toml_text;
    }
    map->load_file(world, *items, path);
    world.close_setup();
  }
};

int count_of(const zs::SiteMap& m, const std::string& kind) {
  return static_cast<int>(std::count_if(m.objects().begin(), m.objects().end(), [&](const zs::MapObject& o) { return o.kind == kind; }));
}

}  // namespace

TEST_CASE("every household gets one dwelling with its own address, written back into the household (A1)") {
  Town town = town_of(300, 3);
  const auto d = plan_dwellings(town, rules(), "upper_middle", 3);
  REQUIRE(d.size() == town.households.size());
  std::set<std::string> addresses;
  for (const Dwelling& w : d) addresses.insert(w.address);
  CHECK(addresses.size() == d.size());
  assign_addresses(town, d);
  for (const Dwelling& w : d) CHECK(town.households[static_cast<std::size_t>(w.household)].address == w.address);
}

TEST_CASE("floor area follows the members and the income group (A1)") {
  for (const char* income : {"low", "upper_middle", "high"}) {
    CAPTURE(income);
    const Town town = town_of(300, 5);
    const auto d = plan_dwellings(town, rules(), income, 5);
    double per_person = 0, people = 0;
    for (const Dwelling& w : d) {
      const int n = static_cast<int>(town.households[static_cast<std::size_t>(w.household)].members.size());
      CHECK(w.target_area_m2 >= rules().min_area_m2);
      CHECK(w.floor_area() >= w.target_area_m2 * 0.8);  // the plan rounds to whole cells and minimum room sizes
      for (const Room& r : w.rooms) {
        CHECK(r.w >= 2);
        CHECK(r.h >= 2);
      }
      CHECK(w.beds() == n);
      per_person += w.floor_area();
      people += n;
    }
    per_person /= people;
    const double want = rules().area_per_person_m2.at(income);
    CHECK(per_person >= want * 0.8);
    CHECK(per_person <= std::max(want * 1.6, want + 12));  // small households are lifted to the minimum area
  }
}

TEST_CASE("a generated house loads as a site map; every free cell is reachable; beds and food follow the household (A1)") {
  for (const char* income : {"low", "lower_middle", "upper_middle", "high"}) {
    const Town town = town_of(120, 7);
    const auto d = plan_dwellings(town, rules(), income, 7);
    for (std::size_t h = 0; h < d.size(); h += 3) {
      CAPTURE(income);
      CAPTURE(h);
      const Dwelling& w = d[h];
      const Loaded house(house_toml(town, w, rules(), 7));
      const zs::SiteMap& m = *house.map;
      CHECK(m.width() == w.lot_width);
      CHECK(m.height() == w.lot_depth);
      // Everything free is reachable from the street in front of the house.
      const zs::Cell street{0, w.house_x + w.corridor_x, 0};
      for (int y = 0; y < m.height(); ++y)
        for (int x = 0; x < m.width(); ++x) {
          const zs::Cell c{0, x, y};
          if (!m.passable(c) || c == street) continue;
          if (m.find_path(street, c).empty()) {
            CAPTURE(x);
            CAPTURE(y);
            FAIL_CHECK("a free cell cannot be reached");
          }
        }
      CHECK(count_of(m, "bed") == static_cast<int>(town.households[h].members.size()));
      CHECK(count_of(m, "pantry") == 1);
      CHECK(count_of(m, "fridge") == (w.fridge ? 1 : 0));
      std::int64_t food = 0;
      for (const zs::MapObject& o : m.objects())
        for (const auto& [k, v] : o.amounts)
          if (k == "food_kj") food += static_cast<std::int64_t>(v);
      CHECK(food == w.pantry_kj + w.fridge_kj);
      CHECK(count_of(m, "home") == 1);
    }
  }
}

TEST_CASE("the same seed gives the same house; golden hash pinned (A1)") {
  const Town town = town_of(60, 1);
  const auto a = plan_dwellings(town, rules(), "upper_middle", 1);
  const auto b = plan_dwellings(town, rules(), "upper_middle", 1);
  const std::string ta = house_toml(town, a[2], rules(), 1);
  CHECK(ta == house_toml(town, b[2], rules(), 1));
  MESSAGE("house hash: ", hash_str(ta));
  CHECK(hash_str(ta) == 9816986032796343444ULL);  // re-pinned on MSVC 2026-10-13 (spec 79: pantry days from FR-9); was 1644481660845478080
}
