# 42 · The block's place in the world

**Status:** Implemented and tested, 2026-10-08 (`zs/place.h`, `zs/gen/place.h`; `test_one_world.cpp`, `test_place.cpp`; MSVC). **Stage:** the one world ([0052](../../decisions/0052-one-executable-one-simulation.md)), second slice after [41](41-one-world.md). **Needs:** [30](30-world-grid.md) (world grid), [31](31-terrain-climate.md) (terrain and climate), [36](36-fields-and-weather.md) (weather from climate), [17](17-power-network.md) (the fridge).

## Purpose

The block has been nowhere: its fridge warmed towards a fixed 20 °C, its day had no season. This spec puts it on one cell of a generated world. The cell's climate is the block's climate, and the weather of [36](36-fields-and-weather.md) at that cell is the block's weather, which the run reads.

## What the code forces

- **`engine/sim` cannot call `engine/gen`.** The world grid and fields are generated in `engine/gen`; the simulation only needs the chosen cell's numbers. So a **place** is plain data in the simulation (cell, latitude, longitude, elevation, biome, climate) and the generator chooses it. The one world is built by `zs::gen::make_world(seed)`, which generates the planet, chooses the cell and calls `tier0::make_run(seed, place)`; `make_run(seed)` without a place uses a fixed default place (tests that do not need the planet).
- **Bodies have no heat balance yet** ([11](11-body-composition.md)–[14](14-action-executor.md) model core temperature from fever only). Cold and heat exposure need a body spec of their own; this slice wires the weather to what already reads a temperature: the fridge.
- **The weather is costly per call** (a day's mean normalises a year of keyed anomalies), so it is computed once per game day and cached; the cache is derived, not saved.

## State

| Field | Notes |
| --- | --- |
| `Place` | `cell` (world grid id), `lat_deg`, `lon_deg`, `elevation_m`, `biome` (name), `Climate` (annual mean, seasonal range, annual rain, southern hemisphere), `world_seed`, `frequency`; saved |
| `PlaceSystem` | holds the place; no wake (`kNever`); answers the weather at the current tick |

## Rules

1. **Choosing the cell** (`zs::gen::choose_place(grid, fields, seed)`): land cells whose biome is temperate forest or grassland, the one whose annual mean is nearest 12 °C (a temperate town like the Tier 0 block's setting); ties by keyed draw on the cell id. If no cell qualifies, the land cell nearest 12 °C. Deterministic from (world parameters, seed).
2. **The default world** for `zs::gen::make_world(seed)`: frequency 24 (5,762 cells, about 88,500 km² each; fast enough to generate at every start), Earth radius, the world sliders' defaults of [31](31-terrain-climate.md), seed = the run's seed. Earth-size worlds (frequency 101) wait for the world creation screen.
3. **The default place** for `make_run(seed)` without a planet: a placeholder temperate cell (12 °C mean, 20 °C range, 800 mm rain, northern hemisphere, 40° N).
4. **Weather.** Outdoor temperature at the current tick is `fields::temperature_c(world_seed, cell, climate, tick)`; today's rain is `fields::rain_day_mm`. The daily mean is cached per game day.
5. **Indoors.** An unheated building's air is today's outdoor mean plus 3 °C of internal gains (placeholder estimate, unheated dwelling), so it follows the season and the weather's day-to-day swing with no diurnal swing.
6. **The fridge** warms towards the indoor temperature of rule 5 instead of the fixed `room_c`, in the one world only; stage scenarios keep `room_c`. Its shelf-life rule is unchanged, so a summer outage spoils food sooner than a winter one.
7. **The window** shows the outdoor temperature and today's rain in the time bar.
8. **Save and hash.** The place saves with the world; the stage 9 golden hash is re-pinned once (the place system's save).

## Invariants

- The chosen cell is land and, when any exists, temperate forest or grassland.
- The same seed chooses the same cell.
- The fridge's target equals `room_c` in stage scenarios and the indoor temperature in the one world.
- Save and load keep the place and the hash.

## Tests

- `engine/gen/tests/test_place.cpp`: the chosen cell is land and temperate; the same seed gives the same cell; `make_world` builds a run whose place is that cell.
- `engine/sim/tests/test_one_world.cpp`: the default place; outdoor temperature over a year averages the climate's mean (the [36](36-fields-and-weather.md) property, through the system); a fridge with no power in a hot place spoils food sooner than in a cold one.

## Not in this spec

- **Bodies reading the weather** (cold and heat exposure, wet clothing): its own body spec.
- **Daylight by latitude and season** (`Power::ambient_lux` still uses a fixed day).
- Saving the whole planet with the world, and Earth-size worlds: with the world creation screen and sectors and zones.
