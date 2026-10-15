// Spec 90: where a country's profile lives. A sourced country's profile is data/setup/nations/real/<ISO>.toml; a
// country with no profile is its income group's base; a custom country (id "custom/<id>") lives in the world's own
// copy of its pack. What is keyed by a real country (calendar, police, wages, absence, missing persons) follows a
// custom country's template.
#pragma once
#include <string>

namespace zs {

// The folder holding the open world's custom countries (its countries/ copy of its pack); empty: none. Set when a
// world is opened (one world per process, decision 0052).
void set_custom_nations_dir(const std::string& dir);
const std::string& custom_nations_dir();

bool is_custom_profile(const std::string& profile);
// The nation file a country's demographics are read from.
std::string nation_file(const std::string& data_dir, const std::string& profile, const std::string& income);
// The profile the ISO-keyed tables use: the profile itself, or a custom country's template ("" for a base template).
std::string table_profile(const std::string& profile);

}  // namespace zs
