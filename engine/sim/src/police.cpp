// The police's missing-person case: a queue with staff. Spec: docs/tier0/specs/64-police-cases.md
#include "zs/police.h"

#include <algorithm>
#include <array>
#include <stdexcept>

#include "toml.hpp"

namespace zs {
namespace {

constexpr std::uint64_t kDrawMark = std::uint64_t{0x504F} << 48;  // "PO": these draws never meet another system's

std::array<double, 3> shares_of(const toml::table& t, const char* key, const std::string& path, const std::string& id) {
  const toml::array* a = t[key].as_array();
  if (!a || a->size() != 3) throw std::runtime_error(path + ": [" + id + "] needs " + key + " as three shares");
  std::array<double, 3> out{};
  double sum = 0;
  for (std::size_t i = 0; i < 3; ++i) {
    const auto v = a->get(i)->value<double>();
    if (!v || *v < 0 || *v > 1) throw std::runtime_error(path + ": [" + id + "] " + key + " shares must be between 0 and 1");
    out[i] = *v;
    sum += *v;
  }
  if (sum < 0.99 || sum > 1.01) throw std::runtime_error(path + ": [" + id + "] " + key + " must add up to 1");
  return out;
}

}  // namespace

const char* risk_name(RiskTier t) {
  switch (t) {
    case RiskTier::High: return "high";
    case RiskTier::Medium: return "medium";
    case RiskTier::Low: return "low";
  }
  return "?";
}

PoliceParams load_police_params(const std::string& path, const std::string& country_profile, const std::string& income) {
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
  PoliceParams p;
  const auto adults = shares_of(*c, "adult_tiers", path, id), children = shares_of(*c, "child_tiers", path, id),
             found = shares_of(*c, "found_by", path, id);
  std::copy(adults.begin(), adults.end(), p.adult_tiers);
  std::copy(children.begin(), children.end(), p.child_tiers);
  std::copy(found.begin(), found.end(), p.found_by);
  const auto age = (*c)["child_age_max"].value<std::int64_t>();
  const auto at_once = (*c)["searches_at_once"].value<std::int64_t>();
  const auto wandered = (*c)["wandered_is_high"].value<bool>();
  if (!age || *age < 0 || !at_once || *at_once < 1 || !wandered) throw std::runtime_error(path + ": [" + id + "] needs child_age_max, searches_at_once (at least 1) and wandered_is_high");
  p.child_age_max = static_cast<int>(*age);
  p.searches_at_once = static_cast<int>(*at_once);
  p.wandered_is_high = *wandered;
  return p;
}

const PoliceCases::Case* PoliceCases::find(PopId pop) const {
  for (const Case& c : cases_)
    if (c.pop == pop) return &c;
  return nullptr;
}

int PoliceCases::active_searches() const {
  return static_cast<int>(std::count_if(cases_.begin(), cases_.end(), [](const Case& c) { return c.searching; }));
}

int PoliceCases::queued() const { return static_cast<int>(cases_.size()) - active_searches(); }

RiskTier PoliceCases::draw_tier(const World& w, const Expectations::Case& c, std::int64_t serial) const {
  if (params_.wandered_is_high && c.cause == AbsenceCause::Wandered) return RiskTier::High;  // lost or confused: out of character, at risk
  const double* shares = is_child_ && is_child_(c.pop) ? params_.child_tiers : params_.adult_tiers;
  const double u = keyed_uniform01(w.rng.seed(), Purpose::PopLife, c.pop, kDrawMark | (static_cast<std::uint64_t>(serial) << 2) | 0);
  return u < shares[0] ? RiskTier::High : u < shares[0] + shares[1] ? RiskTier::Medium : RiskTier::Low;
}

FoundBy PoliceCases::draw_found_by(const World& w, PopId pop, std::int64_t serial) const {
  const double u = keyed_uniform01(w.rng.seed(), Purpose::PopLife, pop, kDrawMark | (static_cast<std::uint64_t>(serial) << 2) | 1);
  return u < params_.found_by[0] ? FoundBy::OwnWill : u < params_.found_by[0] + params_.found_by[1] ? FoundBy::Police : FoundBy::Other;
}

// The place of a waiting case in the queue: high risk first, then the oldest; 1 is next.
int PoliceCases::rank_of(const Case& c) const {
  int rank = 1;
  for (const Case& o : cases_) {
    if (o.searching || &o == &c) continue;
    if (o.tier < c.tier || (o.tier == c.tier && (o.opened < c.opened || (o.opened == c.opened && o.serial < c.serial)))) ++rank;
  }
  return rank;
}

// Staff are free: the best waiting case gets a search (the ladder's search opens), until the force has none left.
void PoliceCases::start_searches(World& w) {
  while (active_searches() < params_.searches_at_once) {
    Case* best = nullptr;
    for (Case& c : cases_)
      if (!c.searching && (!best || rank_of(c) < rank_of(*best))) best = &c;
    if (!best) return;
    best->searching = true;
    exp_->start_search(w, best->pop);
  }
}

void PoliceCases::on_reported(World& w, const Expectations::Case& c, EventId reported) {
  if (find(c.pop)) return;
  Case k;
  k.pop = c.pop;
  k.opened = w.clock.tick;
  k.serial = opened_total_++;
  k.tier = draw_tier(w, c, k.serial);
  cases_.push_back(k);
  const int free_slot = active_searches() < params_.searches_at_once ? 1 : 0;
  const int rank = rank_of(cases_.back());
  w.emit(EventType::PoliceCaseOpened, c.pop, reported, static_cast<std::int64_t>(k.tier), free_slot && rank == 1 ? 0 : rank);
  start_searches(w);
}

void PoliceCases::on_absence_ended(World& w, const Expectations::Case& c, EventId ended) {
  const auto it = std::find_if(cases_.begin(), cases_.end(), [&](const Case& k) { return k.pop == c.pop; });
  if (it == cases_.end()) return;
  w.emit(EventType::PoliceCaseResolved, c.pop, ended, static_cast<std::int64_t>(draw_found_by(w, c.pop, it->serial)), w.clock.tick - it->opened);
  cases_.erase(it);
  ++resolved_total_;
  start_searches(w);  // a free search goes to the next case waiting
}

void PoliceCases::save(ArchiveWriter& w) const {
  w.i64(opened_total_);
  w.i64(resolved_total_);
  w.u64(cases_.size());
  for (const Case& c : cases_) {
    w.u64(c.pop);
    w.i64(c.opened);
    w.u8(static_cast<std::uint8_t>(c.tier));
    w.boolean(c.searching);
    w.i64(c.serial);
  }
}

void PoliceCases::load(ArchiveReader& r) {
  cases_.clear();
  opened_total_ = r.i64();
  resolved_total_ = r.i64();
  const std::uint64_t n = r.u64();
  for (std::uint64_t i = 0; i < n; ++i) {
    Case c;
    c.pop = r.u64();
    c.opened = r.i64();
    c.tier = static_cast<RiskTier>(r.u8());
    c.searching = r.boolean();
    c.serial = r.i64();
    cases_.push_back(c);
  }
}

}  // namespace zs
