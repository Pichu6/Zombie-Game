// Spec: docs/tier0/specs/34-projects.md
#include "zs/project.h"

#include <algorithm>
#include <stdexcept>

namespace zs {

PlanId Projects::add_plan(const Goods& goods, const Recipes& recipes, PlanDef def) {
  if (def.budget_cents < 0 || def.cost_per_stage_cents < 0) throw std::invalid_argument("Projects::add_plan: money must not be negative");
  if (def.stages >= recipes.size()) throw std::invalid_argument("Projects::add_plan: unknown recipe");
  if (def.building >= goods.size()) throw std::invalid_argument("Projects::add_plan: unknown building good");
  const RecipeDef& r = recipes.def(def.stages);
  if (r.output != def.building || r.output_per_batch != 1) throw std::invalid_argument("Projects::add_plan: the recipe must make one stage of the building per batch");
  plans_.push_back(std::move(def));
  return static_cast<PlanId>(plans_.size() - 1);
}

Project* Projects::active(OwnerId site) {
  for (Project& p : all_) if (p.site == site && p.state == ProjectState::Active) return &p;
  return nullptr;
}

const Project* Projects::find(OwnerId site) const {
  const Project* latest = nullptr;
  for (const Project& p : all_) {
    if (p.site != site) continue;
    if (p.state == ProjectState::Active) return &p;
    latest = &p;
  }
  return latest;
}

Projects::OrderResult Projects::order(Ledger& l, const Goods& goods, Money& money, Establishments& estabs,
                                      std::int64_t tick, OwnerId site, PlanId plan, OwnerId orderer, CauseId cause) {
  Establishment* e = estabs.find(site);
  if (!e) return OrderResult::NoSite;
  if (active(site)) return OrderResult::SiteBusy;
  const PlanDef& d = plans_.at(plan);
  if (l.balance(orderer, money.resource()) < d.budget_cents) return OrderResult::NoBudget;
  if (d.budget_cents > 0 && money.pay(l, tick, orderer, site, d.budget_cents, FlowKind::Grant, cause) != d.budget_cents)
    throw std::logic_error("Projects::order: the escrow did not move in full");
  e->recipe = d.stages;
  e->labour_bank = 0;
  e->batch_cap = 0;
  Project p;
  p.site = site;
  p.plan = plan;
  p.orderer = orderer;
  p.ordered_tick = tick;
  p.produced_at_order = e->produced;
  p.produced_seen = e->produced;
  p.caught_up_seen = e->caught_up;
  all_.push_back(p);
  (void)goods;
  return OrderResult::Ok;
}

std::int64_t Projects::cancel(Ledger& l, Money& money, Establishments& estabs, std::int64_t tick, OwnerId site,
                              CauseId cause) {
  Project* p = active(site);
  if (!p) return -1;
  Establishment* e = estabs.find(site);
  const std::int64_t left = l.balance(site, money.resource());
  const std::int64_t returned = left > 0 ? money.pay(l, tick, site, p->orderer, left, FlowKind::Grant, cause) : 0;
  p->state = ProjectState::Cancelled;
  p->finished_tick = tick;
  if (e) {
    e->batch_cap = 0;  // the site stands until a new order (or a new recipe by setup) gives it work
    e->labour_bank = 0;
  }
  return returned;
}

void Projects::before_advance(const Ledger& l, const Goods& goods, const Money& money, Establishments& estabs) {
  for (Project& p : all_) {
    if (p.state != ProjectState::Active) continue;
    Establishment* e = estabs.find(p.site);
    if (!e) continue;
    const PlanDef& d = plans_.at(p.plan);
    const GoodDef& b = goods.def(d.building);
    const std::int64_t stages_left = std::max<std::int64_t>(0, b.bulk_per_unit - l.balance(p.site, b.bulk));
    std::int64_t cap = stages_left;
    if (d.cost_per_stage_cents > 0) cap = std::min(cap, l.balance(p.site, money.resource()) / d.cost_per_stage_cents);
    e->batch_cap = cap;
  }
}

std::vector<Projects::Change> Projects::after_advance(Ledger& l, const Goods& goods, const Recipes& recipes, Money& money,
                                                      Establishments& estabs, std::int64_t tick, CauseId cause) {
  std::vector<Change> out;
  for (Project& p : all_) {
    if (p.state != ProjectState::Active) continue;
    Establishment* e = estabs.find(p.site);
    if (!e) continue;
    const PlanDef& d = plans_.at(p.plan);
    const GoodDef& b = goods.def(d.building);
    // Pay the contractor for the stages built since the last time.
    const std::int64_t built = e->produced - p.produced_at_order;
    const std::int64_t owed = (built - p.stages_paid) * d.cost_per_stage_cents;
    if (owed > 0) {
      if (money.pay(l, tick, p.site, d.contractor, owed, FlowKind::Wage, cause) != owed)
        throw std::logic_error("Projects::after_advance: a stage was built that could not be paid");
    }
    p.stages_paid = built;
    // Finished?
    if (l.balance(p.site, b.bulk) >= b.bulk_per_unit) {
      if (goods.materialise(l, tick, p.site, d.building, 1, cause) != 1) throw std::logic_error("Projects::after_advance: the building did not materialise");
      p.state = ProjectState::Finished;
      p.finished_tick = tick;
      p.stall = StallReason::None;
      e->labour_bank = 0;
      e->batch_cap = d.then ? -1 : 0;
      if (d.then) e->recipe = *d.then;
      out.push_back({Change::Finished, p.site, p.plan, tick, StallReason::None});
      continue;
    }
    // Stalled or resumed? Judged on the state at the end of the interval (so a jump and stepping agree): the
    // project is stalled if it could not start another stage now. A labour bank short of a stage is not a stall.
    const bool time_passed = e->caught_up > p.caught_up_seen;
    p.produced_seen = e->produced;
    p.caught_up_seen = e->caught_up;
    if (!time_passed) continue;
    StallReason why = StallReason::None;
    const RecipeDef& r = recipes.def(e->recipe);
    if (e->staff <= 0) why = StallReason::NoStaff;
    else if (r.power_w > 0 && e->power_w < r.power_w) why = StallReason::NoPower;
    else if (d.cost_per_stage_cents > 0 && l.balance(p.site, money.resource()) < d.cost_per_stage_cents) why = StallReason::NoMoney;
    else if (recipes.affordable(l, goods, p.site, e->recipe) <= 0) why = StallReason::NoMaterials;
    if (why != p.stall) {
      if (why == StallReason::None) out.push_back({Change::Resumed, p.site, p.plan, tick, StallReason::None});
      else out.push_back({Change::Stalled, p.site, p.plan, tick, why});
      p.stall = why;
    }
  }
  return out;
}

Projects::View Projects::view(const Ledger& l, const Goods& goods, const Money& money, OwnerId site) const {
  const Project* p = find(site);
  if (!p) return {ProjectState::Cancelled, 0, 0, 0.0, StallReason::None, 0};
  const PlanDef& d = plans_.at(p->plan);
  const GoodDef& b = goods.def(d.building);
  View v;
  v.state = p->state;
  v.stages_total = b.bulk_per_unit;
  v.stages_done = p->state == ProjectState::Finished ? b.bulk_per_unit : l.balance(site, b.bulk);
  v.progress = static_cast<double>(v.stages_done) / static_cast<double>(v.stages_total);
  v.stall = p->stall;
  v.money_left = l.balance(site, money.resource());
  return v;
}

void Projects::save(ArchiveWriter& w) const {
  w.u64(all_.size());
  for (const Project& p : all_) {
    w.u32(p.site); w.u16(p.plan); w.u32(p.orderer); w.i64(p.ordered_tick); w.i64(p.finished_tick);
    w.i64(p.produced_at_order); w.i64(p.stages_paid); w.i64(p.produced_seen); w.i64(p.caught_up_seen);
    w.u8(static_cast<std::uint8_t>(p.state)); w.u8(static_cast<std::uint8_t>(p.stall));
  }
}

void Projects::load(ArchiveReader& r) {
  std::vector<Project> all(r.u64());
  for (Project& p : all) {
    p.site = r.u32(); p.plan = r.u16(); p.orderer = r.u32(); p.ordered_tick = r.i64(); p.finished_tick = r.i64();
    p.produced_at_order = r.i64(); p.stages_paid = r.i64(); p.produced_seen = r.i64(); p.caught_up_seen = r.i64();
    p.state = static_cast<ProjectState>(r.u8()); p.stall = static_cast<StallReason>(r.u8());
  }
  all_ = std::move(all);
}

}  // namespace zs
