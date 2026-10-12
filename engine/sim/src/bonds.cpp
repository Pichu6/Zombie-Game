// The household after an absence or a lost job. Spec: docs/tier0/specs/63-household-bonds.md (decision 0070 rule 5)
#include "zs/bonds.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "toml.hpp"
#include "zs/detmath.h"
#include "zs/pop_system.h"

namespace zs {
namespace {

constexpr int kCapMilli = 1000;

int milli_of(const toml::table& t, const char* table, const char* key, const std::string& path) {
  const toml::table* k = t[table].as_table();
  const auto v = k ? (*k)[key].value<std::int64_t>() : std::nullopt;
  if (!v || *v < 0 || *v > kCapMilli) throw std::runtime_error(path + ": [" + table + "] needs " + key + " from 0 to 1000");
  return static_cast<int>(*v);
}

}  // namespace

BondParams load_bond_params(const std::string& path) {
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  BondParams p;
  p.stage_milli[static_cast<int>(AbsenceStage::Broken)] = milli_of(t, "stage_milli", "broken", path);
  p.stage_milli[static_cast<int>(AbsenceStage::Asked)] = milli_of(t, "stage_milli", "asked", path);
  p.stage_milli[static_cast<int>(AbsenceStage::Reported)] = milli_of(t, "stage_milli", "reported", path);
  p.stage_milli[static_cast<int>(AbsenceStage::Searching)] = milli_of(t, "stage_milli", "searching", path);
  const auto pct = [&](const char* key) {
    const toml::table* k = t["cause_percent"].as_table();
    const auto v = k ? (*k)[key].value<std::int64_t>() : std::nullopt;
    if (!v || *v < 0 || *v > 1000) throw std::runtime_error(path + ": [cause_percent] needs " + key + " from 0 to 1000");
    return static_cast<int>(*v);
  };
  p.cause_percent[static_cast<int>(AbsenceCause::Possessed)] = pct("possessed");
  p.cause_percent[static_cast<int>(AbsenceCause::Ill)] = pct("ill");
  p.cause_percent[static_cast<int>(AbsenceCause::Chose)] = pct("chose");
  p.cause_percent[static_cast<int>(AbsenceCause::Unknown)] = pct("unknown");
  p.cause_percent[static_cast<int>(AbsenceCause::Wandered)] = pct("wandered");
  p.job_loss_milli = milli_of(t, "job_loss", "milli", path);
  const auto half = t["bond"]["half_life_days"].value<double>();
  if (!half || *half <= 0) throw std::runtime_error(path + ": [bond] needs a positive half_life_days");
  p.half_life_days = *half;
  p.leave_threshold_milli = milli_of(t, "bond", "leave_threshold_milli", path);
  if (p.leave_threshold_milli == 0) throw std::runtime_error(path + ": [bond] leave_threshold_milli must be above 0");
  return p;
}

Bonds::Bond* Bonds::find(PopId holder, PopId pop) {
  for (Bond& b : bonds_)
    if (b.holder == holder && b.pop == pop) return &b;
  return nullptr;
}

const Bonds::Bond* Bonds::find(PopId holder, PopId pop) const {
  for (const Bond& b : bonds_)
    if (b.holder == holder && b.pop == pop) return &b;
  return nullptr;
}

int Bonds::decayed(const Bond& b, std::int64_t tick) const {
  const std::int64_t dt = tick - b.tick;
  if (dt <= 0 || b.milli <= 0) return b.milli;
  const double halves = static_cast<double>(dt) / (params_.half_life_days * static_cast<double>(kTicksPerGameDay));
  return static_cast<int>(std::llround(static_cast<double>(b.milli) * detmath::pow(0.5, halves)));
}

int Bonds::strain(PopId holder, PopId pop, std::int64_t tick) const {
  const Bond* b = find(holder, pop);
  return b ? decayed(*b, tick) : 0;
}

bool Bonds::left_household(PopId pop) const { return std::find(left_.begin(), left_.end(), pop) != left_.end(); }

// Adds strain to one bond (decayed to now first), says so, and past the limit the holder leaves the household.
void Bonds::add(World& w, PopId holder, PopId pop, int milli, EventId cause) {
  if (milli <= 0 || left_household(holder)) return;
  const std::int64_t now = w.clock.tick;
  Bond* b = find(holder, pop);
  if (!b) {
    bonds_.push_back({holder, pop, 0, now});
    b = &bonds_.back();
  }
  b->milli = std::min(kCapMilli, decayed(*b, now) + milli);
  b->tick = now;
  const EventId strained = w.emit(EventType::BondStrained, holder, cause, static_cast<std::int64_t>(pop), b->milli);
  if (b->milli >= params_.leave_threshold_milli) {
    left_.push_back(holder);
    if (const std::uint64_t roster = household_of_ ? household_of_(holder) : 0)
      w.queue_intent({0, RosterLeave, holder, static_cast<std::int64_t>(roster), 0});
    w.emit(EventType::LeftHousehold, holder, strained, static_cast<std::int64_t>(pop), b->milli);
  }
}

void Bonds::on_absence_ended(World& w, const Expectations::Case& c, EventId ended) {
  const int weight = params_.stage_milli[static_cast<int>(c.stage)];  // nobody noticed before a commitment broke: nothing
  if (weight == 0) return;
  const int milli = weight * params_.cause_percent[static_cast<int>(c.cause)] / 100;
  for (PopId holder : c.holders) add(w, holder, c.pop, milli, ended);
}

void Bonds::on_separated(World& w, const Expectations::Case& c, EventId separation) {
  for (PopId holder : c.holders) add(w, holder, c.pop, params_.job_loss_milli, separation);
}

void Bonds::save(ArchiveWriter& w) const {
  w.u64(bonds_.size());
  for (const Bond& b : bonds_) {
    w.u64(b.holder);
    w.u64(b.pop);
    w.i64(b.milli);
    w.i64(b.tick);
  }
  w.u64(left_.size());
  for (PopId p : left_) w.u64(p);
}

void Bonds::load(ArchiveReader& r) {
  bonds_.clear();
  left_.clear();
  const std::uint64_t n = r.u64();
  for (std::uint64_t i = 0; i < n; ++i) {
    Bond b;
    b.holder = r.u64();
    b.pop = r.u64();
    b.milli = static_cast<int>(r.i64());
    b.tick = r.i64();
    bonds_.push_back(b);
  }
  const std::uint64_t left = r.u64();
  for (std::uint64_t i = 0; i < left; ++i) left_.push_back(r.u64());
}

}  // namespace zs
