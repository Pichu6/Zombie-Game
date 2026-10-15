// Spec 90: the Atlas's Countries page: the default pack (read-only, the guide), the user's packs, a country's card,
// and the editor of a custom country (simple, or advanced: every table).
#pragma once
#include <memory>
#include <string>

namespace zs::atlas {

struct CountriesState;
std::shared_ptr<CountriesState> make_countries(const std::string& data_dir);
void draw_countries(CountriesState& st);
// For checks without a click: open a user pack and select one of its custom countries (empty id: none), advanced or not.
void countries_pick(CountriesState& st, const std::string& pack, const std::string& id, bool advanced);

}  // namespace zs::atlas
