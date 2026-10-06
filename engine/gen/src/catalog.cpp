// Loads the generator data packs. docs/design/content-tools.md section 3.
#include "zs/gen/catalog.h"

#include <algorithm>
#include <filesystem>
#include <stdexcept>

#include "toml.hpp"

namespace zs::gen {
namespace {

const char* kEducation[] = {"none", "high_school", "trade", "bachelor", "master", "doctorate"};

std::vector<std::string> strings(const toml::node_view<const toml::node>& n, const std::string& what) {
  std::vector<std::string> out;
  if (const auto* a = n.as_array())
    for (const auto& e : *a) out.push_back(e.value<std::string>().value());
  if (out.empty()) throw std::runtime_error("gen data: empty or missing list " + what);
  return out;
}

std::vector<double> numbers(const toml::node_view<const toml::node>& n, const std::string& what) {
  std::vector<double> out;
  if (const auto* a = n.as_array())
    for (const auto& e : *a) out.push_back(e.value<double>().value());
  if (out.empty()) throw std::runtime_error("gen data: empty or missing list " + what);
  return out;
}

Culture load_culture(const std::string& path) {
  const toml::table t = toml::parse_file(path);
  Culture c;
  c.id = t["id"].value<std::string>().value();
  const std::string w = c.id + ".";
  c.label = t["label"].value_or(c.id);
  c.country = t["country"].value<std::string>().value();
  c.nationality = t["nationality"].value<std::string>().value();
  c.family_first = t["name_order"].value_or(std::string("given_family")) == "family_given";
  const std::string s = t["surnames"].value_or(std::string("single"));
  c.surnames = s == "paternal_maternal" ? Surnames::PaternalMaternal
               : s == "gendered_ski"    ? Surnames::GenderedSki
                                        : Surnames::Single;
  c.given_m.set(strings(t["given_m"], w + "given_m"));
  c.given_f.set(strings(t["given_f"], w + "given_f"));
  c.family.set(strings(t["family"], w + "family"));
  c.places = strings(t["places"], w + "places");
  c.town_forms = strings(t["town_forms"], w + "town_forms");
  c.province_forms = strings(t["province_forms"], w + "province_forms");
  c.country_forms = strings(t["country_forms"], w + "country_forms");
  c.street_format = t["street_format"].value_or(std::string("{name} {kind}"));
  c.street_names.set(strings(t["street_names"], w + "street_names"));
  c.street_kinds.set(strings(t["street_kinds"], w + "street_kinds"));
  c.skin = numbers(t["skin"], w + "skin");
  c.hair = strings(t["hair"], w + "hair");
  c.hair_w = numbers(t["hair_w"], w + "hair_w");
  c.eyes = strings(t["eyes"], w + "eyes");
  c.eyes_w = numbers(t["eyes_w"], w + "eyes_w");
  if (c.skin.size() != 10) throw std::runtime_error("gen data: " + w + "skin needs 10 shares");
  if (c.hair.size() != c.hair_w.size() || c.eyes.size() != c.eyes_w.size())
    throw std::runtime_error("gen data: " + w + "hair/eyes and weights differ in length");
  return c;
}

}  // namespace

void RankedList::set(std::vector<std::string> v) {
  items = std::move(v);
  weights.clear();
  for (std::size_t r = 0; r < items.size(); ++r) weights.push_back(1.0 / static_cast<double>(r + 2));
}

const Culture& Catalog::culture(const std::string& id) const {
  for (const Culture& c : cultures)
    if (c.id == id) return c;
  throw std::runtime_error("gen: unknown culture " + id);
}

const Job* Catalog::job(const std::string& id) const {
  for (const Job& j : jobs)
    if (j.id == id) return &j;
  return nullptr;
}

const BusinessType* Catalog::business(const std::string& id) const {
  for (const BusinessType& b : businesses)
    if (b.id == id) return &b;
  return nullptr;
}

int education_level(const std::string& name) {
  for (int i = 0; i < 6; ++i)
    if (name == kEducation[i]) return i;
  throw std::runtime_error("gen: unknown education " + name);
}

const char* education_name(int level) { return kEducation[std::clamp(level, 0, 5)]; }

Catalog load_catalog(const std::string& dir) {
  Catalog cat;
  std::vector<std::string> files;
  for (const auto& e : std::filesystem::directory_iterator(dir + "/cultures"))
    if (e.path().extension() == ".toml") files.push_back(e.path().string());
  std::sort(files.begin(), files.end());  // directory order is not portable
  for (const auto& f : files) cat.cultures.push_back(load_culture(f));
  std::sort(cat.cultures.begin(), cat.cultures.end(), [](const auto& a, const auto& b) { return a.id < b.id; });

  const toml::table jobs = toml::parse_file(dir + "/jobs.toml");
  for (const auto& n : *jobs["job"].as_array()) {
    const auto& x = *n.as_table();
    Job j;
    j.id = x["id"].value<std::string>().value();
    j.label = x["label"].value<std::string>().value();
    j.share = x["share"].value<double>().value();
    j.workplace = x["workplace"].value_or(std::string{});
    j.education = x["education"].value_or(std::string("none"));
    education_level(j.education);  // validates
    if (const auto* a = x["skills"].as_array())
      for (const auto& e : *a) j.skills.push_back(e.value<std::string>().value());
    cat.jobs.push_back(std::move(j));
  }

  const toml::table biz = toml::parse_file(dir + "/businesses.toml");
  cat.words = strings(biz["words"], "businesses.words");
  for (const auto& n : *biz["type"].as_array()) {
    const auto& x = *n.as_table();
    BusinessType b;
    b.id = x["id"].value<std::string>().value();
    b.count = x["count"].value_or(1);
    b.forms = strings(x["forms"], "businesses." + b.id);
    cat.businesses.push_back(std::move(b));
  }
  if (const auto* cul = biz["culture"].as_table())
    for (const auto& [cid, forms] : *cul)
      for (const auto& [type, list] : *forms.as_table()) {
        BusinessType* b = nullptr;
        for (auto& x : cat.businesses)
          if (x.id == type.str()) b = &x;
        if (!b) throw std::runtime_error("gen data: culture forms for unknown business " + std::string(type.str()));
        auto& out = b->culture_forms[std::string(cid.str())];
        for (const auto& e : *list.as_array()) out.push_back(e.value<std::string>().value());
      }
  for (const Job& j : cat.jobs)
    if (!j.workplace.empty() && !std::any_of(cat.businesses.begin(), cat.businesses.end(), [&](const BusinessType& b) {
          return b.id == j.workplace || b.id.ends_with("_" + j.workplace);
        }))
      throw std::runtime_error("gen data: job " + j.id + " works at unknown business " + j.workplace);
  return cat;
}

}  // namespace zs::gen
