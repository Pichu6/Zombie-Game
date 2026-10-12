// Expectations: broken commitments are noticed (decision 0070, specs 59 and 61). A pop's schedule (spec 54) marks
// commitments somebody expects: a shift start, arriving home. While a pop keeps its plan nothing happens and nothing
// costs; when a pop is absent (a possession, an illness, a choice) and a commitment falls due, the holders react on one
// ladder: phone, ask around, report missing, search. The employer counts the shifts missed without contact: after N
// scheduled days it records a separation (or opens a disciplinary process); a missing-person report pauses the count, and
// a return meets the employer. Every step is an event whose cause chain leads back to the absence.
#pragma once
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "zs/pop.h"
#include "zs/world.h"

namespace zs {

enum class CommitmentKind : std::uint8_t { ShiftStart = 0, ArriveHome = 1, kCount = 2 };
const char* commitment_name(CommitmentKind k);

// Why a pop is absent (the AbsenceBegin command's `a`).
enum class AbsenceCause : std::uint8_t { Possessed = 0, Ill = 1, Chose = 2, Unknown = 3, Wandered = 4, kCount = 5 };  // Wandered: lost, confused (AB-36)

// Who employs a pop with a shift: nobody, a private employer, or the police (a conduct process, not a separation).
enum class EmployerKind : std::uint8_t { None = 0, Private = 1, Police = 2 };

// One commitment as the schedule names it: when it falls due and who expects it, in the order they act (the first
// phones and asks around, the last reports). A shift start also names the employer; a pop nobody lives with has no
// holders but may still have an employer.
struct Commitment {
  CommitmentKind kind = CommitmentKind::ArriveHome;
  std::int64_t due_tick = 0;
  std::vector<PopId> holders;
  EmployerKind employer = EmployerKind::None;  // of a shift start
  int minutes = 0;                             // of a shift start: how long the shift lasts (what the employer pays for)
};
// The next commitment of a pop due at or after `from_tick`, or none (nobody expects it anywhere: nobody notices).
using NextCommitment = std::function<std::optional<Commitment>(PopId pop, std::int64_t from_tick)>;
// The next shift start (a commitment with an employer) at or after `from`, skipping other kinds; none if there is none soon.
std::optional<Commitment> next_shift_of(const NextCommitment& next, PopId pop, std::int64_t from);

// Timings per commitment kind, in minutes (data/life/expectations.toml). Defaults taken by Claude.
struct LadderTimes {
  int tolerance_min = 30;
  int ask_after_min = 120;     // from the broken commitment
  int report_after_min = 360;  // from the broken commitment
  int search_after_min = 60;   // from the report
};
struct ExpectationParams {
  LadderTimes times[static_cast<int>(CommitmentKind::kCount)];
};
ExpectationParams load_expectation_params(const std::string& path);

// The employer's absence rule (spec 61, data/employment/absence_policy.toml): after `abandonment_days` consecutive
// scheduled days missed without contact the employer records a separation, or where the country has no such rule opens a
// disciplinary process; a missing-person report pauses the count (AB-5 to AB-8). Police employers open a conduct case
// after the return instead of separating.
struct EmployerPolicy {
  int abandonment_days = 3;
  bool automatic_separation = true;
  bool pause_on_report = true;
  bool police_conduct_case = true;
};
EmployerPolicy load_employer_policy(const std::string& path, const std::string& country_profile, const std::string& income);

// Commands (a high range of their own).
enum ExpectationCommand : std::uint16_t {
  AbsenceBegin = 0x1101,  // subject = pop, a = AbsenceCause, b = 1 if the pop phoned it in (an ordinary absence: no ladder)
  AbsenceEnd = 0x1102,    // subject = pop: it is back
};

// Where an open absence stands. Append only: the index is saved.
enum class AbsenceStage : std::uint8_t {
  Ordinary = 0,   // phoned in, or nobody expects the pop anywhere: nothing will be noticed
  Waiting = 1,    // a commitment will fall due
  Broken = 2,     // it fell due; the first holder has phoned
  Asked = 3,      // friends, colleagues and hospitals have been asked
  Reported = 4,   // reported missing
  Searching = 5,  // a search has opened
};

// What the return-to-work meeting concludes (ReturnedToWork's `a`).
enum class ReturnOutcome : std::int64_t { Forgiven = 0, Warning = 1, Disciplinary = 2 };

class Expectations : public System {
 public:
  Expectations(ExpectationParams params, NextCommitment next, EmployerPolicy policy = {})
      : params_(params), policy_(policy), next_(std::move(next)) {}
  const char* name() const override { return "expectations"; }
  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;
  std::int64_t next_wake(const World&, std::int64_t now) const override;
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

  struct Case {
    PopId pop = 0;
    std::int64_t began = 0;
    AbsenceCause cause = AbsenceCause::Unknown;
    AbsenceStage stage = AbsenceStage::Ordinary;
    CommitmentKind kind = CommitmentKind::ArriveHome;
    std::int64_t broken_tick = 0;  // when the commitment was found broken (due + tolerance)
    std::int64_t next_tick = 0;    // the next step of the ladder; kNever when none
    std::vector<PopId> holders;
    EventId begin_event = 0, last_event = 0;
    // The employer's count (spec 61).
    EmployerKind employer = EmployerKind::None;  // None: no employer expects this pop's shifts
    std::int64_t shift_due = 0;                  // the next shift start the employer will count
    std::int64_t shift_check = 0;                // when it is counted (due + tolerance); kNever when none
    int missed = 0;                              // consecutive scheduled days missed without contact
    bool paused = false, separated = false, disciplined = false;
    EventId employer_event = 0;
  };
  // Who is told when an absence ends and when an employer separates (spec 63: the household's bonds). Set by the setup,
  // never saved. A listener may emit events and queue intents; it must not change the case.
  struct Listener {
    virtual ~Listener() = default;
    virtual void on_absence_ended(World&, const Case&, EventId /*ended*/) {}
    virtual void on_separated(World&, const Case&, EventId /*separation*/) {}
    virtual void on_reported(World&, const Case&, EventId /*reported*/) {}  // the pop was reported missing (spec 64)
  };
  void add_listener(Listener* listener) { listeners_.push_back(listener); }
  // Spec 64: the police own the search. With this on, the ladder no longer opens a search by itself an hour after the report;
  // start_search does it when the police have staff for it.
  void set_external_search(bool on) { external_search_ = on; }
  void start_search(World& w, PopId pop);
  const std::vector<Case>& cases() const { return cases_; }  // the open absences, in the order they began
  const Case* find(PopId pop) const;
  // A pop whose employer separated from it: its schedule has no more shifts (the provider reads this).
  bool lost_job(PopId pop) const;
  const std::vector<PopId>& lost_jobs() const { return lost_jobs_; }

 private:
  void refuse(World& w, const Command& c, EventId cause, std::int64_t why);
  void employer_step(World& w, Case& c);
  void meet_employer(World& w, const Case& c, EventId after);
  std::optional<Commitment> next_shift(PopId pop, std::int64_t from) const;
  ExpectationParams params_;
  EmployerPolicy policy_;
  NextCommitment next_;  // set by the setup, never saved
  std::vector<Case> cases_;
  std::vector<PopId> lost_jobs_;
  std::vector<Listener*> listeners_;  // never saved
  bool external_search_ = false;      // never saved
};

}  // namespace zs
