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
  // Spec 28: pop levels.
  PopEffect = 41,         // subject = cohort or roster id, a = EffectKind, b = delta (the reason is the cause event)
  PopCommandRefused = 42, // subject = what it was aimed at, a = the command type
  // Spec 29: embodied pops.
  PopEmbodied = 43,       // subject = pop, a = the body (pool slot), b = 1 if restored from a saved state
  PopDisembodied = 44,    // subject = pop, a = the body it left, b = 1 if it died while embodied
  PopDiedDemoted = 45,  // subject = pop; a = DeathCause, b = the tick: the body jump found it dead (spec 29 rule 6)
  // Spec 32: stocks, goods and money.
  EconTraded = 46,      // subject = buyer, a = seller, b = units bought (the good and price are in the Intent)
  EconPaid = 47,        // subject = receiver, a = payer, b = cents
  EconSpoiled = 48,     // subject = 0 (all lots), a = bulk amount moved to the decay sink this jump
  EconRefused = 49,     // subject = what it was aimed at, a = the command type, b = EconRefusal
  EconProduced = 50,    // subject = the establishment's owner (or the pop working by hand), a = recipe, b = batches
  // Spec 34: projects.
  ProjectOrdered = 51,   // subject = site, a = plan, b = orderer
  ProjectFinished = 52,  // subject = site, a = plan, b = the tick
  ProjectStalled = 53,   // subject = site, a = StallReason
  ProjectResumed = 54,   // subject = site
  ProjectCancelled = 55, // subject = site, a = cents returned to the orderer
  // Spec 35: networks above building scale.
  GridChanged = 56,      // subject = network id, a = served W, b = unserved W (after a solve that changed them)
  GridRefused = 57,      // subject = what it was aimed at, a = the command type, b = GridRefusal
  // Spec 37: flora and fauna.
  BiotaTaken = 58,       // subject = the taker, a = population index, b = mass moved (the serial is in the Intent)
  // Spec 39: orders and belief.
  EdictSigned = 59,      // subject = edict index, a = def, b = signer; the root cause of everything the edict does
  EdictApplied = 60,     // subject = edict index; cause = its signing; the transfers of this application descend from it
  EdictRepealed = 61,    // subject = edict index
  OrderRefused = 62,     // subject = what it was aimed at, a = the command type, b = OrderRefusal
  Broadcast = 63,        // subject = the thing told (an edict entity), a = how many heard
  Told = 64,             // subject = the listener pop, a = the teller, b = the thing
  // Spec 59: broken commitments are noticed (decision 0070).
  AbsenceBegan = 65,       // subject = pop, a = AbsenceCause, b = 1 if phoned in (an ordinary absence); the root of the ladder
  CommitmentBroken = 66,   // subject = pop, a = CommitmentKind, b = the first holder
  HolderPhoned = 67,       // subject = the holder, a = the pop, b = 0 (no answer)
  HolderAskedAround = 68,  // subject = the holder, a = the pop, b = how many holders were asked
  MissingReported = 69,    // subject = the pop, a = who reported
  SearchStarted = 70,      // subject = the pop
  AbsenceEnded = 71,       // subject = pop, a = AbsenceStage reached, b = ticks gone
  CaseClosed = 72,         // subject = pop, a = ticks the missing-person case was open
  ExpectationRefused = 73, // subject = pop, a = the command type, b = 1 already absent, 2 not absent
  // Spec 61: the employer's absence rule.
  ShiftMissed = 74,        // subject = pop, a = consecutive scheduled days missed so far, b = EmployerKind
  Separated = 75,          // subject = pop, a = days missed: the employer records a separation (job abandonment)
  DisciplinaryOpened = 76, // subject = pop, a = days missed: a process, where the country has no automatic separation
  ConductCaseOpened = 77,  // subject = pop, a = days missed: a police employer's conduct process after the return
  ReturnedToWork = 78,     // subject = pop, a = ReturnOutcome (0 forgiven, 1 warning, 2 disciplinary), b = days missed
  // Spec 62: money and labour.
  ShiftUnpaid = 79,        // subject = pop, a = cents withheld, b = minutes of the shift: the pop was absent, so the shift is not paid
  LeftEmployer = 80,       // subject = pop, a = the employer owner, b = cents ordered paid in all: the pop left the workplace roster
  // Spec 63: the household after an absence or a lost job.
  BondStrained = 81,       // subject = the holder, a = the pop the bond is toward, b = the strain now, in thousandths
  LeftHousehold = 82,      // subject = the holder who left the household roster, a = the pop that strained it, b = the strain in thousandths
  // Spec 64: the police case of a missing person.
  PoliceCaseOpened = 83,   // subject = pop, a = risk tier (0 high, 1 medium, 2 low), b = the queue position (0: staffed at once)
  PoliceCaseResolved = 84, // subject = pop, a = who found it (0 own free will, 1 police, 2 other), b = ticks the case was open
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
