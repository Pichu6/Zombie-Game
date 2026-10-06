// The dead: the census of neighbours, the walker brain, pushes on doors, bites, turning, strikes.
// Specs: docs/tier0/specs/10-fortification.md (attacks), 20-damage.md, 21-infection.md, 22-census.md,
// 23-walker-brain.md
//
// Compressed model: every walker is a census person with a real body. Walkers perceive with the same
// noise and light fields as everyone, remember the strongest stimulus, path over cells (closed doors
// and windows are things to push, walls are not), push edges part by part, grab and bite. A bite can
// carry the pathogen; the bitten die on a drawn schedule and rise with everything they had.
#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "zs/noise.h"
#include "zs/power.h"

namespace zs {

class Walkers;

// ---- Census (spec 22) ----

enum class Fate : std::uint8_t { Fled, Dead, Survivor, WalkerOffMap, WalkerOnMap, Destroyed, Pop };
inline constexpr std::size_t kFateCount = 7;
const char* to_string(Fate f);

struct Person {
  std::string name, household, home;
  int age = 30;
  Sex sex = Sex::Male;
  bool survivor = false;
  Fate fate = Fate::Fled;
  EntityId body = 0;  // embodied people only
};

class Census : public System {
 public:
  Census(World& w, SiteMap& map, Assemblies& items, Bodies& bodies, Power& power);
  const char* name() const override { return "census"; }
  void update(World& w) override;
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& r) override;

  // Setup: reads the table, draws fates and bodies, puts the dead at home in their houses.
  void load_file(World& w, const std::string& path, Walkers& walkers);
  // A pop of the run (the player) joins the census so conservation covers it.
  void add_pop(World& w, EntityId body, const std::string& name);
  // Census rows still waiting as survivor slots (rule 5), each with a living body.
  std::vector<std::size_t> survivors() const;
  // Setup: gives a survivor slot its body (rule 5) when the run places them.
  EntityId embody(World& w, std::size_t person);

  const std::vector<Person>& people() const { return people_; }
  std::optional<std::size_t> person_of(EntityId body) const;
  std::string name_of(EntityId body) const;
  std::int64_t count(const World& w, Fate f) const;  // read from the ledger
  double attention() const { return attention_; }
  int off_map_walkers() const;

  // State changes (each moves one person between ledger owners).
  void set_fate(World& w, std::size_t person, Fate f);

 private:
  void arrive(World& w, std::size_t person);
  double border_m(const Cell& c) const;

  SiteMap& map_;
  Assemblies& items_;
  Bodies& bodies_;
  Power& power_;
  Walkers* walkers_ = nullptr;
  ResourceId people_r_ = 0;
  std::array<OwnerId, kFateCount> owners_{};
  std::vector<Person> people_;
  std::map<EntityId, std::size_t> by_body_;
  std::vector<Cell> entries_;  // walkable outdoor cells on the map border, ground level
  // State (saved): fates live in people_ (saved), attention below.
  double attention_ = 0;
};

// ---- Walkers (specs 20, 21, 23, 10) ----

enum class Mode : std::uint8_t { Idle, Investigate, Pursue, Bash, Attack, Feed };
const char* to_string(Mode m);

struct WalkerState {
  EntityId body = 0;
  Cell pos{};
  Mode mode = Mode::Idle;
  Cell goal{};
  double memory_db = 0;        // strength of the remembered stimulus (dB above hearing, or a sight value)
  std::int64_t memory_tick = 0;
  EntityId target = 0;         // a living body it pursues, or a corpse it feeds on
  std::vector<Cell> path;
  std::size_t next = 0;
  double step_m = 0;
  double walk_mps = -1;  // body walk speed, refreshed when it thinks (-1 = not yet)
  std::optional<EdgeRef> pushing;
  std::int64_t next_think = 0, next_attack = 0;
};

struct Turning {
  EntityId body = 0;
  EventId cause = 0;            // the bite
  std::int64_t prodrome_tick = 0, death_tick = 0;
  std::int64_t rise_after = 0;  // ticks after death
  std::int64_t died_tick = -1;
  bool risen = false;
};

class Walkers : public System {
 public:
  Walkers(World& w, SiteMap& map, Assemblies& items, Bodies& bodies, Actors& actors, Power& power, Census& census);
  const char* name() const override { return "walkers"; }
  void update(World& w) override;
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& r) override;

  // Setup: wires the executor hooks.
  void setup(World& w);
  // A walker body appears at a cell (setup, arrival, rising).
  void spawn(World& w, EntityId body, const Cell& at);

  const std::map<EntityId, WalkerState>& walkers() const { return walkers_; }
  std::optional<Cell> cell_of(EntityId body) const;
  bool held(EntityId body) const { return held_.contains(body); }
  const std::vector<Turning>& turnings() const { return turnings_; }
  // A corpse whose brain was destroyed before it rose (put down): it never rises (spec 21).
  bool put_down(EntityId body) const { return put_down_.contains(body); }
  // Spec 21 rule 7: the debug inspector's truth (the player view never shows it).
  std::optional<Turning> turning_of(EntityId body) const;

  // Damage (spec 20).
  void bite(World& w, EntityId walker, EntityId victim, EventId cause);
  // A strike: energy from the attacker's body and tool; true if the brain was destroyed.
  bool strike(World& w, EntityId attacker, EntityId target, EntityId tool, EventId cause);
  double strike_energy_j(const World& w, EntityId attacker, EntityId tool) const;

  // Fortification (spec 10): a push of `load_n` for `dt_s` on an edge. Returns true if it breached.
  bool push_edge(World& w, const EdgeRef& e, double load_n, double dt_s, EventId cause);

  // Senses, shared rules (specs 18, 19).
  bool line_of_sight(const World& w, const Cell& a, const Cell& b) const;
  // Arm's reach: the same cell, or the next cell with nothing closed between (no diagonals,
  // nothing through glass or a shut door). Strikes, grabs and bites all use it.
  bool in_reach(const Cell& a, const Cell& b) const;
  // What a living person sees of the dead: walkers in line of sight and light, nearest first
  // (same sight rule the walkers use). Nothing while asleep.
  std::vector<EntityId> visible_to(const World& w, EntityId viewer) const;
  static double sight_range_m(double lux);
  std::vector<NoiseSource> sources(const World& w) const;  // walkers' own noise (pushing)

 private:
  void think(World& w, WalkerState& z);
  void move(World& w, WalkerState& z);
  void attack(World& w, WalkerState& z);
  void plan_path(WalkerState& z);
  bool walker_passes(const EdgeRef& e) const;
  double heard_db(const World& w, const NoiseSource& s, const Cell& at, double background);
  void step_turnings(World& w);
  void sense_threats(World& w);
  void rise(World& w, Turning& t);

  SiteMap& map_;
  Assemblies& items_;
  Bodies& bodies_;
  Actors& actors_;
  Power& power_;
  Census& census_;
  // State (saved).
  std::map<EntityId, WalkerState> walkers_;
  std::map<EntityId, EntityId> held_;  // victim -> walker
  std::vector<Turning> turnings_;
  std::map<EntityId, double> head_j_;  // energy of head blows so far, per walker or corpse (spec 20)
  std::set<EntityId> put_down_;
  // Derived caches (not saved): path loss from a source cell, valid for one map version.
  std::map<std::size_t, std::vector<double>> loss_cache_;
  std::uint64_t cache_version_ = ~std::uint64_t{0};
  std::vector<WindowLeak> leaks_;   // this game minute's window leaks (saved: edge and seen_m only)
  std::int64_t leaks_tick_ = -1;    // the minute they belong to
  std::set<EntityId> crowded_;  // walkers kept off their victim by others this tick (rebuilt every tick)
};

}  // namespace zs
