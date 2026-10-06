// Cohort effects, record store and promotion. Spec: docs/tier0/specs/28-pop-levels.md (P2, P3)
// A pop's state is derive() + the effects on its cohorts + its own exceptions. Looking never changes
// anything; promoting only pins a pop (a record) so it can keep things the seed cannot say.
#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "zs/archive.h"
#include "zs/demography.h"
#include "zs/pop.h"
#include "zs/roster.h"

namespace zs {

enum class Level : std::uint8_t { Lazy = 0, Record = 1 };  // Embodied comes with the body (stage 3 code)

// A generated group. Members are census slots first_slot .. first_slot + size - 1 (a placeholder for
// the human generator's rule); a cohort is a fixed snapshot (spec 28 rule 11).
struct Cohort {
  std::uint64_t id = 0;
  std::uint32_t region = 0;
  std::uint32_t first_slot = 0;
  std::uint64_t size = 0;
};

// An aggregate event: `delta` more members enter the state (negative: that many leave again, last in
// first out). `reason` is the order, edict or project behind it (0 = none).
struct CohortEffect {
  std::uint64_t cohort = 0;
  EffectKind kind = EffectKind::LaidOff;
  std::int64_t delta = 0;
  double year = 0.0;
  std::uint64_t reason = 0;
};

enum class ExceptionField : std::uint8_t { DeathYear = 0 };
struct PopException {
  ExceptionField field = ExceptionField::DeathYear;
  double value = 0.0;
};

struct PopSnapshot {
  PopBase base;
  std::uint32_t state_bits = 0;   // bit k set: in EffectKind k at the asked year, through its generated cohort
  std::uint32_t roster_bits = 0;  // the same, through any roster it belongs to at that year
  Level level = Level::Lazy;
  std::uint32_t office = 0;      // 0 = none
  bool followed = false;
};

struct PopCounts {
  std::uint64_t total = 0, records = 0, lazy = 0;
};

class PopRegistry {
 public:
  // The parameters are copied in and saved with the registry: later changes to any preset never touch this world.
  explicit PopRegistry(std::uint64_t world_seed, const PopParams& params = default_pop_params())
      : seed_(world_seed), params_(params) {}
  const PopParams& params() const { return params_; }

  // Cohorts. Returns false if the id is taken or the slots overlap another cohort.
  bool add_cohort(const Cohort& c);
  const Cohort* cohort(std::uint64_t id) const;

  // Effects. Returns false (and changes nothing) if it would hit more members than the cohort has,
  // or give back more than are in the state.
  bool apply(const CohortEffect& e);
  std::uint64_t count(std::uint64_t cohort, EffectKind kind, double year) const;
  std::uint64_t count_region(std::uint32_t region, EffectKind kind, double year) const;
  // Which order put this member in the state at `year` (0 if none, or if it is not in the state).
  std::uint64_t reason_of(std::uint64_t cohort, std::uint64_t member, EffectKind kind, double year) const;
  bool member_in(std::uint64_t cohort, std::uint64_t member, EffectKind kind, double year) const;

  // The pop as it is at `year`, at whatever level it is (a pure read). A founder is found by its cohort, a
  // born pop by walking its mothers (so it exists only while its mother's births do). Effects reach
  // founders only for now: a born pop has no cohort, so its state_bits are 0. False if there is no such pop.
  bool snapshot(PopId id, double year, PopSnapshot& out) const;

  // Levels. Promotion pins a pop; demotion is refused for an officeholder or a followed pop.
  Level level(PopId id) const { return records_.count(id) ? Level::Record : Level::Lazy; }
  bool promote(PopId id);
  bool demote(PopId id);
  bool set_office(PopId id, std::uint32_t office);  // promotes; 0 clears
  bool set_followed(PopId id, bool followed);       // promotes
  bool set_death_year(PopId id, double year);       // an exception; works at either level
  PopCounts counts() const;

  // Rosters: explicit groups whose members change (a workplace, a household, an office). A pop in a
  // roster is a listed id, not a record; it is promoted only when touched on its own.
  bool add_roster(std::uint64_t id, std::uint32_t region);
  const Roster* roster(std::uint64_t id) const;
  bool roster_join(std::uint64_t roster, PopId pop, double year);
  bool roster_leave(std::uint64_t roster, PopId pop, double year);
  bool roster_apply(std::uint64_t roster, EffectKind kind, std::int64_t delta, double year, std::uint64_t reason);
  std::uint64_t roster_count(std::uint64_t roster, EffectKind kind, double year) const;
  std::uint64_t roster_count_region(std::uint32_t region, EffectKind kind, double year) const;

  // The jump: births, deaths and who is alive over (from, to] for every founder in every cohort and
  // their descendants, applying death-year exceptions. Pure; cost grows with everyone ever in the trees.
  DemographyCounts project_demography(double from, double to) const;

  // Cheap internal checks; returns a description of the first problem (counts add up, every record is a pop).
  std::optional<std::string> audit() const;

  std::vector<std::uint8_t> save() const;
  void load(const std::vector<std::uint8_t>& bytes);  // into a registry made with the same seed
  std::uint64_t hash() const;

 private:
  struct Record {
    std::uint32_t office = 0;
    bool followed = false;
    std::vector<PopException> exceptions;
  };
  struct Range { std::uint64_t begin, end, reason; };
  const Cohort* cohort_of(PopId id, std::uint64_t& member) const;
  bool exists(PopId id) const;
  bool resolve_base(PopId id, PopBase& out) const;  // founder or born, before the pop's own exceptions
  double founder_age(PopId id) const;
  DeathOverrides death_overrides() const;
  CohortPermutation permutation(const Cohort& c, EffectKind kind) const;
  std::vector<Range> ranges(std::uint64_t cohort, EffectKind kind, double year) const;
  void set_exception(std::vector<PopException>& list, const PopException& e);
  void index_effect(const CohortEffect& e);
  void load_v1(ArchiveReader& r);

  std::uint64_t seed_;
  PopParams params_;
  std::map<std::uint64_t, Cohort> cohorts_;            // by id
  std::map<std::uint32_t, std::uint64_t> by_slot_;     // first_slot -> cohort id
  std::vector<CohortEffect> log_;                      // append-only, the single truth for effects
  // Derived from log_ (rebuilt on load): a cohort's effects of one kind, in order, and a region's cohorts.
  std::map<std::pair<std::uint64_t, std::uint8_t>, std::vector<CohortEffect>> by_cohort_kind_;
  std::map<std::uint32_t, std::vector<std::uint64_t>> by_region_;
  std::map<PopId, Record> records_;                    // pinned pops
  std::map<PopId, std::vector<PopException>> kept_;    // exceptions of lazy pops
  std::map<std::uint64_t, Roster> rosters_;
  std::map<PopId, std::vector<std::uint64_t>> rosters_of_;  // derived from the rosters; rebuilt on load
};

}  // namespace zs
