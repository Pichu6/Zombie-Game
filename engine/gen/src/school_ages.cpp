// School stages by country.
#include "zs/gen/school_ages.h"

#include <map>
#include <mutex>
#include <stdexcept>

#include "toml.hpp"

namespace zs::gen {

namespace {
const toml::table& table_of(const std::string& path) {
  static std::mutex m;
  static std::map<std::string, toml::table> cache;
  const std::lock_guard<std::mutex> lock(m);
  auto it = cache.find(path);
  if (it == cache.end()) {
    try {
      it = cache.emplace(path, toml::parse_file(path)).first;
    } catch (const toml::parse_error& e) {
      throw std::runtime_error(path + ": " + std::string(e.description()));
    }
  }
  return it->second;
}
}  // namespace

SchoolAges load_school_ages(const std::string& data_dir, const std::string& profile, const std::string& income) {
  const toml::table& t = table_of(data_dir + "/setup/school/ages.toml");
  const toml::table* row = nullptr;
  if (!profile.empty()) row = t[profile].as_table();
  if (!row && !income.empty()) row = t["base_" + income].as_table();
  SchoolAges a;
  if (!row) return a;
  a.primary_entry = (*row)["primary_entry"].value_or(a.primary_entry);
  a.primary_years = (*row)["primary_years"].value_or(a.primary_years);
  a.lower_entry = (*row)["lower_entry"].value_or(a.lower_entry);
  a.lower_years = (*row)["lower_years"].value_or(a.lower_years);
  a.upper_entry = (*row)["upper_entry"].value_or(a.upper_entry);
  a.upper_years = (*row)["upper_years"].value_or(a.upper_years);
  return a;
}

const char* school_stage(const SchoolAges& a, int age) {
  if (age < a.primary_entry) return "";
  if (age < a.lower_entry) return "elementary_school";
  if (age < a.secondary_end()) return "high_school";
  return "";
}

double primary_share(const SchoolAges& a) {
  const double all = a.primary_years + a.lower_years + a.upper_years;
  return all > 0 ? a.primary_years / all : 0.5;
}

}  // namespace zs::gen
