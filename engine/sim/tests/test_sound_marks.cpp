// Spec 58: sound marks. What one listener hears of the sources: noticed at 0 dB over the background, placed at +10 dB,
// heard at the last opening on the way, with a bounded, repeatable error.
#include <doctest.h>

#include <cmath>
#include <optional>

#include "zs/sound_marks.h"
#include "zs/tier0.h"

using namespace zs;
using namespace zs::tier0;

namespace {

constexpr double kPi = 3.14159265358979323846;

// The basement door between the two basement rooms, the cell on each side of it, and a cell one step further into
// the first room (the sound's source), the way the stage 6 door test finds them.
struct Door {
  EdgeRef ref;
  Cell a, b, far_a;
};

Door basement_door(Tier0& t) {
  std::optional<EdgeRef> door;
  for (const auto& [id, ref] : t.actors->edges())
    if (ref.z == -1 && t.items->def(t.items->get(id).def).family == "door") door = ref;
  REQUIRE(door);
  Door d;
  d.ref = *door;
  d.a = Cell{door->z, door->x, door->y};
  d.b = door->vertical ? Cell{door->z, door->x - 1, door->y} : Cell{door->z, door->x, door->y - 1};
  d.far_a = Cell{d.a.z, d.a.x + (d.a.x - d.b.x), d.a.y + (d.a.y - d.b.y)};
  REQUIRE(t.map->in_bounds(d.far_a));
  REQUIRE(t.map->walkable(d.far_a));
  return d;
}

double angle_gap(double a, double b) {
  double g = std::fmod(a - b, 2 * kPi);
  if (g > kPi) g -= 2 * kPi;
  if (g < -kPi) g += 2 * kPi;
  return std::fabs(g);
}

}  // namespace

TEST_CASE("a source below the background is not heard; a loud one is; the level matches the noise field (A1)") {
  Tier0 t = make_world(1);
  const Door d = basement_door(t);
  t.map->set_open(d.ref, true);
  const double bg = 35;
  CHECK(sound_marks(*t.map, *t.items, d.b, {{d.far_a, 30}}, bg, 1).empty());
  const std::vector<NoiseSource> src{{d.far_a, 80}};
  const auto marks = sound_marks(*t.map, *t.items, d.b, src, bg, 1);
  REQUIRE(marks.size() == 1);
  CHECK(marks[0].over_db == doctest::Approx(noise_at(*t.map, noise_field(*t.map, *t.items, src), d.b) - bg));
  CHECK(marks[0].source == 0);
}

TEST_CASE("noticed at 0 dB over the background, placed at +10 dB (decision 0064) (A1)") {
  Tier0 t = make_world(1);
  const Door d = basement_door(t);
  t.map->set_open(d.ref, true);
  std::optional<double> first_noticed, first_placed;
  for (double db = 20; db <= 120; db += 0.5) {
    const auto m = sound_marks(*t.map, *t.items, d.b, {{d.far_a, db}}, 35, 1);
    if (m.empty()) continue;
    if (!first_noticed) {
      first_noticed = m[0].over_db;
      CHECK(m[0].kind == SoundMark::Kind::Noticed);
    }
    if (!first_placed && m[0].kind == SoundMark::Kind::Placed) first_placed = m[0].over_db;
  }
  REQUIRE(first_noticed);
  REQUIRE(first_placed);
  CHECK(*first_noticed >= kNoticeDb);
  CHECK(*first_noticed < kNoticeDb + 0.5 + 1e-9);
  CHECK(*first_placed >= kPlaceDb);
  CHECK(*first_placed < kPlaceDb + 0.5 + 1e-9);
}

TEST_CASE("behind a closed door the sound seems to come from the door; open, from the source (A1)") {
  Tier0 t = make_world(1);
  const Door d = basement_door(t);
  const std::vector<NoiseSource> src{{d.far_a, 95}};
  t.map->set_open(d.ref, true);
  const auto open = sound_marks(*t.map, *t.items, d.b, src, 35, 7);
  REQUIRE(open.size() == 1);
  t.map->set_open(d.ref, false);
  const auto closed = sound_marks(*t.map, *t.items, d.b, src, 35, 7);
  REQUIRE(closed.size() == 1);
  CHECK(open[0].over_db > closed[0].over_db);  // the door takes its loss off
  // The apparent place is the door's cell on the source's side, give or take the placed error; the source is one
  // step further, so with the door shut the mark must be nearer to the listener than the source is.
  CHECK(distance_m(closed[0].apparent, d.b) <= distance_m(d.far_a, d.b) + 1.5);
  Tier0 u = make_world(1);
  const Door e = basement_door(u);
  u.map->set_open(e.ref, false);
  // A faint noticed sound through the shut door has no placed error: its apparent cell is exactly the door's source side.
  bool saw_noticed = false;
  for (double db = 20; db <= 120 && !saw_noticed; db += 0.5)
    for (const SoundMark& m : sound_marks(*u.map, *u.items, e.b, {{e.far_a, db}}, 35, 7))
      if (m.kind == SoundMark::Kind::Noticed) {
        saw_noticed = true;
        CHECK(m.apparent == e.a);
      }
  CHECK(saw_noticed);
}

TEST_CASE("the same key gives the same marks; another key moves them within the error bounds (A1)") {
  Tier0 t = make_world(1);
  const Door d = basement_door(t);
  t.map->set_open(d.ref, true);
  const std::vector<NoiseSource> faint{{d.far_a, 45}}, loud{{d.far_a, 100}};  // 2 m away: 45 dB is 4 over, 100 dB is far over
  const Cell at = d.b;
  const auto a1 = sound_marks(*t.map, *t.items, at, faint, 35, 5), a2 = sound_marks(*t.map, *t.items, at, faint, 35, 5);
  REQUIRE(a1.size() == 1);
  REQUIRE(a2.size() == 1);
  CHECK(a1[0].bearing == a2[0].bearing);
  bool moved = false;
  for (std::uint64_t key = 0; key < 40; ++key) {
    const auto m = sound_marks(*t.map, *t.items, at, faint, 35, key);
    REQUIRE(m.size() == 1);
    CHECK(m[0].kind == SoundMark::Kind::Noticed);
    const double truth = std::atan2(static_cast<double>(at.y - m[0].apparent.y), static_cast<double>(m[0].apparent.x - at.x));
    CHECK(angle_gap(m[0].bearing, truth) <= 0.5 + 1e-9);  // a noticed sound: within about 29 degrees
    moved = moved || m[0].bearing != a1[0].bearing;
    const auto p = sound_marks(*t.map, *t.items, at, loud, 35, key);
    REQUIRE(p.size() == 1);
    CHECK(p[0].kind == SoundMark::Kind::Placed);
    CHECK(distance_m(p[0].apparent, d.far_a) <= 4.0 * 1.5 + 1.5);  // the placed error is at most 4 cells (plus the door's cell)
    CHECK(std::fmod(p[0].distance_m, 5.0) == doctest::Approx(0.0).epsilon(1e-9));
  }
  CHECK(moved);
}

TEST_CASE("the listener's own noise is not a mark; an unreachable or out-of-bounds source is skipped (A1)") {
  Tier0 t = make_world(1);
  const Cell at = t.actors->actor(t.player).pos;
  CHECK(sound_marks(*t.map, *t.items, at, {{at, 120}}, 35, 1).empty());
  CHECK(sound_marks(*t.map, *t.items, at, {{Cell{0, -5, -5}, 120}}, 35, 1).empty());
  CHECK(sound_marks(*t.map, *t.items, Cell{0, -5, -5}, {{at, 120}}, 35, 1).empty());
}

TEST_CASE("compass words follow the bearing, north being toward smaller y (A1)") {
  CHECK(std::string(compass_word(0)) == "east");
  CHECK(std::string(compass_word(kPi / 2)) == "north");
  CHECK(std::string(compass_word(kPi)) == "west");
  CHECK(std::string(compass_word(-kPi / 2)) == "south");
  CHECK(std::string(compass_word(3 * kPi / 2)) == "south");
  CHECK(std::string(compass_word(kPi / 4)) == "north-east");
}

namespace {
SoundMark mark_of(std::size_t source, SoundMark::Kind kind = SoundMark::Kind::Placed) {
  SoundMark m;
  m.source = source;
  m.kind = kind;
  m.over_db = 12;
  return m;
}
}  // namespace

TEST_CASE("a sound that stops becomes an echo that fades over thirty game seconds, then is gone (A1)") {
  SoundMemory mem;
  mem.hear({mark_of(0)}, 100);
  CHECK(mem.echoes(100).empty());  // heard now: a live mark, not an echo
  mem.hear({}, 120);               // silent at the next recompute
  auto e = mem.echoes(120);
  REQUIRE(e.size() == 1);
  CHECK(e[0].last_heard == 100);
  CHECK(e[0].strength(120) == doctest::Approx(1.0 - 20.0 / SoundMemory::kFadeTicks));
  double prev = 2;
  for (std::int64_t now = 120; now < 100 + SoundMemory::kFadeTicks; now += 7) {
    const auto at_now = mem.echoes(now);
    REQUIRE(at_now.size() == 1);
    CHECK(at_now[0].strength(now) < prev);  // monotone fading
    CHECK(at_now[0].strength(now) > 0);
    prev = at_now[0].strength(now);
  }
  CHECK(mem.echoes(100 + SoundMemory::kFadeTicks).empty());  // strength 0: gone
  mem.hear({}, 100 + SoundMemory::kFadeTicks + 5);           // and forgotten for good
  CHECK(mem.echoes(100 + 1).empty());
}

TEST_CASE("a sound heard again is live again; sources are remembered independently, newest first (A1)") {
  SoundMemory mem;
  mem.hear({mark_of(0), mark_of(1, SoundMark::Kind::Noticed)}, 100);
  mem.hear({mark_of(1, SoundMark::Kind::Noticed)}, 110);  // source 0 goes quiet
  auto e = mem.echoes(110);
  REQUIRE(e.size() == 1);
  CHECK(e[0].mark.source == 0);
  CHECK(e[0].mark.kind == SoundMark::Kind::Placed);
  mem.hear({mark_of(0), mark_of(1, SoundMark::Kind::Noticed)}, 120);  // both again
  CHECK(mem.echoes(120).empty());
  mem.hear({}, 130);  // both quiet; 0 and 1 were last heard at the same tick
  mem.hear({mark_of(0)}, 140);
  e = mem.echoes(140);
  REQUIRE(e.size() == 1);  // only 1 is an echo; 0 is live
  CHECK(e[0].mark.source == 1);
  mem.hear({}, 150);  // 0 last heard at 140, 1 at 120
  e = mem.echoes(150);
  REQUIRE(e.size() == 2);
  CHECK(e[0].mark.source == 0);  // the more recent memory first
  CHECK(e[1].mark.source == 1);
  mem.clear();
  CHECK(mem.echoes(150).empty());
}
