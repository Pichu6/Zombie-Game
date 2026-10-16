// The Atlas (spec 55, decisions 0073 and 0074): the front door of zs.exe. Worlds, the Tour, the map of parts, the Try
// panel and the Breakthroughs board for everyone; the docs reader and the tests only when a repo sits next to the exe.
// It owns no simulation state: every button runs the same executable with the same flags a terminal would.
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "toml.hpp"
#include "zs/gen/place.h"
#include "zs/gen/world_countries.h"
#include "zs/gen/world_deposits.h"
#include "zs/gen/world_fields.h"
#include "zs/gen/world_grid.h"
#include "zs/gen/world_land.h"
#include "zs/gen/world_people.h"
#include "zs/tier0.h"
#include "atlas_cards.h"
#include "atlas_countries.h"
#include "country_packs.h"
#include "zs/gen/town_food.h"
#include "zs/gen/town_fuel.h"
#include "zs/gen/town_power.h"
#include "zs/nations.h"
#include "atlas_md.h"
#include "atlas_sim.h"
#include "modes.h"
#include "planet_raster.h"
#include "world_preview.h"
#include "runner.h"
#include "worlds.h"

namespace fs = std::filesystem;
using namespace zs::atlas;

namespace {

// ---- Data read from data/atlas (spec 55 rules 2, 4, 7) ----

struct Part {
  std::string id, title, line, today;
  std::vector<std::string> docs, specs;
};

struct Try {
  std::string id, part, spec, title, what, kind, added;
  std::vector<std::string> run;
  bool workshop = false;
};

struct Claim {
  std::string title, claim, evidence, barrier, push, status, try_id;
};

struct Stop {
  std::string title, say, show;
};

// A row of the Compute board (spec 55 rule 13): the number the design doc recorded and how to measure it again.
struct Measure {
  std::string id, title, what, unit, commit, date, try_id, pattern;
  double recorded = 0;
  double per_ticks = 0;  // > 0: the run's seconds over this many ticks, in us per tick
  bool gauge = false;
};

struct Speed {
  std::string name;
  double ticks_per_s = 0;
};

std::vector<std::string> strings(const toml::node_view<const toml::node>& n) {
  std::vector<std::string> out;
  if (const toml::array* a = n.as_array())
    for (const auto& e : *a)
      if (auto s = e.value<std::string>()) out.push_back(*s);
  return out;
}

std::string str(const toml::table& t, const char* key) { return t[key].value_or(std::string()); }

template <class F> void each_table(const toml::table& root, const char* key, F f) {
  if (const toml::array* a = root[key].as_array())
    for (const auto& e : *a)
      if (const toml::table* t = e.as_table()) f(*t);
}

struct Data {
  std::vector<Part> parts;
  std::vector<Try> tries;
  std::vector<Claim> claims;
  std::vector<Stop> stops;
  std::vector<Measure> measures;
  std::vector<Speed> speeds;
  std::string error;
};

Data load_data(const std::string& dir) {
  Data d;
  try {
    const toml::table parts = toml::parse_file(dir + "/parts.toml");
    each_table(parts, "part", [&](const toml::table& t) {
      d.parts.push_back({str(t, "id"), str(t, "title"), str(t, "line"), str(t, "today"), strings(t["docs"]), strings(t["specs"])});
    });
    const toml::table tries = toml::parse_file(dir + "/tries.toml");
    each_table(tries, "try", [&](const toml::table& t) {
      Try y{str(t, "id"), str(t, "part"), str(t, "spec"), str(t, "title"), str(t, "what"), str(t, "kind"), "", strings(t["run"]),
            t["workshop"].value_or(false)};
      if (auto date = t["added"].as_date()) {
        std::ostringstream o;
        o << *date;
        y.added = o.str();
      }
      d.tries.push_back(y);
    });
    std::stable_sort(d.tries.begin(), d.tries.end(), [](const Try& a, const Try& b) { return a.added > b.added; });
    const toml::table claims = toml::parse_file(dir + "/breakthroughs.toml");
    each_table(claims, "claim", [&](const toml::table& t) {
      d.claims.push_back({str(t, "title"), str(t, "claim"), str(t, "evidence"), str(t, "barrier"), str(t, "push"), str(t, "status"),
                          str(t, "try")});
    });
    const toml::table tour = toml::parse_file(dir + "/tour.toml");
    each_table(tour, "stop", [&](const toml::table& t) { d.stops.push_back({str(t, "title"), str(t, "say"), str(t, "show")}); });
    const toml::table compute = toml::parse_file(dir + "/compute.toml");
    each_table(compute, "measure", [&](const toml::table& t) {
      Measure m;
      m.id = str(t, "id");
      m.title = str(t, "title");
      m.what = str(t, "what");
      m.unit = str(t, "unit");
      m.commit = str(t, "commit");
      m.try_id = str(t, "try");
      m.pattern = str(t, "pattern");
      m.recorded = t["recorded"].value_or(0.0);
      m.per_ticks = t["per_ticks"].value_or(0.0);
      m.gauge = t["gauge"].value_or(false);
      if (auto date = t["date"].as_date()) {
        std::ostringstream o;
        o << *date;
        m.date = o.str();
      }
      d.measures.push_back(m);
    });
    each_table(compute, "speed", [&](const toml::table& t) { d.speeds.push_back({str(t, "name"), t["ticks_per_s"].value_or(0.0)}); });
  } catch (const std::exception& ex) {
    d.error = ex.what();
  }
  return d;
}

// ---- The repo next to the executable (decision 0073): workshop pages exist only when it is found ----

fs::path find_repo() {
  std::error_code ec;
  auto is_repo = [&](const fs::path& p) { return fs::exists(p / "docs" / "status.md", ec) && fs::exists(p / "CLAUDE.md", ec); };
  const fs::path data = zs::tier0::data_dir();
  if (is_repo(data.parent_path())) return data.parent_path();
  for (fs::path p = zs::worlds::exe_dir(); !p.empty() && p != p.root_path(); p = p.parent_path())
    if (is_repo(p)) return p;
  return {};
}

std::string read_text(const fs::path& p) {
  std::ifstream in(p, std::ios::binary);
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

// Runs a command to completion and returns its output; empty when it could not run (git missing, no repo).
std::string capture(const std::vector<std::string>& argv, const std::string& cwd) {
  Run r(argv, cwd);
  while (!r.done()) std::this_thread::sleep_for(std::chrono::milliseconds(5));
  return r.exit_code() == 0 ? r.output() : std::string();
}

// The release build next to the repo, which the test entries need.
bool has_build(const fs::path& repo) {
  std::error_code ec;
  return !repo.empty() && fs::exists(repo / "build" / "release" / "CMakeCache.txt", ec);
}

// The ctest the build was configured with, from its cache; "ctest" when there is none.
std::string ctest_of(const fs::path& repo) {
  std::ifstream in(repo / "build" / "release" / "CMakeCache.txt");
  std::string line;
  while (std::getline(in, line))
    if (line.rfind("CMAKE_CTEST_COMMAND:INTERNAL=", 0) == 0) return line.substr(29);
  return "ctest";
}

// ---- The planet (Tour stop 1): generated in a thread, drawn as a texture, countries clickable ----

struct CountryInfo {
  std::string name, income;
  std::string profile, origin, alliance;  // specs 90-92
  double area_km2 = 0, people = 0, endowment = 0;
  bool landlocked = false;
  int capital_held = 0, periods = 0, sectors = 0, towns = 0;
};

struct Planet {
  std::uint64_t seed = 0;
  zs::gen::WorldSettings settings;  // the settings the planet was generated with (slice 5)
  std::thread thread;
  std::atomic<bool> ready{false};
  std::atomic<bool> busy{false};
  std::string stage;
  std::mutex mutex;
  PlanetRaster raster;
  // Spec 83 rule 6: the generated world kept so any map layer can be drawn when picked.
  struct Data {
    zs::gen::WorldGrid grid;
    zs::gen::WorldFields fields;
    zs::gen::WorldLand land;
    zs::gen::WorldDeposits deposits;
    zs::gen::WorldPeople people;
    zs::gen::WorldLandUse use;
    zs::gen::WorldIndustry industry;  // spec 93
    zs::gen::WorldEnergy energy;      // spec 94
    zs::gen::WorldRoutes routes;      // spec 95
    zs::gen::WorldDiet diet;
  };
  std::shared_ptr<const Data> data;
  MapLayer layer = MapLayer::Political;
  std::vector<CountryInfo> countries;
  std::vector<CountryCard> cards;  // slice 3: the same countries with their towns, for the Cards page
  WorldCard world_card;
  std::string summary;
  SDL_Texture* texture = nullptr;
  int selected = -1;
  double seconds = 0;

  void generate(std::uint64_t s, const zs::gen::WorldSettings& st = {}) {
    if (busy) return;
    if (thread.joinable()) thread.join();
    seed = s;
    settings = st;
    ready = false;
    busy = true;
    if (texture) SDL_DestroyTexture(texture), texture = nullptr;
    selected = -1;
    thread = std::thread([this] {
      const auto t0 = std::chrono::steady_clock::now();
      try {
        auto set = [&](const char* what) { std::lock_guard<std::mutex> l(mutex); stage = what; };
        // Spec 84: the shared generator, so the world's size and richness show here too.
        const zs::gen::GeneratedWorld gw = zs::gen::generate_world(seed, settings, 0, set);
        const zs::gen::WorldGrid& grid = gw.grid;
        const zs::gen::WorldFields& f = gw.fields;
        const zs::gen::WorldLand& land = gw.land;
        const zs::gen::WorldDeposits& deposits = gw.deposits;
        const zs::gen::WorldPeople& people = gw.people;
        set("the picture");
        MapInputs mi;
        mi.countries = &people.formed;
        mi.deposits = &deposits;
        mi.people = &people;
        mi.use = &gw.use;
        mi.industry = &gw.industry;
        mi.energy = &gw.energy;
        mi.projection = Projection::EqualEarth;  // spec 89 amendment: poles as curved edges, not bands
        PlanetRaster r = render_layer(grid, f, land, mi, layer, 1024, 512);
        std::vector<CountryInfo> cs;
        std::vector<CountryCard> cards(people.countries.size());
        std::vector<int> towns(people.countries.size(), 0);
        for (std::size_t i = 0; i < people.towns.size(); ++i) {
          const auto& t = people.towns[i];
          if (t.country < 0 || t.country >= static_cast<int>(towns.size())) continue;
          ++towns[static_cast<std::size_t>(t.country)];
          TownCard tc;
          tc.index = static_cast<int>(i);
          tc.country = t.country;
          tc.rank = t.rank;
          tc.people = t.people;
          tc.cell = t.cell;
          const auto& v = grid.centre(t.cell);
          tc.lat = std::asin(std::clamp(v.z, -1.0, 1.0)) * 180.0 / 3.14159265358979;
          tc.lon = std::atan2(v.y, v.x) * 180.0 / 3.14159265358979;
          {  // spec 93: its plants by kind
            std::map<int, int> by;
            for (int pl : gw.industry.of_town[i]) ++by[gw.industry.plants[static_cast<std::size_t>(pl)].kind];
            for (const auto& [k, n] : by)
              tc.plants += (tc.plants.empty() ? "" : ", ") + std::to_string(n) + " " + gw.industry.kinds[static_cast<std::size_t>(k)].label;
          }
          cards[static_cast<std::size_t>(t.country)].towns.push_back(tc);
        }
        for (auto& c : cards) std::sort(c.towns.begin(), c.towns.end(), [](const TownCard& a, const TownCard& b) { return a.rank < b.rank; });
        WorldCard wc;
        wc.seed = seed;
        wc.cells = static_cast<std::size_t>(grid.size());
        wc.land_km2 = people.land_km2;
        wc.people = people.total_people;
        wc.countries = people.countries.size();
        wc.towns = people.towns.size();
        wc.start_polities = people.formed.start_polities;
        wc.periods = people.formed.periods;
        wc.events = people.formed.events.size();
        for (std::size_t k = 0; k < people.countries.size(); ++k) {
          const zs::gen::Country& c = people.countries[k];
          CountryInfo ci;
          ci.name = c.name.empty() ? "(unnamed)" : c.name;
          ci.profile = c.profile;
          ci.origin = c.origin;
          if (c.alliance >= 0 && c.alliance < static_cast<int>(people.alliance_names.size()))
            ci.alliance = people.alliance_names[static_cast<std::size_t>(c.alliance)];
          ci.income = zs::gen::kIncomeGroupIds[static_cast<std::size_t>(c.income)];
          ci.area_km2 = c.area_km2;
          ci.people = c.people;
          ci.endowment = c.endowment;
          ci.landlocked = c.landlocked;
          ci.sectors = static_cast<int>(c.sectors.size());
          ci.towns = towns[k];
          if (k < people.formed.countries.size()) ci.capital_held = people.formed.countries[k].capital_held;
          ci.periods = people.formed.periods;
          cs.push_back(ci);
          CountryCard& cc = cards[k];
          cc.name = ci.name;
          cc.income = ci.income;
          cc.profile = c.profile;
          cc.origin = c.origin;  // spec 91
          if (c.alliance >= 0 && c.alliance < static_cast<int>(people.alliance_names.size()))
            cc.alliance = people.alliance_names[static_cast<std::size_t>(c.alliance)];
          cc.area_km2 = c.area_km2;
          cc.people = c.people;
          cc.endowment = c.endowment;
          {  // spec 94: its electricity
            const auto& g2 = gw.energy.gen_twh[k];
            double all = 0;
            for (double x : g2) all += x;
            char eb[256];
            std::snprintf(eb, sizeof eb, "%.0f TWh a year (%.0f kWh a person); ", gw.energy.demand_twh[k], c.people > 0 ? gw.energy.demand_twh[k] * 1e9 / c.people : 0.0);
            cc.energy = eb;
            for (int q = 0; q < zs::gen::kFuels; ++q)
              if (all > 0 && g2[static_cast<std::size_t>(q)] / all >= 0.005) {
                std::snprintf(eb, sizeof eb, "%s%s %.0f %%", cc.energy.back() == ' ' ? "" : ", ", zs::gen::fuel_name(static_cast<zs::gen::Fuel>(q)),
                              100 * g2[static_cast<std::size_t>(q)] / all);
                cc.energy += eb;
              }
          }
          {  // spec 93: the country's plants by kind
            std::map<int, int> by;
            for (const auto& pl : gw.industry.plants)
              if (pl.country == static_cast<int>(k)) ++by[pl.kind];
            for (const auto& [kind, n] : by)
              cc.industry += (cc.industry.empty() ? "" : ", ") + std::to_string(n) + " " + gw.industry.kinds[static_cast<std::size_t>(kind)].label;
          }
          cc.landlocked = c.landlocked;
          cc.capital_held = ci.capital_held;
          cc.periods = ci.periods;
          cc.sectors = ci.sectors;
        }
        char buf[256];
        std::snprintf(buf, sizeof buf, "%zu cells, %.0f million km2 of land, %.0f million people, %zu countries, %zu towns",
                      static_cast<std::size_t>(grid.size()), people.land_km2 / 1e6, people.total_people / 1e6,
                      people.countries.size(), people.towns.size());
        auto kept = std::make_shared<Data>(Data{grid, f, land, deposits, people, gw.use, gw.industry, gw.energy, gw.routes, gw.diet});
        std::lock_guard<std::mutex> l(mutex);
        this->data = std::move(kept);
        raster = std::move(r);
        countries = std::move(cs);
        this->cards = std::move(cards);
        world_card = wc;
        summary = buf;
      } catch (const std::exception& ex) {
        std::lock_guard<std::mutex> l(mutex);
        summary = std::string("could not generate the planet: ") + ex.what();
      }
      seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
      ready = true;
      busy = false;
    });
  }

  void upload(SDL_Renderer* r) {
    if (texture || !ready || raster.w == 0) return;
    std::vector<unsigned char> rgba(static_cast<std::size_t>(raster.w) * raster.h * 4);
    for (std::size_t i = 0; i < static_cast<std::size_t>(raster.w) * raster.h; ++i) {
      rgba[i * 4] = raster.rgb[i * 3];
      rgba[i * 4 + 1] = raster.rgb[i * 3 + 1];
      rgba[i * 4 + 2] = raster.rgb[i * 3 + 2];
      rgba[i * 4 + 3] = 255;
    }
    texture = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, raster.w, raster.h);
    if (texture) {
      SDL_UpdateTexture(texture, nullptr, rgba.data(), raster.w * 4);
      SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    }
  }

  // Draws another layer of the kept world (about half a second at this size) and drops the old texture.
  void set_layer(MapLayer l) {
    if (l == layer || !ready || !data) return;
    layer = l;
    MapInputs mi;
    mi.countries = &data->people.formed;
    mi.deposits = &data->deposits;
    mi.people = &data->people;
    mi.use = &data->use;
    mi.industry = &data->industry;
    mi.energy = &data->energy;
    mi.projection = Projection::EqualEarth;
    raster = render_layer(data->grid, data->fields, data->land, mi, layer, 1024, 512);
    if (texture) SDL_DestroyTexture(texture), texture = nullptr;
  }

  ~Planet() {
    if (thread.joinable()) thread.join();
  }
};

// ---- The app ----

enum class Page { Worlds, Tour, Cards, Countries, Simulation, Parts, Try, Breakthroughs, Compute, Status, Docs };

struct DocView {
  fs::path path;  // relative to the repo
  std::vector<MdBlock> blocks;
  // The "where it lives" strip (atlas design §4 rule 2, slice 2): the last commit that touched the doc, and the
  // docs that link to it (decisions that build on it, specs that need it).
  std::string last_commit;
  std::vector<fs::path> linked_from;
};

struct App {
  SDL_Renderer* renderer = nullptr;
  Data data;
  fs::path repo;  // empty: no workshop pages
  std::string data_dir;
  Page page = Page::Worlds;
  // Worlds
  std::vector<zs::worlds::World> worlds;
  int world = -1;
  char new_name[64] = "";
  char new_seed[24] = "1";
  char new_notes[256] = "";
  char new_history_seed[24] = "";  // spec 91
  SetupDraft new_setup;          // the New world dialog's settings (spec 84)
  bool open_new_world = false;   // --new-world: open the dialog without a click (checks)
  char fork_name[64] = "";
  std::string world_error;
  // Tour
  int stop = 0;
  Planet planet;
  std::shared_ptr<CardsState> cards;
  std::shared_ptr<CountriesState> countries;  // spec 90
  WorldPreview preview;  // spec 84: the world setup's preview
  std::shared_ptr<SimState> sim;
  bool sim_auto_open = false, sim_auto_cold = false, sim_auto_advance = false;  // check shots
  int card_country = -1;  // set by a planet click, read once by the Cards page
  // Try
  int seed = 1;
  std::map<std::string, std::unique_ptr<Run>> runs;
  std::map<std::string, SDL_Texture*> images;
  std::string open_try;  // whose output shows in the panel
  fs::path out_dir;
  // Compute
  std::string build_commit;  // the repo's HEAD when one sits next to the executable; read once
  bool build_commit_read = false;
  // Docs
  std::vector<fs::path> docs;             // every .md under docs/, relative to the repo
  std::map<fs::path, std::string> texts;  // loaded on demand
  DocView doc;
  std::vector<fs::path> history;
  char search[96] = "";
  bool scroll_top = false;
  std::string status;
};

void refresh_worlds(App& a) {
  a.worlds = zs::worlds::list();
  if (a.worlds.empty()) {
    try {
      zs::worlds::create("World 1", 1, "The first world: seed 1, the one the tests pin.");
      a.worlds = zs::worlds::list();
    } catch (const std::exception& ex) {
      a.world_error = ex.what();
    }
  }
  if (a.world >= static_cast<int>(a.worlds.size())) a.world = static_cast<int>(a.worlds.size()) - 1;
  if (a.world < 0 && !a.worlds.empty()) a.world = 0;
}

const zs::worlds::World* current_world(const App& a) {
  return a.world >= 0 && a.world < static_cast<int>(a.worlds.size()) ? &a.worlds[static_cast<std::size_t>(a.world)] : nullptr;
}

std::string substitute(const App& a, std::string s) {
  auto replace = [&](const char* key, const std::string& value) {
    for (std::size_t p; (p = s.find(key)) != std::string::npos;) s.replace(p, std::strlen(key), value);
  };
  replace("{seed}", std::to_string(a.seed));
  replace("{world}", current_world(a) ? current_world(a)->name : "World 1");
  replace("{out}", a.out_dir.string());
  return s;
}

void start_try(App& a, const Try& t) {
  std::vector<std::string> argv;
  std::string cwd;
  if (t.kind == "test") {
    argv.push_back(ctest_of(a.repo));
    for (std::size_t i = 1; i < t.run.size(); ++i) argv.push_back(substitute(a, t.run[i]));
    cwd = a.repo.string();
  } else {
    argv.push_back(self_path());
    for (const auto& r : t.run) argv.push_back(substitute(a, r));
    cwd = zs::worlds::exe_dir().string();
  }
  std::error_code ec;
  fs::create_directories(a.out_dir, ec);
  if (auto it = a.images.find(t.id); it != a.images.end()) {
    SDL_DestroyTexture(it->second);
    a.images.erase(it);
  }
  a.runs[t.id] = std::make_unique<Run>(argv, cwd);
  a.open_try = t.id;
  a.status = "running " + a.runs[t.id]->command_line();
}

void play_world(App& a, const zs::worlds::World& w, const fs::path* save) {
  std::vector<std::string> argv{self_path(), "--play", "--world", w.name};
  if (save) {
    argv.push_back("--from");
    argv.push_back(save->string());
  }
  a.runs["play"] = std::make_unique<Run>(argv, zs::worlds::exe_dir().string());
  a.status = "opened " + w.name + (save ? " from " + save->filename().string() : " from the start");
}

// ---- Docs (workshop) ----

void scan_docs(App& a) {
  a.docs.clear();
  if (a.repo.empty()) return;
  std::error_code ec;
  for (const auto& e : fs::recursive_directory_iterator(a.repo / "docs", ec))
    if (e.is_regular_file() && e.path().extension() == ".md") a.docs.push_back(fs::relative(e.path(), a.repo, ec));
  std::sort(a.docs.begin(), a.docs.end());
}

const std::string& doc_text(App& a, const fs::path& rel) {
  auto it = a.texts.find(rel);
  if (it == a.texts.end()) it = a.texts.emplace(rel, read_text(a.repo / rel)).first;
  return it->second;
}

void open_doc(App& a, const fs::path& rel, bool remember = true) {
  if (remember && !a.doc.path.empty() && a.doc.path != rel) a.history.push_back(a.doc.path);
  a.doc.path = rel;
  a.doc.blocks = parse_markdown(doc_text(a, rel));
  a.page = Page::Docs;
  a.scroll_top = true;
  // The strip: git's last commit for the file (date and subject), and every doc whose text links to this file name.
  std::string line = capture({"git", "log", "-1", "--format=%cs %s", "--", rel.generic_string()}, a.repo.string());
  while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) line.pop_back();
  a.doc.last_commit = line;
  a.doc.linked_from.clear();
  const std::string name = rel.filename().string();
  for (const auto& d : a.docs)
    if (d != rel && doc_text(a, d).find(name + ")") != std::string::npos) a.doc.linked_from.push_back(d);
}

// A link in a doc: relative paths open in the reader, the web opens in the browser.
void follow_link(App& a, const std::string& target) {
  if (target.rfind("http", 0) == 0) {
    SDL_OpenURL(target.c_str());
    return;
  }
  std::string t = target;
  if (const auto hash = t.find('#'); hash != std::string::npos) t = t.substr(0, hash);
  if (t.empty()) return;
  std::error_code ec;
  const fs::path rel = fs::weakly_canonical(a.repo / a.doc.path.parent_path() / t, ec);
  const fs::path r = fs::relative(rel, a.repo, ec);
  if (fs::exists(rel, ec) && rel.extension() == ".md") open_doc(a, r);
  else a.status = "not a doc: " + target;
}

// The spec file for a number ("53" -> docs/tier0/specs/53-calendar.md), if the repo has it.
fs::path spec_path(const App& a, const std::string& number) {
  for (const auto& d : a.docs)
    if (d.parent_path() == fs::path("docs") / "tier0" / "specs" && d.filename().string().rfind(number + "-", 0) == 0) return d;
  return {};
}

// ---- Drawing ----

// A texture drawn as 16 x 8 tiles: one big textured quad overflows the software renderer's fixed-point triangle setup
// (the offscreen shots), and the tiles cost nothing on a GPU. Leaves an item under the cursor for hover and click.
void tiled_image(SDL_Texture* tex, const char* id, ImVec2 size) {
  const ImVec2 pos = ImGui::GetCursorScreenPos();
  ImGui::InvisibleButton(id, size);
  ImDrawList* d = ImGui::GetWindowDrawList();
  const int tx = 16, ty = 8;
  for (int j = 0; j < ty; ++j)
    for (int i = 0; i < tx; ++i) {
      const ImVec2 a0(pos.x + size.x * i / tx, pos.y + size.y * j / ty), a1(pos.x + size.x * (i + 1) / tx, pos.y + size.y * (j + 1) / ty);
      const ImVec2 u0(static_cast<float>(i) / tx, static_cast<float>(j) / ty), u1(static_cast<float>(i + 1) / tx, static_cast<float>(j + 1) / ty);
      d->AddImage(reinterpret_cast<ImTextureID>(tex), a0, a1, u0, u1);
    }
}

void preview_image(SDL_Texture* tex, const char* id, float w, float h) { tiled_image(tex, id, ImVec2(w, h)); }

void badge(const char* text, ImVec4 colour) {
  ImGui::PushStyleColor(ImGuiCol_Button, colour);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colour);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, colour);
  ImGui::SmallButton(text);
  ImGui::PopStyleColor(3);
}

ImVec4 status_colour(const std::string& s) {
  if (s == "measured") return {0.2f, 0.55f, 0.3f, 1};
  if (s == "built") return {0.2f, 0.45f, 0.6f, 1};
  if (s == "built in part") return {0.3f, 0.4f, 0.55f, 1};
  if (s == "decided") return {0.5f, 0.4f, 0.2f, 1};
  return {0.4f, 0.4f, 0.4f, 1};
}

void wrapped(const std::string& s) { ImGui::TextWrapped("%s", s.c_str()); }

void draw_run_output(App& a, const std::string& id) {
  auto it = a.runs.find(id);
  if (it == a.runs.end()) return;
  Run& r = *it->second;
  ImGui::TextDisabled("%s", r.command_line().c_str());
  if (!r.done()) ImGui::TextColored({0.9f, 0.8f, 0.4f, 1}, "running... %.1f s", r.seconds());
  else if (r.exit_code() == 0) ImGui::TextColored({0.55f, 0.9f, 0.55f, 1}, "done in %.1f s", r.seconds());
  else ImGui::TextColored({1, 0.5f, 0.5f, 1}, "exit code %d after %.1f s", r.exit_code(), r.seconds());
  // An image kind shows the file it wrote.
  const Try* t = nullptr;
  for (const Try& y : a.data.tries)
    if (y.id == id) t = &y;
  if (t && t->kind == "image" && r.done() && r.exit_code() == 0) {
    if (!a.images.count(id)) {
      for (const auto& arg : t->run)
        if (arg.find("{out}") != std::string::npos) {
          if (SDL_Surface* s = SDL_LoadBMP(substitute(a, arg).c_str())) {
            a.images[id] = SDL_CreateTextureFromSurface(a.renderer, s);
            SDL_DestroySurface(s);
          }
        }
      if (!a.images.count(id)) a.images[id] = nullptr;
    }
    if (SDL_Texture* tex = a.images[id]) {
      float w = 0, h = 0;
      SDL_GetTextureSize(tex, &w, &h);
      const float avail = ImGui::GetContentRegionAvail().x;
      const float scale = std::min(1.0f, avail / w);
      tiled_image(tex, "image", ImVec2(w * scale, h * scale));
    }
  }
  const std::string out = r.output();
  const float h = std::min(420.0f, ImGui::GetTextLineHeightWithSpacing() * (2 + std::count(out.begin(), out.end(), '\n')));
  ImGui::BeginChild(("out" + id).c_str(), ImVec2(0, h), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
  ImGui::TextUnformatted(out.c_str());
  if (!r.done()) ImGui::SetScrollHereY(1.0f);
  ImGui::EndChild();
}

void draw_planet(App& a) {
  Planet& p = a.planet;
  const zs::worlds::World* w = current_world(a);
  const std::uint64_t seed = w ? w->seed : 1;
  const zs::gen::WorldSettings settings = w ? w->settings : zs::gen::WorldSettings{};
  if (!p.busy && (!p.ready || p.seed != seed || !(p.settings == settings))) p.generate(seed, settings);
  if (p.busy) {
    std::string stage;
    {
      std::lock_guard<std::mutex> l(p.mutex);
      stage = p.stage;
    }
    ImGui::Text("Generating the planet of seed %llu: %s...", static_cast<unsigned long long>(seed), stage.c_str());
    return;
  }
  p.upload(a.renderer);
  ImGui::Text("Seed %llu: %s (%.1f s)", static_cast<unsigned long long>(p.seed), p.summary.c_str(), p.seconds);
  // Spec 83 rule 6: the map's layers, as Cities: Skylines' info views (WMR-19).
  for (int k = 0; k < kMapLayers; ++k) {
    const auto l = static_cast<MapLayer>(k);
    if (k % 6) ImGui::SameLine();  // two rows of six
    if (ImGui::RadioButton(layer_name(l), p.layer == l)) p.set_layer(l), p.upload(a.renderer);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", layer_legend(l).c_str());
  }
  ImGui::TextWrapped("%s", layer_legend(p.layer).c_str());
  if (!p.texture) return;
  const float avail = ImGui::GetContentRegionAvail().x - (p.selected >= 0 ? 320 : 0);
  const float scale = std::max(0.1f, avail / p.raster.w);
  const ImVec2 size(p.raster.w * scale, p.raster.h * scale);
  const ImVec2 pos = ImGui::GetCursorScreenPos();
  tiled_image(p.texture, "planet", size);
  int hover = -1;
  if (ImGui::IsItemHovered()) {
    const ImVec2 m = ImGui::GetMousePos();
    const int x = static_cast<int>((m.x - pos.x) / scale), y = static_cast<int>((m.y - pos.y) / scale);
    if (x >= 0 && y >= 0 && x < p.raster.w && y < p.raster.h) hover = p.raster.country[static_cast<std::size_t>(y) * p.raster.w + x];
    if (hover >= 0 && hover < static_cast<int>(p.countries.size())) {
      ImGui::SetTooltip("%s", p.countries[static_cast<std::size_t>(hover)].name.c_str());
      if (ImGui::IsItemClicked()) p.selected = hover;
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
        a.card_country = hover;
        a.page = Page::Cards;
      }
    } else if (ImGui::IsItemClicked()) p.selected = -1;
  }
  if (p.selected >= 0 && p.selected < static_cast<int>(p.countries.size())) {
    const CountryInfo& c = p.countries[static_cast<std::size_t>(p.selected)];
    ImGui::SameLine();
    ImGui::BeginChild("country", ImVec2(300, size.y), ImGuiChildFlags_Borders);
    if (!a.countries) a.countries = make_countries(a.data_dir, a.renderer);
    draw_country_flag(*a.countries, c.profile.empty() ? c.name : c.profile, 40);  // spec 92
    ImGui::SameLine();
    ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.3f);
    ImGui::TextWrapped("%s", c.name.c_str());
    ImGui::PopFont();
    ImGui::Separator();
    if (ImGui::BeginTable("card", 2, ImGuiTableFlags_RowBg)) {
      auto row = [](const char* k, const std::string& v) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextDisabled("%s", k);
        ImGui::TableSetColumnIndex(1);
        ImGui::TextWrapped("%s", v.c_str());
      };
      char buf[64];
      std::snprintf(buf, sizeof buf, "%.2f million km2", c.area_km2 / 1e6);
      row("Area", buf);
      std::snprintf(buf, sizeof buf, "%.1f million", c.people / 1e6);
      row("People", buf);
      row("Income group", c.income);
      std::snprintf(buf, sizeof buf, "%.2f", c.endowment);
      row("Endowment", buf);
      row("Coast", c.landlocked ? "landlocked" : "has a coast");
      std::snprintf(buf, sizeof buf, "%d of %d periods", c.capital_held, c.periods);
      row("Capital held", buf);
      row("Sectors", std::to_string(c.sectors));
      row("Towns", std::to_string(c.towns));
      if (!c.origin.empty()) row("Origin", c.origin);
      row("Alliance", c.alliance.empty() ? "none" : c.alliance);
      ImGui::EndTable();
    }
    ImGui::Spacing();
    if (ImGui::SmallButton("Open its card")) {
      a.card_country = p.selected;
      a.page = Page::Cards;
    }
    ImGui::TextDisabled("Double-click a country for its card:\nits towns, a pop, their house.");
    ImGui::EndChild();
  }
}

void draw_cards_page(App& a) {
  Planet& p = a.planet;
  const zs::worlds::World* w = current_world(a);
  const std::uint64_t seed = w ? w->seed : 1;
  const zs::gen::WorldSettings settings = w ? w->settings : zs::gen::WorldSettings{};
  if (!p.busy && (!p.ready || p.seed != seed || !(p.settings == settings))) p.generate(seed, settings);
  if (p.busy || !p.ready) {
    ImGui::Text("Generating the planet of seed %llu...", static_cast<unsigned long long>(seed));
    return;
  }
  if (!a.cards) a.cards = make_cards(a.data_dir);
  zs::set_custom_nations_dir(settings.pack == "earth" ? std::string{} : settings.pack_dir);  // spec 90
  // Spec 92: a clicked town's streets: the town the one world would open there (place_of_town, then town_record).
  cards_set_town_source(*a.cards, a.renderer,
                        [data = p.data, seed](int town) {
                          const zs::Place place = zs::gen::place_of_town(data->grid, data->fields, data->people, town, seed);
                          return zs::gen::town_record(place, seed);
                        },
                        seed);
  // Spec 97: a town's filling stations and their depot, worked out when its card opens.
  cards_set_fuel_source(*a.cards, [data = p.data, seed](int town) {
    const auto& t = data->people.towns[static_cast<std::size_t>(town)];
    const int income = t.country >= 0 ? data->people.countries[static_cast<std::size_t>(t.country)].income : 0;
    static const zs::gen::FuelRetailParams fp = zs::gen::load_fuel_retail(zs::tier0::data_dir() + "/gen");
    const zs::gen::FuelDepot depot = zs::gen::fuel_depot(data->routes, data->people, data->industry, town);
    const zs::gen::TownFuel f = zs::gen::plan_town_fuel({}, t.people, income, fp, seed, depot);
    double stock = 0;
    int gens = 0;
    for (const auto& s : f.stations) stock += s.stock_l, gens += s.generator;
    char buf[512];
    std::snprintf(buf, sizeof buf, "%zu filling stations (%d with their own generator) holding %.0f thousand L, %.1f days of the "
                  "%.0f thousand L its vehicles take a day", f.stations.size(), gens, stock / 1e3, f.litres_per_day > 0 ? stock / f.litres_per_day : 0.0,
                  f.litres_per_day / 1e3);
    std::string out = buf;
    if (depot.town >= 0) {
      const auto& d = data->people.towns[static_cast<std::size_t>(depot.town)];
      std::snprintf(buf, sizeof buf, "; tankers from %s in %s's town of %.0f thousand, %.0f h away", depot.refinery ? "a refinery" : "a port terminal",
                    data->people.countries[static_cast<std::size_t>(d.country)].name.c_str(), d.people / 1e3, depot.hours);
      out += buf;
    }
    return out;
  });
  // Spec 96: a town's power and its outage, worked out when its card opens.
  cards_set_power_source(*a.cards, [data = p.data, seed](int town) {
    const zs::gen::TownFeed feed = zs::gen::town_feed(data->grid, data->people, data->energy, town);
    return zs::gen::describe_feed(feed, zs::gen::outage_timeline(feed, data->energy, {}, seed));
  });
  // Spec 95: a town's food supply over the routes, worked out when its card opens.
  cards_set_supply_source(*a.cards, [data = p.data](int town) {
    const auto& t = data->people.towns[static_cast<std::size_t>(town)];
    const zs::gen::TownSupply s = zs::gen::town_supply(data->routes, data->diet, t.cell);
    char buf[512];
    std::snprintf(buf, sizeof buf, "%.0f %% of its region's food need met; %.0f %% of it grown in the region, the rest over %.1f h of routes",
                  100 * s.supplied, 100 * s.local, s.import_hours);
    std::string out = buf;
    for (const auto& f : s.from) {
      int big = -1;  // the supplying region's largest town
      for (int ti : data->routes.nodes[static_cast<std::size_t>(f.node)].towns)
        if (big < 0 || data->people.towns[static_cast<std::size_t>(ti)].people > data->people.towns[static_cast<std::size_t>(big)].people) big = ti;
      if (big < 0) continue;
      const auto& bt = data->people.towns[static_cast<std::size_t>(big)];
      std::snprintf(buf, sizeof buf, "; %.0f %% from %s's town of %.0f thousand (%.0f h%s)", 100 * f.share,
                    data->people.countries[static_cast<std::size_t>(bt.country)].name.c_str(), bt.people / 1e3, f.hours,
                    f.international ? ", abroad" : "");
      out += buf;
    }
    return out;
  });
  // The world's record: its settings, read-only (they are chosen when the world is made, in the New world dialog).
  WorldRecord rec;
  if (const zs::worlds::World* wr = current_world(a)) rec.settings = &wr->settings, rec.world_name = wr->name;
  draw_cards(*a.cards, p.world_card, p.cards, a.card_country, rec.settings ? &rec : nullptr);
  a.card_country = -1;
}

void draw_sim_page(App& a) {
  if (!a.sim) a.sim = make_sim();
  const zs::worlds::World* w = current_world(a);
  draw_sim(*a.sim, w ? w->seed : 1, w ? w->settings : zs::gen::WorldSettings{}, w ? w->name : "World 1", a.sim_auto_open, a.sim_auto_cold,
           a.sim_auto_advance);
  a.sim_auto_open = a.sim_auto_cold = a.sim_auto_advance = false;
}

void draw_worlds(App& a) {
  ImGui::TextWrapped("A world is a folder with its seed and its saves (decision 0074). Pick one; Play and every Try run on it. "
                     "Worlds live in %s", zs::worlds::root().string().c_str());
  ImGui::Spacing();
  if (ImGui::Button("New world") || a.open_new_world) {
    if (!a.open_new_world) {
      std::snprintf(a.new_seed, sizeof a.new_seed, "%llu", static_cast<unsigned long long>(std::rand() % 1000000 + 1));
      a.new_name[0] = 0;
      a.new_notes[0] = 0;
      reset_setup(a.new_setup, {});
      std::snprintf(a.new_history_seed, sizeof a.new_history_seed, "%llu", static_cast<unsigned long long>(std::rand() % 1000000 + 1));
    }
    a.open_new_world = false;
    a.world_error.clear();
    ImGui::OpenPopup("New world");
  }
  ImGui::SameLine();
  if (ImGui::Button("Refresh")) refresh_worlds(a);
  // The New world dialog (decision 0074, spec 84): name, seed and notes, then the setup with its preview; Create makes
  // the world with these settings, which then stay with it.
  const ImGuiViewport* vp = ImGui::GetMainViewport();
  ImGui::SetNextWindowSize({vp->WorkSize.x * 0.85f, vp->WorkSize.y * 0.9f}, ImGuiCond_Appearing);
  ImGui::SetNextWindowPos({vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y * 0.5f}, ImGuiCond_Appearing, {0.5f, 0.5f});
  if (ImGui::BeginPopupModal("New world", nullptr)) {
    if (!a.cards) a.cards = make_cards(a.data_dir);
    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint("Name", "a folder name: letters, digits, spaces, . - _", a.new_name, sizeof a.new_name);
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("Seed", a.new_seed, sizeof a.new_seed, ImGuiInputTextFlags_CharsDecimal);
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("The number the whole world grows from: the same seed and settings always give the same world");
    ImGui::SameLine();
    if (ImGui::SmallButton("Random")) std::snprintf(a.new_seed, sizeof a.new_seed, "%llu", static_cast<unsigned long long>(std::rand() % 1000000 + 1));
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("History seed", a.new_history_seed, sizeof a.new_history_seed, ImGuiInputTextFlags_CharsDecimal);
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("The number the world's history grows from (spec 91): the same planet seed with another history seed keeps\n"
                        "the planet and its founders' capitals but gives other borders, unions, divisions and alliances.");
    ImGui::SameLine();
    if (ImGui::SmallButton("Random##hs")) std::snprintf(a.new_history_seed, sizeof a.new_history_seed, "%llu", static_cast<unsigned long long>(std::rand() % 1000000 + 1));
    a.new_setup.s.history_seed = std::strtoull(a.new_history_seed, nullptr, 10);
    ImGui::InputTextMultiline("Notes", a.new_notes, sizeof a.new_notes, ImVec2(500, 50));
    // Spec 90: the country pack; a user pack is copied into the world when it is made.
    {
      std::vector<std::string> ids = {zs::packs::kEarth};
      for (const std::string& p : zs::packs::list()) ids.push_back(p);
      zs::gen::WorldSettings& d = a.new_setup.s;
      ImGui::SetNextItemWidth(300);
      if (ImGui::BeginCombo("Countries pack", d.pack == "earth" ? "Earth (default)" : d.pack.c_str())) {
        for (const std::string& id : ids)
          if (ImGui::Selectable(id == "earth" ? "Earth (default)" : id.c_str(), d.pack == id)) d.pack = id;
        ImGui::EndCombo();
      }
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Which countries the world's peoples take: Earth's (real data under parody names) or one of your packs\n"
                          "(the Countries page). A pack is copied into the world, so editing it later leaves this world as made.");
      d.pack_dir = zs::packs::dir_of(d.pack);
      ImGui::SameLine();
      if (d.pack == "earth") ImGui::TextDisabled("183 countries");
      else {
        try {
          ImGui::TextDisabled("%d countries", zs::packs::country_count(zs::packs::load(d.pack), a.data_dir));
        } catch (const std::exception& ex) {
          ImGui::TextColored({1, 0.5f, 0.5f, 1}, "%s", ex.what());
        }
      }
    }
    ImGui::Separator();
    ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.15f);
    ImGui::TextColored({1, 0.95f, 0.8f, 1}, "Settings");
    ImGui::PopFont();
    SetupHooks hooks;
    hooks.presets = [] { return zs::worlds::list_presets(); };
    hooks.load_preset = [](const std::string& n) { return zs::worlds::load_preset(n); };
    hooks.save_preset = [](const std::string& n, const zs::gen::WorldSettings& st) { zs::worlds::save_preset(n, st); };
    const float footer = ImGui::GetFrameHeightWithSpacing() * 2.2f;
    // Two columns: the settings, and the preview of this seed with them (spec 84 rule 3: look before creating).
    ImGui::BeginChild("setup", {std::max(560.0f, ImGui::GetContentRegionAvail().x * 0.45f), -footer});
    const bool date_ok = draw_world_setup(*a.cards, a.new_setup, hooks);
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("preview", {0, -footer});
    ImGui::BeginDisabled(!date_ok || a.preview.busy());
    if (ImGui::Button("Preview")) a.preview.start(std::strtoull(a.new_seed, nullptr, 10), a.new_setup.s);
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("Generates this seed with these settings at coarse cells in a few seconds and shows its maps and "
                        "key numbers beside Earth's. Nothing is written.");
    if (!a.preview.ready() && !a.preview.busy())
      ImGui::TextDisabled("Pick the settings, then Preview to see this world before creating it.");
    else if (a.preview.ready() && !(a.preview.settings() == a.new_setup.s))
      ImGui::TextColored({0.95f, 0.75f, 0.35f, 1}, "The settings changed since this preview: Preview again to see them.");
    a.preview.draw(a.renderer, preview_image);
    ImGui::EndChild();
    ImGui::Separator();
    if (!a.world_error.empty()) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "%s", a.world_error.c_str());
    ImGui::BeginDisabled(!date_ok);
    if (ImGui::Button("Create")) {
      try {
        zs::worlds::World w = zs::worlds::create(a.new_name, std::strtoull(a.new_seed, nullptr, 10), a.new_notes);
        w.settings = a.new_setup.s;
        w.realism = realism_of(*a.cards, w.settings);
        zs::packs::copy_into(w.settings.pack, w.dir);  // spec 90: the world keeps its own copy
        w.settings.pack_dir = w.settings.pack == "earth" ? std::string{} : (w.dir / "countries").string();
        zs::worlds::save_meta(w);
        refresh_worlds(a);
        for (std::size_t i = 0; i < a.worlds.size(); ++i)
          if (a.worlds[i].name == a.new_name) a.world = static_cast<int>(i);
        ImGui::CloseCurrentPopup();
      } catch (const std::exception& ex) {
        a.world_error = ex.what();
      }
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Makes the world's folder with this seed and these settings; they stay with it");
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
  ImGui::Spacing();
  if (!a.cards) a.cards = make_cards(a.data_dir);
  if (ImGui::BeginTable("worlds", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
    ImGui::TableSetupColumn("World", 0, 2);
    ImGui::TableSetupColumn("Seed", 0, 1);
    ImGui::TableSetupColumn("Created", 0, 1.5f);
    ImGui::TableSetupColumn("Saves", 0, 0.7f);
    ImGui::TableSetupColumn("Settings", 0, 1.4f);
    ImGui::TableSetupColumn("Built", 0, 1);
    ImGui::TableSetupColumn("Notes", 0, 3);
    ImGui::TableHeadersRow();
    for (std::size_t i = 0; i < a.worlds.size(); ++i) {
      const auto& w = a.worlds[i];
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      if (ImGui::Selectable(w.name.c_str(), static_cast<int>(i) == a.world, ImGuiSelectableFlags_SpanAllColumns)) a.world = static_cast<int>(i);
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%llu", static_cast<unsigned long long>(w.seed));
      ImGui::TableSetColumnIndex(2);
      ImGui::TextUnformatted(w.created.c_str());
      ImGui::TableSetColumnIndex(3);
      ImGui::Text("%zu", w.saves.size());
      ImGui::TableSetColumnIndex(4);
      {
        const int touched = touched_count(*a.cards, w.settings);
        if (touched == 0) ImGui::TextDisabled("Earth's defaults");
        else ImGui::Text("%s, %d touched", realism_of(*a.cards, w.settings).c_str(), touched);
      }
      ImGui::TableSetColumnIndex(5);
      ImGui::TextUnformatted(w.built.c_str());
      ImGui::TableSetColumnIndex(6);
      ImGui::TextWrapped("%s%s", w.forked_from.empty() ? "" : ("fork of " + w.forked_from + ". ").c_str(), w.notes.c_str());
    }
    ImGui::EndTable();
  }
  const zs::worlds::World* w = current_world(a);
  if (!w) return;
  ImGui::Spacing();
  ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.2f);
  ImGui::Text("%s", w->name.c_str());
  ImGui::PopFont();
  if (ImGui::Button("Play from the start")) play_world(a, *w, nullptr);
  ImGui::SameLine();
  if (ImGui::Button("Its card")) {  // the world card (no country selected): its settings and what it has
    a.card_country = -1;
    a.page = Page::Cards;
  }
  ImGui::SameLine();
  if (ImGui::Button("Fork")) {
    std::snprintf(a.fork_name, sizeof a.fork_name, "%s 2", w->name.c_str());
    a.world_error.clear();
    ImGui::OpenPopup("Fork world");
  }
  ImGui::SameLine();
  if (ImGui::Button("Delete")) ImGui::OpenPopup("Delete world");
  ImGui::SameLine();
  if (ImGui::Button("Open folder")) SDL_OpenURL(("file:///" + w->dir.generic_string()).c_str());
  if (ImGui::BeginPopupModal("Fork world", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextWrapped("A new world with the same seed (%llu); the saves stay with %s.", static_cast<unsigned long long>(w->seed), w->name.c_str());
    ImGui::InputText("New name", a.fork_name, sizeof a.fork_name);
    if (!a.world_error.empty()) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "%s", a.world_error.c_str());
    if (ImGui::Button("Fork")) {
      try {
        zs::worlds::fork(*w, a.fork_name);
        refresh_worlds(a);
        ImGui::CloseCurrentPopup();
      } catch (const std::exception& ex) {
        a.world_error = ex.what();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
  if (ImGui::BeginPopupModal("Delete world", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextWrapped("Send \"%s\" and its %zu saves to the recycle bin?", w->name.c_str(), w->saves.size());
    if (ImGui::Button("Delete")) {
      try {
        zs::worlds::remove(*w);
        a.world = -1;
        refresh_worlds(a);
      } catch (const std::exception& ex) {
        a.status = ex.what();
      }
      ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
      return;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
  ImGui::Spacing();
  if (w->saves.empty()) ImGui::TextDisabled("No saves yet: F5 in the window saves into this world.");
  else {
    ImGui::TextUnformatted("Saves (newest first):");
    for (const auto& s : w->saves) {
      ImGui::PushID(s.label.c_str());
      if (ImGui::SmallButton("Play from here")) play_world(a, *w, &s.path);
      ImGui::SameLine();
      ImGui::TextUnformatted(s.label.c_str());
      ImGui::PopID();
    }
  }
  if (a.runs.count("play")) {
    ImGui::Spacing();
    draw_run_output(a, "play");
  }
}

void draw_try_entry(App& a, const Try& t, bool with_output) {
  ImGui::PushID(t.id.c_str());
  const bool running = a.runs.count(t.id) && !a.runs[t.id]->done();
  ImGui::BeginDisabled(running);
  if (ImGui::Button(t.kind == "window" ? "Open" : "Run", ImVec2(60, 0))) start_try(a, t);
  ImGui::EndDisabled();
  ImGui::SameLine();
  ImGui::BeginGroup();
  ImGui::TextUnformatted(t.title.c_str());
  ImGui::SameLine();
  ImGui::TextDisabled("%s%s%s  %s", t.part.c_str(), t.spec.empty() ? "" : ", spec ", t.spec.c_str(), t.added.c_str());
  if (t.workshop) {
    ImGui::SameLine();
    badge("workshop", {0.35f, 0.3f, 0.45f, 1});
  }
  ImGui::TextWrapped("%s", t.what.c_str());
  ImGui::EndGroup();
  if (with_output && a.runs.count(t.id)) {
    if (a.open_try == t.id) draw_run_output(a, t.id);
    else if (ImGui::SmallButton("show output")) a.open_try = t.id;
  }
  ImGui::PopID();
  ImGui::Separator();
}

void draw_try(App& a) {
  ImGui::TextWrapped("Everything runnable, newest first. Each button runs zs.exe with the flags shown, so what it prints is what a terminal "
                     "would print. {seed} is the seed below; {world} is the world picked on the Worlds page.");
  ImGui::SetNextItemWidth(120);
  ImGui::InputInt("Seed", &a.seed);
  if (a.seed < 0) a.seed = 0;
  ImGui::SameLine();
  ImGui::TextDisabled("world: %s", current_world(a) ? current_world(a)->name.c_str() : "(none)");
  ImGui::Separator();
  for (const Try& t : a.data.tries) {
    if (t.workshop && (a.repo.empty() || (t.kind == "test" && !has_build(a.repo)))) continue;
    draw_try_entry(a, t, true);
  }
}

void draw_parts(App& a) {
  ImGui::TextWrapped("The project as the world's parts. Each one says what it is and where it stands today; in the workshop, its "
                     "design docs and specs open in the reader.");
  ImGui::Separator();
  for (const Part& p : a.data.parts) {
    ImGui::PushID(p.id.c_str());
    ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.25f);
    ImGui::TextColored({1, 0.95f, 0.8f, 1}, "%s", p.title.c_str());
    ImGui::PopFont();
    ImGui::TextWrapped("%s", p.line.c_str());
    ImGui::TextDisabled("Today: ");
    ImGui::SameLine();
    ImGui::TextWrapped("%s", p.today.c_str());
    if (!a.repo.empty()) {
      // Small buttons that wrap at the width.
      auto chip = [&](const std::string& label, const fs::path& rel) {
        const float w = ImGui::CalcTextSize(label.c_str()).x + ImGui::GetStyle().FramePadding.x * 2;
        if (ImGui::GetCursorPosX() > ImGui::GetWindowContentRegionMin().x + 1 && w > ImGui::GetContentRegionAvail().x) ImGui::NewLine();
        if (ImGui::SmallButton(label.c_str())) open_doc(a, rel);
        ImGui::SameLine();
      };
      for (const auto& d : p.docs) {
        const fs::path rel = fs::path("docs") / d;
        if (std::find(a.docs.begin(), a.docs.end(), rel) == a.docs.end()) continue;
        chip(rel.stem().string(), rel);
      }
      for (const auto& s : p.specs) {
        const fs::path rel = spec_path(a, s);
        if (!rel.empty()) chip("spec " + s, rel);
      }
      ImGui::NewLine();
    }
    int tries = 0;
    for (const Try& t : a.data.tries)
      if (t.part == p.id && !(t.workshop && (a.repo.empty() || (t.kind == "test" && !has_build(a.repo))))) ++tries;
    if (tries) {
      ImGui::TextDisabled("Try:");
      for (const Try& t : a.data.tries) {
        if (t.part != p.id || (t.workshop && (a.repo.empty() || (t.kind == "test" && !has_build(a.repo))))) continue;
        ImGui::SameLine();
        ImGui::PushID(t.id.c_str());
        if (ImGui::SmallButton(t.title.c_str())) {
          start_try(a, t);
          a.page = Page::Try;
        }
        ImGui::PopID();
      }
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::PopID();
  }
}

void draw_breakthroughs(App& a) {
  ImGui::TextWrapped("What this project does that, as far as we know, nobody else did: the claim, the evidence, the barrier and the next push. "
                     "Where a measurement is the evidence, Measure again runs it on this machine.");
  ImGui::Separator();
  for (const Claim& c : a.data.claims) {
    ImGui::PushID(c.title.c_str());
    ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.25f);
    ImGui::TextColored({1, 0.95f, 0.8f, 1}, "%s", c.title.c_str());
    ImGui::PopFont();
    ImGui::SameLine();
    badge(c.status.c_str(), status_colour(c.status));
    ImGui::TextWrapped("%s", c.claim.c_str());
    if (ImGui::BeginTable("claim", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("k", 0, 1);
      ImGui::TableSetupColumn("v", 0, 7);
      auto row = [](const char* k, const std::string& v) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextDisabled("%s", k);
        ImGui::TableSetColumnIndex(1);
        ImGui::TextWrapped("%s", v.c_str());
      };
      row("Evidence", c.evidence);
      row("Barrier", c.barrier);
      row("Next push", c.push);
      ImGui::EndTable();
    }
    if (!c.try_id.empty()) {
      for (const Try& t : a.data.tries)
        if (t.id == c.try_id) {
          const bool running = a.runs.count(t.id) && !a.runs[t.id]->done();
          ImGui::BeginDisabled(running);
          if (ImGui::Button(t.kind == "window" ? "Show" : "Measure again")) start_try(a, t);
          ImGui::EndDisabled();
          ImGui::SameLine();
          ImGui::TextDisabled("%s", t.title.c_str());
          if (a.runs.count(t.id)) draw_run_output(a, t.id);
        }
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::PopID();
  }
}

// ---- The Compute board (spec 55 rule 13, atlas design §6) ----

const Try* find_try(const App& a, const std::string& id) {
  for (const Try& t : a.data.tries)
    if (t.id == id) return &t;
  return nullptr;
}

// The new number of a row from its Try's finished run, or nothing while it runs, failed or does not match.
std::optional<double> measured(App& a, const Measure& m) {
  auto it = a.runs.find(m.try_id);
  if (it == a.runs.end() || !it->second->done() || it->second->exit_code() != 0) return std::nullopt;
  const Run& r = *it->second;
  if (m.pattern.empty()) {
    if (m.per_ticks > 0) return r.seconds() * 1e6 / m.per_ticks;
    return r.seconds();
  }
  try {
    std::smatch got;
    const std::string out = r.output();
    if (std::regex_search(out, got, std::regex(m.pattern)) && got.size() > 1) return std::atof(got[1].str().c_str());
  } catch (const std::regex_error&) {
  }
  return std::nullopt;
}

std::string number(double v) {
  char buf[32];
  std::snprintf(buf, sizeof buf, v >= 100 ? "%.0f" : v >= 10 ? "%.1f" : "%.2f", v);
  return buf;
}

// A time in microseconds as us, ms or s.
std::string time_us(double us) {
  if (us >= 1e6) return number(us / 1e6) + " s";
  if (us >= 1e3) return number(us / 1e3) + " ms";
  return number(us) + " us";
}

void draw_compute(App& a) {
  if (!a.build_commit_read) {
    a.build_commit_read = true;
    if (!a.repo.empty()) {
      a.build_commit = capture({"git", "rev-parse", "--short", "HEAD"}, a.repo.string());
      while (!a.build_commit.empty() && (a.build_commit.back() == '\n' || a.build_commit.back() == '\r')) a.build_commit.pop_back();
    }
  }
  const std::string now_commit = a.build_commit.empty() ? "this build" : a.build_commit;
  ImGui::TextWrapped("What the engine costs, measured: the number the compute budget recorded, and the same benchmark run again on this "
                     "machine, with the commit each came from. Measure again runs the Try entry whose output holds the number.");
  ImGui::Separator();
  // One button per Try entry; the rows that read it follow.
  std::vector<std::string> order;
  for (const Measure& m : a.data.measures)
    if (std::find(order.begin(), order.end(), m.try_id) == order.end()) order.push_back(m.try_id);
  for (const std::string& try_id : order) {
    const Try* t = find_try(a, try_id);
    ImGui::PushID(try_id.c_str());
    if (t) {
      const bool running = a.runs.count(t->id) && !a.runs[t->id]->done();
      ImGui::BeginDisabled(running);
      if (ImGui::Button("Measure again", ImVec2(120, 0))) start_try(a, *t);
      ImGui::EndDisabled();
      ImGui::SameLine();
      ImGui::TextUnformatted(t->title.c_str());
      ImGui::SameLine();
      ImGui::TextDisabled("zs %s", [&] {
        std::string s;
        for (const auto& r : t->run) s += (s.empty() ? "" : " ") + r;
        return s;
      }().c_str());
      if (auto it = a.runs.find(t->id); it != a.runs.end()) {
        ImGui::SameLine();
        if (!it->second->done()) ImGui::TextColored({0.9f, 0.8f, 0.4f, 1}, "running... %.1f s", it->second->seconds());
        else if (it->second->exit_code() == 0) ImGui::TextColored({0.55f, 0.9f, 0.55f, 1}, "done in %.1f s", it->second->seconds());
        else ImGui::TextColored({1, 0.5f, 0.5f, 1}, "exit code %d", it->second->exit_code());
      }
    } else {
      ImGui::TextColored({1, 0.5f, 0.5f, 1}, "no Try entry '%s' in tries.toml", try_id.c_str());
    }
    if (ImGui::BeginTable("rows", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("What", 0, 5);
      ImGui::TableSetupColumn("Recorded", 0, 2);
      ImGui::TableSetupColumn("On this machine", 0, 2);
      ImGui::TableSetupColumn("Change", 0, 1);
      ImGui::TableHeadersRow();
      for (const Measure& m : a.data.measures) {
        if (m.try_id != try_id) continue;
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(m.title.c_str());
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("%s", m.what.c_str());
        ImGui::PopStyleColor();
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s %s", number(m.recorded).c_str(), m.unit.c_str());
        ImGui::TextDisabled("%s, %s", m.commit.c_str(), m.date.c_str());
        ImGui::TableSetColumnIndex(2);
        const auto got = measured(a, m);
        if (got) {
          ImGui::Text("%s %s", number(*got).c_str(), m.unit.c_str());
          ImGui::TextDisabled("%s", now_commit.c_str());
        } else if (a.runs.count(m.try_id) && a.runs[m.try_id]->done() && a.runs[m.try_id]->exit_code() == 0) {
          ImGui::TextColored({1, 0.5f, 0.5f, 1}, "not found in the output");
        } else {
          ImGui::TextDisabled("-");
        }
        ImGui::TableSetColumnIndex(3);
        if (got && m.recorded > 0) {
          const double ratio = *got / m.recorded;
          const ImVec4 colour = ratio > 1.15 ? ImVec4{1, 0.6f, 0.5f, 1} : ratio < 0.87 ? ImVec4{0.55f, 0.9f, 0.55f, 1} : ImVec4{0.8f, 0.8f, 0.8f, 1};
          ImGui::TextColored(colour, "%+.0f %%", (ratio - 1) * 100);
        }
      }
      ImGui::EndTable();
    }
    ImGui::Spacing();
    ImGui::PopID();
  }
  // The gauge: the budget per tick at each speed against what the block costs today.
  double cost = 0;
  std::string cost_from;
  for (const Measure& m : a.data.measures)
    if (m.gauge) {
      if (auto got = measured(a, m)) {
        cost = *got;
        cost_from = now_commit;
      } else {
        cost = m.recorded;
        cost_from = m.commit + ", recorded";
      }
    }
  ImGui::Separator();
  ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.25f);
  ImGui::TextColored({1, 0.95f, 0.8f, 1}, "The budget per tick");
  ImGui::PopFont();
  ImGui::TextWrapped("The clock ticks every 0.5 game s. Each speed sets how many ticks run per real second, so how much CPU a tick may use. "
                     "The bar is what the Tier 0 block uses of that budget today: %s us per tick (%s). Six cores is the plan once chunks "
                     "run in parallel (0041); today everything runs on one.",
                     number(cost).c_str(), cost_from.c_str());
  if (ImGui::BeginTable("gauge", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
    ImGui::TableSetupColumn("Speed", 0, 3);
    ImGui::TableSetupColumn("Ticks per real s", 0, 2);
    ImGui::TableSetupColumn("One core per tick", 0, 3);
    ImGui::TableSetupColumn("Six cores per tick", 0, 2);
    ImGui::TableHeadersRow();
    for (const Speed& s : a.data.speeds) {
      if (s.ticks_per_s <= 0) continue;
      const double budget_us = 1e6 / s.ticks_per_s;
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(s.name.c_str());
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%.0f", s.ticks_per_s);
      ImGui::TableSetColumnIndex(2);
      const double used = cost / budget_us;
      char label[64];
      std::snprintf(label, sizeof label, "%.3g %% of %s", used * 100, time_us(budget_us).c_str());
      ImGui::PushStyleColor(ImGuiCol_PlotHistogram, used > 1 ? ImVec4{0.9f, 0.3f, 0.3f, 1} : used > 0.5 ? ImVec4{0.9f, 0.7f, 0.3f, 1} : ImVec4{0.35f, 0.65f, 0.4f, 1});
      ImGui::ProgressBar(static_cast<float>(std::min(1.0, used)), ImVec2(-1, 0), label);
      ImGui::PopStyleColor();
      ImGui::TableSetColumnIndex(3);
      ImGui::TextUnformatted(time_us(budget_us * 6).c_str());
    }
    ImGui::EndTable();
  }
}

void draw_tour(App& a) {
  if (a.data.stops.empty()) return;
  a.stop = std::clamp(a.stop, 0, static_cast<int>(a.data.stops.size()) - 1);
  const Stop& s = a.data.stops[static_cast<std::size_t>(a.stop)];
  ImGui::BeginDisabled(a.stop == 0);
  if (ImGui::Button("< Back")) --a.stop;
  ImGui::EndDisabled();
  ImGui::SameLine();
  for (int i = 0; i < static_cast<int>(a.data.stops.size()); ++i) {
    ImGui::SameLine();
    ImGui::PushID(i);
    if (ImGui::RadioButton("", i == a.stop)) a.stop = i;
    ImGui::PopID();
  }
  ImGui::SameLine();
  ImGui::BeginDisabled(a.stop + 1 >= static_cast<int>(a.data.stops.size()));
  if (ImGui::Button("Next >")) ++a.stop;
  ImGui::EndDisabled();
  ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.5f);
  ImGui::TextColored({1, 0.95f, 0.8f, 1}, "%d. %s", a.stop + 1, s.title.c_str());
  ImGui::PopFont();
  ImGui::TextWrapped("%s", s.say.c_str());
  ImGui::Separator();
  if (s.show == "planet") draw_planet(a);
  else if (s.show == "page:parts") draw_parts(a);
  else if (s.show == "page:worlds") draw_worlds(a);
  else if (s.show == "page:breakthroughs") draw_breakthroughs(a);
  else if (s.show == "page:compute") draw_compute(a);
  else if (s.show == "page:try") draw_try(a);
  else if (s.show == "page:simulation") draw_sim_page(a);
  else if (s.show.rfind("try:", 0) == 0) {
    for (const Try& t : a.data.tries)
      if (t.id == s.show.substr(4)) draw_try_entry(a, t, true);
  }
}

void draw_docs(App& a) {
  ImGui::BeginChild("list", ImVec2(260, 0), ImGuiChildFlags_Borders);
  ImGui::SetNextItemWidth(-1);
  ImGui::InputTextWithHint("##search", "search docs", a.search, sizeof a.search);
  const std::string q = a.search;
  std::string lower_q = q;
  std::transform(lower_q.begin(), lower_q.end(), lower_q.begin(), [](unsigned char c) { return std::tolower(c); });
  std::string group;
  for (const auto& d : a.docs) {
    if (!lower_q.empty()) {
      std::string name = d.string();
      std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
      bool hit = name.find(lower_q) != std::string::npos;
      if (!hit) {
        std::string text = doc_text(a, d);
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
        hit = text.find(lower_q) != std::string::npos;
      }
      if (!hit) continue;
    }
    const std::string g = d.parent_path().generic_string();
    if (g != group) {
      group = g;
      ImGui::TextDisabled("%s", g.c_str());
    }
    if (ImGui::Selectable(d.stem().string().c_str(), d == a.doc.path)) open_doc(a, d);
  }
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::BeginChild("doc", ImVec2(0, 0), ImGuiChildFlags_Borders);
  if (!a.history.empty() && ImGui::SmallButton("< back")) {
    const fs::path p = a.history.back();
    a.history.pop_back();
    open_doc(a, p, false);
  }
  if (!a.doc.path.empty()) {
    ImGui::SameLine();
    ImGui::TextDisabled("%s", a.doc.path.generic_string().c_str());
    if (!a.doc.last_commit.empty()) {
      ImGui::SameLine();
      ImGui::TextDisabled("|  last commit %s", a.doc.last_commit.c_str());
    }
    if (!a.doc.linked_from.empty()) {
      ImGui::TextDisabled("linked from:");
      for (const auto& d : a.doc.linked_from) {
        ImGui::SameLine();
        const std::string label = d.stem().string();
        const float w = ImGui::CalcTextSize(label.c_str()).x + ImGui::GetStyle().FramePadding.x * 2;
        if (w > ImGui::GetContentRegionAvail().x) ImGui::NewLine();
        ImGui::PushID(d.generic_string().c_str());
        if (ImGui::SmallButton(label.c_str())) {
          open_doc(a, d);
          ImGui::PopID();
          ImGui::EndChild();
          return;
        }
        ImGui::PopID();
      }
    }
    ImGui::Separator();
    if (a.scroll_top) ImGui::SetScrollY(0), a.scroll_top = false;
    draw_markdown(a.doc.blocks, [&](const std::string& target) { follow_link(a, target); });
  }
  ImGui::EndChild();
}

void draw_nav(App& a) {
  ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.4f);
  ImGui::TextColored({1, 0.95f, 0.8f, 1}, "Juego Zombie");
  ImGui::PopFont();
  ImGui::TextDisabled("the Atlas");
  ImGui::Separator();
  auto item = [&](const char* label, Page p) {
    if (ImGui::Selectable(label, a.page == p)) a.page = p;
  };
  item("Worlds", Page::Worlds);
  item("Tour", Page::Tour);
  item("Cards", Page::Cards);
  item("Countries", Page::Countries);
  item("Simulation", Page::Simulation);
  item("Parts", Page::Parts);
  item("Try", Page::Try);
  item("Breakthroughs", Page::Breakthroughs);
  item("Compute", Page::Compute);
  if (!a.repo.empty()) {
    ImGui::Spacing();
    ImGui::TextDisabled("Workshop");
    ImGui::Separator();
    if (ImGui::Selectable("Status", a.page == Page::Status)) {
      open_doc(a, fs::path("docs") / "status.md");
      a.page = Page::Status;
    }
    item("Docs", Page::Docs);
    ImGui::Spacing();
    ImGui::TextWrapped("repo: %s", a.repo.string().c_str());
  } else {
    ImGui::Spacing();
    ImGui::TextDisabled("No repo next to the\nexecutable: engine\npages only (0073).");
  }
}

}  // namespace

// zs --atlas [--shot out.bmp] [--page worlds|tour|parts|try|breakthroughs|compute|status|docs] [--stop N]
//   [--new-world] [--preview | --preview-preset NAME]   spec 84: the New world dialog open, with a preview of Earth's
//   settings or a preset
// --shot renders the page without a screen and saves it, for checks (like the window's --shot).
int atlas_main(int argc, char** argv) {
  std::string shot, page_name, try_id, preview_preset;
  bool preview = false, new_world = false, advanced_pick = false;
  std::string country_pick;
  int stop = 0, card = -1;
  bool pick = false, open_world = false, cold = false, advance = false;
  float scroll_to = -1.0f;
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--shot") && i + 1 < argc) shot = argv[++i];
    else if (!std::strcmp(argv[i], "--try") && i + 1 < argc) try_id = argv[++i];
    else if (!std::strcmp(argv[i], "--page") && i + 1 < argc) page_name = argv[++i];
    else if (!std::strcmp(argv[i], "--stop") && i + 1 < argc) stop = std::atoi(argv[++i]);
    else if (!std::strcmp(argv[i], "--card") && i + 1 < argc) card = std::atoi(argv[++i]);
    else if (!std::strcmp(argv[i], "--pick")) pick = true;
    else if (!std::strcmp(argv[i], "--scroll") && i + 1 < argc) scroll_to = static_cast<float>(std::atof(argv[++i]));
    else if (!std::strcmp(argv[i], "--open")) open_world = true;
    else if (!std::strcmp(argv[i], "--cold")) cold = true;
    else if (!std::strcmp(argv[i], "--advance")) advance = true;
    else if (!std::strcmp(argv[i], "--preview")) preview = true;  // spec 84: the New world dialog's preview, without a click
    else if (!std::strcmp(argv[i], "--new-world")) new_world = true;
    else if (!std::strcmp(argv[i], "--country-pick") && i + 1 < argc) country_pick = argv[++i];  // spec 90: PACK/ID
    else if (!std::strcmp(argv[i], "--advanced")) advanced_pick = true;
    else if (!std::strcmp(argv[i], "--preview-preset") && i + 1 < argc) preview = true, preview_preset = argv[++i];
  }
  if (!shot.empty()) SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
  auto fail = [](const char* what) {
    const std::string msg = std::string(what) + ": " + SDL_GetError();
    std::fprintf(stderr, "%s\n", msg.c_str());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "zs", msg.c_str(), nullptr);
    return 1;
  };
  if (!SDL_Init(SDL_INIT_VIDEO)) return fail("SDL_Init failed");
  SDL_Window* window = SDL_CreateWindow("Juego Zombie - Atlas", 1440, 900, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
  SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, nullptr) : nullptr;
  if (!renderer) return fail("Could not open the window");
  SDL_SetRenderVSync(renderer, 1);
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().IniFilename = nullptr;
  ImGui::StyleColorsDark();
  ImGui::GetStyle().FontScaleMain = 1.2f;
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);

  // A packaged build keeps its data folder beside the program: work from there, like the window does.
  if (const char* base = SDL_GetBasePath(); base && !std::getenv("ZS_DATA")) {
    std::error_code ec;
    if (fs::exists(fs::path(base) / "data", ec)) fs::current_path(base, ec);
  }
  App a;
  a.renderer = renderer;
  a.data_dir = zs::tier0::data_dir();
  a.data = load_data(a.data_dir + "/atlas");
  a.repo = find_repo();
  a.out_dir = zs::worlds::exe_dir() / "atlas_out";
  std::srand(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
  refresh_worlds(a);
  scan_docs(a);
  a.page = Page::Tour;
  if (page_name == "worlds") a.page = Page::Worlds;
  else if (page_name == "parts") a.page = Page::Parts;
  else if (page_name == "cards") a.page = Page::Cards;
  else if (page_name == "countries") a.page = Page::Countries;
  else if (page_name == "try") a.page = Page::Try;
  else if (page_name == "breakthroughs") a.page = Page::Breakthroughs;
  else if (page_name == "compute") a.page = Page::Compute;
  else if (page_name == "simulation") a.page = Page::Simulation;
  else if (page_name == "docs") a.page = Page::Docs;
  else if (page_name == "status" && !a.repo.empty()) open_doc(a, fs::path("docs") / "status.md");
  a.stop = stop;
  a.card_country = card;
  a.sim_auto_open = open_world;
  a.sim_auto_cold = cold;
  a.sim_auto_advance = advance;
  if (pick) {
    a.cards = make_cards(a.data_dir);
    cards_autopick(*a.cards, true, scroll_to);
  }
  if (!country_pick.empty()) {
    a.page = Page::Countries;
    a.countries = make_countries(a.data_dir, a.renderer);
    const std::size_t slash = country_pick.find('/');
    countries_pick(*a.countries, country_pick.substr(0, slash), slash == std::string::npos ? "" : country_pick.substr(slash + 1), advanced_pick);
  }
  if (new_world || preview) {  // the New world dialog, open; --preview starts its preview (seed 1 unless the world's)
    a.page = Page::Worlds;
    a.open_new_world = true;
    const zs::worlds::World* w = current_world(a);
    std::snprintf(a.new_seed, sizeof a.new_seed, "%llu", static_cast<unsigned long long>(w ? w->seed : 1));
    reset_setup(a.new_setup, preview_preset.empty() ? zs::gen::WorldSettings{} : zs::worlds::load_preset(preview_preset));
    if (preview) a.preview.start(w ? w->seed : 1, a.new_setup.s);
  }
  // --try ID with --shot: start that Try, wait for it, then shoot the Try page with its output (a check without a click).
  if (!try_id.empty())
    for (const Try& t : a.data.tries)
      if (t.id == try_id) {
        start_try(a, t);
        if (page_name.empty()) a.page = Page::Try;  // with --page compute, the board shows the number it read
      }

  bool running = true;
  int frame = 0, ready_frame = -1;  // the frame the planet became ready, for the shots
  while (running) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      ImGui_ImplSDL3_ProcessEvent(&ev);
      if (ev.type == SDL_EVENT_QUIT) running = false;
    }
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("atlas", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::BeginChild("nav", ImVec2(200, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_Borders);
    draw_nav(a);
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("content", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_Borders);
    if (!a.data.error.empty()) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "data/atlas: %s", a.data.error.c_str());
    switch (a.page) {
      case Page::Worlds: draw_worlds(a); break;
      case Page::Tour: draw_tour(a); break;
      case Page::Cards: draw_cards_page(a); break;
      case Page::Countries:
        if (!a.countries) a.countries = make_countries(a.data_dir, a.renderer);
        draw_countries(*a.countries);
        break;
      case Page::Simulation: draw_sim_page(a); break;
      case Page::Parts: draw_parts(a); break;
      case Page::Try: draw_try(a); break;
      case Page::Breakthroughs: draw_breakthroughs(a); break;
      case Page::Compute: draw_compute(a); break;
      case Page::Status:
      case Page::Docs: draw_docs(a); break;
    }
    ImGui::EndChild();
    ImGui::TextDisabled("%s", a.status.c_str());
    ImGui::End();

    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 24, 24, 28, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    if (!shot.empty()) {
      // Wait for the planet when the page shows it, then save the frame after it is drawn.
      if (a.planet.ready && ready_frame < 0) ready_frame = frame;
      bool waiting = a.planet.busy || ((a.page == Page::Cards || (a.page == Page::Tour && a.stop == 0)) && !a.planet.ready && frame < 2000);
      if (a.page == Page::Cards && (ready_frame < 0 || frame < ready_frame + 14)) waiting = true;  // frames to draw, pick and scroll
      if (a.page == Page::Cards && a.cards && cards_busy(*a.cards) && frame < 6000) waiting = true, ready_frame = frame;  // spec 92
      if (a.page == Page::Simulation) {
        if (!a.sim || sim_busy(*a.sim) || frame < 4) waiting = true;
        if (a.sim && !sim_busy(*a.sim) && (open_world || cold) && ready_frame < 0) ready_frame = frame;  // reuse: frames after the sim is idle
        if (ready_frame >= 0 && frame < ready_frame + (advance ? 40 : 3)) waiting = true;  // the advances run after the open
      }
      if (!try_id.empty() && a.runs.count(try_id) && !a.runs[try_id]->done()) waiting = true;
      if (preview && (a.preview.busy() || !a.preview.ready() || frame < ready_frame + 20)) waiting = true;
      if (frame >= 3 && !waiting) {
        if (SDL_Surface* s = SDL_RenderReadPixels(renderer, nullptr)) {
          SDL_SaveBMP(s, shot.c_str());
          SDL_DestroySurface(s);
          std::printf("saved %s\n", shot.c_str());
        }
        running = false;
      }
    }
    ++frame;
    SDL_RenderPresent(renderer);
    // A finished Play refreshes the saves list.
    if (auto it = a.runs.find("play"); it != a.runs.end() && it->second->done()) {
      a.runs.erase(it);
      refresh_worlds(a);
    }
  }
  a.runs.clear();
  for (auto& [id, tex] : a.images)
    if (tex) SDL_DestroyTexture(tex);
  if (a.planet.texture) SDL_DestroyTexture(a.planet.texture);
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
