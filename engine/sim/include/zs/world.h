// The world: shared simulation state, the fixed update order, commands, save and hash.
// Specs: docs/tier0/specs/05-clock.md, 06-save-replay.md
#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "zs/archive.h"
#include "zs/buggify.h"
#include "zs/clock.h"
#include "zs/draw.h"
#include "zs/events.h"
#include "zs/ledger.h"

namespace zs {

// Version history: 1 = stage 1; 2 = adds the entity id counter (stage 2); 3 = adds commands submitted
// but not yet applied (stage 4 review).
// 6: the header carries the codec and the uncompressed size, the state may be zstd-compressed (spec 06 rules 16-17);
// 5: entity ids minted per creator (decision 0065, spec 27); 4: actors' heading and steering (spec 50)
inline constexpr std::uint32_t kSaveFormatVersion = 6;

// How a save's state is stored. Hashes and goldens are always over the uncompressed state.
enum class SaveCodec : std::uint32_t { None = 0, Zstd = 1 };

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
  // Spec 40: the first tick at or after `now` at which this system has work to do. A system that steps every tick
  // (bodies, actors, fields) returns `now`; a cold system returns its next cadence boundary, or kNever if it only
  // reacts to commands. World::jump skips the ticks nobody needs.
  static constexpr std::int64_t kNever = std::int64_t{1} << 62;
  virtual std::int64_t next_wake(const World&, std::int64_t now) const { return now; }
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
  // The systems in update order (spec 55: the Atlas's simulation card lists them with their next wake tick).
  const std::vector<std::unique_ptr<System>>& systems() const { return systems_; }
  template <class T> T* system() const {
    for (const auto& s : systems_)
      if (auto* t = dynamic_cast<T*>(s.get())) return t;
    return nullptr;
  }

  // Entity ids (decision 0065 rule 1): minted per creator from that creator's own counter, so two creators' ids never
  // depend on each other's order (what local rollback and parallel chunks need, RA-114). An id is the creator in the
  // top 16 bits and the creator's sequence (from 1) in the low 48. Ids below kFirstEntity are fixed setup ids; the
  // Legacy creator (0) is the single global counter of saves before format 5 and mints nothing new. When chunks
  // exist, a chunk becomes the creator.
  enum class Creator : std::uint16_t { Legacy = 0, Items = 1, Bodies = 2, Wounds = 3, Actions = 4, kCount = 5 };
  static constexpr EntityId kFirstEntity = EntityId{1} << 20;
  static constexpr int kCreatorShift = 48;
  EntityId new_entity(Creator by) {
    const auto k = static_cast<std::size_t>(by);
    return (static_cast<EntityId>(by) << kCreatorShift) | ++creator_counters_[k];
  }
  static constexpr Creator creator_of(EntityId e) { return static_cast<Creator>(e >> kCreatorShift); }
  std::uint64_t creator_count(Creator by) const { return creator_counters_[static_cast<std::size_t>(by)]; }

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
  // Spec 40: advances to `to_tick` stepping only the ticks at which some system, a pending command or a scheduled
  // event has work; the ticks between are skipped. With every system cold this is one step per hour or day; with
  // a body attended it is every tick. The result equals stepping tick by tick, event log included.
  void jump(std::int64_t to_tick);
  // The next tick at or after `now` at which anything has work (commands, systems, scheduled events).
  std::int64_t next_wake(std::int64_t now) const;

  // Writes an event, remembering it as emitted this tick (the app's TimeControl reads these).
  EventId emit(EventType type, EntityId subject, EventId cause, std::int64_t a = 0, std::int64_t b = 0);
  const std::vector<EventId>& events_this_tick() const { return emitted_; }

  const std::vector<Command>& input_log() const { return input_log_; }

  // Ledger check every tick. On by default in debug builds; tests turn it on everywhere.
  // Fault injection sites (spec 57): off unless a test sets a seed; not state, never saved or hashed.
  Buggify buggify;

  bool check_every_tick = kDebugBuild;
  std::optional<std::string> last_violation;
  // Times step() found the floating-point environment changed (rounding, flush-to-zero, denormals-are-zero) and
  // restored the default (engine architecture section 5). Not state: not saved or hashed.
  std::uint64_t fp_env_restored = 0;

  // Probes (spec 56, ranked items 1 and 67): per-system timers and per-system hashes. Never saved, never hashed,
  // never read by the simulation (sensors-and-soundscape section 1).
  struct SystemProbe {
    std::uint64_t updates = 0;  // calls of update() counted
    std::uint64_t ns = 0;       // wall-clock nanoseconds inside update(), when probe_timing is on
  };
  std::vector<SystemProbe> probes;  // one per system, in update order; grows with add_system
  bool probe_timing = false;        // on: step() times each system's update (tens of nanoseconds per zone)
  // The hash of each system's saved state, plus "core" first (clock, entities, rng, ledger, locations, log,
  // scheduler, commands): two worlds that should agree are compared system by system to find the first divergent
  // tick and the system that diverged (Factorio and Age of Empires, HST-126, HST-127).
  struct SystemHash {
    std::string name;
    std::uint64_t hash = 0;
  };
  std::vector<SystemHash> system_hashes() const;

  // Persistence. Load into a candidate world built by the same setup, then swap if it succeeded.
  // Zstd level 3 for saves, 1 for autosaves and traces (spec 06 rule 17); None for tests that want the raw bytes.
  std::vector<std::uint8_t> save(SaveCodec codec = SaveCodec::Zstd, int level = 3) const;
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
  void save_core(ArchiveWriter& w, bool with_input_log) const;  // save_state without the systems
  void load_state(ArchiveReader& r, std::uint32_t version);

  std::vector<std::unique_ptr<System>> systems_;
  std::vector<Command> pending_;
  std::vector<Command> input_log_;
  std::vector<EventId> emitted_;
  std::uint64_t creator_counters_[static_cast<std::size_t>(Creator::kCount)] = {kFirstEntity, 0, 0, 0, 0};  // Legacy holds the old counter
};

}  // namespace zs
