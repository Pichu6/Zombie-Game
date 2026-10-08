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
2. Integer quantities never round silently. A division returns quotient and remainder, and the caller says where the remainder goes. A continuous rate that feeds an integer stock (fuel burnt per tick, blood lost per second) keeps a saved integer remainder per flow, so a slow flow is never rounded away (the stage 1 generator demo does this).
3. Conversions to display units (kg, L, kcal, hours) happen only in the view.
4. Continuous values (fields, rates, body chemistry) use `double`, built with strict floating point: `/fp:precise` on MSVC (2022 or newer), `-ffp-contract=off` and no `-ffast-math` on GCC and Clang, and `/clang:-ffp-contract=off` on clang-cl, which takes MSVC's flags but keeps contraction on without it ([DET-16](../../research/determinism.md)). SSE2 only (no x87). Targets are x64 only; ARM64, where FMA is in the base instruction set, is out of scope until a target needs it ([0055](../../decisions/0055-defaults-for-fixes-from-engineering-research.md) §3). (Added 2026-10-08 and built the same day in `CMakeLists.txt`, [research fixes](../research-fixes.md) package 3.)
5. The simulation never calls the platform's maths library for transcendental functions (`exp`, `log`, `log10`, `sin`, `cos`, `pow`). It uses our own `detmath` versions built only from `+ − × ÷ √`, which IEEE 754 rounds the same everywhere.

## Determinism target

- **A1 within one binary:** always (same data, seed and inputs give bit-identical state).
- **A1 across compilers (MSVC vs GCC/Clang):** required for integer and float paths. Measured on 2026-10-08: GCC 13.3 and Clang 18.1 reproduce every MSVC golden ([DET-1, DET-2](../../research/determinism.md)); CI checks it on Linux with GCC and Clang as well as on Windows ([0055](../../decisions/0055-defaults-for-fixes-from-engineering-research.md) §3.1; `.github/workflows/linux.yml`). A float path that differs is a bug to fix, not a per-compiler golden.

## Invariants

- No conserved quantity is negative after a tick (checked by the ledger, [03](03-stock-transfer.md)).
- Every schema field declares unit, representation and range.

## Tests

- Compile-fail tests: adding g to mL, assigning a continuous value into a conserved field. (A1)
- Property: split-and-merge of an integer quantity returns the original exactly. (A1)
- `detmath` functions give identical bits on Windows and Linux over 10⁶ fixed inputs, and stay within 1 ulp-scale tolerance of a reference libm. (A1 across compilers)
- `detmath` on non-finite and huge inputs: `sin(inf)` and `sin(1e19)` return NaN or assert instead of casting an unbounded double to an integer ([DET-8](../../research/determinism.md)). (A1)
- Negative check: the goldens fail when built with contraction on (`-ffp-contract=fast`), so the flags of rule 4 cannot quietly disappear ([DET-7](../../research/determinism.md)). (A1)

## Calibration

None: SI units only.

## Not in Tier 0

Generated schema header from one schema file (the research's "one schema"): Tier 0 writes the header by hand and keeps a single file for it.
