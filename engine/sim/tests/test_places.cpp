// Spec 73: a plot opened in play. A command opens a town's plot as a place of the one world: its levels are added after
// the map's, its walls and doors minted, its furniture given entities; it opens once; what does not fit is refused with
// nothing changed; a save holds it and a fresh world loads it back and runs the same; a world that opens nothing saves as
// before.
#include <doctest.h>

#include <string>

#include "zs/actions.h"
#include "zs/places.h"
#include "zs/run.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

// A small house: one floor of 4 x 3 cells inside a 6 x 4 lot, a front door, a bed and a pantry.
const char* kHouse = R"(blocking = []

[map]
name = "7 Test Lane"
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

[[object]]
kind = "bed"
at = [0, 1, 3]

[[object]]
kind = "pantry"
at = [0, 4, 3]
food_kj = 70000
)";

PlotSource source() {
  PlotSource s;
  s.count = [] { return 4; };
  s.address = [](int plot) { return std::to_string(plot + 1) + " Test Lane"; };
  s.site_toml = [](int plot) -> std::string {
    if (plot == 2) return "";  // a plot with a place of its own
    if (plot == 3) return std::string(kHouse).replace(std::string(kHouse).find("width = 6"), 9, "width = 99");  // too big
    return kHouse;
  };
  return s;
}

tier0::Tier0 world_with_town() {
  tier0::Tier0 t = tier0::make_run(1);
  t.world->check_every_tick = false;
  t.places->set_source(source());
  return t;
}

void open(tier0::Tier0& t, int plot) {
  t.world->submit({0, PlaceOpen, 0, plot, 0});
  t.world->run_ticks(1);
}

PlaceRefusal last_refusal(const tier0::Tier0& t) {
  PlaceRefusal out{0};
  for (const Event& e : t.world->log.all())
    if (e.type == EventType::PlaceRefused) out = static_cast<PlaceRefusal>(e.b);
  return out;
}

}  // namespace

TEST_CASE("opening a plot adds it as a place of its own, its walls and furniture known to the actors (A1)") {
  tier0::Tier0 t = world_with_town();
  const std::size_t levels = t.map->levels().size(), objects = t.map->objects().size(), edges = t.actors->edges().size();
  open(t, 1);
  REQUIRE(t.places->opened().size() == 1);
  const Places::Open& o = t.places->opened()[0];
  CHECK(o.plot == 1);
  CHECK(o.z == Places::kFirstLevel);
  CHECK(t.map->levels().size() == levels + 1);
  CHECK(t.map->site_of(o.z) == o.site);
  CHECK(t.map->objects().size() == objects + 2);
  CHECK(t.map->objects().back().entity != 0);
  CHECK(t.actors->object_entity(objects) == t.map->objects()[objects].entity);
  CHECK(t.actors->edges().size() > edges);  // its walls and door can be acted on
  const Cell door{o.z, 3, 1};
  CHECK(!t.map->find_path(Cell{o.z, 3, 0}, door).empty());  // in from its street
  const Cell player = t.actors->actor(t.run->controlled()).pos;
  CHECK(t.map->find_path(player, door).empty());           // not from the block
  int opened = 0;
  for (const Event& e : t.world->log.all()) opened += e.type == EventType::PlaceOpened && e.subject == 1 && e.a == o.z;
  CHECK(opened == 1);
  open(t, 0);
  CHECK(t.places->opened().size() == 2);
  CHECK(t.places->opened()[1].z == Places::kFirstLevel + 10);
}

TEST_CASE("a plot opens once; one with its own place, one too big, an unknown one and no town are refused, nothing changed (A1)") {
  tier0::Tier0 t = world_with_town();
  open(t, 1);
  const std::size_t levels = t.map->levels().size();
  open(t, 1);
  CHECK(last_refusal(t) == PlaceRefusal::AlreadyOpen);
  open(t, 2);
  CHECK(last_refusal(t) == PlaceRefusal::OwnPlace);
  open(t, 3);
  CHECK(last_refusal(t) == PlaceRefusal::DoesNotFit);
  open(t, 9);
  CHECK(last_refusal(t) == PlaceRefusal::NoSuchPlot);
  CHECK(t.map->levels().size() == levels);
  tier0::Tier0 bare = tier0::make_run(1);
  open(bare, 0);
  CHECK(last_refusal(bare) == PlaceRefusal::NoTown);
}

TEST_CASE("a world with places opened saves; a fresh world loads it back with them and runs the same (A1)" *
          doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 a = world_with_town();
  open(a, 1);
  open(a, 0);
  a.world->run_ticks(300);
  tier0::Tier0 b = tier0::make_run(1);  // no source: a load does not need the town, the save holds the places
  b.world->check_every_tick = false;
  b.world->load(a.world->save());
  CHECK(b.world->state_hash() == a.world->state_hash());
  CHECK(b.places->opened().size() == 2);
  CHECK(b.map->levels().size() == a.map->levels().size());
  CHECK(b.actors->edges().size() == a.actors->edges().size());
  CHECK(b.map->objects().back().entity == a.map->objects().back().entity);
  a.world->run_ticks(600);
  b.world->run_ticks(600);
  CHECK(b.world->state_hash() == a.world->state_hash());
}

TEST_CASE("a world that opens nothing saves its map as before, and loads into a world built the same way (A1)") {
  tier0::Tier0 a = world_with_town();
  tier0::Tier0 b = tier0::make_run(1);
  CHECK(a.map->runtime_sites() == 0);
  b.world->load(a.world->save());
  CHECK(b.world->state_hash() == a.world->state_hash());
}
