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
#include "zs/pop_ai.h"
#include "zs/power.h"

namespace zs::tier0 {

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
Tier0 make_run(std::uint64_t seed);

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

// "day 2 10:48" for a tick (calendar day, wall clock).
std::string time_text(std::int64_t tick);
// Spec 25 chronicle line for a Death event: name, age, when, cause, and what led to it from the
// cause chain (the bite and who bit, or the strike and who struck).
std::string death_sentence(const Tier0& t, const Event& death);
// Stage 9 (spec 25): the danger tags of a death, read from its cause chain and the log; and the
// missed counters as one clause each ("the wound was never cleaned").
std::vector<std::string> danger_tags(const Tier0& t, const Event& death);
std::vector<std::string> missed_counters(const Tier0& t, const Event& death);
// The day's notable events (top five by score, in time order) as plain lines.
std::vector<std::string> day_summary(const Tier0& t, int day);

// Gives the player the wound the script treats (a dirty forearm cut).
EntityId wound_player(Tier0& t);

}  // namespace zs::tier0
