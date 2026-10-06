// The run: "The generator problem" (spec 26). Found survivors, the stranger, the radio goal,
// control after death and the end of the run. Scenario effects reach it through Actors::story_*.
#pragma once
#include <cstdint>
#include <vector>

#include "zs/dead.h"

namespace zs {

class PopAI;

enum class RunOutcome : std::uint8_t { Running, Survived, Rescued, AllDead };
const char* to_string(RunOutcome o);

enum class Role : std::uint8_t { Neighbour, Technician, Stranger };
const char* to_string(Role r);

struct Found {
  Role role = Role::Neighbour;
  EntityId body = 0;
  bool joined = false;
};

class Run : public System {
 public:
  Run(World& w, Bodies& bodies, Actors& actors, Power& power, Census& census, Walkers& walkers, PopAI& ai);
  const char* name() const override { return "run"; }
  void update(World& w) override;
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& r) override;

  // Setup.
  void start(World& w, EntityId player, EntityId radio, std::int64_t stranger_tick, bool stranger_bitten,
             std::int64_t broadcast_tick, std::int64_t end_tick);
  void add_found(Role role, EntityId body) { found_.push_back({role, body, false}); }

  const std::vector<Found>& found() const { return found_; }
  const std::vector<EntityId>& group() const { return group_; }  // in the order they joined
  bool waiting(EntityId body) const;  // a found survivor on the map, not yet in the group
  EntityId controlled() const { return controlled_; }
  bool radio_repaired() const { return radio_repaired_; }
  bool heard() const { return heard_; }
  std::int64_t last_listen_tick() const { return last_listen_; }  // a day before the start: never
  EntityId radio() const { return radio_; }
  std::int64_t stranger_tick() const { return stranger_tick_; }
  bool stranger_bitten() const { return stranger_bitten_; }
  std::int64_t broadcast_tick() const { return broadcast_tick_; }
  RunOutcome outcome() const { return outcome_; }

 private:
  Reason check(EntityId target, const std::string& how) const;
  void apply(World& w, EntityId body, EntityId target, const std::string& how, EventId cause);

  Bodies& bodies_;
  Actors& actors_;
  Power& power_;
  Census& census_;
  Walkers& walkers_;
  PopAI& ai_;
  // Fixed at setup.
  EntityId radio_ = 0;
  std::int64_t stranger_tick_ = 0, broadcast_tick_ = 0, end_tick_ = 0;
  bool stranger_bitten_ = false;
  // State (saved).
  std::vector<Found> found_;
  std::vector<EntityId> group_;
  EntityId controlled_ = 0;
  bool radio_repaired_ = false, heard_ = false;
  std::int64_t last_listen_ = -kTicksPerGameDay;
  RunOutcome outcome_ = RunOutcome::Running;
};

}  // namespace zs
