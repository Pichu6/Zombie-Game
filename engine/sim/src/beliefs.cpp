// Spec: docs/tier0/specs/39-orders-and-belief.md
#include "zs/beliefs.h"

#include <algorithm>
#include <cmath>

#include "zs/fields.h"

namespace zs {

namespace {
enum BeliefField : std::uint32_t { Distortion = 301 };
}

void Beliefs::receive(PopId pop, Belief b) {
  std::vector<Belief>& v = by_pop_[pop];
  for (Belief& old : v)
    if (old.about == b.about) { old = b; return; }
  v.push_back(b);
}

const Belief* Beliefs::about(PopId pop, EntityId about) const {
  auto it = by_pop_.find(pop);
  if (it == by_pop_.end()) return nullptr;
  for (const Belief& b : it->second) if (b.about == about) return &b;
  return nullptr;
}

const std::vector<Belief>& Beliefs::of(PopId pop) const {
  static const std::vector<Belief> none;
  auto it = by_pop_.find(pop);
  return it == by_pop_.end() ? none : it->second;
}

bool Beliefs::retell(std::uint64_t seed, PopId from, PopId to, EntityId about, std::int64_t tick, double distortion_per_hop) {
  const Belief* src = this->about(from, about);
  if (!src || from == to) return false;
  Belief b = *src;
  b.source = Source::Told;
  b.from = from;
  b.learned_tick = tick;
  b.hops = src->hops + 1;
  b.confidence = src->confidence * 0.7;
  // The claim drifts by a keyed share of its size per hop, keyed on (teller, listener, thing, hop), so the same
  // retelling always distorts the same way and a story told twice is one story.
  const double n = fields::keyed_normal(seed, (static_cast<std::uint64_t>(from) << 32) ^ to ^ (about << 7), Distortion, static_cast<std::uint32_t>(b.hops));
  const double scale = std::max(1.0, std::fabs(static_cast<double>(src->claim)));
  b.claim = src->claim + static_cast<std::int64_t>(std::llround(n * distortion_per_hop * scale));
  receive(to, b);
  return true;
}

Beliefs::View Beliefs::view(PopId pop, EntityId about, std::int64_t now) const {
  const Belief* b = this->about(pop, about);
  if (!b) return {false, 0, Source::Seen, 0, 0, 0.0};
  return {true, b->claim, b->source, now - b->learned_tick, b->hops, b->confidence};
}

void Beliefs::save(ArchiveWriter& w) const {
  w.u64(by_pop_.size());
  for (const auto& [pop, v] : by_pop_) {
    w.u64(pop);
    w.u64(v.size());
    for (const Belief& b : v) {
      w.u64(b.about); w.i64(b.claim); w.u8(static_cast<std::uint8_t>(b.source)); w.u64(b.from);
      w.i64(b.learned_tick); w.u32(static_cast<std::uint32_t>(b.hops)); w.f64(b.confidence);
    }
  }
}

void Beliefs::load(ArchiveReader& r) {
  std::map<PopId, std::vector<Belief>> by_pop;
  const std::uint64_t pops = r.u64();
  for (std::uint64_t i = 0; i < pops; ++i) {
    const PopId pop = r.u64();
    std::vector<Belief> v(r.u64());
    for (Belief& b : v) {
      b.about = r.u64(); b.claim = r.i64(); b.source = static_cast<Source>(r.u8()); b.from = r.u64();
      b.learned_tick = r.i64(); b.hops = static_cast<int>(r.u32()); b.confidence = r.f64();
    }
    by_pop[pop] = std::move(v);
  }
  by_pop_ = std::move(by_pop);
}

}  // namespace zs
