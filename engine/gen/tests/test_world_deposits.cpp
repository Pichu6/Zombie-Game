// Spec 52 slice 3: deposits where the geology puts them, totals between today's reserves and resources, a reason
// for each, deterministic.
#include <doctest.h>

#include <cstring>
#include <string>

#include "zs/gen/place.h"
#include "zs/gen/keyed.h"
#include "zs/gen/world_deposits.h"

using namespace zs::gen;

namespace {

struct Planet {
  WorldGrid grid;
  WorldFields fields;
  WorldDeposits deposits;
};

const Planet& earthlike() {
  static const Planet p = [] {
    Planet q{WorldGrid(kOneWorldFrequency, 6371.0), {}, {}};  // the checks against Earth run on the one world's grid
    WorldParams wp;
    wp.seed = 1;
    q.fields = generate_fields(q.grid, wp);
    q.deposits = generate_deposits(q.grid, q.fields);
    return q;
  }();
  return p;
}

double land_scale(const Planet& p) {
  double land = 0;
  for (CellId i = 0; i < p.grid.size(); ++i)
    if (p.fields.land(i)) land += p.grid.area_km2(i);
  return land / 148.94e6;  // WNC-1
}

}  // namespace

TEST_CASE("deposits: totals between today's reserves and resources (A2)") {
  const Planet& p = earthlike();
  const WorldDeposits& d = p.deposits;
  const double s = land_scale(p);
  for (int k = 0; k < resource_count; ++k) {
    const auto r = static_cast<Resource>(k);
    MESSAGE(std::string(resource_name(r)), ": ", d.total(r), " ", std::string(resource_unit(r)), ", metal ", d.metal(r));
  }
  // Rule 17, WSR-37..41.
  CHECK(d.total(Resource::Coal) >= 1074e9 * s);
  CHECK(d.total(Resource::Coal) <= 10 * 1074e9 * s);
  CHECK(d.total(Resource::Oil) >= 1650e9 * s);
  CHECK(d.total(Resource::Oil) <= 3 * 1650e9 * s);
  CHECK(d.total(Resource::Gas) >= 207e12 * s);
  CHECK(d.total(Resource::Gas) <= 3 * 207e12 * s);
  CHECK(d.total(Resource::IronOre) >= 180e9 * s);
  CHECK(d.total(Resource::IronOre) <= 900e9 * s);
  CHECK(d.total(Resource::Bauxite) >= 23e9 * s);
  CHECK(d.total(Resource::Bauxite) <= 75e9 * s);
  CHECK(d.metal(Resource::CopperOre) >= 900e6 * s);
  CHECK(d.metal(Resource::CopperOre) <= 5 * 900e6 * s);
}

TEST_CASE("deposits: each lies in a setting that explains it, with a grade where it is an ore (A1)") {
  const Planet& p = earthlike();
  for (const Deposit& d : p.deposits.deposits) {
    CAPTURE(d.cell);
    CHECK(d.settings == p.deposits.settings[d.cell]);
    CHECK(d.amount > 0);
    switch (d.kind) {
      case Resource::Coal: CHECK((d.settings & (kForeland | kRift | kBasin)) != 0); break;
      case Resource::Oil:
      case Resource::Gas: CHECK((d.settings & (kForeland | kRift | kBasin)) != 0); break;
      case Resource::IronOre: CHECK((d.settings & kCraton) != 0); CHECK(d.grade >= 0.30); CHECK(d.grade <= 0.64); break;
      case Resource::CopperOre: CHECK((d.settings & kArc) != 0); CHECK(d.grade > 0); break;
      case Resource::Bauxite: CHECK((d.settings & kLaterite) != 0); CHECK(d.grade >= 0.40); CHECK(d.grade <= 0.55); break;
    }
    if (d.kind == Resource::Coal || d.kind == Resource::IronOre || d.kind == Resource::CopperOre || d.kind == Resource::Bauxite)
      CHECK(p.fields.land(d.cell));
  }
  CHECK(!setting_names(kArc | kBasin).empty());
}

TEST_CASE("deposits: deterministic, golden pinned (A1)") {
  const Planet& p = earthlike();
  const WorldDeposits again = generate_deposits(p.grid, p.fields);
  REQUIRE(again.deposits.size() == p.deposits.deposits.size());
  std::uint64_t h = 0xCBF29CE484222325ull;
  for (const Deposit& d : p.deposits.deposits) {
    std::uint64_t a = 0, g = 0;
    std::memcpy(&a, &d.amount, sizeof a);
    std::memcpy(&g, &d.grade, sizeof g);
    h = mix64(mix64(h, d.cell), mix64(static_cast<std::uint64_t>(d.kind), mix64(a, g)));
  }
  MESSAGE("deposits hash: ", h, ", ", p.deposits.deposits.size(), " deposits");
  CHECK(h == 10582451199011116323ULL);  // re-pinned on MSVC, 2026-10-15 (the hydrology fix: wetter tropics, channel losses)
}
