// Spec 75: travel time in the one world.
#include <doctest.h>

#include <set>

#include "zs/gen/place.h"
#include "zs/gen/town_travel.h"
#include "zs/players.h"
#include "zs/pop.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

const TownRecord& town() {
  static const auto t = town_record(zs::default_place(), 1);
  return *t;
}

}  // namespace

TEST_CASE("town travel: the speeds load from data, and a bad directory is refused (A1)") {
  const TownTravel t = load_town_travel(ZS_DATA_DIR "/gen");
  CHECK(t.walk_kmh > 3);
  CHECK(t.drive_kmh > t.walk_kmh);
  CHECK_THROWS(load_town_travel(ZS_DATA_DIR "/no_such_dir"));
}

TEST_CASE("town travel: levels map to plots, the block, each station, an opened place (A1)") {
  const TownLayout& l = town().layout;
  const std::size_t stations = town().stations.size();
  REQUIRE(stations >= 1);
  CHECK(plot_of_level(l, stations, {}, 0) == l.lot_plot.back());
  CHECK(plot_of_level(l, stations, {}, 9) == l.lot_plot.back());
  for (std::size_t i = 0; i < stations; ++i) {
    CHECK(plot_of_level(l, stations, {}, static_cast<int>(10 * (i + 1))) == l.lot_plot[i]);
    CHECK(plot_of_level(l, stations, {}, static_cast<int>(10 * (i + 1) + 3)) == l.lot_plot[i]);  // an upper floor
  }
  CHECK(plot_of_level(l, stations, {}, static_cast<int>(10 * (stations + 1))) == -1);
  const std::vector<zs::Places::Open> opened = {{42, 1000, 3}, {7, 1010, 4}};
  CHECK(plot_of_level(l, stations, opened, 1000) == 42);
  CHECK(plot_of_level(l, stations, opened, 1012) == 7);
  CHECK(plot_of_level(l, stations, opened, 1020) == -1);
  CHECK(plot_of_level(l, stations, {}, -1) == -1);
}

TEST_CASE("town travel: census pops live on the block; others on residential plots, keyed and spread (A1)") {
  const TownLayout& l = town().layout;
  const std::size_t census = 60;
  CHECK(home_plot(l, zs::census_pop_id(0), census, 1) == l.lot_plot.back());
  CHECK(home_plot(l, zs::census_pop_id(59), census, 1) == l.lot_plot.back());
  std::set<int> homes;
  for (std::uint32_t k = 0; k < 40; ++k) {
    const zs::PopId officer = zs::census_pop_id(static_cast<std::uint32_t>(census) + k);
    const int h = home_plot(l, officer, census, 1);
    REQUIRE(h >= 0);
    const Plot& p = l.plots[static_cast<std::size_t>(h)];
    CHECK(p.use == LandUse::Residential);
    CHECK(p.households > 0);
    CHECK(p.lot < 0);
    CHECK(home_plot(l, officer, census, 1) == h);  // the same pop, the same home
    homes.insert(h);
  }
  CHECK(homes.size() > 20);
}

TEST_CASE("town travel: minutes are symmetric, at least 1, short trips walk and long ones drive (A1)") {
  const TownLayout& l = town().layout;
  const TownTravel t = load_town_travel(ZS_DATA_DIR "/gen");
  const int n = static_cast<int>(l.plots.size());
  REQUIRE(n > 10);
  for (int a = 0; a < n; a += 97)
    for (int b = 0; b < n; b += 89) {
      const std::int64_t m = plot_minutes(l, a, b, t);
      CHECK(m >= 1);
      CHECK(m == plot_minutes(l, b, a, t));
    }
  CHECK(plot_minutes(l, 0, 0, t) == 1);
  // The farthest pair takes the car: its time is the drive plus the overhead, under what walking would take.
  int far_a = 0, far_b = 0;
  double best = -1;
  for (int a = 0; a < n; a += 13)
    for (int b = 0; b < n; b += 17) {
      const Plot &pa = l.plots[static_cast<std::size_t>(a)], &pb = l.plots[static_cast<std::size_t>(b)];
      const double d = std::abs(pa.x - pb.x) + std::abs(pa.y - pb.y);
      if (d > best) best = d, far_a = a, far_b = b;
    }
  const double walk_min = 60.0 * (best / 1000.0) / t.walk_kmh;
  CHECK(static_cast<double>(plot_minutes(l, far_a, far_b, t)) < walk_min);
  MESSAGE("town of " << l.plots.size() << " plots, " << l.width_m << " x " << l.height_m << " m; farthest pair "
                     << plot_minutes(l, far_a, far_b, t) << " min (walking " << walk_min << ")");
}

TEST_CASE("town travel: an officer released at his station gets home after the station-to-home time (A1)") {
  const zs::Place place = zs::default_place();
  const auto rec = town_record(place, 1);
  zs::tier0::Tier0 t = zs::tier0::make_run(1, place, zs::kDefaultStartDay, station_fragments(*rec, 1));
  REQUIRE(t.players);
  REQUIRE(t.census);
  const std::size_t census = t.census->people().size();
  t.players->set_travel(travel_in_town(rec, t.places, census, 1));
  zs::World& w = *t.world;
  auto cmd = [&](std::uint16_t type, std::uint64_t subject, std::int64_t a = 0, std::int64_t b = 0) {
    w.submit({w.clock.tick, type, subject, a, b});
    w.run_ticks(1);
  };
  const zs::PopId officer = zs::census_pop_id(static_cast<std::uint32_t>(census));
  cmd(zs::PlayerJoin, 0, static_cast<std::int64_t>(zs::census_pop_id(0)), static_cast<std::int64_t>(zs::DetachLevel::Free));
  cmd(zs::PlayerPossess, 1, static_cast<std::int64_t>(officer));
  REQUIRE(t.players->players()[0].in == officer);
  cmd(zs::PlayerDetach, 1);
  const std::int64_t released = w.clock.tick - 1;
  REQUIRE(t.players->returns().size() == 1);
  const TownTravel tt = load_town_travel(ZS_DATA_DIR "/gen");
  const std::int64_t want = plot_minutes(rec->layout, rec->layout.lot_plot[0], home_plot(rec->layout, officer, census, 1), tt);
  CHECK(t.players->returns()[0].at == released + want * (zs::kTicksPerGameHour / 60));
  MESSAGE("officer released at his station, home in " << want << " min (spec 66 fixed: 120)");
}
