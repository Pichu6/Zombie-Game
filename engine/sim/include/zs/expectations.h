// Expectations: broken commitments are noticed (decision 0070, spec 59). A pop's schedule (spec 54) marks commitments
// somebody expects: a shift start, arriving home. While a pop keeps its plan nothing happens and nothing costs; when a
// pop is absent (a possession, an illness, a choice) and a commitment falls due, the holders react on one ladder:
// phone, ask around, report missing, search. Every step is an event whose cause chain leads back to the absence.
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
enum class AbsenceCause : std::uint8_t { Possessed = 0, Ill = 1, Chose = 2, Unknown = 3, kCount = 4 };

// One commitment as the schedule names it: when it falls due and who expects it, in the order they act (the first
// phones and asks around, the last reports).
struct Commitment {
  CommitmentKind kind = CommitmentKind::ArriveHome;
  std::int64_t due_tick = 0;
  std::vector<PopId> holders;
};
// The next commitment of a pop due at or after `from_tick`, or none (nobody expects it anywhere: nobody notices).
using NextCommitment = std::function<std::optional<Commitment>(PopId pop, std::int64_t from_tick)>;

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

class Expectations : public System {
 public:
  Expectations(ExpectationParams params, NextCommitment next) : params_(params), next_(std::move(next)) {}
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
    std::int64_t next_tick = 0;    // the next step; kNever when none
    std::vector<PopId> holders;
    EventId begin_event = 0, last_event = 0;
  };
  const std::vector<Case>& cases() const { return cases_; }  // the open absences, in the order they began
  const Case* find(PopId pop) const;

 private:
  void refuse(World& w, const Command& c, EventId cause, std::int64_t why);
  ExpectationParams params_;
  NextCommitment next_;  // set by the setup, never saved
  std::vector<Case> cases_;
};

}  // namespace zs
