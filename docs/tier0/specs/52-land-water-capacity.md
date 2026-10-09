# 52 · Land: rivers, water, soil, capacity and natural resources

**Status:** Slices 1-3 implemented and tested on MSVC (2026-10-09); slice 4 Proposal. Written before its code; built in slices. **Slice 1:** runoff, rivers, lakes, fresh water, soil fertility, arable land and carrying capacity. **Slice 2:** forests, game and fish as living stocks on the map ([37](37-flora-and-fauna.md)). **Slice 3:** deposits. **Slice 4:** countries rebuilt on these ([48](48-people-sectors-countries.md) rules 3-5). **Needs:** [30](30-world-grid.md), [31](31-terrain-climate.md). **Decisions:** [0059](../../decisions/0059-countries-from-water-land-and-resources-everything-alive-can-be-sick.md), [0057](../../decisions/0057-country-economy-geography-zones.md). **Design:** [world systems](../../design/world-systems.md) steps 2-3. **Research:** [rivers and runoff](../../research/rivers-and-runoff.md) (RV), [world numbers capacity](../../research/world-numbers-capacity.md) (WNC), [ecology](../../research/world-systems-ecology.md) (WSE), [resources and networks](../../research/world-systems-resources-networks.md) (WSR).

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

## Rules (slice 2: living resources)

10. **Forest** (estimates): forest share of a land cell by biome (tropical forest 0.9, temperate forest 0.8, boreal 0.75, mountain 0.25, savanna 0.2, grassland 0.08, tundra 0.03, desert 0.01, ice 0), and growing stock per hectare of forest (tropical 195, temperate 140, boreal 90, mountain 75, savanna 40, grassland 35, tundra 20, desert 10 m³/ha), calibrated against 31 % of land and 557 billion m³ ([WSE-1](../../research/world-systems-ecology.md), RV-6). Regrowth rate r by biome inside WSE-34's 0.01-0.1 per year (tropical 0.06, temperate 0.04, boreal 0.02, others 0.03). The stock is today's, after the clearing already done.
11. **Game** (estimates): wild herbivore wet biomass per km² by biome today (savanna 300, grassland 125, temperate forest 75, tropical forest 60, boreal 40, mountain 30, tundra 20, desert 5 kg/km²), half again on a river cell, calibrated so the land's total is a part of RV-10's 47 Mt; r = 0.25 per year (estimate).
12. **Fish** (estimates): sea cells shallower than 1,500 m on the coarse grid (the map's shelf) carry a sustainable catch of 1 t per km² per year, times 1.5 within one cell of land and times (1 + (15 − T) / 30) for water temperature T (colder and coastal seas are richer), at least 0; r = 0.5 per year and K = 4 × catch / r (Schaefer, [WSE-15](../../research/world-systems-ecology.md)); the ocean's total is checked against RV-11.
13. These are the carrying capacities of spec 37's stocks per cell; spec 37's populations take them when the world is populated (slice 4).

## Rules (slice 3: deposits)

14. **Geological setting** per cell, from the plates of spec 31 ([WSR-29](../../research/world-systems-resources-networks.md)); a cell can have several:
    - *arc*: land within 3 cells of a subduction, trench or island-arc boundary;
    - *foreland*: land 2 to 6 cells from a collision boundary (in front of the mountains, [WSR-42](../../research/world-systems-resources-networks.md));
    - *rift*: land or shelf within 3 cells of a rift;
    - *craton*: land at least 8 cells from any boundary (old, stable crust);
    - *basin*: land below 600 m at least 3 cells from a boundary, or shelf (the sedimentary basins);
    - *laterite*: land at 20 °C or more, 1,200 mm of rain or more, below 800 m (deep tropical weathering).
15. **Odds** that a cell holds a deposit of a resource, by setting (estimates; the highest applies): coal: foreland 0.5, rift 0.2, basin 0.15; oil and gas: rift 0.3, foreland 0.2, basin 0.12; iron ore: craton 0.25 ([WSR-1](../../research/world-systems-resources-networks.md): banded iron in old cratons); copper ore: arc 0.4 (porphyry copper, [WSR-7](../../research/world-systems-resources-networks.md)); bauxite: laterite 0.35. Coal and the ores only on land; oil and gas on land and on the shelf. Keyed draws per (cell, resource).
16. **Size and grade.** Coal, iron ore, copper ore and bauxite are lognormal in size (σ = 1.0, estimate); oil and gas follow a Pareto tail with exponent 1, capped at 1,000 times the smallest ([WSR-30](../../research/world-systems-resources-networks.md)). A coarse cell's deposit is the sum of the deposits inside it. Grade independent of size ([WSR-6](../../research/world-systems-resources-networks.md)): copper lognormal around 0.39 % Cu (WSR-7), iron 30-64 % Fe, bauxite 40-55 % Al₂O₃ (estimates). The medians are calibrated once so the planet's totals fall between today's reserves and resources (rule 17's checks).
17. **Totals checked** against [WSR-37..41](../../research/world-systems-resources-networks.md), scaled to the planet's land: coal 1-10 times 1,074 billion t; oil 1-3 times 1,650 billion barrels; gas 1-3 times 207 trillion m³; iron ore 180-900 billion t; bauxite 23-75 billion t; copper metal 1-5 times 900 Mt.
18. Each deposit keeps why it is there (its setting and its distance to the boundary): world systems rule 2.

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

- **Slice 2** (living resources, 2026-10-09): `WorldLand` gains `forest_share`, `wood_m3`, `wood_r`, `game_kg`, `fish_catch_t`, `fish_k_t`; they are the capacities spec 37's stocks will take (not yet wired). **Measured** (seed 1): forest 29.8 % of land (WSE-1: 31 %), wood 536 billion m³ (RV-6: 557 on Earth's land), wild herbivores 21 Mt (RV-10: 47 Mt of wild mammals on land and sea), sustainable fish catch 111 Mt a year (RV-11: 90-110). The first estimates gave forest 25 %, game 42 Mt and fish 442 Mt (the coarse shelf is wide); the tables were scaled once to these.

- **Slice 3** (deposits, 2026-10-09): `engine/gen/include/zs/gen/world_deposits.h`, `src/world_deposits.cpp`: `Resource`, the setting flags, `cell_settings`, `Deposit` (cell, kind, amount, grade, settings, distance to the boundary), `WorldDeposits` (`total`, `metal`), `generate_deposits`; `zs gen world` prints the counts and totals; `test_world_deposits.cpp`, 3 cases, golden pinned on MSVC. **Measured** (seed 1): 739 deposits; coal 3,045 billion t (2.8 times today's reserves), oil 2,503 billion bbl (1.5 times), gas 299 trillion m³ (1.4 times), iron ore 401 billion t with 205 billion t of iron (reserves 180, resources over 900), copper 2.0 billion t of metal in 466 billion t of ore, bauxite 45 billion t (reserves 23-30, resources 55-75). The medians were set once from a probe at median 1 (totals are linear in the median); coal offshore was dropped after the test caught it, and its median doubled to keep the total. Iron is rare (30 cells): few cells of this coarse grid lie 8 or more cells from a plate boundary.

## Not in this spec (slice 1)

Inland seas and endorheic basins (Open 1); seasons and floods; groundwater; irrigation; erosion and soil degradation over time (0059 rule 5, in the simulation, not the generator); slices 2-4.

## Open

1. Endorheic basins (the Caspian, the Great Basin) need evaporation over the lake surface to balance inflow; the slice drains everything to the sea.
2. RV-5's PET and the biome tables of rules 5-6 are estimates, checked only through the totals the tests bound.
