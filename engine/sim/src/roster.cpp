#include "zs/roster.h"

#include <algorithm>

namespace zs {

bool Roster::join(PopId pop, double year) {
  if (year < last_year_) return false;
  auto& list = stints_[pop];
  if (!list.empty() && list.back().left == kNever) return false;
  if (!list.empty() && list.back().left > year) return false;
  list.push_back({year, kNever});
  last_year_ = year;
  return true;
}

bool Roster::leave(PopId pop, double year) {
  if (year < last_year_) return false;
  const auto it = stints_.find(pop);
  if (it == stints_.end() || it->second.empty() || it->second.back().left != kNever || it->second.back().joined > year)
    return false;
  it->second.back().left = year;
  last_year_ = year;
  return true;
}

bool Roster::present(PopId pop, double year) const {
  const auto it = stints_.find(pop);
  if (it == stints_.end()) return false;
  for (const Stint& s : it->second)
    if (s.joined <= year && year < s.left) return true;
  return false;
}

std::uint64_t Roster::size(double year) const {
  std::uint64_t n = 0;
  for (const auto& [pop, list] : stints_)
    for (const Stint& s : list)
      if (s.joined <= year && year < s.left) { ++n; break; }
  return n;
}

std::vector<PopId> Roster::members(double year) const {
  std::vector<PopId> out;
  for (const auto& [pop, list] : stints_)
    for (const Stint& s : list)
      if (s.joined <= year && year < s.left) { out.push_back(pop); break; }
  return out;
}

std::vector<PopId> Roster::ever_members() const {
  std::vector<PopId> out;
  for (const auto& [pop, list] : stints_) out.push_back(pop);
  return out;
}

bool Roster::apply(std::uint64_t seed, EffectKind kind, std::int64_t delta, double year, std::uint64_t reason) {
  if (year < last_year_ || static_cast<std::uint8_t>(kind) >= static_cast<std::uint8_t>(EffectKind::kCount)) return false;
  auto& hits = hits_[static_cast<int>(kind)];
  if (delta > 0) {
    std::vector<std::pair<std::uint64_t, PopId>> candidates;
    const std::uint64_t stream = (id_ * 64 + static_cast<std::uint64_t>(kind)) ^ (serial_ * 0x9E3779B97F4A7C15ull);
    for (PopId pop : members(year)) {
      if (in_state(pop, kind, year)) continue;
      candidates.push_back({keyed_u64(seed, Purpose::PopRank, stream, pop), pop});
    }
    const auto want = static_cast<std::uint64_t>(delta);
    if (want > candidates.size()) return false;
    std::partial_sort(candidates.begin(), candidates.begin() + static_cast<std::ptrdiff_t>(want), candidates.end());
    for (std::uint64_t i = 0; i < want; ++i) hits.push_back({candidates[i].second, year, kNever, reason});
  } else if (delta < 0) {
    std::uint64_t give = static_cast<std::uint64_t>(-delta);
    std::vector<std::size_t> chosen;
    for (std::size_t i = hits.size(); i-- > 0 && chosen.size() < give;)
      if (active(hits[i], year)) chosen.push_back(i);  // the newest first; members who left are skipped
    if (chosen.size() < give) return false;
    for (std::size_t i : chosen) hits[i].ended = year;
  }
  last_year_ = year;
  ++serial_;
  return true;
}

std::uint64_t Roster::count(EffectKind kind, double year) const {
  std::uint64_t n = 0;
  for (const Hit& h : hits_[static_cast<int>(kind)]) n += active(h, year) ? 1 : 0;
  return n;
}

bool Roster::in_state(PopId pop, EffectKind kind, double year) const {
  for (const Hit& h : hits_[static_cast<int>(kind)])
    if (h.pop == pop && active(h, year)) return true;
  return false;
}

std::uint64_t Roster::reason_of(PopId pop, EffectKind kind, double year) const {
  for (const Hit& h : hits_[static_cast<int>(kind)])
    if (h.pop == pop && active(h, year)) return h.reason;
  return 0;
}

std::vector<PopId> Roster::in_state_members(EffectKind kind, double year) const {
  std::vector<PopId> out;
  for (const Hit& h : hits_[static_cast<int>(kind)])
    if (active(h, year)) out.push_back(h.pop);
  return out;
}

void Roster::save(ArchiveWriter& w) const {
  w.u64(id_); w.u32(region_); w.u8(static_cast<std::uint8_t>(kind_)); w.f64(last_year_); w.u64(serial_);
  w.u64(stints_.size());
  for (const auto& [pop, list] : stints_) {
    w.u64(pop); w.u32(static_cast<std::uint32_t>(list.size()));
    for (const Stint& s : list) { w.f64(s.joined); w.f64(s.left); }
  }
  for (const auto& hits : hits_) {
    w.u64(hits.size());
    for (const Hit& h : hits) { w.u64(h.pop); w.f64(h.year); w.f64(h.ended); w.u64(h.reason); }
  }
}

void Roster::load(ArchiveReader& r, std::uint32_t registry_version) {
  id_ = r.u64(); region_ = r.u32();
  kind_ = registry_version >= 3 ? static_cast<RosterKind>(r.u8()) : RosterKind::Other;  // older saves had no kinds
  last_year_ = r.f64(); serial_ = r.u64();
  stints_.clear();
  for (std::uint64_t i = 0, n = r.u64(); i < n; ++i) {
    const PopId pop = r.u64();
    std::vector<Stint> list(r.u32());
    for (Stint& s : list) { s.joined = r.f64(); s.left = r.f64(); }
    stints_[pop] = std::move(list);
  }
  for (auto& hits : hits_) {
    hits.clear();
    for (std::uint64_t i = 0, n = r.u64(); i < n; ++i) {
      Hit h;
      h.pop = r.u64(); h.year = r.f64(); h.ended = r.f64(); h.reason = r.u64();
      hits.push_back(h);
    }
  }
}

}  // namespace zs
