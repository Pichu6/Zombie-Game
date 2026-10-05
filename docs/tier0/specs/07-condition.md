# 07 · Condition

**Status:** Proposal. **Stage:** 2. **Needs:** [01](01-quantity.md), [04](04-hazard-event.md).

## Purpose

How worn or damaged a thing is, separate from how well it was made (Q) and what it is made of ([items and crafting](../../design/items-crafting.md#quality-condition-and-material)).

## State

| Field | Type | Notes |
| --- | --- | --- |
| condition | integer 0–10 000 (0.01 % steps) | Current |
| max condition | integer 0–10 000 | Repairs lower it; never rises except by replacing the part |
| wear law | per definition: per use, per hour running, per hit | Rate scaled by Q: higher Q wears less (Decided rule) |
| failure threshold | per definition | Below it the part stops working |

## Rules

1. Wear: Δcondition = − base wear × load factor × (1.5 − Q/100), clamped at 0. The (1.5 − Q/100) term is a starting curve to tune.
2. Repair restores condition up to max condition and lowers max condition by a per-recipe amount; the repair part's Q sets how long the fix lasts (it becomes a new component with its own Q and condition).
3. Below the failure threshold, the part reports failed; the assembly decides what that means ([08](08-assembly.md)).
4. Condition never changes Q.

## Invariants

- 0 ≤ condition ≤ max condition ≤ 10 000.

## Tests

- Property: wear never raises condition; repair never raises max condition. (A1)
- A Q80 spark plug outlasts a Q30 one under the same use. (A3)

## Calibration

| Value | Start | Source |
| --- | --- | --- |
| Spark plug service life | ~100 running hours for small engines | Typical small-engine maintenance schedules (estimate; tune) |
| Engine oil change | ~50 running hours | Same (estimate) |
| Air filter | ~25–50 hours in dusty use | Same (estimate) |

## Not in Tier 0

Corrosion and environment-driven decay (rust, rot), state changes like wet or frozen beyond what Tier 0 items need.
