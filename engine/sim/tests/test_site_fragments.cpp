// Spec 70: a place beside the block (a fragment): its levels are added after the map's, joined to nothing, so nothing walks
// between them; the map's own indices do not move; a possessed officer of that place arrives at its door; saves carry it.
#include <doctest.h>

#include <string>

#include "zs/players.h"
#include "zs/police_staff.h"
#include "zs/run.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

// Two floors of 4 x 3 cells inside a 6 x 4 lot: a street row, the building with an outer wall and a front door; stairs at
// the back between the floors; the marker inside the front door.
const char* kFragment = R"(blocking = []

[map]
name = "1 Test Street"
width = 6
height = 4
build_quality = 50

[[level]]
z = 0
grid = '''
+ + + + + + +
 _ _ _ _ _ _
+ +#+#+E+#+ +
 "#. . . .#"
+ + + + + + +
 "#. . . .#"
+ + + + + + +
 "#. . . .#"
+ +#+#+#+#+ +
'''

[[level]]
z = 1
grid = '''
+ + + + + + +

+ +#+#+#+#+ +
  #. . . .#
+ + + + + + +
  #. . . .#
+ + + + + + +
  #. . . .#
+ +#+#+#+#+ +
'''

[[stairs]]
a = [0, 3, 3]
b = [1, 3, 3]

[[object]]
kind = "police_station"
at = [0, 3, 1]
id = "Test Station"

[[object]]
kind = "desk"
at = [1, 1, 2]
)";

tier0::Fragment fragment(int officers = 3) {
  tier0::Fragment f;
  f.toml = kFragment;
  f.z = 10;
  f.name = "Test Station";
  f.first_officer = 0;
  f.officers = officers;
  return f;
}

}  // namespace

TEST_CASE("a fragment's levels come after the map's; the map's floors keep their ids; it is a place of its own (A1)") {
  tier0::Tier0 plain = tier0::make_run(1);
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {fragment()});
  const SiteMap& m = *t.map;
  REQUIRE(m.levels().size() == plain.map->levels().size() + 2);
  CHECK(m.levels()[m.levels().size() - 2] == 10);
  CHECK(m.levels().back() == 11);
  CHECK(m.site_of(0) == 0);
  CHECK(m.site_of(10) == 1);
  CHECK(m.site_of(11) == 1);
  CHECK(m.same_site(10, 11));
  CHECK(!m.same_site(0, 10));
  const Cell some{0, 5, 5};
  CHECK(t.actors->floor_at(some) == plain.actors->floor_at(some));  // nothing on the block moved
  REQUIRE(t.sites.size() == 1);
  CHECK(t.sites[0].door == Cell{10, 3, 1});
  CHECK(m.find_path(t.sites[0].door, Cell{11, 1, 3}).size() > 0);  // up the stairs inside the station
  const Cell player = t.actors->actor(t.run->controlled()).pos;
  CHECK(m.find_path(player, t.sites[0].door).empty());              // no way from the block to the station
  CHECK(m.find_object("desk")->z == 11);
}

TEST_CASE("a fragment that does not fit the map, or comes after setup, is refused (A1)") {
  tier0::Fragment big = fragment();
  big.toml = std::string(kFragment).replace(std::string(kFragment).find("width = 6"), 9, "width = 99");
  CHECK_THROWS(tier0::make_run(1, default_place(), kDefaultStartDay, {big}));
  tier0::Tier0 t = tier0::make_run(1);
  CHECK_THROWS(t.map->add_site(*t.world, *t.items, kFragment, 10));
}

TEST_CASE("an officer of the station arrives at its door; an officer of no station on the block (A1)") {
  tier0::Tier0 t = tier0::make_run(1, default_place(), kDefaultStartDay, {fragment(3)});
  t.world->check_every_tick = false;
  const PopId you = census_pop_id(static_cast<std::uint32_t>(*t.census->person_of(t.run->controlled())));
  t.world->submit({0, PlayerJoin, 0, static_cast<std::int64_t>(you), static_cast<std::int64_t>(DetachLevel::Free)});
  t.world->submit({0, PlayerPossess, 1, static_cast<std::int64_t>(t.force->officers()[0]), 0});
  t.world->run_ticks(1);
  const EntityId body = t.players->players()[0].body;
  REQUIRE(body != 0);
  CHECK(t.actors->actor(body).pos == t.sites[0].door);
  t.world->submit({0, PlayerReturn, 1, 0, 0});
  t.world->submit({1, PlayerPossess, 1, static_cast<std::int64_t>(t.force->officers()[5]), 0});  // not one of the station's three
  t.world->run_ticks(2);
  const EntityId other = t.players->players()[0].body;
  REQUIRE(other != 0);
  CHECK(t.actors->actor(other).pos == *t.map->find_object("pop_start", "player"));
}

TEST_CASE("a world with a fragment saves and loads into the same setup, and runs the same (A1)" * doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 a = tier0::make_run(1, default_place(), kDefaultStartDay, {fragment()});
  tier0::Tier0 b = tier0::make_run(1, default_place(), kDefaultStartDay, {fragment()});
  a.world->check_every_tick = b.world->check_every_tick = false;
  a.world->run_ticks(600);
  b.world->load(a.world->save());
  CHECK(a.world->state_hash() == b.world->state_hash());
  a.world->run_ticks(600);
  b.world->run_ticks(600);
  CHECK(a.world->state_hash() == b.world->state_hash());
}
