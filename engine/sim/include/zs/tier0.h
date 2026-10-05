// The Tier 0 world: map, items, bodies and actors wired in the fixed system order.
// Specs: docs/tier0/specs/05-clock.md (update order), 14-action-executor.md
#pragma once
#include <memory>

#include "zs/actions.h"

namespace zs::tier0 {

struct Tier0 {
  std::unique_ptr<World> world;
  Assemblies* items = nullptr;
  SiteMap* map = nullptr;
  Actors* actors = nullptr;
  Bodies* bodies = nullptr;
  EntityId player = 0;  // the body of the pop at the map's player start
};

// Loads items, the block map and processes, creates the player pop and closes setup.
Tier0 make_world(std::uint64_t seed);

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

// Gives the player the wound the script treats (a dirty forearm cut).
EntityId wound_player(Tier0& t);

}  // namespace zs::tier0
