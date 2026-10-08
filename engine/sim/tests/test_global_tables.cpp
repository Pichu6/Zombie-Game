// Spec 43: the global tables (UN WPP 2024) and the base countries. The fitted presets load, and the simulation's own
// draws reproduce the tables: life expectancy at 15 from the mortality draws, age-specific fertility and the total
// fertility rate from the birth draws. The registry's version-6 save keeps the age-specific fertility.
#include <doctest.h>

#include <cmath>
#include <fstream>
#include <iterator>
#include <string>

#include "toml.hpp"
#include "zs/demographics.h"
#include "zs/demography.h"
#include "zs/pop.h"
#include "zs/pop_registry.h"
#include "zs/world.h"

using namespace zs;

namespace {

const char* const kLevels[] = {"high", "upper_middle", "lower_middle", "low"};

std::string data() { return ZS_DATA_DIR; }

toml::table world_tables() { return toml::parse_file(data() + "/setup/world/wpp2024.toml"); }

std::vector<double> numbers(const toml::table& t, const std::string& key) {
  std::vector<double> out;
  for (const auto& e : *t[key].as_array()) out.push_back(*e.value<double>());
  return out;
}

PopParams params_of(const std::string& level) {
  return make_pop_params(load_nation(data() + "/setup/nations/base_" + level + ".toml"), data());
}

}  // namespace

TEST_CASE("global tables: every base country and its preset load, with the sourced pyramid and fertility") {
  const toml::table w = world_tables();
  for (const char* level : kLevels) {
    const NationDemographics n = load_nation(data() + "/setup/nations/base_" + std::string(level) + ".toml");
    CHECK_FALSE(n.sourced);  // other tables are still placeholders
    CHECK(n.age_pyramid_source.text.find("World Population Prospects 2024") != std::string::npos);
    const PopParams p = params_of(level);
    CHECK(p.fertility.hazard_by_band.size() == 7);
    CHECK(p.fertility.end_age == 50.0);
    const double srb = *w[level]["sex_ratio_at_birth"].value<double>();
    CHECK(p.male_share_at_birth == doctest::Approx(srb / (100 + srb)).epsilon(1e-5));
  }
}

TEST_CASE("global tables (spec 45): base countries carry the sourced households and sectors of their group") {
  const toml::table w = world_tables();
  double last_mean = 0;
  for (const char* level : kLevels) {  // richest first: households grow as income falls
    const NationDemographics n = load_nation(data() + "/setup/nations/base_" + std::string(level) + ".toml");
    CHECK(n.household_source.text.find("Household Size and Composition 2022") != std::string::npos);
    CHECK(n.sector_source.text.find("ILO modelled estimates") != std::string::npos);
    const double mean = *w[level]["household_mean_size"].value<double>();
    CHECK(mean_household_size(n) == doctest::Approx(mean).epsilon(1e-3));
    CHECK(mean > last_mean);
    last_mean = mean;
    const std::vector<double> sectors = numbers(*w[level].as_table(), "employment_by_sector");
    for (std::size_t k = 0; k < 3; ++k) CHECK(n.sectors[k] == doctest::Approx(sectors[k]).epsilon(1e-5));
    CHECK(*w[level]["household_coverage"].value<double>() > 0.9);
  }
}

TEST_CASE("global tables: the mortality draws give WPP's life expectancy at 15 (within half a year)"
          * doctest::skip(World::kDebugBuild)) {
  for (const char* level : kLevels) {
    const PopParams p = params_of(level);
    const toml::table pre = toml::parse_file(data() + "/setup/demography/wpp2024_" + std::string(level) + ".toml");
    for (bool male : {true, false}) {
      double sum = 0;
      int n = 0;
      for (PopId id = 1; id <= 200'000; ++id) {
        const PopBase b = derive_base(42, id, -15.0, 15.0, p, male ? PopSex::Male : PopSex::Female);
        sum += b.death_year;  // born at -15, alive at 15 at year 0: the death year is the remaining life
        ++n;
      }
      const double model = *pre["check"][male ? "life_expectancy_15_male" : "life_expectancy_15_female"].value<double>();
      CHECK(sum / n == doctest::Approx(model).epsilon(0.01));  // the draws follow the fitted hazards
      MESSAGE(std::string(level) << std::string(male ? " male" : " female") << " e15 drawn " << sum / n << ", fitted " << model);
    }
  }
}

TEST_CASE("global tables: the birth draws give WPP's age-specific fertility and total fertility rate"
          * doctest::skip(World::kDebugBuild)) {
  const toml::table w = world_tables();
  for (const char* level : kLevels) {
    const PopParams p = params_of(level);
    const std::vector<double> asfr = numbers(*w[level].as_table(), "asfr");
    std::vector<double> births(7, 0.0);
    const int women = 200'000;
    for (PopId id = 1; id <= static_cast<PopId>(women); ++id) {
      PopBase b{id, PopSex::Female, 0.0, 1000.0};  // born at year 0, outlives her fertile years
      for (const Birth& x : births_of(7, id, b, -1.0, p)) births[static_cast<std::size_t>((x.year - 15.0) / 5.0)] += 1;
    }
    double tfr = 0, tfr_wpp = 0;
    for (std::size_t k = 0; k < 7; ++k) {
      const double rate = births[k] / (5.0 * women);
      tfr += 5 * rate;
      tfr_wpp += 5 * asfr[k];
      if (asfr[k] > 0.005) CHECK(rate == doctest::Approx(asfr[k]).epsilon(0.03));
    }
    CHECK(tfr == doctest::Approx(tfr_wpp).epsilon(0.01));
    MESSAGE(std::string(level) << " TFR drawn " << tfr << ", WPP 15-49 " << tfr_wpp);
  }
}

TEST_CASE("global tables (spec 44): the draws give WPP's life expectancy at birth and infant mortality"
          * doctest::skip(World::kDebugBuild)) {
  const toml::table w = world_tables();
  for (const char* level : kLevels) {
    const PopParams p = params_of(level);
    const toml::table pre = toml::parse_file(data() + "/setup/demography/wpp2024_" + std::string(level) + ".toml");
    for (bool male : {true, false}) {
      double sum = 0;
      int infants = 0;
      const int n = 400'000;
      for (PopId id = 1; id <= static_cast<PopId>(n); ++id) {
        const PopBase b = derive_base(43, id, 0.0, 0.0, p, male ? PopSex::Male : PopSex::Female);
        sum += b.death_year;
        infants += b.death_year < 1.0;
      }
      const double model = *pre["check"][male ? "life_expectancy_0_male" : "life_expectancy_0_female"].value<double>();
      const double wpp = *w[level][male ? "life_expectancy_male" : "life_expectancy_female"].value<double>();
      const double q0 = numbers(*w[level].as_table(), male ? "life_qx_male" : "life_qx_female")[0];
      CHECK(sum / n == doctest::Approx(model).epsilon(0.005));
      CHECK(sum / n == doctest::Approx(wpp).epsilon(0.01));
      CHECK(static_cast<double>(infants) / n == doctest::Approx(q0).epsilon(0.1));
      MESSAGE(std::string(level) << std::string(male ? " male" : " female") << " e0 drawn " << sum / n << ", WPP " << wpp
                                 << "; infant deaths drawn " << 1000.0 * infants / n << " per 1000, WPP " << 1000 * q0);
    }
  }
}

namespace {
void check_fixture(const std::vector<std::uint8_t>& bytes, const std::string& name) {
  const std::string path = std::string(ZS_FIXTURES_DIR) + "/" + name;
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    FAIL("wrote the fixture " << name << "; commit it and run again");
  }
  const std::vector<std::uint8_t> fixture((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  PopRegistry from_fixture(3);
  from_fixture.load(fixture);
  CHECK(from_fixture.save() == fixture);
  CHECK(fixture == bytes);
}
}  // namespace

TEST_CASE("global tables: a registry with the child hazard saves as version 7 and loads back") {
  PopRegistry reg(3, params_of("low"));
  reg.add_cohort({1, 0, 0, 50});
  reg.set_death_year(9, 61.5);
  const auto bytes = reg.save();
  PopRegistry loaded(3);
  loaded.load(bytes);
  CHECK(loaded.params() == reg.params());
  CHECK(loaded.save() == bytes);
  // fixtures/popreg_v7.bin was written from this registry when version 7 shipped. Never regenerate it.
  check_fixture(bytes, "popreg_v7.bin");
}

TEST_CASE("global tables: the version-6 fixture still loads and re-saves to the same bytes") {
  // fixtures/popreg_v6.bin was written when version 6 shipped (spec 43, before the child hazard). Never regenerate it:
  // if this fails, the format changed and needs a new version and a new reader beside this one.
  std::ifstream in(std::string(ZS_FIXTURES_DIR) + "/popreg_v6.bin", std::ios::binary);
  REQUIRE(in);
  const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  PopRegistry loaded(3);
  loaded.load(bytes);
  CHECK(loaded.save() == bytes);
  CHECK(loaded.params().fertility.hazard_by_band.size() == 7);
  CHECK(loaded.params().female.child_hazard == 0.0);
}
