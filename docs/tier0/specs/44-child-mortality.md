# 44 · Child mortality

**Status:** Implemented and tested, 2026-10-08 (`Mortality::child_hazard`, `child_decline`; `tools/data/wpp2024.py`; `test_global_tables.cpp`; MSVC). **Stage:** [0052](../../decisions/0052-one-executable-one-simulation.md) item 4, second slice after [43](43-global-tables.md). **Needs:** [28](28-pop-levels.md), [43](43-global-tables.md).

## Purpose

With only accidents and ageing, the pop model had no deaths of small children: life expectancy at birth came out up to 3 years too high in poor countries, where 47 of every 1000 boys die before their first birthday. This spec adds the third hazard of the Siler model, high at birth and falling fast, so the simulation's lives match the real tables from birth to old age.

## Rules

1. **The hazard.** `μ(x) = A + B·e^{Cx} + D·e^{−Ex}`: constant (`constant_hazard`), ageing (`gompertz_b`, `gompertz_c`) and early childhood (`child_hazard` D at birth, `child_decline` E per year). `D = 0` is the spec 28 model.
2. **The draw.** The three hazards are independent; the death age is the earliest of three closed-form draws. The child hazard's cumulative from age `a` is `(D/E)(e^{−Ea} − e^{−Ex})`, bounded by `(D/E)e^{−Ea}`, so for an exponential draw `e3` the death age is `−ln(e^{−Ea} − E·e3/D)/E` when that logarithm's argument is positive and never otherwise (most children survive it). Key: `Purpose::PopLife`, b = 3.
3. **The fit.** All five parameters per group and sex, least squares on log `qx` of every abridged age group below 95 (the model's own interval death probability, so the infant year is exact), each group weighted by its share of the life table's deaths plus a floor of 0.002, so the ages where people die count most. Replaces spec 43's Gompertz–Makeham fit on ages 15–89.
4. **Save.** Registry version 7 carries the child hazard of both sexes after the fertility hazards; written only when either is non-zero. `fixtures/popreg_v7.bin` is frozen; versions 1–6 load as before with no child hazard.

## Results (WPP 2024, 2023)

| Group | e0 m / f, model | e0 m / f, WPP | e15 m / f, model | e15 m / f, WPP |
| --- | --- | --- | --- | --- |
| World | 70.53 / 75.89 | 70.55 / 75.89 | 59.04 / 64.14 | 58.91 / 64.09 |
| High | 78.92 / 84.12 | 78.84 / 84.05 | 64.53 / 69.62 | 64.33 / 69.49 |
| Upper-middle | 72.86 / 79.00 | 72.87 / 79.01 | 59.40 / 65.32 | 59.19 / 65.21 |
| Lower-middle | 67.60 / 71.55 | 67.65 / 71.61 | 56.41 / 60.16 | 56.41 / 60.26 |
| Low | 62.47 / 67.24 | 62.58 / 67.32 | 53.40 / 57.39 | 53.16 / 57.32 |

The simulation's own draws (400,000 lives per case) give life expectancy at birth within 0.12 years of WPP and infant deaths within 4 % (low-income boys: 47.1 per 1000 drawn, 47.4 in WPP).

## Not in this spec

- Causes of death by age (the three hazards are not causes; a cause table is its own data).
- Mortality that changes over time or with the outbreak and the economy (the tables are one year).
