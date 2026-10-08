// Spec: docs/tier0/specs/38-outbreak.md
#include "zs/outbreak.h"

#include <algorithm>
#include <stdexcept>

#include "zs/detmath.h"
#include "zs/fields.h"

namespace zs::outbreak {

namespace {
enum OutbreakField : std::uint32_t { Catch = 201, Bite = 202, Kill = 203, Contacts = 204, Who = 205, Turn = 206, Binomial = 207 };
}

double catch_chance(const Micro& m) {
  // A faster zombie catches more often; warned people keep their distance.
  const double ratio = m.human_speed_mps > 0 ? m.zombie_speed_mps / m.human_speed_mps : 1.0;
  return std::clamp(ratio, 0.0, 1.0) * (1.0 - 0.7 * std::clamp(m.warning, 0.0, 1.0));
}
double bite_chance(const Micro& m) { return catch_chance(m) * std::clamp(m.bite_base, 0.0, 1.0); }
double kill_chance(const Micro& m) { return std::clamp(m.warning, 0.0, 1.0) * std::clamp(m.kill_skill, 0.0, 1.0); }

Outcome encounter(const Micro& m, std::uint64_t seed, std::uint64_t zombie, std::uint32_t k) {
  const bool caught = fields::keyed_unit(seed, zombie, Catch, k) < catch_chance(m);
  const bool bitten = caught && fields::keyed_unit(seed, zombie, Bite, k) < std::clamp(m.bite_base, 0.0, 1.0);
  const bool destroyed = fields::keyed_unit(seed, zombie, Kill, k) < kill_chance(m);
  if (bitten && destroyed) return Outcome::BittenAndDestroyed;
  if (bitten) return Outcome::Bitten;
  if (destroyed) return Outcome::Destroyed;
  return Outcome::Nothing;
}

double Rates::r0() const {
  const double per_contact = bite * convert;
  const double contacts = destroyed > 0 ? std::min(1.0 / destroyed, 60.0 * contacts_per_day) : 60.0 * contacts_per_day;
  return per_contact * contacts;
}

Rates measure(const Micro& m, std::uint64_t seed, std::uint32_t encounters) {
  if (encounters == 0) throw std::invalid_argument("outbreak::measure: no encounters");
  std::uint32_t bites = 0, kills = 0;
  for (std::uint32_t k = 0; k < encounters; ++k) {
    const Outcome o = encounter(m, seed, 0, k);
    if (o == Outcome::Bitten || o == Outcome::BittenAndDestroyed) ++bites;
    if (o == Outcome::Destroyed || o == Outcome::BittenAndDestroyed) ++kills;
  }
  Rates r;
  r.bite = static_cast<double>(bites) / encounters;
  r.destroyed = static_cast<double>(kills) / encounters;
  r.convert = std::clamp(m.convert, 0.0, 1.0);
  r.contacts_per_day = m.contacts_per_day;
  return r;
}

std::int64_t keyed_binomial(std::uint64_t seed, std::uint64_t id, std::uint32_t field, std::uint32_t k, std::int64_t n, double p) {
  if (n <= 0 || p <= 0) return 0;
  if (p >= 1) return n;
  const double u = fields::keyed_unit(seed, id, field, k);
  // Inversion from 0 upward with the recurrence pmf(k+1) = pmf(k) (n-k)/(k+1) p/(1-p).
  const double q = 1.0 - p;
  double pmf = detmath::exp(static_cast<double>(n) * detmath::log(q));
  if (pmf <= 0) {  // underflow for large n p: fall back to a normal approximation, rounded and clamped
    const double mean = static_cast<double>(n) * p, sd = detmath::sqrt(mean * q);
    const double z = detmath::sqrt(-2.0 * detmath::log(std::max(u, 1e-300))) * detmath::cos(2.0 * detmath::pi * fields::keyed_unit(seed, id, field, k + 0x40000000u));
    return std::clamp<std::int64_t>(static_cast<std::int64_t>(mean + sd * z + 0.5), 0, n);
  }
  double cum = pmf;
  std::int64_t x = 0;
  while (u >= cum && x < n) {
    pmf *= static_cast<double>(n - x) / static_cast<double>(x + 1) * p / q;
    cum += pmf;
    ++x;
  }
  return x;
}

void aggregate_day(Compartments& c, const Rates& r, const Micro& m, std::uint64_t seed, std::uint64_t region, std::int64_t day) {
  const auto k = static_cast<std::uint32_t>(day);
  // Exposed turn first (they were bitten on earlier days).
  const double turn_p = m.turn_days > 0 ? std::min(1.0, 1.0 / m.turn_days) : 1.0;
  const std::int64_t turned = keyed_binomial(seed, region, Turn, k, c.e, turn_p);
  c.e -= turned;
  c.z += turned;
  if (c.z <= 0 || c.people() <= 0) return;
  // Encounters this day: each zombie meets contacts_per_day people, drawn from the living.
  const std::int64_t encounters = keyed_binomial(seed, region, Contacts, k, c.z * 100, std::min(1.0, r.contacts_per_day / 100.0));
  const double s_share = static_cast<double>(c.s) / static_cast<double>(c.people());
  const std::int64_t with_s = keyed_binomial(seed, region, Who, k, encounters, s_share);
  std::int64_t new_e = keyed_binomial(seed, region, Bite, k, with_s, r.bite * r.convert);
  new_e = std::min(new_e, c.s);
  std::int64_t destroyed = keyed_binomial(seed, region, Kill, k, encounters, r.destroyed);
  destroyed = std::min(destroyed, c.z);
  c.s -= new_e;
  c.e += new_e;
  c.z -= destroyed;
  c.r += destroyed;
}

void aggregate_days(Compartments& c, const Rates& r, const Micro& m, std::uint64_t seed, std::uint64_t region,
                    std::int64_t from_day, std::int64_t to_day) {
  for (std::int64_t d = from_day; d < to_day; ++d) aggregate_day(c, r, m, seed, region, d);
}

// ---- Entities

Entities::Entities(std::int64_t people, std::int64_t zombies) {
  all_.resize(static_cast<std::size_t>(people + zombies));
  for (std::int64_t i = people; i < people + zombies; ++i) all_[static_cast<std::size_t>(i)].state = State::Zombie;
}

void Entities::run_day(const Micro& m, std::uint64_t seed, std::uint64_t region, std::int64_t day) {
  const double turn_p = m.turn_days > 0 ? std::min(1.0, 1.0 / m.turn_days) : 1.0;
  // Turning: each exposed one, with the same daily chance as the aggregate.
  for (std::size_t i = 0; i < all_.size(); ++i) {
    Entity& e = all_[i];
    if (e.state != State::Exposed) continue;
    if (fields::keyed_unit(seed, region * 1000003ull + i, Turn, static_cast<std::uint32_t>(day)) < turn_p) e.state = State::Zombie;
  }
  // Encounters: each zombie, its contacts for the day, each with a person drawn from the living.
  std::vector<std::size_t> living;
  for (std::size_t i = 0; i < all_.size(); ++i)
    if (all_[i].state == State::Susceptible || all_[i].state == State::Exposed) living.push_back(i);
  const auto k = static_cast<std::uint32_t>(day);
  for (std::size_t zi = 0; zi < all_.size(); ++zi) {
    if (all_[zi].state != State::Zombie || living.empty()) continue;
    const std::uint64_t zid = region * 1000003ull + zi;
    const std::int64_t contacts = keyed_binomial(seed, zid, Contacts, k, 100, std::min(1.0, m.contacts_per_day / 100.0));
    // A zombie finishes its day's encounters even if one of them destroys it, as the aggregate's tau-leap does.
    bool destroyed = false;
    for (std::int64_t c = 0; c < contacts; ++c) {
      const auto kc = static_cast<std::uint32_t>(k * 128 + c);
      const std::size_t who = living[static_cast<std::size_t>(fields::keyed_unit(seed, zid, Who, kc) * static_cast<double>(living.size()))];
      const Outcome o = encounter(m, seed, zid, kc);
      if ((o == Outcome::Bitten || o == Outcome::BittenAndDestroyed) && all_[who].state == State::Susceptible &&
          fields::keyed_unit(seed, zid, Bite + 100, kc) < std::clamp(m.convert, 0.0, 1.0))
        all_[who].state = State::Exposed;
      if (o == Outcome::Destroyed || o == Outcome::BittenAndDestroyed) destroyed = true;
    }
    if (destroyed) all_[zi].state = State::Destroyed;
  }
}

Compartments Entities::counts() const {
  Compartments c;
  for (const Entity& e : all_) {
    switch (e.state) {
      case State::Susceptible: ++c.s; break;
      case State::Exposed: ++c.e; break;
      case State::Zombie: ++c.z; break;
      case State::Destroyed: ++c.r; break;
    }
  }
  return c;
}

}  // namespace zs::outbreak
