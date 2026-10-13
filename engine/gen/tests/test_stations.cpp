// Spec 69: a town's police stations: the agency's staff over its stations, a name and an address, and a building that loads
// as a site map with every free cell reachable, a desk for each officer on a shift, a locker for every officer and its cells.
#include <doctest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <set>

#include "zs/gen/place.h"
#include "zs/gen/stations.h"
#include "zs/gen/keyed.h"
#include "zs/police_staff.h"
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

const StationRules& station_rules() {
  static const StationRules r = load_station_rules(std::string(ZS_DATA_DIR) + "/gen");
  return r;
}

const zs::AgencyParams& agency_params() {
  static const zs::AgencyParams p = zs::load_agency_params(std::string(ZS_DATA_DIR) + "/services/police.toml", "");
  return p;
}

Town town_of(std::uint64_t seed) {
  const Mix mix = load_mix(std::string(ZS_DATA_DIR) + "/gen/populations/tier0_block.toml");
  return generate_town(catalog(), mix, seed);
}

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
    const std::string path = (std::filesystem::temp_directory_path() / "zs_spec69_station.toml").string();
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

TEST_CASE("the agency's staff is split over its stations, each with its own name and address (A1)") {
  const Town town = town_of(3);
  const zs::AgencyPlan agency = zs::plan_agency(agency_params(), 250'000, 6);
  REQUIRE(agency.stations == 3);
  const auto stations = plan_stations(catalog(), town, agency, agency_params().shifts, station_rules(), rules(), 3);
  REQUIRE(stations.size() == 3);
  int sworn = 0, investigators = 0, civilians = 0, cells = 0;
  std::set<std::string> names, addresses;
  for (const Station& s : stations) {
    sworn += s.sworn;
    investigators += s.investigators;
    civilians += s.civilians;
    cells += s.cells;
    CHECK(s.patrol == s.sworn - s.investigators);
    CHECK(s.number >= 1);
    CHECK(s.number <= 99);  // houses are numbered from 101: a station never takes a house's number
    names.insert(s.name);
    addresses.insert(s.address);
  }
  CHECK(sworn == agency.sworn);
  CHECK(investigators == agency.investigators);
  CHECK(civilians == agency.civilians);
  CHECK(cells == agency.cells);
  CHECK(names.size() == 3);
  CHECK(!stations[0].name.empty());
}

TEST_CASE("a station loads as a site map; every free cell is reachable; desks, lockers and cells follow its staff (A1)") {
  for (double people : {2'000.0, 17'626.0, 50'000.0, 180'000.0}) {
    CAPTURE(people);
    const Town town = town_of(7);
    const zs::AgencyPlan agency = zs::plan_agency(agency_params(), people, 6);
    const auto stations = plan_stations(catalog(), town, agency, agency_params().shifts, station_rules(), rules(), 7);
    const Station& s = stations.front();
    const Loaded built(station_toml(town, s, rules(), 7));
    const zs::SiteMap& m = *built.map;
    CHECK(m.width() == s.plan.lot_width);
    CHECK(m.height() == s.plan.lot_depth);
    const zs::Cell street{0, s.plan.house_x + s.plan.corridor_x, 0};
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
    const int on_shift = std::max(1, (s.patrol + s.shifts - 1) / s.shifts);
    // The lobby's counter, the squad room's desks for one shift, the commander's, the investigators' and records'.
    CHECK(count_of(m, "desk") == 1 + on_shift + 1 + s.investigators + s.civilians);
    CHECK(count_of(m, "locker") == s.sworn);
    CHECK(count_of(m, "bed") == s.cells);  // one bunk per holding cell
    CHECK(count_of(m, "police_station") == 1);
    CHECK(count_of(m, "toilet") >= 1 + s.cells);
  }
}

TEST_CASE("the same seed gives the same station; a bad rules file is refused (A1)") {
  const Town town = town_of(1);
  const zs::AgencyPlan agency = zs::plan_agency(agency_params(), 50'000, 6);
  const auto a = plan_stations(catalog(), town, agency, 3, station_rules(), rules(), 1);
  const auto b = plan_stations(catalog(), town, agency, 3, station_rules(), rules(), 1);
  CHECK(station_toml(town, a[0], rules(), 1) == station_toml(town, b[0], rules(), 1));
  MESSAGE("station hash: ", hash_str(station_toml(town, a[0], rules(), 1)));
  CHECK(hash_str(station_toml(town, a[0], rules(), 1)) == 726883634672967044ULL);  // pinned on MSVC, 2026-10-13 (spec 70: ground-floor rooms first); before 4771186321979745881
  CHECK_THROWS(load_station_rules(std::string(ZS_DATA_DIR) + "/no_such_dir"));
}

// ---- Spec 70: a station that must fit a site's frame goes up floors; the town's stations as fragments of the one world.

TEST_CASE("a station deeper than its frame goes up floors joined by stairs; every floor is reachable from the street (A1)") {
  const Town town = town_of(11);
  const zs::AgencyPlan agency = zs::plan_agency(agency_params(), 180'000, 6);
  const int max_depth = 20;
  const auto stations = plan_stations(catalog(), town, agency, agency_params().shifts, station_rules(), rules(), 11, max_depth);
  const Station& s = stations.front();
  CHECK(s.plan.floors > 1);
  CHECK(s.plan.depth <= max_depth);
  for (const Room& r : s.plan.rooms) {
    CHECK(r.y + r.h <= s.plan.depth);
    if (r.type == "lobby" || r.type == "cell") CHECK(r.floor == 0);  // the public and the detained stay downstairs
  }
  const Loaded built(station_toml(town, s, rules(), 11));
  const zs::SiteMap& m = *built.map;
  CHECK(static_cast<int>(m.levels().size()) == s.plan.floors);
  CHECK(static_cast<int>(m.stairs().size()) == s.plan.floors - 1);
  const zs::Cell street{0, s.plan.house_x + s.plan.corridor_x, 0};
  for (int z : m.levels())
    for (int y = 0; y < m.height(); ++y)
      for (int x = 0; x < m.width(); ++x) {
        const zs::Cell c{z, x, y};
        if (!m.passable(c) || c == street) continue;
        if (m.find_path(street, c).empty()) {
          CAPTURE(z);
          CAPTURE(x);
          CAPTURE(y);
          FAIL_CHECK("a free cell cannot be reached");
        }
      }
  const int on_shift = std::max(1, (s.patrol + s.shifts - 1) / s.shifts);
  CHECK(count_of(m, "desk") == 1 + on_shift + 1 + s.investigators + s.civilians);
  CHECK(count_of(m, "locker") == s.sworn);
  CHECK(count_of(m, "bed") == s.cells);
}

TEST_CASE("the one world loads its town's stations beside the block; an officer is a station's; none is reachable from the block (A1)") {
  const zs::Place place = zs::default_place();
  const auto fragments = station_fragments(place, 1);
  REQUIRE(!fragments.empty());
  int officers = 0;
  for (const auto& f : fragments) officers += f.officers;
  zs::tier0::Tier0 t = zs::tier0::make_run(1, place, zs::kDefaultStartDay, fragments);
  CHECK(officers == static_cast<int>(t.force->officers().size()));  // the stations hold the whole force
  REQUIRE(t.sites.size() == fragments.size());
  const zs::Cell start = *t.map->find_object("pop_start", "player");
  for (const auto& s : t.sites) {
    CHECK(t.map->passable(s.door));
    CHECK(t.map->site_of(s.door.z) == s.number);
    CHECK(t.map->find_path(start, s.door).empty());
  }
}
