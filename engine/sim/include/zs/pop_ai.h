// Pop AI: what a pop does when nobody controls it. Spec: docs/tier0/specs/24-pop-ai.md
//
// Compressed model: every decision step (5 s) each uncontrolled pop scores a short list of wants
// in a fixed order (fight or flee, wounds, thirst, hunger, sleep, the squad order, getting a
// weapon, jobs by priority, idle) and turns the winner into a short plan of process steps. It
// sends those steps as intents (World::queue_intent), so the executor and body do the rest, exactly
// as for the player (parity). The AI reads fear from the body; it never sets it.
#pragma once
#include <array>
#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "zs/dead.h"

namespace zs {

enum class Job : std::uint8_t { Haul, Refuel, Repair, Cook, Guard, Barricade };
inline constexpr std::size_t kJobCount = 6;
const char* to_string(Job j);

enum class OrderType : std::uint8_t { None, Go, Hold, Follow, Attack };
struct Order {
  OrderType type = OrderType::None;
  Cell at{};
  EntityId target = 0;  // Follow: a pop; Attack: a walker
  bool insist = false;  // pushes through a refusal if fear is low enough (rule 4)
};

// Spec 24 rule 4. LiveCircuit is reserved: Tier 0 has no process that touches live wiring yet.
enum class Refusal : std::uint8_t { None, Dark, OpenGround, LiveCircuit };
const char* to_string(Refusal r);

// What the pop is doing, for the view and for the rank of the current plan.
enum class Doing : std::uint8_t {
  Idle, Fight, Flee, Treat, Drink, Eat, Sleep, Order, Arm, Refuel, Barricade, Guard, Haul, PutDown
};
const char* to_string(Doing d);

struct PopMind {
  EntityId body = 0;
  std::array<int, kJobCount> priority{};  // 0 = off, 1 = first ... 3 = last
  Order order;
  bool brave = false, honest = true;      // census disposition, drawn once
  bool reported = false;                  // told the group about a bite
  bool panicked = false;                  // in a panic now (one Panicked event per scare)
  EntityId target = 0;                    // the walker or corpse the current Fight or PutDown is about
  Doing doing = Doing::Idle;
  std::deque<std::pair<int, EntityId>> plan;  // process index (-1 = move to a cell, packed in the id)
  std::int64_t next_decide = 0;
  std::map<Doing, std::int64_t> retry_after;  // a want that just failed waits before trying again
};

class PopAI : public System {
 public:
  PopAI(World& w, SiteMap& map, Assemblies& items, Bodies& bodies, Actors& actors, Power& power, Walkers& walkers);
  const char* name() const override { return "pop_ai"; }
  void update(World& w) override;
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& r) override;

  // Setup: a pop joins the group with default job priorities and drawn traits.
  void add_pop(World& w, EntityId body);
  const std::map<EntityId, PopMind>& minds() const { return minds_; }
  const PopMind& mind(EntityId body) const { return minds_.at(body); }

  // Command view (spec 24 rules 2-3). An order is checked for refusals first; a refused order is
  // not kept and the refusal is logged (PopRefused).
  Refusal give_order(World& w, EntityId body, const Order& order);
  void set_priority(EntityId body, Job job, int priority) { minds_.at(body).priority[static_cast<std::size_t>(job)] = priority; }

  // Possession (rule 8): the possessed pop gets no AI; on release the AI picks up the same body.
  void possess(EntityId body) { possessed_ = body; }
  void release() { possessed_ = 0; }
  EntityId possessed() const { return possessed_; }

  // Would this pop refuse to go to `to` right now? (rule 4)
  Refusal refusal(const World& w, EntityId body, const Cell& to) const;
  // What the group has been told (rule 7): honest pops report a bite, secretive ones hide it.
  bool says_bitten(EntityId body) const { return minds_.at(body).reported; }

 private:
  void decide(World& w, PopMind& m);
  bool plan_want(World& w, PopMind& m, Doing want);
  void send_next(World& w, PopMind& m);
  void cancel(World& w, PopMind& m);
  bool armed(const World& w, EntityId body) const;
  bool carries(const World& w, EntityId body, const std::string& def_id) const;
  std::optional<EntityId> loose(const World& w, const std::string& def_id, const Cell& near) const;
  std::optional<EntityId> loose_weapon(const World& w, const Cell& near) const;
  std::optional<Cell> safe_cell(const World& w, EntityId body, const std::vector<EntityId>& threats) const;
  std::optional<EntityId> window_to_board(const World& w, const Cell& from, EntityId me) const;
  std::optional<EntityId> treatable_wound(const Body& b) const;
  // A walker at grips with another pop near enough to hear the struggle (rule 6: they help each other).
  std::optional<EntityId> ally_in_trouble(const World& w, EntityId me) const;
  // One of the group's dead, not yet risen or put down, that this pop can reach.
  std::optional<EntityId> dead_to_put_down(const World& w, EntityId me) const;
  bool night(const World& w) const;
  bool generator_hours(const World& w) const;  // when the cautious AI runs the generator
  // How many other pops are on this want (and this target, when given): jobs are not doubled up.
  int others_doing(EntityId me, Doing d, EntityId target = 0) const;
  EntityId guard_on_duty(const World& w) const;
  int process(const char* id) const;

  SiteMap& map_;
  Assemblies& items_;
  Bodies& bodies_;
  Actors& actors_;
  Power& power_;
  Walkers& walkers_;
  // State (saved).
  std::map<EntityId, PopMind> minds_;
  EntityId possessed_ = 0;
};

}  // namespace zs
