#include "zs/nations.h"

#include <map>
#include <mutex>

#include "toml.hpp"

namespace zs {

namespace {
std::string g_dir;
std::mutex g_mutex;
std::map<std::string, std::string> g_templates;  // custom profile -> its template, read once per folder
}  // namespace

void set_custom_nations_dir(const std::string& dir) {
  std::lock_guard<std::mutex> l(g_mutex);
  if (dir != g_dir) g_templates.clear();
  g_dir = dir;
}

const std::string& custom_nations_dir() { return g_dir; }

bool is_custom_profile(const std::string& profile) { return profile.rfind("custom/", 0) == 0; }

std::string nation_file(const std::string& data_dir, const std::string& profile, const std::string& income) {
  if (profile.empty()) return data_dir + "/setup/nations/base_" + income + ".toml";
  if (is_custom_profile(profile)) return custom_nations_dir() + "/" + profile.substr(7) + ".toml";
  return data_dir + "/setup/nations/real/" + profile + ".toml";
}

std::string table_profile(const std::string& profile) {
  if (!is_custom_profile(profile)) return profile;
  std::lock_guard<std::mutex> l(g_mutex);
  auto it = g_templates.find(profile);
  if (it == g_templates.end()) {
    std::string tpl;
    try {
      const toml::table t = toml::parse_file(g_dir + "/" + profile.substr(7) + ".toml");
      tpl = t["template"].value_or(std::string{});
    } catch (const std::exception&) {
    }
    if (tpl.rfind("base_", 0) == 0) tpl.clear();  // a base template: the tables fall back by income
    it = g_templates.emplace(profile, tpl).first;
  }
  return it->second;
}

}  // namespace zs
