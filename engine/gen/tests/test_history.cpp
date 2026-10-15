// Spec 91: N founding states, a recorded history that unites and divides them, alliances, and the history seed.
#include <doctest.h>

#include <set>

#include "zs/gen/place.h"

using namespace zs::gen;

namespace {

GeneratedWorld world(int countries, double history, std::uint64_t history_seed, std::uint64_t seed = 1) {
  WorldSettings s;
  s.countries = countries;
  s.history = history;
  s.history_seed = history_seed;
  return generate_world(seed, s, 32);
}

std::vector<CellId> capitals(const GeneratedWorld& w) {
  std::vector<CellId> out;
  for (const Entity& e : w.people.formed.entities)
    if (e.origin == Origin::Founder) out.push_back(e.capital);
  return out;
}

}  // namespace

TEST_CASE("history: exactly N states after the founding, all land but ice held") {
  for (int n : {20, 100, 183, 300}) {
    const GeneratedWorld w = world(n, 0.0, 7);
    CAPTURE(n);
    CHECK(static_cast<int>(w.people.countries.size()) == n);
    CHECK(static_cast<int>(capitals(w).size()) == n);
    for (CellId c = 0; c < w.grid.size(); ++c)
      if (w.fields.land(c) && w.fields.biome[c] != Biome::Ice) CHECK(w.people.formed.country_of[c] >= 0);
  }
}

TEST_CASE("history: the history seed changes the history, not the planet or its founders' capitals") {
  const GeneratedWorld a = world(120, 1.0, 11), b = world(120, 1.0, 12), a2 = world(120, 1.0, 11);
  CHECK(capitals(a) == capitals(b));
  CHECK(a.people.formed.country_of != b.people.formed.country_of);
  CHECK(a.people.formed.country_of == a2.people.formed.country_of);  // the same pair, the same map
  CHECK(a.fields.elevation_m == b.fields.elevation_m);
}

TEST_CASE("history: every state's origin is a founder, a union of two earlier ones or a division of one; alliances hold") {
  const GeneratedWorld w = world(183, 2.0, 5);
  const WorldCountries& f = w.people.formed;
  int unions = 0, divisions = 0;
  for (std::size_t e = 0; e < f.entities.size(); ++e) {
    const Entity& x = f.entities[e];
    if (x.origin == Origin::Union) {
      ++unions;
      CHECK(x.a >= 0);
      CHECK(x.b >= 0);
      CHECK(static_cast<std::size_t>(x.a) < e);
      CHECK(static_cast<std::size_t>(x.b) < e);
    } else if (x.origin == Origin::Division) {
      ++divisions;
      CHECK(x.a >= 0);
      CHECK(static_cast<std::size_t>(x.a) < e);
    }
  }
  MESSAGE(unions << " unions, " << divisions << " divisions, " << f.alliances.size() << " alliances, " << w.people.countries.size()
                 << " countries today");
  CHECK(unions + divisions > 0);
  std::set<int> in;
  for (const Alliance& a : f.alliances) {
    CHECK(a.members.size() >= 2);
    for (int m : a.members) {
      CHECK(m >= 0);
      CHECK(m < static_cast<int>(w.people.countries.size()));
      CHECK(in.insert(m).second);  // one alliance at most
    }
  }
  CHECK(w.people.alliance_names.size() == f.alliances.size());
  std::set<std::string> names(w.people.alliance_names.begin(), w.people.alliance_names.end());
  CHECK(names.size() == w.people.alliance_names.size());
}
