#include "zs/pop_registry.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "zs/archive.h"
#include "zs/detmath.h"
#include "zs/hypergeometric.h"

namespace zs {

namespace {
// The save format is versioned. To change it: bump kSaveVersion, write the new reader beside the old one
// (never delete an old reader), keep a frozen fixture of every shipped version in tests/fixtures, and make the
// new reader of an old version fill any new field with the value that reproduces the old behaviour.
constexpr std::uint64_t kSaveMagic = 0x474552504f50ull;  // "POPREG"
constexpr std::uint32_t kSaveVersion = 3;  // 2 adds the region-event counter; 3 adds founder specs
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

bool PopRegistry::set_founder_specs(std::uint64_t cohort_id, std::vector<FounderSpec> specs) {
  const Cohort* c = cohort(cohort_id);
  if (!c || specs.size() != c->size || founder_specs_.count(cohort_id)) return false;
  founder_specs_[cohort_id] = std::move(specs);
  return true;
}

double PopRegistry::founder_age(PopId id) const {
  std::uint64_t member = 0;
  const Cohort* c = cohort_of(id, member);
  const double fraction = keyed_uniform01(seed_, Purpose::PopLife, id, kDrawStartAge);  // born somewhere in the year
  if (c) {
    if (const auto it = founder_specs_.find(c->id); it != founder_specs_.end())
      return static_cast<double>(it->second[member].age) + fraction;
  }
  return params_.founder_min_age + std::floor(fraction * params_.founder_age_span);
}

std::optional<Sex> PopRegistry::founder_sex(PopId id) const {
  std::uint64_t member = 0;
  const Cohort* c = cohort_of(id, member);
  if (!c) return std::nullopt;
  const auto it = founder_specs_.find(c->id);
  if (it == founder_specs_.end()) return std::nullopt;
  return it->second[member].sex;
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
    out = derive_base(seed_, id, -age, age, params_, founder_sex(id));
  } else {
    PopBase mother;
    if (!resolve_base(mother_of(id), mother)) return false;
    // The mother's own exceptions (an earlier death) change her births.
    const DeathOverrides o = death_overrides();
    if (const auto it = o.find(mother_of(id)); it != o.end()) mother.death_year = it->second;
    bool found = false;
    for (const Birth& b : births_of(seed_, mother_of(id), mother, is_born(mother_of(id)) ? -1e9 : 0.0, params_))
      if (b.index == child_index_of(id)) { out = derive_base(seed_, id, b.year, 0.0, params_); found = true; break; }
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
  if (const auto in = rosters_of_.find(id); in != rosters_of_.end())
    for (std::uint64_t rid : in->second)
      for (int k = 0; k < static_cast<int>(EffectKind::kCount); ++k)
        if (rosters_.at(rid).in_state(id, static_cast<EffectKind>(k), year)) out.roster_bits |= 1u << k;
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

bool PopRegistry::add_roster(std::uint64_t id, std::uint32_t region, RosterKind kind) {
  if (rosters_.count(id)) return false;
  rosters_[id] = Roster(id, region, kind);
  return true;
}

const Roster* PopRegistry::roster(std::uint64_t id) const {
  const auto it = rosters_.find(id);
  return it == rosters_.end() ? nullptr : &it->second;
}

bool PopRegistry::roster_join(std::uint64_t id, PopId pop, double year) {
  const auto it = rosters_.find(id);
  if (it == rosters_.end() || !exists(pop) || !it->second.join(pop, year)) return false;
  auto& list = rosters_of_[pop];
  if (std::find(list.begin(), list.end(), id) == list.end()) list.push_back(id);
  return true;
}

bool PopRegistry::roster_leave(std::uint64_t id, PopId pop, double year) {
  const auto it = rosters_.find(id);
  return it != rosters_.end() && it->second.leave(pop, year);
}

bool PopRegistry::roster_apply(std::uint64_t id, EffectKind kind, std::int64_t delta, double year, std::uint64_t reason) {
  const auto it = rosters_.find(id);
  return it != rosters_.end() && it->second.apply(seed_, kind, delta, year, reason);
}

std::uint64_t PopRegistry::roster_count(std::uint64_t id, EffectKind kind, double year) const {
  const auto it = rosters_.find(id);
  return it == rosters_.end() ? 0 : it->second.count(kind, year);
}

std::uint64_t PopRegistry::roster_count_region(std::uint32_t region, EffectKind kind, double year) const {
  std::uint64_t n = 0;
  for (const auto& [id, r] : rosters_)
    if (r.region() == region) n += r.count(kind, year);
  return n;
}

bool PopRegistry::region_apply(std::uint32_t region, EffectKind kind, std::int64_t delta, double year, std::uint64_t reason,
                               std::optional<RosterKind> targets) {
  if (delta == 0 || static_cast<std::uint8_t>(kind) >= static_cast<std::uint8_t>(EffectKind::kCount)) return false;
  struct Part { std::uint64_t id, capacity; };
  std::vector<Part> parts;
  std::uint64_t total = 0;
  for (const auto& [id, r] : rosters_) {
    if (r.region() != region || (targets && r.kind() != *targets)) continue;
    if (year < r.last_year()) return false;  // a roster has already seen a later year
    const std::uint64_t in_state = r.count(kind, year);
    const std::uint64_t cap = delta > 0 ? r.size(year) - in_state : in_state;
    parts.push_back({id, cap});
    total += cap;
  }
  const std::uint64_t want = delta > 0 ? static_cast<std::uint64_t>(delta) : static_cast<std::uint64_t>(-delta);
  if (parts.empty() || want > total) return false;
  const std::uint64_t stream = ((static_cast<std::uint64_t>(region) << 16) | (targets ? static_cast<std::uint64_t>(*targets) + 1 : 0) << 8 | static_cast<std::uint64_t>(kind)) ^
                               (region_events_ * 0x9E3779B97F4A7C15ull);
  std::uint64_t pool = total, left = want;
  for (const Part& p : parts) {
    if (left == 0) break;
    const std::uint64_t k = hypergeometric(keyed_uniform01(seed_, Purpose::PopRank, stream, p.id), pool, p.capacity, left);
    if (k > 0 && !rosters_.at(p.id).apply(seed_, kind, delta > 0 ? static_cast<std::int64_t>(k) : -static_cast<std::int64_t>(k), year, reason))
      throw std::logic_error("pop registry: a region split asked a roster for more than it had");
    pool -= p.capacity;
    left -= k;
  }
  ++region_events_;
  return true;
}

DemographyCounts PopRegistry::project_demography(double from, double to) const {
  std::vector<Founder> founders;
  for (const auto& [cid, c] : cohorts_)
    for (std::uint64_t m = 0; m < c.size; ++m) {
      const PopId id = census_pop_id(static_cast<std::uint32_t>(c.first_slot + m));
      const double age = founder_age(id);
      founders.push_back({id, -age, age, founder_sex(id)});
    }
  const DeathOverrides o = death_overrides();
  return jump(seed_, founders, from, to, &o, params_);
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

std::optional<std::string> PopRegistry::audit() const {
  const PopCounts n = counts();
  if (n.records + n.lazy != n.total) return "pop counts do not add up";
  for (const auto& [id, r] : records_)
    if (!exists(id)) return "a record is not a pop";
  for (const auto& [id, list] : kept_)
    if (records_.count(id)) return "a pop is both a record and a kept lazy pop";
  return std::nullopt;
}

std::vector<std::uint8_t> PopRegistry::save() const {
  ArchiveWriter w;
  w.u64(kSaveMagic);
  w.u32(kSaveVersion);
  w.u64(seed_);
  write_pop_params(w, params_);
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
  w.u64(rosters_.size());
  for (const auto& [id, r] : rosters_) r.save(w);
  w.u64(region_events_);  // v2
  w.u64(founder_specs_.size());  // v3
  for (const auto& [cid, specs] : founder_specs_) {
    w.u64(cid); w.u64(specs.size());
    for (const FounderSpec& f : specs) { w.u16(f.age); w.u8(static_cast<std::uint8_t>(f.sex)); }
  }
  return w.bytes();
}

void PopRegistry::load(const std::vector<std::uint8_t>& bytes) {
  ArchiveReader r(bytes);
  if (r.u64() != kSaveMagic) throw std::runtime_error("not a pop registry save");
  const std::uint32_t version = r.u32();
  switch (version) {
    case 1: load_v1(r); return;
    case 2: load_v2(r); return;
    case 3: load_v3(r); return;
    default:
      throw std::runtime_error("pop registry save is version " + std::to_string(version) + "; this build reads up to " +
                               std::to_string(kSaveVersion));
  }
}

void PopRegistry::load_v1(ArchiveReader& r) {
  load_body(r, 1);
  region_events_ = 0;  // version 1 had none: no region event had ever been applied
  founder_specs_.clear();
  if (!r.at_end()) throw std::runtime_error("pop registry save has trailing bytes");
}

void PopRegistry::load_v2(ArchiveReader& r) {
  load_body(r, 2);
  load_events(r);
  founder_specs_.clear();  // version 2 had none: founders were drawn from their ids
  if (!r.at_end()) throw std::runtime_error("pop registry save has trailing bytes");
}

void PopRegistry::load_v3(ArchiveReader& r) {
  load_body(r, 3);
  load_events(r);
  load_specs(r);
  if (!r.at_end()) throw std::runtime_error("pop registry save has trailing bytes");
}

void PopRegistry::load_events(ArchiveReader& r) { region_events_ = r.u64(); }

void PopRegistry::load_specs(ArchiveReader& r) {
  founder_specs_.clear();
  for (std::uint64_t i = 0, n = r.u64(); i < n; ++i) {
    const std::uint64_t cid = r.u64();
    std::vector<FounderSpec> specs(r.u64());
    for (FounderSpec& f : specs) { f.age = r.u16(); f.sex = static_cast<Sex>(r.u8()); }
    founder_specs_[cid] = std::move(specs);
  }
}

void PopRegistry::load_body(ArchiveReader& r, std::uint32_t version) {
  if (r.u64() != seed_) throw std::runtime_error("pop registry save is from another world seed");
  params_ = read_pop_params(r);
  cohorts_.clear(); by_slot_.clear(); log_.clear(); records_.clear(); kept_.clear();
  by_cohort_kind_.clear(); by_region_.clear(); rosters_.clear(); rosters_of_.clear();
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
  for (std::uint64_t i = 0, n = r.u64(); i < n; ++i) {
    Roster ro;
    ro.load(r, version);
    const std::uint64_t rid = ro.id();
    for (PopId pop : ro.ever_members()) rosters_of_[pop].push_back(rid);
    rosters_[rid] = std::move(ro);
  }
}

std::uint64_t PopRegistry::hash() const { return hash_bytes(save()); }

}  // namespace zs
