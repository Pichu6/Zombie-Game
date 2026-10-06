#include "zs/pop_registry.h"

#include <algorithm>

#include "zs/archive.h"
#include "zs/detmath.h"

namespace zs {

namespace {
constexpr std::uint64_t kSaveMagic = 0x504f5052454731ull;  // "POPREG1"
constexpr std::uint64_t kDrawStartAge = 10;                // the `b` key of Purpose::PopLife
}  // namespace

bool PopRegistry::add_cohort(const Cohort& c) {
  if (c.size == 0 || cohorts_.count(c.id)) return false;
  const std::uint64_t last = static_cast<std::uint64_t>(c.first_slot) + c.size;  // one past
  auto next = by_slot_.lower_bound(c.first_slot);
  if (next != by_slot_.end() && next->first < last) return false;
  if (next != by_slot_.begin()) {
    const Cohort& prev = cohorts_.at(std::prev(next)->second);
    if (static_cast<std::uint64_t>(prev.first_slot) + prev.size > c.first_slot) return false;
  }
  cohorts_[c.id] = c;
  by_slot_[c.first_slot] = c.id;
  by_region_[c.region].push_back(c.id);
  return true;
}

void PopRegistry::index_effect(const CohortEffect& e) {
  by_cohort_kind_[{e.cohort, static_cast<std::uint8_t>(e.kind)}].push_back(e);
}

const Cohort* PopRegistry::cohort(std::uint64_t id) const {
  const auto it = cohorts_.find(id);
  return it == cohorts_.end() ? nullptr : &it->second;
}

CohortPermutation PopRegistry::permutation(const Cohort& c, EffectKind kind) const {
  return CohortPermutation(seed_, c.id * 64 + static_cast<std::uint64_t>(kind), c.size);
}

std::vector<PopRegistry::Range> PopRegistry::ranges(std::uint64_t cohort_id, EffectKind kind, double year) const {
  std::vector<Range> stack;  // the members in the state, as rank ranges, newest last
  std::uint64_t total = 0;
  const auto found = by_cohort_kind_.find({cohort_id, static_cast<std::uint8_t>(kind)});
  if (found == by_cohort_kind_.end()) return stack;
  for (const CohortEffect& e : found->second) {
    if (e.year > year) break;  // in time order
    if (e.delta >= 0) {
      stack.push_back({total, total + static_cast<std::uint64_t>(e.delta), e.reason});
      total += static_cast<std::uint64_t>(e.delta);
    } else {
      std::uint64_t give = static_cast<std::uint64_t>(-e.delta);
      total -= give;
      while (give > 0 && !stack.empty()) {
        Range& r = stack.back();
        const std::uint64_t len = r.end - r.begin;
        if (len <= give) { give -= len; stack.pop_back(); } else { r.end -= give; give = 0; }
      }
    }
  }
  return stack;
}

bool PopRegistry::apply(const CohortEffect& e) {
  const Cohort* c = cohort(e.cohort);
  if (!c || static_cast<std::uint8_t>(e.kind) >= static_cast<std::uint8_t>(EffectKind::kCount)) return false;
  // Effects are appended in time order; the check uses the state after everything logged so far.
  if (!log_.empty() && e.year < log_.back().year) return false;
  const std::uint64_t now = count(e.cohort, e.kind, e.year);
  if (e.delta >= 0 && now + static_cast<std::uint64_t>(e.delta) > c->size) return false;
  if (e.delta < 0 && static_cast<std::uint64_t>(-e.delta) > now) return false;
  log_.push_back(e);
  index_effect(e);
  return true;
}

std::uint64_t PopRegistry::count(std::uint64_t cohort_id, EffectKind kind, double year) const {
  std::uint64_t n = 0;
  for (const Range& r : ranges(cohort_id, kind, year)) n += r.end - r.begin;
  return n;
}

std::uint64_t PopRegistry::count_region(std::uint32_t region, EffectKind kind, double year) const {
  std::uint64_t n = 0;
  const auto it = by_region_.find(region);
  if (it == by_region_.end()) return 0;
  for (std::uint64_t id : it->second) n += count(id, kind, year);
  return n;
}

bool PopRegistry::member_in(std::uint64_t cohort_id, std::uint64_t member, EffectKind kind, double year) const {
  const Cohort* c = cohort(cohort_id);
  if (!c || member >= c->size) return false;
  const std::uint64_t rank = permutation(*c, kind).forward(member);
  for (const Range& r : ranges(cohort_id, kind, year))
    if (rank >= r.begin && rank < r.end) return true;
  return false;
}

std::uint64_t PopRegistry::reason_of(std::uint64_t cohort_id, std::uint64_t member, EffectKind kind, double year) const {
  const Cohort* c = cohort(cohort_id);
  if (!c || member >= c->size) return 0;
  const std::uint64_t rank = permutation(*c, kind).forward(member);
  for (const Range& r : ranges(cohort_id, kind, year))
    if (rank >= r.begin && rank < r.end) return r.reason;
  return 0;
}

const Cohort* PopRegistry::cohort_of(PopId id, std::uint64_t& member) const {
  if (id >> 32) return nullptr;  // census ids only, for now
  auto it = by_slot_.upper_bound(static_cast<std::uint32_t>(id));
  if (it == by_slot_.begin()) return nullptr;
  const Cohort& c = cohorts_.at(std::prev(it)->second);
  member = id - c.first_slot;
  return member < c.size ? &c : nullptr;
}

double PopRegistry::founder_age(PopId id) const {
  return 18.0 + static_cast<double>(static_cast<std::uint64_t>(keyed_uniform01(seed_, Purpose::PopLife, id, kDrawStartAge) * 47.0));
}

DeathOverrides PopRegistry::death_overrides() const {
  DeathOverrides o;
  const auto scan = [&](PopId id, const std::vector<PopException>& list) {
    for (const PopException& e : list)
      if (e.field == ExceptionField::DeathYear) o[id] = e.value;
  };
  for (const auto& [id, r] : records_) scan(id, r.exceptions);
  for (const auto& [id, list] : kept_) scan(id, list);
  return o;
}

bool PopRegistry::resolve_base(PopId id, PopBase& out) const {
  if (!is_born(id)) {
    std::uint64_t member = 0;
    if (!cohort_of(id, member)) return false;
    const double age = founder_age(id);
    out = derive_base(seed_, id, -age, age);
  } else {
    PopBase mother;
    if (!resolve_base(mother_of(id), mother)) return false;
    // The mother's own exceptions (an earlier death) change her births.
    const DeathOverrides o = death_overrides();
    if (const auto it = o.find(mother_of(id)); it != o.end()) mother.death_year = it->second;
    bool found = false;
    for (const Birth& b : births_of(seed_, mother_of(id), mother, is_born(mother_of(id)) ? -1e9 : 0.0))
      if (b.index == child_index_of(id)) { out = derive_base(seed_, id, b.year, 0.0); found = true; break; }
    if (!found) return false;
  }
  return true;
}

bool PopRegistry::exists(PopId id) const {
  PopBase b;
  return resolve_base(id, b);
}

bool PopRegistry::snapshot(PopId id, double year, PopSnapshot& out) const {
  PopBase base;
  if (!resolve_base(id, base)) return false;
  out = PopSnapshot{};
  out.base = base;
  std::uint64_t member = 0;
  if (const Cohort* c = is_born(id) ? nullptr : cohort_of(id, member)) {
    for (int k = 0; k < static_cast<int>(EffectKind::kCount); ++k) {
      const auto kind = static_cast<EffectKind>(k);
      const std::uint64_t rank = permutation(*c, kind).forward(member);
      for (const Range& r : ranges(c->id, kind, year))
        if (rank >= r.begin && rank < r.end) { out.state_bits |= 1u << k; break; }
    }
  }
  const std::vector<PopException>* ex = nullptr;
  if (const auto it = records_.find(id); it != records_.end()) {
    out.level = Level::Record;
    out.office = it->second.office;
    out.followed = it->second.followed;
    ex = &it->second.exceptions;
  } else if (const auto it2 = kept_.find(id); it2 != kept_.end()) {
    ex = &it2->second;
  }
  if (ex)
    for (const PopException& e : *ex)
      if (e.field == ExceptionField::DeathYear) out.base.death_year = e.value;
  return true;
}

DemographyCounts PopRegistry::project_demography(double from, double to, const Fertility& f) const {
  std::vector<Founder> founders;
  for (const auto& [cid, c] : cohorts_)
    for (std::uint64_t m = 0; m < c.size; ++m) {
      const PopId id = census_pop_id(static_cast<std::uint32_t>(c.first_slot + m));
      const double age = founder_age(id);
      founders.push_back({id, -age, age});
    }
  const DeathOverrides o = death_overrides();
  return jump(seed_, founders, from, to, &o, f);
}

bool PopRegistry::promote(PopId id) {
  if (records_.count(id) || !exists(id)) return false;
  Record r;
  if (const auto it = kept_.find(id); it != kept_.end()) {
    r.exceptions = std::move(it->second);
    kept_.erase(it);
  }
  records_[id] = std::move(r);
  return true;
}

bool PopRegistry::demote(PopId id) {
  const auto it = records_.find(id);
  if (it == records_.end() || it->second.office != 0 || it->second.followed) return false;
  if (!it->second.exceptions.empty()) kept_[id] = std::move(it->second.exceptions);  // nothing the pop did is lost
  records_.erase(it);
  return true;
}

bool PopRegistry::set_office(PopId id, std::uint32_t office) {
  promote(id);
  const auto it = records_.find(id);
  if (it == records_.end()) return false;
  it->second.office = office;
  return true;
}

bool PopRegistry::set_followed(PopId id, bool followed) {
  promote(id);
  const auto it = records_.find(id);
  if (it == records_.end()) return false;
  it->second.followed = followed;
  return true;
}

void PopRegistry::set_exception(std::vector<PopException>& list, const PopException& e) {
  for (PopException& x : list)
    if (x.field == e.field) { x.value = e.value; return; }
  list.push_back(e);
}

bool PopRegistry::set_death_year(PopId id, double year) {
  if (!exists(id)) return false;
  if (const auto it = records_.find(id); it != records_.end()) set_exception(it->second.exceptions, {ExceptionField::DeathYear, year});
  else set_exception(kept_[id], {ExceptionField::DeathYear, year});
  return true;
}

PopCounts PopRegistry::counts() const {
  PopCounts n;
  for (const auto& [id, c] : cohorts_) n.total += c.size;
  n.records = records_.size();
  n.lazy = n.total - n.records;
  return n;
}

std::vector<std::uint8_t> PopRegistry::save() const {
  ArchiveWriter w;
  w.u64(kSaveMagic);
  w.u64(seed_);
  w.u64(cohorts_.size());
  for (const auto& [id, c] : cohorts_) { w.u64(c.id); w.u32(c.region); w.u32(c.first_slot); w.u64(c.size); }
  w.u64(log_.size());
  for (const CohortEffect& e : log_) { w.u64(e.cohort); w.u8(static_cast<std::uint8_t>(e.kind)); w.i64(e.delta); w.f64(e.year); w.u64(e.reason); }
  const auto put_exceptions = [&](const std::vector<PopException>& list) {
    w.u32(static_cast<std::uint32_t>(list.size()));
    for (const PopException& x : list) { w.u8(static_cast<std::uint8_t>(x.field)); w.f64(x.value); }
  };
  w.u64(records_.size());
  for (const auto& [id, r] : records_) { w.u64(id); w.u32(r.office); w.boolean(r.followed); put_exceptions(r.exceptions); }
  w.u64(kept_.size());
  for (const auto& [id, list] : kept_) { w.u64(id); put_exceptions(list); }
  return w.bytes();
}

void PopRegistry::load(const std::vector<std::uint8_t>& bytes) {
  ArchiveReader r(bytes);
  if (r.u64() != kSaveMagic) throw std::runtime_error("not a pop registry save");
  if (r.u64() != seed_) throw std::runtime_error("pop registry save is from another world seed");
  cohorts_.clear(); by_slot_.clear(); log_.clear(); records_.clear(); kept_.clear();
  by_cohort_kind_.clear(); by_region_.clear();
  for (std::uint64_t i = 0, n = r.u64(); i < n; ++i) {
    Cohort c;
    c.id = r.u64(); c.region = r.u32(); c.first_slot = r.u32(); c.size = r.u64();
    cohorts_[c.id] = c;
    by_slot_[c.first_slot] = c.id;
    by_region_[c.region].push_back(c.id);
  }
  for (std::uint64_t i = 0, n = r.u64(); i < n; ++i) {
    CohortEffect e;
    e.cohort = r.u64(); e.kind = static_cast<EffectKind>(r.u8()); e.delta = r.i64(); e.year = r.f64(); e.reason = r.u64();
    log_.push_back(e);
    index_effect(e);
  }
  const auto get_exceptions = [&] {
    std::vector<PopException> list(r.u32());
    for (PopException& x : list) { x.field = static_cast<ExceptionField>(r.u8()); x.value = r.f64(); }
    return list;
  };
  for (std::uint64_t i = 0, n = r.u64(); i < n; ++i) {
    const PopId id = r.u64();
    Record rec;
    rec.office = r.u32(); rec.followed = r.boolean(); rec.exceptions = get_exceptions();
    records_[id] = std::move(rec);
  }
  for (std::uint64_t i = 0, n = r.u64(); i < n; ++i) {
    const PopId id = r.u64();
    kept_[id] = get_exceptions();
  }
  if (!r.at_end()) throw std::runtime_error("pop registry save has trailing bytes");
}

std::uint64_t PopRegistry::hash() const { return hash_bytes(save()); }

}  // namespace zs
