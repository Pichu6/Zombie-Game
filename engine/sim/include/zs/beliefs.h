// News and belief across levels: a message becomes a belief (what, claimed value, source, age, confidence) in a
// pop's store; retelling adds a hop and keyed distortion; a panel shown while playing a person reads beliefs,
// never the truth. Spec: docs/tier0/specs/39-orders-and-belief.md
#pragma once
#include <cstdint>
#include <map>
#include <vector>

#include "zs/archive.h"
#include "zs/draw.h"
#include "zs/pop.h"

namespace zs {

enum class Source : std::uint8_t { Seen = 0, Told = 1, Broadcast = 2 };

struct Belief {
  EntityId about;          // the thing (an edict, a place, a person)
  std::int64_t claim;      // the value believed (a share, a count, a state)
  Source source;
  PopId from = 0;          // who told it (Told)
  std::int64_t learned_tick;
  int hops = 0;            // retellings since the source
  double confidence = 1.0;
};

class Beliefs {
 public:
  // A message arrives: the pop now believes it, replacing an older belief about the same thing.
  void receive(PopId pop, Belief b);
  const Belief* about(PopId pop, EntityId about) const;
  const std::vector<Belief>& of(PopId pop) const;
  std::size_t pops() const { return by_pop_.size(); }

  // `from` tells `to` what it believes about `about`: one more hop, a keyed distortion of the claim (rumours distort
  // as they are retold) and less confidence. False if `from` believes nothing about it.
  bool retell(std::uint64_t seed, PopId from, PopId to, EntityId about, std::int64_t tick, double distortion_per_hop);

  // The lens while playing `pop`: what it believes, with its source and age; `known` false if nothing.
  struct View {
    bool known;
    std::int64_t claim;
    Source source;
    std::int64_t age_ticks;
    int hops;
    double confidence;
  };
  View view(PopId pop, EntityId about, std::int64_t now) const;

  void save(ArchiveWriter& w) const;
  void load(ArchiveReader& r);

 private:
  std::map<PopId, std::vector<Belief>> by_pop_;
};

}  // namespace zs
