#include "atlas_countries.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

#include <SDL3/SDL.h>

#include <filesystem>
#include <map>
#include <mutex>

#include "country_packs.h"
#include "flags.h"
#include "imgui.h"
#include "toml.hpp"
#include "zs/nations.h"

namespace zs::atlas {

namespace {

constexpr const char* kIncomes[] = {"high", "upper_middle", "lower_middle", "low"};
constexpr const char* kIncomeLabels[] = {"high income", "upper-middle income", "lower-middle income", "low income"};

void tip(const char* text) {
  if (!ImGui::IsItemHovered()) return;
  ImGui::BeginTooltip();
  ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30);
  ImGui::TextUnformatted(text);
  ImGui::PopTextWrapPos();
  ImGui::EndTooltip();
}

std::vector<double> numbers(const toml::node_view<toml::node>& v) {
  std::vector<double> out;
  if (const toml::array* a = v.as_array())
    for (const auto& x : *a) out.push_back(x.value_or(0.0));
  return out;
}

void set_numbers(toml::table& t, const char* table, const char* key, const std::vector<double>& xs) {
  toml::array a;
  for (double x : xs) a.push_back(std::round(x * 1e6) / 1e6);
  if (toml::table* s = t[table].as_table()) s->insert_or_assign(key, std::move(a));
}

// The median age of a pyramid of 5-year bands (the last open).
double median_age(const std::vector<double>& shares) {
  double sum = 0;
  for (double s : shares) sum += s;
  double acc = 0;
  for (std::size_t i = 0; i < shares.size(); ++i) {
    if (acc + shares[i] >= sum / 2) return 5.0 * i + 5.0 * (sum / 2 - acc) / std::max(1e-12, shares[i]);
    acc += shares[i];
  }
  return 5.0 * shares.size();
}

// The pyramid tilted by exp(k x band) and renormalised, k found by bisection so its median is `target`.
std::vector<double> tilt_to_median(const std::vector<double>& base, double target) {
  auto tilted = [&](double k) {
    std::vector<double> t(base.size());
    double sum = 0;
    for (std::size_t i = 0; i < base.size(); ++i) sum += t[i] = base[i] * std::exp(k * static_cast<double>(i));
    for (double& x : t) x /= sum;
    return t;
  };
  double lo = -2, hi = 2;
  for (int it = 0; it < 60; ++it) {
    const double mid = (lo + hi) / 2;
    (median_age(tilted(mid)) < target ? lo : hi) = mid;
  }
  return tilted((lo + hi) / 2);
}

// Household sizes in four classes (1, 2-3, 4-5, 6+) with means 1, 2.5, 4.5 and 7: tilted the same way to a mean size.
constexpr double kClassSize[] = {1.0, 2.5, 4.5, 7.0};
std::vector<double> tilt_to_mean(const std::vector<double>& base, double target) {
  auto tilted = [&](double k) {
    std::vector<double> t(base.size());
    double sum = 0;
    for (std::size_t i = 0; i < base.size(); ++i) sum += t[i] = std::max(1e-6, base[i]) * std::exp(k * static_cast<double>(i));
    for (double& x : t) x /= sum;
    return t;
  };
  auto mean = [&](const std::vector<double>& t) {
    double m = 0;
    for (std::size_t i = 0; i < t.size() && i < 4; ++i) m += t[i] * kClassSize[i];
    return m;
  };
  double lo = -4, hi = 4;
  for (int it = 0; it < 60; ++it) {
    const double mid = (lo + hi) / 2;
    (mean(tilted(mid)) < target ? lo : hi) = mid;
  }
  return tilted((lo + hi) / 2);
}

int income_index(const std::string& s) {
  for (int i = 0; i < 4; ++i)
    if (s == kIncomes[i]) return i;
  return 1;
}

ImVec4 parse_color(const std::string& hex) {
  unsigned v = 0x808080;
  if (hex.size() == 7 && hex[0] == '#') std::sscanf(hex.c_str() + 1, "%x", &v);
  return {((v >> 16) & 255) / 255.0f, ((v >> 8) & 255) / 255.0f, (v & 255) / 255.0f, 1};
}

std::string color_text(const float c[3]) {
  char buf[16];
  std::snprintf(buf, sizeof buf, "#%02x%02x%02x", static_cast<int>(c[0] * 255 + 0.5f), static_cast<int>(c[1] * 255 + 0.5f),
                static_cast<int>(c[2] * 255 + 0.5f));
  return buf;
}

}  // namespace

struct CountriesState {
  std::string data_dir;
  SDL_Renderer* renderer = nullptr;
  std::map<std::string, SDL_Texture*> textures;  // flags drawn, by their pattern and colours or their image file
  std::mutex picked_mutex;
  std::string picked;  // the file the open dialog returned (it may answer from another thread)
  std::map<std::string, flags::Flag> profile_flags;  // read once per profile for other pages
  std::vector<packs::Default> defaults;
  std::vector<std::string> user_packs;
  int pack = -1;  // -1: earth; else an index into user_packs
  packs::Pack open;
  std::vector<std::string> customs, custom_names;
  // The selection: a default country (by ISO) or a custom one (by id).
  std::string sel_iso, sel_custom;
  toml::table draft;  // the custom country being edited
  bool dirty = false, advanced = false;
  char new_pack[64] = "";
  char new_name[64] = "";
  char filter[64] = "";
  int template_pick = 0;  // 0..3 the bases, then the defaults
  std::string error, note;
  // The default country's card, read once per selection.
  std::string card_iso;
  toml::table card;
};

std::shared_ptr<CountriesState> make_countries(const std::string& data_dir, SDL_Renderer* renderer) {
  auto st = std::make_shared<CountriesState>();
  st->data_dir = data_dir;
  st->renderer = renderer;
  st->defaults = packs::defaults(data_dir);
  st->user_packs = packs::list();
  return st;
}

namespace {

// A flag as a texture: drawn from its pattern, or loaded from its image (PNG or BMP) in `dir`; cached.
SDL_Texture* flag_texture(CountriesState& st, const flags::Flag& f, const std::filesystem::path& dir) {
  if (!st.renderer) return nullptr;
  std::string key;
  if (!f.image.empty()) {
    std::error_code ec;
    const auto when = std::filesystem::last_write_time(dir / f.image, ec);
    key = (dir / f.image).string() + "@" + std::to_string(ec ? 0 : when.time_since_epoch().count());
  } else {
    key = flags::pattern_id(f.pattern);
    for (std::uint32_t c : f.colors) key += flags::hex(c);
  }
  if (auto it = st.textures.find(key); it != st.textures.end()) return it->second;
  SDL_Texture* tex = nullptr;
  if (!f.image.empty()) {
    if (SDL_Surface* s = SDL_LoadSurface((dir / f.image).string().c_str())) {
      tex = SDL_CreateTextureFromSurface(st.renderer, s);
      SDL_DestroySurface(s);
    }
  } else {
    const int w = 96, h = 64;
    const std::vector<unsigned char> px = flags::draw(f, w, h);
    tex = SDL_CreateTexture(st.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w, h);
    if (tex) SDL_UpdateTexture(tex, nullptr, px.data(), w * 4);
  }
  if (tex) SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
  st.textures[key] = tex;
  return tex;
}

void show_flag(CountriesState& st, const flags::Flag& f, const std::filesystem::path& dir, float height) {
  if (SDL_Texture* t = flag_texture(st, f, dir)) ImGui::Image(reinterpret_cast<ImTextureID>(t), ImVec2(height * 1.5f, height));
  else ImGui::Dummy(ImVec2(height * 1.5f, height));
}

ImVec4 rgb4(std::uint32_t c) { return {((c >> 16) & 255) / 255.0f, ((c >> 8) & 255) / 255.0f, (c & 255) / 255.0f, 1}; }

// A colour chooser: the basic swatches, then "More..." for the full picker (wheel, RGB, HSV, hex). True when changed.
bool color_choice(const char* id, std::uint32_t& c) {
  bool changed = false;
  ImGui::PushID(id);
  if (ImGui::ColorButton("##now", rgb4(c), ImGuiColorEditFlags_NoTooltip, ImVec2(28, 20))) ImGui::OpenPopup("pick");
  if (ImGui::BeginPopup("pick")) {
    const auto& basic = flags::basic_colors();
    ImGui::TextDisabled("Basic colours");
    for (std::size_t i = 0; i < basic.size(); ++i) {
      if (i % 8) ImGui::SameLine();
      ImGui::PushID(static_cast<int>(i));
      if (ImGui::ColorButton("##b", rgb4(basic[i]), 0, ImVec2(24, 24))) c = basic[i], changed = true;
      ImGui::PopID();
    }
    ImGui::Separator();
    ImGui::TextDisabled("Any colour");
    float f[3] = {((c >> 16) & 255) / 255.0f, ((c >> 8) & 255) / 255.0f, (c & 255) / 255.0f};
    if (ImGui::ColorPicker3("##full", f, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHex)) {
      c = (static_cast<std::uint32_t>(f[0] * 255 + 0.5f) << 16) | (static_cast<std::uint32_t>(f[1] * 255 + 0.5f) << 8) |
          static_cast<std::uint32_t>(f[2] * 255 + 0.5f);
      changed = true;
    }
    ImGui::EndPopup();
  }
  ImGui::PopID();
  return changed;
}

void SDLCALL on_flag_file(void* user, const char* const* files, int) {
  auto* st = static_cast<CountriesState*>(user);
  if (!files || !files[0]) return;
  std::lock_guard<std::mutex> l(st->picked_mutex);
  st->picked = files[0];
}

}  // namespace

void draw_country_flag(CountriesState& st, const std::string& profile, float height) {
  auto it = st.profile_flags.find(profile);
  if (it == st.profile_flags.end()) {
    flags::Flag f = flags::default_flag(profile);
    if (zs::is_custom_profile(profile)) {
      try {
        f = flags::read_flag(toml::parse_file(zs::nation_file(st.data_dir, profile, "")), profile);
      } catch (const std::exception&) {
      }
    }
    it = st.profile_flags.emplace(profile, f).first;
  }
  show_flag(st, it->second, zs::custom_nations_dir(), height);
}

namespace {

void refresh_customs(CountriesState& st) {
  st.customs = packs::custom_ids(st.open);
  st.custom_names.clear();
  for (const std::string& id : st.customs) {
    std::string n = id;
    try {
      n = packs::load_country(st.open, id)["name"].value_or(id);
    } catch (const std::exception&) {
    }
    st.custom_names.push_back(n);
  }
}

void open_pack(CountriesState& st, int index) {
  st.pack = index;
  st.sel_iso.clear();
  st.sel_custom.clear();
  st.dirty = false;
  st.error.clear();
  st.note.clear();
  st.customs.clear();
  if (index < 0) return;
  try {
    st.open = packs::load(st.user_packs[static_cast<std::size_t>(index)]);
    refresh_customs(st);
  } catch (const std::exception& ex) {
    st.error = ex.what();
    st.pack = -1;
  }
}

bool includes(const packs::Pack& p, const std::string& iso) {
  return std::find(p.include.begin(), p.include.end(), "all") != p.include.end() ||
         std::find(p.include.begin(), p.include.end(), iso) != p.include.end();
}

const packs::Default* find_default(const CountriesState& st, const std::string& iso) {
  for (const auto& d : st.defaults)
    if (d.iso == iso) return &d;
  return nullptr;
}

// The packs column.
void draw_packs(CountriesState& st) {
  ImGui::TextDisabled("Packs");
  if (ImGui::Selectable("Earth (default, read-only)", st.pack < 0)) open_pack(st, -1);
  tip("The 183 countries with real data under parody names. They cannot be edited: they are the guide and the sample. "
      "Copy one into your own pack to make a country from it.");
  for (int i = 0; i < static_cast<int>(st.user_packs.size()); ++i)
    if (ImGui::Selectable(st.user_packs[static_cast<std::size_t>(i)].c_str(), st.pack == i)) open_pack(st, i);
  ImGui::Spacing();
  ImGui::SetNextItemWidth(-1);
  ImGui::InputTextWithHint("##newpack", "a new pack's name", st.new_pack, sizeof st.new_pack);
  ImGui::BeginDisabled(!st.new_pack[0]);
  if (ImGui::Button("New pack")) {
    try {
      packs::create(st.new_pack, st.new_pack);
      st.user_packs = packs::list();
      for (int i = 0; i < static_cast<int>(st.user_packs.size()); ++i)
        if (st.user_packs[static_cast<std::size_t>(i)] == st.new_pack) open_pack(st, i);
      st.new_pack[0] = 0;
    } catch (const std::exception& ex) {
      st.error = ex.what();
    }
  }
  ImGui::EndDisabled();
  tip("An empty pack of your own, beside the program (packs/countries/). Add default countries to it, or make new ones.");
  if (st.pack >= 0) {
    if (ImGui::Button("Delete this pack")) ImGui::OpenPopup("Delete pack");
    if (ImGui::BeginPopupModal("Delete pack", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Send the pack \"%s\" to the recycle bin? Worlds made with it keep their own copy.", st.open.id.c_str());
      if (ImGui::Button("Delete")) {
        try {
          packs::remove(st.open.id);
        } catch (const std::exception& ex) {
          st.error = ex.what();
        }
        st.user_packs = packs::list();
        open_pack(st, -1);
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }
}

// The countries column.
void draw_list(CountriesState& st) {
  ImGui::SetNextItemWidth(-1);
  ImGui::InputTextWithHint("##filter", "filter by name", st.filter, sizeof st.filter);
  auto shown = [&](const std::string& name) {
    if (!st.filter[0]) return true;
    std::string a = name, b = st.filter;
    std::transform(a.begin(), a.end(), a.begin(), ::tolower);
    std::transform(b.begin(), b.end(), b.begin(), ::tolower);
    return a.find(b) != std::string::npos;
  };
  if (st.pack >= 0) {
    ImGui::TextDisabled("Your countries (%zu)", st.customs.size());
    for (std::size_t k = 0; k < st.customs.size(); ++k) {
      const std::string& id = st.customs[k];
      if (!shown(st.custom_names[k])) continue;
      if (ImGui::Selectable((st.custom_names[k] + "##c" + id).c_str(), st.sel_custom == id)) {
        st.sel_custom = id;
        st.sel_iso.clear();
        st.draft = packs::load_country(st.open, id);
        st.dirty = false;
      }
    }
    if (ImGui::Button("New country")) ImGui::OpenPopup("New country");
    tip("A country of your own, made from a template: its tables (ages, schooling, jobs, households) are copied, then you edit them.");
    if (ImGui::BeginPopupModal("New country", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::InputText("Name", st.new_name, sizeof st.new_name);
      std::vector<std::string> labels = {"Base: high income", "Base: upper-middle income", "Base: lower-middle income", "Base: low income"};
      for (const auto& d : st.defaults) labels.push_back(d.name + " (" + d.iso + ")");
      ImGui::SetNextItemWidth(360);
      if (ImGui::BeginCombo("Template", labels[static_cast<std::size_t>(std::clamp(st.template_pick, 0, static_cast<int>(labels.size()) - 1))].c_str())) {
        for (int i = 0; i < static_cast<int>(labels.size()); ++i)
          if (ImGui::Selectable(labels[static_cast<std::size_t>(i)].c_str(), i == st.template_pick)) st.template_pick = i;
        ImGui::EndCombo();
      }
      tip("The country whose tables the new one starts from. Its calendar, police and wages also follow the template.");
      ImGui::BeginDisabled(!st.new_name[0]);
      if (ImGui::Button("Create")) {
        try {
          const std::string tpl = st.template_pick < 4 ? std::string("base_") + kIncomes[st.template_pick]
                                                       : st.defaults[static_cast<std::size_t>(st.template_pick - 4)].iso;
          const std::string id = packs::new_country(st.open, st.data_dir, tpl, st.new_name);
          refresh_customs(st);
          st.sel_custom = id;
          st.sel_iso.clear();
          st.draft = packs::load_country(st.open, id);
          st.new_name[0] = 0;
          ImGui::CloseCurrentPopup();
        } catch (const std::exception& ex) {
          st.error = ex.what();
        }
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
    ImGui::Separator();
    const bool all = std::find(st.open.include.begin(), st.open.include.end(), "all") != st.open.include.end();
    bool every = all;
    if (ImGui::Checkbox("Include every default country", &every)) {
      if (every) st.open.include = {"all"};
      else st.open.include.clear();
      packs::save(st.open);
    }
    tip("A pack can use the default countries as they are (read-only), beside its own. Tick one in the list below to include it.");
  }
  ImGui::TextDisabled(st.pack < 0 ? "Default countries (%zu)" : "Default countries: tick to include", st.defaults.size());
  for (const auto& d : st.defaults) {
    if (!shown(d.name)) continue;
    ImGui::PushID(d.iso.c_str());
    if (st.pack >= 0) {
      bool in = includes(st.open, d.iso);
      const bool all = std::find(st.open.include.begin(), st.open.include.end(), "all") != st.open.include.end();
      ImGui::BeginDisabled(all);
      if (ImGui::Checkbox("##in", &in)) {
        if (in) st.open.include.push_back(d.iso);
        else st.open.include.erase(std::remove(st.open.include.begin(), st.open.include.end(), d.iso), st.open.include.end());
        packs::save(st.open);
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
    }
    char label[160];
    std::snprintf(label, sizeof label, d.population < 1e7 ? "%s  %.1f M" : "%s  %.0f M", d.name.c_str(), d.population / 1e6);
    if (ImGui::Selectable(label, st.sel_iso == d.iso)) {
      st.sel_iso = d.iso;
      st.sel_custom.clear();
    }
    ImGui::PopID();
  }
}

// A default country's card: read-only, with a copy into a pack.
void draw_default(CountriesState& st) {
  const packs::Default* d = find_default(st, st.sel_iso);
  if (!d) return;
  if (st.card_iso != d->iso) {
    st.card_iso = d->iso;
    try {
      st.card = toml::parse_file(st.data_dir + "/setup/nations/real/" + d->iso + ".toml");
    } catch (const std::exception& ex) {
      st.card = {};
      st.error = ex.what();
    }
  }
  ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.3f);
  ImGui::TextColored({1, 0.95f, 0.8f, 1}, "%s", d->name.c_str());
  ImGui::PopFont();
  ImGui::TextDisabled("A default country (%s): real data under a parody name. Read-only: copy it into your pack to edit.", d->iso.c_str());
  show_flag(st, flags::default_flag(d->iso), {}, 48);
  ImGui::SameLine();
  ImGui::TextDisabled("its parody flag (not the real one)");
  const std::vector<double> ages = numbers(st.card["age_pyramid"]["shares"]);
  ImGui::Text("Income group: %s", kIncomeLabels[income_index(d->income)]);
  ImGui::Text("People: %.1f million", d->population / 1e6);
  ImGui::Text("Urban share: %.0f %%", 100 * st.card["settlement"]["urban_share"].value_or(0.0));
  ImGui::Text("Mean household: %.2f people", st.card["households"]["mean_size"].value_or(0.0));
  if (!ages.empty()) ImGui::Text("Median age: %.1f years", median_age(ages));
  ImGui::Spacing();
  if (st.user_packs.empty()) {
    ImGui::TextDisabled("Make a pack (left) to copy this country into it.");
    return;
  }
  if (st.pack >= 0) {
    if (ImGui::Button(("Copy into " + st.open.id + " as a new country").c_str())) {
      try {
        const std::string id = packs::new_country(st.open, st.data_dir, d->iso, d->name + " 2");
        refresh_customs(st);
        st.sel_custom = id;
        st.sel_iso.clear();
        st.draft = packs::load_country(st.open, id);
      } catch (const std::exception& ex) {
        st.error = ex.what();
      }
    }
    tip("A custom country of your pack, starting as a copy of this one, that you can edit");
  } else {
    ImGui::TextDisabled("Open one of your packs (left) to copy this country into it.");
  }
}

// The advanced editor: every number of every table.
void draw_tables(CountriesState& st) {
  for (auto&& [name, node] : st.draft) {
    toml::table* t = node.as_table();
    if (!t || name.str() == "country") continue;
    if (!ImGui::TreeNode(std::string(name.str()).c_str())) continue;
    for (auto&& [key, val] : *t) {
      const std::string k(key.str());
      if (k == "source" || k == "year") continue;
      ImGui::PushID(k.c_str());
      if (toml::array* a = val.as_array()) {
        std::vector<double> xs;
        for (auto& x : *a) xs.push_back(x.value_or(0.0));
        bool changed = false;
        ImGui::Text("%s", k.c_str());
        for (std::size_t i = 0; i < xs.size(); ++i) {
          if (i % 5) ImGui::SameLine();
          ImGui::PushID(static_cast<int>(i));
          ImGui::SetNextItemWidth(80);
          changed |= ImGui::InputDouble("##x", &xs[i], 0, 0, "%.4f");
          ImGui::PopID();
        }
        double sum = 0;
        for (double x : xs) sum += x;
        const bool shares = k == "shares" || k == "sizes" || k.rfind("age_", 0) == 0;
        if (shares) {
          ImGui::SameLine();
          ImGui::TextDisabled("sum %.4f", sum);
          ImGui::SameLine();
          if (ImGui::SmallButton("normalise") && sum > 0) {
            for (double& x : xs) x /= sum;
            changed = true;
          }
        }
        if (changed) {
          a->clear();
          for (double x : xs) a->push_back(x);
          st.dirty = true;
        }
      } else if (val.is_floating_point() || val.is_integer()) {
        double x = val.value_or(0.0);
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputDouble(k.c_str(), &x, 0, 0, "%.4f")) {
          if (val.is_integer()) t->insert_or_assign(k, static_cast<std::int64_t>(std::llround(x)));
          else t->insert_or_assign(k, x);
          st.dirty = true;
        }
      }
      ImGui::PopID();
    }
    ImGui::TreePop();
  }
}

// A custom country's editor: simple by default, every table in advanced.
void draw_custom(CountriesState& st) {
  toml::table& t = st.draft;
  char name[96];
  std::snprintf(name, sizeof name, "%s", t["name"].value_or(st.sel_custom).c_str());
  ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.3f);
  ImGui::TextColored(parse_color(t["color"].value_or(std::string("#808080"))), "%s", name);
  ImGui::PopFont();
  ImGui::TextDisabled("Your country, in the pack %s; made from %s.", st.open.id.c_str(), t["template"].value_or(std::string("?")).c_str());
  ImGui::SetNextItemWidth(300);
  if (ImGui::InputText("Name", name, sizeof name)) t.insert_or_assign("name", std::string(name)), st.dirty = true;
  tip("The country's name on maps and cards");
  {
    std::uint32_t mc = flags::parse_hex(t["color"].value_or(std::string("#808080")), 0x808080);
    if (color_choice("map", mc)) t.insert_or_assign("color", flags::hex(mc)), st.dirty = true;
    tip("Its colour on the political map: a basic colour, or any colour with the full picker");
    ImGui::SameLine();
    ImGui::TextUnformatted("Map colour");
  }
  // The flag: a pattern in three colours, or an image of its own (PNG or BMP), copied beside the country.
  {
    flags::Flag fl = flags::read_flag(t, st.sel_custom);
    bool changed = false;
    show_flag(st, fl, st.open.dir, 64);
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::BeginDisabled(!fl.image.empty());
    int pat = static_cast<int>(fl.pattern);
    ImGui::SetNextItemWidth(220);
    if (ImGui::BeginCombo("Flag", flags::pattern_label(fl.pattern))) {
      for (int i = 0; i < flags::kPatterns; ++i) {
        flags::Flag sample = fl;
        sample.pattern = static_cast<flags::Pattern>(i);
        ImGui::PushID(i);
        show_flag(st, sample, {}, 18);
        ImGui::SameLine();
        if (ImGui::Selectable(flags::pattern_label(sample.pattern), i == pat)) fl.pattern = sample.pattern, changed = true;
        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
    tip("The flag's design; its three colours below");
    for (int k = 0; k < 3; ++k) {
      if (k) ImGui::SameLine();
      changed |= color_choice(k == 0 ? "f0" : k == 1 ? "f1" : "f2", fl.colors[static_cast<std::size_t>(k)]);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("colours");
    ImGui::EndDisabled();
    if (ImGui::Button("Load an image...")) {
      static const SDL_DialogFileFilter filters[] = {{"Flag image (PNG, BMP)", "png;bmp"}};
      SDL_ShowOpenFileDialog(on_flag_file, &st, nullptr, filters, 1, nullptr, false);
    }
    tip("Use a picture of your own as the flag (PNG or BMP, about 3:2); it is copied into the pack");
    if (!fl.image.empty()) {
      ImGui::SameLine();
      if (ImGui::Button("Use the pattern")) fl.image.clear(), changed = true;
      tip("Back to the drawn flag; the image stays in the pack's folder");
    }
    ImGui::EndGroup();
    std::string picked;
    {
      std::lock_guard<std::mutex> l(st.picked_mutex);
      picked.swap(st.picked);
    }
    if (!picked.empty()) {
      try {
        const std::filesystem::path src(picked);
        std::string ext = src.extension().string();
        for (char& ch : ext) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        const std::string name = st.sel_custom + "_flag" + ext;
        std::filesystem::copy_file(src, st.open.dir / name, std::filesystem::copy_options::overwrite_existing);
        fl.image = name;
        changed = true;
      } catch (const std::exception& ex) {
        st.error = std::string("could not copy the flag image: ") + ex.what();
      }
    }
    if (changed) flags::write_flag(t, fl), st.dirty = true;
  }
  toml::table* country = t["country"].as_table();
  if (country) {
    int inc = income_index((*country)["income"].value_or(std::string("upper_middle")));
    ImGui::SetNextItemWidth(300);
    if (ImGui::Combo("Income group", &inc, kIncomeLabels, 4)) country->insert_or_assign("income", std::string(kIncomes[inc])), st.dirty = true;
    tip("Sets its wealth: wages, diet, urban share by default and which lands it tends to take. The world fills each "
        "group to its real share of people, so a rich country among many rich ones may be placed in a nearby group.");
    double people = (*country)["population"].value_or(1e7) / 1e6;
    ImGui::SetNextItemWidth(300);
    const double lo = 0.1, hi = 2000;
    if (ImGui::SliderScalar("People (millions)", ImGuiDataType_Double, &people, &lo, &hi, "%.1f", ImGuiSliderFlags_Logarithmic))
      country->insert_or_assign("population", std::round(people * 1e6)), st.dirty = true;
    tip("How populous it is, as a rank among the pack's countries: the most populous take the most peopled lands, "
        "where the geography puts people (spec 86). The world's total comes from the People setting.");
  }
  if (toml::table* s = t["settlement"].as_table()) {
    double u = (*s)["urban_share"].value_or(0.5);
    ImGui::SetNextItemWidth(300);
    const double lo = 0.05, hi = 1.0;
    if (ImGui::SliderScalar("Urban share", ImGuiDataType_Double, &u, &lo, &hi, "%.2f")) s->insert_or_assign("urban_share", u), st.dirty = true;
    tip("The share of its people living in towns and cities; the rest live in villages and the countryside");
  }
  {
    const std::vector<double> sizes = numbers(t["households"]["sizes"]);
    double m = t["households"]["mean_size"].value_or(3.0);
    ImGui::SetNextItemWidth(300);
    const double lo = 1.5, hi = 8.0;
    if (ImGui::SliderScalar("Mean household (people)", ImGuiDataType_Double, &m, &lo, &hi, "%.2f") && sizes.size() == 4) {
      set_numbers(t, "households", "sizes", tilt_to_mean(sizes, m));
      if (toml::table* h = t["households"].as_table()) h->insert_or_assign("mean_size", std::round(m * 100) / 100);
      st.dirty = true;
    }
    tip("How many people live together on average; the shares of households of 1, 2-3, 4-5 and 6 or more follow it");
  }
  {
    const std::vector<double> ages = numbers(t["age_pyramid"]["shares"]);
    if (!ages.empty()) {
      double med = median_age(ages);
      ImGui::SetNextItemWidth(300);
      const double lo = 14, hi = 55;
      if (ImGui::SliderScalar("Median age (years)", ImGuiDataType_Double, &med, &lo, &hi, "%.1f")) {
        set_numbers(t, "age_pyramid", "shares", tilt_to_median(ages, med));
        st.dirty = true;
      }
      tip("Half its people are younger than this: a young country has many children and few elderly, an old one the reverse. "
          "The template's age pyramid is tilted to it.");
    }
  }
  ImGui::Checkbox("Advanced: every table", &st.advanced);
  tip("Edit every number: age pyramid, schooling by age, work by age and sex, occupations, sectors, households");
  if (st.advanced) draw_tables(st);
  ImGui::Spacing();
  ImGui::BeginDisabled(!st.dirty);
  if (ImGui::Button("Save")) {
    try {
      packs::save_country(st.open, st.sel_custom, st.draft);
      refresh_customs(st);
      st.dirty = false;
      st.note = "saved; worlds made from now on with this pack use it (worlds already made keep their copy)";
    } catch (const std::exception& ex) {
      st.error = ex.what();
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Revert")) {
    st.draft = packs::load_country(st.open, st.sel_custom);
    st.dirty = false;
  }
  ImGui::EndDisabled();
  ImGui::SameLine();
  if (ImGui::Button("Delete")) ImGui::OpenPopup("Delete country");
  if (ImGui::BeginPopupModal("Delete country", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Send %s to the recycle bin?", name);
    if (ImGui::Button("Delete")) {
      try {
        packs::remove_country(st.open, st.sel_custom);
      } catch (const std::exception& ex) {
        st.error = ex.what();
      }
      refresh_customs(st);
      st.sel_custom.clear();
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}

}  // namespace

void countries_pick(CountriesState& st, const std::string& pack, const std::string& id, bool advanced) {
  for (int i = 0; i < static_cast<int>(st.user_packs.size()); ++i)
    if (st.user_packs[static_cast<std::size_t>(i)] == pack) open_pack(st, i);
  if (st.pack < 0 || id.empty()) return;
  try {
    st.draft = packs::load_country(st.open, id);
    st.sel_custom = id;
    st.advanced = advanced;
  } catch (const std::exception& ex) {
    st.error = ex.what();
  }
}

void draw_countries(CountriesState& st) {
  ImGui::TextWrapped("Countries come in packs (spec 90). The default pack, Earth, holds the countries with real data under "
                     "parody names; it cannot be edited and serves as the guide. Make your own packs: include default "
                     "countries, make new ones from a template, and pick the pack when you make a world.");
  if (!st.error.empty()) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "%s", st.error.c_str());
  else if (!st.note.empty()) ImGui::TextDisabled("%s", st.note.c_str());
  const float h = ImGui::GetContentRegionAvail().y;
  ImGui::BeginChild("packs", {230, h}, ImGuiChildFlags_Borders);
  draw_packs(st);
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::BeginChild("list", {340, h}, ImGuiChildFlags_Borders);
  draw_list(st);
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::BeginChild("card", {0, h}, ImGuiChildFlags_Borders);
  if (!st.sel_custom.empty() && st.pack >= 0) draw_custom(st);
  else if (!st.sel_iso.empty()) draw_default(st);
  else ImGui::TextDisabled("Pick a country.");
  ImGui::EndChild();
}

}  // namespace zs::atlas
