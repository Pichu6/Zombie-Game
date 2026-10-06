// Spec 28: parameters as data, kept in the save; a versioned save format with a frozen fixture.
#include <doctest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "zs/archive.h"
#include "zs/pop_registry.h"

using namespace zs;

namespace {
std::string write_temp(const std::string& name, const std::string& text) {
  const auto path = std::filesystem::temp_directory_path() / ("zs_test_" + name);
  std::ofstream(path) << text;
  return path.string();
}

// The registry the fixture was made from. Changing this builder changes what the fixture must still load as.
PopRegistry fixture_registry() {
  PopRegistry reg(41);
  reg.add_cohort({1, 0, 0, 300});
  reg.add_cohort({2, 1, 1000, 200});
  reg.apply({1, EffectKind::LaidOff, 40, 1.0, 5});
  reg.apply({2, EffectKind::Ill, 25, 1.5, 6});
  reg.add_roster(1, 0);
  for (PopId p = 0; p < 60; ++p) reg.roster_join(1, p, 0.0);
  reg.roster_apply(1, EffectKind::LaidOff, 12, 2.0, 7);
  reg.roster_leave(1, 5, 3.0);
  reg.promote(9);
  reg.set_office(9, 3);
  reg.set_death_year(9, 61.5);
  reg.set_death_year(1100, 44.0);
  return reg;
}
}  // namespace

TEST_CASE("the preset file and the compiled-in defaults agree, so neither drifts alone (A1)") {
  const PopParams file = load_pop_params(std::string(ZS_DATA_DIR) + "/setup/demography/default.toml");
  CHECK(file == default_pop_params());
}

TEST_CASE("a preset with an unknown key, a wrong type or a nonsense value is refused; missing keys keep defaults (A1)") {
  CHECK_THROWS(load_pop_params(write_temp("unknown.toml", "bogus = 1\n")));
  CHECK_THROWS(load_pop_params(write_temp("unknown2.toml", "[fertility]\nbogus = 1\n")));
  CHECK_THROWS(load_pop_params(write_temp("type.toml", "male_share_at_birth = \"half\"\n")));
  CHECK_THROWS(load_pop_params(write_temp("share.toml", "male_share_at_birth = 1.5\n")));
  CHECK_THROWS(load_pop_params(write_temp("hazard.toml", "[mortality_male]\ngompertz_c = -0.1\n")));
  CHECK_THROWS(load_pop_params(write_temp("missing_file.toml", "this is not toml [[[\n")));
  const PopParams partial = load_pop_params(write_temp("partial.toml", "male_share_at_birth = 0.5\n[fertility]\nchildless_share = 0.3\n"));
  CHECK(partial.male_share_at_birth == 0.5);
  CHECK(partial.fertility.childless_share == 0.3);
  CHECK(partial.fertility.gap_min == default_pop_params().fertility.gap_min);  // untouched
  CHECK(partial.male == default_pop_params().male);
}

TEST_CASE("a world keeps the parameters it was made with; retuning later never changes its pops (A1)") {
  PopParams harsh = default_pop_params();
  harsh.female.constant_hazard *= 20;  // a much more dangerous world
  harsh.male.constant_hazard *= 20;
  PopRegistry a(5, harsh), b(5);       // same seed, different parameters
  REQUIRE(a.add_cohort({1, 0, 0, 500}));
  REQUIRE(b.add_cohort({1, 0, 0, 500}));
  double sum_a = 0, sum_b = 0;
  for (PopId id = 0; id < 500; ++id) {
    PopSnapshot x, y;
    REQUIRE(a.snapshot(id, 0.0, x));
    REQUIRE(b.snapshot(id, 0.0, y));
    sum_a += x.base.death_year;
    sum_b += y.base.death_year;
  }
  CHECK(sum_a < sum_b);  // the harsh world dies sooner
  // Save the harsh world, load it into a registry made with the (changed) defaults: it keeps its own numbers.
  const auto bytes = a.save();
  PopRegistry loaded(5);
  loaded.load(bytes);
  CHECK(loaded.params() == harsh);
  CHECK(loaded.save() == bytes);
  for (PopId id = 0; id < 500; id += 17) {
    PopSnapshot x, y;
    REQUIRE(a.snapshot(id, 0.0, x));
    REQUIRE(loaded.snapshot(id, 0.0, y));
    CHECK(x.base.death_year == y.base.death_year);
  }
}

TEST_CASE("the save has a version, and a newer or alien save is refused with a clear error (A1)") {
  PopRegistry reg = fixture_registry();
  auto bytes = reg.save();
  REQUIRE(bytes.size() > 12);
  CHECK(bytes[8] == 3);  // the newest version, right after the 8-byte magic
  PopRegistry other(41);
  bytes[8] = 99;
  CHECK_THROWS_WITH_AS(other.load(bytes), doctest::Contains("version 99"), std::runtime_error);
  std::vector<std::uint8_t> junk(64, 7);
  CHECK_THROWS(other.load(junk));
}

TEST_CASE("a save made by version 1 still loads and gives the same pops (frozen fixture, A1)") {
  // fixtures/popreg_v1.bin was written from fixture_registry() when version 1 shipped. Never regenerate
  // it (there is deliberately no writer here): when the format changes, bump the version, keep this test, and
  // add a fixture for the new version.
  PopRegistry loaded(41);
  loaded.load(read_file(std::string(ZS_FIXTURES_DIR) + "/popreg_v1.bin"));
  const PopRegistry fresh = fixture_registry();
  CHECK(loaded.params() == default_pop_params());
  CHECK(loaded.counts().total == fresh.counts().total);
  CHECK(loaded.counts().records == fresh.counts().records);
  for (PopId id : {0u, 5u, 9u, 20u, 59u, 1000u, 1100u, 1199u}) {
    PopSnapshot x, y;
    REQUIRE(loaded.snapshot(id, 4.0, x));
    REQUIRE(fresh.snapshot(id, 4.0, y));
    CHECK(x.base.death_year == y.base.death_year);
    CHECK(x.base.birth_year == y.base.birth_year);
    CHECK(x.state_bits == y.state_bits);
    CHECK(x.roster_bits == y.roster_bits);
    CHECK(x.office == y.office);
    CHECK(x.level == y.level);
  }
  CHECK(loaded.count(1, EffectKind::LaidOff, 4.0) == 40);
  CHECK(loaded.roster_count(1, EffectKind::LaidOff, 4.0) == fresh.roster_count(1, EffectKind::LaidOff, 4.0));
  CHECK(loaded.roster_count(1, EffectKind::LaidOff, 4.0) == 12);  // 12 laid off; the one who left was not one of them
  CHECK(loaded.hash() == fresh.hash());                           // true while version 1 is the newest; then compare pops only
}

TEST_CASE("a save made by version 2 still loads and gives the same pops (frozen fixture, A1)") {
  // fixtures/popreg_v2.bin is a version-2 save of fixture_registry() (made by converting the version-3 bytes:
  // drop the trailing founder-spec section, stamp version 2) and written once. Never regenerate it.
  PopRegistry loaded(41);
  loaded.load(read_file(std::string(ZS_FIXTURES_DIR) + "/popreg_v2.bin"));
  const PopRegistry fresh = fixture_registry();
  CHECK(loaded.counts().records == fresh.counts().records);
  for (PopId id : {0u, 5u, 9u, 20u, 59u, 1000u, 1100u, 1199u}) {
    PopSnapshot x, y;
    REQUIRE(loaded.snapshot(id, 4.0, x));
    REQUIRE(fresh.snapshot(id, 4.0, y));
    CHECK(x.base.death_year == y.base.death_year);
    CHECK(x.base.sex == y.base.sex);
    CHECK(x.state_bits == y.state_bits);
    CHECK(x.roster_bits == y.roster_bits);
  }
  CHECK(loaded.hash() == fresh.hash());
}

TEST_CASE("generated founder attributes are honoured, saved, and drive the jump (A1)") {
  PopRegistry reg(61);
  REQUIRE(reg.add_cohort({1, 0, 0, 400}));
  std::vector<FounderSpec> specs;
  for (int i = 0; i < 400; ++i) specs.push_back({static_cast<std::uint16_t>(i < 200 ? 20 + i % 10 : 52 + i % 10), i % 2 ? Sex::Male : Sex::Female});
  CHECK_FALSE(reg.set_founder_specs(1, std::vector<FounderSpec>(399)));  // wrong size
  CHECK_FALSE(reg.set_founder_specs(9, specs));                          // no such cohort
  REQUIRE(reg.set_founder_specs(1, specs));
  CHECK_FALSE(reg.set_founder_specs(1, specs));                          // only once
  for (int i = 0; i < 400; ++i) {
    PopSnapshot s;
    REQUIRE(reg.snapshot(static_cast<PopId>(i), 0.0, s));
    CHECK(s.base.sex == specs[i].sex);
    CHECK(static_cast<int>(-s.base.birth_year) == specs[i].age);   // age in whole years, the fraction is drawn
    CHECK(s.base.death_year > 0.0);
  }
  // Women over 45 have no births ahead of them; the young women do.
  const auto counts = reg.project_demography(0.0, 30.0);
  CHECK(counts.births > 50);
  PopRegistry old_only(61);
  REQUIRE(old_only.add_cohort({1, 0, 0, 200}));
  std::vector<FounderSpec> old(200, FounderSpec{55, Sex::Female});
  REQUIRE(old_only.set_founder_specs(1, old));
  CHECK(old_only.project_demography(0.0, 30.0).births == 0);
  // Saved and loaded with the registry.
  PopRegistry loaded(61);
  loaded.load(reg.save());
  CHECK(loaded.save() == reg.save());
  PopSnapshot a, b;
  REQUIRE(reg.snapshot(7, 0.0, a));
  REQUIRE(loaded.snapshot(7, 0.0, b));
  CHECK(a.base.sex == b.base.sex);
  CHECK(a.base.birth_year == b.base.birth_year);
  CHECK(loaded.project_demography(0.0, 30.0).births == counts.births);
}
