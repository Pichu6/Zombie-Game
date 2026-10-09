// Spec 50: 8-direction paths across open corners, blocking furniture, the Walk intent (steering without stops,
// sliding along walls), doors opened on the way, and a map that stays connected.
#include <doctest.h>

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <vector>

#include "zs/tier0.h"

using namespace zs;
using namespace zs::tier0;

namespace {

Command walk(const Tier0& t, int heading) {
  Command c;
  c.type = static_cast<std::uint16_t>(CommandType::Walk);
  c.subject = t.player;
  c.a = heading;
  return c;
}

Command move_to(const Tier0& t, const Cell& to) {
  Command c;
  c.type = static_cast<std::uint16_t>(CommandType::MoveTo);
  c.subject = t.player;
  c.a = pack_cell(to);
  return c;
}

// Every cell reachable from `from` by 4-neighbour steps and stairs; `skip_blocked` treats furniture as walls.
std::vector<Cell> reachable(const SiteMap& m, const Cell& from, bool skip_blocked) {
  std::vector<Cell> seen{from};
  for (std::size_t i = 0; i < seen.size(); ++i) {
    const Cell c = seen[i];
    std::vector<Cell> next;
    for (const Cell& nb : {Cell{c.z, c.x + 1, c.y}, Cell{c.z, c.x - 1, c.y}, Cell{c.z, c.x, c.y + 1}, Cell{c.z, c.x, c.y - 1}})
      if (m.in_bounds(nb) && m.walkable(nb) && (!skip_blocked || !m.blocked(nb)) && m.can_walk(SiteMap::edge_between(c, nb)))
        next.push_back(nb);
    for (const Stairs& s : m.stairs()) {
      if (s.a == c) next.push_back(s.b);
      if (s.b == c) next.push_back(s.a);
    }
    for (const Cell& nb : next)
      if (std::find(seen.begin(), seen.end(), nb) == seen.end()) seen.push_back(nb);
  }
  return seen;
}

// A 4x4 patch of open floor with no edges inside: a diagonal can cross it.
std::optional<Cell> open_patch(const SiteMap& m) {
  for (int y = 0; y + 3 < m.height(); ++y)
    for (int x = 0; x + 3 < m.width(); ++x) {
      bool ok = true;
      for (int i = 0; i < 3 && ok; ++i)
        ok = m.step_allowed({0, x + i, y + i}, {0, x + i + 1, y + i + 1});
      if (ok) return Cell{0, x, y};
    }
  return std::nullopt;
}

}  // namespace

TEST_CASE("paths step diagonally across open corners and never through a corner with an edge (A1)") {
  Tier0 t = make_world(1);
  const SiteMap& m = *t.map;
  const auto p = open_patch(m);
  REQUIRE(p);
  const auto path = m.find_path(*p, {0, p->x + 3, p->y + 3});
  CHECK(path.size() == 4);  // three diagonal steps, not six straight ones
  // Over many pairs: every diagonal step is an open corner, every door or window is crossed straight.
  const Cell start = t.actors->actor(t.player).pos;
  int diagonals = 0;
  for (const Cell& goal : reachable(m, start, true)) {
    if (goal.z != start.z || (goal.x + goal.y) % 7 != 0) continue;
    const auto route = m.find_path(start, goal);
    REQUIRE(!route.empty());
    for (std::size_t i = 1; i < route.size(); ++i) {
      const Cell a = route[i - 1], b = route[i];
      if (a.z != b.z) continue;
      CHECK(m.step_allowed(a, b));
      if (a.x != b.x && a.y != b.y) ++diagonals;
    }
  }
  CHECK(diagonals > 0);
}

TEST_CASE("furniture fills its cell: no path, step or walker enters it; it is used from beside it (A1)") {
  Tier0 t = make_world(1);
  const SiteMap& m = *t.map;
  const auto fridge = m.find_object("fridge");
  REQUIRE(fridge);
  CHECK(m.blocked(*fridge));
  CHECK(!m.passable(*fridge));
  CHECK(m.find_path(t.actors->actor(t.player).pos, *fridge).empty());
  for (int dy = -1; dy <= 1; ++dy)
    for (int dx = -1; dx <= 1; ++dx) {
      const Cell from{fridge->z, fridge->x - dx, fridge->y - dy};
      if ((dx || dy) && m.passable(from)) CHECK(!m.step_allowed(from, *fridge));
    }
  CHECK(!m.blocked(*m.find_object("lamp")));  // lamps do not block
}

TEST_CASE("every cell reachable without furniture is still reachable from the pop start (A1)") {
  Tier0 t = make_world(1);
  const SiteMap& m = *t.map;
  const Cell start = t.actors->actor(t.player).pos;
  const auto before = reachable(m, start, false);
  const auto after = reachable(m, start, true);
  for (const Cell& c : before) {
    if (m.blocked(c)) continue;
    CAPTURE(c.z);
    CAPTURE(c.x);
    CAPTURE(c.y);
    CHECK(std::find(after.begin(), after.end(), c) != after.end());
  }
}

TEST_CASE("Walk moves cell to cell without stopping, slides along walls and stops at the next cell on -1 (A1)") {
  Tier0 t = make_world(1);
  World& w = *t.world;
  Actor& me = t.actors->actor(t.player);
  // Find a heading with four free cells ahead of the start.
  int heading = -1;
  for (int h = 0; h < 8 && heading < 0; h += 2) {
    int dx = 0, dy = 0;
    heading_step(h, dx, dy);
    Cell c = me.pos;
    int free = 0;
    while (free < 4 && t.map->steer(c, dx, dy) == Cell{c.z, c.x + dx, c.y + dy}) {
      c = {c.z, c.x + dx, c.y + dy};
      ++free;
    }
    if (free == 4) heading = h;
  }
  REQUIRE(heading >= 0);
  const Cell from = me.pos;
  w.submit(walk(t, heading));
  w.step();
  int idle_ticks = 0;
  for (int i = 0; i < 6; ++i) {  // 3 game seconds
    w.step();
    if (me.action.phase == Phase::Idle) ++idle_ticks;
  }
  CHECK(idle_ticks == 0);
  const int moved = std::abs(me.pos.x - from.x) + std::abs(me.pos.y - from.y);
  CHECK(moved >= 3);
  CHECK(moved <= 5);
  w.submit(walk(t, -1));
  for (int i = 0; i < 4; ++i) w.step();
  CHECK(me.action.phase == Phase::Idle);
  CHECK(me.heading == -1);
  // Walking into a wall: the actor stays put with its heading, nothing breaks.
  for (int h = 0; h < 8; ++h) {
    int dx = 0, dy = 0;
    heading_step(h, dx, dy);
    if (!t.map->steer(me.pos, dx, dy)) {
      const Cell here = me.pos;
      w.submit(walk(t, h));
      for (int i = 0; i < 4; ++i) w.step();
      CHECK(me.pos == here);
      CHECK(me.heading == h);
      break;
    }
  }
}

TEST_CASE("a closed door on the way costs half a second, not two (A1)") {
  Tier0 t = make_world(1);
  World& w = *t.world;
  SiteMap& m = *t.map;
  // An interior door on the player's level with free floor on both sides.
  std::optional<std::pair<Cell, Cell>> sides;
  for (int y = 1; y < m.height() && !sides; ++y)
    for (int x = 1; x < m.width() && !sides; ++x)
      for (bool vertical : {false, true}) {
        const EdgeRef e{0, x, y, vertical};
        if (m.edge(e).kind != EdgeKind::DoorInterior || !m.can_walk(e)) continue;
        const Cell a{0, x, y}, b = vertical ? Cell{0, x - 1, y} : Cell{0, x, y - 1};
        if (m.passable(a) && m.passable(b)) {
          sides = std::pair{a, b};
          break;
        }
      }
  REQUIRE(sides);
  const EdgeRef door = SiteMap::edge_between(sides->first, sides->second);
  auto ticks_through = [&](bool open) {
    m.set_open(door, open);
    Actor& me = t.actors->actor(t.player);
    me.pos = sides->first;
    me.action = Action{};
    w.submit(move_to(t, sides->second));
    int n = 0;
    while (!(me.pos == sides->second) && n < 100) {
      w.step();
      ++n;
    }
    return n;
  };
  const int open = ticks_through(true);
  const int closed = ticks_through(false);
  CHECK(closed - open >= 1);
  CHECK(closed - open <= 2);  // 0.5 s is one tick, plus at most one of rounding
}
