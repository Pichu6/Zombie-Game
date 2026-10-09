# 52 · Land: rivers, water, soil, capacity and natural resources

**Status:** Slices 1-4 implemented and tested on MSVC (2026-10-09). Written before its code; built in slices. **Slice 1:** runoff, rivers, lakes, fresh water, soil fertility, arable land and carrying capacity. **Slice 2:** forests, game and fish as living stocks on the map ([37](37-flora-and-fauna.md)). **Slice 3:** deposits. **Slice 4:** countries rebuilt on these ([48](48-people-sectors-countries.md) rules 3-5). **Needs:** [30](30-world-grid.md), [31](31-terrain-climate.md). **Decisions:** [0059](../../decisions/0059-countries-from-water-land-and-resources-everything-alive-can-be-sick.md), [0057](../../decisions/0057-country-economy-geography-zones.md). **Design:** [world systems](../../design/world-systems.md) steps 2-3. **Research:** [rivers and runoff](../../research/rivers-and-runoff.md) (RV), [world numbers capacity](../../research/world-numbers-capacity.md) (WNC), [ecology](../../research/world-systems-ecology.md) (WSE), [resources and networks](../../research/world-systems-resources-networks.md) (WSR).

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

## Rules (slice 4: countries from the land)

Replaces spec 48 rules 3-5 (sectors cut from the whole land, countries of three sectors, income by suitability). The contest is the Fractured-Land model ([CF-24..29](../../research/country-formation.md)) on our grid; the pipeline is CF-45..51. Runs once at world creation, a pure function of (grid, fields, land, deposits, people, params, seed). `zs::gen::CountryParams`: `fragmentation` in [0, 1], default 0.5; `periods`; the calibrated constants below.

19. **Core value** y of a land cell (CF-45): food capacity (rule 7) × cereal share (arable / (arable + pasture), 0 when both are 0, CF-5) × 1.5 on a large river, 1.2 on a river × 1.2 on a coast (a sea neighbour) × (1 + c), with c the share of the cells within two frequency-24 spacings (about 600 km) whose capacity is below a tenth of its own (sea counts: circumscription, CF-1). Ice and cells with no capacity have y = 0.
20. **Terrain** Θ·x of a land cell (CF-25, CF-26): ruggedness r = the mean |Δh| to its land neighbours, heat h = max(0, T − 24 °C), cold k = max(0, 2 °C − T); Θ·x = θ × max(r / r₉₀, h / h₉₀, k / k₉₀), each divided by its 90th percentile over the land cells where it is above 0, and θ = 2 at fragmentation 0.5 (two equal sides fighting on the 90th-percentile cell stalemate two times in three, CF-26).
21. **Start** (CF-47): every land cell whose capacity density is at least 25 people per km² is its own polity (WNN-32's 50,000 people over CF-24's 2,000 km² cell, estimate); the rest is unruled.
22. **One period**, cells in ascending id, each ruled cell c that borders another owner:
    - it has a conflict with probability α·y_c, α = 1 / the largest y (CF-25);
    - the opponent cell d is drawn among c's neighbours of another owner, and the land cells one sea cell away (a crossing of at most one sea cell, about CF-26's 330 km), weighted by y_d, sea crossings × 0.1 (CF-26);
    - c's polity i takes d with probability Y_i / (Y_i + Y_j) / (1 + Θ·x_d), with Y the sum of y over a polity's cells (an unruled d counts its own y_d); a sea crossing brings 0.33 of Y_i (σ, CF-26); otherwise nothing changes;
    - then each border cell of a polity with B border cells secedes with probability β · Θ·x · B (CF-27) and becomes its own polity;
    - a polity no longer connected (by land or a one-sea-cell crossing) splits; the part holding its capital keeps the polity, each other part is a new one.
    A polity's capital is the cell it started from; when lost, its highest-y cell. Each annexation and secession is a `HistoryEvent` (period, kind, cell, winner, loser): the chronicle of world systems rule 2.
23. **Fragmentation** (CF-48) sets β = β₀ · 10^(2(f − 0.5)) and θ = 2 · 2^(2(f − 0.5)) together; the middle is calibrated so the default planet (seed 1, frequency 24, about 89,000 km² per cell) ends near Earth's count of states of at least one cell's area: 110 UN members are 89,000 km² or larger, 88 are 180,000 km² or larger, 29 are 1 million km² or larger (CF-57). Distance does not damp power in this slice (WSF-30 has no sourced constant; default taken, Open 4). **Resolution** (added 2026-10-09): the parameters are set for a frequency-24 cell of spacing s₀; on a grid of spacing s the history runs s₀ / s times as many rounds (a round moves one cell, so it moves less land), secession uses the border's length in s₀ units (B · s / s₀), and rule 19's circumscription ring is the cells within 2 s₀ km, so counts of large countries and the shares of the largest do not depend on the grid.
24. **Present-day fill** (CF-49): ice belongs to no country (as Antarctica); other unruled land joins the country that reaches it cheapest by multi-source shortest paths over land and one-sea-cell crossings (step cost of spec 48 rule 3: distance × (1 + |Δh| / 500 m), a crossing 4 × its distance; times CF-46's biome factor of the cell entered: desert 3, tundra and ice 5, tropical forest 2, others 1); a cell whose cheapest cost is above 5,000 (km-equivalent, estimate: about five temperate steps of a frequency-24 cell) or that no country reaches stays out, and each connected group of such cells becomes its own country.
25. **Sectors** are cut inside each country, spec 48 rule 3 per country: count = max(1, round(country area / 3 million km²)), seeds its most populous cells, spaced; a sector never crosses a border. A country's `capital` sector holds its capital cell.
26. **Wealth** (CF-51, CF-52): endowment = the mean of six world ranks scaled to [0, 1], per person: food capacity, water capacity, energy (coal, oil and gas as primary energy: 24 GJ/t, 6.1 GJ/bbl, 0.038 GJ/m³), metal (iron, copper and aluminum oxide content), wood, and fish (each sea cell's catch shared equally by the countries with land beside it); access = 1 with a coast, 0.6 landlocked (CF-30); history = exp(0.25 z) with z a keyed normal, × (1 + 0.25 × the share of the periods its capital was held). Development = endowment × access × history; income groups are filled from it as in spec 48 rule 5. No resource curse by default (CF-52).
27. The country records its cells, area, capital cell, founding period, core Y, coast and landlocked flags, and its events.

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

## Tests (`engine/gen/tests/test_world_countries.cpp`, slice 4)

- Every land cell except ice is in exactly one country; each country is connected by land or a one-sea-cell crossing; every sector lies in one country. (A1)
- Same seed and parameters give the same countries; golden hash of the owners pinned on MSVC. (A1)
- On the default planet (seed 1): 60-160 countries (Earth: 110 of at least one cell's area, CF-57); countries of two cells or more within 0.5-1.5 times 88 scaled by the count; the ten largest hold 35-65 % of the land (CF-35: about half). Seeds 2 and 3 are reported. (A2)
- Metamorphic: fragmentation 0.9 gives more countries than 0.1 on the same planet. (A1)
- A desert river cell has a higher core value than the desert cells beside it. (A1)
- Income groups still hold WPP's shares within 10 points (spec 48's check). (A2)

## As built (slice 1)

- `engine/gen/include/zs/gen/world_land.h`, `src/world_land.cpp`: `LandParams`, `WorldLand` (`runoff_mm`, `downstream`, `discharge_m3s`, `catchment_km2`, `river`, `lake`, `fertility`, `arable_share`, `pasture_share`, `capacity_food`, `capacity_water`, `capacity()`), `generate_land`.
- `zs gen world` prints the land summary and draws rivers (light blue) and large rivers (bright blue) on its `--bmp` map; the text map marks large rivers `~`.
- `engine/gen/tests/test_world_land.cpp`: 5 cases; golden pinned on MSVC.
- **Measured** (seed 1, frequency 24, default sliders): runoff 31,019 km³ a year, 34.2 % of the land's 90,762 km³ of rain (RV-1: 35 %; PET calibrated by a sweep, RV-5); largest river 39,168 m³/s (the Amazon carries about 200,000: the coarse grid and the generated continents make smaller basins); 610 river cells and 95 large-river cells of 1,673 land cells; 87 lake cells; arable land 17.6 % of land (Earth's cropland is about 11 %, WNC-3, and more could be cropped); capacity 16.9 billion people, 2.1 times today's population scaled to the planet's land.

- **Slice 2** (living resources, 2026-10-09): `WorldLand` gains `forest_share`, `wood_m3`, `wood_r`, `game_kg`, `fish_catch_t`, `fish_k_t`; they are the capacities spec 37's stocks will take (not yet wired). **Measured** (seed 1): forest 29.8 % of land (WSE-1: 31 %), wood 536 billion m³ (RV-6: 557 on Earth's land), wild herbivores 21 Mt (RV-10: 47 Mt of wild mammals on land and sea), sustainable fish catch 111 Mt a year (RV-11: 90-110). The first estimates gave forest 25 %, game 42 Mt and fish 442 Mt (the coarse shelf is wide); the tables were scaled once to these.

- **Slice 3** (deposits, 2026-10-09): `engine/gen/include/zs/gen/world_deposits.h`, `src/world_deposits.cpp`: `Resource`, the setting flags, `cell_settings`, `Deposit` (cell, kind, amount, grade, settings, distance to the boundary), `WorldDeposits` (`total`, `metal`), `generate_deposits`; `zs gen world` prints the counts and totals; `test_world_deposits.cpp`, 3 cases, golden pinned on MSVC. **Measured** (seed 1): 739 deposits; coal 3,045 billion t (2.8 times today's reserves), oil 2,503 billion bbl (1.5 times), gas 299 trillion m³ (1.4 times), iron ore 401 billion t with 205 billion t of iron (reserves 180, resources over 900), copper 2.0 billion t of metal in 466 billion t of ore, bauxite 45 billion t (reserves 23-30, resources 55-75). The medians were set once from a probe at median 1 (totals are linear in the median); coal offshore was dropped after the test caught it, and its median doubled to keep the total. Iron is rare (30 cells): few cells of this coarse grid lie 8 or more cells from a plate boundary.

- **Slice 4** (countries, 2026-10-09): `engine/gen/include/zs/gen/world_countries.h`, `src/world_countries.cpp`: `LandGraph` (land links and one-sea-cell crossings), `CountryParams`, `HistoryEvent`, `CountryShape`, `WorldCountries` (core, terrain, `country_of`, the shapes, the events), `form_countries`. `populate_world` (spec 48) now takes the land and the deposits, forms the countries, cuts sectors inside them and fills income groups from rule 26; `Country` gains `capital_cell`, `area_km2`, `endowment`, `landlocked`, and `WorldPeople::formed` keeps the history. Spec 49's profiles: a country takes its own group's when one is left, else the nearest group's (100-odd countries can outrun the 24 low-income profiles). `zs gen world` prints the countries and `--bmp FILE --political` draws one colour per country with borders. `test_world_countries.cpp`: 5 cases plus a skipped calibration probe; golden pinned on MSVC. **Calibration:** a sweep of β₀ (10⁻⁴, 3·10⁻⁴, 10⁻³) and periods (100, 200, 400) on seeds 1-3: more periods consolidate (seed 2 with 400 periods ends with 23 countries, the ten largest holding 99 %); β₀ = 3·10⁻⁴ with 100 periods is the closest to Earth. The first run also showed the polar ice and tundra filled into one empty 15-million-km² country: ice is now no one's (rule 24) and the fill limit went from 1,500 to 5,000 (one tundra step of a frequency-24 cell costs about 1,500). **Measured** (seed 1): 1,193 start polities; 3,931 annexations, 247 secessions, 152 splits, 109 cells filled; 108 countries (Earth 110 of at least one cell), 72 of two cells or more (88), 31 over 1 million km² (29); the ten largest hold 51 % of the land held (about half); 51 landlocked; 58 % of border edges touch a river cell (Open 3). Seed 2: 92, 65, 26, 65 %; seed 3: 110, 54, 30, 57 %. Too many one-cell countries (36 against Earth's 22) is the gap left; 47 % landlocked looks high, but Earth's share is not yet checked (CF Open 2). Fragmentation 0.1 gives 56 countries, 0.9 gives 344. Income groups (seed 1): high 16.2 % of people (WPP 15.6), upper-middle 41.6 % (34.9), lower-middle 32.4 % (40.4), low 9.8 % (9.1). The one world's block now stands in a town of 24,383 in a lower-middle-income country (spec 48's flood put it in an upper-middle one). **Recalibrated 2026-10-09** with spec 31's revised terrain and the resolution rule above: the land changed (more, more irregular landmasses), so a sweep of β₀ (3·10⁻⁴ to 2·10⁻³) and periods (40, 60, 100) on seeds 1-3 picked β₀ = 3·10⁻⁴ with 60 periods. Seed 1: 124 countries, 76 of two cells or more, 36 over 1 million km², the ten largest holding 42 %; seed 2: 150, 84, 25, 61 %; seed 3: 143, 80, 28, 54 % (Earth 110, 88, 29, about half). Countries over 1 million km² now agree between frequency 24 and 64 (seeds 1-3: 26/35, 21/21, 25/23 in a probe before the last terrain round); the total count at 64 is higher because a finer grid holds smaller countries.

## Not in this spec (slice 1)

Inland seas and endorheic basins (Open 1); seasons and floods; groundwater; irrigation; erosion and soil degradation over time (0059 rule 5, in the simulation, not the generator); slices 2-4.

## Open

1. Endorheic basins (the Caspian, the Great Basin) need evaporation over the lake surface to balance inflow; the slice drains everything to the sea.
2. RV-5's PET and the biome tables of rules 5-6 are estimates, checked only through the totals the tests bound.
3. River borders (CF-23: 15-35 % of borders): a border at cell resolution cannot run along a river inside a cell; slice 4 reports the share of border edges touching a river cell, with no target.
4. Power damped by distance from the capital (WSF-30) and tech spreading from a frontier (CF-9): off in slice 4; add if the contest over-unifies (CF-29).
5. The country card and the start-type finders (CF-53, CF-54) are the next slice.
