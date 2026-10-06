// An explicit group whose members change over time (a workplace, a household, an office, an army).
// Spec: docs/tier0/specs/28-pop-levels.md (dynamic groups). Unlike a generated cohort, a roster is a
// real list of pop ids: it is the ledger's owner of "works here". An aggregate event picks its members
// once, deterministically, and stores them, so counts and membership always agree, however the group
// grows or shrinks, with no versions and no permutation that could shift under old events.
#pragma once
#include <cstdint>
#include <limits>
#include <map>
#include <vector>

#include "zs/archive.h"
#include "zs/pop.h"

namespace zs {

// What an aggregate event can put members into. Append only: the index is saved.
enum class EffectKind : std::uint8_t { LaidOff = 0, Moved = 1, Ill = 2, kCount = 3 };

// What a roster is, so a region-wide event can aim at workplaces without touching households. Append only.
enum class RosterKind : std::uint8_t { Other = 0, Household = 1, Workplace = 2, Office = 3, kCount = 4 };

class Roster {
 public:
  Roster() = default;
  Roster(std::uint64_t id, std::uint32_t region, RosterKind kind = RosterKind::Other) : id_(id), region_(region), kind_(kind) {}
  std::uint64_t id() const { return id_; }
  std::uint32_t region() const { return region_; }
  RosterKind kind() const { return kind_; }
  double last_year() const { return last_year_; }  // events and joins never go back before this

  // Membership. A pop can join again after leaving; it cannot join twice at once. Years never go back.
  bool join(PopId pop, double year);
  bool leave(PopId pop, double year);
  bool present(PopId pop, double year) const;
  std::uint64_t size(double year) const;
  std::vector<PopId> members(double year) const;
  std::vector<PopId> ever_members() const;  // anyone who has been on the roster, for past-year questions

  // An aggregate event: `delta` more members enter the state (negative: that many are given back,
  // the newest first). Returns false and changes nothing if it cannot happen. The members picked are
  // those present and not already in the state with the smallest keyed values, so the same history
  // always picks the same people.
  bool apply(std::uint64_t world_seed, EffectKind kind, std::int64_t delta, double year, std::uint64_t reason);
  std::uint64_t count(EffectKind kind, double year) const;
  bool in_state(PopId pop, EffectKind kind, double year) const;
  std::uint64_t reason_of(PopId pop, EffectKind kind, double year) const;  // 0 if not in the state
  std::vector<PopId> in_state_members(EffectKind kind, double year) const;

  void save(ArchiveWriter& w) const;
  void load(ArchiveReader& r, std::uint32_t registry_version);  // the kind is stored from registry version 3

 private:
  struct Stint { double joined, left; };
  struct Hit { PopId pop; double year; double ended; std::uint64_t reason; };
  static constexpr double kNever = std::numeric_limits<double>::infinity();
  bool active(const Hit& h, double year) const { return h.year <= year && year < h.ended && present(h.pop, year); }

  std::uint64_t id_ = 0;
  std::uint32_t region_ = 0;
  RosterKind kind_ = RosterKind::Other;
  double last_year_ = -kNever;
  std::uint64_t serial_ = 0;                  // events applied, part of the pick's key
  std::map<PopId, std::vector<Stint>> stints_;
  std::vector<Hit> hits_[static_cast<int>(EffectKind::kCount)];  // chronological
};

}  // namespace zs
