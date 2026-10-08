// The Tier 0 window (spec 16): the run seen from above, a sandbox to play and inspect it.
// It reads the simulation and only sends it intents (World::submit); it keeps no game state.
//
// usage: zs [--seed N] [--watch] [--shot out.bmp [--ticks N] [--menu x,y] [--overlay noise|light]]
//   --watch starts with the AI driving everyone (nobody possessed).
//   --shot renders one frame without a screen (offscreen driver) and saves it, for checks.
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "zs/noise.h"
#include "zs/run.h"
#include "zs/gen/place.h"
#include "zs/tier0.h"
#include "modes.h"

using namespace zs;
using namespace zs::tier0;

namespace {

constexpr double kTicksPerRealSecond = 48;  // 1x: one real hour is one game day (0.5 game s per tick)
const int kSpeeds[] = {0, 1, 3, 10, 60, 360};

enum class Overlay { None, Noise, Light };

struct LogLine {
  std::int64_t tick;
  std::string text;
  const char* kind;
};

struct Target {
  EntityId id;
  std::string label;
};

struct App {
  std::uint64_t seed = 1;
  Tier0 t;
  int speed = 1;            // index into kSpeeds
  double tick_debt = 0;     // fractional ticks owed to real time
  bool resting = false;     // rest until something happens
  std::int64_t rest_until = 0;
  int level = 0;
  float cell = 26;          // pixels per cell
  ImVec2 origin{16, 60};    // screen position of cell (0, 0)
  Overlay overlay = Overlay::None;
  bool truth = false;       // debug: hidden infection and the like
  EntityId selected = 0;
  std::vector<LogLine> log;
  std::size_t seen = 0;     // events already turned into log lines
  char filter[64] = "";
  // Cached fields, refreshed every few game minutes.
  std::int64_t field_tick = -1000000;
  int field_level = -999;
  std::vector<double> noise, lux;
  double background = 0;
  // Context menu.
  std::optional<Cell> menu_cell;
  std::string status;       // last save/load message

  // The pop the player drives: the possessed one, or nobody while the AI drives everyone.
  EntityId me() const { return t.ai ? t.ai->possessed() : 0; }
  // Whom the view follows when nobody is possessed.
  EntityId followed() const { return me() ? me() : t.run->controlled(); }
  std::string name(EntityId b) const { return t.census ? t.census->name_of(b) : std::string("You"); }
};

void start(App& a, std::uint64_t seed) {
  a.seed = seed;
  a.t = zs::gen::make_world(seed);  // the one world (specs 41, 42)
  a.t.world->check_every_tick = false;
  a.t.ai->possess(a.t.run->controlled());
  a.selected = a.me();
  a.log.clear();
  a.seen = 0;
  a.field_tick = -1000000;
  a.resting = false;
  a.tick_debt = 0;
  a.level = a.t.actors->actor(a.followed()).pos.z;
}

std::string save_path(const App& a) { return "sandbox_seed" + std::to_string(a.seed) + ".sav"; }

void save_game(App& a) {
  const auto bytes = a.t.world->save();
  std::ofstream f(save_path(a), std::ios::binary);
  f.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  a.status = f ? "Saved " + save_path(a) : "Could not write " + save_path(a);
}

void load_game(App& a) {
  std::ifstream f(save_path(a), std::ios::binary);
  if (!f) {
    a.status = "No save for this seed yet (F5 saves)";
    return;
  }
  const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  try {
    start(a, a.seed);  // the same setup, then the saved state over it
    a.t.world->load(bytes);
    a.status = "Loaded " + save_path(a);
  } catch (const std::exception& ex) {
    start(a, a.seed);
    a.status = std::string("Load failed (") + ex.what() + "): started the seed again";
  }
  a.selected = a.followed();
  a.level = a.t.actors->actor(a.followed()).pos.z;
  a.seen = a.t.world->log.all().size();  // the log restarts at the loaded moment
}

void submit(App& a, CommandType type, std::int64_t x, std::int64_t y = 0) {
  if (!a.me()) return;  // the AI drives everyone
  Command c;
  c.type = static_cast<std::uint16_t>(type);
  c.subject = a.me();
  c.a = x;
  c.b = y;
  a.t.world->submit(c);
}

// ---- Simulation pacing ----

void read_events(App& a) {
  const auto& all = a.t.world->log.all();
  for (; a.seen < all.size(); ++a.seen) {
    const Event& e = all[a.seen];
    if (auto l = event_line(a.t, e)) {
      if (a.resting && (std::strcmp(l->kind, "alert") == 0 || std::strcmp(l->kind, "dead") == 0)) a.resting = false;
      a.log.push_back({e.tick, l->text, l->kind});
    }
  }
}

void advance(App& a, double real_dt) {
  const bool over = a.t.run->outcome() != RunOutcome::Running;
  if (over) a.resting = false;
  if (a.resting) {
    // As fast as the frame allows, until an alert or eight game hours.
    for (int i = 0; i < 20000 && a.resting; ++i) {
      a.t.world->step();
      if (a.t.world->clock.tick >= a.rest_until) a.resting = false;
      if ((i & 63) == 0) read_events(a);
    }
    read_events(a);
    return;
  }
  if (over || kSpeeds[a.speed] == 0) return;
  a.tick_debt += real_dt * kTicksPerRealSecond * kSpeeds[a.speed];
  int n = static_cast<int>(std::min(a.tick_debt, 20000.0));
  a.tick_debt = std::min(a.tick_debt - n, 1.0);
  while (n-- > 0) a.t.world->step();
  read_events(a);
}

// ---- Drawing ----

ImU32 rgb(int r, int g, int b, int alpha = 255) { return IM_COL32(r, g, b, alpha); }

ImU32 floor_colour(Floor f) {
  switch (f) {
    case Floor::Wood: return rgb(150, 120, 86);
    case Floor::Concrete: return rgb(140, 140, 136);
    case Floor::Grass: return rgb(88, 120, 70);
    case Floor::Asphalt: return rgb(70, 72, 76);
    case Floor::Dirt: return rgb(120, 100, 72);
    default: return 0;
  }
}

ImU32 dim(ImU32 c, double k) {
  const auto ch = [&](int shift) { return static_cast<int>(((c >> shift) & 0xFF) * k); };
  return IM_COL32(ch(IM_COL32_R_SHIFT), ch(IM_COL32_G_SHIFT), ch(IM_COL32_B_SHIFT), 255);
}

const ImU32 kPopColours[] = {rgb(232, 163, 61), rgb(111, 179, 217), rgb(156, 204, 101), rgb(217, 138, 201)};

std::string initials(const std::string& n) {
  std::string out;
  bool next = true;
  for (char c : n) {
    if (next && c != ' ') out += c;
    next = c == ' ';
  }
  return out.substr(0, 2);
}

void refresh_fields(App& a) {
  const World& w = *a.t.world;
  if (w.clock.tick - a.field_tick < 600 && a.field_level == a.level) return;  // every 5 game minutes
  a.field_tick = w.clock.tick;
  a.field_level = a.level;
  a.background = background_db(a.t.power->mains_alive(), w.clock.hour());
  a.noise = noise_field(*a.t.map, *a.t.items, noise_sources(w, *a.t.actors, *a.t.power));
  const SiteMap& m = *a.t.map;
  a.lux.assign(static_cast<std::size_t>(m.width() * m.height()), 0);
  for (int y = 0; y < m.height(); ++y)
    for (int x = 0; x < m.width(); ++x) {
      const Cell c{a.level, x, y};
      if (m.walkable(c)) a.lux[static_cast<std::size_t>(y * m.width() + x)] = a.t.power->lux_at(w, c);
    }
}

ImVec2 at(const App& a, double x, double y) {
  return {a.origin.x + static_cast<float>(x) * a.cell, a.origin.y + static_cast<float>(y) * a.cell};
}

void draw_map(App& a, ImDrawList* d) {
  const SiteMap& m = *a.t.map;
  const World& w = *a.t.world;
  refresh_fields(a);
  // Floors, darkened by how little light falls on them (outdoors at night is near black).
  for (int y = 0; y < m.height(); ++y)
    for (int x = 0; x < m.width(); ++x) {
      const Cell c{a.level, x, y};
      const ImU32 f = floor_colour(m.floor(c));
      if (!f) continue;
      const double lux = a.lux[static_cast<std::size_t>(y * m.width() + x)];
      const double k = std::clamp(0.3 + 0.7 * std::log10(1 + lux) / 3.0, 0.3, 1.0);
      d->AddRectFilled(at(a, x, y), at(a, x + 1, y + 1), dim(f, k));
      if (a.overlay == Overlay::Noise) {
        const double margin = noise_at(m, a.noise, c) - a.background;
        if (margin > 0) d->AddRectFilled(at(a, x, y), at(a, x + 1, y + 1), rgb(230, 60, 40, static_cast<int>(std::min(200.0, margin * 6))));
      } else if (a.overlay == Overlay::Light && lux > 1) {
        d->AddRectFilled(at(a, x, y), at(a, x + 1, y + 1), rgb(250, 230, 120, static_cast<int>(std::min(180.0, 40 * std::log10(lux)))));
      }
    }
  // Map objects (placeholder squares with a letter).
  for (const MapObject& o : m.objects()) {
    if (o.at.z != a.level || o.kind == "pop_start" || o.kind == "found") continue;
    d->AddRectFilled(at(a, o.at.x + 0.2, o.at.y + 0.2), at(a, o.at.x + 0.8, o.at.y + 0.8), rgb(60, 60, 70));
    d->AddText(at(a, o.at.x + 0.32, o.at.y + 0.22), rgb(230, 230, 230), o.kind.substr(0, 1).c_str());
  }
  // Items on the floor.
  for (const auto& [id, inst] : a.t.items->all()) {
    const auto c = a.t.actors->floor_cell(w.locations.parent_of(id));
    if (c && c->z == a.level) d->AddCircleFilled(at(a, c->x + 0.78, c->y + 0.78), a.cell * 0.1f, rgb(240, 220, 120));
  }
  // Edges: walls, doors, windows, fences; barricades in orange.
  for (int v = 0; v < 2; ++v)
    for (int y = 0; y <= m.height() - v; ++y)
      for (int x = 0; x <= m.width() - (v ? 0 : 1); ++x) {
        const Edge& e = m.edge({a.level, x, y, v == 1});
        ImU32 col;
        float thick = 3;
        switch (e.kind) {
          case EdgeKind::WallInterior: col = rgb(190, 185, 175); break;
          case EdgeKind::WallExterior: col = rgb(235, 230, 220); thick = 4; break;
          case EdgeKind::DoorInterior:
          case EdgeKind::DoorExterior: col = rgb(170, 110, 50); break;
          case EdgeKind::Window: col = rgb(140, 200, 240); break;
          case EdgeKind::Fence: col = rgb(110, 110, 100); thick = 2; break;
          case EdgeKind::Gate: col = rgb(150, 130, 90); thick = 2; break;
          default: continue;
        }
        const ImVec2 p0 = at(a, x, y), p1 = v ? at(a, x, y + 1) : at(a, x + 1, y);
        d->AddLine(p0, p1, col, thick);
        if (e.barricade) d->AddLine(p0, p1, rgb(240, 140, 40), 1.5f);
      }
  // Walkers.
  if (a.t.walkers)
    for (const auto& [id, z] : a.t.walkers->walkers()) {
      if (!a.t.bodies->get(id).alive || z.pos.z != a.level) continue;
      const ImVec2 c = at(a, z.pos.x + 0.5, z.pos.y + 0.5);
      d->AddCircleFilled(c, a.cell * 0.33f, z.mode == Mode::Idle ? rgb(150, 50, 50) : rgb(220, 40, 40));
      if (id == a.selected) d->AddCircle(c, a.cell * 0.45f, rgb(255, 255, 255), 0, 2);
    }
  // The group: coloured by member, grey when dead; a ring on the controlled pop. Pops sharing a cell
  // are drawn side by side so none hides another.
  std::vector<Cell> drawn;
  for (std::size_t i = 0; i < a.t.group.size(); ++i) {
    const EntityId id = a.t.group[i];
    const Body& b = a.t.bodies->get(id);
    const Cell p = a.t.actors->actor(id).pos;
    if (p.z != a.level || b.away || b.spec.walker) continue;
    const auto shared = std::count(drawn.begin(), drawn.end(), p);
    drawn.push_back(p);
    const ImVec2 c = at(a, p.x + 0.5 + 0.45 * static_cast<double>(shared % 2), p.y + 0.5 + 0.45 * static_cast<double>(shared / 2));
    d->AddCircleFilled(c, a.cell * 0.36f, b.alive ? kPopColours[i % 4] : rgb(110, 110, 110));
    const std::string ini = initials(a.name(id));
    d->AddText({c.x - 6, c.y - 7}, rgb(20, 20, 20), ini.c_str());
    if (id == a.me()) d->AddCircle(c, a.cell * 0.46f, rgb(255, 255, 255), 0, 2.5f);
    else if (id == a.selected) d->AddCircle(c, a.cell * 0.46f, rgb(200, 200, 200), 0, 1.5f);
  }
}

// ---- Input on the map ----

std::optional<Cell> cell_under(const App& a, ImVec2 mouse) {
  const int x = static_cast<int>(std::floor((mouse.x - a.origin.x) / a.cell));
  const int y = static_cast<int>(std::floor((mouse.y - a.origin.y) / a.cell));
  if (x < 0 || y < 0 || x >= a.t.map->width() || y >= a.t.map->height()) return std::nullopt;
  return Cell{a.level, x, y};
}

// Everything at a cell the controlled pop could act on, the pop itself included.
std::vector<Target> targets_at(const App& a, const Cell& c) {
  std::vector<Target> out;
  const World& w = *a.t.world;
  const auto& objs = a.t.map->objects();
  for (std::size_t i = 0; i < objs.size(); ++i)
    if (objs[i].at == c && objs[i].kind != "pop_start" && objs[i].kind != "found")
      out.push_back({a.t.actors->object_entity(i), objs[i].kind + (objs[i].id.empty() ? "" : " (" + objs[i].id + ")")});
  for (const auto& [id, inst] : a.t.items->all())
    if (w.locations.parent_of(id) == a.t.actors->floor_at(c)) out.push_back({id, a.t.items->def(inst.def).name});
  for (const auto& [id, act] : a.t.actors->actors())
    if (act.pos == c && !a.t.bodies->get(id).away) out.push_back({id, id == a.me() ? "yourself" : a.name(id)});
  if (a.t.walkers)
    for (const auto& [id, z] : a.t.walkers->walkers())
      if (z.pos == c && a.t.bodies->get(id).alive) out.push_back({id, a.name(id) + " (dead)"});
  for (const auto& [id, e] : a.t.actors->edges()) {
    const bool side = e.z == c.z && ((!e.vertical && e.x == c.x && (e.y == c.y || e.y == c.y + 1)) ||
                                     (e.vertical && e.y == c.y && (e.x == c.x || e.x == c.x + 1)));
    if (side) out.push_back({id, a.t.items->def(a.t.items->get(id).def).name});
  }
  out.push_back({a.t.actors->floor_at(c), "the floor here"});
  return out;
}

void context_menu(App& a) {
  if (!a.menu_cell) return;
  if (ImGui::BeginPopup("offers")) {
    const auto& procs = a.t.actors->processes();
    bool any = false;
    if (!a.me()) ImGui::TextDisabled("The AI drives everyone: take control in the Group panel");
    for (const Target& tg : targets_at(a, *a.menu_cell)) {
      if (!a.me()) break;
      const auto offers = a.t.actors->offers_for(*a.t.world, a.me(), tg.id);
      std::vector<Offer> shown;
      for (const Offer& o : offers)
        if (o.reason != Reason::WrongTarget && o.reason != Reason::UnknownProcess) shown.push_back(o);
      if (shown.empty()) continue;
      any = true;
      ImGui::SeparatorText(tg.label.c_str());
      for (const Offer& o : shown) {
        std::string label = procs[o.process].name;
        if (!o.enabled) label += "  (" + std::string(to_string(o.reason)) + ")";
        label += "##" + std::to_string(tg.id) + "_" + std::to_string(o.process);
        if (ImGui::Selectable(label.c_str(), false, o.enabled ? 0 : ImGuiSelectableFlags_Disabled))
          submit(a, CommandType::Use, static_cast<std::int64_t>(o.process), static_cast<std::int64_t>(tg.id));
      }
    }
    if (a.me() && a.t.map->walkable(*a.menu_cell)) {
      ImGui::Separator();
      if (ImGui::Selectable("Walk here")) submit(a, CommandType::MoveTo, pack_cell(*a.menu_cell));
    }
    if (!any && !a.t.map->walkable(*a.menu_cell)) ImGui::TextDisabled("Nothing here");
    ImGui::EndPopup();
  } else {
    a.menu_cell.reset();
  }
}

void map_input(App& a) {
  const ImGuiIO& io = ImGui::GetIO();
  if (!io.WantCaptureMouse) {
    if (io.MouseWheel != 0) {
      const float old = a.cell;
      a.cell = std::clamp(a.cell * (io.MouseWheel > 0 ? 1.15f : 1 / 1.15f), 8.0f, 80.0f);
      a.origin.x = io.MousePos.x - (io.MousePos.x - a.origin.x) * a.cell / old;
      a.origin.y = io.MousePos.y - (io.MousePos.y - a.origin.y) * a.cell / old;
    }
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
      a.origin.x += io.MouseDelta.x;
      a.origin.y += io.MouseDelta.y;
    }
    if (const auto c = cell_under(a, io.MousePos)) {
      if (!ImGui::IsPopupOpen("offers")) {
        // What is here, in words: the map uses placeholder shapes.
        std::string tip;
        for (const Target& tg : targets_at(a, *c)) tip += tg.label + "\n";
        ImGui::SetTooltip("%s", tip.c_str());
      }
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        for (const Target& tg : targets_at(a, *c))
          if (a.t.bodies->exists(tg.id)) {
            a.selected = tg.id;
            break;
          }
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        a.menu_cell = *c;
        ImGui::OpenPopup("offers");
      }
    }
  }
  if (io.WantCaptureKeyboard) return;
  // WASD: one cell at a time while held, whenever the pop is free.
  int dx = 0, dy = 0;
  if (ImGui::IsKeyDown(ImGuiKey_W)) dy = -1;
  if (ImGui::IsKeyDown(ImGuiKey_S)) dy = 1;
  if (ImGui::IsKeyDown(ImGuiKey_A)) dx = -1;
  if (ImGui::IsKeyDown(ImGuiKey_D)) dx = 1;
  const EntityId me = a.me();
  const bool moving_time = kSpeeds[a.speed] != 0 || a.resting;
  if ((dx || dy) && me && moving_time && a.t.bodies->get(me).alive && a.t.actors->idle(me)) {
    const Cell p = a.t.actors->actor(me).pos;
    const Cell to{p.z, p.x + dx, p.y + dy};
    if (a.t.map->walkable(to)) submit(a, CommandType::MoveTo, pack_cell(to));
  }
  if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
    a.speed = a.speed ? 0 : 1;
    a.resting = false;
  }
  for (int i = 0; i < 6; ++i)
    if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(ImGuiKey_1 + i))) {
      a.speed = i;
      a.resting = false;
    }
  if (ImGui::IsKeyPressed(ImGuiKey_PageUp) || ImGui::IsKeyPressed(ImGuiKey_RightBracket)) {
    const auto& lv = a.t.map->levels();
    auto it = std::find(lv.begin(), lv.end(), a.level);
    if (it != lv.end() && std::next(it) != lv.end()) a.level = *std::next(it);
  }
  if (ImGui::IsKeyPressed(ImGuiKey_PageDown) || ImGui::IsKeyPressed(ImGuiKey_LeftBracket)) {
    const auto& lv = a.t.map->levels();
    auto it = std::find(lv.begin(), lv.end(), a.level);
    if (it != lv.end() && it != lv.begin()) a.level = *std::prev(it);
  }
  if (ImGui::IsKeyPressed(ImGuiKey_C)) submit(a, CommandType::Cancel, 0);
  if (ImGui::IsKeyPressed(ImGuiKey_F5)) save_game(a);
  if (ImGui::IsKeyPressed(ImGuiKey_F9)) load_game(a);
}

// ---- Panels ----

void time_bar(App& a) {
  const World& w = *a.t.world;
  const Power& p = *a.t.power;
  ImGui::SetNextWindowPos({8, 8}, ImGuiCond_FirstUseEver);
  ImGui::Begin("Time", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
  ImGui::Text("%s", time_text(w.clock.tick).c_str());
  ImGui::SameLine();
  const char* names[] = {"Pause", "1x", "3x", "10x", "60x", "360x"};
  for (int i = 0; i < 6; ++i) {
    ImGui::SameLine();
    if (ImGui::RadioButton(names[i], a.speed == i && !a.resting)) {
      a.speed = i;
      a.resting = false;
    }
  }
  ImGui::SameLine();
  if (ImGui::Button(a.resting ? "Resting..." : "Rest until something happens")) {
    a.resting = !a.resting;
    a.rest_until = w.clock.tick + 8 * kTicksPerGameHour;
  }
  const double fuel_l = static_cast<double>(p.fuel_ul(w)) / 1e6;
  if (const PlaceSystem* place = a.t.place) {  // spec 42 rule 7
    const Place& at = place->place();
    ImGui::Text("A town of %.0f, %s-income country (spec 48) | %s, %.0f %s %.0f %s, %.0f m | outside %.1f C | rain today %.1f mm",
                at.town_people, at.income == "upper_middle" ? "upper-middle" : at.income == "lower_middle" ? "lower-middle" : at.income.c_str(),
                at.biome.c_str(), std::abs(at.lat_deg), at.lat_deg < 0 ? "S" : "N", std::abs(at.lon_deg), at.lon_deg < 0 ? "W" : "E",
                at.elevation_m, place->outdoor_c(w), place->today_rain_mm(w));
  }
  ImGui::Text("Grid %s | generator %s, %.1f L | level %d (PgUp/PgDn) | overlay:", p.mains_alive() ? "on" : "DEAD",
              p.generator_running() ? "running" : "off", fuel_l, a.level);
  ImGui::SameLine();
  int ov = static_cast<int>(a.overlay);
  ImGui::RadioButton("none", &ov, 0);
  ImGui::SameLine();
  ImGui::RadioButton("noise", &ov, 1);
  ImGui::SameLine();
  ImGui::RadioButton("light", &ov, 2);
  a.overlay = static_cast<Overlay>(ov);
  if (a.t.run->outcome() != RunOutcome::Running)
    ImGui::TextColored({1, 0.8f, 0.3f, 1}, "The run is over: %s", to_string(a.t.run->outcome()));
  if (!a.status.empty()) ImGui::TextDisabled("%s", a.status.c_str());
  ImGui::End();
}

void bar(const char* label, double v) {
  ImGui::ProgressBar(static_cast<float>(std::clamp(v, 0.0, 1.0)), {120, 0});
  ImGui::SameLine();
  ImGui::TextUnformatted(label);
}

void inspector(App& a) {
  const ImGuiIO& io = ImGui::GetIO();
  ImGui::SetNextWindowPos({io.DisplaySize.x - 340, 8}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize({330, 470}, ImGuiCond_FirstUseEver);
  ImGui::Begin("Inspector");
  const EntityId id = a.selected;
  if (!id || !a.t.bodies->exists(id)) {
    ImGui::TextDisabled("Left click a pop or a walker");
    ImGui::End();
    return;
  }
  const World& w = *a.t.world;
  const Body& b = a.t.bodies->get(id);
  ImGui::Text("%s, %d%s", a.name(id).c_str(), static_cast<int>(b.spec.age),
              b.spec.walker ? " (walker)" : !b.alive ? " (dead)" : id == a.me() ? " (controlled)"
              : a.t.ai && a.t.ai->minds().contains(id) ? " (AI)" : "");
  if (a.t.actors->actors().contains(id)) {
    const Actor& act = a.t.actors->actor(id);
    std::string doing = act.action.phase == Phase::Idle ? "nothing" : act.action.process < 0 ? "walking"
                        : a.t.actors->processes()[static_cast<std::size_t>(act.action.process)].name;
    if (act.action.phase == Phase::Approach && act.action.process >= 0) doing = "going to: " + doing;
    ImGui::Text("Doing: %s%s", doing.c_str(), b.activity.asleep ? " (asleep)" : "");
    if (a.t.ai && a.t.ai->minds().contains(id) && id != a.t.ai->possessed())
      ImGui::Text("Mind: %s", to_string(a.t.ai->mind(id).doing));
  }
  if (b.alive && !b.spec.walker) {
    ImGui::SeparatorText("Body");
    const Needs n = a.t.bodies->needs(b);
    bar("thirst", n.thirst);
    bar("hunger", n.hunger);
    bar("tiredness", n.tiredness);
    bar("exhaustion", n.exhaustion);
    bar("blood lost", a.t.bodies->blood_loss_fraction(b));
    bar("fear", b.fear);
    bar("morale", b.morale);
  }
  ImGui::SeparatorText("Wounds");
  if (b.wounds.empty()) ImGui::TextDisabled("none");
  for (const Wound& wd : b.wounds) {
    ImGui::BulletText("%s: %s, %.0f mm%s%s%s", to_string(wd.region), to_string(wd.type), wd.length_mm,
                      wd.cleaned != Cleaned::No ? ", cleaned" : "", wd.dressed ? ", dressed" : "",
                      wd.infection_visible || (a.truth && wd.infection != Infection::None) ? ", INFECTED" : "");
  }
  if (a.truth && a.t.walkers && a.t.walkers->turning_of(id)) ImGui::TextColored({1, 0.3f, 0.3f, 1}, "Truth: turning (walker pathogen)");
  if (a.t.actors->actors().contains(id)) {
    ImGui::SeparatorText("Carrying");
    const auto carried = a.t.actors->carried(w, id);
    if (carried.empty()) ImGui::TextDisabled("nothing");
    for (EntityId it : carried) ImGui::BulletText("%s", a.t.items->def(a.t.items->get(it).def).name.c_str());
  }
  ImGui::Separator();
  ImGui::Checkbox("Debug: show the truth", &a.truth);
  ImGui::End();
}

void group_panel(App& a) {
  const ImGuiIO& io = ImGui::GetIO();
  ImGui::SetNextWindowPos({io.DisplaySize.x - 340, 486}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize({330, 200}, ImGuiCond_FirstUseEver);
  ImGui::Begin("Group");
  for (std::size_t i = 0; i < a.t.group.size(); ++i) {
    const EntityId id = a.t.group[i];
    const Body& b = a.t.bodies->get(id);
    const bool joined = std::find(a.t.run->group().begin(), a.t.run->group().end(), id) != a.t.run->group().end();
    const char* state = b.spec.walker ? "a walker" : !b.alive ? "dead" : b.away ? "not here yet" : !joined ? "not found yet" : "in the group";
    ImGui::PushID(static_cast<int>(i));
    ImGui::ColorButton("##c", ImGui::ColorConvertU32ToFloat4(kPopColours[i % 4]), ImGuiColorEditFlags_NoTooltip, {12, 12});
    ImGui::SameLine();
    if (ImGui::Selectable((a.name(id) + ": " + state).c_str(), a.selected == id, 0, {200, 0})) a.selected = id;
    if (joined && b.alive && !b.spec.walker && id != a.me()) {
      ImGui::SameLine();
      if (ImGui::SmallButton("Control")) {
        a.t.ai->possess(id);
        a.selected = id;
      }
    }
    ImGui::PopID();
  }
  if (a.t.ai->possessed()) {
    if (ImGui::Button("Let the AI drive everyone")) a.t.ai->release();
  } else if (ImGui::Button("Take control again")) {
    a.t.ai->possess(a.t.run->controlled());
  }
  ImGui::End();
}

void log_panel(App& a) {
  const ImGuiIO& io = ImGui::GetIO();
  ImGui::SetNextWindowPos({8, io.DisplaySize.y - 236}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize({io.DisplaySize.x - 360, 220}, ImGuiCond_FirstUseEver);
  ImGui::Begin("Events");
  ImGui::SetNextItemWidth(200);
  ImGui::InputTextWithHint("##filter", "filter", a.filter, sizeof a.filter);
  ImGui::BeginChild("lines");
  for (const LogLine& l : a.log) {
    if (a.filter[0] && l.text.find(a.filter) == std::string::npos) continue;
    const ImVec4 col = std::strcmp(l.kind, "dead") == 0 ? ImVec4{1, 0.45f, 0.45f, 1}
                       : std::strcmp(l.kind, "body") == 0 ? ImVec4{0.7f, 0.7f, 0.7f, 1}
                       : std::strcmp(l.kind, "done") == 0 ? ImVec4{0.6f, 0.85f, 0.6f, 1}
                                                          : ImVec4{1, 0.85f, 0.4f, 1};
    ImGui::TextColored(col, "%s  %s", time_text(l.tick).c_str(), l.text.c_str());
  }
  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20) ImGui::SetScrollHereY(1.0f);
  ImGui::EndChild();
  ImGui::End();
}

void help_line() {
  const ImGuiIO& io = ImGui::GetIO();
  ImGui::GetForegroundDrawList()->AddText({8, io.DisplaySize.y - 18}, IM_COL32(200, 200, 200, 160),
                                          "WASD move | right click: actions | left click: select | wheel: zoom | middle drag: pan | "
                                          "Space pause, 1-6 speed | C cancel | F5 save, F9 load | placeholder art");
}

}  // namespace

int window_main(int argc, char** argv) {
  App a;
  std::uint64_t seed = 1;
  std::string shot;
  std::int64_t shot_ticks = 0;
  std::optional<Cell> shot_menu;
  bool watch = false;
  Overlay overlay = Overlay::None;
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--seed") && i + 1 < argc) seed = std::strtoull(argv[++i], nullptr, 10);
    else if (!std::strcmp(argv[i], "--shot") && i + 1 < argc) shot = argv[++i];
    else if (!std::strcmp(argv[i], "--watch")) watch = true;
    else if (!std::strcmp(argv[i], "--overlay") && i + 1 < argc) {
      ++i;
      overlay = !std::strcmp(argv[i], "noise") ? Overlay::Noise : !std::strcmp(argv[i], "light") ? Overlay::Light : Overlay::None;
    }
    else if (!std::strcmp(argv[i], "--ticks") && i + 1 < argc) shot_ticks = std::strtoll(argv[++i], nullptr, 10);
    else if (!std::strcmp(argv[i], "--menu") && i + 1 < argc) {
      int x = 0, y = 0;
      const std::string v = argv[++i];
      const auto comma = v.find(',');
      if (comma != std::string::npos) {
        x = std::atoi(v.substr(0, comma).c_str());
        y = std::atoi(v.substr(comma + 1).c_str());
        shot_menu = Cell{0, x, y};
      }
    }
  }
  if (!shot.empty()) SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
  // A Windows app has no console: errors go to a message box too.
  auto fail = [](const char* what) {
    const std::string msg = std::string(what) + ": " + SDL_GetError();
    std::fprintf(stderr, "%s\n", msg.c_str());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "zs", msg.c_str(), nullptr);
    return 1;
  };
  if (!SDL_Init(SDL_INIT_VIDEO)) return fail("SDL_Init failed");
  SDL_Window* window = SDL_CreateWindow("Juego Zombie - Tier 0 sandbox", 1440, 900, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
  SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, nullptr) : nullptr;
  if (!renderer) return fail("Could not open the window");
  SDL_SetRenderVSync(renderer, 1);
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().IniFilename = shot.empty() ? "sandbox_layout.ini" : nullptr;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);

  // A packaged build keeps its data folder beside the program: work from there (saves land there too).
  if (const char* base = SDL_GetBasePath(); base && !std::getenv("ZS_DATA")) {
    std::error_code ec;
    if (std::filesystem::exists(std::filesystem::path(base) / "data", ec)) std::filesystem::current_path(base, ec);
  }
  start(a, seed);
  if (watch) a.t.ai->release();
  a.overlay = overlay;
  if (shot_ticks > 0) {
    for (std::int64_t i = 0; i < shot_ticks; ++i) a.t.world->step();
    read_events(a);
    a.speed = 0;
    a.level = a.t.actors->actor(a.followed()).pos.z;
  }

  bool running = true;
  int frame = 0;
  std::uint64_t last = SDL_GetTicksNS();
  while (running) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      ImGui_ImplSDL3_ProcessEvent(&ev);
      if (ev.type == SDL_EVENT_QUIT) running = false;
    }
    const std::uint64_t now = SDL_GetTicksNS();
    const double dt = std::min(0.1, static_cast<double>(now - last) / 1e9);
    last = now;
    if (shot.empty()) advance(a, dt);

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    if (frame == 0) {
      // Fit the map between the time bar, the side panels and the log.
      const ImVec2 ds = ImGui::GetIO().DisplaySize;
      a.cell = std::clamp(std::min((ds.x - 380) / static_cast<float>(a.t.map->width()),
                                   (ds.y - 330) / static_cast<float>(a.t.map->height())), 8.0f, 80.0f);
      a.origin = {12, 84};
    }
    draw_map(a, ImGui::GetBackgroundDrawList());
    map_input(a);
    if (shot_menu && frame == 1) {
      a.menu_cell = shot_menu;
      ImGui::SetNextWindowPos(at(a, shot_menu->x + 1, shot_menu->y));
      ImGui::OpenPopup("offers");
    }
    context_menu(a);
    time_bar(a);
    inspector(a);
    group_panel(a);
    log_panel(a);
    help_line();
    ImGui::Render();

    SDL_SetRenderDrawColor(renderer, 24, 24, 28, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    if (!shot.empty() && frame == 3) {
      if (SDL_Surface* s = SDL_RenderReadPixels(renderer, nullptr)) {
        SDL_SaveBMP(s, shot.c_str());
        SDL_DestroySurface(s);
        std::printf("saved %s at %s\n", shot.c_str(), time_text(a.t.world->clock.tick).c_str());
      }
      running = false;
    }
    SDL_RenderPresent(renderer);
    ++frame;
  }
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
