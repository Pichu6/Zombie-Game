// Worlds as folders (decision 0074, spec 55): `worlds/<name>/world.toml` next to the executable, with the world's
// seed, when it was made, notes and its saves under `saves/`. Used by the Atlas, the window (F5, F9) and `zs world`.
#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "zs/gen/place.h"

namespace zs::worlds {

struct SaveFile {
  std::filesystem::path path;
  std::string label;  // the file name without its extension: "<date>-tick<N>" or a user name
};

struct World {
  std::string name;
  std::uint64_t seed = 1;
  std::string created;   // ISO date and time, local
  std::string built;     // the build that made it (its date), spec 55 rule 6
  std::string notes;
  std::string forked_from;
  zs::gen::WorldSettings settings;  // spec 55 slice 5: the [settings] table; defaults when absent
  std::string realism;              // "realistic" or "sandbox", from the bands at the last save
  std::filesystem::path dir;
  std::vector<SaveFile> saves;  // newest first
};

// The folder the executable runs from (the `play` folder, or the build's apps/zs). Worlds live in `<that>/worlds`.
std::filesystem::path exe_dir();
std::filesystem::path root();
// A world name is a folder name: letters, digits, spaces, dots, dashes and underscores; nothing else.
bool valid_name(const std::string& name);

std::vector<World> list();                         // by created, newest first
bool exists(const std::string& name);
World create(const std::string& name, std::uint64_t seed, const std::string& notes = "");
World fork(const World& from, const std::string& new_name);  // same seed, the saves left behind
World load(const std::string& name);                         // throws when it is not there
void save_meta(const World& w);                              // rewrites world.toml (name, notes)
// To the recycle bin on Windows; to `worlds/_trash/<name>-<time>` elsewhere. Never erased by this code.
void remove(const World& w);
// A file or folder to the recycle bin on Windows; elsewhere to `<fallback_root>/_trash/`.
void to_trash(const std::filesystem::path& path, const std::filesystem::path& fallback_root);
// A world's settings are locked once it has a save (decision 0074 rule 4): then a change forks.
inline bool settings_locked(const World& w) { return !w.saves.empty(); }
// Presets: `presets/world/<name>.toml` next to the executable, the same [settings] table as world.toml.
std::filesystem::path presets_root();
std::vector<std::string> list_presets();
zs::gen::WorldSettings load_preset(const std::string& name);
void save_preset(const std::string& name, const zs::gen::WorldSettings& s);
// "2027-01-11" <-> a day number.
std::string date_text(std::int64_t day);
bool parse_date(const std::string& text, std::int64_t& day);

// Where the window writes its next save for a world at a tick: `saves/<game date>-tick<N>.sav`.
std::filesystem::path next_save_path(const World& w, const std::string& game_date, std::int64_t tick);

// `zs world new|list|fork|delete ...`: argv[0] is "world".
int cli(int argc, char** argv);

}  // namespace zs::worlds
