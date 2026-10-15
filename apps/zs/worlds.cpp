#include "worlds.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "toml.hpp"
#include "zs/calendar.h"
#include "zs/tier0.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#endif

namespace zs::worlds {

namespace {

std::string now_text() {
  const std::time_t t = std::time(nullptr);
  char buf[32];
  std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M", std::localtime(&t));
  return buf;
}

std::string stamp_text() {
  const std::time_t t = std::time(nullptr);
  char buf[32];
  std::strftime(buf, sizeof buf, "%Y%m%d-%H%M%S", std::localtime(&t));
  return buf;
}

std::string quoted(const std::string& s) {
  std::string out = "\"";
  for (char c : s) {
    if (c == '"' || c == '\\') out += '\\';
    if (c == '\n') out += "\\n";
    else out += c;
  }
  return out + "\"";
}

void write_settings(std::ostream& o, const zs::gen::WorldSettings& s) {
  o << "\n[settings]\n"
    << "land_share = " << s.land_share << "\n"
    << "global_temp_c = " << s.global_temp_c << "\n"
    << "relief = " << s.relief << "\n"
    << "moisture = " << s.moisture << "\n"
    << "fragmentation = " << s.fragmentation << "\n"
    << "start_date = " << quoted(date_text(s.start_day)) << "\n"
    << "radius_km = " << s.radius_km << "\n"
    << "fuel_richness = " << s.fuel_richness << "\n"
    << "metal_richness = " << s.metal_richness << "\n"
    << "fertility = " << s.fertility << "\n"
    << "forest = " << s.forest << "\n"
    << "fish = " << s.fish << "\n"
    << "people = " << s.people << "\n"
    << "continents = " << s.continents << "\n"
    << "pack = " << quoted(s.pack) << "\n"  // spec 90
    << "countries = " << s.countries << "\n"  // spec 91
    << "history = " << s.history << "\n"
    << "history_seed = " << static_cast<std::int64_t>(s.history_seed) << "\n";
}

zs::gen::WorldSettings read_settings(const toml::table& t) {
  zs::gen::WorldSettings s;
  if (const toml::table* st = t["settings"].as_table()) {
    s.land_share = (*st)["land_share"].value_or(s.land_share);
    s.global_temp_c = (*st)["global_temp_c"].value_or(s.global_temp_c);
    s.relief = (*st)["relief"].value_or(s.relief);
    s.moisture = (*st)["moisture"].value_or(s.moisture);
    s.fragmentation = (*st)["fragmentation"].value_or(s.fragmentation);
    if (auto d = (*st)["start_date"].value<std::string>()) parse_date(*d, s.start_day);
    s.radius_km = (*st)["radius_km"].value_or(s.radius_km);
    s.fuel_richness = (*st)["fuel_richness"].value_or(s.fuel_richness);
    s.metal_richness = (*st)["metal_richness"].value_or(s.metal_richness);
    s.fertility = (*st)["fertility"].value_or(s.fertility);
    s.forest = (*st)["forest"].value_or(s.forest);
    s.fish = (*st)["fish"].value_or(s.fish);
    s.people = (*st)["people"].value_or(s.people);
    s.continents = (*st)["continents"].value_or(s.continents);
    s.pack = (*st)["pack"].value_or(s.pack);
    s.countries = (*st)["countries"].value_or(s.countries);
    s.history = (*st)["history"].value_or(s.history);
    s.history_seed = static_cast<std::uint64_t>((*st)["history_seed"].value_or<std::int64_t>(0));
  }
  return s;
}

void write_meta(const World& w) {
  std::ofstream o(w.dir / "world.toml", std::ios::binary);
  if (!o) throw std::runtime_error("cannot write " + (w.dir / "world.toml").string());
  o << "# A world (decision 0074): its seed and settings. Saves live in saves/.\n"
    << "name = " << quoted(w.name) << "\n"
    << "seed = " << w.seed << "\n"
    << "created = " << quoted(w.created) << "\n"
    << "built = " << quoted(w.built) << "\n"
    << "notes = " << quoted(w.notes) << "\n";
  if (!w.forked_from.empty()) o << "forked_from = " << quoted(w.forked_from) << "\n";
  if (!w.realism.empty()) o << "realism = " << quoted(w.realism) << "\n";
  write_settings(o, w.settings);
}

void read_saves(World& w) {
  w.saves.clear();
  std::error_code ec;
  for (const auto& e : std::filesystem::directory_iterator(w.dir / "saves", ec)) {
    if (!e.is_regular_file() || e.path().extension() != ".sav") continue;
    w.saves.push_back({e.path(), e.path().stem().string()});
  }
  std::sort(w.saves.begin(), w.saves.end(), [](const SaveFile& a, const SaveFile& b) {
    std::error_code ec1, ec2;
    return std::filesystem::last_write_time(a.path, ec1) > std::filesystem::last_write_time(b.path, ec2);
  });
}

World read_world(const std::filesystem::path& dir) {
  World w;
  w.dir = dir;
  w.name = dir.filename().string();
  const toml::table t = toml::parse_file((dir / "world.toml").string());
  w.name = t["name"].value_or(w.name);
  w.seed = static_cast<std::uint64_t>(t["seed"].value_or<std::int64_t>(1));
  w.created = t["created"].value_or("");
  w.built = t["built"].value_or("");
  w.notes = t["notes"].value_or("");
  w.forked_from = t["forked_from"].value_or("");
  w.realism = t["realism"].value_or("");
  w.settings = read_settings(t);
  if (w.settings.pack != "earth") w.settings.pack_dir = (dir / "countries").string();  // spec 90: the world's own copy
  read_saves(w);
  return w;
}

}  // namespace

std::filesystem::path exe_dir() {
#ifdef _WIN32
  wchar_t buf[MAX_PATH * 2];
  const DWORD n = GetModuleFileNameW(nullptr, buf, static_cast<DWORD>(std::size(buf)));
  if (n > 0 && n < std::size(buf)) return std::filesystem::path(buf).parent_path();
#else
  char buf[4096];
  const ssize_t n = readlink("/proc/self/exe", buf, sizeof buf - 1);
  if (n > 0) {
    buf[n] = 0;
    return std::filesystem::path(buf).parent_path();
  }
#endif
  return std::filesystem::current_path();
}

std::filesystem::path root() { return exe_dir() / "worlds"; }

bool valid_name(const std::string& name) {
  if (name.empty() || name.size() > 64 || name[0] == '_' || name[0] == '.' || name[0] == ' ') return false;
  for (char c : name)
    if (!(std::isalnum(static_cast<unsigned char>(c)) || c == ' ' || c == '.' || c == '-' || c == '_')) return false;
  return name.back() != ' ' && name.back() != '.';
}

std::vector<World> list() {
  std::vector<World> out;
  std::error_code ec;
  for (const auto& e : std::filesystem::directory_iterator(root(), ec)) {
    if (!e.is_directory() || e.path().filename().string()[0] == '_') continue;
    if (!std::filesystem::exists(e.path() / "world.toml", ec)) continue;
    try {
      out.push_back(read_world(e.path()));
    } catch (const std::exception& ex) {
      std::fprintf(stderr, "world %s: %s\n", e.path().string().c_str(), ex.what());
    }
  }
  std::sort(out.begin(), out.end(), [](const World& a, const World& b) { return a.created > b.created; });
  return out;
}

bool exists(const std::string& name) {
  std::error_code ec;
  return std::filesystem::exists(root() / name / "world.toml", ec);
}

World create(const std::string& name, std::uint64_t seed, const std::string& notes) {
  if (!valid_name(name)) throw std::runtime_error("a world name uses letters, digits, spaces, dots, dashes and underscores");
  if (exists(name)) throw std::runtime_error("a world called \"" + name + "\" already exists");
  World w;
  w.name = name;
  w.seed = seed;
  w.created = now_text();
  w.built = __DATE__;
  w.notes = notes;
  w.dir = root() / name;
  std::filesystem::create_directories(w.dir / "saves");
  write_meta(w);
  return w;
}

World fork(const World& from, const std::string& new_name) {
  World w = create(new_name, from.seed, from.notes);
  w.forked_from = from.name;
  w.settings = from.settings;
  w.realism = from.realism;
  std::error_code ec;
  if (std::filesystem::exists(from.dir / "countries", ec)) {  // spec 90: the pack goes with the world
    std::filesystem::copy(from.dir / "countries", w.dir / "countries", std::filesystem::copy_options::recursive);
    w.settings.pack_dir = (w.dir / "countries").string();
  }
  write_meta(w);
  return w;
}

std::filesystem::path presets_root() { return exe_dir() / "presets" / "world"; }

// Spec 84 rule 5: shipped presets in data/atlas/presets/world, the user's own next to the executable (theirs win a name).
std::filesystem::path shipped_presets_root() { return std::filesystem::path(zs::tier0::data_dir()) / "atlas" / "presets" / "world"; }

std::vector<std::string> list_presets() {
  std::vector<std::string> out;
  for (const auto& root : {presets_root(), shipped_presets_root()}) {
    std::error_code ec;
    for (const auto& e : std::filesystem::directory_iterator(root, ec))
      if (e.is_regular_file() && e.path().extension() == ".toml") out.push_back(e.path().stem().string());
  }
  std::sort(out.begin(), out.end());
  out.erase(std::unique(out.begin(), out.end()), out.end());
  return out;
}

zs::gen::WorldSettings load_preset(const std::string& name) {
  std::filesystem::path p = presets_root() / (name + ".toml");
  if (!std::filesystem::exists(p)) p = shipped_presets_root() / (name + ".toml");
  return read_settings(toml::parse_file(p.string()));
}

void save_preset(const std::string& name, const zs::gen::WorldSettings& s) {
  if (!valid_name(name)) throw std::runtime_error("a preset name uses letters, digits, spaces, dots, dashes and underscores");
  std::filesystem::create_directories(presets_root());
  std::ofstream o(presets_root() / (name + ".toml"), std::ios::binary);
  if (!o) throw std::runtime_error("cannot write the preset");
  o << "# A world preset (spec 55 slice 5): the [settings] table of a world.toml.\nname = " << quoted(name) << "\n";
  write_settings(o, s);
}

std::string date_text(std::int64_t day) {
  const zs::CivilDate d = zs::civil_from_days(day);
  char buf[32];
  std::snprintf(buf, sizeof buf, "%04d-%02d-%02d", d.year, d.month, d.day);
  return buf;
}

bool parse_date(const std::string& text, std::int64_t& day) {
  int y = 0, m = 0, d = 0;
  if (std::sscanf(text.c_str(), "%d-%d-%d", &y, &m, &d) != 3) return false;
  if (y < 1 || y > 9999 || m < 1 || m > 12 || d < 1 || d > 31) return false;
  const std::int64_t n = zs::days_from_civil(y, m, d);
  if (zs::civil_from_days(n).day != d) return false;  // 31 February and the like
  day = n;
  return true;
}

World load(const std::string& name) {
  if (!exists(name)) throw std::runtime_error("no world called \"" + name + "\"");
  return read_world(root() / name);
}

void save_meta(const World& w) { write_meta(w); }

void remove(const World& w) { to_trash(w.dir, root()); }

void to_trash(const std::filesystem::path& path, const std::filesystem::path& fallback_root) {
#ifdef _WIN32
  std::wstring from = path.wstring();
  from.push_back(0);
  from.push_back(0);  // double null terminated
  SHFILEOPSTRUCTW op{};
  op.wFunc = FO_DELETE;
  op.pFrom = from.c_str();
  op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
  if (SHFileOperationW(&op) != 0) throw std::runtime_error("could not send " + path.filename().string() + " to the recycle bin");
#else
  std::filesystem::create_directories(fallback_root / "_trash");
  std::filesystem::rename(path, fallback_root / "_trash" / (path.filename().string() + "-" + stamp_text()));
#endif
}

std::filesystem::path next_save_path(const World& w, const std::string& game_date, std::int64_t tick) {
  std::string date = game_date;
  for (char& c : date)
    if (c == ' ' || c == ':') c = '-';
  return w.dir / "saves" / (date + "-tick" + std::to_string(tick) + ".sav");
}

int cli(int argc, char** argv) {
  const std::string cmd = argc >= 2 ? argv[1] : "";
  try {
    if (cmd == "list") {
      for (const World& w : list())
        std::cout << w.name << "  seed " << w.seed << "  created " << w.created << "  saves " << w.saves.size()
                  << (w.forked_from.empty() ? "" : "  fork of " + w.forked_from) << "\n";
      return 0;
    }
    if (cmd == "new" && argc >= 3) {
      std::uint64_t seed = static_cast<std::uint64_t>(std::chrono::system_clock::now().time_since_epoch().count() % 1000000);
      for (int i = 3; i + 1 < argc; ++i)
        if (!std::strcmp(argv[i], "--seed")) seed = std::strtoull(argv[i + 1], nullptr, 10);
      const World w = create(argv[2], seed);
      std::cout << "made " << w.dir.string() << " with seed " << w.seed << "\n";
      return 0;
    }
    if (cmd == "fork" && argc >= 4) {
      const World w = fork(load(argv[2]), argv[3]);
      std::cout << "forked " << argv[2] << " into " << w.dir.string() << "\n";
      return 0;
    }
    if (cmd == "delete" && argc >= 3) {
      remove(load(argv[2]));
      std::cout << "sent " << argv[2] << " to the recycle bin\n";
      return 0;
    }
  } catch (const std::exception& ex) {
    std::cerr << "zs world: " << ex.what() << "\n";
    return 1;
  }
  std::cout << "usage: zs world list | new NAME [--seed N] | fork NAME NEW_NAME | delete NAME\n"
            << "       worlds live in " << root().string() << "\n";
  return 2;
}

}  // namespace zs::worlds
