# 36 · Fields and weather across levels

**Status:** Implemented and tested, headless (pure functions; nothing reads them yet). **Stage:** not Tier 0; the [layer jumps](../../design/layer-jumps.md) audit rows 11 (fields) and 12 (weather and climate). **Needs:** [02](02-draw.md), [31](31-terrain-climate.md). Relates to [18](18-noise-field.md) and [19](19-light-field.md) (the block-scale fields). Design: [world simulation](../../design/world-simulation.md) §9 (communication: the grid dying lets voices carry), [layer jumps](../../design/layer-jumps.md) §4 item 6. Decision: [0039](../../decisions/0039-fill-the-layer-jumps.md).

## Purpose

A climate cell's annual mean at Macro, a district's weather at City, the temperature on one body and the rain on one roof at Individual: one field. A sector's ambient noise at City and what a body hears at Individual: one field. This spec gives the two operators every field needs, **coarsen** (what is below, averaged) and **refine** (the coarse value plus keyed variation that averages back to it), and applies them to weather and to ambient noise, so that local conditions never contradict the field's mean and the mean is never kept apart from what is under it.

## What the code forces

- **Keyed draws only** ([02](02-draw.md), [31](31-terrain-climate.md) rule 1): a field value at a place and time is a pure function of (seed, cell, field, k). The simulation library cannot use the content tools' `Keyed` (it lives in `engine/gen`, which depends on `engine/sim`), so the same Philox is called here directly with the same key layout; a value drawn here and a value drawn by a generator with the same key are the same bits.
- **Deterministic transcendentals** ([01](01-quantity.md) rule 5): cosines and logs come from `detmath`, so a Windows build and a Linux build give the same weather.
- **Exactness over an interval** ([27](27-detail-levels.md) rule 2): a quantity that bodies and stocks will integrate (rain on a tank, degree-hours on a body) must sum the same over any partition of time. Rain is defined per day and falls evenly through the day, so a sum of pieces is the whole.

## State

None. Everything here is a function of the climate fields of [31](31-terrain-climate.md) (`temp_mean_c`, `temp_range_c`, `rain_mm`, hemisphere) and a key. There is nothing to save.

## Rules

1. **Refine.** `zero_mean_variation(seed, id, field, n, amplitude)` gives `n` keyed normal values of standard deviation about `amplitude`, shifted so their sum is exactly zero. A coarse value refined by them keeps its mean exactly (A1), with no band to tune.
2. **Coarsen.** `coarsen(values, weights)` is the area-weighted mean. Refine then coarsen is the identity (A1). This is the "aggregate against enumeration" property of the harness for fields.
3. **Daily temperature.** `daily_mean_c(cell, year, day)` = annual mean + a seasonal cosine of amplitude half the seasonal range (coldest near day 20, late January, in the north; half a year away in the south) + a keyed daily anomaly. The year's 365 anomalies are keyed normals smoothed over five days (weather has memory), scaled to a quarter of the seasonal range, and shifted so they **sum to zero**: the year's mean of daily means equals the climate's mean exactly (A1).
4. **Hourly temperature.** `temperature_c(cell, tick)` = the daily mean + a diurnal cosine, warmest at 15:00 and coldest at 03:00, of amplitude 2 °C plus 15 % of the seasonal range (a placeholder for continentality).
5. **Rain.** `rain_day_mm(cell, year, day)`: wet days drawn keyed with a share that rises with annual rain (15 % in a desert, about 50 % at 2,000 mm), amounts drawn exponential, and the year scaled so the 365 days **sum to the annual total exactly** (A1). `rain_between_mm(from, to)` spreads each day's rain evenly over its ticks, so any partition of an interval sums to the same total (A1).
6. **Ambient noise.** `ambient_db(sources)`: each source's level at 1 m less 20·log10(distance) ([18](18-noise-field.md) rule 1, spherical spreading, no walls at this scale), summed by energy with a floor of 20 dB (a still night). `audible_range_m(level, ambient)` = 10^((level − ambient − 3) ÷ 20) ([18](18-noise-field.md) rule 2): the distance at which the source is still 3 dB above the ambient. A 20 dB quieter ambient is a tenfold range, which is the design's claim ([world simulation](../../design/world-simulation.md) §9: voices carry about ten times farther when the grid dies).
7. **Keys.** Field numbers are append only (`Variation`, `DailyTemp`, `RainWet`, `RainAmount`); the year goes into `k`, so every year is a fresh draw and every year is reproducible on its own.

## Invariants

- Refine then coarsen returns the coarse value to floating-point rounding.
- Year mean of daily temperatures equals `temp_mean_c`; year sum of daily rain equals `rain_mm`; no negative rain.
- Rain over any partition of an interval equals the single interval's.
- Ambient is at least the floor; adding a source never lowers it; audible range is 0 for a source below the ambient.

## Tests (`test_fields.cpp`)

- **Round trip (A1):** property over seeds, ids, sizes and coarse values; determinism and keying; spread near the amplitude; weighted coarsen.
- **Weather (A1, A3):** three cells' yearly means equal the climate's mean to 1e-9; July minus January within the seasonal range band and reversed in the south; same key same weather, other year or cell different; neighbouring days closer than days a fortnight apart; 15:00 above and 03:00 below the daily mean by the diurnal amplitude.
- **Rain (A1):** desert and rainforest years sum to their totals exactly, with wet-day counts in the expected bands; no rain where the climate has none; a property over random partitions; whole years equal the annual total and two years differ day by day.
- **Noise (A1):** one source alone is its own level, two equal sources add 3 dB, the ambient falls when the grid's sources go; a 20 dB quieter ambient is exactly a tenfold range; a source below the ambient is not heard; an 89 dB engine carries from about 63 m at a 50 dB city night to about 630 m at 30 dB.

## As built

- `engine/sim/include/zs/fields.h`, `src/fields.cpp`, namespace `zs::fields`: `keyed_unit`, `keyed_normal`, `zero_mean_variation`, `coarsen`, `time_of_year`, `daily_mean_c`, `temperature_c`, `rain_day_mm`, `rain_between_mm`, `ambient_db`, `audible_range_m`.

## Calibration

| Value | Source |
| --- | --- |
| Daily anomaly a quarter of the seasonal range, five-day memory | Placeholder; to fit to station data (daily standard deviation about 3–5 °C in temperate climates) |
| Diurnal amplitude 2 °C + 15 % of the range | Placeholder; real diurnal range runs 5–20 °C by humidity and continentality |
| Wet-day share 15–50 % | Placeholder; to fit to station data (about 100–150 wet days a year in temperate Europe) |
| Spreading 20 dB per decade, floor 20 dB, margin 3 dB | [18](18-noise-field.md) and its sources |

## Not in this spec

Bodies reading the temperature (exposure, [11](11-body-composition.md) thermoregulation) and roofs or tanks reading the rain; wind, humidity, snow and storms as hazards drawn over an interval; the block-scale noise field ([18](18-noise-field.md)) taking its background from `ambient_db` of the district's sources; light at district scale (a city's glow); smoke and scent; monthly climate series in [31](31-terrain-climate.md) replacing the two-number seasonality.

## Open

1. **What refines below a cell.** A climate cell is about 5,000 km²; weather within it (a valley against a ridge) needs the terrain of [31](31-terrain-climate.md) at a finer grid before keyed variation alone is honest. Recommendation: keyed variation per zone now, terrain-driven refinement when the zone grid exists ([0037](../../decisions/0037-sectors-zones-and-world-creation-answers.md)).
2. **Correlation between cells.** Each cell's anomalies are keyed on its own id, so neighbouring cells' weather is independent, which is wrong for a front. Recommendation: key the anomalies on a coarser weather cell (a few hundred km) and refine down with rule 1, when weather matters to play.
