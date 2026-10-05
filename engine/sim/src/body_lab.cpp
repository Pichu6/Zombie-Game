// Body lab scenarios. Spec bands: docs/tier0/specs/11-body-composition.md, 12-body-wounds.md, 13-capabilities.md
#include "zs/body_lab.h"

#include <algorithm>
#include <stdexcept>

namespace zs::body_lab {
namespace {

constexpr std::int64_t kHour = kTicksPerGameHour;
constexpr std::int64_t kMinute = kTicksPerGameHour / 60;
constexpr double kKcal = 4184.0;

BodySpec adult(double mass = 75, double fat = 0.18) {
  BodySpec s;
  s.mass_kg = mass;
  s.fat_fraction = fat;
  return s;
}

std::int64_t hour_of(const World& w) { return (w.clock.tick / kHour) % 24; }

Result make(std::string name, std::string band, double value, bool pass, std::string unit) {
  return Result{std::move(name), std::move(band), value, pass, std::move(unit)};
}

Wound wound(Region r, WoundType t, Depth d, double length_mm, double contamination) {
  Wound w;
  w.region = r;
  w.type = t;
  w.depth = d;
  w.length_mm = length_mm;
  w.contamination = contamination;
  return w;
}

}  // namespace

void advance(Lab& lab, std::int64_t n) {
  lab.world->run_ticks(n);
  if (auto v = lab.world->ledger.check()) throw std::logic_error("ledger: " + *v);
}

Lab make_lab(std::uint64_t seed, const std::vector<BodySpec>& specs) {
  Lab lab;
  lab.world = std::make_unique<World>(seed);
  auto bodies = std::make_unique<Bodies>(*lab.world);
  lab.bodies = bodies.get();
  lab.world->add_system(std::move(bodies));
  auto& l = lab.world->ledger;
  lab.pantry = l.add_owner("pantry");
  lab.tap = l.add_owner("tap");
  l.mint(lab.pantry, lab.bodies->ids().food_j, std::int64_t{1} << 50);
  l.mint(lab.tap, lab.bodies->ids().water_ul, std::int64_t{1} << 50);
  for (std::size_t i = 0; i < specs.size(); ++i) lab.bodies->create(*lab.world, specs[i], "body" + std::to_string(i));
  lab.world->close_setup();
  // The per-tick ledger check is too slow for week-long, 100-body scenarios; advance() checks each call.
  lab.world->check_every_tick = false;
  return lab;
}

void daily_routine(Lab& lab, EntityId id, const Activity& awake) {
  Body& b = lab.bodies->get(id);
  if (!b.alive) return;
  const std::int64_t h = hour_of(*lab.world);
  if (h >= 23 || h < 7) {
    if (!b.activity.asleep) {
      b.activity = Activity{};
      b.activity.asleep = true;
      b.posture = Posture::Lie;
    }
  } else {
    b.activity = awake;
    b.posture = Posture::Stand;
  }
}

void keep_supplied(Lab& lab, EntityId id, bool feed, bool water) {
  Bodies& bodies = *lab.bodies;
  const Body& b = bodies.get(id);
  if (!b.alive || b.activity.asleep) return;
  if (water) {
    const double deficit_ul = bodies.water_deficit_fraction(b) * bodies.mass_kg(b) * 1e6;
    if (deficit_ul > 0) bodies.drink(*lab.world, id, lab.tap, static_cast<std::int64_t>(deficit_ul) + 1, 0);
  }
  if (feed && bodies.needs(b).hunger >= 0.3) bodies.eat(*lab.world, id, lab.pantry, 2'500'000, 0);
}

Result total_fast_14d(std::uint64_t seed) {
  Lab lab = make_lab(seed, {adult()});
  const EntityId id = lab.bodies->all().begin()->first;
  const double start = lab.bodies->mass_kg(lab.bodies->get(id));
  for (int h = 0; h < 14 * 24; ++h) {
    daily_routine(lab, id, Activity{});
    keep_supplied(lab, id, false, true);
    advance(lab, kHour);
  }
  const double lost = start - lab.bodies->mass_kg(lab.bodies->get(id));
  return make("total fast, 14 days, water given", "4-8 kg (Cahill 1970; Hall et al. 2011)", lost,
              lost >= 4 && lost <= 8, "kg lost");
}

Result semi_starvation_14d(std::uint64_t seed) {
  Lab lab = make_lab(seed, {adult()});
  const EntityId id = lab.bodies->all().begin()->first;
  const double start = lab.bodies->mass_kg(lab.bodies->get(id));
  Activity work;
  work.met = 1.6;
  for (int h = 0; h < 14 * 24; ++h) {
    const std::int64_t hour = hour_of(*lab.world);
    Activity a = work;
    if (hour == 10) a.speed_mps = 1.35;  // about 5 km of walking a day, as in the Minnesota study
    daily_routine(lab, id, a);
    keep_supplied(lab, id, false, true);
    if (hour == 8 || hour == 13 || hour == 19)
      lab.bodies->eat(*lab.world, id, lab.pantry, static_cast<std::int64_t>(1600 * kKcal / 3), 0);
    advance(lab, kHour);
  }
  const double lost = start - lab.bodies->mass_kg(lab.bodies->get(id));
  return make("semi-starvation 1600 kcal/day, light work, 14 days",
              "1.5-4.5 kg (Keys et al. 1950, early weeks)", lost, lost >= 1.5 && lost <= 4.5, "kg lost");
}

Result no_water_critical_day(std::uint64_t seed) {
  Lab lab = make_lab(seed, {adult()});
  const EntityId id = lab.bodies->all().begin()->first;
  Activity light;
  light.met = 2.0;
  double day = -1;
  for (int h = 0; h < 8 * 24 && day < 0; ++h) {
    daily_routine(lab, id, light);
    keep_supplied(lab, id, true, false);
    advance(lab, kHour);
    if (lab.bodies->water_deficit_fraction(lab.bodies->get(id)) >= 0.08)
      day = static_cast<double>(lab.world->clock.tick) / kTicksPerGameDay;
  }
  return make("no water, light work: first reaches 8 % deficit", "day 2-4 (Sawka et al. 2007; Adolph 1947)", day,
              day >= 2 && day <= 4, "days");
}

namespace {
// Returns hours to death (or -1 if alive after 24 h) and the worst blood loss seen.
std::pair<double, double> thigh_bleed(std::uint64_t seed, bool dressed) {
  Lab lab = make_lab(seed, {adult()});
  const EntityId id = lab.bodies->all().begin()->first;
  Wound w = wound(Region::ThighL, WoundType::Laceration, Depth::Deep, 80, 0.5);
  w.dressed = dressed;
  lab.bodies->add_wound(*lab.world, id, w, 0);
  Body& b = lab.bodies->get(id);
  b.activity = Activity{};
  b.activity.met = 1.0;
  b.posture = Posture::Lie;
  double worst = 0;
  for (std::int64_t t = 0; t < 24 * kHour; ++t) {
    lab.world->step();
    worst = std::max(worst, lab.bodies->blood_loss_fraction(b));
    if (!b.alive) break;
  }
  if (auto v = lab.world->ledger.check()) throw std::logic_error("ledger: " + *v);
  return {b.alive ? -1.0 : static_cast<double>(lab.world->clock.tick) / kHour, worst};
}
}  // namespace

Result thigh_bleed_untreated(std::uint64_t seed) {
  const auto [hours, worst] = thigh_bleed(seed, false);
  return make("deep 80 mm thigh laceration, untreated", "bleeds out in 1-8 h (ATLS class IV)", hours,
              hours >= 1 && hours <= 8, "hours to death");
}

Result thigh_bleed_dressed(std::uint64_t seed) {
  const auto [hours, worst] = thigh_bleed(seed, true);
  return make("same wound, dressed at once", "survives 24 h (worst loss < 45 %)", worst, hours < 0,
              "worst blood loss fraction");
}

Result infection_rate(Cleaned how, std::uint64_t seed) {
  const int n = 100;
  Lab lab = make_lab(seed, std::vector<BodySpec>(n, adult()));
  std::vector<EntityId> ids;
  for (const auto& [id, b] : lab.bodies->all()) ids.push_back(id);
  for (EntityId id : ids) {
    Wound w = wound(Region::ForearmR, WoundType::Laceration, Depth::Fat, 30, 1.0);
    w.cleaned = how;
    lab.bodies->add_wound(*lab.world, id, w, 0);
  }
  for (int h = 0; h < 7 * 24; ++h) {
    for (EntityId id : ids) {
      daily_routine(lab, id, Activity{});
      keep_supplied(lab, id, true, true);
    }
    advance(lab, kHour);
  }
  int infected = 0;
  for (EntityId id : ids)
    for (const Wound& w : lab.bodies->get(id).wounds)
      if (w.infected_tick >= 0) ++infected;
  const double rate = static_cast<double>(infected) / n;
  switch (how) {
    case Cleaned::No:
      return make("100 dirty lacerations, not cleaned, 7 days", "12-40 % (ED laceration reviews; tune)", rate,
                  rate >= 0.12 && rate <= 0.40, "share infected");
    case Cleaned::Water:
      return make("same, cleaned with clean water", "<= 18 %, below uncleaned", rate, rate <= 0.18, "share infected");
    case Cleaned::Disinfectant:
      return make("same, disinfected", "<= 10 %", rate, rate <= 0.10, "share infected");
  }
  return {};
}

Result awake_40h(std::uint64_t seed) {
  Lab lab = make_lab(seed, {adult()});
  const EntityId id = lab.bodies->all().begin()->first;
  for (int h = 0; h < 7; ++h) {  // a normal night first (midnight to 07:00)
    daily_routine(lab, id, Activity{});
    advance(lab, kHour);
  }
  Body& b = lab.bodies->get(id);
  b.activity = Activity{};
  for (int h = 0; h < 40; ++h) {
    keep_supplied(lab, id, true, true);
    advance(lab, kHour);
  }
  const double c = lab.bodies->capabilities(b, lab.world->clock.tick).consciousness;
  return make("40 h awake after a normal night", "consciousness <= 0.6 (Van Dongen et al. 2003)", c,
              c <= 0.6 && !b.activity.asleep, "consciousness");
}

Result heavy_haul(std::uint64_t seed) {
  // Same muscle mass and the same absolute VO2max: 60 kg lean and 110 kg with much more fat.
  BodySpec light = adult(60, 0.10);
  BodySpec heavy = adult(110, 1 - 54.0 / 110);
  heavy.vo2max = light.vo2max * 60 / 110;
  Lab lab = make_lab(seed, {light, heavy});
  std::vector<EntityId> ids;
  for (const auto& [id, b] : lab.bodies->all()) ids.push_back(id);
  Activity haul;
  haul.speed_mps = 0.7;
  haul.grade_pct = 15;
  haul.carried_kg = 20;
  double energy[2] = {0, 0};
  for (int k = 0; k < 2; ++k) lab.bodies->get(ids[k]).activity = haul;
  for (int m = 0; m < 10; ++m) {
    for (int k = 0; k < 2; ++k) energy[k] += lab.bodies->expenditure_w(lab.bodies->get(ids[k])) * 60;
    advance(lab, kMinute);
  }
  const double ratio = energy[1] / energy[0];
  const bool tires = lab.bodies->get(ids[1]).exertion < lab.bodies->get(ids[0]).exertion;
  return make("haul 20 kg up stairs for 10 min: 110 kg vs 60 kg", "heavier burns more (> 1) and tires sooner",
              ratio, ratio > 1 && tires, "energy ratio");
}

namespace {
double minutes_to_exhaustion(Lab& lab, EntityId id) {
  Body& b = lab.bodies->get(id);
  // Line up with a body step so each minute is one step.
  advance(lab, kBodyStepTicks - lab.world->clock.tick % kBodyStepTicks);
  b.activity = Activity{};
  b.activity.speed_mps = 3.0;
  int m = 0;
  while (b.exertion > 0.05 && m < 120) {
    advance(lab, kMinute);
    ++m;
  }
  b.activity = Activity{};
  return m;
}
}  // namespace

Result the_wall(std::uint64_t seed) {
  Lab fed = make_lab(seed, {adult()});
  const double fed_min = minutes_to_exhaustion(fed, fed.bodies->all().begin()->first);
  Lab fasted = make_lab(seed, {adult()});
  const EntityId id = fasted.bodies->all().begin()->first;
  for (int h = 0; h < 30; ++h) {
    daily_routine(fasted, id, Activity{});
    keep_supplied(fasted, id, false, true);
    advance(fasted, kHour);
  }
  fasted.bodies->get(id).activity.asleep = false;
  const double fasted_min = minutes_to_exhaustion(fasted, id);
  const double ratio = fasted_min / fed_min;
  return make("run at 3 m/s until exhausted: after 30 h without food vs fed",
              "ends sooner when glycogen is low (Bergstrom et al. 1967)", ratio, ratio < 0.7, "minutes ratio");
}

Result load_on_light_vs_strong(std::uint64_t seed) {
  Lab lab = make_lab(seed, {adult(60, 0.12), adult(85, 0.12)});
  std::vector<EntityId> ids;
  for (const auto& [id, b] : lab.bodies->all()) ids.push_back(id);
  double factor[2];
  for (int k = 0; k < 2; ++k) {
    Body& b = lab.bodies->get(ids[k]);
    const double free = lab.bodies->walk_speed(b, 0);
    b.activity.carried_kg = 30;
    factor[k] = lab.bodies->walk_speed(b, 0) / free;
    b.activity.carried_kg = 0;
  }
  return make("30 kg load: speed kept by a 60 kg body vs an 85 kg strong body", "light body slows more (< 1)",
              factor[0] / factor[1], factor[0] < factor[1], "ratio of speed factors");
}

Result fracture_vs_hand(std::uint64_t seed) {
  Lab lab = make_lab(seed, {adult(), adult(), adult()});
  std::vector<EntityId> ids;
  for (const auto& [id, b] : lab.bodies->all()) ids.push_back(id);
  Bodies& bodies = *lab.bodies;
  World& w = *lab.world;
  const double healthy = bodies.capabilities(bodies.get(ids[2]), 0).moving;
  const EntityId fx = bodies.add_wound(w, ids[0], wound(Region::ShinL, WoundType::Fracture, Depth::Deep, 0, 0), 0);
  bodies.add_wound(w, ids[1], wound(Region::HandR, WoundType::Laceration, Depth::Muscle, 30, 0.3), 0);
  const double fractured = bodies.capabilities(bodies.get(ids[0]), 0).moving;
  const double hand = bodies.capabilities(bodies.get(ids[1]), 0).moving;
  const double speed_unsplinted = bodies.walk_speed(bodies.get(ids[0]), 0);
  bodies.splint(ids[0], fx, true);
  const double speed_splinted = bodies.walk_speed(bodies.get(ids[0]), 0);
  return make("fractured shin vs a hand wound", "fracture drops moving (< 0.6), hand wound leaves it, splint helps",
              fractured, fractured < 0.6 && hand == healthy && speed_splinted > speed_unsplinted, "moving");
}

Result lean_outruns_heavy(std::uint64_t seed) {
  Lab lab = make_lab(seed, {adult(60, 0.10), adult(110, 1 - 54.0 / 110)});
  std::vector<EntityId> ids;
  for (const auto& [id, b] : lab.bodies->all()) ids.push_back(id);
  const double ratio = lab.bodies->run_speed(lab.bodies->get(ids[0]), 0) / lab.bodies->run_speed(lab.bodies->get(ids[1]), 0);
  return make("run speed: 60 kg lean vs 110 kg, same fitness", "lean is faster (> 1)", ratio, ratio > 1, "speed ratio");
}

std::vector<Result> run_all(std::uint64_t seed) {
  return {total_fast_14d(seed),       semi_starvation_14d(seed),  no_water_critical_day(seed),
          thigh_bleed_untreated(seed), thigh_bleed_dressed(seed), infection_rate(Cleaned::No, seed),
          infection_rate(Cleaned::Water, seed), infection_rate(Cleaned::Disinfectant, seed), awake_40h(seed),
          heavy_haul(seed),           the_wall(seed),             load_on_light_vs_strong(seed),
          fracture_vs_hand(seed),     lean_outruns_heavy(seed)};
}

}  // namespace zs::body_lab
