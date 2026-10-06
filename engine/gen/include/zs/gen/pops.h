// The pop creator: households and people for a town from a population mix.
// docs/design/content-tools.md section 4. generate_town is a pure function of (catalog, mix, seed).
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "zs/gen/catalog.h"

namespace zs::gen {

struct Group {
  std::string culture;
  double share = 0, born_abroad = 0, host_given = 1;
};

struct Mix {
  std::string town_culture;
  int people = 60;
  std::vector<std::string> on_map_homes;
  int streets = 4;
  std::vector<double> household_sizes;  // shares of 1, 2, 3, 4, 5+ people
  double intermarriage = 0, same_sex_couples = 0;
  std::vector<Group> groups;
};
Mix load_mix(const std::string& path);

// One stretch of a pop's past (or present): ages [from, to).
struct Term {
  int from = 0, to = 0;
  std::string what;    // "student", "cook", "out of work", "served in the Army"
  std::string where;   // "at <a business in the town directory>" or "in <real city>, <country>"; may be empty
  std::vector<std::pair<int, std::string>> events;  // (age, text)
};

struct Pop {
  std::uint64_t id = 0;
  int household = 0;
  std::string role;  // head, partner, child, relative
  char sex = 'm';
  int age = 0;
  std::string culture;      // heritage
  std::string culture_label;
  std::string given, family, family2, full_name;
  bool born_abroad = false;
  std::string birthplace, birth_country, nationality;
  int moved_at = -1;        // age they came to the town's country; -1 if born there
  int years_in_town = 0;
  std::string education, occupation, job_id, workplace;
  int years_in_job = 0;
  std::string past_job;     // label, or empty
  std::string last_job_id;  // the job a retired or out-of-work pop had
  std::vector<std::string> skills;
  int skin_tone = 1;        // Monk Skin Tone 1-10
  std::string hair, eyes;
  std::uint64_t body_seed = 0;
  std::vector<Term> lifepath;  // oldest first; the last term is the present
};

struct Household {
  int index = 0;
  std::string home;         // census home id: an on-map home or street_N
  std::string address;
  std::string culture;
  int years = 0;            // years at this address
  std::string before_city;  // the city in the town's country it lived in before moving here
  std::vector<std::size_t> members;
};

struct Business {
  std::string type, name, owner_culture;
};

struct Town {
  std::uint64_t seed = 0;
  std::string name, culture;
  std::vector<std::string> streets;
  std::vector<Business> businesses;
  std::vector<Household> households;
  std::vector<Pop> pops;
};

Town generate_town(const Catalog& cat, const Mix& mix, std::uint64_t seed);

// Adds every pop's lifepath. Runs last: it names only places that already exist (the town
// directory, or real cities in the culture packs) and people who already exist (household members).
void add_lifepaths(const Catalog& cat, Town& t);

// Multi-line dossier for one pop; with a year (the run's start year) it adds the lifepath.
std::string dossier(const Town& t, const Pop& p, int year = 0);
// The town as a data/tier0/census.toml-compatible table (spec 22); marks three survivor slots.
std::string census_toml(const Town& t);

}  // namespace zs::gen
