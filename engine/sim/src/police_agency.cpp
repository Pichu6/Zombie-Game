// A town's police agency by its size. Spec: docs/tier0/specs/69-the-police-station.md (decision 0077)
#include "zs/police_agency.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "toml.hpp"

namespace zs {
namespace {

std::vector<SizeClass> load_classes(const toml::table& t, const char* table, const char* key, const std::string& path) {
  const toml::array* rows = t[table][key].as_array();
  if (!rows || rows->empty()) throw std::runtime_error(path + ": [" + table + "] needs " + key);
  std::vector<SizeClass> out;
  for (const toml::node& row : *rows) {
    const toml::array* r = row.as_array();
    if (!r || r->size() != 3) throw std::runtime_error(path + ": [" + table + "] " + key + " rows are [min, max, value]");
    SizeClass c;
    c.min_people = r->get(0)->value<double>().value_or(-1);
    c.max_people = r->get(1)->value<double>().value_or(-1);
    c.value = r->get(2)->value<double>().value_or(-1);
    if (c.min_people < 0 || c.max_people < 0 || c.value < 0) throw std::runtime_error(path + ": [" + table + "] " + key + " has a bad row");
    out.push_back(c);
  }
  return out;
}

}  // namespace

AgencyParams load_agency_params(const std::string& path, const std::string& country_profile, const std::string& income) {
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  AgencyParams p;
  p.per_1000 = load_classes(t, "curve", "classes", path);
  p.female = load_classes(t, "curve", "female", path);
  const auto need = [&](const char* table, const char* key) {
    const auto v = t[table][key].value<double>();
    if (!v || *v < 0) throw std::runtime_error(path + ": [" + table + "] needs a non-negative " + key);
    return *v;
  };
  p.reference_per_100k = need("curve", "reference_per_100k");
  p.country_per_100k = p.reference_per_100k;
  if (const auto v = t["country"][country_profile].value<double>(); v && !country_profile.empty()) {
    p.country_per_100k = *v;
  } else if (const auto g = t["group"][income].value<double>(); g && !income.empty()) {
    p.country_per_100k = *g;  // DRL-4: the median of the group's countries
  }
  p.investigator_share = need("staff", "investigator_share");
  p.min_sworn_for_investigator = static_cast<int>(need("staff", "min_sworn_for_investigator"));
  p.civilian_share = need("staff", "civilian_share");
  p.shifts = std::max(1, static_cast<int>(need("staff", "shifts")));
  p.people_per_station = need("station", "people_per_station");
  p.officers_per_cell = std::max(1, static_cast<int>(need("station", "officers_per_cell")));
  if (p.civilian_share >= 1 || p.reference_per_100k <= 0 || p.people_per_station <= 0) throw std::runtime_error(path + ": a share or level out of range");
  return p;
}

double size_class_value(const std::vector<SizeClass>& classes, double people) {
  for (const SizeClass& c : classes)
    if (people >= c.min_people && (c.max_people == 0 || people <= c.max_people)) return c.value;
  return classes.empty() ? 0 : classes.back().value;
}

AgencyPlan plan_agency(const AgencyParams& p, double people, int min_sworn) {
  AgencyPlan a;
  // Rule 1: the class's rate, scaled to the country's level.
  a.per_1000 = size_class_value(p.per_1000, people) * p.country_per_100k / p.reference_per_100k;
  a.sworn = std::max(min_sworn, static_cast<int>(std::lround(people * a.per_1000 / 1000.0)));
  // Rule 2: investigators from the sworn, none in a department too small to hold one (SV-10, SV-11).
  a.investigators = a.sworn >= p.min_sworn_for_investigator ? static_cast<int>(std::lround(p.investigator_share * a.sworn)) : 0;
  a.patrol = a.sworn - a.investigators;
  // Civilians are a share of all full-time staff (SV-2): civilians / (sworn + civilians) = share.
  a.civilians = static_cast<int>(std::lround(a.sworn * p.civilian_share / (1 - p.civilian_share)));
  // Rule 5: women among the sworn by the class's share (SV-12).
  a.women = static_cast<int>(std::lround(a.sworn * size_class_value(p.female, people)));
  // Rule 3: stations and cells (defaults).
  a.stations = 1 + static_cast<int>(std::max(0.0, std::floor((people - 1) / p.people_per_station)));
  a.cells = std::max(a.stations, (a.sworn + p.officers_per_cell - 1) / p.officers_per_cell);
  return a;
}

bool officer_is_woman(const AgencyPlan& plan, int index) {
  if (plan.sworn <= 0 || index < 0) return false;
  const auto upto = [&](int n) { return (static_cast<long long>(n) * plan.women + plan.sworn / 2) / plan.sworn; };  // round(n x share)
  return upto(index + 1) > upto(index);
}

}  // namespace zs
