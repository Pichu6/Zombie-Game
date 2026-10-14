// Spec 48: people, sectors, countries and towns on the world grid.
#include <doctest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <string>
#include <vector>

#include "zs/gen/place.h"
#include "zs/gen/world_people.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

struct Planet {
  WorldGrid grid;
  WorldFields fields;
  WorldLand land;
  WorldDeposits deposits;
  WorldPeople people;
  GroupTables tables;
};

Planet planet(int frequency, std::uint64_t seed, const std::vector<RealProfile>& profiles = {}) {
  Planet p{WorldGrid(frequency, 6371.0), {}, {}, {}, {}, load_group_tables(ZS_DATA_DIR)};
  WorldParams wp;
  wp.seed = seed;
  p.fields = generate_fields(p.grid, wp);
  p.land = generate_land(p.grid, p.fields);
  DepositParams dp;
  dp.seed = seed;
  p.deposits = generate_deposits(p.grid, p.fields, dp);
  p.people = populate_world(p.grid, p.fields, p.land, p.deposits, seed, p.tables, profiles);
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
      CHECK((w.sector_of[c] >= 0) == (p.fields.land(c) && p.fields.biome[c] != Biome::Ice));  // spec 52 rule 24
      covered += w.sector_of[c] >= 0;
    }
    std::size_t in_sectors = 0;
    for (const Sector& s : w.sectors) in_sectors += s.cells.size();
    CHECK(in_sectors == covered);
    // Spec 52 rule 25: sectors are cut inside countries, at least one each, never across a border.
    CHECK(w.sectors.size() >= w.countries.size());
    for (const Sector& s : w.sectors)
      for (CellId c : s.cells) CHECK(w.formed.country_of[c] == s.country);
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
  double largest = 0;
  for (const Country& c : w.countries) {
    got[static_cast<std::size_t>(c.income)] += c.people / w.total_people;
    largest = std::max(largest, c.people / w.total_people);
  }
  // Whole countries: a group can be off by half the largest country (spec 83 moved seed 1's largest to 31 % of the
  // world's people, a country size no real country has: decision 0081 step 1 takes that up).
  const double tolerance = std::max(0.10, largest / 2);
  for (int i = 0; i < kIncomeGroups; ++i) {
    const double want = p.tables.population[static_cast<std::size_t>(i)] / group_total;
    MESSAGE(std::string(kIncomeGroupIds[static_cast<std::size_t>(i)]) << ": " << got[static_cast<std::size_t>(i)] << " of people (WPP " << want << ")");
    CHECK(std::abs(got[static_cast<std::size_t>(i)] - want) < tolerance);
  }
  for (std::size_t k = 0; k < w.countries.size(); ++k) {
    const Country& c = w.countries[k];
    double urban = 0;
    std::vector<double> sizes;
    for (const Settlement& t : w.towns)
      if (t.country == static_cast<int>(k)) {
        urban += t.people;
        if (t.site != TownSite::Resource) sizes.push_back(t.people);  // spec 86 rule 3: resource towns stand outside rank-size
        CHECK(w.sector_of[t.cell] >= 0);
        CHECK(w.sectors[static_cast<std::size_t>(w.sector_of[t.cell])].country == static_cast<int>(k));
      }
    const double want = c.people * p.tables.urban_share[static_cast<std::size_t>(c.income)];
    if (want > 1e6) CHECK(urban == doctest::Approx(want).epsilon(0.10));
    if (sizes.size() >= 10) sizes.pop_back();  // spec 78 rule 3: the last town holds what is left, off the line
    if (sizes.size() >= 10) {  // least-squares slope of log size on log rank is -1/zeta
      double sx = 0, sy = 0, sxx = 0, sxy = 0;
      for (std::size_t i = 0; i < sizes.size(); ++i) {
        const double x = std::log(static_cast<double>(i + 1)), y = std::log(sizes[i]);
        sx += x; sy += y; sxx += x * x; sxy += x * y;
      }
      const double nn = static_cast<double>(sizes.size());
      const double slope = (nn * sxy - sx * sy) / (nn * sxx - sx * sx);
      CHECK(std::abs(-1.0 / slope - c.zeta) < 0.1);  // spec 78 rule 3: each country's own exponent
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

TEST_CASE("real profiles (spec 49): each country takes a real profile of its own income group, with a distinct name") {
  const auto profiles = load_real_profiles(ZS_DATA_DIR);
  REQUIRE(profiles.size() > 150);
  const Planet p = planet(kOneWorldFrequency, 1, profiles);
  std::set<std::string> taken, names;
  // A profile is used twice only once all are used (an Earth-size world forms more countries than there are profiles).
  for (const Country& c : p.people.countries) taken.insert(c.profile);
  const bool all_used = taken.size() == profiles.size();
  if (!all_used) CHECK(taken.size() == p.people.countries.size());
  for (const Country& c : p.people.countries) {
    REQUIRE_FALSE(c.profile.empty());
    CHECK(names.insert(c.name).second);
    const auto it = std::find_if(profiles.begin(), profiles.end(), [&](const RealProfile& r) { return r.iso == c.profile; });
    REQUIRE(it != profiles.end());
    // Its own group's profile, unless that group's are all taken.
    std::size_t own = 0, own_taken = 0;
    for (const RealProfile& r : profiles)
      if (r.income == c.income) {
        ++own;
        for (const Country& o : p.people.countries) own_taken += o.profile == r.iso;
      }
    if (it->income != c.income) CHECK(own_taken >= own);  // counts a reused profile once per country
    CHECK(c.name.size() >= it->name.size());
    CHECK(c.name.compare(c.name.size() - it->name.size(), it->name.size(), it->name) == 0);  // the name, maybe "New ..."
    MESSAGE(c.name << " (" << c.profile << "), " << kIncomeGroupIds[static_cast<std::size_t>(c.income)] << ", " << c.people / 1e6 << " million");
  }
  const auto t = make_world(1);
  CHECK_FALSE(t.place->place().country_name.empty());
  CHECK_FALSE(t.place->place().profile.empty());
}

// Spec 78: sizes that fit the real world.

TEST_CASE("spec 78: income groups hold countries of their real sizes; the largest towns follow primacy (A2)" *
          doctest::skip(zs::World::kDebugBuild)) {
  const auto profiles = load_real_profiles(ZS_DATA_DIR);
  for (std::uint64_t seed : {1ull, 4ull}) {
    const Planet p = planet(kOneWorldFrequency, seed, profiles);
    const WorldPeople& w = p.people;
    std::array<double, kIncomeGroups> largest{};
    for (const RealProfile& r : profiles) largest[static_cast<std::size_t>(r.income)] = std::max(largest[static_cast<std::size_t>(r.income)], r.population);
    const double scale = w.total_people / p.tables.world_population;
    double any = 0;  // a country larger than every group's largest keeps the group it had (rule 1)
    for (double l : largest) any = std::max(any, l);
    for (const Country& c : w.countries)
      if (c.people <= kGroupSizeTolerance * any * scale) CHECK(c.people <= kGroupSizeTolerance * largest[static_cast<std::size_t>(c.income)] * scale * (1 + 1e-9));
    std::vector<double> urban(w.countries.size(), 0);
    std::vector<double> top(w.countries.size(), 0);
    std::size_t ten = 0, five = 0, one = 0;
    for (const Settlement& t : w.towns) {
      urban[static_cast<std::size_t>(t.country)] += t.people;
      if (t.rank == 1) top[static_cast<std::size_t>(t.country)] = t.people;
      CHECK(t.people < 50e6);
      ten += t.people >= 10e6;
      five += t.people >= 5e6 && t.people < 10e6;
      one += t.people >= 1e6 && t.people < 5e6;
    }
    for (std::size_t k = 0; k < w.countries.size(); ++k) {
      if (urban[k] <= 0) continue;
      const Country& c = w.countries[k];
      const double want = c.people * p.tables.urban_share[static_cast<std::size_t>(c.income)];
      CHECK(std::abs(urban[k] - want) <= kTownMinPeople);  // the towns hold the urban people, all but a last town's worth
      CHECK(top[k] >= std::min(want, kPrimacyMin * want) - 1);
      CHECK(top[k] <= want + 1);
      CHECK(c.zeta >= kZetaMin);
      CHECK(c.zeta <= kZetaMax);
    }
    // DR-4: the median country's largest town holds 30 % of its urban people (middle half 20-40 %).
    std::vector<double> shares;
    for (std::size_t k = 0; k < w.countries.size(); ++k)
      if (urban[k] > 0) shares.push_back(top[k] / urban[k]);
    std::sort(shares.begin(), shares.end());
    REQUIRE(!shares.empty());
    const double median = shares[shares.size() / 2];
    CHECK(median > 0.20);
    CHECK(median < 0.45);
    // DR-1: 33 of 10 million or more, 48 of 5-10 million, 467 of 1-5 million: within half to twice.
    CHECK(ten >= 33 / 2);
    CHECK(ten <= 33 * 2);
    CHECK(five >= 48 / 2);
    CHECK(five <= 48 * 2);
    CHECK(one >= 467 / 2);
    CHECK(one <= 467 * 2);
    double biggest = 0;
    for (const Country& c : w.countries) biggest = std::max(biggest, c.people);
    MESSAGE("seed " << seed << ": agglomerations 10M+ " << ten << " (33), 5-10M " << five << " (48), 1-5M " << one
                    << " (467); primacy median " << median << " (0.30); the most populous country " << biggest / 1e6 << " million");
  }
}
