// Fault injection sites (spec 57, ranked item 4): rare failures forced at named points inside the systems, so the
// branches that real play reaches once in a long while run in every test of the three promises (a jump equals
// stepping, a save loaded into a fresh setup equals the world that wrote it, the same seed gives the same run).
// Off by default (seed 0). The answer is a pure function of (seed, tick, site, key): a stepped world, a jumped one and
// a reloaded one see the same faults. Never saved, never hashed: a world is built with its seed by the setup.
// Research: docs/research/hordes-sound-sky-and-testing.md HST-104..HST-118 (FoundationDB's buggify, TigerBeetle).
#pragma once
#include <array>
#include <cstdint>

namespace zs {

enum class FaultSite : std::uint32_t {
  GeneratorWontStart,  // Power::start_generator: the engine does not catch this try
  PopHesitates,        // PopAI::update: a pop's decision is skipped this period
  WalkerStumbles,      // Walkers::move: a walker does not move this tick
  kCount
};

class Buggify {
 public:
  std::uint64_t seed = 0;  // 0 = off

  // True for about `per_million` of the (tick, site, key) triples, the same triples on every run and compiler.
  bool fire(FaultSite site, std::int64_t tick, std::uint64_t key, std::uint32_t per_million) {
    if (seed == 0) return false;
    std::uint64_t z = seed ^ (static_cast<std::uint64_t>(site) + 1) * 0x9E3779B97F4A7C15ull;
    z ^= static_cast<std::uint64_t>(tick) * 0xBF58476D1CE4E5B9ull;
    z ^= key * 0x94D049BB133111EBull;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    z ^= z >> 31;
    const bool hit = z % 1000000 < per_million;
    if (hit) ++hits[static_cast<std::size_t>(site)];
    return hit;
  }

  // How many times each site fired in this world instance (coverage: a site that never fires tests nothing).
  std::array<std::uint64_t, static_cast<std::size_t>(FaultSite::kCount)> hits{};
};

}  // namespace zs
