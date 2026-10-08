// Census of the block. Spec: docs/tier0/specs/22-census.md
#include <algorithm>
#include <stdexcept>

#include "toml.hpp"
#include "zs/dead.h"
#include "zs/detmath.h"
#include "zs/hazard.h"
#include "zs/pop_system.h"

namespace zs {
namespace {

// Default preset fates (estimates, tuned in playtests): people at home on the map are walkers in
// their houses half the time; off-map neighbours fled, turned or died elsewhere.
constexpr double kHomeWalker = 0.5;
constexpr double kOffMapWalker = 0.35, kOffMapDead = 0.15;  // the rest fled
// Arrivals (spec 22 rule 3): rate per hour = base + k x attention; attention gains from noise and
// light that reach past the map border (m beyond it / 1000 per hour) and halves every 4 h.
constexpr double kBaseArrivalsPerH = 0.05, kArrivalsPerAttention = 0.35, kAttentionHalfLifeH = 4;

// Roughly normal, mean 0, sd 1: the sum of four uniforms (Irwin-Hall), enough for body sizes.
double normal(Rng& rng, EntityId e) {
  double s = 0;
  for (int i = 0; i < 4; ++i) s += rng.uniform01(e, Purpose::CensusBody);
  return (s - 2.0) / detmath::sqrt(1.0 / 3.0);
}

// A body from a person: NHANES-like adult heights and BMI, Deurenberg body fat, VO2max by age.
BodySpec body_for(Rng& rng, EntityId key, const Person& p) {
  BodySpec s;
  s.sex = p.sex;
  s.age = p.age;
  const bool male = p.sex == Sex::Male;
  if (p.age >= 18) {
    s.height_m = (male ? 1.75 : 1.62) + (male ? 0.075 : 0.07) * normal(rng, key);
    const double bmi = std::clamp(27.5 * detmath::exp(0.18 * normal(rng, key)), 16.0, 50.0);
    s.mass_kg = bmi * s.height_m * s.height_m;
    s.fat_fraction = std::clamp((1.2 * bmi + 0.23 * p.age - 10.8 * (male ? 1 : 0) - 5.4) / 100, 0.05, 0.55);
    s.vo2max = std::clamp((male ? 48 : 40) - (male ? 0.3 : 0.27) * (p.age - 20) + 4 * normal(rng, key), 15.0, 65.0);
  } else {
    s.height_m = std::clamp(0.80 + 0.058 * p.age + 0.05 * normal(rng, key), 0.9, 1.85);
    const double bmi = std::clamp(15.5 + 0.25 * p.age + 2 * normal(rng, key), 13.0, 35.0);
    s.mass_kg = bmi * s.height_m * s.height_m;
    s.fat_fraction = std::clamp((1.51 * bmi - 0.70 * p.age - 3.6 * (male ? 1 : 0) + 1.4) / 100, 0.08, 0.45);
    s.vo2max = male ? 47 : 42;
  }
  s.walker = !p.survivor;  // survivor slots are living people (rule 5)
  return s;
}

}  // namespace

const char* to_string(Fate f) {
  switch (f) {
    case Fate::Fled: return "fled";
    case Fate::Dead: return "dead";
    case Fate::Survivor: return "survivor slot";
    case Fate::WalkerOffMap: return "walker off the map";
    case Fate::WalkerOnMap: return "walker on the map";
    case Fate::Destroyed: return "destroyed walker";
    case Fate::Pop: return "alive in the group";
  }
  return "?";
}

Census::Census(World& w, SiteMap& map, Assemblies& items, Bodies& bodies, Power& power)
    : map_(map), items_(items), bodies_(bodies), power_(power) {
  people_r_ = w.ledger.add_resource("people");
  for (std::size_t f = 0; f < kFateCount; ++f)
    owners_[f] = w.ledger.add_owner(std::string("census:") + to_string(static_cast<Fate>(f)));
}

void Census::load_file(World& w, const std::string& path, Walkers& walkers) {
  walkers_ = &walkers;
  const toml::table t = toml::parse_file(path);
  const auto* list = t["person"].as_array();
  if (!list) throw std::runtime_error("census: no [[person]] tables");
  for (const auto& n : *list) {
    const auto& x = *n.as_table();
    Person p;
    p.name = x["name"].value<std::string>().value();
    p.age = x["age"].value_or(30);
    p.sex = x["sex"].value_or(std::string("m")) == "f" ? Sex::Female : Sex::Male;
    p.household = x["household"].value_or(std::string{});
    p.home = x["home"].value_or(std::string{});
    p.survivor = x["survivor"].value_or(false);
    people_.push_back(std::move(p));
  }
  // Fates, drawn once at the start (rule 2), keyed by the person's index.
  for (std::size_t i = 0; i < people_.size(); ++i) {
    Person& p = people_[i];
    const auto key = static_cast<EntityId>(i + 1);
    const double u = w.rng.uniform01(key, Purpose::CensusFate);
    const auto home_cell = map_.find_object("home", p.home);
    if (p.survivor) p.fate = Fate::Survivor;
    else if (home_cell) p.fate = u < kHomeWalker ? Fate::WalkerOnMap : Fate::Fled;
    else p.fate = u < kOffMapWalker ? Fate::WalkerOffMap : u < kOffMapWalker + kOffMapDead ? Fate::Dead : Fate::Fled;
    w.ledger.mint(owners_[static_cast<std::size_t>(p.fate)], people_r_, 1);
    if (p.fate == Fate::WalkerOnMap || p.fate == Fate::WalkerOffMap) {
      p.body = bodies_.create(w, body_for(w.rng, key, p), p.name);
      by_body_[p.body] = i;
      if (p.fate == Fate::WalkerOnMap) walkers.spawn(w, p.body, *home_cell);
    }
  }
  // Entry cells: walkable outdoor ground-level cells on the border.
  for (int y = 0; y < map_.height(); ++y)
    for (int x = 0; x < map_.width(); ++x) {
      const Cell c{0, x, y};
      const bool border = x == 0 || y == 0 || x == map_.width() - 1 || y == map_.height() - 1;
      if (border && map_.walkable(c) && !is_indoor(map_.floor(c))) entries_.push_back(c);
    }
  if (entries_.empty()) throw std::runtime_error("census: the map has no outdoor border cells");
}

EntityId Census::embody(World& w, std::size_t person) {
  Person& p = people_.at(person);
  if (!p.body) {
    p.body = bodies_.create(w, body_for(w.rng, static_cast<EntityId>(person + 1), p), p.name);
    by_body_[p.body] = person;
  }
  return p.body;
}

std::vector<std::size_t> Census::survivors() const {
  std::vector<std::size_t> out;
  for (std::size_t i = 0; i < people_.size(); ++i)
    if (people_[i].fate == Fate::Survivor) out.push_back(i);
  return out;
}

void Census::add_pop(World& w, EntityId body, const std::string& name) {
  Person p;
  p.name = name;
  p.fate = Fate::Pop;
  p.body = body;
  const Body& b = bodies_.get(body);
  p.age = static_cast<int>(b.spec.age);
  p.sex = b.spec.sex;
  by_body_[body] = people_.size();
  people_.push_back(p);
  w.ledger.mint(owners_[static_cast<std::size_t>(Fate::Pop)], people_r_, 1);
}

std::optional<std::size_t> Census::person_of(EntityId body) const {
  auto it = by_body_.find(body);
  if (it == by_body_.end()) return std::nullopt;
  return it->second;
}

std::string Census::name_of(EntityId body) const {
  auto p = person_of(body);
  return p ? people_[*p].name : "someone";
}

std::int64_t Census::count(const World& w, Fate f) const {
  return w.ledger.balance(owners_[static_cast<std::size_t>(f)], people_r_);
}

int Census::off_map_walkers() const {
  int n = 0;
  for (const Person& p : people_) n += p.fate == Fate::WalkerOffMap;
  return n;
}

void Census::set_fate(World& w, std::size_t person, Fate f) {
  Person& p = people_.at(person);
  if (p.fate == f) return;
  w.ledger.transfer(w.clock.tick, owners_[static_cast<std::size_t>(p.fate)], owners_[static_cast<std::size_t>(f)],
                    people_r_, 1, 0);
  const bool died = is_dead(f) && !is_dead(p.fate);
  p.fate = f;
  if (died && pops_) pops_->record_death(w, census_pop_id(static_cast<std::uint32_t>(person)));
}

bool is_dead(Fate f) {
  return f == Fate::Dead || f == Fate::WalkerOffMap || f == Fate::WalkerOnMap || f == Fate::Destroyed;
}

double Census::border_m(const Cell& c) const {
  return std::min({c.x, c.y, map_.width() - 1 - c.x, map_.height() - 1 - c.y}) * 1.0;
}

void Census::arrive(World& w, std::size_t person) {
  const Cell at = entries_[w.rng.uniform_int(static_cast<EntityId>(person + 1), Purpose::Arrival, entries_.size())];
  set_fate(w, person, Fate::WalkerOnMap);
  walkers_->spawn(w, people_[person].body, at);
  w.emit(EventType::WalkerArrived, people_[person].body, 0, static_cast<std::int64_t>(person));
}

void Census::update(World& w) {
  if (!w.clock.is_body_step()) return;
  const double dt_h = kBodyStepTicks * kTickSeconds / 3600.0;
  // Attention from what reaches past the border (rule 3).
  const double bg = background_db(power_.mains_alive(), w.clock.hour());
  double beyond = 0;
  if (power_.generator_running()) {
    const NoiseSource gen{power_.generator_cell(), power_.generator().noise_db_1m};
    beyond += std::max(0.0, audible_range_m(map_, items_, gen, bg) - border_m(gen.at));
  }
  for (const WindowLeak& l : power_.window_leaks(w)) {
    const Cell c{l.edge.z, l.edge.x, l.edge.y};
    beyond += std::max(0.0, l.seen_m - border_m(c));
  }
  attention_ = attention_ * detmath::exp(-dt_h * 0.6931471805599453 / kAttentionHalfLifeH) + dt_h * beyond / 1000.0;
  // Arrivals.
  if (off_map_walkers() == 0) return;
  const double rate_per_s = (kBaseArrivalsPerH + kArrivalsPerAttention * attention_) / 3600.0;
  if (!hazard_fires(w.rng, 0, rate_per_s, dt_h * 3600.0)) return;
  for (std::size_t i = 0; i < people_.size(); ++i)
    if (people_[i].fate == Fate::WalkerOffMap) {
      arrive(w, i);
      break;
    }
}

void Census::save(ArchiveWriter& a) const {
  a.f64(attention_);
  a.u64(people_.size());
  for (const Person& p : people_) a.u8(static_cast<std::uint8_t>(p.fate));
}

void Census::load(ArchiveReader& r) {
  attention_ = r.f64();
  if (r.u64() != people_.size()) throw std::runtime_error("census: save does not match the table");
  for (Person& p : people_) p.fate = static_cast<Fate>(r.u8());
}

}  // namespace zs
