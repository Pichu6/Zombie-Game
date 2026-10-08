// Content generators: names, pops, towns (docs/design/content-tools.md section 7).
#include <doctest.h>

#include <map>
#include <set>

#include "zs/gen/names.h"
#include "zs/gen/pops.h"

using namespace zs::gen;

namespace {

const Catalog& catalog() {
  static const Catalog cat = load_catalog(std::string(ZS_DATA_DIR) + "/gen");
  return cat;
}

const Mix& block_mix() {
  static const Mix mix = load_mix(std::string(ZS_DATA_DIR) + "/gen/populations/tier0_block.toml");
  return mix;
}

std::string all_text(const Town& t) {
  std::string s = t.name;
  for (const auto& b : t.businesses) s += b.name;
  for (const auto& p : t.pops) s += dossier(t, p, 2026);
  return s;
}

}  // namespace

TEST_CASE("every culture pack loads with usable lists") {
  const Catalog& cat = catalog();
  CHECK(cat.cultures.size() >= 6);
  for (const Culture& c : cat.cultures) {
    CHECK(c.given_m.items.size() >= 20);
    CHECK(c.given_f.items.size() >= 20);
    CHECK(c.family.items.size() >= 20);
    CHECK(c.places.size() >= 20);
  }
  for (const Group& g : block_mix().groups) CHECK_NOTHROW(cat.culture(g.culture));
}

TEST_CASE("names follow each culture's rules") {
  const Catalog& cat = catalog();
  const Keyed r(7);
  const Culture& pl = cat.culture("polish");
  CHECK(family_form(pl, "Kowalski", 'f') == "Kowalska");
  CHECK(family_form(pl, "Nowak", 'f') == "Nowak");
  CHECK(family_form(pl, "Kowalski", 'm') == "Kowalski");
  const Culture& mx = cat.culture("mexican");
  for (std::uint64_t id = 0; id < 50; ++id) {
    const PersonName n = person_name(mx, r, id, 'm');
    CHECK(!n.family2.empty());
    CHECK(n.family2 != n.family);
  }
  const Culture& kr = cat.culture("korean");
  const PersonName n = person_name(kr, r, 3, 'f');
  CHECK(full_name(kr, n, 'f') == n.family + " " + n.given);
}

TEST_CASE("generated place names are new and within length") {
  const Keyed r(11);
  for (const Culture& c : catalog().cultures) {
    std::set<std::string> distinct;
    for (std::uint64_t id = 0; id < 40; ++id) {
      const std::string s = place_name(c, r, id, PlaceKind::Town);
      distinct.insert(s);
      CHECK(s.size() >= 4);
      for (const std::string& real : c.places) CHECK(s != real);
    }
    CHECK(distinct.size() >= 25);
  }
}

TEST_CASE("a town is a pure function of the seed") {
  const Town a = generate_town(catalog(), block_mix(), 42);
  const Town b = generate_town(catalog(), block_mix(), 42);
  const Town c = generate_town(catalog(), block_mix(), 43);
  CHECK(all_text(a) == all_text(b));
  CHECK(all_text(a) != all_text(c));
}

TEST_CASE("households are consistent") {
  for (std::uint64_t seed = 1; seed <= 30; ++seed) {
    const Town t = generate_town(catalog(), block_mix(), seed);
    REQUIRE(static_cast<int>(t.pops.size()) == block_mix().people);
    CHECK(t.households[0].home == "house_a");
    CHECK(t.households[1].home == "house_b");
    std::set<std::string> businesses;
    for (const auto& b : t.businesses) CHECK(businesses.insert(b.name).second);
    for (const Household& hh : t.households) {
      const Pop& head = t.pops[hh.members.front()];
      for (const std::size_t i : hh.members) {
        const Pop& p = t.pops[i];
        CHECK(p.age >= 0);
        CHECK(p.age <= 92);
        CHECK(p.skin_tone >= 1);
        CHECK(p.skin_tone <= 10);
        CHECK(p.years_in_town <= p.age);
        if (p.role == "child") {
          CHECK(head.age - p.age >= 18);
          const bool from_parent = p.family == head.family ||
                                   (hh.members.size() > 1 && p.family == t.pops[hh.members[1]].family);
          CHECK(from_parent);
        }
        if (p.age < 16) CHECK(p.job_id.empty());
        if (!p.job_id.empty()) {
          const Job* job = catalog().job(p.job_id);
          REQUIRE(job != nullptr);
          CHECK(education_level(p.education) >= education_level(job->education));
          if (!job->workplace.empty()) CHECK(!p.workplace.empty());
        }
        if (p.born_abroad) {
          CHECK(p.moved_at >= 0);
          CHECK(p.moved_at <= p.age);
          CHECK(p.birth_country != "United States");
        }
      }
    }
  }
}

TEST_CASE("the mix shows up over many towns") {
  std::set<std::string> cultures, jobs;
  int abroad = 0, total = 0;
  for (std::uint64_t seed = 1; seed <= 40; ++seed) {
    const Town t = generate_town(catalog(), block_mix(), seed);
    for (const Pop& p : t.pops) {
      cultures.insert(p.culture);
      if (!p.job_id.empty()) jobs.insert(p.job_id);
      abroad += p.born_abroad;
      ++total;
    }
  }
  CHECK(cultures.size() == block_mix().groups.size());
  CHECK(jobs.size() >= 25);
  CHECK(abroad > total / 50);   // some first-generation immigrants
  CHECK(abroad < total / 4);
}

TEST_CASE("census export matches the census table format") {
  const Town t = generate_town(catalog(), block_mix(), 5);
  const std::string s = census_toml(t);
  std::size_t people = 0, survivors = 0;
  for (std::size_t at = s.find("[[person]]"); at != std::string::npos; at = s.find("[[person]]", at + 1)) ++people;
  for (std::size_t at = s.find("\nsurvivor = true"); at != std::string::npos; at = s.find("\nsurvivor = true", at + 1)) ++survivors;
  CHECK(people == t.pops.size());
  CHECK(survivors == 3);
}

TEST_CASE("lifepaths only name what exists and add up to the present") {
  const Catalog& cat = catalog();
  std::set<std::string> cities;
  for (const Culture& c : cat.cultures)
    for (const auto& pl : c.places) cities.insert(pl + ", " + c.country);
  for (std::uint64_t seed = 1; seed <= 30; ++seed) {
    const Town t = generate_town(cat, block_mix(), seed);
    std::set<std::string> businesses;
    for (const auto& b : t.businesses) businesses.insert("at " + b.name);
    cities.insert(t.name + ", United States");
    std::map<int, std::set<int>> married_at;  // household -> years
    for (const Pop& p : t.pops) {
      REQUIRE(!p.lifepath.empty());
      CHECK(p.lifepath.back().to == p.age);
      for (std::size_t i = 0; i < p.lifepath.size(); ++i) {
        const Term& term = p.lifepath[i];
        CHECK(term.from <= term.to);
        if (i > 0) CHECK(term.from == p.lifepath[i - 1].to);
        const std::string& w = term.where;
        if (w.starts_with("at ") && w != "at home") CHECK_MESSAGE(businesses.count(w), w);
        if (w.starts_with("in ") && w.find(", ") != std::string::npos) CHECK_MESSAGE(cities.count(w.substr(3)), w);
        for (const auto& [a, text] : term.events) {
          CHECK(a >= term.from);
          CHECK(a <= term.to);
          if (text.starts_with("married ")) married_at[p.household].insert(a - p.age);
        }
      }
      if (!p.job_id.empty()) {
        CHECK_MESSAGE(p.lifepath.back().what == p.occupation, dossier(t, p, 2026));
        CHECK(p.lifepath.back().where == (p.workplace.empty() ? "in " + t.name : "at " + p.workplace));
        CHECK(p.lifepath.back().to - p.lifepath.back().from == p.years_in_job);
      }
    }
    for (const auto& [hh, years] : married_at) CHECK(years.size() == 1);  // both partners, same year
  }
}

TEST_CASE("golden: town seed 1 is the same on every compiler") {
  const Town t = generate_town(catalog(), block_mix(), 1);
  const std::uint64_t h = hash_str(all_text(t) + census_toml(t));
  CHECK(h == 1412021876019485665ull);  // re-pinned on MSVC for spec 47 (three technician jobs); update only on a deliberate generator or data change
}
