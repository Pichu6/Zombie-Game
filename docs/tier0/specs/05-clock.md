# 05 · Clock and update order

**Status:** Proposal. **Stage:** 1. **Needs:** [01](01-quantity.md).

## Purpose

One global clock and one fixed update order for every system, so a run is reproducible and every system reads the previous tick's state (contract rule 5).

## State

| Field | Type | Notes |
| --- | --- | --- |
| tick | `int64` | Game time = tick × tick length |
| tick length | 0.5 game s | Movement, actions, combat, fields |
| body step | every 120 ticks (60 game s) | Metabolism, needs, infection; bleeding uses the fine tick |
| speed | paused, 1×, 3×, 10×, rest-until-event | 1× = 1 game day per real hour (Decided) |
| slow-down rules | list of event types | Which events drop to 1× and optionally pause |

## Rules

1. Update order per tick (fixed): input commands → AI decisions → action executor → movement → damage → body (fine) → networks (on dirty) → fields → perception → body (slow, when due) → census schedule → ledger check → event log flush.
2. The fixed system order is what makes a tick reproducible. Inside a system, entities update in id order. Where one entity's update reads another's value and order would change the result (fields, perception, combat), the system reads start-of-tick values (a small copy), not a full double buffer of the world. (Simplified after stage 1: double-buffering every system was more machinery than Tier 0 needs.)
3. Speed is not simulation state: it lives in the app (`TimeControl`) and is not saved or hashed. It changes how many ticks run per real second, never what a tick does. At 1× that is 48 ticks per real second; at 10×, 480; rest-until-event runs as fast as the machine allows.
4. Event slow-down (RimWorld rule): alarm, attack on a pop, new wound, breakdown, new symptom noticed, found survivor → speed 1×, and pause if the setting is on. A zombie merely seen does not slow time.
5. Rest-until-event stops on any slow-down event or at a chosen hour.
6. Combat forces 1× while any pop is in melee range of a hostile.

7. No system iterates over a hash container (unordered map or set) when the order can affect results; entity lists are kept sorted by id. Path searches ([09](09-site-map.md), [18](18-noise-field.md)) use a fixed neighbour order and explicit tie-breaks.

## Invariants

- Tick count only grows; a save stores it.
- Results after N ticks do not depend on the speed setting or frame rate.

## Tests

- Same seed and inputs at 1× and at rest-until-event give the same state hash at day 3. (A1)
- Each slow-down event type triggers 1×. (A1)

## Calibration

Time scale: Decided ([0010](../../decisions/0010-concept-v2.md)). Tick length is an engineering choice; the benchmark in stage 3 confirms rest-until-event stays fast with 4 pops and ~60 walkers.

## Not in Tier 0

Multiple clocks per layer, level-of-detail wake-ups, the frozen clock during body switches.
