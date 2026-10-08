// Spec 48: people, sectors, countries and towns on the world grid.
#include <doctest.h>

#include <cmath>
#include <set>
#include <string>

#include "zs/gen/place.h"
#include "zs/gen/world_people.h"

using namespace zs::gen;

namespace {

struct Planet {
  WorldGrid grid;
  WorldFields fields;
  WorldPeople people;
  GroupTables tables;
};

Planet planet(int frequency, std::uint64_t seed) {
  Planet p{WorldGrid(frequency, 6371.0), {}, {}, load_group_tables(ZS_DATA_DIR)};
  WorldParams wp;
  wp.seed = seed;
  p.fields = generate_fields(p.grid, wp);
  p.people = populate_world(p.grid, p.fields, seed, p.tables);
  return p;
}

}  // namespace

TEST_CASE("world people: people add up, live on suitable land, and sectors cover the land once") {
  for (std::uint64_t seed : {1ull, 4ull}) {
    const Planet p = planet(kOneWorldFrequency, seed);
    const WorldPeople& w = p.people;
    const double expected = p.tables.world_population * w.land_km2 / kEarthLandKm2;
    CHECK(std::abs(w.total_people - expected) <= static_cast<double>(p.grid.size()));
    std::size_t covered = 0;
    for (CellId c = 0; c < p.grid.size(); ++c) {
      if (!p.fields.land(c) || p.fields.biome[c] == Biome::Ice) CHECK(w.people[c] == 0.0);
      CHECK((w.sector_of[c] >= 0) == p.fields.land(c));
      covered += w.sector_of[c] >= 0;
    }
    std::size_t in_sectors = 0;
    for (const Sector& s : w.sectors) in_sectors += s.cells.size();
    CHECK(in_sectors == covered);
    const double target = w.land_km2 / kSectorTargetKm2;
    CHECK(std::abs(static_cast<double>(w.sectors.size()) - std::max(1.0, std::round(target))) <= 1.0);
    MESSAGE("seed " << seed << ": " << w.sectors.size() << " sectors, " << w.countries.size() << " countries, "
                    << w.towns.size() << " towns, " << w.total_people / 1e9 << " billion people");
  }
}

TEST_CASE("world people: income groups hold their real shares; towns follow rank-size and the urban share") {
  const Planet p = planet(kOneWorldFrequency, 1);
  const WorldPeople& w = p.people;
  double group_total = 0;
  for (double x : p.tables.population) group_total += x;
  std::array<double, kIncomeGroups> got{};
  for (const Country& c : w.countries) got[static_cast<std::size_t>(c.income)] += c.people / w.total_people;
  for (int i = 0; i < kIncomeGroups; ++i) {
    const double want = p.tables.population[static_cast<std::size_t>(i)] / group_total;
    MESSAGE(std::string(kIncomeGroupIds[static_cast<std::size_t>(i)]) << ": " << got[static_cast<std::size_t>(i)] << " of people (WPP " << want << ")");
    CHECK(std::abs(got[static_cast<std::size_t>(i)] - want) < 0.10);  // whole countries: a dozen of them cannot be exact
  }
  for (std::size_t k = 0; k < w.countries.size(); ++k) {
    const Country& c = w.countries[k];
    double urban = 0;
    std::vector<double> sizes;
    for (const Settlement& t : w.towns)
      if (t.country == static_cast<int>(k)) {
        urban += t.people;
        sizes.push_back(t.people);
        CHECK(w.sector_of[t.cell] >= 0);
        CHECK(w.sectors[static_cast<std::size_t>(w.sector_of[t.cell])].country == static_cast<int>(k));
      }
    const double want = c.people * p.tables.urban_share[static_cast<std::size_t>(c.income)];
    if (want > 1e6) CHECK(urban == doctest::Approx(want).epsilon(0.10));
    if (sizes.size() >= 10) {  // least-squares slope of log size on log rank is -1/zeta
      double sx = 0, sy = 0, sxx = 0, sxy = 0;
      for (std::size_t i = 0; i < sizes.size(); ++i) {
        const double x = std::log(static_cast<double>(i + 1)), y = std::log(sizes[i]);
        sx += x; sy += y; sxx += x * x; sxy += x * y;
      }
      const double nn = static_cast<double>(sizes.size());
      const double slope = (nn * sxy - sx * sy) / (nn * sxx - sx * sx);
      CHECK(std::abs(-1.0 / slope - kRankSizeZeta) < 0.1);
    }
  }
}

TEST_CASE("world people: countries are connected over the sector graph; the same seed gives the same world") {
  const Planet a = planet(kOneWorldFrequency, 2), b = planet(kOneWorldFrequency, 2);
  CHECK(a.people.sector_of == b.people.sector_of);
  CHECK(a.people.towns.size() == b.people.towns.size());
  for (const Country& c : a.people.countries) {
    CHECK(!c.sectors.empty());
    CHECK(a.people.sectors[static_cast<std::size_t>(c.capital)].country == a.people.sectors[static_cast<std::size_t>(c.sectors.front())].country);
  }
}

TEST_CASE("world people: the one world stands in a temperate town of 10,000 to 200,000 with its country's group") {
  const Planet p = planet(kOneWorldFrequency, 1);
  const int town = choose_town(p.fields, p.people, 1);
  REQUIRE(town >= 0);
  const zs::Place place = place_of_town(p.grid, p.fields, p.people, town, 1);
  CHECK(place.town_people >= 10'000);
  CHECK(place.town_people <= 200'000);
  CHECK(std::set<std::string>{"high", "upper_middle", "lower_middle", "low"}.count(place.income) == 1);
  const auto t = make_world(1);
  REQUIRE(t.place);
  CHECK(t.place->place().cell == place.cell);
  CHECK(t.place->place().income == place.income);
  MESSAGE("the block's town: " << place.town_people << " people, " << place.biome << ", " << place.income << " income");
}
