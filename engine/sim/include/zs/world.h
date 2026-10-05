// The world: shared simulation state, the fixed update order, commands, save and hash.
// Specs: docs/tier0/specs/05-clock.md, 06-save-replay.md
#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "zs/archive.h"
#include "zs/clock.h"
#include "zs/draw.h"
#include "zs/events.h"
#include "zs/ledger.h"

namespace zs {

// Version history: 1 = stage 1; 2 = adds the entity id counter (stage 2); 3 = adds commands submitted
// but not yet applied (stage 4 review).
inline constexpr std::uint32_t kSaveFormatVersion = 3;

// A player intent, applied at the start of the tick it was submitted for. The AI produces the
// same kind of intents (parity), but only player commands are recorded in the input log.
struct Command {
  std::int64_t tick = 0;
  std::uint16_t type = 0;
  EntityId subject = 0;
  std::int64_t a = 0;
  std::int64_t b = 0;
};

class World;

// A simulation system. Systems run in the order they were added; that order is fixed per build.
class System {
 public:
  virtual ~System() = default;
  virtual const char* name() const = 0;
  virtual void apply_command(World&, const Command&, EventId) {}
  virtual void on_scheduled(World&, const ScheduledEvent&) {}
  virtual void update(World&) = 0;
  virtual void save(ArchiveWriter&) const = 0;
  virtual void load(ArchiveReader&) = 0;
};

class World {
 public:
  explicit World(std::uint64_t seed);

  Rng rng;
  Ledger ledger;
  Locations locations;
  EventLog log;
  Scheduler scheduler;
  Clock clock;

  void add_system(std::unique_ptr<System> system);
  template <class T> T* system() const {
    for (const auto& s : systems_)
      if (auto* t = dynamic_cast<T*>(s.get())) return t;
    return nullptr;
  }

  // Allocates a new entity id. Ids below kFirstEntity are reserved for fixed setup ids.
  static constexpr EntityId kFirstEntity = EntityId{1} << 20;
  EntityId new_entity() { return next_entity_++; }

  // Ends setup: no more minting, logs the run start.
  void close_setup();

  // Queues a player command for the next tick and records it in the input log.
  void submit(Command command);
  // Queues an AI intent for the next tick. Same path as a player command (parity), but not
  // recorded: replays regenerate AI intents from the state.
  void queue_intent(Command command);

  // Advances one tick: commands, systems in order, due scheduled events, ledger check.
  void step();
  void run_ticks(std::int64_t n) { for (std::int64_t i = 0; i < n; ++i) step(); }

  // Writes an event, remembering it as emitted this tick (the app's TimeControl reads these).
  EventId emit(EventType type, EntityId subject, EventId cause, std::int64_t a = 0, std::int64_t b = 0);
  const std::vector<EventId>& events_this_tick() const { return emitted_; }

  const std::vector<Command>& input_log() const { return input_log_; }

  // Ledger check every tick. On by default in debug builds; tests turn it on everywhere.
  bool check_every_tick = kDebugBuild;
  std::optional<std::string> last_violation;

  // Persistence. Load into a candidate world built by the same setup, then swap if it succeeded.
  std::vector<std::uint8_t> save() const;
  void load(const std::vector<std::uint8_t>& bytes);
  std::uint64_t state_hash() const;  // hash of the state part of a save (header excluded)
  // The same without the input log: equal for a player-driven and an AI-driven run of the same
  // intents (spec 14 parity test).
  std::uint64_t world_hash() const;

#ifdef NDEBUG
  static constexpr bool kDebugBuild = false;
#else
  static constexpr bool kDebugBuild = true;
#endif

 private:
  void save_state(ArchiveWriter& w, bool with_input_log = true) const;
  void load_state(ArchiveReader& r, std::uint32_t version);

  std::vector<std::unique_ptr<System>> systems_;
  std::vector<Command> pending_;
  std::vector<Command> input_log_;
  std::vector<EventId> emitted_;
  EntityId next_entity_ = kFirstEntity;
};

}  // namespace zs
