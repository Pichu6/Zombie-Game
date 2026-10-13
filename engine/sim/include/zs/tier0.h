// The Tier 0 world: map, items, bodies and actors wired in the fixed system order.
// Specs: docs/tier0/specs/05-clock.md (update order), 14-action-executor.md
#pragma once
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "zs/actions.h"
#include "zs/dead.h"
#include "zs/grid.h"
#include "zs/calendar.h"
#include "zs/place.h"
#include "zs/police_agency.h"
#include "zs/pop_ai.h"
#include "zs/power.h"

namespace zs {
class PopSystem;
class EconomySystem;
class OrdersSystem;
class GridSystem;
class CalendarSystem;
class Expectations;
class MissingPersons;
class Payroll;
class Bonds;
class PoliceCases;
class PoliceStaff;
class Players;
struct Hands;
}  // namespace zs

namespace zs::tier0 {

// Spec 70: a place made beside the block (decision 0077): a site map (one or more levels from z = 0) loaded as levels of
// its own from `z`, joined to nothing. A police station names the officers whose station it is, in the agency's order.
struct Fragment {
  std::string toml;
  int z = 10;
  std::string kind = "police_station";
  std::string name;
  int first_officer = 0, officers = 0;  // officers [first, first + officers) of the town's force
};
// A fragment as the one world holds it: where its door is, once loaded.
struct Site {
  std::string kind, name;
  int number = 0;  // SiteMap::add_site's site number
  Cell door;       // the cell inside its front door (its marker object)
  int first_officer = 0, officers = 0;
};

// Spec 41: the owners the one world sets up (the run only).
struct WorldOwners {
  OwnerId country = 0, region = 0;
  OwnerId main_building = 0, house_a = 0, house_b = 0;
  OwnerId employers = 0;  // spec 62: the placeholder employer that pays every wage until establishments are placed
  std::vector<OwnerId> pops;  // by census row (= registry slot)
  GridId nation = 0, district = 0;
};

struct Tier0 {
  std::unique_ptr<World> world;
  Assemblies* items = nullptr;
  SiteMap* map = nullptr;
  Actors* actors = nullptr;
  Bodies* bodies = nullptr;
  Power* power = nullptr;
  Census* census = nullptr;    // stage 7, with_dead only
  Walkers* walkers = nullptr;
  PopAI* ai = nullptr;         // stage 8, group runs only
  Run* run = nullptr;          // stage 9, make_run only
  // Spec 41, the one world (make_run only): the registry, the economy, orders and the grid tree.
  PopSystem* pops = nullptr;
  EconomySystem* econ = nullptr;
  OrdersSystem* orders = nullptr;
  GridSystem* grids = nullptr;
  PlaceSystem* place = nullptr;  // spec 42
  CalendarSystem* calendar = nullptr;  // spec 53
  Expectations* expectations = nullptr;  // spec 59 (the one world only)
  MissingPersons* missing = nullptr;     // spec 60 (the one world only)
  Payroll* payroll = nullptr;            // spec 62 (the one world only)
  Bonds* bonds = nullptr;                // spec 63 (the one world only)
  PoliceCases* police = nullptr;         // spec 64 (the one world only)
  std::shared_ptr<PoliceStaff> force;    // spec 65: the officers with their shifts (the one world only)
  Players* players = nullptr;            // spec 66: players and possession (the one world only)
  std::shared_ptr<Hands> hands;          // spec 67: how the players reach bodies (the one world only)
  AgencyPlan agency;                     // spec 69: the town's police agency by its size (the one world only)
  std::vector<Site> sites;               // spec 70: the places loaded beside the block (the one world from zs::gen::make_world)
  WorldOwners owners;
  EntityId player = 0;  // the body of the pop at the map's player start
  std::vector<EntityId> group;  // the player first, then survivors who start with it
};

// Loads items, the block map and processes, creates the player pop and closes setup.
// with_dead adds the census and walkers (stage 7); earlier stages' scenarios run without them.
// group > 0 (needs with_dead) adds the pop AI for a group of that size: the player plus census
// survivor slots starting next to it, every one driven by the AI unless possessed (stage 8).
Tier0 make_world(std::uint64_t seed, bool with_dead = false, int group = 0);

// The run "The generator problem" (spec 26): the player alone in the main building with the dead,
// the found survivors waiting where data/tier0/run.toml puts them, the stranger still away, every pop
// driven by the AI unless possessed. `group` lists the player and the found survivors (joined or not).
// Spec 41: it is the one world: the census rows are the registry's founders, and the registry, the economy,
// orders and the grid tree run in it after the Tier 0 systems.
// Spec 42: `place` is the block's cell on a generated planet (zs::gen::make_world); the default is a placeholder.
// Spec 70: `fragments` are loaded beside the block (the town's police stations); none by default.
Tier0 make_run(std::uint64_t seed, const Place& place = default_place(), std::int64_t start_day = kDefaultStartDay,
               const std::vector<Fragment>& fragments = {});

// Spec 41: adds the world-scale systems to a run under setup (called by make_run; src/one_world.cpp).
void add_world_systems(Tier0& t, const Place& place, std::int64_t start_day = kDefaultStartDay);

// The stage 4 scripted pop (plan: "done when"): drinks, eats, picks up first-aid supplies,
// disinfects and dresses its wound, takes tools, breaks a chair into planks and nails them across
// the nearest window. The script lives outside the simulation and only sends intents, so the same
// script can drive the pop as the player (World::submit) or as the AI (World::queue_intent).
class ScriptedPop {
 public:
  explicit ScriptedPop(bool as_ai) : as_ai_(as_ai) {}
  // Call before each step. Sends the next intent when the pop is idle. Returns false when done.
  bool drive(Tier0& t);
  int step() const { return step_; }
  static const char* step_name(int step);

 private:
  bool as_ai_;
  int step_ = 0;
};

// The stage 6 scenario (plan: "done when"): the pop eats and drinks every 8 h and sleeps at night.
// When the grid fails it takes the fuel can and a blanket, switches the panel to the generator,
// starts it, lights the front room, hangs the blanket over that room's window and refuels once when
// the tank runs low. It ends when the fridge food spoils after the fuel is gone (or on day 10).
// With the dead (stage 7) it first takes the crowbar and strikes any walker within reach.
class GeneratorScenario {
 public:
  explicit GeneratorScenario(bool as_ai) : as_ai_(as_ai) {}
  bool drive(Tier0& t);  // call before each step; false when finished
  bool refueled() const { return refueled_; }
  // The window the blanket goes over: one of the front-room lamp's walled space.
  static std::optional<EntityId> front_window(const Tier0& t);

 private:
  void use(Tier0& t, const char* process, std::optional<EntityId> target);
  bool as_ai_;
  std::deque<std::pair<const char*, EntityId>> todo_;
  std::int64_t next_meal_ = 0;
  std::int64_t slept_day_ = -1;
  bool set_up_ = false, refueled_ = false, out_twice_ = false, armed_ = false;
};

// Where the game data is: $ZS_DATA, else the source tree it was built from, else ./data.
std::string data_dir();
// "day 2 10:48" for a tick (calendar day, wall clock).
std::string time_text(std::int64_t tick);
// Spec 25 chronicle line for a Death event: name, age, when, cause, and what led to it from the
// cause chain (the bite and who bit, or the strike and who struck).
std::string death_sentence(const Tier0& t, const Event& death);
// Stage 9 (spec 25): the danger tags of a death, read from its cause chain and the log; and the
// missed counters as one clause each ("the wound was never cleaned").
std::vector<std::string> danger_tags(const Tier0& t, const Event& death);
std::vector<std::string> missed_counters(const Tier0& t, const Event& death);
// One event as a plain log line for the player (spec 25), with its kind: "alert", "dead", "body" or
// "done". Events the player does not read (intents, routine actions) give nothing.
struct EventLine {
  std::string text;
  const char* kind = "alert";
};
std::optional<EventLine> event_line(const Tier0& t, const Event& e);
// The day's notable events (top five by score, in time order) as plain lines.
std::vector<std::string> day_summary(const Tier0& t, int day);

// Gives the player the wound the script treats (a dirty forearm cut).
EntityId wound_player(Tier0& t);

}  // namespace zs::tier0
