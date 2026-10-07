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

1. A single hazard h over a step Δt fires with p = 1 − exp(−h·Δt) (`detmath`), drawn with the hazard's own key.
2. When several hazards compete on one entity (an exit set), one draw decides whether any fires, with total rate H = Σh: p = 1 − exp(−H·Δt). A second draw picks which one, with weight hᵢ/H, walking the list in its fixed order (contract rule 3). No hazard is favoured by its position.
3. Known-time outcomes (a census death at a drawn time) are drawn once, up front, and scheduled; they are not re-rolled each tick.
4. Every event stores its cause (another event or a hazard with its source), so chains can be read back ([25](25-chronicle.md)).
5. Settings change hazard numbers, never which hazards exist.

Jump forms (added 2026-10-07 from [catch-up-jump-forms.md](../../research/catch-up-jump-forms.md); apply from stage 8 so [27](27-detail-levels.md) can jump):

6. Every hazard also provides its cumulative form H(t0, t1) along the entity's trajectory (closed form where the trajectory has one, else a fixed quadrature), so a jump can invert it.
7. An exit set may hold one Exp(1) draw per hazard, keyed (entity, hazard), and fire hazard k when H_k reaches its draw; the earliest wins. This is the preferred form for causes of death: hot (test H_k ≥ E_k each step) and cold (invert H_k) then give identical times and causes. Rule 2's per-step form stays valid and gives the same distribution.
8. A hazard that consumes an integer stock (people bitten, litres drunk, kg eaten) draws Binomial(stock, 1 − exp(−h·Δt)), never Poisson; hazards sharing a stock draw in the exit set's fixed order, each on what the previous left.
9. A jump over an interval with coupled hazards (one event changes another's rate) runs a next-reaction event loop inside the interval, applying events in time order; only hazards independent inside the interval may use a single closed-form draw.

## Invariants

- 0 ≤ p ≤ 1; at most one hazard of an exit set fires per step.
- Every event except the run start has a cause.

## Tests

- Halving Δt with the rate fixed gives the same firing-time distribution. (A2)
- A zeroed hazard never fires; its transition disappears. (A1, metamorphic)
- Two competing hazards with rates h₁ and h₂: each wins in proportion hᵢ/(h₁+h₂) over 10⁵ draws. (A2)

## Calibration

None for the unit itself; each user documents its rates.

## Not in Tier 0

Compartment (SEIRZ) hazards over cells.
