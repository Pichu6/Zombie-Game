// Events with causes, the event log and the schedule. Spec: docs/tier0/specs/04-hazard-event.md
#pragma once
#include <cstdint>
#include <map>
#include <vector>

#include "zs/draw.h"

namespace zs {

using EventId = std::uint64_t;  // 0 = no event

// Fixed list of event types. Append only.
enum class EventType : std::uint16_t {
  RunStart = 0,
  PlayerCommand = 1,
  Scheduled = 2,
  Alarm = 3,
  AttackOnPop = 4,
  NewWound = 5,
  Breakdown = 6,
  NewSymptom = 7,
  SurvivorFound = 8,
  Death = 9,             // a = DeathCause
  InfectionVisible = 10,  // subject = body, a = wound id
  InfectionWorse = 11,    // a = wound id, b = new stage
  WoundHealed = 12,       // a = wound id
  FellAsleep = 13,
  WokeUp = 14,
  // Stage 1 demo scenario events.
  DemoGeneratorStarted = 100,
  DemoGeneratorStopped = 101,
  DemoOutOfFuel = 102,
  DemoRefuelDelivery = 103,
};

struct Event {
  EventId id = 0;
  std::int64_t tick = 0;
  EventType type = EventType::RunStart;
  EntityId subject = 0;
  EventId cause = 0;
  std::int64_t a = 0;  // small payload, meaning set by the type
  std::int64_t b = 0;
};

// Append-only during a run. The simulation never reads it to decide anything.
class EventLog {
 public:
  EventId append(std::int64_t tick, EventType type, EntityId subject, EventId cause, std::int64_t a = 0,
                 std::int64_t b = 0);
  const Event* get(EventId id) const;
  std::vector<Event> cause_chain(EventId id) const;  // the event, then its cause, back to the root
  const std::vector<Event>& all() const { return events_; }
  void restore(std::vector<Event> events) { events_ = std::move(events); }

 private:
  std::vector<Event> events_;  // ids are 1-based positions
};

// Known-time outcomes, drawn once and scheduled.
struct ScheduledEvent {
  std::int64_t tick;
  EventType type;
  EntityId subject;
  EventId cause;
  std::int64_t a;
  std::int64_t b;
};

class Scheduler {
 public:
  void schedule(const ScheduledEvent& e);
  // Removes and returns everything due at or before `tick`, in (tick, insertion order).
  std::vector<ScheduledEvent> pop_due(std::int64_t tick);
  const std::multimap<std::pair<std::int64_t, std::uint64_t>, ScheduledEvent>& pending() const { return queue_; }
  std::uint64_t next_sequence() const { return sequence_; }
  void restore(std::multimap<std::pair<std::int64_t, std::uint64_t>, ScheduledEvent> q, std::uint64_t seq) {
    queue_ = std::move(q);
    sequence_ = seq;
  }

 private:
  std::multimap<std::pair<std::int64_t, std::uint64_t>, ScheduledEvent> queue_;
  std::uint64_t sequence_ = 0;
};

}  // namespace zs
