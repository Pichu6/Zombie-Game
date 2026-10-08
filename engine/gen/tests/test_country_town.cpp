// Spec 47: a town generated for a country has that country's households, schooling, work and occupations.
// Large towns, so the shares can be compared with the tables; release only (generation with lifepaths is slow in debug).
#include <doctest.h>

#include <array>
#include <cmath>
#include <string>

#include "zs/gen/pops.h"

using namespace zs::gen;

namespace {

#ifdef NDEBUG
constexpr bool kDebug = false;
#else
constexpr bool kDebug = true;
#endif

struct Measured {
  double mean_size = 0;
  std::array<double, 4> tertiary_25_34{};  // attainment shares, ages 25-34
  double lfp_25_54_men = 0, lfp_25_54_women = 0, unemployment = 0;
  std::array<double, 8> occupations{};
};

Measured measure(const Catalog& cat, const Town& t) {
  Measured m;
  m.mean_size = static_cast<double>(t.pops.size()) / static_cast<double>(t.households.size());
  double young = 0, men = 0, women = 0, men_lf = 0, women_lf = 0, lf = 0, unemployed = 0, employed = 0;
  for (const Pop& p : t.pops) {
    if (p.age >= 25 && p.age < 35) {
      m.tertiary_25_34[static_cast<std::size_t>(p.attainment)] += 1;
      young += 1;
    }
    const bool in_lf = !p.job_id.empty() || p.occupation == "out of work";
    if (p.age >= 25 && p.age < 55) {
      (p.sex == 'f' ? women : men) += 1;
      if (in_lf) (p.sex == 'f' ? women_lf : men_lf) += 1;
    }
    if (p.age >= 18 && in_lf) {
      lf += 1;
      unemployed += p.occupation == "out of work";
    }
    if (!p.job_id.empty()) {
      m.occupations[static_cast<std::size_t>(cat.job(p.job_id)->group)] += 1;
      employed += 1;
    }
  }
  for (double& x : m.tertiary_25_34) x /= young;
  for (double& x : m.occupations) x /= employed;
  m.lfp_25_54_men = men_lf / men;
  m.lfp_25_54_women = women_lf / women;
  m.unemployment = unemployed / lf;
  return m;
}

}  // namespace

TEST_CASE("country town: households, schooling, work and occupations follow the country's tables" * doctest::skip(kDebug)) {
  const std::string data = ZS_DATA_DIR;
  const Catalog cat = load_catalog(data + "/gen");
  for (const char* country : {"base_high", "base_low"}) {
    Mix mix = load_mix(data + "/gen/populations/tier0_block.toml");
    mix.people = 20'000;
    mix.on_map_homes.clear();
    set_country(mix, country, data);
    const Town t = generate_town(cat, mix, 11);
    const zs::NationDemographics& n = *mix.nation;
    const Measured m = measure(cat, t);
    MESSAGE(std::string(country) << ": household " << m.mean_size << " (" << zs::mean_household_size(n) << "), tertiary 25-34 "
                    << m.tertiary_25_34[3] << " (" << n.education[0][3] << "), participation 25-54 " << m.lfp_25_54_men
                    << "/" << m.lfp_25_54_women << " (" << n.participation[1][0] << "/" << n.participation[1][1]
                    << "), unemployment " << m.unemployment << " (" << n.unemployment_rate << ")");
    CHECK(m.mean_size == doctest::Approx(zs::mean_household_size(n)).epsilon(0.05));
    for (std::size_t k = 0; k < 4; ++k) CHECK(std::abs(m.tertiary_25_34[k] - n.education[0][k]) < 0.03);
    CHECK(std::abs(m.lfp_25_54_men - n.participation[1][0]) < 0.03);
    CHECK(std::abs(m.lfp_25_54_women - n.participation[1][1]) < 0.03);
    CHECK(std::abs(m.unemployment - n.unemployment_rate) < 0.015);
    for (std::size_t g = 0; g < 8; ++g) {
      MESSAGE(std::string(country) << " occupation group " << g << ": " << m.occupations[g] << " (" << n.occupations[g] << ")");
      CHECK(std::abs(m.occupations[g] - n.occupations[g]) < 0.03);
    }
  }
}

TEST_CASE("country town: the same seed and country give the same town; no country keeps the old draws") {
  const std::string data = ZS_DATA_DIR;
  const Catalog cat = load_catalog(data + "/gen");
  Mix mix = load_mix(data + "/gen/populations/tier0_block.toml");
  CHECK_FALSE(static_cast<bool>(mix.nation));
  Mix low = mix;
  set_country(low, "base_low", data);
  const Town a = generate_town(cat, low, 5), b = generate_town(cat, low, 5);
  REQUIRE(a.pops.size() == b.pops.size());
  for (std::size_t i = 0; i < a.pops.size(); ++i) {
    CHECK(a.pops[i].occupation == b.pops[i].occupation);
    CHECK(a.pops[i].attainment == b.pops[i].attainment);
  }
  for (const Pop& p : generate_town(cat, mix, 5).pops) CHECK(p.attainment == -1);
}
