# 03 · Stock and transfer (the ledger)

**Status:** Proposal. **Stage:** 1. **Needs:** [01](01-quantity.md).

## Purpose

Fuel, food, water, nails, blood and people are integers held by exactly one owner and moved, never recomputed. Contract rules 2, 3 and 6.

## State

| Field | Type | Conserved | Owner |
| --- | --- | --- | --- |
| stock | (owner id, resource id) → integer Quantity | Yes | The owner (container, body, tank, census) |
| ledger entry | (tick, from, to, resource, amount, cause event) | — | Ledger |

Sources and sinks are explicit owners too: `world.burned` (fuel burnt), `world.metabolised`, `world.decayed`, `world.offmap`. Nothing appears or disappears without one.

## Rules

1. A transfer moves an amount from one owner to another in one step and writes one ledger entry with its cause.
2. Competing claims on one stock in the same tick are served in a fixed order (by system order, then entity id), each clamped to what is left. Nothing goes negative.
3. A transfer that cannot be served in full returns what it did move; the caller decides what that means.
4. A **conversion** turns an amount of one resource into another at a rate declared in data (food kJ into glycogen g, glycogen g into energy spent, fat g into kJ). It is one ledger entry with both sides, integer amounts, and the remainder handled as in [01](01-quantity.md) rule 2. Only declared pairs can convert; there is no kJ → mL pair.
5. Instances with identity (an axe, a person) move by changing their location record, through the same ledger.

## Invariants

- Per resource: sum over all owners, including sinks, is constant every tick, up to the conversions recorded in the ledger that tick.
- No stock below zero.
- Each instance has exactly one location; locations form a tree with no cycles.

## Tests

- Property (RapidCheck): random transfer and conversion sequences keep totals (net of recorded conversions) and never go negative. (A1)
- Debug builds check the invariants at the end of every tick and stop with the ledger tail on failure. (A1)

## Calibration

None.

## Not in Tier 0

Money and economy flows; cross-layer hand-offs between census, records and bodies.
