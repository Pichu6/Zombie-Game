// Projects (construction): an order becomes a plan on a site, money in escrow, work by stages on the site's
// establishment, a contractor paid per stage, and one finished building. Spec: docs/tier0/specs/34-projects.md
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "zs/archive.h"
#include "zs/economy.h"
#include "zs/establishment.h"

namespace zs {

using PlanId = std::uint16_t;

// A plan: the construction recipe (one batch = one stage: the primary material to one stage of the building good,
// the other materials consumed, labour and power per stage), the building good (its bulk is stages, one unit is the
// finished building, so bulk_per_unit is the stages the plan takes), the budget, the contractor's price per stage,
// and what the building runs once finished.
struct PlanDef {
  std::string name;
  RecipeId stages;
  GoodId building;
  std::int64_t budget_cents;
  std::int64_t cost_per_stage_cents;
  OwnerId contractor;
  std::optional<RecipeId> then;
};

enum class StallReason : std::uint8_t { None = 0, NoStaff = 1, NoPower = 2, NoMoney = 3, NoMaterials = 4 };
enum class ProjectState : std::uint8_t { Active = 0, Finished = 1, Cancelled = 2 };

struct Project {
  OwnerId site;
  PlanId plan;
  OwnerId orderer;
  std::int64_t ordered_tick;
  std::int64_t finished_tick = -1;
  std::int64_t produced_at_order = 0;  // the site establishment's batches when the order came
  std::int64_t stages_paid = 0;
  std::int64_t produced_seen = 0;      // for stall detection: the establishment's count after the last advance
  std::int64_t caught_up_seen = 0;
  ProjectState state = ProjectState::Active;
  StallReason stall = StallReason::None;
};

class Projects {
 public:
  PlanId add_plan(const Goods& goods, const Recipes& recipes, PlanDef def);
  const PlanDef& plan(PlanId p) const { return plans_.at(p); }
  std::size_t plans() const { return plans_.size(); }

  enum class OrderResult { Ok, NoSite, SiteBusy, NoBudget };
  // The site must be an establishment; it takes the plan's recipe. The whole budget moves from the orderer into the
  // site's account (escrow) or the order is refused.
  OrderResult order(Ledger& l, const Goods& goods, Money& money, Establishments& estabs, std::int64_t tick,
                    OwnerId site, PlanId plan, OwnerId orderer, CauseId cause);
  // Returns the cents given back to the orderer, or -1 if the site has no active project. Materials and stages stay.
  std::int64_t cancel(Ledger& l, Money& money, Establishments& estabs, std::int64_t tick, OwnerId site, CauseId cause);

  // Before the establishments advance: each active project caps its site's batches by the stages left and the
  // money left (one stage is never started that cannot be paid).
  void before_advance(const Ledger& l, const Goods& goods, const Money& money, Establishments& estabs);

  struct Change {
    enum What { Finished, Stalled, Resumed } what;
    OwnerId site;
    PlanId plan;
    std::int64_t tick;
    StallReason stall;
  };
  // After the establishments advanced: pay the contractor for new stages, finish a project whose stages are all
  // built (the building unit is made, the site runs the plan's `then` recipe), and report stalls and resumptions.
  std::vector<Change> after_advance(Ledger& l, const Goods& goods, const Recipes& recipes, Money& money,
                                    Establishments& estabs, std::int64_t tick, CauseId cause);

  const Project* find(OwnerId site) const;  // the active or latest project on the site
  const std::vector<Project>& all() const { return all_; }

  struct View {
    ProjectState state;
    std::int64_t stages_done;
    std::int64_t stages_total;
    double progress;  // stages_done / stages_total
    StallReason stall;
    std::int64_t money_left;
  };
  View view(const Ledger& l, const Goods& goods, const Money& money, OwnerId site) const;

  void save(ArchiveWriter& w) const;
  void load(ArchiveReader& r);

 private:
  Project* active(OwnerId site);
  std::vector<PlanDef> plans_;
  std::vector<Project> all_;
};

}  // namespace zs
