// Spec 69: a town's police agency by its size: the BJS curve at the country's level, investigators, civilians, women,
// stations and holding cells (research services-by-settlement-size, SV).
#include <doctest.h>

#include "zs/police_agency.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

const AgencyParams& params() {
  static const AgencyParams p = load_agency_params(tier0::data_dir() + "/services/police.toml", "");
  return p;
}

}  // namespace

TEST_CASE("the size curve loads from data with the BJS classes, and a bad file is refused (A1)") {
  CHECK(params().per_1000.size() == 8);
  CHECK(size_class_value(params().per_1000, 5'000) == doctest::Approx(2.3));     // SV-5
  CHECK(size_class_value(params().per_1000, 75'000) == doctest::Approx(1.6));    // the bottom of the U
  CHECK(size_class_value(params().per_1000, 3'000'000) == doctest::Approx(2.3));
  CHECK(size_class_value(params().female, 20'000) == doctest::Approx(0.090));    // SV-12
  CHECK(params().reference_per_100k == doctest::Approx(207.2));                   // DRL-4: UNODC's USA
  CHECK(params().country_per_100k == doctest::Approx(207.2));                     // no country, no group: the reference
  CHECK_THROWS(load_agency_params(tier0::data_dir() + "/services/no_such.toml", ""));
}

TEST_CASE("sworn officers are the town's people times its class's rate; investigators, civilians and women follow (A1)") {
  const AgencyPlan a = plan_agency(params(), 50'000, 6);
  CHECK(a.sworn == 80);           // 1.6 per 1,000 (SV-5)
  CHECK(a.investigators == 12);   // 15% (SV-10)
  CHECK(a.patrol == 68);
  CHECK(a.civilians == 23);       // 22% of full-time staff (SV-2): 23 / (80 + 23)
  CHECK(a.women == 8);            // 10.5% (SV-12)
  CHECK(a.stations == 1);
  CHECK(a.cells == 6);            // one per 15 sworn
  const AgencyPlan town = plan_agency(params(), 17'626, 6);
  CHECK(town.sworn == 33);        // 1.9 per 1,000
  const AgencyPlan big = plan_agency(params(), 250'000, 6);
  CHECK(big.stations == 3);
  CHECK(big.sworn == 475);        // 1.9 per 1,000
}

TEST_CASE("a small department has no investigator, and never fewer officers than its shifts need (A1)") {
  const AgencyPlan small = plan_agency(params(), 2'000, 6);
  CHECK(small.sworn == 6);  // 2.3 per 1,000 gives 5: the smallest force keeps three shifts
  CHECK(small.investigators == 0);
  const AgencyPlan tiny = plan_agency(params(), 2'000, 1);
  CHECK(tiny.sworn == 5);
  CHECK(tiny.investigators == 0);  // under 7 sworn (SV-11)
}

TEST_CASE("a country with its own level scales the curve (A1)") {
  AgencyParams p = params();
  p.country_per_100k = 2 * p.reference_per_100k;  // twice the reference
  CHECK(plan_agency(p, 50'000, 6).sworn == 160);
  CHECK(load_agency_params(tier0::data_dir() + "/services/police.toml", "USA").country_per_100k == doctest::Approx(207.2));
  // DRL-4: a country UNODC does not cover takes its income group's median; one it covers, its own.
  CHECK(load_agency_params(tier0::data_dir() + "/services/police.toml", "GBR", "high").country_per_100k == doctest::Approx(346.0));
  CHECK(load_agency_params(tier0::data_dir() + "/services/police.toml", "", "low").country_per_100k == doctest::Approx(78.2));
  CHECK(load_agency_params(tier0::data_dir() + "/services/police.toml", "IND", "lower_middle").country_per_100k == doctest::Approx(133.6));
}

TEST_CASE("women are spread evenly over the officers: any first n hold round(n x share), with no draw (A1)") {
  const AgencyPlan a = plan_agency(params(), 180'000, 6);
  int women = 0;
  for (int i = 0; i < a.sworn; ++i) {
    women += officer_is_woman(a, i) ? 1 : 0;
    if ((i + 1) % 50 == 0) CHECK(women == (static_cast<long long>(i + 1) * a.women + a.sworn / 2) / a.sworn);
  }
  CHECK(women == a.women);
}
