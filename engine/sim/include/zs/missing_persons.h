// Real people go missing without any player (decision 0070 rule 6, spec 60): each tracked pop is a Poisson process of
// absences at a rate per head by age class, each lasting a drawn time (most come back within two days, a few never do
// within the run). The system only issues AbsenceBegin and AbsenceEnd; the Expectations system runs the household's
// ladder, so a player's possession hides among these.
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "zs/expectations.h"

namespace zs {

struct MissingParams {
  double adult_per_1000_year = 1.8, child_per_1000_year = 5.6;
  int child_age_max = 17;
  // (hours, share returned by then), increasing; what is left after the last knot never returns within the run.
  std::vector<std::pair<double, double>> adult_return, child_return;
  double adult_chose = 0.16, adult_wandered = 0.086, child_chose = 0.71, child_wandered = 0.0;  // the rest: unknown
  double rate_scale = 1.0;  // a sandbox option: more or fewer disappearances (not saved: the same in every setup)
};
// data/life/missing.toml: the table `id`, or the one the income group's [index] names when `id` has no table of its own.
MissingParams load_missing_params(const std::string& path, const std::string& country_profile, const std::string& income);

class MissingPersons : public System {
 public:
  // `exp` is the ladder's system (to see who is already absent); `eligible` says whether a pop can go missing now.
  MissingPersons(MissingParams params, const Expectations* exp, std::function<bool(PopId)> eligible)
      : params_(std::move(params)), exp_(exp), eligible_(std::move(eligible)) {}
  const char* name() const override { return "missing"; }

  // Setup only: starts tracking a pop (its first absence is drawn from tick `from`).
  void track(World& w, PopId pop, int age, std::int64_t from = 0);
  void set_rate_scale(double scale) { params_.rate_scale = scale; }
  // Setup or a sandbox option changing the scale: draws every idle pop's next absence again from `from`.
  void redraw(World& w, std::int64_t from);

  void update(World& w) override;
  std::int64_t next_wake(const World&, std::int64_t now) const override;
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

  struct Entry {
    PopId pop = 0;
    bool child = false;
    std::int64_t draws = 0;       // draws spent so far: the next draw's index
    std::int64_t next_start = 0;  // the next absence begins at or after this tick (while not away)
    std::int64_t away_until = 0;  // 0: not away by this system; kNever: away for good (long-term missing)
    std::int64_t incidents = 0;
  };
  const std::vector<Entry>& entries() const { return entries_; }

 private:
  std::int64_t gap(const World& w, const Entry& e) const;       // ticks to the next absence, from draw `e.draws`
  std::int64_t duration(const World& w, const Entry& e) const;  // ticks away, from draw `e.draws`; kNever if long-term
  AbsenceCause cause(const World& w, const Entry& e) const;
  MissingParams params_;
  const Expectations* exp_;
  std::function<bool(PopId)> eligible_;  // set by the setup, never saved
  std::vector<Entry> entries_;
};

}  // namespace zs
