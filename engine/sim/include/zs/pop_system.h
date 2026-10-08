// The pop registry as a world system: saved, hashed and replayed with the world, changed only through
// commands after setup. Spec: docs/tier0/specs/28-pop-levels.md
#pragma once
#include <cstdint>

#include "zs/body.h"
#include "zs/pop_registry.h"
#include "zs/world.h"

namespace zs {

// Command types for pops. A high range, so they never meet the numbers other systems use.
enum PopCommand : std::uint16_t {
  PopPromote = 0x1001,      // subject = pop
  PopSetOffice = 0x1002,    // subject = pop, a = office (0 clears)
  PopSetFollowed = 0x1003,  // subject = pop, a = 1 or 0
  RosterJoin = 0x1004,      // subject = pop, a = roster
  RosterLeave = 0x1005,     // subject = pop, a = roster
  RosterApply = 0x1006,     // subject = roster, a = EffectKind, b = delta
  CohortApply = 0x1007,     // subject = cohort, a = EffectKind, b = delta
  PopEmbody = 0x1009,       // subject = pop (spec 29)
  PopDisembody = 0x100a,    // subject = pop
  RegionApply = 0x1008,     // subject = region, a = EffectKind | (RosterKind + 1) << 8 (0 in the high byte = all rosters), b = delta
};

// Why a pop command was refused: carried in the PopCommandRefused event's `b` (0 = none given).
enum class Refusal : std::int64_t {
  None = 0,
  NoBodies = 1,       // no body simulation attached
  NoSuchPop = 2,
  Dead = 3,
  AlreadyEmbodied = 4,
  NotEmbodied = 5,
  PoolFull = 6,
  Dwell = 7,          // demoted before the dwell time was up
  BodyBusy = 8,       // Bodies::cannot_disembody said no (walker, away, items held)
};

// Game years are calendar days / 365 (no leap days: the sim has none).
inline double year_of_tick(std::int64_t tick) { return static_cast<double>(tick) / (static_cast<double>(kTicksPerGameDay) * 365.0); }

class PopSystem : public System {
 public:
  explicit PopSystem(World& w, const PopParams& params = default_pop_params()) : registry_(w.rng.seed(), params) {}
  const char* name() const override { return "pops"; }

  // Setup access: allowed only while the world's setup is open. After that, change pops by commands.
  PopRegistry& setup(World& w);
  const PopRegistry& registry() const { return registry_; }

  // Spec 29: links the pool. Setup only, and the same call in every setup (like the pool size, it is not saved).
  void attach_bodies(World& w, Bodies& bodies);
  // The least time a pop stays embodied before it may be demoted (hysteresis); a setting, not saved. Default 1 game hour.
  void set_dwell_ticks(std::int64_t ticks) { dwell_ticks_ = ticks; }

  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;  // audits the registry when the world checks every tick
  // With bodies attached a pop may die any tick; without them the registry only changes by command.
  std::int64_t next_wake(const World&, std::int64_t now) const override { return bodies_ ? now : kNever; }
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

 private:
  void embody(World& w, const Command& c, EventId cause);
  void disembody(World& w, const Command& c, EventId cause);
  void refuse(World& w, const Command& c, EventId cause, Refusal why);
  std::optional<std::string> audit_bodies() const;

  PopRegistry registry_;
  Bodies* bodies_ = nullptr;
  std::int64_t dwell_ticks_ = kTicksPerGameHour;
};

}  // namespace zs
