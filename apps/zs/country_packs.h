// Spec 90: country packs. The default pack "earth" is the sourced profiles (data/setup/nations/real/, read-only); a
// user's packs live next to the executable, `packs/countries/<id>/`: pack.toml (name, notes, the default countries it
// includes) and one file per custom country, a nation profile with custom = true, its template and its colour. A
// world made with a user pack keeps its own copy (`<world>/countries/`), so editing the pack later leaves it as made.
#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "toml.hpp"

namespace zs::packs {

inline constexpr const char* kEarth = "earth";

struct Pack {
  std::string id, name, notes;
  std::vector<std::string> include;  // ISO codes of default countries; "all" for every one
  std::filesystem::path dir;
};

// A default country, as the Countries page lists it.
struct Default {
  std::string iso, name, income;
  double population = 0;
};

std::filesystem::path root();                     // <exe>/packs/countries
std::vector<std::string> list();                  // the user's pack ids, sorted
Pack load(const std::string& id);
void save(const Pack& p);                         // rewrites pack.toml
Pack create(const std::string& id, const std::string& name);
void remove(const std::string& id);               // to the recycle bin
// The folder a world's settings point a pack at for generation (empty for earth).
std::string dir_of(const std::string& pack_id);

std::vector<Default> defaults(const std::string& data_dir);      // the earth pack, by name
std::vector<std::string> custom_ids(const Pack& p);              // its custom countries' file stems, sorted
toml::table load_country(const Pack& p, const std::string& id);
void save_country(const Pack& p, const std::string& id, const toml::table& t);
// A new custom country from a template (an ISO code of a default country, or "base_<income>"): its tables copied, its
// status "custom", its name, colour and template set. Returns the new id.
std::string new_country(const Pack& p, const std::string& data_dir, const std::string& template_id, const std::string& name);
void remove_country(const Pack& p, const std::string& id);
int country_count(const Pack& p, const std::string& data_dir);  // included plus custom

// Copies a user pack into a world's folder (`countries/`); nothing for earth.
void copy_into(const std::string& pack_id, const std::filesystem::path& world_dir);

}  // namespace zs::packs
