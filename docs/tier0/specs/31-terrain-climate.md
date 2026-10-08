# 31 · Terrain and climate fields

**Status:** Implemented and tested (headless, GCC and Clang, same golden hash; see As built). Not Validated: the maps are plausible to the eye, not checked against Earth data. **Stage:** not Tier 0; steps 1 and 2 of the generator pipeline ([world systems](../../design/world-systems.md) §4, [world simulation](../../design/world-simulation.md) §12). **Needs:** [30](30-world-grid.md), [02](02-draw.md). Rivers, soil, flora and fauna are the next spec; this one stops at biome.

## Purpose

Fill every world cell with the physical fields the rest of the world is built from: elevation, sea, temperature, rain and biome. Everything downstream reads only these fields, so the same systems work on generated worlds and, later, on real Earth data ([world simulation](../../design/world-simulation.md) §12). The pipeline is plates → elevation → sea level → temperature → rain → biome. **Biome is derived, never a slider**, so it cannot contradict the climate (no rainforest at −10 °C).

## State

| Field | Unit | Owner | Notes |
| --- | --- | --- | --- |
| `plate[i]` | id | generator | Which plate the cell sits on |
| `elevation_m[i]` | m above sea level | generator | Negative under water |
| `land[i]` | bool | derived | `elevation_m > 0` |
| `temp_mean_c[i]` | °C | generator | Annual mean at the cell's elevation |
| `temp_range_c[i]` | °C | generator | Summer minus winter monthly mean (seasonality) |
| `rain_mm[i]` | mm per year | generator | Annual precipitation |
| `biome[i]` | enum | derived | From temperature, rain, elevation, land |

Parameters (the **world sliders**, [sandbox parameters](../../design/sandbox-parameters.md) §1.3; presets later): `land_share` (default 0.29), `global_temp_c` (mean sea-level temperature, default 14), `relief` (scales elevation spread, default 1), `moisture` (scales rain, default 1), `plate_count` (default about 14 at Earth's area, scaled by area, never fewer than 40 cells each), `seed`. All lock at creation. Fields are **saved with the world**; a generator update never changes a save.

## Rules

1. **Keyed draws only.** Every random choice is a pure function of `(seed, entity id, field, k)` through `Keyed` ([02](02-draw.md), content-tools §1 rule 1); no counters, no order dependence. New fields append to the field list, never renumber.
2. **Plates.** `plate_count` seed cells chosen by keyed best-candidate draws (spread out); a weighted flood over the graph (random cell cost, random plate weight) gives each cell a plate, so plates are connected. In keyed random order, plates turn continental until their area covers about 1.35 times `land_share` (Earth: continental crust about 40% against 29% land); the rest are oceanic. Each plate gets a base elevation (continental about +300 m, oceanic about −4,000 m, from the hypsometry of Earth, calibration below) and a rotation axis (velocity at a point is axis × position).
3. **Boundaries.** For two neighbouring cells on different plates, the **convergence** is the relative drift projected on the line between them. Convergent continent–continent or continent–ocean boundaries raise a mountain belt (elevation + convergence × `relief` × up to 5,000 m, falling off over about 4 cells); convergent ocean–ocean makes a trench (down) and an island arc; divergent boundaries make a rift or ridge. Cells far from boundaries are plain.
4. **Roughness.** A keyed random field is smoothed by repeated neighbour averaging (a fixed count, so it is deterministic), scaled to a few hundred metres, and added so no two cells are identical. Pure `+ − × /`.
5. **Sea level.** Chosen as the elevation quantile that makes the **area-weighted** land share equal `land_share` (cells sorted by elevation, ties by id). So the slider is exact and the number is a property of the result.
6. **Temperature.** Sea-level mean from a zonal profile by `z` = sine of latitude (26 °C at the equator, 20 at 30°, 8 at 45°, −1 at 60°, −15 at 75°, −25 at the pole, linear between), shifted so the area-weighted mean over the planet equals `global_temp_c` (an energy-balance stand-in: one global slider moves everything). Then a lapse of 6.5 °C per km of land elevation, and a mild ocean moderation. Seasonality `temp_range_c` grows with `|z|` and with distance from the sea (graph distance, computed once).
7. **Rain.** A latitude profile (wet equator, dry subtropics near 30°, wetter mid-latitudes, dry poles) times a moisture decay with graph distance from the sea along the prevailing wind direction (so interiors are drier), times `moisture`. Mountain belts add rain on their windward side and subtract on their lee (rain shadow, from the wind direction by latitude band). Rain never goes negative and is capped at 4,000 mm.
8. **Biome** from a Whittaker-style table of (`temp_mean_c`, `rain_mm`), with ice below −15 °C mean (height already lowers the temperature), a mountain class at 2,500 m and above, and ocean for water cells: ocean, ice, tundra, boreal forest, temperate forest, grassland, desert, savanna, tropical forest, mountain. The table is code for now (`classify_biome`), a pure function; moving it to data is later.
9. **Cause record.** Each cell keeps the few numbers that explain it (plate, nearest boundary kind and distance, latitude, distance from sea) so the inspector can answer "why is this here?" ([world systems](../../design/world-systems.md) §1 rule 2).
10. **Same seed, same world.** Output is a pure function of `(grid, parameters, seed)`.

## Invariants

- Area-weighted land share equals `land_share` within one cell's area share.
- Every cell has exactly one plate; every plate is non-empty and connected.
- No NaN or infinity anywhere; `rain_mm` in [0, 4000]; `elevation_m` within [−11,000, 9,000].
- Area-weighted mean of `temp_mean_c` (sea level equivalent) equals `global_temp_c` within 0.5 °C.
- Biome is a function of its inputs only: equal inputs give the same biome; no `tropical forest` below 18 °C mean, no `desert` above 1,000 mm of rain.

## Tests (agreement levels)

- Determinism: same seed twice gives identical fields; a different seed differs. (A1)
- Golden: hash of all fields for n = 8, seed 1, pinned on Linux; Windows must reproduce. (A1 across compilers)
- Land share is exact to one cell for `land_share` in {0.1, 0.29, 0.5, 0.8}. (A1)
- Hypsometry, mean of 10 seeds: fraction of land above 1,000 m between 5 % and 25 % (measured about 16 % at n = 16, 11 % at n = 32); mean ocean depth between 2,500 and 5,000 m (measured about 3,450 m; Earth: about 11 % and 3,700 m). (A3)
- Climate relationships: warmer at the equator than the poles; colder with height; the mean follows `global_temp_c` when it is changed by ±5 °C; more rain at the equator than at 30° over land on average; interior rain below coastal rain at the same latitude on average. (A3)
- Biome mix over 50 seeds at n = 16: every class but ice and mountain appears in at least 48 of 50 worlds, ice and mountain in at least 35 (they need a polar continent and a collision; measured 46 and 40); mean desert share 8–25 % of land (measured 17.7), ice plus tundra under 30 % (measured 15). (A2)
- Slider sweep: each of `relief`, `moisture`, `global_temp_c` moves its own field in the expected direction and leaves land share unchanged. (A3)
- Metamorphic: n = 32 against n = 16 over 10 seeds: mean biome shares within 8 points (the worlds differ, since plates are keyed to cell ids; only the distribution is compared). (A2)

## Calibration

| Value | Source |
| --- | --- |
| Land share 29 %, mean sea-level temperature 14 °C, lapse 6.5 °C/km | standard Earth values |
| Continental plateau about +840 m mean land height, mean ocean depth about 3,700 m, about 11 % of land above 1,000 m | hypsometric curve of Earth (second-hand, to firm up with the data pipeline) |
| Rain bands wet equator, dry 30°, wetter 45–60°, dry poles | general circulation (Hadley, Ferrel, polar cells) |
| Whittaker biome table | textbook classes, boundaries are design and tuned in the table |
| Mountain belt height up to about 5,000 m, trench to −11,000 m | Himalaya to Mariana Trench, order of magnitude |

All numbers are starting values to tune; the data pipeline ([world numbers](../../design/world-numbers.md) §10) replaces them with sourced ones.

## Not in this spec

Rivers and lakes, soil, flora and fauna (next spec), geology and deposits, monthly series (only mean and range now), ocean currents and ice dynamics, erosion over time, real Earth import, climate change during a run, a renderer or map view.

## As built

- `engine/gen`: `generate_fields(grid, params)` returning `WorldFields` (`zs/gen/world_fields.h`, `src/world_fields.cpp`), `classify_biome`, `area_share`. Tests: `engine/gen/tests/test_world_fields.cpp` (12 cases). Command: `zs_gen world [--seed N] [--freq F] [--land S] [--temp C] [--relief R] [--moisture M]` prints land share, hypsometry, biome shares and a text map.
- Boundary effects: collision +7,000 m × convergence, subduction (land side) +5,000, island arc +3,500, trench −6,000, ridge +1,500, rift −700; spread over about 350 km with a tenfold fade. Strengthened from the first draft (5,000 and 3,500) because mountains were absent from a quarter of worlds.
- Rain: latitude profile × (0.35 + 0.65 × moisture carried by the wind) + 900 mm per km of rising ground, moisture falling on rising ground (shadow behind it). Winds: easterlies below 30° and above about 60°, westerlies between.
- Biome thresholds as tuned: tropical forest from 1,100 mm (savanna was 30–49% of land at 1,400 mm).
- Cause record kept per cell: plate, `boundary_kind`, `boundary_cells` (distance to the nearest plate boundary), `sea_cells` (distance to the sea).
- *Found by building it:* (1) plate types drawn independently per plate made the sea-level cut land in odd places (ocean depth 700 m in one world), so types are now chosen by area; (2) a quadratic latitude temperature profile put ice at 57° latitude, so the zonal profile above replaced it; (3) the plate count must follow the planet's area, not the grid's cell count.
- Earth size (n = 101, 102,012 cells): grid and fields in about 0.3 s (Clang release); 12.8% of land above 1,000 m, mean ocean depth 3,474 m.
