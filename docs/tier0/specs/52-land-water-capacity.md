# 52 · Land: rivers, water, soil, capacity and natural resources

**Status:** Slice 1 implemented and tested on MSVC (2026-10-09); slices 2-4 Proposal. Written before its code; built in slices. **Slice 1:** runoff, rivers, lakes, fresh water, soil fertility, arable land and carrying capacity. **Slice 2:** forests, game and fish as living stocks on the map ([37](37-flora-and-fauna.md)). **Slice 3:** deposits. **Slice 4:** countries rebuilt on these ([48](48-people-sectors-countries.md) rules 3-5). **Needs:** [30](30-world-grid.md), [31](31-terrain-climate.md). **Decisions:** [0059](../../decisions/0059-countries-from-water-land-and-resources-everything-alive-can-be-sick.md), [0057](../../decisions/0057-country-economy-geography-zones.md). **Design:** [world systems](../../design/world-systems.md) steps 2-3. **Research:** [rivers and runoff](../../research/rivers-and-runoff.md) (RV), [world numbers capacity](../../research/world-numbers-capacity.md) (WNC), [ecology](../../research/world-systems-ecology.md) (WSE), [resources and networks](../../research/world-systems-resources-networks.md) (WSR).

## Purpose

Countries must form where water, food and resources are (0059), and everything placed later must have a reason (0057). Today the world has temperature, rain, elevation and biome per cell, and people are spread by a suitability score. This spec adds what the land holds: where the rain goes (rivers, lakes), how fertile the soil is, how much of it can be farmed, and how many people the cell can feed and water. Later slices add forests, game, fish and deposits.

## State (slice 1)

`Land` per cell, a pure function of (grid, fields, params), saved with the world like the fields:

| Field | Notes |
| --- | --- |
| `runoff_mm` | P − E per year on land, 0 at sea |
| `downstream` | the cell this one drains to (a sea cell for the last land step); none at sea |
| `discharge_m3s` | the water passing through: its own runoff plus everything upstream |
| `catchment_km2` | the land that drains through the cell, itself included |
| `river` | 0 none, 1 river, 2 large river (RV-9) |
| `lake` | a filled depression: water stands there |
| `fertility` | soil fertility 0-1 |
| `arable_share` | share of the cell that can be cropped |
| `pasture_share` | share fit for grazing only |
| `capacity_food`, `capacity_water` | people the cell can feed and water at the start's technology; `capacity` = the smaller |

## Rules (slice 1)

1. **Evaporation and runoff.** PET = max(100, 40 T + 200) mm (RV-5, estimate calibrated to RV-1, T the annual mean); E from Fu's equation with ω = 2.6 (RV-3); runoff = P − E, never negative. Evaluated with `detmath` (spec 01 rule 5).
2. **Drainage.** A priority flood from the sea over the land, lowest first (ties by cell id), gives every land cell a downstream neighbour and the elevation water fills it to; a cell filled above its own elevation is a lake. All land drains to the sea in this slice (inland seas: Open 1).
3. **Discharge** sums runoff × area downstream, upstream cells first (the reverse of the flood's order); water is conserved: the discharge into the sea equals the sum of the land's runoff.
4. **Rivers:** at least 100 m³/s and a catchment of 100,000 km² make a river, 1,000 m³/s and 500,000 km² a large river (RV-9); the catchment keeps a single wet cell of a coarse grid from counting.
5. **Fertility** by biome (estimates: grassland 0.9, temperate forest 0.8, savanna 0.6, tropical forest 0.5, boreal 0.4, mountain 0.25, tundra 0.15, desert 0.1, ice 0), plus 0.2 on a river cell (alluvium), capped at 1.
6. **Arable and pasture shares** by biome (estimates: arable grassland 0.45, temperate forest 0.35, savanna 0.25, tropical forest 0.15, boreal 0.08, mountain 0.04, desert 0.02, tundra and ice 0; pasture grassland 0.35, savanna 0.45, desert 0.08, mountain 0.15, tundra 0.15, others 0.05), plus 0.25 arable on a river cell in a desert (an oasis valley), capped so the two sum to at most 1.
7. **Food capacity** (WNC-26): cropland ha × yield × fertility × kcal per t × (1 − feed share) + pasture ha × a tenth of that per ha (WNC-25), over 365 × kcal per person per day. Start technology (estimates): yield 4 t/ha (WNC-13 spans 1-8 t/ha by era), 3.4 million kcal per t, feed share 0.36, 2,500 kcal per person per day.
8. **Water capacity:** (runoff volume + the discharge of a river cell) / 500 m³ per person per year, the absolute-scarcity line (WNC-17); a river's water is counted in every cell it crosses (it is reused downstream).
9. **Capacity** = min(food, water) (WNC-28). Trade and technology later let a country exceed its own capacity.

## Invariants

- Every land cell drains to the sea by a finite chain of downstream steps.
- Discharge into the sea equals the land's total runoff (to rounding).

## Tests (`engine/gen/tests/test_world_land.cpp`, slice 1)

- Drainage reaches the sea from every land cell; water is conserved. (A1)
- On the default (Earth-like) world the runoff ratio is within 25-45 % of land precipitation (RV-1: 35 %). (A2)
- The largest river's discharge is between 20,000 and 400,000 m³/s (the Amazon carries about 200,000). (A2)
- Arable land is 10-35 % of land; total capacity is between 0.5 and 5 times today's world population scaled to the planet's land (spec 48 rule 2). (A2)
- A desert river cell has more capacity than the desert cells beside it. (A1)
- Same grid, fields and seed give the same land; golden hash pinned on MSVC. (A1)

## As built (slice 1)

- `engine/gen/include/zs/gen/world_land.h`, `src/world_land.cpp`: `LandParams`, `WorldLand` (`runoff_mm`, `downstream`, `discharge_m3s`, `catchment_km2`, `river`, `lake`, `fertility`, `arable_share`, `pasture_share`, `capacity_food`, `capacity_water`, `capacity()`), `generate_land`.
- `zs gen world` prints the land summary and draws rivers (light blue) and large rivers (bright blue) on its `--bmp` map; the text map marks large rivers `~`.
- `engine/gen/tests/test_world_land.cpp`: 5 cases; golden pinned on MSVC.
- **Measured** (seed 1, frequency 24, default sliders): runoff 31,019 km³ a year, 34.2 % of the land's 90,762 km³ of rain (RV-1: 35 %; PET calibrated by a sweep, RV-5); largest river 39,168 m³/s (the Amazon carries about 200,000: the coarse grid and the generated continents make smaller basins); 610 river cells and 95 large-river cells of 1,673 land cells; 87 lake cells; arable land 17.6 % of land (Earth's cropland is about 11 %, WNC-3, and more could be cropped); capacity 16.9 billion people, 2.1 times today's population scaled to the planet's land.

## Not in this spec (slice 1)

Inland seas and endorheic basins (Open 1); seasons and floods; groundwater; irrigation; erosion and soil degradation over time (0059 rule 5, in the simulation, not the generator); slices 2-4.

## Open

1. Endorheic basins (the Caspian, the Great Basin) need evaporation over the lake surface to balance inflow; the slice drains everything to the sea.
2. RV-5's PET and the biome tables of rules 5-6 are estimates, checked only through the totals the tests bound.
