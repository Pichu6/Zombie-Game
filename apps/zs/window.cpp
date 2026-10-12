// The Tier 0 window (spec 16): the run seen from above, a sandbox to play and inspect it.
// It reads the simulation and only sends it intents (World::submit); it keeps no game state.
// Time runs through the spec 40 pacer; WASD, the follow camera, shapes, the state column and alerts follow
// spec 16 rules 4-13 (decision 0055, research note view-readability).
//
// usage: zs [--seed N] [--watch] [--shot out.bmp [--ticks N] [--menu x,y] [--overlay noise|light] [--generator]]
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
#include <cctype>
#include <cstring>
#include <filesystem>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "zs/archive.h"
#include "zs/calendar.h"
#include "zs/expectations.h"
#include "zs/noise.h"
#include "zs/sound_marks.h"
#include "zs/pacer.h"
#include "zs/run.h"
#include "zs/gen/place.h"
#include "zs/tier0.h"
#include "modes.h"
#include "worlds.h"

using namespace zs;
using namespace zs::tier0;

namespace {

// The speeds offered (spec 40 ladder); keys 1-6 pick the ones after Pause.
const Pace kSpeeds[] = {Pace::Pause, Pace::Real, Pace::X3, Pace::X10, Pace::DayPerHour, Pace::X60, Pace::X360};
constexpr std::int64_t kGameMinute = 120;           // ticks
constexpr double kFollowTau = 0.1;                  // s, camera smoothing (spec 16 rule 7)
constexpr double kDeadZone = 0.2;                   // of the half view
constexpr float kStartCellPx = 32;
// Need bands for colours and words (estimate: the body has no named bands yet; spec 16 rule 10).
constexpr double kMild = 0.33, kBad = 0.6, kSevere = 0.85;

enum class Overlay { None, Noise, Light };

// Log categories, each with a filter checkbox (spec 16 rule 12).
enum Category { kActions = 0, kBody = 1, kDanger = 2, kWorld = 3, kCategories = 4 };
const char* const kCategoryNames[kCategories] = {"Actions", "Body", "Danger", "World"};

struct LogLine {
  std::int64_t tick;       // first occurrence
  std::int64_t last_tick;  // last repeat, for coalescing within a game minute
  std::string text;
  const char* kind;
  int category = kWorld;
  EntityId subject = 0;
  EventId event = 0;       // the last occurrence: its cause chain shows on hover
  int count = 1;
};

struct AlertLine {
  std::int64_t tick;
  std::string text;
};

struct Target {
  EntityId id;
  std::string label;
};

struct V2 {
  double x = 0, y = 0;
};

struct App {
  std::uint64_t seed = 1;
  Tier0 t;
  Pacer pacer;
  Pace resume = kStartPace;  // what Space resumes to
  bool resting = false;      // rest until something happens
  std::int64_t rest_until = 0;
  int level = 0;
  // Camera: the cell under the view's centre, pixels per cell, and the map's screen rectangle.
  V2 cam;
  float cell = kStartCellPx;
  bool follow = true;
  ImVec2 view_min{8, 90}, view_max{1000, 600};
  ImVec2 origin{16, 60};  // screen position of cell (0, 0), derived from cam each frame
  float time_bar_bottom = 90;
  Overlay overlay = Overlay::None;
  bool truth = false;  // debug: hidden infection and the like
  EntityId selected = 0;
  std::vector<LogLine> log;
  std::size_t seen = 0;  // events already turned into log lines
  char filter[64] = "";
  bool show[kCategories] = {true, true, true, true};
  bool pause_on_alerts = false;  // a sandbox option, off by default: watching lets the simulation run (decision 0056)
  bool only_selected = false;
  std::vector<AlertLine> alerts;
  std::map<EntityId, std::int64_t> sighted;  // walker -> last tick a group member saw it
  std::int64_t sight_tick = -1;
  // Cached fields, refreshed every few game minutes.
  std::int64_t field_tick = -1000000;
  int field_level = -999;
  std::vector<double> noise, lux;
  double background = 0;
  bool noise_any = false, light_any = false;
  // What the followed pop hears (spec 58): recomputed when its cell or the game-ten-seconds key changes.
  std::vector<SoundMark> marks;
  std::int64_t marks_key = -1;
  Cell marks_cell{};
  SoundMemory echoes;  // what was heard a moment ago, fading (spec 58)
  EntityId marks_id = 0;
  // Input.
  std::optional<Cell> menu_cell;
  std::optional<Cell> hover_cell;
  std::int64_t last_move_tick = -1;  // the tick the last Walk was sent at: one per applied tick
  int face_dx = 0, face_dy = 1;      // the controlled pop's arrow
  std::vector<EdgeRef> flash;        // edges that refused a WASD step
  double flash_until = 0, hint_until = 0, real_s = 0;
  std::string status;  // last save/load message
  std::optional<zs::worlds::World> world;  // decision 0074: the world folder the saves go to (--world)

  // The pop the player drives: the possessed one, or nobody while the AI drives everyone.
  EntityId me() const { return t.ai ? t.ai->possessed() : 0; }
  // Whom the view follows when nobody is possessed.
  EntityId followed() const { return me() ? me() : t.run->controlled(); }
  std::string name(EntityId b) const { return t.census ? t.census->name_of(b) : std::string("You"); }
  bool in_group(EntityId b) const { return std::find(t.group.begin(), t.group.end(), b) != t.group.end(); }
};

// Where a body is drawn, in cells: its cell centre plus its progress toward the next cell of its path (read-only,
// spec 16 rule 8). Cells are 1 m and paths step one cell, so the progress in metres is the fraction.
V2 body_at(const Cell& pos, const std::vector<Cell>& path, std::size_t next, double step_m, bool moving) {
  V2 p{pos.x + 0.5, pos.y + 0.5};
  if (moving && next < path.size() && path[next].z == pos.z) {
    const double f = std::clamp(step_m, 0.0, 1.0);
    p.x += (path[next].x - pos.x) * f;
    p.y += (path[next].y - pos.y) * f;
  }
  return p;
}

V2 actor_at(const App& a, EntityId id) {
  const Actor& act = a.t.actors->actor(id);
  return body_at(act.pos, act.action.path, act.action.path_next, act.action.step_m, act.action.phase == Phase::Approach);
}

void start(App& a, std::uint64_t seed) {
  a.seed = seed;
  a.t = zs::gen::make_world(seed, a.world ? a.world->settings : zs::gen::WorldSettings{});  // the one world (specs 41, 42), the world's settings (spec 55)
  a.t.world->check_every_tick = false;
  a.t.ai->possess(a.t.run->controlled());
  a.selected = a.me();
  a.log.clear();
  a.alerts.clear();
  a.sighted.clear();
  a.sight_tick = -1;
  a.seen = 0;
  a.field_tick = -1000000;
  a.resting = false;
  a.pacer = Pacer{};
  a.last_move_tick = -1;
  a.level = a.t.actors->actor(a.followed()).pos.z;
  a.follow = true;
  a.cam = actor_at(a, a.followed());
}

// Without a world folder (a bare --seed), the old file beside the program; with one, saves/<date>-tick<N>.sav (0074).
std::string save_path(const App& a) {
  if (a.world) return zs::worlds::next_save_path(*a.world, format_civil(civil_now(*a.t.world)), a.t.world->clock.tick).string();
  return "sandbox_seed" + std::to_string(a.seed) + ".sav";
}
// What F9 loads: the newest save of the world, or the old file beside the program.
std::string load_path(const App& a) {
  if (a.world) {
    zs::worlds::World w = zs::worlds::load(a.world->name);  // re-read: F5 may have added one
    return w.saves.empty() ? "" : w.saves.front().path.string();
  }
  return "sandbox_seed" + std::to_string(a.seed) + ".sav";
}

void save_game(App& a) {
  try {
    zs::write_file(save_path(a), a.t.world->save());  // temp file, read back, rename (spec 06 rules 5, 13, 15)
    a.status = "Saved " + save_path(a);
  } catch (const std::exception& ex) {
    a.status = "Could not save " + save_path(a) + ": " + ex.what();
  }
}

void load_game(App& a, std::string path = "") {
  if (path.empty()) path = load_path(a);
  std::vector<std::uint8_t> bytes;
  try {
    if (path.empty()) throw std::runtime_error("none");
    bytes = zs::read_file(path);
  } catch (const std::exception&) {
    a.status = a.world ? "No save in this world yet (F5 saves)" : "No save for this seed yet (F5 saves)";
    return;
  }
  try {
    start(a, a.seed);  // the same setup, then the saved state over it
    a.t.world->load(bytes);
    a.status = "Loaded " + path;
  } catch (const std::exception& ex) {
    start(a, a.seed);
    a.status = std::string("Load failed (") + ex.what() + "): started the seed again";
  }
  a.selected = a.followed();
  a.level = a.t.actors->actor(a.followed()).pos.z;
  a.cam = actor_at(a, a.followed());
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

// ---- Alerts (spec 16 rule 11, spec 40 rule 10) ----

const char* compass(double dx, double dy) {
  static const char* names[] = {"E", "SE", "S", "SW", "W", "NW", "N", "NE"};  // y grows southwards
  const double turn = std::atan2(dy, dx) / (2 * 3.14159265358979) * 8;
  return names[((static_cast<int>(std::lround(turn)) % 8) + 8) % 8];
}

void raise_alert(App& a, const std::string& text) {
  const std::int64_t now = a.t.world->clock.tick;
  if (a.pause_on_alerts) a.pacer.alert(text, now);  // otherwise the alert is only shown, time runs on
  a.alerts.push_back({now, text});
  if (a.alerts.size() > 20) a.alerts.erase(a.alerts.begin());
  a.resting = false;
}

// A walker coming into sight of a group member. Re-alerts only for a walker unseen for five game minutes.
void check_sight(App& a) {
  const World& w = *a.t.world;
  if (!a.t.walkers || w.clock.tick == a.sight_tick) return;
  a.sight_tick = w.clock.tick;
  for (EntityId m : a.t.group) {
    const Body& b = a.t.bodies->get(m);
    if (!b.alive || b.away || b.spec.walker || !a.t.actors->actors().contains(m)) continue;
    const Cell p = a.t.actors->actor(m).pos;
    int fresh = 0;
    std::optional<Cell> first;
    for (EntityId z : a.t.walkers->visible_to(w, m)) {
      const auto it = a.sighted.find(z);
      if (it == a.sighted.end() || w.clock.tick - it->second > 5 * kGameMinute) {
        ++fresh;
        if (!first) first = a.t.walkers->walkers().at(z).pos;
      }
      a.sighted[z] = w.clock.tick;
    }
    if (fresh) {
      const double dx = first->x - p.x, dy = first->y - p.y;
      char buf[160];
      std::snprintf(buf, sizeof buf, "%s sees %s, %.0f m %s", a.name(m).c_str(),
                    fresh == 1 ? "a walker" : (std::to_string(fresh) + " walkers").c_str(), std::hypot(dx, dy), compass(dx, dy));
      raise_alert(a, buf);
    }
  }
}

const char* const kHeadingNames[8] = {"north", "north-east", "east", "south-east", "south", "south-west", "west", "north-west"};

std::string process_name(const App& a, std::int64_t p) {
  const auto& procs = a.t.actors->processes();
  return p >= 0 && static_cast<std::size_t>(p) < procs.size() ? procs[static_cast<std::size_t>(p)].name : std::string("walking");
}

std::string where(const App& a, const Cell& c) {
  const int r = a.t.map->room_of(c);
  const bool indoor = r >= 0 && a.t.map->rooms()[static_cast<std::size_t>(r)].indoor;
  const char* level = c.z < 0 ? "the basement" : c.z == 0 ? "the ground floor" : "upstairs";
  return std::string(indoor ? "indoors on " : "outdoors on ") + level + " (" + std::to_string(c.x) + ", " + std::to_string(c.y) + ")";
}

struct Described {
  std::string text;
  const char* kind;  // colour: alert, dead, body, done
  int category;
};

// A pop id's name from the census (the founders); "someone" for anyone else (a born pop is not in the census).
std::string pop_name(const App& a, PopId id) {
  if (a.t.census && !is_born(id) && founder_slot_of(id) < a.t.census->people().size()) return a.t.census->people()[founder_slot_of(id)].name;
  return "someone";
}

// One event in words, with its context (spec 16 rule 12). The chronicle's own lines (tier0::event_line, shared with the
// headless app) stay as they are; the window adds the everyday ones: orders, actions done or stopped, sleep, wounds.
std::optional<Described> describe(const App& a, const Event& e) {
  const auto nm = [&](EntityId b) { return a.name(b); };
  switch (e.type) {
    case EventType::Intent: {
      switch (static_cast<CommandType>(e.a)) {
        case CommandType::MoveTo: return Described{nm(e.subject) + ": going to a spot " + where(a, unpack_cell(e.b)), "body", kActions};
        case CommandType::Use: return Described{nm(e.subject) + ": decided to " + process_name(a, e.b), "body", kActions};
        case CommandType::Cancel: return Described{nm(e.subject) + ": dropped what they were doing", "body", kActions};
        case CommandType::Walk:
          if (e.b < 0 || e.b > 7) return std::nullopt;  // releasing the keys says nothing new
          return Described{nm(e.subject) + ": walking " + kHeadingNames[e.b], "body", kActions};
      }
      return std::nullopt;
    }
    case EventType::ActionDone:
      if (e.a < 0) return std::nullopt;
      return Described{nm(e.subject) + ": finished \"" + process_name(a, e.a) + "\"", "done", kActions};
    case EventType::ActionInterrupted:
      return Described{nm(e.subject) + ": stopped " + (e.a < 0 ? std::string("walking") : "\"" + process_name(a, e.a) + "\"") +
                       " (" + to_string(static_cast<Reason>(e.b)) + ")", "body", kActions};
    case EventType::ActionRefused:
      return Described{nm(e.subject) + ": can't " + (e.a < 0 ? std::string("get there") : process_name(a, e.a)) + " (" +
                       to_string(static_cast<Reason>(e.b)) + ")", "body", kActions};
    case EventType::FellAsleep: return Described{nm(e.subject) + " fell asleep", "body", kBody};
    case EventType::WokeUp: return Described{nm(e.subject) + " woke up", "body", kBody};
    case EventType::NewWound:
      return Described{nm(e.subject) + " got a wound (" + to_string(static_cast<Region>(e.b)) + ")", "dead", kBody};
    case EventType::InfectionVisible: return Described{nm(e.subject) + "'s wound looks infected", "alert", kBody};
    case EventType::WoundHealed: return Described{nm(e.subject) + "'s wound healed", "done", kBody};
    case EventType::Grabbed: return Described{"A walker grabbed " + nm(static_cast<EntityId>(e.a)), "dead", kDanger};
    case EventType::GeneratorStalled: return Described{"The generator stalled", "alert", kWorld};
    case EventType::BreakerTripped: return Described{"A breaker tripped", "alert", kWorld};
    case EventType::PartFailed: return Described{"A part gave way", "alert", kDanger};
    // Spec 59: broken commitments are noticed (decision 0070). Subjects are pop ids, named from the census.
    case EventType::AbsenceBegan:
      return Described{pop_name(a, e.subject) + (e.b ? " called in: away for a while" : " is away"), "world", kWorld};
    case EventType::CommitmentBroken:
      return Described{pop_name(a, e.subject) + " did not keep " + commitment_name(static_cast<CommitmentKind>(e.a)), "alert", kWorld};
    case EventType::HolderPhoned:
      return Described{pop_name(a, e.subject) + " phoned " + pop_name(a, static_cast<PopId>(e.a)) + ": no answer", "world", kWorld};
    case EventType::HolderAskedAround:
      return Described{pop_name(a, e.subject) + " asked around about " + pop_name(a, static_cast<PopId>(e.a)), "world", kWorld};
    case EventType::MissingReported: return Described{pop_name(a, e.subject) + " was reported missing", "alert", kWorld};
    case EventType::SearchStarted: return Described{"A search opened for " + pop_name(a, e.subject), "alert", kWorld};
    case EventType::AbsenceEnded:
      return Described{pop_name(a, e.subject) + " is back (gone " + std::to_string(e.b / kTicksPerGameHour) + " h)", "done", kWorld};
    case EventType::CaseClosed: return Described{"The missing-person case for " + pop_name(a, e.subject) + " was closed", "done", kWorld};
    // Spec 61: the employer's absence rule.
    case EventType::ShiftMissed:
      return Described{pop_name(a, e.subject) + " missed a shift without calling (" + std::to_string(e.a) + " in a row)", "alert", kWorld};
    case EventType::Separated:
      return Described{"The employer recorded " + pop_name(a, e.subject) + " as having left the job (" + std::to_string(e.a) + " days without contact)", "dead", kWorld};
    case EventType::DisciplinaryOpened:
      return Described{"The employer opened a disciplinary process for " + pop_name(a, e.subject) + " (" + std::to_string(e.a) + " days without contact)", "alert", kWorld};
    case EventType::ConductCaseOpened:
      return Described{"A conduct case opened for " + pop_name(a, e.subject) + " (" + std::to_string(e.a) + " shifts missed)", "alert", kWorld};
    // Spec 62: money and labour.
    case EventType::ShiftUnpaid:
      return Described{pop_name(a, e.subject) + "'s shift is not paid: " + std::to_string(e.a / 100) + " withheld", "alert", kWorld};
    case EventType::LeftEmployer: return Described{pop_name(a, e.subject) + " is off the employer's roster: the wages stop", "dead", kWorld};
    // Spec 63: the household after an absence or a lost job.
    case EventType::BondStrained:
      return Described{pop_name(a, e.subject) + " is strained toward " + pop_name(a, static_cast<PopId>(e.a)) + " (" + std::to_string(e.b / 10) + " %)", "alert", kWorld};
    case EventType::LeftHousehold:
      return Described{pop_name(a, e.subject) + " left the household over " + pop_name(a, static_cast<PopId>(e.a)), "dead", kWorld};
    // Spec 64: the police's missing-person case.
    case EventType::PoliceCaseOpened:
      return Described{"A police case opened for " + pop_name(a, e.subject) + " (" + (e.a == 0 ? "high" : e.a == 1 ? "medium" : "low") + " risk" +
                           (e.b > 0 ? ", waiting: number " + std::to_string(e.b) + ")" : ", a search at once)"), "alert", kWorld};
    case EventType::PoliceCaseResolved:
      return Described{"The police case for " + pop_name(a, e.subject) + " is resolved: " +
                           (e.a == 0 ? "returned of their own free will" : e.a == 1 ? "found by the police" : "found by others"), "done", kWorld};
    case EventType::ReturnedToWork:
      return Described{pop_name(a, e.subject) + " met the employer: " + (e.a == 0 ? "forgiven" : e.a == 1 ? "a warning" : "a disciplinary process"), "done", kWorld};
    default: break;
  }
  if (auto l = event_line(a.t, e)) {
    const bool danger = std::strcmp(l->kind, "dead") == 0 || e.type == EventType::Bite || e.type == EventType::EdgeBreached ||
                        e.type == EventType::Panicked;
    const int cat = danger ? kDanger : std::strcmp(l->kind, "body") == 0 ? kBody : std::strcmp(l->kind, "done") == 0 ? kActions : kWorld;
    return Described{l->text, l->kind, cat};
  }
  return std::nullopt;
}

void read_events(App& a) {
  const auto& all = a.t.world->log.all();
  for (; a.seen < all.size(); ++a.seen) {
    const Event& e = all[a.seen];
    // Alerts that pause: a bite on a group member, a group member down.
    if (e.type == EventType::Bite && a.in_group(static_cast<EntityId>(e.a)))
      raise_alert(a, a.name(e.subject) + " bit " + a.name(static_cast<EntityId>(e.a)));
    else if (e.type == EventType::Death && a.in_group(e.subject))
      raise_alert(a, a.name(e.subject) == "You" ? std::string("You are down") : a.name(e.subject) + " is down");
    if (auto l = describe(a, e)) {
      if (a.resting && (std::strcmp(l->kind, "alert") == 0 || std::strcmp(l->kind, "dead") == 0)) a.resting = false;
      // Identical lines within a game minute coalesce as "x N" (spec 16 rule 12).
      if (!a.log.empty() && a.log.back().text == l->text && e.tick - a.log.back().last_tick <= kGameMinute) {
        ++a.log.back().count;
        a.log.back().last_tick = e.tick;
        a.log.back().event = e.id;
      } else {
        a.log.push_back({e.tick, e.tick, l->text, l->kind, l->category, e.subject, e.id});
      }
    }
  }
}

// ---- Simulation pacing ----

// Caps from what the controlled pop is doing (spec 40 rule 10): walking holds 3x, a walker within reach holds 1:1.
void set_caps(App& a, bool wasd_held) {
  const std::int64_t now = a.t.world->clock.tick;
  const EntityId me = a.me();
  bool walking = false, fight = false;
  if (me && a.t.bodies->get(me).alive) {
    const Actor& act = a.t.actors->actor(me);
    walking = wasd_held || act.action.phase == Phase::Approach;
    if (a.t.walkers)
      for (const auto& [id, z] : a.t.walkers->walkers()) {
        if (!a.t.bodies->get(id).alive || z.pos.z != act.pos.z) continue;
        const bool near = std::max(std::abs(z.pos.x - act.pos.x), std::abs(z.pos.y - act.pos.y)) <= 1;
        if (a.t.walkers->in_reach(z.pos, act.pos) || (near && z.target == me)) fight = true;
      }
  }
  if (walking) a.pacer.cap("walking", Pace::X3, now);
  else a.pacer.release("walking", now);
  if (fight) a.pacer.cap("fight", Pace::Real, now);
  else a.pacer.release("fight", now);
}

void advance(App& a, double real_dt) {
  World& w = *a.t.world;
  const bool over = a.t.run->outcome() != RunOutcome::Running;
  if (over) a.resting = false;
  if (a.resting) {
    // As fast as the frame allows, until an alert or eight game hours.
    for (int i = 0; i < 20000 && a.resting; ++i) {
      w.step();
      if (w.clock.tick >= a.rest_until) a.resting = false;
      if ((i & 63) == 0) {
        read_events(a);
        check_sight(a);
      }
    }
    read_events(a);
    return;
  }
  if (over) return;
  const std::int64_t n = std::min<std::int64_t>(a.pacer.ticks_for(real_dt, w.clock.tick), 20000);
  if (n > 0) w.jump(w.clock.tick + n);
  read_events(a);
  check_sight(a);
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

// Paul Tol's "bright" palette: blue, cyan, yellow, purple (spec 16 rule 9). Red stays for the dead.
const ImU32 kPopColours[] = {rgb(68, 119, 170), rgb(102, 204, 238), rgb(204, 187, 68), rgb(170, 51, 119)};
const ImU32 kWalkerIdle = rgb(150, 50, 50), kWalkerActive = rgb(238, 102, 119);

// Two- or three-letter labels for map objects, explained in the Map key.
struct ObjectLabel {
  const char* kind;
  const char* label;
};
const ObjectLabel kObjectLabels[] = {{"fridge", "Fr"},   {"generator", "Gen"}, {"home", "Hm"},     {"hot_plate", "HP"},
                                     {"lamp", "Lmp"},    {"pantry", "Pan"},    {"power_panel", "PP"}, {"radio", "Rad"},
                                     {"saw", "Saw"},     {"sink", "Snk"},      {"stockpile", "Stk"}, {"stove", "Stv"}};

std::string object_label(const std::string& kind) {
  for (const ObjectLabel& l : kObjectLabels)
    if (kind == l.kind) return l.label;
  std::string s = kind.substr(0, 3);
  if (!s.empty()) s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
  return s;
}

bool drawn_object(const MapObject& o) { return o.kind != "pop_start" && o.kind != "found" && o.kind != "item"; }

std::string initials(const std::string& n) {
  std::string out;
  bool next = true;
  for (char c : n) {
    if (next && c != ' ') out += c;
    next = c == ' ';
  }
  return out.substr(0, 2);
}

// The worst states of a living pop, worst first (spec 16 rule 10). Sickness only from visible symptoms.
struct State {
  const char* name;
  double v;
};

std::vector<State> worst_states(const App& a, EntityId id) {
  std::vector<State> s;
  const Body& b = a.t.bodies->get(id);
  if (!b.alive || b.spec.walker) return s;
  const Needs n = a.t.bodies->needs(b);
  const std::int64_t now = a.t.world->clock.tick;
  bool sick = false;
  for (const Wound& wd : b.wounds) sick = sick || wd.infection_visible;
  const State all[] = {{"thirst", n.thirst},
                       {"hunger", n.hunger},
                       {"fatigue", std::max(n.tiredness, n.exhaustion)},
                       {"pain", a.t.bodies->pain(b, now)},
                       {"bleeding", std::min(1.0, a.t.bodies->blood_loss_fraction(b) / 0.3)},  // 30 % lost = severe (estimate)
                       {"fear", b.fear},
                       {"sick", sick ? kBad : 0.0}};
  for (const State& st : all)
    if (st.v >= kMild) s.push_back(st);
  std::stable_sort(s.begin(), s.end(), [](const State& x, const State& y) { return x.v > y.v; });
  if (s.size() > 5) s.resize(5);
  return s;
}

const char* band_word(double v) { return v >= kSevere ? "severe" : v >= kBad ? "bad" : v >= kMild ? "mild" : "fine"; }
ImU32 band_colour(double v) {
  return v >= kSevere ? rgb(238, 102, 119) : v >= kBad ? rgb(238, 153, 51) : v >= kMild ? rgb(204, 187, 68) : rgb(120, 190, 120);
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
  const double outdoor = Power::ambient_lux(w.clock.hour());
  a.noise_any = a.light_any = false;
  for (int y = 0; y < m.height(); ++y)
    for (int x = 0; x < m.width(); ++x) {
      const Cell c{a.level, x, y};
      if (!m.walkable(c)) continue;
      const double lux = a.t.power->lux_at(w, c);
      a.lux[static_cast<std::size_t>(y * m.width() + x)] = lux;
      if (noise_at(m, a.noise, c) > a.background) a.noise_any = true;
      if (lux > outdoor * 1.05 + 1) a.light_any = true;
    }
}

// What the followed pop hears of the noise sources (spec 58), kept while its cell and the key stay the same: the key
// changes every ten game seconds, so a mark's error holds still for a while instead of jittering every frame.
void refresh_marks(App& a) {
  const World& w = *a.t.world;
  const EntityId id = a.followed();
  if (!id || !a.t.actors->actors().contains(id) || !a.t.bodies->get(id).alive) {
    a.marks.clear();
    a.echoes.clear();
    a.marks_key = -1;
    a.marks_id = 0;
    return;
  }
  if (id != a.marks_id) {  // another pop's ears: what the last one heard is not this one's memory
    a.echoes.clear();
    a.marks_id = id;
    a.marks_key = -1;
  }
  const Cell here = a.t.actors->actor(id).pos;
  const std::int64_t key = w.clock.tick / 20;
  if (key == a.marks_key && here == a.marks_cell) return;
  a.marks_key = key;
  a.marks_cell = here;
  a.marks = sound_marks(*a.t.map, *a.t.items, here, noise_sources(w, *a.t.actors, *a.t.power),
                        background_db(a.t.power->mains_alive(), w.clock.hour()), static_cast<std::uint64_t>(key));
  a.echoes.hear(a.marks, w.clock.tick);  // what is no longer heard stays a while as an echo
}

ImVec2 at(const App& a, double x, double y) {
  return {a.origin.x + static_cast<float>(x) * a.cell, a.origin.y + static_cast<float>(y) * a.cell};
}

ImVec2 centred(ImVec2 c, const char* text) {
  const ImVec2 sz = ImGui::CalcTextSize(text);
  return {c.x - sz.x / 2, c.y - sz.y / 2};
}

// What a pop is doing, for its action bar: the name, and the fraction done while at work (spec 16 rule 14).
struct ActionView {
  std::string name;
  double done = -1;  // -1: approaching, no bar
  double left_s = 0;
};

std::optional<ActionView> doing_of(const App& a, EntityId id) {
  if (!a.t.actors->actors().contains(id)) return std::nullopt;
  const Action& act = a.t.actors->actor(id).action;
  if (act.phase == Phase::Idle || act.process < 0) return std::nullopt;
  ActionView d{a.t.actors->processes()[static_cast<std::size_t>(act.process)].name};
  if (act.phase == Phase::Work && act.total_s > 0) {
    d.done = std::clamp(act.progress_s / act.total_s, 0.0, 1.0);
    d.left_s = std::max(0.0, act.total_s - act.progress_s);
  }
  return d;
}

std::string duration_text(double s) {
  char buf[32];
  if (s < 60) std::snprintf(buf, sizeof buf, "%.0f s", s);
  else if (s < 3600) std::snprintf(buf, sizeof buf, "%.0f min", s / 60);
  else std::snprintf(buf, sizeof buf, "%.1f h", s / 3600);
  return buf;
}

void draw_action_bar(const App& a, ImDrawList* d, EntityId id, ImVec2 c, float r) {
  const auto doing = doing_of(a, id);
  if (!doing) return;
  const bool me = id == a.me();
  const float w = me ? std::max(70.0f, a.cell * 2.2f) : std::max(24.0f, a.cell * 0.9f), h = me ? 7.0f : 4.0f;
  const ImVec2 p0{c.x - w / 2, c.y - r - h - 6}, p1{c.x + w / 2, c.y - r - 6};
  if (doing->done >= 0) {
    d->AddRectFilled({p0.x - 1, p0.y - 1}, {p1.x + 1, p1.y + 1}, rgb(15, 15, 18, 220));
    d->AddRectFilled(p0, {p0.x + w * static_cast<float>(doing->done), p1.y}, rgb(120, 200, 120));
  }
  if (!me) return;
  const std::string label = doing->done >= 0 ? doing->name + ", " + duration_text(doing->left_s) + " left" : "-> " + doing->name;
  const ImVec2 sz = ImGui::CalcTextSize(label.c_str());
  const ImVec2 q{c.x - sz.x / 2, p0.y - sz.y - 2};
  d->AddRectFilled({q.x - 3, q.y - 1}, {q.x + sz.x + 3, q.y + sz.y + 1}, rgb(15, 15, 18, 200));
  d->AddText(q, rgb(235, 235, 235), label.c_str());
}

struct DrawnBody {
  EntityId id;
  V2 p;
  Cell cell;
  bool walker;
  std::size_t index;  // into the group, for pops
};

void draw_map(App& a, ImDrawList* d) {
  const SiteMap& m = *a.t.map;
  const World& w = *a.t.world;
  refresh_fields(a);
  d->PushClipRect(a.view_min, a.view_max, true);
  const double outdoor = Power::ambient_lux(w.clock.hour());
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
      } else if (a.overlay == Overlay::Light && lux > outdoor * 1.05 + 1) {
        // Only cells brighter than outdoors, so the light that leaks at night shows (spec 16 rule 12).
        d->AddRectFilled(at(a, x, y), at(a, x + 1, y + 1), rgb(250, 230, 120, static_cast<int>(std::min(180.0, 40 * std::log10(lux)))));
      }
    }
  // Noise contours (spec 58): the line where a sound is just noticed (0 dB over the background) and where it is placed
  // (+10 dB), between neighbouring cells either side of the threshold.
  if (a.overlay == Overlay::Noise) {
    const auto margin_at = [&](int x, int y) { return noise_at(m, a.noise, {a.level, x, y}) - a.background; };
    const auto contour = [&](double threshold, ImU32 col, float thick) {
      for (int y = 0; y < m.height(); ++y)
        for (int x = 0; x < m.width(); ++x) {
          if (!m.walkable({a.level, x, y})) continue;
          const bool here = margin_at(x, y) >= threshold;
          if (x + 1 < m.width() && m.walkable({a.level, x + 1, y}) && here != (margin_at(x + 1, y) >= threshold))
            d->AddLine(at(a, x + 1, y), at(a, x + 1, y + 1), col, thick);  // between this cell and the one to the east
          if (y + 1 < m.height() && m.walkable({a.level, x, y + 1}) && here != (margin_at(x, y + 1) >= threshold))
            d->AddLine(at(a, x, y + 1), at(a, x + 1, y + 1), col, thick);  // between this cell and the one to the south
        }
    };
    contour(kNoticeDb, rgb(250, 220, 90), 1.5f);
    contour(kPlaceDb, rgb(255, 255, 255), 2.5f);
  }
  // Map objects: outlined squares with a short label (spec 16 rule 9); item spawn markers are not drawn.
  for (const MapObject& o : m.objects()) {
    if (o.at.z != a.level || !drawn_object(o)) continue;
    const ImVec2 p0 = at(a, o.at.x + 0.12, o.at.y + 0.12), p1 = at(a, o.at.x + 0.88, o.at.y + 0.88);
    d->AddRectFilled(p0, p1, rgb(50, 50, 60));
    d->AddRect(p0, p1, rgb(200, 200, 210), 0.0f, 0, 1.5f);
    if (a.cell >= 22) {
      const std::string l = object_label(o.kind);
      d->AddText(centred(at(a, o.at.x + 0.5, o.at.y + 0.5), l.c_str()), rgb(235, 235, 235), l.c_str());
    }
  }
  // Items on the floor.
  for (const auto& [id, inst] : a.t.items->all()) {
    const auto c = a.t.actors->floor_cell(w.locations.parent_of(id));
    if (c && c->z == a.level) d->AddCircleFilled(at(a, c->x + 0.8, c->y + 0.8), std::max(2.5f, a.cell * 0.1f), rgb(240, 220, 120));
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
  // Edges that refused a WASD step flash (spec 16 rule 5).
  if (a.real_s < a.flash_until && std::fmod(a.flash_until - a.real_s, 0.2) < 0.12)
    for (const EdgeRef& e : a.flash)
      if (e.z == a.level)
        d->AddLine(at(a, e.x, e.y), e.vertical ? at(a, e.x, e.y + 1) : at(a, e.x + 1, e.y), rgb(255, 80, 80), 6);
  // The controlled pop's path, dotted (spec 16 rule 12).
  if (const EntityId me = a.me(); me && a.t.actors->actors().contains(me)) {
    const Action& act = a.t.actors->actor(me).action;
    if (act.phase == Phase::Approach)
      for (std::size_t i = act.path_next; i < act.path.size(); ++i)
        if (act.path[i].z == a.level) d->AddCircleFilled(at(a, act.path[i].x + 0.5, act.path[i].y + 0.5), 2.5f, rgb(255, 255, 255, 170));
  }
  // Bodies: walkers as diamonds, pops as discs (spec 16 rule 9). Bodies sharing a cell are drawn apart.
  std::vector<DrawnBody> bodies;
  if (a.t.walkers)
    for (const auto& [id, z] : a.t.walkers->walkers()) {
      if (!a.t.bodies->get(id).alive || z.pos.z != a.level) continue;
      bodies.push_back({id, body_at(z.pos, z.path, z.next, z.step_m, z.mode != Mode::Idle), z.pos, true, 0});
    }
  for (std::size_t i = 0; i < a.t.group.size(); ++i) {
    const EntityId id = a.t.group[i];
    const Body& b = a.t.bodies->get(id);
    if (b.away || b.spec.walker || !a.t.actors->actors().contains(id)) continue;
    const Cell p = a.t.actors->actor(id).pos;
    if (p.z != a.level) continue;
    bodies.push_back({id, actor_at(a, id), p, false, i});
  }
  static const V2 kSpread[] = {{-0.2, -0.2}, {0.2, -0.2}, {-0.2, 0.2}, {0.2, 0.2}};
  const float r = a.cell * 0.42f;
  for (std::size_t k = 0; k < bodies.size(); ++k) {
    DrawnBody& db = bodies[k];
    std::size_t same = 0, before = 0;
    for (std::size_t j = 0; j < bodies.size(); ++j)
      if (bodies[j].cell == db.cell) {
        ++same;
        if (j < k) ++before;
      }
    const float rr = same > 1 ? r * 0.62f : r;
    const V2 off = same > 1 ? kSpread[before % 4] : V2{};
    const ImVec2 c = at(a, db.p.x + off.x, db.p.y + off.y);
    if (same > 4 && before == 4) {
      const std::string more = "+" + std::to_string(same - 4);
      d->AddText(centred(at(a, db.cell.x + 0.5, db.cell.y + 0.5), more.c_str()), rgb(255, 255, 255), more.c_str());
    }
    if (same > 4 && before >= 4) continue;
    if (db.walker) {
      const WalkerState& z = a.t.walkers->walkers().at(db.id);
      const bool active = z.mode != Mode::Idle;
      if (active && z.goal.z == a.level)
        d->AddLine(c, at(a, z.goal.x + 0.5, z.goal.y + 0.5), rgb(238, 102, 119, 140), 1.5f);
      d->AddQuadFilled({c.x, c.y - rr}, {c.x + rr, c.y}, {c.x, c.y + rr}, {c.x - rr, c.y}, active ? kWalkerActive : kWalkerIdle);
      d->AddQuad({c.x, c.y - rr}, {c.x + rr, c.y}, {c.x, c.y + rr}, {c.x - rr, c.y}, rgb(30, 10, 10), 1.5f);
      if (db.id == a.selected) d->AddCircle(c, rr + 5, rgb(255, 230, 120), 0, 2);
      continue;
    }
    const Body& b = a.t.bodies->get(db.id);
    d->AddCircleFilled(c, rr, b.alive ? kPopColours[db.index % 4] : rgb(110, 110, 110));
    d->AddCircle(c, rr, rgb(20, 20, 20), 0, 1.5f);
    std::string ini = initials(a.name(db.id));
    if (2 * rr < 24) ini = ini.substr(0, 1);
    d->AddText(centred(c, ini.c_str()), rgb(15, 15, 15), ini.c_str());
    if (db.id == a.me()) {
      d->AddCircle(c, rr + 2, rgb(255, 255, 255), 0, 3.5f);
      // The arrow: where it walks, or where WASD last pointed.
      const float len = std::hypot(static_cast<float>(a.face_dx), static_cast<float>(a.face_dy));
      if (len > 0) {
        const float ux = a.face_dx / len, uy = a.face_dy / len;
        const ImVec2 tip{c.x + ux * (rr + 10), c.y + uy * (rr + 10)};
        d->AddTriangleFilled(tip, {c.x + ux * (rr + 3) - uy * 5, c.y + uy * (rr + 3) + ux * 5},
                             {c.x + ux * (rr + 3) + uy * 5, c.y + uy * (rr + 3) - ux * 5}, rgb(255, 255, 255));
      }
    }
    if (db.id == a.selected && db.id != a.me()) d->AddCircle(c, rr + 5, rgb(255, 230, 120), 0, 2);
    else if (db.id == a.selected) d->AddCircle(c, rr + 7, rgb(255, 230, 120), 0, 1.5f);
    draw_action_bar(a, d, db.id, c, rr);
  }
  // What the followed pop hears (spec 58): a placed sound is rings at the place it seems to come from; a noticed one is
  // only a cone on the pop's ring toward the direction, wide as the error. What stopped sounding a moment ago stays as
  // an echo (one thin ring or a thin arc) that fades. Teal, never red: red is the noise overlay.
  refresh_marks(a);
  if (const EntityId id = a.followed(); id && a.marks_cell.z == a.level) {
    const V2 pp = actor_at(a, id);
    const ImVec2 pc = at(a, pp.x + 0.5, pp.y + 0.5);
    const float ring = a.cell * 1.45f;
    const auto teal = [](double k) { return rgb(80, 205, 225, static_cast<int>(255 * k)); };
    const auto draw_mark = [&](const SoundMark& mk, double k, bool echo) {
      if (mk.kind == SoundMark::Kind::Placed) {
        if (mk.apparent.z != a.level) return;
        const ImVec2 c = at(a, mk.apparent.x + 0.5, mk.apparent.y + 0.5);
        if (echo) {
          d->AddCircle(c, a.cell * 0.55f, teal(k * 0.7), 0, 1.0f);
        } else {
          for (float r : {0.3f, 0.55f, 0.8f}) d->AddCircle(c, a.cell * r, teal(r < 0.5f ? 1.0 : 0.35), 0, 2.0f);
          if (a.cell >= 22) {
            char buf[24];
            std::snprintf(buf, sizeof buf, "~%.0f m", mk.distance_m);
            d->AddText({c.x + a.cell * 0.85f, c.y - a.cell * 0.3f}, teal(1), buf);
          }
        }
      } else {
        // The cone: from the bearing minus the error to plus the error, then an arrowhead at the bearing itself.
        constexpr float kErr = 0.5f;
        const float b = static_cast<float>(mk.bearing);
        d->PathClear();
        for (int i = -4; i <= 4; ++i) d->PathLineTo({pc.x + std::cos(b + kErr * i / 4) * ring, pc.y - std::sin(b + kErr * i / 4) * ring});
        d->PathStroke(teal(echo ? k * 0.4 : 0.35), 0, echo ? 2.0f : 5.0f);
        if (echo) return;
        const ImVec2 tip{pc.x + std::cos(b) * (ring + 12), pc.y - std::sin(b) * (ring + 12)};
        const ImVec2 base{pc.x + std::cos(b) * (ring + 2), pc.y - std::sin(b) * (ring + 2)};
        const ImVec2 side{-std::sin(b) * -5, -std::cos(b) * -5};
        d->AddTriangleFilled(tip, {base.x + side.x, base.y + side.y}, {base.x - side.x, base.y - side.y}, teal(1));
      }
    };
    for (const SoundEcho& e : a.echoes.echoes(w.clock.tick)) draw_mark(e.mark, e.strength(w.clock.tick), true);
    for (const SoundMark& mk : a.marks) draw_mark(mk, 1, false);
  }
  // The state column by the controlled pop (spec 16 rule 10).
  if (const EntityId me = a.me(); me && a.t.bodies->get(me).alive && a.t.actors->actor(me).pos.z == a.level) {
    const V2 p = actor_at(a, me);
    ImVec2 q = at(a, p.x + 0.75, p.y - 0.5);
    for (const State& s : worst_states(a, me)) {
      const std::string txt = std::string(s.name) + " (" + band_word(s.v) + ")";
      const ImVec2 sz = ImGui::CalcTextSize(txt.c_str());
      d->AddRectFilled({q.x - 2, q.y}, {q.x + sz.x + 2, q.y + sz.y}, rgb(15, 15, 18, 200));
      d->AddText(q, band_colour(s.v), txt.c_str());
      q.y += sz.y + 1;
    }
  }
  // The hovered cell and the menu's cell, outlined (spec 16 rule 12).
  if (a.hover_cell) d->AddRect(at(a, a.hover_cell->x, a.hover_cell->y), at(a, a.hover_cell->x + 1, a.hover_cell->y + 1), rgb(255, 255, 255, 160), 0.0f, 0, 1.0f);
  if (a.menu_cell) d->AddRect(at(a, a.menu_cell->x, a.menu_cell->y), at(a, a.menu_cell->x + 1, a.menu_cell->y + 1), rgb(255, 230, 120), 0.0f, 0, 2.0f);
  d->PopClipRect();
}

// Overlay legend and the alert column, over the map's edges (spec 16 rules 11, 12).
void map_notes(App& a, ImDrawList* d) {
  const World& w = *a.t.world;
  ImVec2 q{a.view_min.x + 6, a.view_max.y - 22};
  auto note = [&](ImVec2 p, ImU32 col, const std::string& s) {
    const ImVec2 sz = ImGui::CalcTextSize(s.c_str());
    d->AddRectFilled({p.x - 3, p.y - 2}, {p.x + sz.x + 3, p.y + sz.y + 2}, rgb(15, 15, 18, 210));
    d->AddText(p, col, s.c_str());
    return sz;
  };
  char buf[200];
  if (a.overlay == Overlay::Noise) {
    std::snprintf(buf, sizeof buf, a.noise_any ? "Noise: red = louder than the background (%.0f dB); full red = 30 dB above"
                                               : "Noise: nothing above the background (%.0f dB)", a.background);
    note(q, rgb(240, 160, 150), buf);
  } else if (a.overlay == Overlay::Light) {
    std::snprintf(buf, sizeof buf, a.light_any ? "Light: yellow = brighter than outdoors (%.0f lux); this light leaks"
                                               : "Light: nothing brighter than outdoors (%.0f lux)", Power::ambient_lux(w.clock.hour()));
    note(q, rgb(250, 230, 140), buf);
  }
  // What the followed pop hears, in words: the loudest first, direction and how sure (spec 58), above the overlay note.
  {
    ImVec2 h{q.x, q.y - (a.overlay == Overlay::Noise || a.overlay == Overlay::Light ? 26.0f : 0.0f)};
    std::vector<const SoundMark*> heard;
    for (const SoundMark& mk : a.marks) heard.push_back(&mk);
    std::sort(heard.begin(), heard.end(), [](const SoundMark* x, const SoundMark* y) { return x->over_db > y->over_db; });
    for (std::size_t i = 0; i < heard.size() && i < 3; ++i) {
      const SoundMark& mk = *heard[i];
      if (mk.kind == SoundMark::Kind::Placed)
        std::snprintf(buf, sizeof buf, "You hear a clear sound, %s, about %.0f m%s", compass_word(mk.bearing), mk.distance_m,
                      mk.apparent.z != a.marks_cell.z ? (mk.apparent.z > a.marks_cell.z ? " (above)" : " (below)") : "");
      else
        std::snprintf(buf, sizeof buf, "You hear something faint, toward the %s", compass_word(mk.bearing));
      note(h, rgb(120, 215, 230), buf);
      h.y -= 26;
    }
    int shown_echoes = 0;
    for (const SoundEcho& e : a.echoes.echoes(w.clock.tick)) {
      if (shown_echoes++ >= 2) break;
      std::snprintf(buf, sizeof buf, "You heard %s, %s, a moment ago", e.mark.kind == SoundMark::Kind::Placed ? "a clear sound" : "something faint",
                    compass_word(e.mark.bearing));
      note(h, rgb(110, 150, 160), buf);
      h.y -= 26;
    }
  }
  if (a.real_s < a.hint_until) {
    const char* hint = "Paused: Space to run";
    note({(a.view_min.x + a.view_max.x) / 2 - ImGui::CalcTextSize(hint).x / 2, a.view_min.y + 8}, rgb(255, 230, 120), hint);
  }
  // Alerts of the last game hour, newest first.
  float y = a.view_min.y + 8;
  int shown = 0;
  for (auto it = a.alerts.rbegin(); it != a.alerts.rend() && shown < 5; ++it, ++shown) {
    if (w.clock.tick - it->tick > 60 * kGameMinute) break;
    const std::string s = time_text(it->tick).substr(time_text(it->tick).size() - 5) + "  " + it->text;
    const float wdt = ImGui::CalcTextSize(s.c_str()).x;
    note({a.view_max.x - wdt - 10, y}, shown == 0 && w.clock.tick - it->tick < 2 * kGameMinute ? rgb(255, 120, 120) : rgb(230, 200, 160), s);
    y += 20;
  }
}

// ---- Camera (spec 16 rule 7) ----

void clamp_cam(App& a) {
  const double hw = (a.view_max.x - a.view_min.x) / 2.0 / a.cell, hh = (a.view_max.y - a.view_min.y) / 2.0 / a.cell;
  const double mw = a.t.map->width(), mh = a.t.map->height();
  a.cam.x = mw <= 2 * hw ? mw / 2 : std::clamp(a.cam.x, hw, mw - hw);
  a.cam.y = mh <= 2 * hh ? mh / 2 : std::clamp(a.cam.y, hh, mh - hh);
}

void update_camera(App& a, double dt) {
  if (a.follow && a.t.actors->actors().contains(a.followed())) {
    const V2 p = actor_at(a, a.followed());
    a.level = a.t.actors->actor(a.followed()).pos.z;  // the drawn floor follows the pop
    const double hw = (a.view_max.x - a.view_min.x) / 2.0 / a.cell, hh = (a.view_max.y - a.view_min.y) / 2.0 / a.cell;
    const double k = 1 - std::exp(-dt / kFollowTau);
    const auto pull = [&](double off, double half) {
      const double dz = kDeadZone * half;
      return off > dz ? off - dz : off < -dz ? off + dz : 0.0;
    };
    a.cam.x += pull(p.x - a.cam.x, hw) * k;
    a.cam.y += pull(p.y - a.cam.y, hh) * k;
  }
  clamp_cam(a);
  const ImVec2 mid{(a.view_min.x + a.view_max.x) / 2, (a.view_min.y + a.view_max.y) / 2};
  a.origin = {mid.x - static_cast<float>(a.cam.x) * a.cell, mid.y - static_cast<float>(a.cam.y) * a.cell};
}

void recentre(App& a) {
  a.follow = true;
  a.cam = actor_at(a, a.followed());
  a.level = a.t.actors->actor(a.followed()).pos.z;
}

// ---- Input on the map ----

std::optional<Cell> cell_under(const App& a, ImVec2 mouse) {
  if (mouse.x < a.view_min.x || mouse.y < a.view_min.y || mouse.x >= a.view_max.x || mouse.y >= a.view_max.y) return std::nullopt;
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
      if (z.pos == c && a.t.bodies->get(id).alive) out.push_back({id, "a walker (" + a.name(id) + ")"});
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

// The Walk heading (spec 50: 0-7 clockwise from north) for a key direction, or -1.
int heading_of(int dx, int dy) {
  for (int h = 0; h < 8; ++h) {
    int x = 0, y = 0;
    heading_step(h, x, y);
    if (x == dx && y == dy) return h;
  }
  return -1;
}

// The pop stands against something in its heading: flash what stopped it (spec 16 rule 5). The rule itself is the
// simulation's (SiteMap::steer); the window only shows it.
void flash_block(App& a, const Cell& p, int dx, int dy) {
  if (a.t.map->steer(p, dx, dy) || a.real_s < a.flash_until) return;
  const Cell ox{p.z, p.x + dx, p.y}, oy{p.z, p.x, p.y + dy};
  a.flash.clear();
  if (dx) a.flash.push_back(SiteMap::edge_between(p, ox));
  if (dy) a.flash.push_back(SiteMap::edge_between(p, oy));
  a.flash_until = a.real_s + 0.6;
}

void set_speed(App& a, Pace p) {
  a.pacer.set_requested(p);
  if (p != Pace::Pause) a.resume = p;
  a.resting = false;
}

void map_input(App& a) {
  const ImGuiIO& io = ImGui::GetIO();
  World& w = *a.t.world;
  a.hover_cell.reset();
  if (!io.WantCaptureMouse) {
    const ImVec2 mid{(a.view_min.x + a.view_max.x) / 2, (a.view_min.y + a.view_max.y) / 2};
    if (io.MouseWheel != 0) {
      // Zoom to the cursor: the cell under it stays under it.
      const double px = a.cam.x + (io.MousePos.x - mid.x) / a.cell, py = a.cam.y + (io.MousePos.y - mid.y) / a.cell;
      a.cell = std::clamp(a.cell * (io.MouseWheel > 0 ? 1.15f : 1 / 1.15f), 8.0f, 96.0f);
      a.cam = {px - (io.MousePos.x - mid.x) / a.cell, py - (io.MousePos.y - mid.y) / a.cell};
    }
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
      a.cam.x -= io.MouseDelta.x / a.cell;
      a.cam.y -= io.MouseDelta.y / a.cell;
      a.follow = false;  // a pan breaks the follow; F, Home or WASD resume it
    }
    if (const auto c = cell_under(a, io.MousePos)) {
      a.hover_cell = c;
      if (!ImGui::IsPopupOpen("offers")) {
        // What is here, in words.
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
  if (io.WantCaptureKeyboard) {
    set_caps(a, false);
    return;
  }
  int dx = 0, dy = 0;
  if (ImGui::IsKeyDown(ImGuiKey_W)) dy -= 1;
  if (ImGui::IsKeyDown(ImGuiKey_S)) dy += 1;
  if (ImGui::IsKeyDown(ImGuiKey_A)) dx -= 1;
  if (ImGui::IsKeyDown(ImGuiKey_D)) dx += 1;
  const bool pressed = ImGui::IsKeyPressed(ImGuiKey_W, false) || ImGui::IsKeyPressed(ImGuiKey_A, false) ||
                       ImGui::IsKeyPressed(ImGuiKey_S, false) || ImGui::IsKeyPressed(ImGuiKey_D, false);
  const EntityId me = a.me();
  const bool held = (dx || dy) && me && a.t.bodies->get(me).alive;
  set_caps(a, held);
  // WASD sends a Walk heading when the keys change, and -1 when they are released (spec 50 rule 5): the simulation
  // walks cell to cell without stopping and takes over from any other action. One command per applied tick.
  if (me && a.t.actors->actors().contains(me) && a.t.bodies->get(me).alive) {
    const Actor& act = a.t.actors->actor(me);
    const int want = held ? heading_of(dx, dy) : -1;
    if (want != act.heading && w.clock.tick > a.last_move_tick) {
      submit(a, CommandType::Walk, want);
      a.last_move_tick = w.clock.tick;
    }
    if (held && act.heading == want && act.action.phase == Phase::Idle) flash_block(a, act.pos, dx, dy);
  }
  if (held) {
    if (!a.follow) recentre(a);  // any WASD press resumes the follow
    a.face_dx = dx;
    a.face_dy = dy;
    if (pressed && !a.resting && a.pacer.effective(w.clock.tick) == Pace::Pause) a.hint_until = a.real_s + 2.5;
  } else if (me && a.t.actors->actors().contains(me)) {
    const Action& act = a.t.actors->actor(me).action;
    if (act.phase == Phase::Approach && act.path_next < act.path.size()) {
      const Cell n = act.path[act.path_next], p = a.t.actors->actor(me).pos;
      if (n.z == p.z && (n.x != p.x || n.y != p.y)) {
        a.face_dx = n.x - p.x;
        a.face_dy = n.y - p.y;
      }
    }
  }
  if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
    if (a.pacer.alerted()) a.pacer.acknowledge();  // an alert waits for Space
    else set_speed(a, a.pacer.requested() == Pace::Pause ? a.resume : Pace::Pause);
    a.resting = false;
  }
  for (int i = 0; i < 6; ++i)
    if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(ImGuiKey_1 + i), false)) set_speed(a, kSpeeds[i + 1]);
  if (ImGui::IsKeyPressed(ImGuiKey_F, false) || ImGui::IsKeyPressed(ImGuiKey_Home, false)) recentre(a);
  const auto& lv = a.t.map->levels();
  if (ImGui::IsKeyPressed(ImGuiKey_PageUp) || ImGui::IsKeyPressed(ImGuiKey_RightBracket)) {
    auto it = std::find(lv.begin(), lv.end(), a.level);
    if (it != lv.end() && std::next(it) != lv.end()) a.level = *std::next(it);
    a.follow = false;
  }
  if (ImGui::IsKeyPressed(ImGuiKey_PageDown) || ImGui::IsKeyPressed(ImGuiKey_LeftBracket)) {
    auto it = std::find(lv.begin(), lv.end(), a.level);
    if (it != lv.end() && it != lv.begin()) a.level = *std::prev(it);
    a.follow = false;
  }
  if (ImGui::IsKeyPressed(ImGuiKey_C, false)) submit(a, CommandType::Cancel, 0);
  if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) save_game(a);
  if (ImGui::IsKeyPressed(ImGuiKey_F9, false)) load_game(a);
}

// ---- Panels ----

// What a speed means, in words (spec 16 rule 4).
const char* pace_meaning(Pace p) {
  switch (p) {
    case Pace::Real: return "real time: one second is one second";
    case Pace::X3: return "3x: one real second is three game seconds";
    case Pace::X10: return "10x: one real second is ten game seconds";
    case Pace::DayPerHour: return "one real hour is one game day (24x)";
    case Pace::X60: return "60x: one real second is a game minute";
    case Pace::X360: return "360x: one real second is six game minutes";
    default: return pace_name(p);
  }
}

std::string run_time(std::int64_t ticks) {
  const std::int64_t s = static_cast<std::int64_t>(static_cast<double>(ticks) * kTickSeconds);
  char buf[48];
  std::snprintf(buf, sizeof buf, "%lld d %02lld:%02lld", static_cast<long long>(s / 86400), static_cast<long long>(s / 3600 % 24),
                static_cast<long long>(s / 60 % 60));
  return buf;
}

void time_bar(App& a) {
  const World& w = *a.t.world;
  const Power& p = *a.t.power;
  const std::int64_t now = w.clock.tick;
  ImGui::SetNextWindowPos({8, 8}, ImGuiCond_FirstUseEver);
  ImGui::Begin("Time", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
  // 1. The state, in one plain line: is time running, how fast, and why it is not.
  const Pace eff = a.pacer.effective(now);
  if (a.pacer.alerted()) {
    ImGui::TextColored({1, 0.45f, 0.45f, 1}, "PAUSED by an alert: %s.  Press Space to continue.", a.pacer.alert_text().c_str());
  } else if (a.resting) {
    ImGui::TextColored({0.6f, 0.8f, 1, 1}, "RESTING: skipping time until something happens or %s.", time_text(a.rest_until).c_str());
  } else if (eff == Pace::Pause) {
    ImGui::TextColored({1, 0.85f, 0.4f, 1}, "PAUSED.  Press Space (or the button) to continue.");
  } else if (const std::string* who = a.pacer.limiting(now)) {
    ImGui::TextColored({0.55f, 0.9f, 0.55f, 1}, "RUNNING at %s - slowed from %s while \"%s\".", pace_meaning(eff), pace_name(a.pacer.requested()),
                       who->c_str());
  } else {
    ImGui::TextColored({0.55f, 0.9f, 0.55f, 1}, "RUNNING at %s.", pace_meaning(eff));
  }
  // 2. The clocks: the game's date and hour, how long the run has lasted, how long you have played.
  const int played = static_cast<int>(a.real_s);
  const CivilTime civil = civil_now(w);  // spec 53 rule 10
  std::string day_note;
  if (const CalendarSystem* cal = a.t.calendar) {
    const CountryCalendar& cc = cal->calendar();
    const DayKind kind = cc.day_kind(civil.day_number, Sector::Official);
    if (const Holiday* h = cc.holiday_on(civil.day_number)) day_note = std::string(h->half ? "  |  Half holiday: " : "  |  Holiday: ") + h->name;
    else if (kind == DayKind::Rest) day_note = "  |  Rest day";
    else if (kind == DayKind::ShortDay) day_note = "  |  Short day";
    day_note += cc.is_school_day(civil.day_number) ? "  |  schools open" : "  |  schools closed";
  }
  ImGui::Text("Now: %s (%s)%s   |   run so far: %s   |   played: %d:%02d", format_civil(civil).c_str(), time_text(now).c_str(),
              day_note.c_str(), run_time(now).c_str(), played / 60, played % 60);
  // 3. The controls.
  const bool running = a.pacer.requested() != Pace::Pause || a.resting;
  if (ImGui::Button(a.pacer.alerted() ? "Continue (Space)" : running ? "Pause (Space)" : "Continue (Space)", {130, 0})) {
    if (a.pacer.alerted()) a.pacer.acknowledge();
    else set_speed(a, running ? Pace::Pause : a.resume);
    a.resting = false;
  }
  ImGui::SameLine();
  ImGui::TextDisabled("Speed:");
  for (int i = 1; i < 7; ++i) {
    ImGui::SameLine();
    const std::string label = std::string(i == 4 ? "1h = 1 day" : pace_name(kSpeeds[i])) + " (" + std::to_string(i) + ")##s" + std::to_string(i);
    if (ImGui::RadioButton(label.c_str(), a.pacer.requested() == kSpeeds[i] && !a.resting)) set_speed(a, kSpeeds[i]);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", pace_meaning(kSpeeds[i]));
  }
  ImGui::SameLine();
  ImGui::Checkbox("Pause on alerts", &a.pause_on_alerts);
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Off: alerts are shown at the top right and time keeps running.\nOn: a walker in sight, a bite or someone down pauses the game.");
  ImGui::SameLine();
  if (ImGui::Button(a.resting ? "Stop resting" : "Rest until something happens")) {
    a.resting = !a.resting;
    a.rest_until = now + 8 * kTicksPerGameHour;
    if (a.resting) a.pacer.acknowledge();
  }
  // 4. Where we are and the house's power.
  const double fuel_l = static_cast<double>(p.fuel_ul(w)) / 1e6;
  if (const PlaceSystem* place = a.t.place) {  // spec 42 rule 7
    const Place& at = place->place();
    ImGui::TextDisabled("A town of %.0f in %s, %s income | %s, %.0f %s %.0f %s, %.0f m | outside %.1f C | rain today %.1f mm",
                        at.town_people, at.country_name.empty() ? "a base country" : at.country_name.c_str(),
                        at.income == "upper_middle" ? "upper-middle" : at.income == "lower_middle" ? "lower-middle" : at.income.c_str(),
                        at.biome.c_str(), std::abs(at.lat_deg), at.lat_deg < 0 ? "S" : "N", std::abs(at.lon_deg), at.lon_deg < 0 ? "W" : "E",
                        at.elevation_m, place->outdoor_c(w), place->today_rain_mm(w));
  }
  ImGui::Text("Grid %s | generator %s, %.1f L | level %d (PgUp/PgDn)%s | overlay:", p.mains_alive() ? "on" : "DEAD",
              p.generator_running() ? "running" : "off", fuel_l, a.level, a.follow ? "" : " | F: follow again");
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
  a.time_bar_bottom = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
  ImGui::End();
}

// A need bar coloured by its band, with the word (spec 16 rule 10). `v` is always "higher is worse".
void bar(const char* label, double v) {
  ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::ColorConvertU32ToFloat4(band_colour(v)));
  ImGui::ProgressBar(static_cast<float>(std::clamp(v, 0.0, 1.0)), {120, 0}, band_word(v));
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::TextUnformatted(label);
}

// What to do about a state, in a few words.
const char* state_hint(const std::string& s) {
  if (s == "thirst") return "drink water";
  if (s == "hunger") return "eat something";
  if (s == "fatigue") return "rest or sleep";
  if (s == "pain") return "treat the wounds, rest";
  if (s == "bleeding") return "dress the wound now";
  if (s == "fear") return "get away from walkers, stay with the group";
  if (s == "sick") return "clean the infected wound";
  return "";
}

void inspector(App& a) {
  const ImGuiIO& io = ImGui::GetIO();
  ImGui::SetNextWindowPos({io.DisplaySize.x - 340, 8}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize({330, 470}, ImGuiCond_FirstUseEver);
  const EntityId id = a.selected;
  const bool any = id && a.t.bodies->exists(id);
  const std::string title = (any ? "Selected: " + a.name(id) : std::string("Selected: nobody")) + "###Inspector";
  ImGui::Begin(title.c_str());
  if (!any) {
    ImGui::TextWrapped("Left click a person or a walker on the map (or a name in the Group panel) to see them here.");
    ImGui::End();
    return;
  }
  const World& w = *a.t.world;
  const Body& b = a.t.bodies->get(id);
  // Who this is, in one line.
  const char* who = b.spec.walker ? "a walker: a dead person who walks" : !b.alive ? "dead" : id == a.me() ? "you control this person"
                    : a.t.ai && a.t.ai->minds().contains(id) ? "driven by the AI" : "not in your group yet";
  ImGui::Text("%s, %d", a.name(id).c_str(), static_cast<int>(b.spec.age));
  ImGui::TextDisabled("%s", who);
  if (a.t.actors->actors().contains(id) && b.alive && !b.spec.walker) {
    ImGui::SeparatorText("Doing");
    const Actor& act = a.t.actors->actor(id);
    std::string doing = act.action.phase == Phase::Idle ? "nothing" : act.action.process < 0 ? "walking"
                        : a.t.actors->processes()[static_cast<std::size_t>(act.action.process)].name;
    if (act.action.phase == Phase::Approach && act.action.process >= 0) doing = "on the way to: " + doing;
    ImGui::Text("%s%s", doing.c_str(), b.activity.asleep ? " (asleep)" : "");
    if (const auto d = doing_of(a, id); d && d->done >= 0)
      ImGui::ProgressBar(static_cast<float>(d->done), {-1, 0}, (duration_text(d->left_s) + " left").c_str());
    if (a.t.ai && a.t.ai->minds().contains(id) && id != a.t.ai->possessed())
      ImGui::TextDisabled("The AI wants to: %s", to_string(a.t.ai->mind(id).doing));
  }
  if (b.alive && !b.spec.walker) {
    // How they are: only what is wrong, worst first, with what helps; everything else is fine.
    ImGui::SeparatorText("How they are");
    const auto states = worst_states(a, id);
    if (states.empty()) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.55f, 0.85f, 0.55f, 1});
      ImGui::TextWrapped("Fine: not hungry, thirsty, tired, hurt or afraid.");
      ImGui::PopStyleColor();
    }
    for (const State& s : states) {
      std::string name = s.name;
      name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
      ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(band_colour(s.v)), "%s: %s", name.c_str(), band_word(s.v));
      ImGui::SameLine();
      ImGui::TextDisabled("- %s", state_hint(s.name));
    }
    if (ImGui::TreeNode("All body values")) {
      const Needs n = a.t.bodies->needs(b);
      bar("thirst", n.thirst);
      bar("hunger", n.hunger);
      bar("tiredness", n.tiredness);
      bar("exhaustion", n.exhaustion);
      bar("blood lost", std::min(1.0, a.t.bodies->blood_loss_fraction(b) / 0.3));
      bar("pain", a.t.bodies->pain(b, w.clock.tick));
      bar("fear", b.fear);
      bar("low morale", 1 - b.morale);
      ImGui::TextDisabled("Bars fill as things get worse.");
      ImGui::TreePop();
    }
  }
  // Wounds grouped by region, with a count (spec 16 rule 12).
  ImGui::SeparatorText("Wounds");
  if (b.wounds.empty()) ImGui::TextDisabled("No wounds.");
  std::map<int, std::vector<const Wound*>> by_region;
  for (const Wound& wd : b.wounds) by_region[static_cast<int>(wd.region)].push_back(&wd);
  for (const auto& [region, list] : by_region) {
    bool infected = false;
    for (const Wound* wd : list) infected = infected || wd->infection_visible || (a.truth && wd->infection != Infection::None);
    const std::string head = std::string(to_string(static_cast<Region>(region))) + ": " + std::to_string(list.size()) +
                             (list.size() == 1 ? " wound" : " wounds") + (infected ? ", INFECTED" : "") + "##w" + std::to_string(region);
    if (ImGui::TreeNode(head.c_str())) {
      for (const Wound* wd : list)
        ImGui::BulletText("%s, %.0f mm%s%s%s", to_string(wd->type), wd->length_mm, wd->cleaned != Cleaned::No ? ", cleaned" : "",
                          wd->dressed ? ", dressed" : "",
                          wd->infection_visible || (a.truth && wd->infection != Infection::None) ? ", INFECTED" : "");
      ImGui::TreePop();
    }
  }
  if (a.truth && a.t.walkers && a.t.walkers->turning_of(id)) ImGui::TextColored({1, 0.3f, 0.3f, 1}, "Truth: turning (walker pathogen)");
  if (a.t.actors->actors().contains(id)) {
    ImGui::SeparatorText("Carrying");
    const auto carried = a.t.actors->carried(w, id);
    if (carried.empty()) ImGui::TextDisabled("Nothing.");
    for (EntityId it : carried) ImGui::BulletText("%s", a.t.items->def(a.t.items->get(it).def).name.c_str());
  }
  ImGui::Separator();
  ImGui::Checkbox("Debug: show hidden truth (infection)", &a.truth);
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
        recentre(a);
      }
    }
    // The state column: the worst states, in words and their band's colour.
    if (b.alive && !b.away && !b.spec.walker) {
      bool first = true;
      for (const State& s : worst_states(a, id)) {
        if (first) ImGui::Indent(18);
        else ImGui::SameLine();
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(band_colour(s.v)), "%s (%s)", s.name, band_word(s.v));
        first = false;
      }
      if (!first) ImGui::Unindent(18);
    }
    ImGui::PopID();
  }
  if (a.t.ai->possessed()) {
    if (ImGui::Button("Let the AI drive everyone")) a.t.ai->release();
  } else if (ImGui::Button("Take control again")) {
    a.t.ai->possess(a.t.run->controlled());
    recentre(a);
  }
  ImGui::End();
}

// One legend row: a sample drawn by `draw` in a 28x18 box, then the words.
template <typename Draw>
void legend_row(const char* words, Draw draw) {
  const ImVec2 p = ImGui::GetCursorScreenPos();
  draw(ImGui::GetWindowDrawList(), ImVec2{p.x + 14, p.y + 9});
  ImGui::Dummy({30, 18});
  ImGui::SameLine();
  ImGui::TextUnformatted(words);
}

void map_key(App& a) {
  const ImGuiIO& io = ImGui::GetIO();
  ImGui::SetNextWindowPos({io.DisplaySize.x - 340, 694}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize({330, 200}, ImGuiCond_FirstUseEver);
  ImGui::Begin("Legend: what the map shows###Legend");
  const auto line = [](ImU32 col, float thick) {
    return [col, thick](ImDrawList* d, ImVec2 c) { d->AddLine({c.x - 12, c.y}, {c.x + 12, c.y}, col, thick); };
  };
  legend_row("you (white ring, arrow: where you head)", [](ImDrawList* d, ImVec2 c) {
    d->AddCircleFilled(c, 7, kPopColours[0]);
    d->AddCircle(c, 9, rgb(255, 255, 255), 0, 2.5f);
  });
  legend_row("a person of your group", [](ImDrawList* d, ImVec2 c) { d->AddCircleFilled(c, 7, kPopColours[1]); });
  legend_row("a walker standing still", [](ImDrawList* d, ImVec2 c) {
    d->AddQuadFilled({c.x, c.y - 8}, {c.x + 8, c.y}, {c.x, c.y + 8}, {c.x - 8, c.y}, kWalkerIdle);
  });
  legend_row("a walker after something (line: where it goes)", [](ImDrawList* d, ImVec2 c) {
    d->AddLine(c, {c.x + 14, c.y - 6}, rgb(238, 102, 119, 160), 1.5f);
    d->AddQuadFilled({c.x, c.y - 8}, {c.x + 8, c.y}, {c.x, c.y + 8}, {c.x - 8, c.y}, kWalkerActive);
  });
  legend_row("an object or furniture (name below)", [](ImDrawList* d, ImVec2 c) {
    d->AddRectFilled({c.x - 8, c.y - 8}, {c.x + 8, c.y + 8}, rgb(50, 50, 60));
    d->AddRect({c.x - 8, c.y - 8}, {c.x + 8, c.y + 8}, rgb(200, 200, 210), 0.0f, 0, 1.5f);
  });
  legend_row("an item on the floor", [](ImDrawList* d, ImVec2 c) { d->AddCircleFilled(c, 3, rgb(240, 220, 120)); });
  legend_row("work in progress (fills up)", [](ImDrawList* d, ImVec2 c) {
    d->AddRectFilled({c.x - 12, c.y - 3}, {c.x + 12, c.y + 3}, rgb(15, 15, 18));
    d->AddRectFilled({c.x - 12, c.y - 3}, {c.x + 2, c.y + 3}, rgb(120, 200, 120));
  });
  legend_row("a sound you hear, placed (rings: where it seems to come from)", [](ImDrawList* d, ImVec2 c) {
    for (float r : {3.0f, 6.0f, 9.0f}) d->AddCircle(c, r, r < 5 ? rgb(80, 205, 225) : rgb(80, 205, 225, 90), 0, 2.0f);
  });
  legend_row("a sound you hear, faint (cone: only the direction)", [](ImDrawList* d, ImVec2 c) {
    d->PathClear();
    for (int i = -4; i <= 4; ++i) d->PathLineTo({c.x + std::cos(0.5f * i / 4) * 9, c.y - std::sin(0.5f * i / 4) * 9});
    d->PathStroke(rgb(80, 205, 225, 90), 0, 5.0f);
    d->AddTriangleFilled({c.x + 14, c.y}, {c.x + 10, c.y - 4}, {c.x + 10, c.y + 4}, rgb(80, 205, 225));
  });
  legend_row("a sound heard a moment ago (thin, fading away)", [](ImDrawList* d, ImVec2 c) {
    d->AddCircle(c, 7, rgb(80, 205, 225, 130), 0, 1.0f);
  });
  legend_row("noise overlay: noticed line (0 dB over the background)", line(rgb(250, 220, 90), 1.5f));
  legend_row("noise overlay: placed line (+10 dB: you would know where)", line(rgb(255, 255, 255), 2.5f));
  legend_row("outside wall", line(rgb(235, 230, 220), 4));
  legend_row("inside wall", line(rgb(190, 185, 175), 3));
  legend_row("door", line(rgb(170, 110, 50), 3));
  legend_row("window", line(rgb(140, 200, 240), 3));
  legend_row("boards nailed over a door or window", line(rgb(240, 140, 40), 3));
  legend_row("a wall that stopped your step (flashes)", line(rgb(255, 80, 80), 5));
  ImGui::SeparatorText("Objects");
  std::vector<std::string> kinds;
  for (const MapObject& o : a.t.map->objects())
    if (drawn_object(o) && std::find(kinds.begin(), kinds.end(), o.kind) == kinds.end()) kinds.push_back(o.kind);
  std::sort(kinds.begin(), kinds.end());
  for (const std::string& k : kinds) {
    std::string name = k;
    std::replace(name.begin(), name.end(), '_', ' ');
    ImGui::BulletText("%-4s %s%s", object_label(k).c_str(), name.c_str(), a.t.map->blocked(*a.t.map->find_object(k)) ? " (blocks the way)" : "");
  }
  ImGui::SeparatorText("States (by you on the map and in Group)");
  ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(band_colour(kMild)), "mild");
  ImGui::SameLine();
  ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(band_colour(kBad)), "bad");
  ImGui::SameLine();
  ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(band_colour(kSevere)), "severe");
  ImGui::End();
}

void log_panel(App& a) {
  const ImGuiIO& io = ImGui::GetIO();
  ImGui::SetNextWindowPos({8, io.DisplaySize.y - 236}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize({io.DisplaySize.x - 360, 210}, ImGuiCond_FirstUseEver);
  ImGui::Begin("Events: what happened, and why (hover a line)###Events");
  for (int c = 0; c < kCategories; ++c) {
    ImGui::Checkbox(kCategoryNames[c], &a.show[c]);
    ImGui::SameLine();
  }
  ImGui::Checkbox("only the selected person", &a.only_selected);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(160);
  ImGui::InputTextWithHint("##filter", "search", a.filter, sizeof a.filter);
  ImGui::BeginChild("lines");
  for (const LogLine& l : a.log) {
    if (!a.show[l.category]) continue;
    if (a.only_selected && l.subject != a.selected) continue;
    if (a.filter[0] && l.text.find(a.filter) == std::string::npos) continue;
    const ImVec4 col = std::strcmp(l.kind, "dead") == 0 ? ImVec4{1, 0.45f, 0.45f, 1}
                       : std::strcmp(l.kind, "body") == 0 ? ImVec4{0.7f, 0.7f, 0.7f, 1}
                       : std::strcmp(l.kind, "done") == 0 ? ImVec4{0.6f, 0.85f, 0.6f, 1}
                                                          : ImVec4{1, 0.85f, 0.4f, 1};
    if (l.count > 1) ImGui::TextColored(col, "%s  [%s]  %s  x%d", time_text(l.tick).c_str(), kCategoryNames[l.category], l.text.c_str(), l.count);
    else ImGui::TextColored(col, "%s  [%s]  %s", time_text(l.tick).c_str(), kCategoryNames[l.category], l.text.c_str());
    // The cause chain: what led to this, back to the first cause (spec 16: log lines link to their causes).
    if (ImGui::IsItemHovered() && l.event) {
      const auto chain = a.t.world->log.cause_chain(l.event);
      ImGui::BeginTooltip();
      ImGui::TextUnformatted("Why this happened:");
      if (chain.size() <= 1) ImGui::TextDisabled("  nothing recorded before it (it started here)");
      for (std::size_t i = 1; i < chain.size() && i < 8; ++i) {
        const auto d = describe(a, chain[i]);
        ImGui::Text("  %s  %s", time_text(chain[i].tick).c_str(),
                    d ? d->text.c_str() : (std::string("event type ") + std::to_string(static_cast<int>(chain[i].type))).c_str());
      }
      ImGui::EndTooltip();
    }
  }
  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20) ImGui::SetScrollHereY(1.0f);
  ImGui::EndChild();
  ImGui::End();
}

void help_line() {
  const ImGuiIO& io = ImGui::GetIO();
  ImGui::GetForegroundDrawList()->AddText({8, io.DisplaySize.y - 18}, IM_COL32(200, 200, 200, 160),
                                          "WASD walk (takes over what you were doing) | right click: actions | left click: select | wheel: zoom | "
                                          "middle drag: pan, F/Home: follow | Space pause or continue, 1-6 speed | C cancel | "
                                          "F5 save, F9 load | placeholder art");
}

// The map fills the space below the time bar, left of the side panels and above the log (spec 16 rule 13).
void layout_view(App& a) {
  const ImVec2 ds = ImGui::GetIO().DisplaySize;
  a.view_min = {8, a.time_bar_bottom + 6};
  a.view_max = {std::max(a.view_min.x + 100, ds.x - 348), std::max(a.view_min.y + 100, ds.y - 240)};
}

}  // namespace

int window_main(int argc, char** argv) {
  App a;
  std::uint64_t seed = 1;
  std::string shot;
  std::int64_t shot_ticks = 0;
  std::optional<Cell> shot_menu;
  bool watch = false, generator = false;  // generator: a shot aid, the grid down and the generator running
  Overlay overlay = Overlay::None;
  std::string world_name, from_save;
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--seed") && i + 1 < argc) seed = std::strtoull(argv[++i], nullptr, 10);
    else if (!std::strcmp(argv[i], "--world") && i + 1 < argc) world_name = argv[++i];
    else if (!std::strcmp(argv[i], "--from") && i + 1 < argc) from_save = argv[++i];
    else if (!std::strcmp(argv[i], "--play")) {}
    else if (!std::strcmp(argv[i], "--shot") && i + 1 < argc) shot = argv[++i];
    else if (!std::strcmp(argv[i], "--watch")) watch = true;
    else if (!std::strcmp(argv[i], "--generator")) generator = true;
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
  // A world folder (0074): its seed, and its saves folder for F5 and F9.
  if (!world_name.empty()) {
    try {
      a.world = zs::worlds::load(world_name);
      seed = a.world->seed;
    } catch (const std::exception& ex) {
      std::fprintf(stderr, "%s\n", ex.what());
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "zs", ex.what(), nullptr);
      return 1;
    }
  }
  const std::string title = "Juego Zombie - " + (a.world ? a.world->name : std::string("Tier 0 sandbox"));
  SDL_Window* window = SDL_CreateWindow(title.c_str(), 1440, 900, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
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
  if (!from_save.empty()) load_game(a, from_save);
  if (watch) a.t.ai->release();
  if (generator && a.t.power) {  // for checking the sound marks by eye: the same calls the stage 6 scenario makes
    a.t.power->fail_grid(*a.t.world, 0);
    a.t.power->set_transfer(*a.t.world, Transfer::Generator, 0);
    a.t.power->start_generator(*a.t.world, 0);
  }
  a.overlay = overlay;
  if (shot_ticks > 0) {
    for (std::int64_t i = 0; i < shot_ticks; ++i) a.t.world->step();
    read_events(a);
    a.pacer.set_requested(Pace::Pause);
    recentre(a);
  }
  check_sight(a);

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
    a.real_s += dt;
    if (shot.empty()) advance(a, dt);

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    layout_view(a);
    update_camera(a, frame == 0 ? 1.0 : dt);
    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    draw_map(a, bg);
    map_notes(a, bg);
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
    map_key(a);
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
