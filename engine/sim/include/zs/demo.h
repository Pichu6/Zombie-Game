// Stage 1 demo scenario: a generator that burns fuel, can break down and is refuelled by player
// commands. It exists only to exercise the foundation units end to end (ledger, conversions,
// hazards, schedule, commands, save and replay). Real systems replace it from stage 2 on.
#pragma once
#include <cstdint>
#include <memory>

#include "zs/world.h"

namespace zs::demo {

enum CommandType : std::uint16_t { StartGenerator = 1, StopGenerator = 2, Refuel = 3, Repair = 4 };

struct Ids {
  ResourceId fuel_ml;       // gasoline, mL
  ResourceId electric_kj;   // electricity delivered, kJ
  OwnerId tank;             // generator tank
  OwnerId cans;             // fuel cans in the building
  OwnerId stash;            // a stash in house B, delivered by a scheduled event
  OwnerId house;            // receives electricity
  ConversionId burn;        // 1 mL gasoline -> 7 kJ electricity (~20 % of 34 kJ/mL)
  EntityId generator = 10;
  EntityId can_from_stash = 1001;
  EntityId building = 1;
};

class GeneratorSystem : public System {
 public:
  explicit GeneratorSystem(Ids ids) : ids_(ids) {}
  const char* name() const override { return "demo.generator"; }
  void apply_command(World& w, const Command& c, EventId cause) override;
  void on_scheduled(World& w, const ScheduledEvent& e) override;
  void update(World& w) override;
  void save(ArchiveWriter& out) const override;
  void load(ArchiveReader& in) override;

  const Ids& ids() const { return ids_; }
  bool running() const { return running_; }
  bool broken() const { return broken_; }

  // Fuel use in mL per game hour at the demo load (3 kW class at half load).
  static constexpr std::int64_t kFuelMlPerHour = 1200;
  // Breakdown hazards while running, per game second: spark plug fouls, engine stalls.
  static constexpr double kSparkPlugRate = 1.0 / (100.0 * 3600.0);
  static constexpr double kStallRate = 1.0 / (40.0 * 3600.0);

 private:
  void stop(World& w, EventType why, EventId cause);

  Ids ids_;
  bool running_ = false;
  bool broken_ = false;
  std::int64_t burn_remainder_ = 0;  // mL x ticks-per-hour not yet burnt (exact integer flow)
  EventId started_by_ = 0;
};

// Builds the demo world: resources, owners, stocks, the generator system and a scheduled delivery.
std::unique_ptr<World> make_world(std::uint64_t seed);
Ids ids_of(const World& w);

// A scripted player: runs the generator in the evening, refuels when low, repairs breakdowns.
// It only submits commands, so a replay of its input log reproduces the run.
void scripted_player(World& w);

}  // namespace zs::demo
