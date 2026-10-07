// The body: one model for the player, every pop and every walker.
// Specs: docs/tier0/specs/11-body-composition.md, 12-body-wounds.md, 13-capabilities.md
//
// Stored state is the minimum: tissue masses, glycogen, stomach, water and blood live in the ledger
// (integer mg, uL and J); the rest is a few numbers and the wound list. Pain, needs, capabilities,
// speeds and labels are derived every time they are read and never stored.
#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "zs/world.h"

namespace zs {

enum class Region : std::uint8_t {
  Head, Neck, Chest, Abdomen, Pelvis,
  UpperArmL, UpperArmR, ForearmL, ForearmR, HandL, HandR,
  ThighL, ThighR, ShinL, ShinR, FootL, FootR,
};
inline constexpr int kRegionCount = 17;

struct RegionInfo {
  const char* name;
  double mass_fraction;  // de Leva (1996), share of total body mass
  double vessel_weight;  // how much a wound here bleeds (authored, from the prototype table)
};
const RegionInfo& region_info(Region r);

enum class WoundType : std::uint8_t { Scratch, Laceration, Puncture, Bite, Contusion, Fracture };
enum class Depth : std::uint8_t { Skin, Fat, Muscle, Deep };
enum class Cleaned : std::uint8_t { No, Water, Disinfectant };
enum class Infection : std::uint8_t { None, Local, Spreading, Systemic, Resolved };

struct Wound {
  EntityId id = 0;  // own id: keys its hazard draws
  Region region = Region::Chest;
  WoundType type = WoundType::Laceration;
  Depth depth = Depth::Skin;
  double length_mm = 20;
  double contamination = 0.5;  // 0-1
  Cleaned cleaned = Cleaned::No;
  bool dressed = false;
  bool splinted = false;
  bool tourniquet = false;
  Infection infection = Infection::None;
  std::int64_t infected_tick = -1;  // when bacterial infection started (hidden until visible)
  bool infection_visible = false;
  double pathogen_dose = 0;  // walker pathogen (spec 21), recorded only
  double clot = 0;           // 0-1, share of the bleed stopped by clotting
  std::int64_t created_tick = 0;
  std::int64_t heal_tick = 0;  // healed at this tick if no infection is active
  EventId cause = 0;
};

enum class Sex : std::uint8_t { Male, Female };
enum class Posture : std::uint8_t { Stand, Crouch, Sit, Lie };
enum class DeathCause : std::uint8_t { None, Exsanguination, Dehydration, Starvation, Sepsis, Pathogen, BrainDestroyed };
const char* to_string(DeathCause c);
const char* to_string(Region r);
const char* to_string(WoundType t);

// What the body is doing right now. Set by the action executor (stage 4); tests set it directly.
struct Activity {
  double met = 1.3;          // metabolic equivalent of the current action when not walking
  double speed_mps = 0;      // walking or running speed
  double grade_pct = 0;      // slope or stairs
  double carried_kg = 0;     // load, including worn gear
  bool asleep = false;
};

struct BodySpec {
  Sex sex = Sex::Male;
  double age = 30;
  double height_m = 1.75;
  double mass_kg = 75;
  double fat_fraction = 0.18;
  double vo2max = 42;  // mL/kg/min
  bool walker = false;
};

struct Body {
  EntityId id = 0;
  OwnerId owner = 0;
  std::string label;
  BodySpec spec;
  // Normal values set at creation (the reference for deficits and loss fractions).
  std::int64_t blood_normal_ul = 0;
  std::int64_t water_normal_ul = 0;
  std::int64_t muscle_start_mg = 0;
  std::int64_t glycogen_max_mg = 0;
  bool alive = true;
  DeathCause death_cause = DeathCause::None;
  Posture posture = Posture::Stand;
  Activity activity;
  double sleep_pressure = 0.2;  // S, 0-1
  double exertion = 1;          // short-term reserve, 0-1
  double vo2max = 42;
  double fear = 0, morale = 0.7;
  double threat = 0;  // 0-1, what the senses report right now (walkers seen, in reach, holding it)
  bool away = false;  // off the map, cold (spec 27): its body does not run and nothing senses it
  bool vacant = false;           // a pool slot with no occupant (spec 29): not alive, holds nothing, skipped by every rule
  std::uint64_t pop = 0;         // the registry pop whose body this is (a pool slot's occupant), 0 if none
  double active_minutes_today = 0;  // minutes at >= 2.5 MET
  double heavy_minutes_today = 0;   // training stimulus (one value, whole body, in Tier 0)
  std::array<double, 8> carry{};    // saved remainders of the continuous flows (spec 01 rule 2)
  std::vector<Wound> wounds;
};

// Ledger resources and sinks the body uses. Units: mg, uL and J so the slow flows stay exact.
struct BodyLedger {
  ResourceId fat_mg, muscle_mg, structure_mg, glycogen_mg, water_ul, blood_ul, food_j, spent_j;
  OwnerId metabolised, excreted, blood_lost;
  ConversionId food_to_glycogen, food_to_fat, glycogen_burn, fat_burn, muscle_burn, glycogen_to_muscle,
      water_to_muscle, water_to_blood;
};

struct Capabilities {
  double consciousness = 1, moving = 1, manipulation_l = 1, manipulation_r = 1, sight = 1, hearing = 1;
};

struct Needs {
  double hunger = 0, thirst = 0, tiredness = 0, exhaustion = 0;  // 0-1
};

// Everything the body model holds about one body, so a body can leave the simulation and come back exactly
// (spec 29). The tissue amounts are the body owner's ledger balances in mg, uL and J.
struct BodyState {
  BodySpec spec;
  std::string label;
  std::int64_t blood_normal_ul = 0, water_normal_ul = 0, muscle_start_mg = 0, glycogen_max_mg = 0;
  Posture posture = Posture::Stand;
  Activity activity;
  double sleep_pressure = 0.2, exertion = 1, vo2max = 42, fear = 0, morale = 0.7, threat = 0;
  double active_minutes_today = 0, heavy_minutes_today = 0;
  std::array<double, 8> carry{};
  std::vector<Wound> wounds;
  std::int64_t fat_mg = 0, muscle_mg = 0, structure_mg = 0, glycogen_mg = 0, water_ul = 0, blood_ul = 0, food_j = 0;
};
void write_body_state(ArchiveWriter& w, const BodyState& s);
BodyState read_body_state(ArchiveReader& r);

class Bodies : public System {
 public:
  explicit Bodies(World& w);  // registers resources, sinks and conversions (setup)
  const char* name() const override { return "bodies"; }
  void update(World& w) override;
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

  const BodyLedger& ids() const { return L_; }

  // Creates a body and mints its tissues (setup only).
  EntityId create(World& w, const BodySpec& spec, const std::string& label);

  // Spec 29: a pool of body slots reserved at setup, for pops that enter and leave the body simulation. A vacant
  // slot is a body that is not alive, holds nothing and is skipped by every rule; `all()` includes vacant slots,
  // so a reader in a world with a pool must skip `vacant` bodies.
  void reserve_slots(World& w, std::size_t n);  // setup only
  const std::vector<EntityId>& slots() const { return slots_; }
  std::size_t free_slots() const;
  // Puts a body in the first free slot: its tissue enters the ledger as a declared inflow caused by `cause`.
  // Returns 0 if no slot is free (the request is refused, never queued).
  EntityId embody(World& w, const BodySpec& spec, const std::string& label, std::uint64_t pop, EventId cause);
  EntityId embody_state(World& w, const BodyState& state, std::uint64_t pop, EventId cause);
  // Why a body cannot leave right now ("" if it can): not a pool body, not alive, a walker, away, or holding items.
  std::string cannot_disembody(const World& w, EntityId body) const;
  // Takes a body out of its slot, returning everything it held; its tissue leaves the ledger as a declared outflow.
  BodyState disembody(World& w, EntityId body, EventId cause);
  // A pool body that died while embodied leaves its slot the same way (the corpse as a thing in the world is a later
  // unit: for now its mass leaves the ledger as a declared outflow). Throws if the body is alive or not an occupied slot.
  BodyState release_dead(World& w, EntityId body, EventId cause);
  bool exists(EntityId id) const { return bodies_.contains(id); }
  const Body& get(EntityId id) const { return bodies_.at(id); }
  Body& get(EntityId id) { return bodies_.at(id); }
  const std::map<EntityId, Body>& all() const { return bodies_; }  // includes vacant pool slots

  // Masses (g, kg) read from the ledger.
  double grams(const Body& b, ResourceId r) const;
  double mass_kg(const Body& b) const;
  double fat_fraction(const Body& b) const;

  // Things done to a body. Food and water move from `source` (a pantry, a bottle).
  std::int64_t eat(World& w, EntityId body, OwnerId source, std::int64_t food_j, EventId cause);
  std::int64_t drink(World& w, EntityId body, OwnerId source, std::int64_t water_ul, EventId cause);
  EntityId add_wound(World& w, EntityId body, Wound wound, EventId cause);  // emits NewWound
  void clean(EntityId body, EntityId wound, Cleaned how);
  void dress(EntityId body, EntityId wound, bool on);
  void splint(EntityId body, EntityId wound, bool on);
  void tourniquet(EntityId body, EntityId wound, bool on);
  // Stage 7: death from outside the body model (the pathogen, a destroyed brain), and rising as a
  // walker with everything the body had (spec 21 rule 4).
  void kill(World& w, EntityId body, DeathCause c, EventId cause);
  void reanimate(World& w, EntityId body, EventId cause);

  // Derived readouts: pure functions of the state.
  double region_pain(const Body& b, Region r, std::int64_t now) const;
  double pain(const Body& b, std::int64_t now) const;
  double blood_loss_fraction(const Body& b) const;
  double water_deficit_fraction(const Body& b) const;  // share of the body's normal mobile water
  double core_temp_c(const Body& b) const;             // 37 C plus fever from infected wounds
  double bleed_ml_per_min(const Body& b) const;
  double wound_bleed_ml_per_min(const Body& b, const Wound& wound) const;
  Capabilities capabilities(const Body& b, std::int64_t now) const;
  Needs needs(const Body& b) const;
  double carry_limit_kg(const Body& b, std::int64_t now) const;
  double walk_speed(const Body& b, std::int64_t now) const;
  double run_speed(const Body& b, std::int64_t now) const;
  double work_rate(const Body& b, std::int64_t now) const;
  double push_force_n(const Body& b, std::int64_t now) const;
  double footstep_db(const Body& b, double speed_mps, double floor_db) const;
  double expenditure_w(const Body& b) const;  // current metabolic power
  std::vector<std::string> trait_labels(const Body& b) const;

 private:
  BodyState vacate(World& w, EntityId body, EventId cause);  // the shared tail of disembody and release_dead
  void fine_tick(World& w, Body& b);
  void body_step(World& w, Body& b);
  void metabolism(World& w, Body& b, double dt_s);
  void water(World& w, Body& b, double dt_s);
  void sleep_and_exertion(World& w, Body& b, double dt_s);
  void wounds_step(World& w, Body& b, double dt_s);
  void daily_drift(World& w, Body& b);
  void check_death(World& w, Body& b, EventId cause);
  void die(World& w, Body& b, DeathCause c, EventId cause);
  // Moves amount (plus the saved remainder of flow `slot`) as whole units; returns what moved.
  std::int64_t flow(double& carry, double amount);

  struct Amounts {
    double blood_l, fat_kg, muscle_kg, structure_kg, water_kg, glycogen_kg;
  };
  static Amounts amounts_for(const BodySpec& spec);  // the tissue split of a spec, shared by create and embody
  BodyLedger L_{};
  std::vector<EntityId> slots_;  // the pool, in the order a free slot is chosen
  const Ledger* world_ledger_ = nullptr;  // the owning world's ledger, for the readouts
  std::map<EntityId, Body> bodies_;
};

}  // namespace zs
