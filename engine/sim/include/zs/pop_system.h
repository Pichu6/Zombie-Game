// The pop registry as a world system: saved, hashed and replayed with the world, changed only through
// commands after setup. Spec: docs/tier0/specs/28-pop-levels.md
#pragma once
#include <cstdint>

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
  RegionApply = 0x1008,     // subject = region, a = EffectKind | (RosterKind + 1) << 8 (0 in the high byte = all rosters), b = delta
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

  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;  // audits the registry when the world checks every tick
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

 private:
  PopRegistry registry_;
};

}  // namespace zs
