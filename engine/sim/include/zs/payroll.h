// Payroll: what a job means for money and labour (spec 62). Every shift of a pop with a job is paid by its employer when it
// starts, unless the pop is absent (then the shift is unpaid); a separation (spec 61) ends the pay and takes the pop off the
// employer's workplace roster, so any establishment staffed from that roster has one worker fewer.
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "zs/expectations.h"
#include "zs/economy.h"

namespace zs {

struct WageParams {
  double cents_per_hour = 1000;
};
// data/employment/wages.toml: the table `country_profile`, or the one the income group's [index] names.
WageParams load_wage_params(const std::string& path, const std::string& country_profile, const std::string& income);

class Payroll : public System {
 public:
  // `employer` pays; `owner_of` gives a pop's wallet owner; `roster` is the employer's workplace roster. `next` and `exp` are
  // the schedule's commitments and the absence system (to see who is absent and who lost the job); neither is saved.
  Payroll(WageParams params, const Expectations* exp, NextCommitment next, OwnerId employer, std::uint64_t roster,
          std::function<OwnerId(PopId)> owner_of)
      : params_(params), exp_(exp), next_(std::move(next)), employer_(employer), roster_(roster), owner_of_(std::move(owner_of)) {}
  const char* name() const override { return "payroll"; }

  // Setup only: the pop has a job (and is on the roster); its first shift is looked up from tick 0.
  void track(PopId pop);

  void update(World& w) override;
  std::int64_t next_wake(const World&, std::int64_t now) const override;
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

  struct Entry {
    PopId pop = 0;
    bool active = true;               // false once the pop left the employer
    std::int64_t next_due = 0;        // the next shift start the employer will pay (or withhold); kNever when none
    int minutes = 0;                  // the shift's length
    std::int64_t ordered_cents = 0;   // wages ordered paid so far
    std::int64_t shifts_paid = 0, shifts_unpaid = 0;
  };
  const std::vector<Entry>& entries() const { return entries_; }
  std::size_t headcount() const;            // pops still employed
  std::int64_t wages_ordered() const;       // over every pop, so far
  std::int64_t shifts_paid_total() const;   // shifts the employer paid, over every pop
  const Entry* find(PopId pop) const;

 private:
  void schedule(Entry& e, std::int64_t from) const;
  WageParams params_;
  const Expectations* exp_;
  NextCommitment next_;
  OwnerId employer_;
  std::uint64_t roster_;
  std::function<OwnerId(PopId)> owner_of_;
  std::vector<Entry> entries_;
  std::size_t seen_lost_ = 0;  // how many of the absence system's lost jobs this system has acted on
};

}  // namespace zs
