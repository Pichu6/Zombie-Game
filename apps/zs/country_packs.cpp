#include "country_packs.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "worlds.h"
#include "zs/tier0.h"

namespace zs::packs {

namespace {

std::string quoted(const std::string& s) {
  std::string out = "\"";
  for (char c : s) {
    if (c == '"' || c == '\\') out += '\\';
    if (c == '\n') out += "\\n";
    else out += c;
  }
  return out + "\"";
}

// A file stem from a name: lower case letters and digits, others as underscores.
std::string stem_of(const std::string& name) {
  std::string s;
  for (char c : name) s += std::isalnum(static_cast<unsigned char>(c)) ? static_cast<char>(std::tolower(static_cast<unsigned char>(c))) : '_';
  while (!s.empty() && s.back() == '_') s.pop_back();
  return s.empty() ? "country" : s;
}

}  // namespace

std::filesystem::path root() { return zs::worlds::exe_dir() / "packs" / "countries"; }

std::vector<std::string> list() {
  std::vector<std::string> out;
  std::error_code ec;
  for (const auto& e : std::filesystem::directory_iterator(root(), ec))
    if (e.is_directory() && std::filesystem::exists(e.path() / "pack.toml", ec)) out.push_back(e.path().filename().string());
  std::sort(out.begin(), out.end());
  return out;
}

Pack load(const std::string& id) {
  Pack p;
  p.id = id;
  p.dir = root() / id;
  const toml::table t = toml::parse_file((p.dir / "pack.toml").string());
  p.name = t["name"].value_or(id);
  p.notes = t["notes"].value_or(std::string{});
  if (const toml::array* inc = t["include"].as_array())
    for (const auto& v : *inc)
      if (auto s = v.value<std::string>()) p.include.push_back(*s);
  return p;
}

void save(const Pack& p) {
  std::filesystem::create_directories(p.dir);
  std::ofstream o(p.dir / "pack.toml", std::ios::binary);
  if (!o) throw std::runtime_error("cannot write " + (p.dir / "pack.toml").string());
  o << "# A country pack (spec 90): the default countries it includes, by ISO code (\"all\" for every one), and its\n"
    << "# custom countries, one file each beside this one.\n"
    << "name = " << quoted(p.name) << "\n"
    << "notes = " << quoted(p.notes) << "\n"
    << "include = [";
  for (std::size_t i = 0; i < p.include.size(); ++i) o << (i ? ", " : "") << quoted(p.include[i]);
  o << "]\n";
}

Pack create(const std::string& id, const std::string& name) {
  if (!zs::worlds::valid_name(id) || id == kEarth) throw std::runtime_error("a pack name uses letters, digits, spaces, dots, dashes and underscores");
  Pack p;
  p.id = id;
  p.name = name.empty() ? id : name;
  p.dir = root() / id;
  if (std::filesystem::exists(p.dir / "pack.toml")) throw std::runtime_error("a pack called \"" + id + "\" already exists");
  save(p);
  return p;
}

void remove(const std::string& id) { zs::worlds::to_trash(root() / id, root()); }

std::string dir_of(const std::string& pack_id) { return pack_id.empty() || pack_id == kEarth ? std::string{} : (root() / pack_id).string(); }

std::vector<Default> defaults(const std::string& data_dir) {
  std::vector<Default> out;
  std::error_code ec;
  for (const auto& e : std::filesystem::directory_iterator(std::filesystem::path(data_dir) / "setup" / "nations" / "real", ec)) {
    if (e.path().extension() != ".toml") continue;
    try {
      const toml::table t = toml::parse_file(e.path().string());
      Default d;
      d.iso = t["id"].value_or(e.path().stem().string());
      d.name = t["name"].value_or(d.iso);
      d.income = t["country"]["income"].value_or(std::string{});
      d.population = t["country"]["population"].value_or(0.0);
      if (d.population > 0) out.push_back(std::move(d));
    } catch (const std::exception&) {
    }
  }
  std::sort(out.begin(), out.end(), [](const Default& a, const Default& b) { return a.name < b.name; });
  return out;
}

std::vector<std::string> custom_ids(const Pack& p) {
  std::vector<std::string> out;
  std::error_code ec;
  for (const auto& e : std::filesystem::directory_iterator(p.dir, ec))
    if (e.path().extension() == ".toml" && e.path().filename() != "pack.toml") out.push_back(e.path().stem().string());
  std::sort(out.begin(), out.end());
  return out;
}

toml::table load_country(const Pack& p, const std::string& id) { return toml::parse_file((p.dir / (id + ".toml")).string()); }

void save_country(const Pack& p, const std::string& id, const toml::table& t) {
  std::ofstream o(p.dir / (id + ".toml"), std::ios::binary);
  if (!o) throw std::runtime_error("cannot write the country " + id);
  o << "# A custom country (spec 90): a nation profile made from its template and edited in the Atlas.\n" << t << "\n";
}

std::string new_country(const Pack& p, const std::string& data_dir, const std::string& template_id, const std::string& name) {
  const std::filesystem::path src = template_id.rfind("base_", 0) == 0
                                        ? std::filesystem::path(data_dir) / "setup" / "nations" / (template_id + ".toml")
                                        : std::filesystem::path(data_dir) / "setup" / "nations" / "real" / (template_id + ".toml");
  toml::table t = toml::parse_file(src.string());
  std::string id = stem_of(name);
  for (int k = 2; std::filesystem::exists(p.dir / (id + ".toml")); ++k) id = stem_of(name) + "_" + std::to_string(k);
  t.insert_or_assign("id", id);
  t.insert_or_assign("name", name);
  t.insert_or_assign("status", "custom");
  t.insert_or_assign("custom", true);
  t.insert_or_assign("template", template_id);
  if (!t.contains("color")) {
    // A keyed colour from the name, so two countries rarely share one.
    std::uint32_t h = 2166136261u;
    for (char c : id) h = (h ^ static_cast<unsigned char>(c)) * 16777619u;
    std::ostringstream c;
    c << "#" << std::hex << ((h & 0x7f7f7f) | 0x404040);
    t.insert_or_assign("color", c.str());
  }
  if (toml::table* c = t["country"].as_table(); c && !c->contains("population")) c->insert_or_assign("population", 10000000.0);
  save_country(p, id, t);
  return id;
}

void remove_country(const Pack& p, const std::string& id) { zs::worlds::to_trash(p.dir / (id + ".toml"), p.dir); }

int country_count(const Pack& p, const std::string& data_dir) {
  int n = static_cast<int>(custom_ids(p).size());
  if (std::find(p.include.begin(), p.include.end(), "all") != p.include.end()) return n + static_cast<int>(defaults(data_dir).size());
  return n + static_cast<int>(p.include.size());
}

void copy_into(const std::string& pack_id, const std::filesystem::path& world_dir) {
  if (pack_id.empty() || pack_id == kEarth) return;
  const std::filesystem::path from = root() / pack_id, to = world_dir / "countries";
  if (!std::filesystem::exists(from / "pack.toml")) throw std::runtime_error("no country pack called \"" + pack_id + "\"");
  std::filesystem::create_directories(to);
  std::filesystem::copy(from, to, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
}

}  // namespace zs::packs
