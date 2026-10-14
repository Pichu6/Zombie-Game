// Actors, body primitives, data-driven processes and the action executor.
// Specs: docs/tier0/specs/14-action-executor.md, 15-items-recipes.md
//
// Code holds only the primitives (body verbs) and the effect vocabulary. Every contextual action is
// a process in data/tier0/processes.toml that finds its tools and inputs by property, never by name.
#pragma once
#include <array>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "zs/assembly.h"
#include "zs/body.h"
#include "zs/site_map.h"

namespace zs {

// ---- Primitives: the only verbs written in code ----

enum class Primitive : std::uint8_t {
  Walk, Run, Crawl, Climb, Hold, Strike, PushPull, Cut, Pry, Fine, Consume, Rest, Sleep, Exercise, Search, Wait,
  Shout, Count
};
enum class Requires : std::uint8_t { Consciousness, Moving, Manipulation };

struct PrimitiveInfo {
  const char* name;
  double met;       // body cost while doing it (Ainsworth et al. 2011 Compendium; estimates marked in actions.cpp)
  double noise_db;  // at 1 m (spec 18 reads it)
  Requires needs;
};
const PrimitiveInfo& primitive_info(Primitive p);
std::optional<Primitive> primitive_from(const std::string& name);

// ---- Processes: contextual actions, in data ----

enum class TargetKind : std::uint8_t { Self, Wound, Object, Item, Edge, Body };  // Body: another body ("body:walker")
enum class Trade : std::uint8_t { Everyday, FirstAid, Carpentry, Mechanics, Electronics, Count };
inline constexpr std::size_t kTradeCount = static_cast<std::size_t>(Trade::Count);
const char* to_string(Trade t);

// A carried tool that qualifies when prop >= min; speed scales with prop / ref.
struct ToolNeed {
  Prop prop;
  double min = 0;
  double ref = 1;
};
// A carried thing the effects use up or move (a plank, a bandage).
struct UseNeed {
  Prop prop;
  double min = 0;
  bool optional = false;  // e.g. nails only when the barricade has none yet
};
// A stock that must be available at the target or in a carried container.
struct StockNeed {
  std::string resource;
  std::int64_t amount = 0;
  bool from_target = false;
};

enum class EffectOp : std::uint8_t {
  Eat,        // amount J from the target store into the body
  Drink,      // amount uL from the target store (or a carried container) into the body
  Fill,       // amount uL from the target store into carried containers, up to their capacity
  UseStock,   // amount of `resource` from carried containers into the sink "used"
  Treat,      // how = clean_water | disinfect | dress | splint | tourniquet on the target wound
  Board,      // nail the first carried board (and nails, if needed) across the target edge
  Salvage,    // unmount every part of the target onto the floor
  Take,       // pick up the target item
  Sleep,      // sleep until the body wakes
  Power,      // how = start | stop | refuel | on | off | transfer_generator | transfer_mains | reset (spec 17)
  Cover,      // hang the first carried `uses` item over the target window (a curtain; spec 19)
  Hit,        // strike the target body with the tool in hand (spec 20)
  Wake,       // wake every sleeper within 15 m on the same level (a shout; spec 24 guard)
  Story,      // a scenario effect named by `how` (invite, repair_radio, listen_radio; spec 26)
};

struct Effect {
  EffectOp op;
  std::string resource;  // UseStock
  std::int64_t amount = 0;
  std::string how;       // Treat, Power
};

struct ProcessDef {
  std::string id, name;
  Primitive primitive = Primitive::Wait;
  TargetKind target = TargetKind::Self;
  std::string target_match;  // object kind, item family or edge family ("" = any)
  Trade trade = Trade::Everyday;
  double time_s = 10;
  bool resumable = false;
  std::vector<ToolNeed> tools;  // any one of them qualifies (the best is used)
  std::vector<UseNeed> uses;
  std::vector<StockNeed> stocks;
  std::vector<Effect> effects;
  std::string source;
};

// ---- Commands and offers ----

// Walk (spec 50): a = heading 0-7 clockwise from north, or -1 to stop at the next cell.
enum class CommandType : std::uint16_t { MoveTo = 1, Use = 2, Cancel = 3, Walk = 4 };
// The step of a heading 0-7 (clockwise from north; y grows southwards).
inline void heading_step(int heading, int& dx, int& dy) {
  constexpr int kDx[8] = {0, 1, 1, 1, 0, -1, -1, -1}, kDy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
  dx = heading >= 0 && heading < 8 ? kDx[heading] : 0;
  dy = heading >= 0 && heading < 8 ? kDy[heading] : 0;
}
std::int64_t pack_cell(const Cell& c);
Cell unpack_cell(std::int64_t v);

enum class Reason : std::uint8_t {
  Ok, UnknownProcess, WrongTarget, Dead, Unconscious, CannotMove, CannotUseHands, NoTool, MissingItem,
  MissingStock, NoRoute, TargetGone, InUse, Full, Interrupted, Asleep, TooHeavy,
  AlreadyDone, NoFuel, Held, Broken, NoPower
};
const char* to_string(Reason r);

struct Offer {
  std::size_t process = 0;
  EntityId target = 0;
  bool enabled = false;
  Reason reason = Reason::Ok;
  double time_s = 0;  // expected work time with the best tool
};

enum class Phase : std::uint8_t { Idle, Approach, Work };

struct Action {
  Phase phase = Phase::Idle;
  int process = -1;  // -1 = plain move
  EntityId target = 0;
  Cell goal{};
  std::vector<Cell> path;
  std::size_t path_next = 0;
  double step_m = 0;      // progress toward path[path_next]
  double door_wait_s = 0; // opening a door on the way
  double progress_s = 0;
  double total_s = 0;
  EntityId tool = 0;
  EntityId held = 0;      // an item extracted into the hands during the work phase (Take)
  bool close_behind = false;  // a door opened on the way, closed after stepping through
  EdgeRef opened{};
  bool steering = false;      // a Walk: the path grows one cell at a time from the heading (spec 50 rule 5)
};

struct Actor {
  EntityId body = 0;
  OwnerId inventory = 0;  // stocks carried outside containers (none yet; kept for later)
  Cell pos{};
  std::array<int, kTradeCount> skill{};  // 0-100; -1 = trade unknown (hides its processes)
  Action action;
  std::deque<std::pair<Command, EventId>> queue;  // intents and the event that carried them
  EventId cause = 0;                              // of the running action
  double carried_kg = 0;                          // cached; recomputed when the hands change
  double noise_db = 0;  // what the current primitive makes (spec 18 reads it)
  int heading = -1;     // a Walk intent's direction 0-7, or -1 (spec 50)
};

// ---- The executor ----

class Actors : public System {
 public:
  Actors(World& w, Assemblies& items, SiteMap& map, Bodies& bodies);
  const char* name() const override { return "actors"; }
  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& r) override;

  // Setup.
  void load_processes(const std::string& path);
  // Gives map objects entities, creates stores (pantry, sink) and loose items.
  void populate(World& w);
  EntityId add_actor(World& w, EntityId body, const Cell& at);

  const std::vector<ProcessDef>& processes() const { return processes_; }
  std::optional<std::size_t> find_process(const std::string& id) const;
  const Actor& actor(EntityId body) const { return actors_.at(body); }
  Actor& actor(EntityId body) { return actors_.at(body); }
  const std::map<EntityId, Actor>& actors() const { return actors_; }

  // Map objects and stores.
  EntityId object_entity(std::size_t object_index) const { return object_entities_.at(object_index); }
  // Spec 73: after a place is opened in play, or a save holding one is loaded: its objects get entities (minted now, or the
  // ones the map kept), and the walls and doors of every level are indexed again. Objects opened in play hold no stocks.
  void sync_with_map(World& w);
  std::optional<EntityId> find_object(const std::string& kind, const std::string& id = "") const;
  std::optional<OwnerId> store_of(EntityId holder) const;
  // Spec 80: the first map object of a kind that holds a store ("pantry", "fridge"), by entity order; none if there is none.
  // Spec 81: `id` names the house it belongs to ("": the main building's, with no id).
  std::optional<OwnerId> first_store(const std::string& kind, const std::string& id = "") const;
  const std::map<EntityId, EdgeRef>& edges() const { return edge_of_; }  // edge assembly -> place
  // Floors are fixed ids below World::kFirstEntity (one per cell), so they need no saved state.
  static constexpr EntityId kFloorBase = EntityId{1} << 19;
  EntityId floor_at(const Cell& c) const;
  std::optional<Cell> floor_cell(EntityId e) const;

  // Where a thing is: the cell of the actor, object or floor it is under (or the edge's cell).
  std::optional<Cell> cell_of(const World& w, EntityId entity) const;
  std::vector<EntityId> carried(const World& w, EntityId body) const;
  double carried_kg(const World& w, EntityId body) const;

  // Offers: preview runs the same checks as execution and changes nothing.
  Offer preview(const World& w, EntityId body, std::size_t process, EntityId target) const;
  std::vector<Offer> offers_for(const World& w, EntityId body, EntityId target) const;

  // Hooks to systems that come later in the update order, so the executor does not depend on them.
  // power_check returns Ok or why the power effect can't be done; power_apply does it.
  std::function<Reason(const World&, EntityId body, EntityId target, const std::string& how)> power_check;
  std::function<void(World&, EntityId body, EntityId target, const std::string& how, EventId cause)> power_apply;
  // story_check / story_apply: scenario effects (spec 26), same contract as power_check / power_apply.
  std::function<Reason(const World&, EntityId body, EntityId target, const std::string& how)> story_check;
  std::function<void(World&, EntityId body, EntityId target, const std::string& how, EventId cause)> story_apply;
  std::function<double(const World&, const Cell&)> light_lux;  // illuminance where a pop works (spec 19)
  // Stage 7: where a walker body is, whether a body is held by a walker, and a strike landing.
  std::function<std::optional<Cell>(EntityId body)> body_cell;
  std::function<bool(EntityId body)> held;
  std::function<void(World&, EntityId attacker, EntityId target, EntityId tool, EventId cause)> hit;
  // Work-rate factor from light for fine, strike, pry and cut work: 0.5 below 10 lux, 1 from 100 lux.
  double light_factor(const World& w, const Actor& a, const ProcessDef& p) const;

  // A curtain (cloth of >= 1 m2) hangs on this window.
  bool covered(const World& w, EntityId window) const;

  bool idle(EntityId body) const { return actors_.at(body).action.phase == Phase::Idle && actors_.at(body).queue.empty(); }

 private:
  struct Found {
    Reason reason = Reason::Ok;
    EntityId tool = 0;
    double tool_factor = 1;
  };
  Found check(const World& w, const Actor& a, const ProcessDef& p, EntityId target) const;
  bool target_matches(const World& w, const ProcessDef& p, const Actor& a, EntityId target) const;
  std::vector<Cell> reach_cells(const World& w, const ProcessDef& p, const Actor& a, EntityId target) const;
  std::optional<EntityId> carried_with(const World& w, const Actor& a, Prop prop, double min) const;
  std::int64_t carried_stock(const World& w, const Actor& a, ResourceId r) const;
  // A carried part of `family` (what a slot accepts) with prop >= 0.5, best first.
  std::optional<EntityId> fitting_part(const World& w, const Actor& a, const std::string& family, Prop prop) const;
  double work_time(const World& w, const Actor& a, const ProcessDef& p, double tool_factor) const;

  void start_next(World& w, Actor& a);
  void start_steer(World& w, Actor& a);  // spec 50 rule 5
  void start(World& w, Actor& a, const Command& c, EventId cause);
  void recompute_carried(const World& w, Actor& a);
  void interrupt(World& w, Actor& a, Reason why);
  void finish(World& w, Actor& a);
  void step_move(World& w, Actor& a);
  void set_body_activity(World& w, Actor& a);
  void apply_effect(World& w, Actor& a, const ProcessDef& p, const Effect& e);

  Assemblies& items_;
  SiteMap& map_;
  Bodies& bodies_;
  ResourceId disinfectant_ = 0;
  std::vector<std::pair<ResourceId, double>> liquids_;  // what a container can hold (one at a time), g per mL
  double contents_g(const World& w, EntityId container) const;
  OwnerId used_ = 0;
  std::vector<ProcessDef> processes_;
  std::vector<EntityId> object_entities_;           // per map object
  std::map<EntityId, std::size_t> object_index_;    // entity -> map object
  std::map<EntityId, OwnerId> stores_;              // holder (object or container) -> ledger owner
  std::map<EntityId, EdgeRef> edge_of_;             // edge assembly -> where it is
  void index_edges();
  std::map<EntityId, EntityId> reserved_;           // instance -> actor
  std::map<std::pair<int, EntityId>, double> resume_;  // (process, target) -> kept progress
  std::map<EntityId, Actor> actors_;
};

}  // namespace zs
