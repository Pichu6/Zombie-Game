// Stage 2: condition (spec 07), assemblies (08), site map and room graph (09), fortification data (10).
#include <doctest.h>

#include <array>

#include "property.h"
#include "zs/assembly.h"
#include "zs/site_map.h"

using namespace zs;

namespace {
struct Block {
  World world{1};
  Assemblies* items;
  SiteMap* map;
  Block() {
    auto a = std::make_unique<Assemblies>();
    auto m = std::make_unique<SiteMap>();
    items = a.get();
    map = m.get();
    world.add_system(std::move(a));
    world.add_system(std::move(m));
    load_item_defs(*items, std::string(ZS_DATA_DIR) + "/tier0/items.toml");
    map->load_file(world, *items, std::string(ZS_DATA_DIR) + "/tier0/map/block.toml");
    world.close_setup();
  }
};
}  // namespace

TEST_CASE("condition: wear never raises condition, repair never raises max (A1)") {
  property("condition bounds", [](std::uint32_t base, std::uint8_t q, std::uint32_t fix, std::uint32_t loss) {
    Condition c;
    const auto before = c.value;
    c.wear(base % 200000, 1.0, q % 101);
    RC_ASSERT(c.value <= before);
    const auto max_before = c.max;
    c.repair(fix % 2000000, loss % 100000);
    RC_ASSERT(c.max <= max_before);
    RC_ASSERT(0 <= c.value && c.value <= c.max && c.max <= kConditionFull);
  });
}

TEST_CASE("condition: a Q80 part outlasts a Q30 part under the same use (A3)") {
  Condition good, bad;
  int uses_good = 0, uses_bad = 0;
  while (!good.failed(100000)) { good.wear(1000, 1.0, 80); ++uses_good; }
  while (!bad.failed(100000)) { bad.wear(1000, 1.0, 30); ++uses_bad; }
  CHECK(uses_good > uses_bad);
}

TEST_CASE("made quality: Q40 head and Q90 handle at 70/30 give Q55 before caps (A1)") {
  const std::array<std::pair<int, double>, 2> parts = {{{40, 0.7}, {90, 0.3}}};
  CHECK(made_quality(parts, {}) == doctest::Approx(55.0));
  CHECK(made_quality(parts, {.skill = 30}) == doctest::Approx(30.0));
}

TEST_CASE("assemblies: salvage then reassemble keeps parts, mass and Q (A1)") {
  Block b;
  const DefId door = *b.items->find_def("door_interior");
  const EntityId d = b.items->create(b.world, door, 60, Locations::kWorld);
  const std::int64_t mass = b.items->mass_g(d);
  CHECK(mass == 150 + 600 + 12000 + 8000);
  CHECK(b.items->works(d));
  const std::size_t plate_slot = *b.items->slot_index(d, "strike_plate");
  const EntityId plate = b.items->unmount(b.world, d, plate_slot, Locations::kWorld, 50000);
  CHECK(plate != 0);
  CHECK_FALSE(b.items->works(d));  // a missing required part stops the function
  CHECK(b.items->mass_g(d) + b.items->mass_g(plate) == mass);
  CHECK(b.items->mount(b.world, d, plate_slot, plate));
  CHECK(b.items->works(d));
  CHECK(b.items->get(plate).quality == 60);
  CHECK(b.items->mass_g(d) == mass);
  // A part that does not fit is refused.
  const EntityId glass = b.items->create(b.world, *b.items->find_def("glass_pane"), 50, Locations::kWorld);
  b.items->unmount(b.world, d, plate_slot, Locations::kWorld, 0);
  CHECK_FALSE(b.items->mount(b.world, d, plate_slot, glass));
}

TEST_CASE("map: the Tier 0 block loads with the expected room graph (A1, golden)") {
  Block b;
  CHECK(b.map->width() == 32);
  CHECK(b.map->height() == 28);
  // Golden numbers for data/tier0/map/block.toml. Update them only when the map changes on purpose.
  const auto& rooms = b.map->rooms();
  int indoor = 0, outdoor = 0, border = 0;
  for (const Room& r : rooms) {
    (r.indoor ? indoor : outdoor)++;
    if (r.touches_border) ++border;
  }
  CHECK(indoor == 15);  // ground 3 + first 4 + basement 2 + house A 3 + house B 3
  CHECK(outdoor == 2);  // the yard and the outdoors around the block
  CHECK(border == 1);
  // Every walkable cell belongs to exactly one room; every portal joins two different rooms.
  int cells = 0;
  for (const Room& r : rooms) cells += r.cells;
  int walkable = 0;
  for (int z : b.map->levels())
    for (int y = 0; y < b.map->height(); ++y)
      for (int x = 0; x < b.map->width(); ++x) walkable += b.map->walkable({z, x, y});
  CHECK(cells == walkable);
  for (const Portal& p : b.map->portals()) CHECK(p.room_a != p.room_b);
}

TEST_CASE("map: opening a door merges two rooms; closing splits them again (A1)") {
  Block b;
  const Cell front{0, 5, 9}, kitchen{0, 5, 10};
  const EdgeRef door = SiteMap::edge_between(front, kitchen);
  CHECK(b.map->edge(door).kind == EdgeKind::DoorInterior);
  const auto before = b.map->rooms().size();
  CHECK(b.map->room_of(front) != b.map->room_of(kitchen));
  b.map->set_open(door, true);
  CHECK(b.map->rooms().size() == before - 1);
  CHECK(b.map->room_of(front) == b.map->room_of(kitchen));
  b.map->set_open(door, false);
  CHECK(b.map->rooms().size() == before);
}

TEST_CASE("map: breaching a wall merges two rooms and adds no others (A1)") {
  Block b;
  const Cell kitchen{0, 8, 12}, workshop{0, 9, 13};
  const Cell kitchen_side{0, 8, 13};
  const EdgeRef wall = SiteMap::edge_between(kitchen_side, workshop);
  CHECK(b.map->edge(wall).kind == EdgeKind::WallInterior);
  const auto before = b.map->rooms().size();
  b.map->set_breached(wall, true);
  CHECK(b.map->rooms().size() == before - 1);
  CHECK(b.map->room_of(kitchen) == b.map->room_of(workshop));
}

TEST_CASE("map: paths exist from the player start to the yard, the basement and both houses (A1)") {
  Block b;
  const Cell start = *b.map->find_object("pop_start", "player");
  for (const Cell goal : {Cell{0, 8, 20}, Cell{-1, 12, 8}, Cell{1, 9, 12}, Cell{0, 22, 10}, Cell{0, 27, 21}}) {
    const auto path = b.map->find_path(start, goal);
    REQUIRE_FALSE(path.empty());
    CHECK(path.front() == start);
    CHECK(path.back() == goal);
  }
  // Locking the front and back doors seals the main building from outside walkers on foot.
  b.map->set_locked(SiteMap::edge_between({0, 8, 3}, {0, 8, 4}), true);
  b.map->set_locked(SiteMap::edge_between({0, 12, 15}, {0, 12, 16}), true);
  CHECK(b.map->find_path({0, 8, 1}, start).empty());
}

TEST_CASE("map: every edge is an assembly; doors fail in chain order data (spec 10)") {
  Block b;
  const Edge& front_door = b.map->edge(SiteMap::edge_between({0, 8, 3}, {0, 8, 4}));
  REQUIRE(front_door.kind == EdgeKind::DoorExterior);
  const ItemDef& d = b.items->def(b.items->get(front_door.assembly).def);
  CHECK(d.failure_chain.front() == "strike_plate");
  CHECK(d.sound_loss_db == 30);
  CHECK(b.items->works(front_door.assembly));
}

TEST_CASE("map: save and load keep edge states (A1)") {
  Block a;
  a.map->set_open(SiteMap::edge_between({0, 5, 9}, {0, 5, 10}), true);
  a.map->set_breached(SiteMap::edge_between({0, 8, 13}, {0, 9, 13}), true);
  const auto bytes = a.world.save();
  Block b;
  b.world.load(bytes);
  CHECK(b.world.save() == bytes);
  CHECK(b.map->rooms().size() == a.map->rooms().size());
}
