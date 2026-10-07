// The body. Specs: docs/tier0/specs/11-body-composition.md, 12-body-wounds.md, 13-capabilities.md
#include "zs/body.h"

#include <algorithm>
#include <stdexcept>

#include "zs/detmath.h"
#include "zs/hazard.h"

namespace zs {
namespace {

namespace dm = detmath;

// ---- Calibration (Proposal; every number has a source or is marked authored) ----------------------
// Energy densities (Hall 2008; Forbes 2000): fat 39.5 kJ/g, lean tissue 7.6 kJ/g, glycogen 17 kJ/g.
constexpr std::int64_t kFatJPer2mg = 79, kLeanJPer5mg = 38, kGlycogenJPerMg = 17;
constexpr double kForbesC = 10.4;                 // Forbes (2000): lean share of loss = C / (C + fat kg)
constexpr double kMetW = 1.163;                   // 1 MET = 1 kcal/kg/h (Ainsworth et al. 2011)
constexpr double kStomachTauS = 90 * 60.0;        // gastric emptying, first order (authored, tune)
constexpr double kBaselineWaterLossLPerDay = 1.6; // insensible + obligatory urine (Sawka et al. 2007)
constexpr double kSweatLPerHPerMet = 0.10;        // above 1.5 MET at ~20 C (authored, tune)
constexpr double kKidneyTauS = 45 * 60.0;         // excess water excreted (authored)
constexpr double kBoundWaterPerGlycogen = 3.0;    // g water stored with each g glycogen (Olsson and Saltin 1970)
constexpr double kBloodRefillLPerDay = 1.5;       // plasma refill from body water (authored, tune)
constexpr double kDeathBloodLoss = 0.45;          // past ATLS class IV (> 40 %)
// Water deficit is a share of the body's own mobile pool (like blood): for the 75 kg reference body,
// 0.35 of the pool is ~8.6 % of body mass and 0.65 is ~16 % (Adolph 1947 lethal band; to check).
constexpr double kCriticalWaterDeficit = 0.35;
constexpr double kDeathWaterDeficit = 0.65;
constexpr double kStarvationMuscleLeft = 0.6;     // lean loss beyond ~40 % is lethal (Keys 1950; Hall)
constexpr double kSleepRiseTauH = 18.2, kSleepFallTauH = 4.2;  // Borbely (1982) two-process model
constexpr double kWakeAtS = 0.1, kInvoluntarySleepAtS = 0.98;
constexpr double kExertionDrainPerMinAtMax = 0.1, kExertionRecoverPerMin = 0.05;  // authored
constexpr double kInfectionVisibleH = 36;  // onset shows 24-72 h after infection (clinical; tune)

// de Leva (1996) segment masses (male), head/neck and trunk split authored; vessel weights authored
// in the prototype (one table, not per body).
constexpr RegionInfo kRegions[kRegionCount] = {
    {"head", 0.055, 1.8},       {"neck", 0.0144, 2.5},      {"chest", 0.1596, 1.5},
    {"abdomen", 0.1633, 1.3},   {"pelvis", 0.1117, 2.0},    {"left upper arm", 0.0271, 1.0},
    {"right upper arm", 0.0271, 1.0}, {"left forearm", 0.0162, 0.9}, {"right forearm", 0.0162, 0.9},
    {"left hand", 0.0061, 0.8}, {"right hand", 0.0061, 0.8}, {"left thigh", 0.1416, 1.4},
    {"right thigh", 0.1416, 1.4}, {"left shin", 0.0433, 1.1}, {"right shin", 0.0433, 1.1},
    {"left foot", 0.0137, 0.9}, {"right foot", 0.0137, 0.9},
};

// Bleed at the wound (mL/min) for a 50 mm wound in a vessel weight 1 region, by depth (authored).
constexpr double kBleedByDepth[] = {0.3, 1.0, 6.0, 15.0};
constexpr double kClotTauMin[] = {3, 8, 30, 120};
constexpr double kHealDays[] = {5, 10, 21, 35};
constexpr double kPainByDepth[] = {0.1, 0.2, 0.35, 0.5};

// Infection: untreated dirty laceration ~25 % in 7 days (ED laceration reviews); bites ~3x.
double infection_base_per_day(WoundType t) {
  switch (t) {
    case WoundType::Scratch: return 0.02;
    case WoundType::Laceration: return 0.041;
    case WoundType::Puncture: return 0.06;
    case WoundType::Bite: return 0.123;
    default: return 0;  // closed wounds
  }
}
double clean_factor(Cleaned c) { return c == Cleaned::No ? 1.0 : c == Cleaned::Water ? 0.3 : 0.15; }

constexpr std::int64_t ticks(double seconds) { return static_cast<std::int64_t>(seconds / kTickSeconds); }
double clamp01(double x) { return std::clamp(x, 0.0, 1.0); }
double lerp_band(double x, double x0, double y0, double x1, double y1) {
  if (x <= x0) return y0;
  if (x >= x1) return y1;
  return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

bool is_leg(Region r) {
  return r == Region::Pelvis || (r >= Region::ThighL && r <= Region::FootR);
}
bool is_arm(Region r) { return r >= Region::UpperArmL && r <= Region::HandR; }
bool is_limb(Region r) { return is_arm(r) || (r >= Region::ThighL && r <= Region::FootR); }
bool region_moving(const Body& b, Region r) {
  if (b.activity.asleep) return false;
  if (is_leg(r)) return b.activity.speed_mps > 0;
  if (is_arm(r)) return b.activity.met >= 2.5 || b.activity.speed_mps > 2.2;
  return b.activity.speed_mps > 2.2 || b.activity.met >= 5;  // trunk, neck and head
}

bool open_wound(const Wound& w) { return w.type != WoundType::Contusion && w.type != WoundType::Fracture; }

double wound_bleed(const Body& b, const Wound& w) {
  if (!open_wound(w) || (w.tourniquet && is_limb(w.region))) return 0;
  double rate = kBleedByDepth[static_cast<int>(w.depth)] * kRegions[static_cast<int>(w.region)].vessel_weight *
                (w.length_mm / 50.0) * (1 - w.clot);
  if (w.type == WoundType::Scratch) rate *= 0.3;
  if (w.dressed) rate *= 0.15;
  (void)b;
  return rate;
}

double wound_pain(const Body& b, const Wound& w, std::int64_t now) {
  double p;
  switch (w.type) {
    case WoundType::Scratch: p = 0.05; break;
    case WoundType::Contusion: p = 0.15; break;
    case WoundType::Fracture: p = w.splinted ? 0.3 : 0.7; break;
    default: p = kPainByDepth[static_cast<int>(w.depth)];
  }
  if (w.infection_visible) {
    if (w.infection == Infection::Local) p += 0.15;
    if (w.infection == Infection::Spreading) p += 0.3;
    if (w.infection == Infection::Systemic) p += 0.4;
  }
  const double age_days = static_cast<double>(now - w.created_tick) / kTicksPerGameDay;
  p *= 0.5 + 0.5 * dm::exp(-age_days / 3.0);
  p *= region_moving(b, w.region) ? 1.0 : 0.6;
  return clamp01(p);
}

// How well one limb segment works (1 = fine). An unsplinted fracture blocks it.
double limb_factor(const Body& b, Region r) {
  double f = 1;
  for (const Wound& w : b.wounds) {
    if (w.region != r) continue;
    if (w.type == WoundType::Fracture) f *= w.splinted ? 0.6 : 0.0;
    else if (w.depth == Depth::Deep) f *= 0.7;
    else if (w.depth == Depth::Muscle) f *= 0.85;
    else if (w.type != WoundType::Scratch) f *= 0.97;
  }
  return f;
}

void save_wound(ArchiveWriter& a, const Wound& w) {
  a.u64(w.id);
  a.u8(static_cast<std::uint8_t>(w.region));
  a.u8(static_cast<std::uint8_t>(w.type));
  a.u8(static_cast<std::uint8_t>(w.depth));
  a.f64(w.length_mm);
  a.f64(w.contamination);
  a.u8(static_cast<std::uint8_t>(w.cleaned));
  a.boolean(w.dressed);
  a.boolean(w.splinted);
  a.boolean(w.tourniquet);
  a.u8(static_cast<std::uint8_t>(w.infection));
  a.i64(w.infected_tick);
  a.boolean(w.infection_visible);
  a.f64(w.pathogen_dose);
  a.f64(w.clot);
  a.i64(w.created_tick);
  a.i64(w.heal_tick);
  a.u64(w.cause);
}

Wound load_wound(ArchiveReader& a) {
  Wound w;
  w.id = a.u64();
  w.region = static_cast<Region>(a.u8());
  w.type = static_cast<WoundType>(a.u8());
  w.depth = static_cast<Depth>(a.u8());
  w.length_mm = a.f64();
  w.contamination = a.f64();
  w.cleaned = static_cast<Cleaned>(a.u8());
  w.dressed = a.boolean();
  w.splinted = a.boolean();
  w.tourniquet = a.boolean();
  w.infection = static_cast<Infection>(a.u8());
  w.infected_tick = a.i64();
  w.infection_visible = a.boolean();
  w.pathogen_dose = a.f64();
  w.clot = a.f64();
  w.created_tick = a.i64();
  w.heal_tick = a.i64();
  w.cause = a.u64();
  return w;
}

enum Carry { kGlycogenBurn, kFatBurn, kMuscleBurn, kFoodToGlycogen, kFoodToFat, kWaterLoss, kBleed, kRefill };

}  // namespace

const RegionInfo& region_info(Region r) { return kRegions[static_cast<int>(r)]; }
const char* to_string(Region r) { return kRegions[static_cast<int>(r)].name; }

const char* to_string(DeathCause c) {
  switch (c) {
    case DeathCause::None: return "alive";
    case DeathCause::Exsanguination: return "bled out";
    case DeathCause::Dehydration: return "dehydration";
    case DeathCause::Starvation: return "starvation";
    case DeathCause::Sepsis: return "sepsis from an infected wound";
    case DeathCause::Pathogen: return "the infection from a bite";
    case DeathCause::BrainDestroyed: return "a destroyed brain";
  }
  return "?";
}

const char* to_string(WoundType t) {
  switch (t) {
    case WoundType::Scratch: return "scratch";
    case WoundType::Laceration: return "laceration";
    case WoundType::Puncture: return "puncture";
    case WoundType::Bite: return "bite";
    case WoundType::Contusion: return "contusion";
    case WoundType::Fracture: return "fracture";
  }
  return "?";
}

// ---- Setup ------------------------------------------------------------------------------------------

Bodies::Bodies(World& w) : world_ledger_(&w.ledger) {
  auto& l = w.ledger;
  L_.fat_mg = l.add_resource("fat_mg");
  L_.muscle_mg = l.add_resource("muscle_mg");
  L_.structure_mg = l.add_resource("structure_mg");
  L_.glycogen_mg = l.add_resource("glycogen_mg");
  L_.water_ul = l.add_resource("body_water_ul");
  L_.blood_ul = l.add_resource("blood_ul");
  L_.food_j = l.add_resource("food_j");
  L_.spent_j = l.add_resource("spent_j");
  L_.metabolised = l.add_owner("sink:metabolised");
  L_.excreted = l.add_owner("sink:excreted");
  L_.blood_lost = l.add_owner("sink:blood_lost");
  L_.food_to_glycogen = l.add_conversion({L_.food_j, L_.glycogen_mg, kGlycogenJPerMg, 1});
  L_.food_to_fat = l.add_conversion({L_.food_j, L_.fat_mg, kFatJPer2mg, 2});
  L_.glycogen_burn = l.add_conversion({L_.glycogen_mg, L_.spent_j, 1, kGlycogenJPerMg});
  L_.fat_burn = l.add_conversion({L_.fat_mg, L_.spent_j, 2, kFatJPer2mg});
  L_.muscle_burn = l.add_conversion({L_.muscle_mg, L_.spent_j, 5, kLeanJPer5mg});
  L_.glycogen_to_muscle = l.add_conversion({L_.glycogen_mg, L_.muscle_mg, 1, 1});
  L_.water_to_muscle = l.add_conversion({L_.water_ul, L_.muscle_mg, 1, 1});  // wet muscle is mostly water
  L_.water_to_blood = l.add_conversion({L_.water_ul, L_.blood_ul, 1, 1});
}

Bodies::Amounts Bodies::amounts_for(const BodySpec& spec) {
  const double h = spec.height_m, m = spec.mass_kg;
  const double blood_l = spec.sex == Sex::Male ? 0.3669 * h * h * h + 0.03219 * m + 0.6041   // Nadler (1962)
                                               : 0.3561 * h * h * h + 0.03308 * m + 0.1833;
  const double fat_kg = spec.fat_fraction * m;
  const double muscle_kg = (spec.sex == Sex::Male ? 0.5 : 0.45) * (m - fat_kg);  // skeletal muscle share of FFM
  const double water_kg = 0.3 * (m - fat_kg);  // mobile water pool, held by lean tissue (authored share)
  const double glycogen_kg = 0.1 + 0.012 * muscle_kg;  // liver ~100 g + ~12 g per kg muscle
  const double structure_kg = m - fat_kg - muscle_kg - water_kg - blood_l - glycogen_kg;
  if (structure_kg <= 0) throw std::invalid_argument("body spec leaves no structure mass");
  return {blood_l, fat_kg, muscle_kg, structure_kg, water_kg, glycogen_kg};
}

EntityId Bodies::create(World& w, const BodySpec& spec, const std::string& label) {
  Body b;
  b.id = w.new_entity();
  b.label = label;
  b.spec = spec;
  b.owner = w.ledger.add_owner("body:" + label);
  b.vo2max = std::clamp(spec.vo2max, 15.0, 75.0);
  const Amounts a = amounts_for(spec);
  auto mg = [](double kg) { return static_cast<std::int64_t>(kg * 1e6); };
  b.blood_normal_ul = mg(a.blood_l);
  b.water_normal_ul = mg(a.water_kg);
  b.muscle_start_mg = mg(a.muscle_kg);
  b.glycogen_max_mg = mg(a.glycogen_kg);
  auto& l = w.ledger;
  l.mint(b.owner, L_.fat_mg, mg(a.fat_kg));
  l.mint(b.owner, L_.muscle_mg, mg(a.muscle_kg));
  l.mint(b.owner, L_.structure_mg, mg(a.structure_kg));
  l.mint(b.owner, L_.glycogen_mg, mg(a.glycogen_kg));
  l.mint(b.owner, L_.water_ul, mg(a.water_kg));
  l.mint(b.owner, L_.blood_ul, mg(a.blood_l));
  if (spec.walker) b.sleep_pressure = 0;
  const EntityId id = b.id;
  bodies_[id] = std::move(b);
  return id;
}

// ---- The body pool (spec 29) -------------------------------------------------------------------------

void Bodies::reserve_slots(World& w, std::size_t n) {
  if (!w.ledger.setup_open()) throw std::logic_error("body slots may only be reserved during setup");
  for (std::size_t i = 0; i < n; ++i) {
    Body b;
    b.id = w.new_entity();
    b.label = "slot " + std::to_string(slots_.size());
    b.owner = w.ledger.add_owner("slot:" + std::to_string(slots_.size()));
    b.vacant = true;
    b.alive = false;
    slots_.push_back(b.id);
    bodies_[b.id] = std::move(b);
  }
}

std::size_t Bodies::free_slots() const {
  std::size_t n = 0;
  for (EntityId id : slots_) n += bodies_.at(id).vacant ? 1 : 0;
  return n;
}

namespace {
std::int64_t mg_of(double kg) { return static_cast<std::int64_t>(kg * 1e6); }
}  // namespace

EntityId Bodies::embody(World& w, const BodySpec& spec, const std::string& label, std::uint64_t pop, EventId cause) {
  BodyState st;
  st.spec = spec;
  st.label = label;
  const Amounts a = amounts_for(spec);
  st.blood_normal_ul = mg_of(a.blood_l);
  st.water_normal_ul = mg_of(a.water_kg);
  st.muscle_start_mg = mg_of(a.muscle_kg);
  st.glycogen_max_mg = mg_of(a.glycogen_kg);
  st.vo2max = std::clamp(spec.vo2max, 15.0, 75.0);
  st.sleep_pressure = spec.walker ? 0 : 0.2;
  st.fat_mg = mg_of(a.fat_kg);
  st.muscle_mg = mg_of(a.muscle_kg);
  st.structure_mg = mg_of(a.structure_kg);
  st.glycogen_mg = mg_of(a.glycogen_kg);
  st.water_ul = mg_of(a.water_kg);
  st.blood_ul = mg_of(a.blood_l);
  return embody_state(w, st, pop, cause);
}

EntityId Bodies::embody_state(World& w, const BodyState& st, std::uint64_t pop, EventId cause) {
  EntityId slot = 0;
  for (EntityId id : slots_)
    if (bodies_.at(id).vacant) { slot = id; break; }
  if (slot == 0) return 0;
  Body& b = bodies_.at(slot);
  b.vacant = false;
  b.alive = true;
  b.death_cause = DeathCause::None;
  b.pop = pop;
  b.label = st.label;
  b.spec = st.spec;
  b.blood_normal_ul = st.blood_normal_ul;
  b.water_normal_ul = st.water_normal_ul;
  b.muscle_start_mg = st.muscle_start_mg;
  b.glycogen_max_mg = st.glycogen_max_mg;
  b.posture = st.posture;
  b.activity = st.activity;
  b.sleep_pressure = st.sleep_pressure;
  b.exertion = st.exertion;
  b.vo2max = st.vo2max;
  b.fear = st.fear;
  b.morale = st.morale;
  b.threat = st.threat;
  b.away = false;
  b.active_minutes_today = st.active_minutes_today;
  b.heavy_minutes_today = st.heavy_minutes_today;
  b.carry = st.carry;
  b.wounds = st.wounds;
  for (const Wound& wd : b.wounds) w.locations.place(wd.id, b.id);
  const std::int64_t tick = w.clock.tick;
  w.ledger.declare_inflow(tick, b.owner, L_.fat_mg, st.fat_mg, cause);
  w.ledger.declare_inflow(tick, b.owner, L_.muscle_mg, st.muscle_mg, cause);
  w.ledger.declare_inflow(tick, b.owner, L_.structure_mg, st.structure_mg, cause);
  w.ledger.declare_inflow(tick, b.owner, L_.glycogen_mg, st.glycogen_mg, cause);
  w.ledger.declare_inflow(tick, b.owner, L_.water_ul, st.water_ul, cause);
  w.ledger.declare_inflow(tick, b.owner, L_.blood_ul, st.blood_ul, cause);
  w.ledger.declare_inflow(tick, b.owner, L_.food_j, st.food_j, cause);
  return b.id;
}

std::string Bodies::cannot_disembody(const World& w, EntityId body) const {
  const auto it = bodies_.find(body);
  if (it == bodies_.end() || std::find(slots_.begin(), slots_.end(), body) == slots_.end()) return "not a pool body";
  const Body& b = it->second;
  if (b.vacant) return "slot is vacant";
  if (!b.alive) return "the body is dead";
  if (b.spec.walker) return "a walker";
  if (b.away) return "the body is away";
  for (EntityId child : w.locations.children(body)) {
    const bool is_wound = std::any_of(b.wounds.begin(), b.wounds.end(), [&](const Wound& wd) { return wd.id == child; });
    if (!is_wound) return "the body holds items";
  }
  for (const Wound& wd : b.wounds)
    if (!w.locations.children(wd.id).empty()) return "a wound has a dressing item on it";
  return "";
}

BodyState Bodies::disembody(World& w, EntityId body, EventId cause) {
  if (const std::string why = cannot_disembody(w, body); !why.empty()) throw std::logic_error("disembody: " + why);
  return vacate(w, body, cause);
}

BodyState Bodies::release_dead(World& w, EntityId body, EventId cause) {
  const auto it = bodies_.find(body);
  if (it == bodies_.end() || std::find(slots_.begin(), slots_.end(), body) == slots_.end() || it->second.vacant)
    throw std::logic_error("release_dead: not an occupied pool body");
  if (it->second.alive) throw std::logic_error("release_dead: the body is alive");
  return vacate(w, body, cause);
}

BodyState Bodies::vacate(World& w, EntityId body, EventId cause) {
  Body& b = bodies_.at(body);
  BodyState st;
  st.spec = b.spec;
  st.label = b.label;
  st.blood_normal_ul = b.blood_normal_ul;
  st.water_normal_ul = b.water_normal_ul;
  st.muscle_start_mg = b.muscle_start_mg;
  st.glycogen_max_mg = b.glycogen_max_mg;
  st.posture = b.posture;
  st.activity = b.activity;
  st.sleep_pressure = b.sleep_pressure;
  st.exertion = b.exertion;
  st.vo2max = b.vo2max;
  st.fear = b.fear;
  st.morale = b.morale;
  st.threat = b.threat;
  st.active_minutes_today = b.active_minutes_today;
  st.heavy_minutes_today = b.heavy_minutes_today;
  st.carry = b.carry;
  st.wounds = b.wounds;
  auto& l = w.ledger;
  const std::int64_t tick = w.clock.tick;
  const auto take = [&](ResourceId r, std::int64_t& into) {
    into = l.balance(b.owner, r);
    l.declare_outflow(tick, b.owner, r, into, cause);
  };
  take(L_.fat_mg, st.fat_mg);
  take(L_.muscle_mg, st.muscle_mg);
  take(L_.structure_mg, st.structure_mg);
  take(L_.glycogen_mg, st.glycogen_mg);
  take(L_.water_ul, st.water_ul);
  take(L_.blood_ul, st.blood_ul);
  take(L_.food_j, st.food_j);
  for (const Wound& wd : b.wounds) w.locations.place(wd.id, Locations::kWorld);
  const OwnerId owner = b.owner;
  const auto index = static_cast<std::size_t>(std::find(slots_.begin(), slots_.end(), body) - slots_.begin());
  b = Body{};  // back to a vacant slot: nothing alive, nothing held
  b.id = body;
  b.owner = owner;
  b.label = "slot " + std::to_string(index);
  b.vacant = true;
  b.alive = false;
  return st;
}

// ---- Actions on a body -------------------------------------------------------------------------------

std::int64_t Bodies::eat(World& w, EntityId body, OwnerId source, std::int64_t food_j, EventId cause) {
  Body& b = get(body);
  if (!b.alive) return 0;
  return w.ledger.transfer(w.clock.tick, source, b.owner, L_.food_j, food_j, cause);
}

std::int64_t Bodies::drink(World& w, EntityId body, OwnerId source, std::int64_t water_ul, EventId cause) {
  Body& b = get(body);
  if (!b.alive) return 0;
  return w.ledger.transfer(w.clock.tick, source, b.owner, L_.water_ul, water_ul, cause);
}

EntityId Bodies::add_wound(World& w, EntityId body, Wound wound, EventId cause) {
  Body& b = get(body);
  wound.id = w.new_entity();
  wound.created_tick = w.clock.tick;
  wound.cause = cause;
  const double days = wound.type == WoundType::Fracture ? 42 : kHealDays[static_cast<int>(wound.depth)];
  wound.heal_tick = w.clock.tick + static_cast<std::int64_t>(days * kTicksPerGameDay);
  w.locations.place(wound.id, body);  // dressings go on the wound, so they stay with the body
  b.wounds.push_back(wound);
  w.emit(EventType::NewWound, body, cause, static_cast<std::int64_t>(wound.id), static_cast<std::int64_t>(wound.region));
  return wound.id;
}

namespace {
Wound* find_wound(Body& b, EntityId id) {
  for (Wound& w : b.wounds)
    if (w.id == id) return &w;
  return nullptr;
}
}  // namespace

void Bodies::clean(EntityId body, EntityId wound, Cleaned how) {
  if (Wound* w = find_wound(get(body), wound)) w->cleaned = std::max(w->cleaned, how);
}
void Bodies::dress(EntityId body, EntityId wound, bool on) {
  if (Wound* w = find_wound(get(body), wound)) w->dressed = on;
}
void Bodies::splint(EntityId body, EntityId wound, bool on) {
  if (Wound* w = find_wound(get(body), wound)) w->splinted = on;
}
void Bodies::tourniquet(EntityId body, EntityId wound, bool on) {
  if (Wound* w = find_wound(get(body), wound)) w->tourniquet = on;
}

// ---- Readouts ----------------------------------------------------------------------------------------

double Bodies::grams(const Body& b, ResourceId r) const {
  return static_cast<double>(world_ledger_->balance(b.owner, r)) / 1000.0;
}

double Bodies::mass_kg(const Body& b) const {
  double g = 0;
  for (ResourceId r : {L_.fat_mg, L_.muscle_mg, L_.structure_mg, L_.glycogen_mg, L_.water_ul, L_.blood_ul})
    g += grams(b, r);
  return g / 1000.0;
}

double Bodies::fat_fraction(const Body& b) const { return grams(b, L_.fat_mg) / 1000.0 / mass_kg(b); }

double Bodies::blood_loss_fraction(const Body& b) const {
  const double blood = static_cast<double>(world_ledger_->balance(b.owner, L_.blood_ul));
  return clamp01(1.0 - blood / static_cast<double>(b.blood_normal_ul));
}

namespace {
// Normal water pool now: glycogen holds bound water, so an empty store means a lighter normal.
double water_target_ul(const Body& b, double glycogen_mg) {
  return static_cast<double>(b.water_normal_ul) -
         kBoundWaterPerGlycogen * (static_cast<double>(b.glycogen_max_mg) - glycogen_mg);
}
}  // namespace

double Bodies::water_deficit_fraction(const Body& b) const {
  const double glycogen = static_cast<double>(world_ledger_->balance(b.owner, L_.glycogen_mg));
  const double water = static_cast<double>(world_ledger_->balance(b.owner, L_.water_ul));
  return std::max(0.0, water_target_ul(b, glycogen) - water) / static_cast<double>(b.water_normal_ul);
}

double Bodies::core_temp_c(const Body& b) const {
  double fever = 0;  // derived from the wounds, never stored
  for (const Wound& w : b.wounds) {
    if (!w.infection_visible) continue;
    if (w.infection == Infection::Spreading) fever = std::max(fever, 1.0);
    if (w.infection == Infection::Systemic) fever = std::max(fever, 2.0);
  }
  return 37 + fever;
}

double Bodies::wound_bleed_ml_per_min(const Body& b, const Wound& wound) const { return wound_bleed(b, wound); }

double Bodies::bleed_ml_per_min(const Body& b) const {
  double total = 0;
  for (const Wound& w : b.wounds) total += wound_bleed(b, w);
  return total;
}

double Bodies::region_pain(const Body& b, Region r, std::int64_t now) const {
  double keep = 1;
  for (const Wound& w : b.wounds)
    if (w.region == r) keep *= 1 - wound_pain(b, w, now);
  return 1 - keep;
}

double Bodies::pain(const Body& b, std::int64_t now) const {
  double keep = 1;
  for (const Wound& w : b.wounds) keep *= 1 - wound_pain(b, w, now);
  return 1 - keep;
}

Capabilities Bodies::capabilities(const Body& b, std::int64_t now) const {
  Capabilities c;
  if (!b.alive) return Capabilities{0, 0, 0, 0, 0, 0};
  const double loss = blood_loss_fraction(b);
  // Hemorrhage classes (ATLS): I < 15 %, II 15-30 %, III 30-40 %, IV > 40 %.
  const double blood_f = loss < 0.15   ? 1.0
                         : loss < 0.30 ? lerp_band(loss, 0.15, 1.0, 0.30, 0.85)
                         : loss < 0.40 ? lerp_band(loss, 0.30, 0.85, 0.40, 0.5)
                                       : lerp_band(loss, 0.40, 0.5, kDeathBloodLoss, 0.1);
  const double sleep_f = b.spec.walker ? 1.0 : lerp_band(b.sleep_pressure, 0.6, 1.0, 1.0, 0.4);
  const double d = water_deficit_fraction(b);
  const double water_f = d < 0.20   ? lerp_band(d, 0.08, 1.0, 0.20, 0.9)
                         : d < 0.40 ? lerp_band(d, 0.20, 0.9, 0.40, 0.4)
                                    : lerp_band(d, 0.40, 0.4, kDeathWaterDeficit, 0.1);
  const double fever_f = clamp01(1 - 0.15 * std::max(0.0, core_temp_c(b) - 38));
  const double total_pain = pain(b, now);
  double head_f = 1;
  for (const Wound& w : b.wounds)
    if (w.region == Region::Head && (w.depth == Depth::Deep || w.type == WoundType::Fracture)) head_f *= 0.7;
  c.consciousness = clamp01(blood_f * sleep_f * water_f * fever_f * (1 - 0.3 * total_pain) * head_f);
  c.sight = head_f;
  c.hearing = head_f;

  const double left = limb_factor(b, Region::ThighL) * limb_factor(b, Region::ShinL) * limb_factor(b, Region::FootL);
  const double right = limb_factor(b, Region::ThighR) * limb_factor(b, Region::ShinR) * limb_factor(b, Region::FootR);
  double legs_pain_keep = 1;
  for (Region r : {Region::Pelvis, Region::ThighL, Region::ThighR, Region::ShinL, Region::ShinR, Region::FootL,
                   Region::FootR})
    legs_pain_keep *= 1 - region_pain(b, r, now);
  const double awake = c.consciousness < 0.3 ? 0.0 : std::min(1.0, c.consciousness / 0.6);
  c.moving = clamp01(0.5 * (left + right) * limb_factor(b, Region::Pelvis) * (1 - 0.4 * (1 - legs_pain_keep)) *
                     blood_f * (0.5 + 0.5 * b.exertion) * awake);
  auto arm = [&](Region u, Region f, Region h) {
    const double pain_keep = (1 - region_pain(b, u, now)) * (1 - region_pain(b, f, now)) * (1 - region_pain(b, h, now));
    return clamp01(limb_factor(b, u) * limb_factor(b, f) * limb_factor(b, h) * (1 - 0.5 * (1 - pain_keep)) * awake);
  };
  c.manipulation_l = arm(Region::UpperArmL, Region::ForearmL, Region::HandL);
  c.manipulation_r = arm(Region::UpperArmR, Region::ForearmR, Region::HandR);
  return c;
}

Needs Bodies::needs(const Body& b) const {
  Needs n;
  if (b.spec.walker || !b.alive) return n;
  const double glycogen = static_cast<double>(world_ledger_->balance(b.owner, L_.glycogen_mg));
  const double stomach = static_cast<double>(world_ledger_->balance(b.owner, L_.food_j));
  n.hunger = clamp01(0.9 - 0.9 * glycogen / static_cast<double>(b.glycogen_max_mg) - stomach / 3e6);
  n.thirst = clamp01(water_deficit_fraction(b) / 0.2);
  n.tiredness = clamp01((b.sleep_pressure - 0.3) / 0.6);
  n.exhaustion = clamp01(1 - b.exertion);
  return n;
}

double Bodies::carry_limit_kg(const Body& b, std::int64_t now) const {
  return 0.9 * grams(b, L_.muscle_mg) / 1000.0 * capabilities(b, now).moving;
}

namespace {
double load_factor(double carried, double limit) {
  if (carried <= 0) return 1;
  if (limit <= 0) return 0;
  const double r = carried / limit;
  return r <= 1 ? 1 - 0.5 * r * r : std::max(0.15, 0.5 - 0.6 * (r - 1));
}
double age_factor(double age) { return age > 50 ? std::max(0.5, 1 - 0.005 * (age - 50)) : 1.0; }
}  // namespace

double Bodies::walk_speed(const Body& b, std::int64_t now) const {
  const double limit = carry_limit_kg(b, now);
  return 1.35 * dm::sqrt(b.spec.height_m / 1.75) * age_factor(b.spec.age) * capabilities(b, now).moving *
         load_factor(b.activity.carried_kg, limit);
}

double Bodies::run_speed(const Body& b, std::int64_t now) const {
  const double limit = carry_limit_kg(b, now);
  const double fat_f = std::max(0.3, 1 - 1.2 * std::max(0.0, fat_fraction(b) - 0.15));
  return std::max(0.0, 2.5 + 0.06 * (b.vo2max - 35)) * dm::sqrt(b.spec.height_m / 1.75) * age_factor(b.spec.age) *
         fat_f * capabilities(b, now).moving * load_factor(b.activity.carried_kg, limit);
}

double Bodies::work_rate(const Body& b, std::int64_t now) const {
  const Capabilities c = capabilities(b, now);
  return 0.5 * (c.manipulation_l + c.manipulation_r) * c.consciousness * (0.5 + 0.5 * b.exertion);
}

double Bodies::push_force_n(const Body& b, std::int64_t now) const {
  const double posture = b.posture == Posture::Stand ? 1.0 : b.posture == Posture::Crouch ? 1.1
                         : b.posture == Posture::Sit ? 0.5 : 0.2;
  const Capabilities c = capabilities(b, now);
  return (4 * mass_kg(b) + 6 * grams(b, L_.muscle_mg) / 1000.0) * posture * c.consciousness *
         1.0;
}

double Bodies::footstep_db(const Body& b, double speed_mps, double floor_db) const {
  if (speed_mps <= 0) return 0;
  double db = 40 + 10 * dm::log10(mass_kg(b) / 70) + 20 * dm::log10(std::max(speed_mps, 0.3) / 1.35) + floor_db;
  if (b.posture == Posture::Crouch) db -= 6;
  return std::max(0.0, db);
}

double Bodies::expenditure_w(const Body& b) const {
  const double m = mass_kg(b);
  const BodySpec& s = b.spec;
  const double bmr_kcal = 10 * m + 625 * s.height_m - 5 * s.age + (s.sex == Sex::Male ? 5 : -161);  // Mifflin-St Jeor
  const double bmr_w = bmr_kcal * 4184.0 / 86400.0;
  const Activity& a = b.activity;
  double p;
  if (a.asleep) {
    p = 0.95 * bmr_w;
  } else if (a.speed_mps > 0) {
    const double W = m, L = a.carried_kg, V = a.speed_mps, G = a.grade_pct;  // Pandolf et al. (1977), eta = 1
    p = 1.5 * W + 2.0 * (W + L) * (L / W) * (L / W) + (W + L) * (1.5 * V * V + 0.35 * V * G);
    // Above ~2 m/s people switch to the cheaper gait: running at ~4 J/kg/m net (authored).
    if (V >= 2.0) p = std::min(p, 1.5 * W + 4.0 * (W + L) * V + (W + L) * 0.35 * V * G);
  } else {
    p = bmr_w * std::max(1.0, a.met);
  }
  return std::max(p, bmr_w * 0.95) + bmr_w * 0.13 * std::max(0.0, core_temp_c(b) - 37);  // fever cost
}

std::vector<std::string> Bodies::trait_labels(const Body& b) const {
  std::vector<std::string> out;
  const bool male = b.spec.sex == Sex::Male;
  const double fat = fat_fraction(b), muscle = grams(b, L_.muscle_mg) / 1000.0 / mass_kg(b);
  if (fat > (male ? 0.25 : 0.32)) out.emplace_back("overweight");
  if (fat < (male ? 0.12 : 0.20)) out.emplace_back("lean");
  if (muscle > (male ? 0.45 : 0.38)) out.emplace_back("strong");
  if (b.vo2max < 30) out.emplace_back("unfit");
  return out;
}

// ---- Update ------------------------------------------------------------------------------------------

std::int64_t Bodies::flow(double& carry, double amount) {
  const double t = std::max(0.0, amount) + carry;
  const auto n = static_cast<std::int64_t>(t);
  carry = t - static_cast<double>(n);
  return n;
}

void Bodies::update(World& w) {
  const bool slow = w.clock.is_body_step();
  for (auto& [id, b] : bodies_) {
    if (!b.alive || b.away) continue;
    fine_tick(w, b);
    if (slow && b.alive) body_step(w, b);
  }
}

void Bodies::fine_tick(World& w, Body& b) {
  const double bleed = bleed_ml_per_min(b);
  if (bleed <= 0) return;
  const std::int64_t ul = flow(b.carry[kBleed], bleed * 1000.0 * kTickSeconds / 60.0);
  if (ul > 0) w.ledger.transfer(w.clock.tick, b.owner, L_.blood_lost, L_.blood_ul, ul, 0);
  if (!b.spec.walker && blood_loss_fraction(b) >= kDeathBloodLoss) {
    // The chronicle's cause: the wound that bleeds most.
    const Wound* worst = nullptr;
    for (const Wound& wd : b.wounds)
      if (!worst || wound_bleed(b, wd) > wound_bleed(b, *worst)) worst = &wd;
    die(w, b, DeathCause::Exsanguination, worst ? worst->cause : 0);
  }
}

void Bodies::body_step(World& w, Body& b) {
  const double dt = static_cast<double>(kBodyStepTicks) * kTickSeconds;
  metabolism(w, b, dt);
  water(w, b, dt);
  sleep_and_exertion(w, b, dt);
  wounds_step(w, b, dt);
  if (!b.alive) return;
  const Needs n = needs(b);
  const double p = pain(b, w.clock.tick);
  // Fear follows pain and what the senses report: it jumps up within seconds and settles over
  // half an hour (spec 11; the threat comes from the walkers system).
  const double fear_target = std::max(0.6 * p, b.threat);
  b.fear += (fear_target - b.fear) * (1 - dm::exp(-dt / (fear_target > b.fear ? 20.0 : 1800.0)));
  const double morale_target = clamp01(0.75 - 0.25 * n.hunger - 0.25 * n.tiredness - 0.2 * p);
  b.morale += (morale_target - b.morale) * (1 - dm::exp(-dt / (8 * 3600.0)));
  if (w.clock.tick > 0 && w.clock.tick % kTicksPerGameDay == 0) daily_drift(w, b);
  check_death(w, b, 0);
}

void Bodies::metabolism(World& w, Body& b, double dt) {
  auto& l = w.ledger;
  const std::int64_t t = w.clock.tick;
  // Energy in: the stomach empties into glycogen until full; the rest is stored as fat.
  const double stomach = static_cast<double>(l.balance(b.owner, L_.food_j));
  if (stomach > 0) {
    double absorbed = stomach * (1 - dm::exp(-dt / kStomachTauS));
    if (stomach < 20000) absorbed = stomach;  // the last bit empties at once
    const double room_mg = static_cast<double>(b.glycogen_max_mg - l.balance(b.owner, L_.glycogen_mg));
    const double to_glycogen = std::min(absorbed, std::max(0.0, room_mg) * kGlycogenJPerMg);
    l.convert(t, b.owner, b.owner, L_.food_to_glycogen, flow(b.carry[kFoodToGlycogen], to_glycogen / kGlycogenJPerMg), 0);
    l.convert(t, b.owner, b.owner, L_.food_to_fat, flow(b.carry[kFoodToFat], (absorbed - to_glycogen) / kFatJPer2mg), 0);
  }
  // Energy out: glycogen first, then fat and muscle by the Forbes partition.
  const double power = expenditure_w(b);
  double need_j = power * dt;
  auto burn = [&](ConversionId c, int slot, double j, double j_per_batch) {
    const double carry_before = b.carry[slot];
    const std::int64_t want = flow(b.carry[slot], j / j_per_batch);
    const std::int64_t done = l.convert(t, b.owner, L_.metabolised, c, want, 0);
    if (done < want) {  // the store ran out: pass on exactly what was not burnt
      b.carry[slot] = 0;
      return std::max(0.0, j + (carry_before - static_cast<double>(done)) * j_per_batch);
    }
    return 0.0;
  };
  need_j = burn(L_.glycogen_burn, kGlycogenBurn, need_j, kGlycogenJPerMg);
  if (need_j > 0) {
    const double fat_kg = grams(b, L_.fat_mg) / 1000.0;
    const double lean_share = kForbesC / (kForbesC + fat_kg);  // share of mass lost that is lean
    const double kj_per_g = lean_share * 7.6 + (1 - lean_share) * 39.5;
    const double mass_g = need_j / 1000.0 / kj_per_g;
    double rest = burn(L_.fat_burn, kFatBurn, (1 - lean_share) * mass_g * 39.5 * 1000.0, static_cast<double>(kFatJPer2mg));
    rest = burn(L_.muscle_burn, kMuscleBurn, lean_share * mass_g * 7.6 * 1000.0 + rest, kLeanJPer5mg);
    (void)rest;  // nothing left to burn: starvation is checked on muscle mass
  }
  // Activity counters for the daily drift.
  const double met = power / (kMetW * mass_kg(b));
  if (met >= 2.5) b.active_minutes_today += dt / 60.0;
  if (met >= 5 || (b.activity.carried_kg >= 15 && b.activity.speed_mps > 0)) b.heavy_minutes_today += dt / 60.0;
}

void Bodies::water(World& w, Body& b, double dt) {
  auto& l = w.ledger;
  const std::int64_t t = w.clock.tick;
  const double met = expenditure_w(b) / (kMetW * mass_kg(b));
  double loss_l = kBaselineWaterLossLPerDay * dt / 86400.0;
  if (met > 1.5 && !b.activity.asleep) loss_l += kSweatLPerHPerMet * (met - 1.5) * dt / 3600.0;
  double loss_ul = loss_l * 1e6;
  const double water = static_cast<double>(l.balance(b.owner, L_.water_ul));
  const double target = water_target_ul(b, static_cast<double>(l.balance(b.owner, L_.glycogen_mg)));
  if (water > target) loss_ul += (water - target) * (1 - dm::exp(-dt / kKidneyTauS));  // kidneys
  l.transfer(t, b.owner, L_.excreted, L_.water_ul, flow(b.carry[kWaterLoss], loss_ul), 0);
  // Plasma refill from body water while not badly dehydrated.
  const std::int64_t blood = l.balance(b.owner, L_.blood_ul);
  if (blood < b.blood_normal_ul && water_deficit_fraction(b) < kCriticalWaterDeficit) {
    const double refill = std::min(static_cast<double>(b.blood_normal_ul - blood), kBloodRefillLPerDay * 1e6 * dt / 86400.0);
    l.convert(t, b.owner, b.owner, L_.water_to_blood, flow(b.carry[kRefill], refill), 0);
  }
}

void Bodies::sleep_and_exertion(World& w, Body& b, double dt) {
  if (!b.spec.walker) {
    if (b.activity.asleep) {
      b.sleep_pressure *= dm::exp(-dt / (kSleepFallTauH * 3600));
      if (b.sleep_pressure <= kWakeAtS) {
        b.activity.asleep = false;
        w.emit(EventType::WokeUp, b.id, 0);
      }
    } else {
      b.sleep_pressure = 1 - (1 - b.sleep_pressure) * dm::exp(-dt / (kSleepRiseTauH * 3600));
      if (b.sleep_pressure >= kInvoluntarySleepAtS) {
        b.activity = Activity{};
        b.activity.asleep = true;
        b.posture = Posture::Lie;
        w.emit(EventType::FellAsleep, b.id, 0, 1);  // a = 1: involuntary
      }
    }
  }
  // Exertion reserve: drains above half of VO2max, recovers below it; "the wall" caps it by glycogen.
  const double met = expenditure_w(b) / (kMetW * mass_kg(b));
  const double max_met = b.vo2max / 3.5, sustainable = 0.5 * max_met;
  const double glycogen = static_cast<double>(w.ledger.balance(b.owner, L_.glycogen_mg));
  const double ceiling = std::clamp(glycogen / (0.45 * static_cast<double>(b.glycogen_max_mg)), 0.3, 1.0);
  if (met > sustainable)
    b.exertion -= dt / 60 * kExertionDrainPerMinAtMax * (met - sustainable) / (max_met - sustainable);
  else
    b.exertion += dt / 60 * kExertionRecoverPerMin;
  b.exertion = std::clamp(b.exertion, 0.0, ceiling);
}

void Bodies::wounds_step(World& w, Body& b, double dt) {
  const std::int64_t now = w.clock.tick;
  for (Wound& wd : b.wounds) {
    // Clotting: grows toward its cap at rest, breaks down while the region moves.
    const RegionInfo& ri = kRegions[static_cast<int>(wd.region)];
    // Only a deep wound in a high-vessel region keeps bleeding without a dressing; longer ones more.
    const double cap = (wd.depth == Depth::Deep && ri.vessel_weight >= 1.4 && !wd.dressed)
                           ? 1 - 0.4 * std::min(1.0, wd.length_mm / 80.0) : 1.0;
    if (region_moving(b, wd.region)) wd.clot *= 0.8;
    else wd.clot += (cap - wd.clot) * (1 - dm::exp(-dt / 60.0 / kClotTauMin[static_cast<int>(wd.depth)]));
    if (wd.clot > cap - 1e-3) wd.clot = cap;  // settled: a fully clotted wound stops bleeding exactly

    // Bacterial infection (spec 12 rules 3-4). Each wound draws on its own stream.
    if (wd.infection == Infection::None && open_wound(wd)) {
      const double per_day = infection_base_per_day(wd.type) * wd.contamination * clean_factor(wd.cleaned) *
                             (wd.dressed ? 0.7 : 1.0);
      if (per_day > 0 && hazard_fires(w.rng, wd.id, per_day / 86400.0, dt)) {
        wd.infection = Infection::Local;
        wd.infected_tick = now;
      }
    } else if (wd.infection == Infection::Local || wd.infection == Infection::Spreading ||
               wd.infection == Infection::Systemic) {
      if (!wd.infection_visible) {
        if (now - wd.infected_tick >= ticks(kInfectionVisibleH * 3600)) {
          wd.infection_visible = true;
          w.emit(EventType::InfectionVisible, b.id, wd.cause, static_cast<std::int64_t>(wd.id));
        }
      } else {
        // Exit set per stage: {resolve, get worse}, per day (authored, tune).
        const double day = 86400.0;
        // Most local infections resolve; cleaning and a dressing still slow a local one (spec 12 rule 4).
        const double rates_by_stage[3][2] = {{0.4, 0.2}, {0.2, 0.25}, {0.05, 0.5}};
        const auto* r = rates_by_stage[static_cast<int>(wd.infection) - 1];
        double worse = r[1];
        if (wd.infection == Infection::Local)
          worse *= (wd.cleaned == Cleaned::No ? 1.0 : 0.5) * (wd.dressed ? 0.7 : 1.0);
        const double rates[2] = {r[0] / day, worse / day};
        if (auto hit = resolve_exit_set(w.rng, wd.id, rates, dt)) {
          if (*hit == 0) {
            wd.infection = Infection::Resolved;
          } else if (wd.infection == Infection::Systemic) {
            const EventId e = w.emit(EventType::InfectionWorse, b.id, wd.cause, static_cast<std::int64_t>(wd.id), 4);
            die(w, b, DeathCause::Sepsis, e);
            return;
          } else {
            wd.infection = static_cast<Infection>(static_cast<int>(wd.infection) + 1);
            w.emit(EventType::InfectionWorse, b.id, wd.cause, static_cast<std::int64_t>(wd.id),
                   static_cast<std::int64_t>(wd.infection));
          }
        }
      }
      if (wd.infection != Infection::Resolved) wd.heal_tick += kBodyStepTicks;  // no healing while infected
    }
  }
  // Healing: closed and gone.
  for (std::size_t i = 0; i < b.wounds.size();) {
    if (now >= b.wounds[i].heal_tick) {
      w.emit(EventType::WoundHealed, b.id, b.wounds[i].cause, static_cast<std::int64_t>(b.wounds[i].id));
      b.wounds.erase(b.wounds.begin() + static_cast<std::ptrdiff_t>(i));
    } else {
      ++i;
    }
  }
}

void Bodies::daily_drift(World& w, Body& b) {
  auto& l = w.ledger;
  const std::int64_t t = w.clock.tick;
  if (b.active_minutes_today >= 60) b.vo2max += 0.05;
  if (b.heavy_minutes_today >= 30) b.vo2max += 0.05;
  if (b.active_minutes_today < 20) b.vo2max -= 0.03;
  b.vo2max = std::clamp(b.vo2max, 15.0, 75.0);
  const std::int64_t muscle = l.balance(b.owner, L_.muscle_mg);
  if (b.heavy_minutes_today >= 30) {
    const std::int64_t gain = l.convert(t, b.owner, b.owner, L_.glycogen_to_muscle, muscle / 2000, 0);  // +0.1 %/day
    l.convert(t, b.owner, b.owner, L_.water_to_muscle, gain, 0);
  } else if (b.active_minutes_today < 20) {
    l.convert(t, b.owner, L_.metabolised, L_.muscle_burn, muscle / 2000 / 5, 0);  // disuse -0.05 %/day
  }
  b.active_minutes_today = 0;
  b.heavy_minutes_today = 0;
}

void Bodies::check_death(World& w, Body& b, EventId cause) {
  if (b.spec.walker || !b.alive) return;  // walkers die by damage (stage 7)
  if (blood_loss_fraction(b) >= kDeathBloodLoss) die(w, b, DeathCause::Exsanguination, cause);
  else if (water_deficit_fraction(b) >= kDeathWaterDeficit) die(w, b, DeathCause::Dehydration, cause);
  else if (static_cast<double>(w.ledger.balance(b.owner, L_.muscle_mg)) <
               kStarvationMuscleLeft * static_cast<double>(b.muscle_start_mg) &&
           fat_fraction(b) < 0.03)  // starvation is lean loss with the fat store gone
    die(w, b, DeathCause::Starvation, cause);
}

void Bodies::die(World& w, Body& b, DeathCause c, EventId cause) {
  if (!b.alive) return;
  b.alive = false;
  b.death_cause = c;
  b.activity = Activity{};
  b.posture = Posture::Lie;
  w.emit(EventType::Death, b.id, cause, static_cast<std::int64_t>(c), b.spec.walker ? 1 : 0);
}

void Bodies::kill(World& w, EntityId body, DeathCause c, EventId cause) { die(w, bodies_.at(body), c, cause); }

void Bodies::reanimate(World& w, EntityId body, EventId cause) {
  Body& b = bodies_.at(body);
  if (b.alive || b.spec.walker || b.death_cause == DeathCause::BrainDestroyed) return;
  b.alive = true;
  b.spec.walker = true;
  b.sleep_pressure = 0;
  b.activity = Activity{};
  b.posture = Posture::Stand;
  b.fear = 0;
  b.threat = 0;
  w.emit(EventType::Reanimated, b.id, cause);
}

// ---- Persistence -------------------------------------------------------------------------------------

void Bodies::save(ArchiveWriter& a) const {
  a.u64(bodies_.size());
  for (const auto& [id, b] : bodies_) {
    a.u64(id);
    a.boolean(b.alive);
    a.boolean(b.spec.walker);
    a.u8(static_cast<std::uint8_t>(b.death_cause));
    a.u8(static_cast<std::uint8_t>(b.posture));
    a.f64(b.activity.met);
    a.f64(b.activity.speed_mps);
    a.f64(b.activity.grade_pct);
    a.f64(b.activity.carried_kg);
    a.boolean(b.activity.asleep);
    a.boolean(b.away);
    for (double v : {b.sleep_pressure, b.exertion, b.vo2max, b.fear, b.morale, b.threat,
                     b.active_minutes_today, b.heavy_minutes_today})
      a.f64(v);
    for (double c : b.carry) a.f64(c);
    a.u64(b.wounds.size());
    for (const Wound& wd : b.wounds) save_wound(a, wd);
  }
  // The pool (spec 29): what makes a slot's occupant, written only when a world reserved a pool, so a world
  // without one saves the same bytes as before.
  for (EntityId id : slots_) {
    const Body& b = bodies_.at(id);
    a.boolean(b.vacant);
    a.u64(b.pop);
    a.str(b.label);
    a.u8(static_cast<std::uint8_t>(b.spec.sex));
    for (double v : {b.spec.age, b.spec.height_m, b.spec.mass_kg, b.spec.fat_fraction, b.spec.vo2max}) a.f64(v);
    for (std::int64_t v : {b.blood_normal_ul, b.water_normal_ul, b.muscle_start_mg, b.glycogen_max_mg}) a.i64(v);
  }
}

void Bodies::load(ArchiveReader& a) {
  // Bodies are created by setup; the save holds what changed since.
  if (a.u64() != bodies_.size()) throw std::runtime_error("save: body list differs");
  for (auto& [id, b] : bodies_) {
    if (a.u64() != id) throw std::runtime_error("save: body ids differ");
    b.alive = a.boolean();
    b.spec.walker = a.boolean();
    b.death_cause = static_cast<DeathCause>(a.u8());
    b.posture = static_cast<Posture>(a.u8());
    b.activity.met = a.f64();
    b.activity.speed_mps = a.f64();
    b.activity.grade_pct = a.f64();
    b.activity.carried_kg = a.f64();
    b.activity.asleep = a.boolean();
    b.away = a.boolean();
    for (double* v : {&b.sleep_pressure, &b.exertion, &b.vo2max, &b.fear, &b.morale, &b.threat,
                      &b.active_minutes_today, &b.heavy_minutes_today})
      *v = a.f64();
    for (double& c : b.carry) c = a.f64();
    b.wounds.clear();
    for (std::uint64_t n = a.u64(); n > 0; --n) b.wounds.push_back(load_wound(a));
  }
  for (EntityId id : slots_) {
    Body& b = bodies_.at(id);
    b.vacant = a.boolean();
    b.pop = a.u64();
    b.label = a.str();
    b.spec.sex = static_cast<Sex>(a.u8());
    for (double* v : {&b.spec.age, &b.spec.height_m, &b.spec.mass_kg, &b.spec.fat_fraction, &b.spec.vo2max}) *v = a.f64();
    for (std::int64_t* v : {&b.blood_normal_ul, &b.water_normal_ul, &b.muscle_start_mg, &b.glycogen_max_mg}) *v = a.i64();
  }
}

void write_body_state(ArchiveWriter& a, const BodyState& s) {
  a.u8(static_cast<std::uint8_t>(s.spec.sex));
  for (double v : {s.spec.age, s.spec.height_m, s.spec.mass_kg, s.spec.fat_fraction, s.spec.vo2max}) a.f64(v);
  a.boolean(s.spec.walker);
  a.str(s.label);
  for (std::int64_t v : {s.blood_normal_ul, s.water_normal_ul, s.muscle_start_mg, s.glycogen_max_mg}) a.i64(v);
  a.u8(static_cast<std::uint8_t>(s.posture));
  a.f64(s.activity.met);
  a.f64(s.activity.speed_mps);
  a.f64(s.activity.grade_pct);
  a.f64(s.activity.carried_kg);
  a.boolean(s.activity.asleep);
  for (double v : {s.sleep_pressure, s.exertion, s.vo2max, s.fear, s.morale, s.threat, s.active_minutes_today, s.heavy_minutes_today})
    a.f64(v);
  for (double c : s.carry) a.f64(c);
  a.u64(s.wounds.size());
  for (const Wound& wd : s.wounds) save_wound(a, wd);
  for (std::int64_t v : {s.fat_mg, s.muscle_mg, s.structure_mg, s.glycogen_mg, s.water_ul, s.blood_ul, s.food_j}) a.i64(v);
}

BodyState read_body_state(ArchiveReader& a) {
  BodyState s;
  s.spec.sex = static_cast<Sex>(a.u8());
  for (double* v : {&s.spec.age, &s.spec.height_m, &s.spec.mass_kg, &s.spec.fat_fraction, &s.spec.vo2max}) *v = a.f64();
  s.spec.walker = a.boolean();
  s.label = a.str();
  for (std::int64_t* v : {&s.blood_normal_ul, &s.water_normal_ul, &s.muscle_start_mg, &s.glycogen_max_mg}) *v = a.i64();
  s.posture = static_cast<Posture>(a.u8());
  s.activity.met = a.f64();
  s.activity.speed_mps = a.f64();
  s.activity.grade_pct = a.f64();
  s.activity.carried_kg = a.f64();
  s.activity.asleep = a.boolean();
  for (double* v : {&s.sleep_pressure, &s.exertion, &s.vo2max, &s.fear, &s.morale, &s.threat, &s.active_minutes_today, &s.heavy_minutes_today})
    *v = a.f64();
  for (double& c : s.carry) c = a.f64();
  for (std::uint64_t n = a.u64(); n > 0; --n) s.wounds.push_back(load_wound(a));
  for (std::int64_t* v : {&s.fat_mg, &s.muscle_mg, &s.structure_mg, &s.glycogen_mg, &s.water_ul, &s.blood_ul, &s.food_j}) *v = a.i64();
  return s;
}

}  // namespace zs
