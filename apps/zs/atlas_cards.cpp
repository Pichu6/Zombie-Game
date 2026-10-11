#include "atlas_cards.h"

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <optional>
#include <stdexcept>

#include "toml.hpp"
#include "zs/gen/catalog.h"
#include "zs/gen/dwellings.h"
#include "zs/gen/keyed.h"
#include "zs/gen/pops.h"
#include "zs/calendar.h"
#include "worlds.h"

namespace zs::atlas {

namespace {

struct NeedRow {
  std::string field, need, spec, note;
};

struct NeedCard {
  std::string id, title, design;
  std::vector<NeedRow> rows;
};

// A setting's bands (cards.toml [[setting]]; sandbox parameters §1.4).
struct SettingBand {
  std::string field, label, what;
  double earth = 0, r0 = 0, r1 = 0, s0 = 0, s1 = 0, step = 0.01;
};

// The values a card has for its fields: field -> text. A field the engine produces but the card does not read yet
// stays absent and shows as "not read yet".
using Values = std::map<std::string, std::string>;

std::string fmt(const char* f, double v) {
  char buf[64];
  std::snprintf(buf, sizeof buf, f, v);
  return buf;
}

}  // namespace

struct CardsState {
  std::string data_dir;
  std::vector<NeedCard> needs;
  std::string error;
  int country = -1, town = -1, pop = -1, house = -1;
  // The selected planet town, peopled by the generator for its country (a stand-in until spec 48's towns carry
  // their own people): the town, its dwellings and the rules, built once per town.
  std::unique_ptr<gen::Catalog> catalog;
  std::optional<gen::DwellingRules> rules;
  std::optional<gen::Town> gen_town;
  std::vector<gen::Dwelling> dwellings;
  std::string gen_income, gen_error, house_map;
  std::uint64_t gen_seed = 0;
  int gen_for_town = -1;
  int gen_for_country = -1;
  bool autopick = false;
  std::map<std::string, std::string> calendars;  // profile id -> one line (spec 53), read once
  // Slice 5: the settings and their bands; the draft being edited and the world it belongs to.
  std::vector<SettingBand> bands;
  gen::WorldSettings draft;
  std::string draft_world;
  char date_text[16] = "";
  char preset_name[64] = "";
  char fork_name[64] = "";
  int preset_pick = -1;
  std::string edit_error, edit_note;
};

std::shared_ptr<CardsState> make_cards(const std::string& data_dir) {
  auto st = std::make_shared<CardsState>();
  st->data_dir = data_dir;
  try {
    const toml::table t = toml::parse_file(data_dir + "/atlas/cards.toml");
    if (const toml::array* cards = t["card"].as_array())
      for (const auto& e : *cards)
        if (const toml::table* c = e.as_table()) {
          NeedCard nc{(*c)["id"].value_or(""), (*c)["title"].value_or(""), (*c)["design"].value_or(""), {}};
          if (const toml::array* rows = (*c)["rows"].as_array())
            for (const auto& r : *rows)
              if (const toml::table* rt = r.as_table())
                nc.rows.push_back({(*rt)["field"].value_or(""), (*rt)["need"].value_or(""), (*rt)["spec"].value_or(""),
                                   (*rt)["note"].value_or("")});
          st->needs.push_back(nc);
        }
    if (const toml::array* settings = t["setting"].as_array())
      for (const auto& e : *settings)
        if (const toml::table* c = e.as_table()) {
          SettingBand b;
          b.field = (*c)["field"].value_or("");
          b.label = (*c)["label"].value_or("");
          b.what = (*c)["what"].value_or("");
          b.earth = (*c)["earth"].value_or(0.0);
          b.step = (*c)["step"].value_or(0.01);
          if (const toml::array* r = (*c)["realistic"].as_array(); r && r->size() == 2) b.r0 = (*r)[0].value_or(0.0), b.r1 = (*r)[1].value_or(0.0);
          if (const toml::array* r = (*c)["sandbox"].as_array(); r && r->size() == 2) b.s0 = (*r)[0].value_or(0.0), b.s1 = (*r)[1].value_or(0.0);
          st->bands.push_back(b);
        }
  } catch (const std::exception& ex) {
    st->error = ex.what();
  }
  return st;
}

void cards_autopick(CardsState& st, bool on) { st.autopick = on; }

int cards_country(const CardsState& st) { return st.country; }

namespace {

double* field_of(gen::WorldSettings& s, const std::string& field) {
  if (field == "land_share") return &s.land_share;
  if (field == "global_temp_c") return &s.global_temp_c;
  if (field == "relief") return &s.relief;
  if (field == "moisture") return &s.moisture;
  if (field == "fragmentation") return &s.fragmentation;
  return nullptr;
}

}  // namespace

std::string realism_of(const CardsState& st, const gen::WorldSettings& s) {
  gen::WorldSettings copy = s;
  for (const SettingBand& b : st.bands)
    if (const double* v = field_of(copy, b.field); v && (*v < b.r0 || *v > b.r1)) return "sandbox";
  return "realistic";
}

int touched_count(const CardsState& st, const gen::WorldSettings& s) {
  gen::WorldSettings copy = s;
  int n = 0;
  for (const SettingBand& b : st.bands)
    if (const double* v = field_of(copy, b.field); v && std::abs(*v - b.earth) > 1e-9) ++n;
  if (s.start_day != kDefaultStartDay) ++n;
  return n;
}

namespace {

const NeedCard* needs_of(const CardsState& st, const char* id) {
  for (const auto& c : st.needs)
    if (c.id == id) return &c;
  return nullptr;
}

// The card itself: title, then one row per need with its value, its spec, or "lacking".
void draw_card(const CardsState& st, const char* id, const std::string& heading, const Values& values) {
  const NeedCard* card = needs_of(st, id);
  ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.3f);
  ImGui::TextColored({1, 0.95f, 0.8f, 1}, "%s", heading.c_str());
  ImGui::PopFont();
  if (!card) {
    ImGui::TextDisabled("no needs table for %s in data/atlas/cards.toml", id);
    return;
  }
  int has = 0, lacking = 0, unread = 0;
  for (const auto& r : card->rows) {
    if (r.spec.empty()) ++lacking;
    else if (values.count(r.field)) ++has;
    else ++unread;
  }
  ImGui::TextDisabled("%s: %d rows produced, %d produced but not read here, %d lacking  (needs from %s)", card->title.c_str(), has,
                      unread, lacking, card->design.c_str());
  const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
  if (ImGui::BeginTable(id, 3, flags)) {
    ImGui::TableSetupColumn("Needs", 0, 3);
    ImGui::TableSetupColumn("Has", 0, 5);
    ImGui::TableSetupColumn("Spec", 0, 0.8f);
    ImGui::TableHeadersRow();
    for (const auto& r : card->rows) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextWrapped("%s", r.need.c_str());
      ImGui::TableSetColumnIndex(1);
      if (r.spec.empty()) ImGui::TextColored({0.9f, 0.55f, 0.4f, 1}, "lacking");
      else if (auto it = values.find(r.field); it != values.end()) {
        ImGui::TextWrapped("%s", it->second.c_str());
        if (!r.note.empty()) ImGui::TextDisabled("%s", r.note.c_str());
      } else ImGui::TextDisabled("not read yet");
      ImGui::TableSetColumnIndex(2);
      if (!r.spec.empty()) ImGui::TextDisabled("%s", r.spec.c_str());
    }
    ImGui::EndTable();
  }
}

// Peoples the selected planet town for its country, once.
void ensure_town(CardsState& st, const WorldCard& world, const CountryCard& c, const TownCard& t) {
  if (st.gen_town && st.gen_for_town == t.index && st.gen_for_country == st.country) return;
  st.gen_town.reset();
  st.dwellings.clear();
  st.gen_error.clear();
  st.house_map.clear();
  st.pop = st.house = -1;
  st.gen_for_town = t.index;
  st.gen_for_country = st.country;
  try {
    if (!st.catalog) st.catalog = std::make_unique<gen::Catalog>(gen::load_catalog(st.data_dir + "/gen"));
    if (!st.rules) st.rules = gen::load_dwelling_rules(st.data_dir + "/gen");
    gen::Mix mix = gen::load_mix(st.data_dir + "/gen/populations/tier0_block.toml");
    gen::set_country(mix, c.profile.empty() ? "base_" + c.income : "real/" + c.profile, st.data_dir);
    mix.people = 120;  // a roster worth reading; the planet town's people are the card's own row
    st.gen_seed = gen::mix64(world.seed, t.cell);
    gen::Town town = gen::generate_town(*st.catalog, mix, st.gen_seed);
    gen::add_lifepaths(*st.catalog, town);
    st.gen_income = gen::income_group(mix);
    st.dwellings = gen::plan_dwellings(town, *st.rules, st.gen_income, st.gen_seed);
    gen::assign_addresses(town, st.dwellings);
    st.gen_town = std::move(town);
  } catch (const std::exception& ex) {
    st.gen_error = ex.what();
  }
}

// The settings section of the world card: a draft of the world's settings, a slider per band, the start date, presets,
// and Apply (unlocked) or Fork (locked). Settings change numbers, never rules (sandbox parameters §1.1).
void draw_world_settings(CardsState& st, WorldEditor& ed) {
  if (st.draft_world != ed.world_name) {  // a new world: the draft starts from its record
    st.draft = *ed.settings;
    st.draft_world = ed.world_name;
    std::snprintf(st.date_text, sizeof st.date_text, "%s", zs::worlds::date_text(st.draft.start_day).c_str());
    st.edit_error.clear();
    st.edit_note.clear();
    std::snprintf(st.fork_name, sizeof st.fork_name, "%s 2", ed.world_name.c_str());
  }
  ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.2f);
  ImGui::TextColored({1, 0.95f, 0.8f, 1}, "Settings of %s", ed.world_name.c_str());
  ImGui::PopFont();
  ImGui::TextWrapped("%s", ed.locked ? "This world has saves, so its setup is locked (0074 rule 4): changes go to a fork with the same seed."
                                     : "No saves yet: changes apply to this world itself.");
  bool dirty = !(st.draft == *ed.settings);
  for (const SettingBand& b : st.bands) {
    double* v = field_of(st.draft, b.field);
    if (!v) continue;
    ImGui::PushID(b.field.c_str());
    const bool realistic = *v >= b.r0 && *v <= b.r1;
    const bool touched = std::abs(*v - b.earth) > 1e-9;
    ImGui::PushStyleColor(ImGuiCol_FrameBg, realistic ? ImVec4(0.16f, 0.30f, 0.20f, 1) : ImVec4(0.38f, 0.28f, 0.10f, 1));
    ImGui::SetNextItemWidth(260);
    ImGui::SliderScalar("##v", ImGuiDataType_Double, v, &b.s0, &b.s1, b.step >= 0.5 ? "%.1f" : "%.2f");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Text("%s%s", touched ? "* " : "", b.label.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("[%s %.2g to %.2g; sandbox to %.2g]", realistic ? "realistic" : "SANDBOX", b.r0, b.r1, b.s1);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", b.what.c_str());
    ImGui::SameLine();
    if (ImGui::SmallButton("Earth")) *v = b.earth;
    ImGui::PopID();
  }
  ImGui::SetNextItemWidth(120);
  ImGui::InputText("##date", st.date_text, sizeof st.date_text);
  ImGui::SameLine();
  std::int64_t day = 0;
  const bool date_ok = zs::worlds::parse_date(st.date_text, day);
  if (date_ok) st.draft.start_day = day;
  ImGui::Text("%sStart date (YYYY-MM-DD)", st.draft.start_day != kDefaultStartDay ? "* " : "");
  ImGui::SameLine();
  if (!date_ok) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "not a date");
  else {
    const zs::CivilDate d = zs::civil_from_days(day);
    static const char* const days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    static const char* const months[] = {"", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    const int wd = static_cast<int>((((day % 7) + 7) % 7 + 3) % 7);  // 1970-01-01 was a Thursday (3, Monday first)
    ImGui::TextDisabled("%s %d %s %d; tick 0 is 08:00 (spec 53)", days[wd], d.day, months[d.month], d.year);
  }
  dirty = !(st.draft == *ed.settings);
  const std::string realism = realism_of(st, st.draft);
  ImGui::Text("Realism: ");
  ImGui::SameLine();
  ImGui::TextColored(realism == "realistic" ? ImVec4(0.55f, 0.9f, 0.55f, 1) : ImVec4(0.95f, 0.75f, 0.35f, 1), "%s", realism.c_str());
  ImGui::SameLine();
  ImGui::TextDisabled("(%d of %d settings touched)", touched_count(st, st.draft), static_cast<int>(st.bands.size()) + 1);
  // Apply or fork.
  if (!ed.locked) {
    ImGui::BeginDisabled(!dirty || !date_ok);
    if (ImGui::Button(("Apply to " + ed.world_name).c_str())) {
      *ed.settings = st.draft;
      ed.apply();
      st.edit_note = "applied; the planet is regenerated with these settings and Play uses them";
    }
    ImGui::EndDisabled();
  } else {
    ImGui::SetNextItemWidth(200);
    ImGui::InputText("##fork", st.fork_name, sizeof st.fork_name);
    ImGui::SameLine();
    ImGui::BeginDisabled(!date_ok);
    if (ImGui::Button("Fork with these settings")) {
      try {
        ed.fork(st.fork_name, st.draft);
        st.edit_note = std::string("forked into ") + st.fork_name;
        st.edit_error.clear();
      } catch (const std::exception& ex) {
        st.edit_error = ex.what();
      }
    }
    ImGui::EndDisabled();
  }
  ImGui::SameLine();
  if (ImGui::Button("Back to the world's")) {
    st.draft = *ed.settings;
    std::snprintf(st.date_text, sizeof st.date_text, "%s", zs::worlds::date_text(st.draft.start_day).c_str());
  }
  // Presets.
  const std::vector<std::string> presets = ed.presets ? ed.presets() : std::vector<std::string>{};
  ImGui::SetNextItemWidth(200);
  if (ImGui::BeginCombo("##preset", st.preset_pick >= 0 && st.preset_pick < static_cast<int>(presets.size()) ? presets[static_cast<std::size_t>(st.preset_pick)].c_str() : "a preset...")) {
    for (int i = 0; i < static_cast<int>(presets.size()); ++i)
      if (ImGui::Selectable(presets[static_cast<std::size_t>(i)].c_str(), i == st.preset_pick)) st.preset_pick = i;
    ImGui::EndCombo();
  }
  ImGui::SameLine();
  ImGui::BeginDisabled(st.preset_pick < 0 || st.preset_pick >= static_cast<int>(presets.size()));
  if (ImGui::Button("Load preset")) {
    try {
      st.draft = ed.load_preset(presets[static_cast<std::size_t>(st.preset_pick)]);
      std::snprintf(st.date_text, sizeof st.date_text, "%s", zs::worlds::date_text(st.draft.start_day).c_str());
      st.edit_error.clear();
    } catch (const std::exception& ex) {
      st.edit_error = ex.what();
    }
  }
  ImGui::EndDisabled();
  ImGui::SameLine();
  ImGui::SetNextItemWidth(160);
  ImGui::InputTextWithHint("##pname", "preset name", st.preset_name, sizeof st.preset_name);
  ImGui::SameLine();
  ImGui::BeginDisabled(!st.preset_name[0] || !date_ok);
  if (ImGui::Button("Save as preset")) {
    try {
      ed.save_preset(st.preset_name, st.draft);
      st.edit_note = std::string("saved preset ") + st.preset_name;
      st.edit_error.clear();
    } catch (const std::exception& ex) {
      st.edit_error = ex.what();
    }
  }
  ImGui::EndDisabled();
  if (!st.edit_error.empty()) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "%s", st.edit_error.c_str());
  else if (!st.edit_note.empty()) ImGui::TextDisabled("%s", st.edit_note.c_str());
  ImGui::Spacing();
  ImGui::Separator();
}

void draw_world_card(CardsState& st, const WorldCard& w, WorldEditor* editor) {
  if (editor && editor->settings) draw_world_settings(st, *editor);
  const gen::WorldSettings* s = editor && editor->settings ? editor->settings : nullptr;
  Values v;
  v["seed"] = std::to_string(w.seed);
  v["cells"] = std::to_string(w.cells) + " cells of about 5,000 km2, 70 km apart";
  v["land"] = fmt("%.1f million km2", w.land_km2 / 1e6);
  v["people"] = fmt("%.0f million", w.people / 1e6);
  v["countries"] = std::to_string(w.countries);
  v["towns"] = std::to_string(w.towns) + " of 10,000 people or more";
  v["history"] = std::to_string(w.start_polities) + " start polities, " + std::to_string(w.periods) + " periods, " +
                 std::to_string(w.events) + " events";
  v["start_date"] = s ? zs::worlds::date_text(s->start_day) + ", 08:00" + (s->start_day == kDefaultStartDay ? " (the default)" : " (set on this world)")
                      : "Monday 11 January 2027, 08:00 (the default)";
  v["sliders"] = s ? fmt("land %.2f, ", s->land_share) + fmt("%.1f C, ", s->global_temp_c) + fmt("relief %.2f, ", s->relief) +
                         fmt("moisture %.2f, ", s->moisture) + fmt("fragmentation %.2f", s->fragmentation) + " (" + realism_of(st, *s) + ")"
                   : "land 0.29, 14 C, relief 1, moisture 1, fragmentation 0.5 (the defaults)";
  if (w.fed > 0) v["fed"] = fmt("%.0f million people", w.fed / 1e6);
  draw_card(st, "world", "The world of seed " + std::to_string(w.seed), v);
}

// One line from the country's calendar file (spec 53): rest days, holidays in the start year, the school year.
const std::string& calendar_line(CardsState& st, const CountryCard& c) {
  const std::string id = c.profile.empty() ? "base_" + c.income : c.profile;
  auto it = st.calendars.find(id);
  if (it != st.calendars.end()) return it->second;
  std::string line;
  try {
    const CountryCalendar cal = load_country_calendar(st.data_dir + "/setup/calendar/" + id + ".toml");
    static const char* const days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    std::string rest;
    for (int d = 0; d < 7; ++d)
      if (cal.is_weekend(d)) rest += (rest.empty() ? "" : " and ") + std::string(days[d]);
    line = "rest on " + (rest.empty() ? std::string("no fixed day") : rest) + " (main " + days[cal.main_rest_day] + "); ";
    const auto dated = cal.dated.find(2027);
    line += (dated == cal.dated.end() ? std::string("holidays by rule") : std::to_string(dated->second.size()) + " public holidays in 2027") + "; ";
    line += "school from " + std::to_string(cal.school_first_day) + "/" + std::to_string(cal.school_first_month) + ", " +
            std::to_string(cal.instruction_days) + " instruction days, " + std::to_string(cal.school_week_days) + "-day week (" + id + ".toml)";
  } catch (const std::exception& ex) {
    line = std::string("no calendar file: ") + ex.what();
  }
  return st.calendars.emplace(id, line).first->second;
}

void draw_country_card(CardsState& st, const WorldCard& world, const CountryCard& c) {
  Values v;
  v["calendar"] = calendar_line(st, c);
  v["name"] = c.name;
  v["area"] = fmt("%.2f million km2", c.area_km2 / 1e6);
  v["people"] = fmt("%.1f million", c.people / 1e6);
  v["income"] = c.income;
  v["endowment"] = fmt("%.2f", c.endowment);
  v["coast"] = c.landlocked ? "landlocked" : "has a coast";
  v["capital"] = "held " + std::to_string(c.capital_held) + " of " + std::to_string(c.periods) + " periods";
  v["sectors"] = std::to_string(c.sectors);
  v["towns"] = std::to_string(c.towns.size()) + (c.towns.empty() ? "" : ", the largest of " + fmt("%.0f thousand", c.towns.front().people / 1e3));
  v["profile"] = c.profile.empty() ? "the base country of its income group (data/setup/nations/base_" + c.income + ".toml)"
                                   : c.profile + " (data/setup/nations/real/" + c.profile + ".toml), under a parody name";
  draw_card(st, "country", c.name, v);
  if (c.towns.empty()) return;
  ImGui::Spacing();
  ImGui::TextUnformatted("Towns (by rank; click one):");
  const int shown = std::min<int>(12, static_cast<int>(c.towns.size()));
  if (ImGui::BeginTable("towns", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
    ImGui::TableSetupColumn("Rank", 0, 0.6f);
    ImGui::TableSetupColumn("People", 0, 1);
    ImGui::TableSetupColumn("Lat", 0, 0.8f);
    ImGui::TableSetupColumn("Lon", 0, 0.8f);
    ImGui::TableHeadersRow();
    for (int i = 0; i < shown; ++i) {
      const TownCard& t = c.towns[static_cast<std::size_t>(i)];
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::PushID(i);
      if (ImGui::Selectable(std::to_string(t.rank).c_str(), st.town == i, ImGuiSelectableFlags_SpanAllColumns)) st.town = i;
      ImGui::PopID();
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%.0f thousand", t.people / 1e3);
      ImGui::TableSetColumnIndex(2);
      ImGui::Text("%.1f", t.lat);
      ImGui::TableSetColumnIndex(3);
      ImGui::Text("%.1f", t.lon);
    }
    ImGui::EndTable();
  }
  if (static_cast<int>(c.towns.size()) > shown) ImGui::TextDisabled("and %d more", static_cast<int>(c.towns.size()) - shown);
  (void)world;
}

void draw_town_card(CardsState& st, const WorldCard& world, const CountryCard& c, const TownCard& t) {
  ensure_town(st, world, c, t);
  Values v;
  v["people"] = fmt("%.0f thousand", t.people / 1e3);
  v["rank"] = std::to_string(t.rank) + " in " + c.name;
  v["cell"] = std::to_string(t.cell) + fmt(" (lat %.1f", t.lat) + fmt(", lon %.1f)", t.lon);
  if (st.gen_town) {
    const gen::Town& g = *st.gen_town;
    v["name"] = g.name + " (culture " + g.culture + ", seed " + std::to_string(st.gen_seed) + ")";
    std::string streets;
    for (const auto& s : g.streets) streets += (streets.empty() ? "" : "; ") + s;
    v["streets"] = streets;
    v["businesses"] = std::to_string(g.businesses.size());
    v["households"] = std::to_string(g.households.size()) + " households, " + std::to_string(g.pops.size()) + " people drawn from " +
                      (c.profile.empty() ? "base_" + c.income : c.profile) + "'s tables";
    v["dwellings"] = std::to_string(st.dwellings.size()) + " dwellings, " + st.gen_income + " income";
  }
  draw_card(st, "town", "Town " + std::to_string(t.rank) + " of " + c.name, v);
  if (!st.gen_error.empty()) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "%s", st.gen_error.c_str());
  if (!st.gen_town) return;
  ImGui::TextWrapped("The people below are a stand-in: households, schooling, work and occupations follow this country's tables "
                     "(spec 47), but names, birthplaces and the town's name still come from the block's culture pack, and the "
                     "planet town itself has only its size and rank. Towns named and peopled from their country's culture is "
                     "status step 3, the next generator work.");
  ImGui::Spacing();
  ImGui::TextUnformatted("Businesses:");
  const gen::Town& g = *st.gen_town;
  for (std::size_t i = 0; i < g.businesses.size() && i < 10; ++i) {
    ImGui::SameLine();
    ImGui::TextDisabled("%s%s", g.businesses[i].name.c_str(), i + 1 < std::min<std::size_t>(10, g.businesses.size()) ? "," : "");
  }
  ImGui::Spacing();
  ImGui::TextUnformatted("People (click one):");
  ImGui::BeginChild("roster", ImVec2(0, 220), ImGuiChildFlags_Borders);
  for (std::size_t i = 0; i < g.pops.size(); ++i) {
    const gen::Pop& p = g.pops[i];
    const std::string label = p.full_name + ", " + std::to_string(p.age) + ", " + p.occupation;
    ImGui::PushID(static_cast<int>(i));
    if (ImGui::Selectable(label.c_str(), st.pop == static_cast<int>(i))) {
      st.pop = static_cast<int>(i);
      st.house = p.household;
      st.house_map.clear();
    }
    ImGui::PopID();
  }
  ImGui::EndChild();
}

void draw_pop_card(CardsState& st) {
  if (!st.gen_town || st.pop < 0 || st.pop >= static_cast<int>(st.gen_town->pops.size())) return;
  const gen::Town& g = *st.gen_town;
  const gen::Pop& p = g.pops[static_cast<std::size_t>(st.pop)];
  const gen::Household& hh = g.households[static_cast<std::size_t>(p.household)];
  Values v;
  v["name"] = p.full_name;
  v["age_sex"] = std::to_string(p.age) + ", " + (p.sex == 'f' ? "female" : "male");
  v["nationality"] = p.nationality + "; " + p.culture_label + "; born in " + p.birthplace + ", " + p.birth_country +
                     (p.moved_at >= 0 ? "; moved here at " + std::to_string(p.moved_at) : "") + "; " +
                     std::to_string(p.years_in_town) + " years in town";
  v["household"] = p.role + " of household " + std::to_string(p.household + 1) + " (" + std::to_string(hh.members.size()) +
                   " people) at " + hh.address;
  v["education"] = p.education + (p.attainment >= 0 ? " (level " + std::to_string(p.attainment) + ")" : "");
  v["occupation"] = p.occupation + (p.workplace.empty() ? "" : " at " + p.workplace) +
                    (p.years_in_job > 0 ? ", " + std::to_string(p.years_in_job) + " years" : "") +
                    (p.past_job.empty() ? "" : "; before that " + p.past_job);
  std::string skills;
  for (const auto& s : p.skills) skills += (skills.empty() ? "" : ", ") + s;
  v["skills"] = skills.empty() ? "none" : skills;
  v["appearance"] = "skin " + std::to_string(p.skin_tone) + "/10, " + p.hair + " hair, " + p.eyes + " eyes";
  v["body_seed"] = std::to_string(p.body_seed);
  std::string life;
  for (const auto& t : p.lifepath)
    life += std::to_string(t.from) + "-" + std::to_string(t.to) + " " + t.what + (t.where.empty() ? "" : " " + t.where) + "\n";
  if (!life.empty()) life.pop_back();
  v["lifepath"] = life.empty() ? "none" : life;
  draw_card(st, "pop", p.full_name, v);
  ImGui::Spacing();
  ImGui::TextDisabled("As the generator prints it (zs gen town):");
  ImGui::BeginChild("dossier", ImVec2(0, 120), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
  ImGui::TextUnformatted(gen::dossier(g, p, st.catalog->life.year).c_str());
  ImGui::EndChild();
}

void draw_house_card(CardsState& st) {
  if (!st.gen_town || st.house < 0) return;
  const gen::Town& g = *st.gen_town;
  const gen::Dwelling* d = nullptr;
  for (const auto& x : st.dwellings)
    if (x.household == st.house) d = &x;
  if (!d) return;
  const gen::Household& hh = g.households[static_cast<std::size_t>(st.house)];
  Values v;
  v["address"] = d->address;
  std::string who;
  for (std::size_t m : hh.members) who += (who.empty() ? "" : ", ") + g.pops[m].given + " (" + std::to_string(g.pops[m].age) + ")";
  v["members"] = who;
  v["size"] = std::to_string(d->floor_area()) + " m2 (" + std::to_string(d->width) + " x " + std::to_string(d->depth) + "), target " +
              fmt("%.0f m2", d->target_area_m2) + ", " + d->income + " income";
  std::string rooms;
  for (const auto& r : d->rooms)
    rooms += (rooms.empty() ? "" : "; ") + r.type + " " + std::to_string(r.w) + "x" + std::to_string(r.h) +
             (r.occupants ? " (" + std::to_string(r.occupants) + " sleep)" : "");
  v["rooms"] = rooms;
  v["food"] = std::to_string((d->pantry_kj + d->fridge_kj) / 1000) + " MJ (pantry " + std::to_string(d->pantry_kj / 1000) + ", fridge " +
              std::to_string(d->fridge_kj / 1000) + ")";
  v["services"] = std::string(d->fridge ? "fridge" : "no fridge") + ", " + (d->bathroom ? "bathroom" : "no bathroom");
  if (st.house_map.empty()) {
    try {
      st.house_map = gen::house_toml(g, *d, *st.rules, st.gen_seed);
    } catch (const std::exception& ex) {
      st.house_map = std::string("could not write the map: ") + ex.what();
    }
  }
  v["map"] = std::to_string(std::count(st.house_map.begin(), st.house_map.end(), '\n')) + " lines of site map (below)";
  draw_card(st, "house", "The house at " + d->address, v);
  ImGui::Spacing();
  ImGui::TextDisabled("The site map (zs gen house --household %d):", st.house + 1);
  ImGui::BeginChild("housemap", ImVec2(0, 200), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
  ImGui::TextUnformatted(st.house_map.c_str());
  ImGui::EndChild();
}

}  // namespace

void draw_cards(CardsState& st, const WorldCard& world, const std::vector<CountryCard>& countries, int select_country,
                WorldEditor* editor) {
  if (!st.error.empty()) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "cards.toml: %s", st.error.c_str());
  if (select_country >= 0 && select_country != st.country) {
    st.country = select_country;
    st.town = -1;
    st.pop = st.house = -1;
  }
  if (st.autopick && st.country >= 0 && st.country < static_cast<int>(countries.size())) {
    if (st.town < 0 && !countries[static_cast<std::size_t>(st.country)].towns.empty()) st.town = 0;
    if (st.pop < 0 && st.gen_town && !st.gen_town->pops.empty()) {
      st.pop = 0;
      st.house = st.gen_town->pops[0].household;
    }
  }
  ImGui::TextWrapped("Each card is a diff between the design and the build: what the thing needs, what it has today, and the spec "
                     "that produces it. Rows marked lacking are designed and not produced; the count per card is honest because "
                     "nobody maintains it. Pick a country (here or on the Tour's planet), then a town, a pop, their house.");
  ImGui::Separator();
  ImGui::BeginChild("pick", ImVec2(300, 0), ImGuiChildFlags_Borders);
  if (ImGui::Selectable("The world", st.country == -1)) st.country = -1, st.town = -1, st.pop = st.house = -1;
  ImGui::TextDisabled("Countries by people:");
  std::vector<int> order(countries.size());
  for (std::size_t i = 0; i < order.size(); ++i) order[i] = static_cast<int>(i);
  std::sort(order.begin(), order.end(), [&](int a, int b) { return countries[static_cast<std::size_t>(a)].people > countries[static_cast<std::size_t>(b)].people; });
  for (int k : order) {
    const CountryCard& c = countries[static_cast<std::size_t>(k)];
    ImGui::PushID(k);
    const std::string label = c.name + "  " + fmt("%.0f M", c.people / 1e6);
    if (ImGui::Selectable(label.c_str(), st.country == k)) {
      st.country = k;
      st.town = -1;
      st.pop = st.house = -1;
    }
    ImGui::PopID();
  }
  ImGui::EndChild();
  ImGui::SameLine();
  if (st.autopick) ImGui::SetNextWindowScroll(ImVec2(-1.0f, 1.0e8f));  // a check shot shows the end of the chain
  ImGui::BeginChild("cards", ImVec2(0, 0), ImGuiChildFlags_Borders);
  if (st.country < 0 || st.country >= static_cast<int>(countries.size())) {
    draw_world_card(st, world, editor);
  } else {
    const CountryCard& c = countries[static_cast<std::size_t>(st.country)];
    draw_country_card(st, world, c);
    if (st.town >= 0 && st.town < static_cast<int>(c.towns.size())) {
      ImGui::Spacing();
      ImGui::Separator();
      draw_town_card(st, world, c, c.towns[static_cast<std::size_t>(st.town)]);
      if (st.pop >= 0) {
        ImGui::Spacing();
        ImGui::Separator();
        draw_pop_card(st);
        ImGui::Spacing();
        ImGui::Separator();
        draw_house_card(st);
      }
    }
  }
  ImGui::EndChild();
}

}  // namespace zs::atlas
