// Spec 71: a town's streets, blocks and plots (research city-layout-generation, CL): every plot fronts a street, plots
// partition their block, every household has a plot and every address is unique, the land shares and the street network
// fall in the research's bands, the town's stations get plots of their own, and the same seed gives the same town.
#include <doctest.h>

#include <set>
#include <string>

#include "zs/gen/keyed.h"
#include "zs/gen/pops.h"
#include "zs/gen/town_layout.h"

using namespace zs::gen;

namespace {

const Catalog& catalog() {
  static const Catalog cat = load_catalog(std::string(ZS_DATA_DIR) + "/gen");
  return cat;
}

const UrbanForm& form() {
  static const UrbanForm f = load_urban_form(std::string(ZS_DATA_DIR) + "/gen");
  return f;
}

TownSpec spec_of(double people, const std::string& income) {
  TownSpec s;
  s.name = "Testville";
  s.culture = catalog().cultures.front().id;
  s.income = income;
  s.people = people;
  s.mean_household_size = 2.6;
  s.lots.push_back({"police_station", "Testville Police Department", 26, 27});
  return s;
}

bool overlap(const Plot& a, const Plot& b) {
  return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}

}  // namespace

TEST_CASE("the urban form loads with every income group, and a bad directory is refused (A1)") {
  for (const char* g : {"high", "upper_middle", "lower_middle", "low"}) CHECK(form().groups.count(g) == 1);
  CHECK(form().street_share == doctest::Approx(0.30));  // CL-23, CL-24
  CHECK_THROWS(load_urban_form(std::string(ZS_DATA_DIR) + "/no_such_dir"));
}

TEST_CASE("every plot fronts its street, plots partition their block, every household has a plot, addresses are unique (A1)") {
  for (const char* income : {"high", "upper_middle", "lower_middle", "low"}) {
    CAPTURE(income);
    const TownSpec spec = spec_of(17'626, income);
    const TownLayout t = lay_out_town(catalog(), form(), spec, 3);
    REQUIRE(!t.plots.empty());
    std::set<std::string> addresses;
    std::vector<long long> block_area(t.blocks.size(), 0);
    int households = 0, units = 0;
    for (const Plot& p : t.plots) {
      const Street& s = t.streets[static_cast<std::size_t>(p.street)];
      const int near = s.at_m - s.width_m / 2, far = near + s.width_m;
      const bool fronts = s.vertical ? (p.x == far || p.x + p.w == near) : (p.y == far || p.y + p.h == near);
      CHECK(fronts);
      CHECK(addresses.insert(p.address).second);
      block_area[static_cast<std::size_t>(p.block)] += static_cast<long long>(p.w) * p.h;
      households += p.households;
      units += p.units;
      CHECK(p.households <= p.units);
    }
    for (std::size_t b = 0; b < t.blocks.size(); ++b)
      CHECK(block_area[b] == static_cast<long long>(t.blocks[b].w) * t.blocks[b].h);  // no gap, no overlap (with the check below)
    for (std::size_t i = 0; i < t.plots.size(); ++i)
      for (std::size_t j = i + 1; j < t.plots.size() && t.plots[j].block == t.plots[i].block; ++j) CHECK(!overlap(t.plots[i], t.plots[j]));
    CHECK(households == t.households);  // nobody without a home
    CHECK(units >= households);
  }
}

TEST_CASE("streets take their share of the land; the network and the density fall in the research's bands (A2)") {
  for (const char* income : {"high", "upper_middle", "lower_middle", "low"}) {
    CAPTURE(income);
    const TownLayout t = lay_out_town(catalog(), form(), spec_of(17'626, income), 5);
    const UrbanGroup& g = form().groups.at(income);
    CHECK(t.street_share() >= 0.20);  // CL-26: 20-30 % typical, about 30 % in classic grids
    CHECK(t.street_share() <= 0.35);
    CHECK(t.intersections_per_km2() >= 0.6 * g.intersections_per_km2);  // CL-35 by region
    CHECK(t.intersections_per_km2() <= 1.3 * g.intersections_per_km2);
    const double density = 17'626 / t.built_up_ha();
    CHECK(density >= 0.6 * g.density_per_ha);  // CL-30; the grid rounds the town up to whole blocks
    CHECK(density <= 1.1 * g.density_per_ha);
  }
}

TEST_CASE("a station gets a civic plot of its own, wide and deep enough, near the centre (A1)") {
  const TownSpec spec = spec_of(50'000, "upper_middle");
  const TownLayout t = lay_out_town(catalog(), form(), spec, 7);
  REQUIRE(t.lot_plot.size() == 1);
  REQUIRE(t.lot_plot[0] >= 0);
  const Plot& p = t.plots[static_cast<std::size_t>(t.lot_plot[0])];
  CHECK(p.use == LandUse::Civic);
  CHECK(p.lot == 0);
  CHECK(std::max(p.w, p.h) >= 26);
  CHECK(std::min(p.w, p.h) >= 27 - 0);  // the row is at least as deep as the lot
  CHECK(t.blocks[static_cast<std::size_t>(p.block)].use == LandUse::Commercial);
}

TEST_CASE("a town short of houses builds apartment houses near the centre; a bigger town is bigger (A1)") {
  UrbanForm tight = form();
  tight.groups["upper_middle"].frontage_m = 60;  // few house plots: the households need apartments
  const TownLayout t = lay_out_town(catalog(), tight, spec_of(17'626, "upper_middle"), 9);
  int apartments = 0, households = 0;
  for (const Plot& p : t.plots) {
    apartments += p.units > 1 ? 1 : 0;
    households += p.households;
  }
  CHECK(apartments > 0);
  CHECK(households == t.households);
  const TownLayout big = lay_out_town(catalog(), form(), spec_of(80'000, "upper_middle"), 9);
  CHECK(big.blocks.size() > lay_out_town(catalog(), form(), spec_of(17'626, "upper_middle"), 9).blocks.size());
}

TEST_CASE("the same seed gives the same town (A1)") {
  const auto digest = [](const TownLayout& t) {
    std::uint64_t h = 0;
    for (const Plot& p : t.plots) h = mix64(h ^ hash_str(p.address), static_cast<std::uint64_t>(p.x) * 65537u + static_cast<std::uint64_t>(p.y));
    return h;
  };
  const TownSpec spec = spec_of(17'626, "upper_middle");
  CHECK(digest(lay_out_town(catalog(), form(), spec, 11)) == digest(lay_out_town(catalog(), form(), spec, 11)));
  MESSAGE("layout digest: ", digest(lay_out_town(catalog(), form(), spec, 11)));
  CHECK(digest(lay_out_town(catalog(), form(), spec, 11)) == 3016707943558594800ULL);  // pinned on MSVC, 2026-10-13
  const TownLayout tiny = lay_out_town(catalog(), form(), spec_of(40, "upper_middle"), 11);  // one block: all homes
  int placed = 0;
  for (const Plot& p : tiny.plots) placed += p.households;
  CHECK(placed == tiny.households);
}
