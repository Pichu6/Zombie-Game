# 04 · Hazard and event

**Status:** Proposal. **Stage:** 1. **Needs:** [02](02-draw.md), [05](05-clock.md).

## Purpose

Every "may happen" in the game (a wound gets infected, a hinge fails, a census neighbour turns) is a rate with a source, and every "did happen" is an event with a cause. One rule written once (contract rule 1).

## State

| Field | Type | Notes |
| --- | --- | --- |
| hazard | rate per second, source fields, purpose id | e.g. infection onset rate of one wound |
| exit set | ordered list of competing hazards on one entity | Order is fixed (contract rule 3) |
| event | id, tick, type, subject, cause event id, payload | Immutable once written |
| schedule | priority queue of (tick, event) | For deferred, known-time events |

## Rules

1. A hazard h over a step Δt fires with p = 1 − exp(−h·Δt), drawn with the hazard's own key.
2. When several hazards compete on one entity in one step, they are tested in exit-set order; the first that fires wins and the rest are skipped this step.
3. Known-time outcomes (a census death at a drawn time) are drawn once, up front, and scheduled; they are not re-rolled each tick.
4. Every event stores its cause (another event or a hazard with its source), so chains can be read back ([25](25-chronicle.md)).
5. Settings change hazard numbers, never which hazards exist.

## Invariants

- 0 ≤ p ≤ 1 and the probabilities of one exit set sum to at most 1.
- Every event except the run start has a cause.

## Tests

- Halving Δt with the rate fixed gives the same firing-time distribution. (A2)
- A zeroed hazard never fires; its transition disappears. (A1, metamorphic)
- Exit order respected: with two certain hazards the first always wins. (A1)

## Calibration

None for the unit itself; each user documents its rates.

## Not in Tier 0

Compartment (SEIRZ) hazards over cells.
