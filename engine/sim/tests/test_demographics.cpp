// Spec 28 and sandbox parameters section 5: a nation's own demographics, as data.
#include <doctest.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "zs/demographics.h"
#include "zs/pop_registry.h"

using namespace zs;

namespace {
std::string placeholder_path() { return std::string(ZS_DATA_DIR) + "/setup/nations/placeholder.toml"; }

std::string read_text(const std::string& path) {
  std::ifstream f(path);
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

std::string write_temp(const std::string& name, const std::string& text) {
  const auto path = std::filesystem::temp_directory_path() / ("zs_test_nation_" + name);
  std::ofstream(path) << text;
  return path.string();
}

// The placeholder file with one piece of text replaced.
std::string edited(const std::string& name, const std::string& from, const std::string& to) {
  std::string text = read_text(placeholder_path());
  const auto at = text.find(from);
  REQUIRE(at != std::string::npos);
  text.replace(at, from.size(), to);
  return write_temp(name, text);
}
}  // namespace

TEST_CASE("the placeholder nation loads, every table adds up, and it says it is not data (A1)") {
  const NationDemographics n = load_nation(placeholder_path());
  CHECK(n.id == "placeholder");
  CHECK_FALSE(n.sourced);
  double sum = 0;
  for (double x : n.age_pyramid) sum += x;
  CHECK(sum == doctest::Approx(1.0).epsilon(1e-9));
  CHECK(n.age_pyramid.size() == 20);
  for (const auto& group : n.education) { double g = 0; for (double x : group) g += x; CHECK(g == doctest::Approx(1.0).epsilon(1e-9)); }
  double occ = 0; for (double x : n.occupations) occ += x;
  CHECK(occ == doctest::Approx(1.0).epsilon(1e-9));
  CHECK(mean_household_size(n) > 2.2);
  CHECK(mean_household_size(n) < 3.2);
  CHECK(share_under(n, 0) == 0.0);
  CHECK(share_under(n, 100) == doctest::Approx(1.0).epsilon(1e-9));
  CHECK(share_under(n, 15) > 0.14);
  CHECK(share_under(n, 15) < 0.20);
  CHECK(std::string(occupation_group_name(1)) == "managers");
  CHECK(std::string(sector_name(2)) == "services");
  CHECK(std::string(education_level_name(3)) == "tertiary");
}

TEST_CASE("a profile with a bad table is refused with a plain message (A1)") {
  CHECK_THROWS_WITH_AS(load_nation(edited("unknown.toml", "[settlement]", "[settlement]\nbogus = 1")), doctest::Contains("unknown key 'bogus'"), std::runtime_error);
  CHECK_THROWS_WITH_AS(load_nation(edited("sum.toml", "shares = [0.04, 0.22, 0.74]", "shares = [0.10, 0.22, 0.74]")), doctest::Contains("add up to 1"), std::runtime_error);
  CHECK_THROWS_WITH_AS(load_nation(edited("count.toml", "sizes = [0.28, 0.33, 0.16, 0.14, 0.09]", "sizes = [0.28, 0.33, 0.16, 0.23]")), doctest::Contains("needs 5 numbers"), std::runtime_error);
  CHECK_THROWS_WITH_AS(load_nation(edited("negative.toml", "age_25_34 = [0.02, 0.06, 0.52, 0.40]", "age_25_34 = [-0.02, 0.10, 0.52, 0.40]")), doctest::Contains("negative"), std::runtime_error);
  CHECK_THROWS_WITH_AS(load_nation(edited("rate.toml", "unemployment_rate = 0.06", "unemployment_rate = 1.4")), doctest::Contains("between 0 and 1"), std::runtime_error);
  CHECK_THROWS_WITH_AS(load_nation(edited("retire.toml", "retirement_age = 65", "retirement_age = 30")), doctest::Contains("retirement_age"), std::runtime_error);
  CHECK_THROWS_WITH_AS(load_nation(edited("status.toml", "status = \"placeholder\"", "status = \"maybe\"")), doctest::Contains("status must be"), std::runtime_error);
  CHECK_THROWS_WITH_AS(load_nation(edited("noid.toml", "id = \"placeholder\"", "")), doctest::Contains("needs an id"), std::runtime_error);
  CHECK_THROWS_WITH_AS(load_nation(edited("notable.toml", "[sectors]            # of the employed: agriculture, industry, services\nshares = [0.04, 0.22, 0.74]\nsource = \"shape only, not data\"\nyear = 0\n", "")),
                       doctest::Contains("missing table [sectors]"), std::runtime_error);
  CHECK_THROWS(load_nation(write_temp("garbage.toml", "this is [[[ not toml")));
}

TEST_CASE("a profile claiming to be sourced must cite every table; with citations it loads (A1)") {
  // Marking the placeholder "sourced" without citations is refused: invented numbers cannot pass as data.
  CHECK_THROWS_WITH_AS(load_nation(edited("fake_sourced.toml", "status = \"placeholder\"", "status = \"sourced\"")), doctest::Contains("needs a source and a year"), std::runtime_error);
  std::string text = read_text(placeholder_path());
  for (std::size_t at = text.find("source = \"shape only, not data\"\nyear = 0"); at != std::string::npos; at = text.find("source = \"shape only, not data\"\nyear = 0"))
    text.replace(at, std::string("source = \"shape only, not data\"\nyear = 0").size(), "source = \"A Test Statistics Office, table 1\"\nyear = 2020");
  text.replace(text.find("status = \"placeholder\""), std::string("status = \"placeholder\"").size(), "status = \"sourced\"");
  const NationDemographics n = load_nation(write_temp("sourced_ok.toml", text));
  CHECK(n.sourced);
  CHECK(n.age_pyramid_source.year == 2020);
  CHECK(n.labour_source.text == "A Test Statistics Office, table 1");
}

TEST_CASE("a nation's pop parameters carry its age pyramid and the demography preset it names (A1)") {
  const NationDemographics n = load_nation(placeholder_path());
  const PopParams p = make_pop_params(n, ZS_DATA_DIR);
  CHECK(p.age_pyramid == n.age_pyramid);
  CHECK(p.female == default_pop_params().female);
  CHECK(p.fertility == default_pop_params().fertility);
}

TEST_CASE("founders drawn by rule follow the nation's age pyramid, and different nations differ (A2)") {
  const NationDemographics n = load_nation(placeholder_path());
  PopParams young = make_pop_params(n, ZS_DATA_DIR), old = young;
  for (int b = 0; b < kAgeBands; ++b) {  // a young nation and an old one, same total
    young.age_pyramid[static_cast<std::size_t>(b)] = b < 8 ? 0.11 : (b < 14 ? 0.02 : 0.0);
    old.age_pyramid[static_cast<std::size_t>(b)] = b < 6 ? 0.02 : 0.06 * (b < 14 ? 1.0 : 0.5);
  }
  for (auto* v : {&young.age_pyramid, &old.age_pyramid}) { double s = 0; for (double x : *v) s += x; for (double& x : *v) x /= s; }
  PopRegistry a(71, make_pop_params(n, ZS_DATA_DIR)), y(71, young), o(71, old);
  const std::uint32_t count = 40000;
  for (PopRegistry* r : {&a, &y, &o}) REQUIRE(r->add_cohort({1, 0, 0, count}));
  std::vector<double> share(kAgeBands, 0.0);
  double mean_a = 0, mean_y = 0, mean_o = 0;
  for (std::uint32_t i = 0; i < count; ++i) {
    PopSnapshot s;
    REQUIRE(a.snapshot(i, 0.0, s));
    const double age = -s.base.birth_year;
    REQUIRE(age >= 0.0);
    REQUIRE(age < 100.0);
    share[static_cast<std::size_t>(age / 5.0)] += 1.0 / count;
    mean_a += age / count;
    REQUIRE(y.snapshot(i, 0.0, s)); mean_y += -s.base.birth_year / count;
    REQUIRE(o.snapshot(i, 0.0, s)); mean_o += -s.base.birth_year / count;
  }
  for (int b = 0; b < kAgeBands; ++b) {
    const double p = n.age_pyramid[static_cast<std::size_t>(b)];
    CHECK(std::fabs(share[static_cast<std::size_t>(b)] - p) < 5 * std::sqrt(p * (1 - p) / count) + 1e-4);
  }
  CHECK(mean_y < mean_a);
  CHECK(mean_a < mean_o);
  CHECK(mean_o - mean_y > 15.0);
}

TEST_CASE("a world keeps its nation's age pyramid through save and load (A1)") {
  const NationDemographics n = load_nation(placeholder_path());
  PopRegistry reg(72, make_pop_params(n, ZS_DATA_DIR));
  REQUIRE(reg.add_cohort({1, 0, 0, 500}));
  PopRegistry loaded(72);  // made with the plain defaults
  loaded.load(reg.save());
  CHECK(loaded.params().age_pyramid == n.age_pyramid);
  CHECK(loaded.save() == reg.save());
  PopSnapshot a, b;
  REQUIRE(reg.snapshot(33, 0.0, a));
  REQUIRE(loaded.snapshot(33, 0.0, b));
  CHECK(a.base.birth_year == b.base.birth_year);
}

TEST_CASE("a save made by version 3 still loads and gives the same pops (frozen fixture, A1)") {
  // fixtures/popreg_v3.bin is a version-3 save of a registry with generated founders, rosters of different kinds and
  // events (made by converting the version-4 bytes: remove the empty age pyramid from the params, stamp version 3)
  // and written once. Never regenerate it.
  PopRegistry loaded(81);
  loaded.load(read_file(std::string(ZS_FIXTURES_DIR) + "/popreg_v3.bin"));
  CHECK(loaded.params() == default_pop_params());
  CHECK(loaded.params().age_pyramid.empty());           // a version-3 world drew founders by the placeholder rule
  CHECK(loaded.counts().total == 400);
  CHECK(loaded.roster(1)->kind() == RosterKind::Workplace);
  CHECK(loaded.roster(2)->kind() == RosterKind::Household);
  CHECK(loaded.roster(3)->kind() == RosterKind::School);
  CHECK(loaded.roster_count(1, EffectKind::LaidOff, 3.0) == 12);
  CHECK(loaded.count(1, EffectKind::Ill, 3.0) == 30);
  PopSnapshot s;
  REQUIRE(loaded.snapshot(5, 0.0, s));
  CHECK(s.base.sex == PopSex::Male);                        // the generated attributes survived
  CHECK(static_cast<int>(-s.base.birth_year) == 25);
  REQUIRE(loaded.snapshot(6, 0.0, s));
  CHECK(s.base.sex == PopSex::Female);
  CHECK(static_cast<int>(-s.base.birth_year) == 26);
  CHECK(loaded.hash() != 0);
}
