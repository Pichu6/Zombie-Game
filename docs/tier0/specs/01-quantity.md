# 01 · Quantity

**Status:** Proposal. **Stage:** 1. **Needs:** nothing.

## Purpose

Every number in the simulation carries its unit and its representation, so a kilogram is never added to a litre and conserved values are never floats.

## State

| Field | Type | Notes |
| --- | --- | --- |
| value | `int64` (conserved) or `double` (continuous) | Chosen per field in the schema, never at the call site |
| unit | compile-time tag | SI base: g, mL, J, m, s, W, dB, lux, K |
| representation | `Integer` / `Continuous` | Integer for anything that moves between owners |

Conserved fields in Tier 0 use integer base units: mass in **grams**, volume in **millilitres**, food energy in **kilojoules**, fuel in **millilitres**, people as **counts**, time as **ticks**.

## Rules

1. Arithmetic is only defined between compatible units; mixed units fail to compile.
2. Integer quantities never round silently. A division returns quotient and remainder, and the caller says where the remainder goes.
3. Conversions to display units (kg, L, kcal, hours) happen only in the view.
4. Continuous values (fields, rates, body chemistry) use `double`, built with strict floating point (`/fp:precise`, no `-ffast-math`), so the same binary gives the same result.

## Invariants

- No conserved quantity is negative after a tick (checked by the ledger, [03](03-stock-transfer.md)).
- Every schema field declares unit, representation and range.

## Tests

- Compile-fail tests: adding g to mL, assigning a continuous value into a conserved field. (A1)
- Property: split-and-merge of an integer quantity returns the original exactly. (A1)

## Calibration

None: SI units only.

## Not in Tier 0

Generated schema header from one schema file (the research's "one schema"): Tier 0 writes the header by hand and keeps a single file for it.
