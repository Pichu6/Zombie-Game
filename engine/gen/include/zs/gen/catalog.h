// Data packs for the content generators: cultures, jobs, business types (data/gen/).
// docs/design/content-tools.md section 3.
#pragma once
#include <map>
#include <string>
#include <vector>

namespace zs::gen {

// A list drawn by rank: entry r has weight 1 / (r + 2), so lists are written most common first.
struct RankedList {
  std::vector<std::string> items;
  std::vector<double> weights;
  void set(std::vector<std::string> v);
  bool empty() const { return items.empty(); }
};

enum class Surnames { Single, PaternalMaternal, GenderedSki };

struct Culture {
  std::string id, label, country, nationality;
  bool family_first = false;
  Surnames surnames = Surnames::Single;
  RankedList given_m, given_f, family;
  std::vector<std::string> places;  // real towns: birthplaces and the toponym training set
  std::vector<std::string> town_forms, province_forms, country_forms;
  std::string street_format;        // "{name} {kind}"
  RankedList street_names, street_kinds;
  std::vector<double> skin;         // Monk Skin Tone 1-10 shares
  std::vector<std::string> hair, eyes;
  std::vector<double> hair_w, eyes_w;
};

struct Job {
  std::string id, label, workplace, education;
  double share = 0;
  std::vector<std::string> skills;
};

struct BusinessType {
  std::string id;
  int count = 1;
  std::vector<std::string> forms;
  std::map<std::string, std::vector<std::string>> culture_forms;  // culture id -> extra forms
};

struct Catalog {
  std::vector<Culture> cultures;  // sorted by id
  std::vector<Job> jobs;
  std::vector<BusinessType> businesses;
  std::vector<std::string> words;

  const Culture& culture(const std::string& id) const;  // throws if missing
  const Job* job(const std::string& id) const;
  const BusinessType* business(const std::string& id) const;
};

// Loads <dir>/cultures/*.toml, <dir>/jobs.toml and <dir>/businesses.toml. Throws on bad data.
Catalog load_catalog(const std::string& dir);

// Education levels in order; index is the level.
int education_level(const std::string& name);
const char* education_name(int level);

}  // namespace zs::gen
