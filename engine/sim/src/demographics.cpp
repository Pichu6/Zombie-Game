#include "zs/demographics.h"

#include <cmath>
#include <set>
#include <stdexcept>

#include "toml.hpp"

namespace zs {

namespace {

const char* kOccupations[kOccupationGroups] = {"armed_forces", "managers", "professionals", "technicians", "clerks",
                                               "services_sales", "skilled_agriculture", "craft_trades", "plant_machine", "elementary"};
const char* kSectorNames[kSectors] = {"agriculture", "industry", "services"};
const char* kEducation[kEducationLevels] = {"none", "primary", "secondary", "tertiary"};

[[noreturn]] void fail(const std::string& what) { throw std::runtime_error("nation profile: " + what); }

const toml::table& table(const toml::table& t, const char* name) {
  const toml::table* x = t[name].as_table();
  if (!x) fail(std::string("missing table [") + name + "]");
  return *x;
}

void only_keys(const toml::table& t, const std::set<std::string>& known, const std::string& where) {
  for (const auto& [k, v] : t)
    if (!known.count(std::string(k.str()))) fail("unknown key '" + std::string(k.str()) + "' in " + where);
}

std::vector<double> numbers(const toml::table& t, const char* key, std::size_t count, const std::string& where) {
  const toml::array* a = t[key].as_array();
  if (!a) fail(std::string("missing list '") + key + "' in " + where);
  std::vector<double> out;
  for (const auto& v : *a) {
    const auto x = v.value<double>();
    if (!x) fail(std::string("'") + key + "' in " + where + " must be numbers");
    out.push_back(*x);
  }
  if (out.size() != count) fail(std::string("'") + key + "' in " + where + " needs " + std::to_string(count) + " numbers");
  return out;
}

double number(const toml::table& t, const char* key, const std::string& where) {
  const auto x = t[key].value<double>();
  if (!x) fail(std::string("missing number '") + key + "' in " + where);
  return *x;
}

void shares(std::vector<double>& v, const std::string& what) {
  double sum = 0;
  for (double x : v) {
    if (x < 0) fail(what + " has a negative share");
    sum += x;
  }
  if (sum < 0.99 || sum > 1.01) fail(what + " shares must add up to 1 (they add up to " + std::to_string(sum) + ")");
  for (double& x : v) x /= sum;
}

void rate(double x, const std::string& what) {
  if (x < 0 || x > 1) fail(what + " must be between 0 and 1");
}

Source source(const toml::table& t, const std::string& where) {
  Source s;
  if (const auto text = t["source"].value<std::string>()) s.text = *text;
  if (const auto year = t["year"].value<int64_t>()) s.year = static_cast<int>(*year);
  (void)where;
  return s;
}

}  // namespace

NationDemographics load_nation(const std::string& path) {
  const toml::table t = toml::parse_file(path);
  only_keys(t, {"id", "name", "status", "pop_params", "age_pyramid", "education", "labour", "occupations", "sectors", "households", "settlement"}, "the file");
  NationDemographics n;
  n.id = t["id"].value_or(std::string{});
  n.name = t["name"].value_or(std::string{});
  if (n.id.empty() || n.name.empty()) fail("needs an id and a name");
  const std::string status = t["status"].value_or(std::string{});
  if (status != "placeholder" && status != "sourced") fail("status must be \"placeholder\" or \"sourced\"");
  n.sourced = status == "sourced";
  n.pop_params = t["pop_params"].value_or(std::string{});

  const toml::table& pyr = table(t, "age_pyramid");
  only_keys(pyr, {"shares", "source", "year"}, "[age_pyramid]");
  n.age_pyramid = numbers(pyr, "shares", kAgeBands, "[age_pyramid]");
  shares(n.age_pyramid, "the age pyramid");
  n.age_pyramid_source = source(pyr, "[age_pyramid]");

  const toml::table& edu = table(t, "education");
  only_keys(edu, {"age_25_34", "age_35_64", "age_65_plus", "source", "year"}, "[education]");
  const char* edu_keys[kEducationAgeGroups] = {"age_25_34", "age_35_64", "age_65_plus"};
  for (int g = 0; g < kEducationAgeGroups; ++g) {
    std::vector<double> v = numbers(edu, edu_keys[g], kEducationLevels, "[education]");
    shares(v, std::string("education ") + edu_keys[g]);
    for (int l = 0; l < kEducationLevels; ++l) n.education[static_cast<std::size_t>(g)][static_cast<std::size_t>(l)] = v[static_cast<std::size_t>(l)];
  }
  n.education_source = source(edu, "[education]");

  const toml::table& lab = table(t, "labour");
  only_keys(lab, {"participation_15_24", "participation_25_54", "participation_55_64", "participation_65_plus", "unemployment_rate", "retirement_age", "source", "year"}, "[labour]");
  const char* lab_keys[kLabourAgeGroups] = {"participation_15_24", "participation_25_54", "participation_55_64", "participation_65_plus"};
  for (int g = 0; g < kLabourAgeGroups; ++g) {
    const auto v = numbers(lab, lab_keys[g], 2, "[labour]");
    for (int s = 0; s < 2; ++s) { rate(v[static_cast<std::size_t>(s)], lab_keys[g]); n.participation[static_cast<std::size_t>(g)][static_cast<std::size_t>(s)] = v[static_cast<std::size_t>(s)]; }
  }
  n.unemployment_rate = number(lab, "unemployment_rate", "[labour]");
  rate(n.unemployment_rate, "unemployment_rate");
  n.retirement_age = number(lab, "retirement_age", "[labour]");
  if (n.retirement_age < 50 || n.retirement_age > 80) fail("retirement_age must be between 50 and 80");
  n.labour_source = source(lab, "[labour]");

  const toml::table& occ = table(t, "occupations");
  only_keys(occ, {"shares", "source", "year"}, "[occupations]");
  std::vector<double> o = numbers(occ, "shares", kOccupationGroups, "[occupations]");
  shares(o, "occupations");
  for (int i = 0; i < kOccupationGroups; ++i) n.occupations[static_cast<std::size_t>(i)] = o[static_cast<std::size_t>(i)];
  n.occupation_source = source(occ, "[occupations]");

  const toml::table& sec = table(t, "sectors");
  only_keys(sec, {"shares", "source", "year"}, "[sectors]");
  std::vector<double> sv = numbers(sec, "shares", kSectors, "[sectors]");
  shares(sv, "sectors");
  for (int i = 0; i < kSectors; ++i) n.sectors[static_cast<std::size_t>(i)] = sv[static_cast<std::size_t>(i)];
  n.sector_source = source(sec, "[sectors]");

  const toml::table& hh = table(t, "households");
  only_keys(hh, {"sizes", "source", "year"}, "[households]");
  std::vector<double> hv = numbers(hh, "sizes", kHouseholdSizes, "[households]");
  shares(hv, "household sizes");
  for (int i = 0; i < kHouseholdSizes; ++i) n.household_sizes[static_cast<std::size_t>(i)] = hv[static_cast<std::size_t>(i)];
  n.household_source = source(hh, "[households]");

  const toml::table& set = table(t, "settlement");
  only_keys(set, {"urban_share", "source", "year"}, "[settlement]");
  n.urban_share = number(set, "urban_share", "[settlement]");
  rate(n.urban_share, "urban_share");
  n.settlement_source = source(set, "[settlement]");

  if (n.sourced) {  // real data must say where it came from
    const std::pair<const char*, const Source*> all[] = {{"age_pyramid", &n.age_pyramid_source}, {"education", &n.education_source},
        {"labour", &n.labour_source}, {"occupations", &n.occupation_source}, {"sectors", &n.sector_source},
        {"households", &n.household_source}, {"settlement", &n.settlement_source}};
    for (const auto& [name, src] : all)
      if (src->text.empty() || src->year < 1900) fail(std::string("a sourced profile needs a source and a year for [") + name + "]");
  }
  return n;
}

PopParams make_pop_params(const NationDemographics& nation, const std::string& data_dir) {
  PopParams p = nation.pop_params.empty() ? default_pop_params() : load_pop_params(data_dir + "/setup/" + nation.pop_params);
  p.age_pyramid = nation.age_pyramid;
  return p;
}

double mean_household_size(const NationDemographics& n) {
  // The last bucket is "5 or more"; counted as 5.5, a stated approximation.
  static constexpr double sizes[kHouseholdSizes] = {1, 2, 3, 4, 5.5};
  double m = 0;
  for (int i = 0; i < kHouseholdSizes; ++i) m += sizes[i] * n.household_sizes[static_cast<std::size_t>(i)];
  return m;
}

double share_under(const NationDemographics& n, double age) {
  double s = 0;
  for (int b = 0; b < kAgeBands; ++b) {
    const double lo = 5.0 * b, hi = lo + 5.0;
    if (age >= hi) s += n.age_pyramid[static_cast<std::size_t>(b)];
    else if (age > lo) s += n.age_pyramid[static_cast<std::size_t>(b)] * (age - lo) / 5.0;
  }
  return s;
}

const char* occupation_group_name(int g) { return g >= 0 && g < kOccupationGroups ? kOccupations[g] : "?"; }
const char* sector_name(int s) { return s >= 0 && s < kSectors ? kSectorNames[s] : "?"; }
const char* education_level_name(int l) { return l >= 0 && l < kEducationLevels ? kEducation[l] : "?"; }

}  // namespace zs
