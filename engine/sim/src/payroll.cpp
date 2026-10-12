// Payroll: wages for shifts worked, nothing for shifts missed, and the job ending with a separation.
// Spec: docs/tier0/specs/62-payroll.md
#include "zs/payroll.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "toml.hpp"
#include "zs/economy_system.h"
#include "zs/pop_system.h"

namespace zs {

WageParams load_wage_params(const std::string& path, const std::string& country_profile, const std::string& income) {
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  std::string id = country_profile;
  if (id.empty() || !t[id].is_table()) {
    const auto named = t["index"][income].value<std::string>();
    if (!named) throw std::runtime_error(path + ": [index] has no table for " + income);
    id = *named;
  }
  const toml::table* c = t[id].as_table();
  if (!c) throw std::runtime_error(path + ": no table [" + id + "]");
  const auto cents = (*c)["cents_per_hour"].value<double>();
  if (!cents || *cents <= 0) throw std::runtime_error(path + ": [" + id + "] needs a positive cents_per_hour");
  WageParams p;
  p.cents_per_hour = *cents;
  return p;
}

std::int64_t Payroll::shifts_paid_total() const {
  std::int64_t n = 0;
  for (const Entry& e : entries_) n += e.shifts_paid;
  return n;
}

const Payroll::Entry* Payroll::find(PopId pop) const {
  for (const Entry& e : entries_)
    if (e.pop == pop) return &e;
  return nullptr;
}

std::size_t Payroll::headcount() const {
  return static_cast<std::size_t>(std::count_if(entries_.begin(), entries_.end(), [](const Entry& e) { return e.active; }));
}

std::int64_t Payroll::wages_ordered() const {
  std::int64_t sum = 0;
  for (const Entry& e : entries_) sum += e.ordered_cents;
  return sum;
}

// Looks up the pop's next shift from `from`; none leaves the entry waiting for nothing.
void Payroll::schedule(Entry& e, std::int64_t from) const {
  if (const std::optional<Commitment> shift = next_shift_of(next_, e.pop, from)) {
    e.next_due = shift->due_tick;
    e.minutes = shift->minutes;
  } else {
    e.next_due = System::kNever;
  }
}

void Payroll::track(PopId pop) {
  Entry e;
  e.pop = pop;
  schedule(e, 0);
  entries_.push_back(e);
}

void Payroll::update(World& w) {
  const std::int64_t now = w.clock.tick;
  // A separation ends the job: the pop leaves the employer's roster (one worker fewer wherever staff come from it).
  if (exp_) {
    const std::vector<PopId>& lost = exp_->lost_jobs();
    for (; seen_lost_ < lost.size(); ++seen_lost_) {
      for (Entry& e : entries_) {
        if (e.pop != lost[seen_lost_] || !e.active) continue;
        e.active = false;
        e.next_due = System::kNever;
        w.queue_intent({0, RosterLeave, e.pop, static_cast<std::int64_t>(roster_), 0});
        w.emit(EventType::LeftEmployer, e.pop, 0, static_cast<std::int64_t>(employer_), e.ordered_cents);
      }
    }
  }
  // A shift due: paid by the employer if the pop is at work, withheld if it is absent.
  for (Entry& e : entries_) {
    while (e.active && e.next_due <= now) {
      const std::int64_t cents = static_cast<std::int64_t>(std::llround(params_.cents_per_hour * e.minutes / 60.0));
      if (exp_ && exp_->find(e.pop)) {
        ++e.shifts_unpaid;
        w.emit(EventType::ShiftUnpaid, e.pop, 0, cents, e.minutes);
      } else if (cents > 0) {
        w.queue_intent({0, EconPay, static_cast<EntityId>(employer_), static_cast<std::int64_t>(owner_of_(e.pop)), econ_pay_arg(FlowKind::Wage, cents)});
        e.ordered_cents += cents;
        ++e.shifts_paid;
      }
      schedule(e, e.next_due + 1);
    }
  }
}

std::int64_t Payroll::next_wake(const World&, std::int64_t now) const {
  std::int64_t next = kNever;
  if (exp_ && seen_lost_ < exp_->lost_jobs().size()) next = now;
  for (const Entry& e : entries_)
    if (e.active) next = std::min(next, std::max(now, e.next_due));
  return next;
}

void Payroll::save(ArchiveWriter& w) const {
  w.u64(seen_lost_);
  w.u64(entries_.size());
  for (const Entry& e : entries_) {
    w.u64(e.pop);
    w.boolean(e.active);
    w.i64(e.next_due);
    w.i64(e.minutes);
    w.i64(e.ordered_cents);
    w.i64(e.shifts_paid);
    w.i64(e.shifts_unpaid);
  }
}

void Payroll::load(ArchiveReader& r) {
  entries_.clear();
  seen_lost_ = static_cast<std::size_t>(r.u64());
  const std::uint64_t n = r.u64();
  for (std::uint64_t i = 0; i < n; ++i) {
    Entry e;
    e.pop = r.u64();
    e.active = r.boolean();
    e.next_due = r.i64();
    e.minutes = static_cast<int>(r.i64());
    e.ordered_cents = r.i64();
    e.shifts_paid = r.i64();
    e.shifts_unpaid = r.i64();
    entries_.push_back(e);
  }
}

}  // namespace zs
