// Real people go missing without any player. Spec: docs/tier0/specs/60-missing-persons.md (decision 0070 rule 6)
#include "zs/missing_persons.h"

#include <algorithm>
#include <stdexcept>

#include "toml.hpp"
#include "zs/detmath.h"

namespace zs {
namespace {

constexpr std::int64_t kTicksPerYear = kTicksPerGameDay * 365;
constexpr std::uint64_t kDrawMark = std::uint64_t{0x4D50} << 48;  // "MP": these draws never meet the pop life's own

// Draw number `draws`, purpose `which` (0 gap, 1 duration, 2 cause) of one pop: keyed, so no call order matters.
double draw(const World& w, PopId pop, std::int64_t draws, int which) {
  return keyed_uniform01(w.rng.seed(), Purpose::PopLife, pop, kDrawMark | (static_cast<std::uint64_t>(draws) << 2) | static_cast<std::uint64_t>(which));
}

std::vector<std::pair<double, double>> cdf_of(const toml::table& t, const char* key, const std::string& path) {
  const toml::array* a = t[key].as_array();
  if (!a || a->empty()) throw std::runtime_error(path + ": missing " + key);
  std::vector<std::pair<double, double>> out;
  for (const auto& node : *a) {
    const toml::array* k = node.as_array();
    if (!k || k->size() != 2 || !k->get(0)->is_number() || !k->get(1)->is_number())
      throw std::runtime_error(path + ": " + key + " wants [hours, share] pairs");
    out.emplace_back(k->get(0)->value<double>().value(), k->get(1)->value<double>().value());
    if (out.back().first <= 0 || out.back().second <= 0 || out.back().second > 1 ||
        (out.size() > 1 && (out.back().first <= out[out.size() - 2].first || out.back().second < out[out.size() - 2].second)))
      throw std::runtime_error(path + ": " + key + " must increase in hours and share (share at most 1)");
  }
  return out;
}

double number(const toml::table& t, const char* key, const std::string& path) {
  const auto v = t[key].value<double>();
  if (!v || *v < 0) throw std::runtime_error(path + ": needs a non-negative " + key);
  return *v;
}

// Hours until return for a uniform draw: linear between the knots from (0, 0); beyond the last share, never.
bool hours_to_return(const std::vector<std::pair<double, double>>& cdf, double u, double& hours) {
  double h0 = 0, p0 = 0;
  for (const auto& [h, p] : cdf) {
    if (u < p) {
      hours = h0 + (h - h0) * (u - p0) / (p - p0);
      return true;
    }
    h0 = h;
    p0 = p;
  }
  return false;
}

}  // namespace

MissingParams load_missing_params(const std::string& path, const std::string& country_profile, const std::string& income) {
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
  MissingParams p;
  p.adult_per_1000_year = number(*c, "adult_per_1000_year", path);
  p.child_per_1000_year = number(*c, "child_per_1000_year", path);
  p.child_age_max = static_cast<int>(number(*c, "child_age_max", path));
  p.adult_return = cdf_of(*c, "adult_return_cdf", path);
  p.child_return = cdf_of(*c, "child_return_cdf", path);
  p.adult_chose = number(*c, "adult_chose", path);
  p.adult_wandered = number(*c, "adult_wandered", path);
  p.child_chose = number(*c, "child_chose", path);
  p.child_wandered = number(*c, "child_wandered", path);
  if (p.adult_chose + p.adult_wandered > 1 || p.child_chose + p.child_wandered > 1)
    throw std::runtime_error(path + ": cause shares add to more than 1");
  return p;
}

void MissingPersons::track(World& w, PopId pop, int age, std::int64_t from) {
  Entry e;
  e.pop = pop;
  e.child = age <= params_.child_age_max;
  e.next_start = from;
  e.next_start = from + gap(w, e);
  ++e.draws;
  entries_.push_back(e);
}

void MissingPersons::redraw(World& w, std::int64_t from) {
  for (Entry& e : entries_) {
    if (e.away_until != 0) continue;
    e.next_start = from + gap(w, e);
    ++e.draws;
  }
}

std::int64_t MissingPersons::gap(const World& w, const Entry& e) const {
  const double per_1000 = (e.child ? params_.child_per_1000_year : params_.adult_per_1000_year) * params_.rate_scale;
  if (per_1000 <= 0) return System::kNever / 4;
  const double years = -detmath::log1p(-draw(w, e.pop, e.draws, 0)) / (per_1000 / 1000.0);
  return std::max<std::int64_t>(1, static_cast<std::int64_t>(years * static_cast<double>(kTicksPerYear)));
}

std::int64_t MissingPersons::duration(const World& w, const Entry& e) const {
  double hours = 0;
  if (!hours_to_return(e.child ? params_.child_return : params_.adult_return, draw(w, e.pop, e.draws, 1), hours)) return System::kNever;
  return std::max<std::int64_t>(1, static_cast<std::int64_t>(hours * static_cast<double>(kTicksPerGameHour)));
}

AbsenceCause MissingPersons::cause(const World& w, const Entry& e) const {
  const double u = draw(w, e.pop, e.draws, 2);
  const double chose = e.child ? params_.child_chose : params_.adult_chose;
  const double wandered = e.child ? params_.child_wandered : params_.adult_wandered;
  return u < chose ? AbsenceCause::Chose : u < chose + wandered ? AbsenceCause::Wandered : AbsenceCause::Unknown;
}

void MissingPersons::update(World& w) {
  const std::int64_t now = w.clock.tick;
  for (Entry& e : entries_) {
    if (e.away_until != 0 && e.away_until != System::kNever && e.away_until <= now) {
      if (exp_ && exp_->find(e.pop)) w.queue_intent({0, AbsenceEnd, e.pop, 0, 0});  // back (unless someone else already ended it)
      e.away_until = 0;
      e.next_start = now + gap(w, e);
      ++e.draws;
    }
    if (e.away_until == 0 && e.next_start <= now) {
      if ((exp_ && exp_->find(e.pop)) || (eligible_ && !eligible_(e.pop))) {  // already absent (a possession), or not able to
        e.next_start = now + gap(w, e);
        ++e.draws;
        continue;
      }
      w.queue_intent({0, AbsenceBegin, e.pop, static_cast<std::int64_t>(cause(w, e)), 0});
      const std::int64_t away = duration(w, e);
      e.away_until = away == System::kNever ? System::kNever : now + away;
      ++e.incidents;
      ++e.draws;
    }
  }
}

std::int64_t MissingPersons::next_wake(const World&, std::int64_t now) const {
  std::int64_t next = kNever;
  for (const Entry& e : entries_) {
    const std::int64_t at = e.away_until == 0 ? e.next_start : e.away_until;  // kNever stays kNever
    next = std::min(next, std::max(now, at));
  }
  return next;
}

void MissingPersons::save(ArchiveWriter& w) const {
  w.u64(entries_.size());
  for (const Entry& e : entries_) {
    w.u64(e.pop);
    w.boolean(e.child);
    w.i64(e.draws);
    w.i64(e.next_start);
    w.i64(e.away_until);
    w.i64(e.incidents);
  }
}

void MissingPersons::load(ArchiveReader& r) {
  entries_.clear();
  const std::uint64_t n = r.u64();
  for (std::uint64_t i = 0; i < n; ++i) {
    Entry e;
    e.pop = r.u64();
    e.child = r.boolean();
    e.draws = r.i64();
    e.next_start = r.i64();
    e.away_until = r.i64();
    e.incidents = r.i64();
    entries_.push_back(e);
  }
}

}  // namespace zs
