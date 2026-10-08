// Spec: docs/tier0/specs/40-time.md
#include "zs/pacer.h"

#include <algorithm>
#include <cmath>

namespace zs {

double pace_factor(Pace s) {
  switch (s) {
    case Pace::Pause: return 0;
    case Pace::Quarter: return 0.25;
    case Pace::Half: return 0.5;
    case Pace::Real: return 1;
    case Pace::X3: return 3;
    case Pace::X10: return 10;
    case Pace::DayPerHour: return 24;
    case Pace::X60: return 60;
    case Pace::X360: return 360;
    case Pace::DayPerSecond: return 86400;
    case Pace::WeekPerSecond: return 7 * 86400;
    case Pace::MonthPerSecond: return 30 * 86400;
    case Pace::YearPerSecond: return 365 * 86400;
  }
  return 1;
}

const char* pace_name(Pace s) {
  switch (s) {
    case Pace::Pause: return "paused";
    case Pace::Quarter: return "1/4";
    case Pace::Half: return "1/2";
    case Pace::Real: return "1:1";
    case Pace::X3: return "3x";
    case Pace::X10: return "10x";
    case Pace::DayPerHour: return "1 hour = 1 day";
    case Pace::X60: return "60x";
    case Pace::X360: return "360x";
    case Pace::DayPerSecond: return "a day a second";
    case Pace::WeekPerSecond: return "a week a second";
    case Pace::MonthPerSecond: return "a month a second";
    case Pace::YearPerSecond: return "a year a second";
  }
  return "?";
}

Pace faster(Pace s) { return s == Pace::YearPerSecond ? s : static_cast<Pace>(static_cast<std::uint8_t>(s) + 1); }
Pace slower(Pace s) { return s == Pace::Pause ? s : static_cast<Pace>(static_cast<std::uint8_t>(s) - 1); }

void Pacer::cap(const std::string& who, Pace at_most, std::int64_t) { caps_[who] = Cap{at_most, -1}; }

void Pacer::release(const std::string& who, std::int64_t now) {
  auto it = caps_.find(who);
  if (it != caps_.end() && it->second.released_at < 0) it->second.released_at = now;
}

void Pacer::alert(const std::string& what, std::int64_t) { alert_ = what; }
void Pacer::acknowledge() { alert_.clear(); }

const std::string* Pacer::limiting(std::int64_t now) const {
  const std::string* who = nullptr;
  Pace tightest = Pace::YearPerSecond;
  for (const auto& [name, c] : caps_) {
    const bool in_force = c.released_at < 0 || now < c.released_at + dwell_ticks_;
    if (!in_force) continue;
    if (static_cast<std::uint8_t>(c.at_most) < static_cast<std::uint8_t>(tightest) || !who) {
      tightest = c.at_most;
      who = &name;
    }
  }
  if (who && static_cast<std::uint8_t>(tightest) >= static_cast<std::uint8_t>(requested_)) return nullptr;  // the cap is above the request
  return who;
}

Pace Pacer::effective(std::int64_t now) const {
  if (!alert_.empty()) return Pace::Pause;
  Pace s = requested_;
  for (const auto& [name, c] : caps_) {
    const bool in_force = c.released_at < 0 || now < c.released_at + dwell_ticks_;
    if (in_force && static_cast<std::uint8_t>(c.at_most) < static_cast<std::uint8_t>(s)) s = c.at_most;
  }
  return s;
}

std::int64_t Pacer::ticks_for(double real_seconds, std::int64_t now) {
  if (real_seconds <= 0) return 0;
  const double game_seconds = real_seconds * pace_factor(effective(now));
  const double ticks = game_seconds / kTickSeconds + carry_;
  const double whole = std::floor(ticks);
  carry_ = ticks - whole;
  return static_cast<std::int64_t>(whole);
}

void Pacer::save(ArchiveWriter& w) const {
  w.u8(static_cast<std::uint8_t>(requested_));
  w.u64(caps_.size());
  for (const auto& [name, c] : caps_) { w.str(name); w.u8(static_cast<std::uint8_t>(c.at_most)); w.i64(c.released_at); }
  w.str(alert_);
  w.f64(carry_);
  w.i64(dwell_ticks_);
}

void Pacer::load(ArchiveReader& r) {
  requested_ = static_cast<Pace>(r.u8());
  std::map<std::string, Cap> caps;
  const std::uint64_t n = r.u64();
  for (std::uint64_t i = 0; i < n; ++i) {
    const std::string name = r.str();
    Cap c;
    c.at_most = static_cast<Pace>(r.u8());
    c.released_at = r.i64();
    caps[name] = c;
  }
  caps_ = std::move(caps);
  alert_ = r.str();
  carry_ = r.f64();
  dwell_ticks_ = r.i64();
}

}  // namespace zs
