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
  Intent = 1,  // a command from a controller, player or AI (subject = actor, a = command type, b = a)
  Scheduled = 2,
  Alarm = 3,
  AttackOnPop = 4,
  NewWound = 5,
  Breakdown = 6,
  NewSymptom = 7,
  SurvivorFound = 8,
  Death = 9,             // a = DeathCause, b = 1 if the body was a walker
  InfectionVisible = 10,  // subject = body, a = wound id
  InfectionWorse = 11,    // a = wound id, b = new stage
  WoundHealed = 12,       // a = wound id
  FellAsleep = 13,
  WokeUp = 14,
  ActionDone = 15,         // subject = actor, a = process index, b = target
  ActionInterrupted = 16,  // a = process index, b = Reason
  ActionRefused = 17,      // a = process index, b = Reason
  GridFailed = 18,         // the mains are dead from now on
  GeneratorStalled = 19,   // a = StallCause
  BreakerTripped = 20,     // a = circuit index
  FoodSpoiled = 21,        // subject = the fridge, a = J spoiled
  GeneratorOutOfFuel = 22,
  Reanimated = 23,        // subject = body, now a walker
  WalkerArrived = 24,     // subject = walker body, a = census person index
  Bite = 25,              // subject = walker, a = victim body, b = Region
  Grabbed = 26,           // subject = walker, a = victim body
  PartFailed = 27,        // subject = the failed part, a = the edge assembly
  EdgeBreached = 28,      // subject = the edge assembly
  Hit = 29,               // subject = attacker, a = target body, b = 1 if it destroyed the brain
  PopRefused = 30,        // subject = pop, a = Refusal, b = Doing it refused (spec 24 rule 4)
  BiteReported = 31,      // subject = pop that told the group it was bitten (spec 24 rule 7)
  Panicked = 32,          // subject = pop that dropped what it did and ran
  // Stage 9: the run (spec 26).
  Joined = 33,            // subject = found survivor who joined, a = the pop that invited
  StrangerArrived = 34,   // subject = the stranger, at the yard gate
  RadioRepaired = 35,     // subject = who repaired it
  RadioHeard = 36,        // subject = who heard the evacuation broadcast (the better ending)
  RadioStatic = 37,       // subject = who listened before the broadcast
  ControlPassed = 38,     // subject = the newly controlled pop, a = the dead one
  RunEnded = 39,          // a = RunOutcome
  Searched = 40,          // subject = pop, a = the home object it searched
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
