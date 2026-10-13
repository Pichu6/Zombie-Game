// Spec 72: what stands on a town's plots. Every plot's building loads as a site map that fits its plot, with every free
// cell reachable from the street; houses and flats have a bed for each member; households follow the country's sizes;
// the Tier 0 block stands on a residential plot of its own; the same plot always gives the same building.
#include <doctest.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <memory>

#include "zs/demographics.h"
#include "zs/gen/keyed.h"
#include "zs/gen/plot_contents.h"
#include "zs/site_map.h"
#include "zs/world.h"

using namespace zs::gen;

namespace {

const Catalog& catalog() {
  static const Catalog cat = load_catalog(std::string(ZS_DATA_DIR) + "/gen");
  return cat;
}
const DwellingRules& rules() {
  static const DwellingRules r = load_dwelling_rules(std::string(ZS_DATA_DIR) + "/gen");
  return r;
}
const PlotRules& plot_rules() {
  static const PlotRules r = load_plot_rules(std::string(ZS_DATA_DIR) + "/gen");
  return r;
}

TownSpec spec_of(const std::string& income, double people = 6'000) {
  const zs::NationDemographics nation = zs::load_nation(std::string(ZS_DATA_DIR) + "/setup/nations/base_" + income + ".toml");
  TownSpec s;
  s.name = "Testville";
  s.culture = catalog().cultures.front().id;
  s.income = income;
  s.people = people;
  s.mean_household_size = zs::mean_household_size(nation);
  s.household_shares = nation.household_sizes;
  s.lots.push_back({"police_station", "Testville Police Department", 22, 27});
  s.lots.push_back({"tier0_block", "the Tier 0 block", 32, 28, LandUse::Residential, 20});
  return s;
}

struct Loaded {
  zs::World world{1};
  zs::Assemblies* items;
  zs::SiteMap* map;
  explicit Loaded(const std::string& toml_text) {
    auto a = std::make_unique<zs::Assemblies>();
    auto m = std::make_unique<zs::SiteMap>();
    items = a.get();
    map = m.get();
    world.add_system(std::move(a));
    world.add_system(std::move(m));
    zs::load_item_defs(*items, std::string(ZS_DATA_DIR) + "/tier0/items.toml");
    const std::string path = (std::filesystem::temp_directory_path() / "zs_spec72_plot.toml").string();
    {
      std::ofstream(path, std::ios::binary) << toml_text;
    }
    map->load_file(world, *items, path);
    world.close_setup();
  }
};

int count_of(const zs::SiteMap& m, const std::string& kind) {
  return static_cast<int>(std::count_if(m.objects().begin(), m.objects().end(), [&](const zs::MapObject& o) { return o.kind == kind; }));
}

}  // namespace

TEST_CASE("the plot rules load, and a bad directory is refused (A1)") {
  CHECK(plot_rules().coverage_commercial > 0);
  CHECK_THROWS(load_plot_rules(std::string(ZS_DATA_DIR) + "/no_such_dir"));
}

TEST_CASE("a plot's households follow the country's household sizes (A2)") {
  const TownSpec spec = spec_of("upper_middle", 20'000);
  const TownLayout t = lay_out_town(catalog(), load_urban_form(std::string(ZS_DATA_DIR) + "/gen"), spec, 3);
  double members = 0, households = 0;
  for (int i = 0; i < static_cast<int>(t.plots.size()); ++i) {
    const auto homes = homes_on(spec, t, i, plot_rules(), 3);
    CHECK(static_cast<int>(homes.size()) == t.plots[static_cast<std::size_t>(i)].households);
    for (const Home& h : homes) {
      members += h.members;
      CHECK((h.couple ? h.members >= 2 : true));
    }
    households += static_cast<double>(homes.size());
  }
  REQUIRE(households > 0);
  CHECK(members / households == doctest::Approx(spec.mean_household_size).epsilon(0.12));  // spec 45's sizes, drawn
}

TEST_CASE("every kind of plot loads as a site map that fits its plot, every free cell reachable from the street (A1)") {
  for (const char* income : {"high", "upper_middle", "low"}) {
    CAPTURE(income);
    const TownSpec spec = spec_of(income);
    const TownLayout t = lay_out_town(catalog(), load_urban_form(std::string(ZS_DATA_DIR) + "/gen"), spec, 5);
    std::map<std::string, int> seen;
    for (int i = 0; i < static_cast<int>(t.plots.size()); ++i) {
      const PlotPlan plan = plan_plot(spec, t, i, rules(), plot_rules(), 5);
      if (seen[plan.kind]++ >= 3) continue;  // three of each kind per town
      CAPTURE(plan.kind);
      CAPTURE(i);
      const std::string toml = plot_toml(spec, t, i, rules(), plot_rules(), 5);
      if (plan.kind == "civic" || plan.kind == "block") {
        CHECK(toml.empty());
        continue;
      }
      const Loaded built(toml);
      const zs::SiteMap& m = *built.map;
      CHECK(m.width() == plan.frontage_m);
      CHECK(m.height() == plan.depth_m);
      const zs::Cell street{0, plan.kind == "park" ? 0 : plan.building.house_x + plan.building.corridor_x, 0};
      for (int z : m.levels())
        for (int y = 0; y < m.height(); ++y)
          for (int x = 0; x < m.width(); ++x) {
            const zs::Cell c{z, x, y};
            if (!m.passable(c) || c == street) continue;
            if (m.find_path(street, c).empty()) {
              CAPTURE(z);
              CAPTURE(x);
              CAPTURE(y);
              FAIL_CHECK("a free cell cannot be reached");
            }
          }
      if (plan.kind == "house" && !plan.homes.empty()) CHECK(count_of(m, "bed") == plan.homes.front().members);
      if (plan.kind == "apartments") {
        int beds = 0;
        for (const Room& r : plan.building.rooms) beds += r.type == "flat" ? r.occupants : 0;
        CHECK(count_of(m, "bed") == beds);
        CHECK(count_of(m, "stove") == t.plots[static_cast<std::size_t>(i)].units);  // a kitchen in every flat
        CHECK(static_cast<int>(m.stairs().size()) == plan.building.floors - 1);
      }
      if (plan.kind == "shop") CHECK(count_of(m, "desk") >= 1);
      if (plan.kind == "workshop") CHECK(count_of(m, "workbench") == 1);
      if (plan.kind == "park") CHECK(count_of(m, "tree") >= 1);
    }
    CHECK(seen["house"] + seen["apartments"] > 0);
    CHECK(seen["shop"] > 0);
    CHECK(seen["park"] > 0);
  }
}

TEST_CASE("the Tier 0 block stands on a residential plot with its census households; the station on a civic one (A1)") {
  const TownSpec spec = spec_of("upper_middle", 17'626);
  const TownLayout t = lay_out_town(catalog(), load_urban_form(std::string(ZS_DATA_DIR) + "/gen"), spec, 7);
  REQUIRE(t.lot_plot.size() == 2);
  REQUIRE(t.lot_plot[1] >= 0);
  const Plot& block = t.plots[static_cast<std::size_t>(t.lot_plot[1])];
  CHECK(block.use == LandUse::Residential);
  CHECK(block.households == 20);
  CHECK(block.w * block.h >= 32 * 28);
  CHECK(plan_plot(spec, t, t.lot_plot[1], rules(), plot_rules(), 7).kind == "block");
  CHECK(plan_plot(spec, t, t.lot_plot[0], rules(), plot_rules(), 7).kind == "civic");
  int households = 0;
  for (const Plot& p : t.plots) households += p.households;
  CHECK(households == t.households);  // the block's 20 are among the town's, not added to them
}

TEST_CASE("the same plot always gives the same building (A1)") {
  const TownSpec spec = spec_of("upper_middle");
  const TownLayout t = lay_out_town(catalog(), load_urban_form(std::string(ZS_DATA_DIR) + "/gen"), spec, 9);
  for (int i = 0; i < static_cast<int>(t.plots.size()); i += 97)
    CHECK(plot_toml(spec, t, i, rules(), plot_rules(), 9) == plot_toml(spec, t, i, rules(), plot_rules(), 9));
}
