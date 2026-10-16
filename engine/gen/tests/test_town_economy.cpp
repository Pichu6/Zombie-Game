// Spec 98: the town scaled: workforce, jobs by the country's occupations, farm work, the region's plants, workplaces.
#include <doctest.h>

#include <set>

#include "zs/demographics.h"
#include "zs/gen/place.h"
#include "zs/gen/town_economy.h"
#include "zs/nations.h"
#include "zs/tier0.h"

using namespace zs::gen;

namespace {
struct Fixture {
  Catalog cat = load_catalog(zs::tier0::data_dir() + "/gen");
  TownEconomyParams ep = load_town_economy(zs::tier0::data_dir() + "/gen");
  zs::NationDemographics high = zs::load_nation(zs::tier0::data_dir() + "/setup/nations/base_high.toml");
  zs::NationDemographics low = zs::load_nation(zs::tier0::data_dir() + "/setup/nations/base_low.toml");
  TownEconomy plan(const zs::NationDemographics& n, double people, const std::vector<TownPlant>& plants = {}) const {
    return plan_town_economy(cat, n, ep, people, plants, false, {}, "Testville", cat.cultures.front().id, 1);
  }
};
double sum(const std::map<std::string, double>& m) {
  double t = 0;
  for (const auto& [k, v] : m) t += v;
  return t;
}
}  // namespace

TEST_CASE("town economy: the employed are the labour force less the unemployed, all of them in a job (rules 1, 2)") {
  const Fixture f;
  const TownEconomy e = f.plan(f.high, 20000);
  CHECK(e.labour_force > 0.35 * 20000);
  CHECK(e.labour_force < 0.65 * 20000);
  CHECK(e.employed == doctest::Approx(e.labour_force * (1 - f.high.unemployment_rate)));
  CHECK(sum(e.jobs) == doctest::Approx(e.employed));
  // Managers are the one job of their group: the country's share of the employed.
  CHECK(e.jobs.at("manager") == doctest::Approx(e.employed * f.high.occupations[0]));
}

TEST_CASE("town economy: farm work follows the country's agriculture share and the town's size (rule 3)") {
  const Fixture f;
  const TownEconomy rich = f.plan(f.high, 20000), poor = f.plan(f.low, 20000), poor_city = f.plan(f.low, 2000000);
  CHECK(poor.by_sector[0] / poor.employed > 5 * rich.by_sector[0] / rich.employed);
  CHECK(poor_city.by_sector[0] / poor_city.employed < poor.by_sector[0] / poor.employed);
  CHECK(sum(poor.jobs) == doctest::Approx(poor.employed));
}

TEST_CASE("town economy: the region's plants hire their staff from the town, the workforce unchanged (rule 4)") {
  const Fixture f;
  const TownEconomy plain = f.plan(f.high, 20000);
  const TownEconomy steel = f.plan(f.high, 20000, {{"Steel works", 2000}});
  CHECK(steel.plants == 2000);
  CHECK(steel.employed == doctest::Approx(plain.employed));
  CHECK(steel.by_sector[1] > plain.by_sector[1] + 1000);
  int plants = 0;
  for (const TownWorkplace& w : steel.workplaces) plants += w.type == "plant";
  CHECK(plants == 1);
}

TEST_CASE("town economy: workplaces by the jobs they hold; a small town's hospital jobs are in a larger town (rule 6)") {
  const Fixture f;
  const TownEconomy town = f.plan(f.high, 20000), village = f.plan(f.high, 1500);
  auto count = [](const TownEconomy& e, const char* type) {
    int n = 0;
    for (const TownWorkplace& w : e.workplaces) n += w.type == type;
    return n;
  };
  CHECK(count(town, "hospital") >= 1);
  CHECK(count(village, "hospital") == 0);
  CHECK(village.commuting > 0);
  CHECK(count(town, "office") > count(village, "office"));
  for (const TownWorkplace& w : town.workplaces) CHECK(w.staff > 0);
}

TEST_CASE("town economy: the one world's town on its plots, its groceries the food outlets (rules 5, 6)") {
  const GeneratedWorld w = generate_world(1, {});
  const int town = choose_town(w.fields, w.people, 1);
  REQUIRE(town >= 0);
  const zs::Place place = place_of_town(w.grid, w.fields, w.people, town, 1);
  const auto rec = town_record(place, 1);
  const TownEconomy e = town_economy(w, town, place, *rec, 1);
  MESSAGE(describe_economy(e));
  std::set<int> plots;
  int groceries = 0;
  for (const TownWorkplace& x : e.workplaces) {
    groceries += x.type == "grocery";
    if (x.plot < 0 || x.type == "grocery" || x.type == "gas_station") continue;
    CHECK(plots.insert(x.plot).second);  // one workplace a plot
    CHECK(rec->layout.plots[static_cast<std::size_t>(x.plot)].use != LandUse::Residential);
  }
  CHECK(groceries > 0);
}
