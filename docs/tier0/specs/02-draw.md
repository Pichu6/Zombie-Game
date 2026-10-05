# 02 · Draw (keyed randomness)

**Status:** Proposal. **Stage:** 1. **Needs:** [01](01-quantity.md).

## Purpose

Randomness that never depends on the order in which systems run or on what the player looked at. Contract rule 4.

## State

| Field | Type | Notes |
| --- | --- | --- |
| world seed | `uint64` | Set in setup; part of the save |
| key | (entity id, purpose id, counter) | Purpose ids are a fixed enum: `wound_infection`, `bite_hit`, `census_fate`, ... |

## Rules

1. One counter-based generator: Philox-4x32-10 (Salmon et al. 2011, Random123). Output = Philox(seed, key).
2. Each (entity, purpose) keeps its own counter; drawing for one purpose never shifts another.
3. Our own samplers (uniform, normal by Box-Muller, exponential, Bernoulli, weighted choice), using `detmath` ([01](01-quantity.md) rule 5) for any log, sin or cos. Never `std::*_distribution`, whose output differs between standard libraries.
4. A draw is a pure function of seed, key and counter. Looking at an entity draws nothing.

## Invariants

- Same seed, same inputs: identical draws on every platform.
- Adding a new purpose does not change any existing stream.

## Tests

- Known-answer vectors from the Random123 reference. (A1)
- Same raw Philox integers on Windows (MSVC) and Linux (GCC/Clang). (A1 across compilers)
- Same sampler outputs across compilers, via `detmath`. (A1 across compilers; see [01](01-quantity.md) determinism target)
- Sampler moments within tolerance over 10⁶ draws (mean, variance). (A2)
- Metamorphic: relabelling entity ids permutes results, never changes their distribution. (A2)

## Calibration

None.

## Not in Tier 0

Gillespie switching for small counts in census cells.
