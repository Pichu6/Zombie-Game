// The police's missing-person case (spec 64, decisions 0069 rule 3 and 0070 rule 3): an institution as a queue with staff.
// A reported pop opens a case with a risk tier; cases wait for a free search (high risk first, then oldest); the search opens
// when the force has staff; the case resolves when the pop is found. The open cases are the national record: a check on a
// pop in any town hits it.
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "zs/expectations.h"

namespace zs {

enum class RiskTier : std::uint8_t { High = 0, Medium = 1, Low = 2 };
enum class FoundBy : std::uint8_t { OwnWill = 0, Police = 1, Other = 2 };
const char* risk_name(RiskTier t);

struct PoliceParams {
  double adult_tiers[3] = {0.31, 0.58, 0.11};  // high, medium, low (AB-15)
  double child_tiers[3] = {0.14, 0.83, 0.03};
  int child_age_max = 17;
  double found_by[3] = {0.39, 0.33, 0.28};    // own will, police, other (AB-24)
  bool wandered_is_high = true;
  int searches_at_once = 2;                    // a default
};
// data/institutions/police/missing.toml: the table `country_profile`, or the one the income group's [index] names.
PoliceParams load_police_params(const std::string& path, const std::string& country_profile, const std::string& income);

class PoliceCases : public System, public Expectations::Listener {
 public:
  // `exp` is the ladder's system (the police open its search); `is_child` says whether a pop is a child.
  PoliceCases(PoliceParams params, Expectations* exp, std::function<bool(PopId)> is_child)
      : params_(params), exp_(exp), is_child_(std::move(is_child)) {}
  const char* name() const override { return "police"; }

  void on_reported(World& w, const Expectations::Case& c, EventId reported) override;
  void on_absence_ended(World& w, const Expectations::Case& c, EventId ended) override;
  void on_absence_begun(World& w, const Expectations::Case& c, EventId began) override;

  // Spec 65: staff. `capacity` is how many searches the force can run now (its officers on duty and present); `next_change`
  // is the next tick after `now` it may change. Without them the force runs the table's `searches_at_once` always.
  using Capacity = std::function<int(const World&)>;
  using NextChange = std::function<std::int64_t(const World&, std::int64_t now)>;
  void set_staffing(Capacity capacity, NextChange next_change) {
    capacity_ = std::move(capacity);
    next_change_ = std::move(next_change);
  }
  int capacity(const World& w) const { return capacity_ ? capacity_(w) : params_.searches_at_once; }

  void update(World& w) override;
  std::int64_t next_wake(const World& w, std::int64_t now) const override;
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

  struct Case {
    PopId pop = 0;
    std::int64_t opened = 0;
    RiskTier tier = RiskTier::Medium;
    bool searching = false;
    std::int64_t serial = 0;  // the case's number: part of its draws
  };
  const std::vector<Case>& cases() const { return cases_; }  // the open cases, in the order opened
  const Case* find(PopId pop) const;
  bool has_record(PopId pop) const { return find(pop) != nullptr; }  // the national record: any force's check hits it
  int active_searches() const;
  int queued() const;
  std::int64_t opened_total() const { return opened_total_; }
  std::int64_t resolved_total() const { return resolved_total_; }

 private:
  RiskTier draw_tier(const World& w, const Expectations::Case& c, std::int64_t serial) const;
  FoundBy draw_found_by(const World& w, PopId pop, std::int64_t serial) const;
  int rank_of(const Case& c) const;  // 1-based place in the queue among the cases waiting
  void start_searches(World& w);
  void reevaluate(World& w);  // the staff changed: suspend the searches it can no longer run, start the ones it can
  PoliceParams params_;
  Expectations* exp_;
  std::function<bool(PopId)> is_child_;  // never saved
  Capacity capacity_;                    // never saved
  NextChange next_change_;               // never saved
  std::int64_t next_eval_ = 0;           // the next tick the staff may change (never saved: after a load the first update looks again)
  std::vector<Case> cases_;
  std::int64_t opened_total_ = 0, resolved_total_ = 0;
};

}  // namespace zs
