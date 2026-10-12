// The household after an absence or a lost job (decision 0070 rule 5, spec 63). Each holder who was told of a pop's absence, or
// whose pop lost its job, carries a strain toward it (thousandths) that eases over time; past a limit the holder leaves the
// household roster. Strain is lazy: stored with its tick and decayed when read, so the system never wakes by itself.
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "zs/expectations.h"

namespace zs {

struct BondParams {
  int stage_milli[6] = {0, 0, 50, 100, 200, 300};  // by AbsenceStage: nothing before a commitment broke
  int cause_percent[5] = {100, 30, 150, 100, 50};  // by AbsenceCause: possessed, ill, chose, unknown, wandered
  int job_loss_milli = 150;
  double half_life_days = 30;
  int leave_threshold_milli = 900;
};
BondParams load_bond_params(const std::string& path);

class Bonds : public System, public Expectations::Listener {
 public:
  // `household_of` gives a pop's household roster id (0 for none): the roster a holder leaves.
  Bonds(BondParams params, std::function<std::uint64_t(PopId)> household_of)
      : params_(params), household_of_(std::move(household_of)) {}
  const char* name() const override { return "bonds"; }

  void on_absence_ended(World& w, const Expectations::Case& c, EventId ended) override;
  void on_separated(World& w, const Expectations::Case& c, EventId separation) override;

  void update(World&) override {}
  std::int64_t next_wake(const World&, std::int64_t) const override { return kNever; }
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

  // The strain `holder` carries toward `pop` at `tick`, in thousandths (0 if none).
  int strain(PopId holder, PopId pop, std::int64_t tick) const;
  // A holder who left its household because of a strained bond: it no longer holds anybody's commitments, and nobody at home
  // holds its.
  bool left_household(PopId pop) const;
  const std::vector<PopId>& left() const { return left_; }
  std::size_t bonds() const { return bonds_.size(); }

 private:
  struct Bond {
    PopId holder = 0, pop = 0;
    int milli = 0;            // the strain at `tick`
    std::int64_t tick = 0;
  };
  Bond* find(PopId holder, PopId pop);
  const Bond* find(PopId holder, PopId pop) const;
  int decayed(const Bond& b, std::int64_t tick) const;
  void add(World& w, PopId holder, PopId pop, int milli, EventId cause);
  BondParams params_;
  std::function<std::uint64_t(PopId)> household_of_;  // never saved
  std::vector<Bond> bonds_;
  std::vector<PopId> left_;
};

}  // namespace zs
