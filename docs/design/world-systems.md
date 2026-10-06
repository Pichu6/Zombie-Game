# World systems

Everything a generated world is made of, how the parts depend on each other, in what order a starting world is built so that **everything placed follows logic**, how factions hold and change territory, and how a decision at state level reaches pops and buildings and comes back as the state's capability. This is the map; each system gets its own spec before its code ([CLAUDE.md](../../CLAUDE.md)).

Intent **Decided** ([0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md), [0035](../decisions/0035-realistic-world-numbers-and-the-pop-nation-loop.md), [0018](../decisions/0018-sandbox-answers.md)): nations exist when a world starts, placed in logical places, with their cities, sizes, supply chains and grids; the user sets the starting parameters; factions hold areas of control and can expand, shrink, be subjugated or conquer; the loop runs macro → micro → macro, state → pop → state; the simulation must cover economy, flora, fauna, resources, processing, building and the rest. The model below is a **Proposal**. Quantitative laws: [world numbers](world-numbers.md). Research for the new systems is in `docs/research/world-systems-*.md` ([economy](../research/world-systems-economy.md), [ecology](../research/world-systems-ecology.md), [resources and networks](../research/world-systems-resources-networks.md), [factions](../research/world-systems-factions.md)); section 10 summarises it, section 11 lists the licences. **Evidence caveat as in [world numbers](world-numbers.md):** every number is a second-hand search summary (the data sites could not be opened), several are low confidence, and parameters marked *design* are ours and unsourced.

## 1. The contract that makes it logical

1. **A thing exists only because of its inputs.** A city exists where land, water and surplus allow it; a steel mill where ore, fuel or power, workers and a route allow it; a grid link where there is load to serve. A generator step reads only what earlier steps made.
2. **Every placement records why** (a cause chain: which fields and which earlier placements). The inspector can answer "why is this here?", the same field the ledger uses for transfers ([macro layer](macro-layer.md) §5).
3. **Conservation:** people, goods, money, fuel and resource stocks move between owners; nothing is created. A deposit's stock is `endowment − extracted`; an item's mass comes from its inputs ([0013](../decisions/0013-production-ladder.md)).
4. **Macro numbers are sums over micro entities** ([pop continuity](pop-continuity.md) §3): a sector's output is the sum of its establishments' output; its workers are rosters; its stocks are stocks. Nothing at state level is stored apart from what lies beneath it.
5. **Settings change numbers, never rules** (Decided). The user's starting parameters feed the same laws.
6. **A nation never has a stat that is not a head count, a stock or a flow of something real.**

## 2. What the world is made of: coverage map

| Domain | What it is | Designed already | Missing |
| --- | --- | --- | --- |
| Terrain and climate | Elevation, sea, rivers, rain, temperature, biome | World fields and generator pipeline ([world simulation](world-simulation.md) §12) | The generator itself |
| Geology and soil | Deposits (ores, fuels, stone, salt), aquifers, soil fertility | Soil as a field | Deposit generation, stocks, grades |
| Flora | Forests, grassland, crops, fuelwood, timber | Agriculture and food chain at one survivor's scale ([world survival](world-survival.md)) | Region-scale stocks, growth, harvest, depletion |
| Fauna | Wildlife, livestock, fisheries, pests, reservoirs | Wildlife and hunting locally; zombies ([threats](threats.md)) | Region-scale populations, fisheries, livestock, zoonoses |
| Pops | People: bodies, ages, education, jobs, wealth | [Spec 28](../tier0/specs/28-pop-levels.md), [0034](../decisions/0034-nation-demographics.md), [world numbers](world-numbers.md) §3 | Household formation, wealth transmission |
| Settlements | Cities and towns, sizes, spacing | Laws in [world numbers](world-numbers.md) §2.3; town and pop generators | The world-scale settlement generator |
| Economy | Production, jobs, trade, money, prices, shortages | Stock and flow per region, scarcest-input rule ([world simulation](world-simulation.md) §5) | Sectors, input-output structure, markets, trade flows, money |
| Processing chains | Extraction → refining → manufacturing → distribution | Production ladder and quality at item scale ([items and crafting](items-crafting.md)) | Chain model at region scale, plant siting |
| Building and construction | Housing, workplaces, plants, public works; building projects | Assemblies and Projects ([world simulation](world-simulation.md) §7) | Building stock per settlement, construction cost and time, upkeep |
| Networks | Power, water, roads, rail, ports, telecom, pipelines | Power and water inside a building and block ([infrastructure](infrastructure.md)) | Regional and national topology, sizing, losses, failure cascades |
| Energy and materials | Flows by sector and per person | Anchors in [world numbers](world-numbers.md) §2.2 | Balance per sector, fuel trade |
| Factions and territory | Who controls what, borders, claims | Country actor, standing, assembly ([macro layer](macro-layer.md)) | Sectors, control and occupation, expansion, subjection, conquest |
| Culture, religion, language | How groups differ | [Sandbox parameters](sandbox-parameters.md) §6 | Generation and spread |
| Technology and capability | What a nation can do | Readout of what exists ([0029](../decisions/0029-leader-records-foxhole-tech-readout.md), [0030](../decisions/0030-pop-continuity-and-capability-loss.md)) | Capability list and its links to yield, cost and output |
| Military and war | Forces, supply, resolution | [Sandbox parameters](sandbox-parameters.md) §5.1; [war research](../research/war-in-other-games.md) | Secondary for now |
| Hazards | Outbreak, disasters | [Threats](threats.md); disasters in [sandbox parameters](sandbox-parameters.md) §4 | Hooks into the coverage above |
| Information | News, rumours, fog | [World simulation](world-simulation.md) §9 | Later |

## 3. How the systems depend on each other

```
 terrain, climate ─► soil, flora, fauna ─► food, fuelwood, timber ─┐
        │                                                           ├─► carrying capacity K ─► pops ─► workers, taxpayers, recruits
 geology ─► deposits ─► extraction ─► processing ─► manufacturing ──┤                                  │
        │                    ▲            ▲              ▲          │                                  ▼
        └─► water         energy ◄── fuel, hydro, sun   workers ────┘            settlements ◄── suitability, routes
                               │                                                         │
                         power grid, water works, roads, rail, ports  ◄── construction (workers, materials, time)
                               │                                                         │
                               └──────────► economy: output, trade, money ◄──────────────┘
                                                        │
                       factions: revenue, army, officials, reach, stability ──► policy ──► back to pops, buildings, networks
```

Each arrow is a rate with a realistic band ([world numbers](world-numbers.md) §6). Loops close in three places: pops → output → state → services and security → pops; pops → demand → processing → extraction → deposit stock; state → construction → networks and buildings → output.

## 4. Generating a world that follows logic

Each step reads only earlier outputs, draws with keyed randomness (so a seed reproduces the world), and writes a cause record for everything it places. The result is saved with the world, so a generator update never changes a save ([world simulation](world-simulation.md) §12). The user's starting parameters (section 7) shape every step.

| # | Step | Reads | Writes (and why it is there) |
| --- | --- | --- | --- |
| 1 | Planet and terrain | size, land share, climate sliders | elevation, sea level, rivers, rain, temperature, biome |
| 2 | Soil, flora, fauna, water | step 1 | soil fertility, potential vegetation, wildlife and fish potential, renewable water per cell |
| 3 | Geology and deposits | terrain history (plates, basins, arcs, shields), step 1 | deposits with grade and stock by simple geological rules |
| 4 | Capacity K per cell | steps 1–3, start era | food, water, fuel and timber ceilings ([world numbers](world-numbers.md) §2.2) |
| 5 | Population and settlements | K, suitability, coast, rivers | densities, town sites, sizes by rank-size, urban share; each with its cause |
| 6 | Routes | settlements, terrain, transport cost by mode | rivers, sea lanes, roads, ports by cost-distance (spanning tree, then loops by flow) |
| 7 | Sectors | settlements, routes | control units: cell clusters around a settlement and its hinterland |
| 8 | Factions and borders | sectors, circumscription, surplus, reach | states seeded in circumscribed high-surplus regions and grown by cost-distance from capitals until reach, competition or geography stops them; count and sizes follow the laws ([world numbers](world-numbers.md) §4.1); cultures assigned with drift |
| 9 | Economy and chains | pops, deposits, flora, fauna, energy, routes | extraction sites at deposits, processing and factories where inputs, workers, power and an outlet meet, farms and fisheries by capacity; sized to demand and supply |
| 10 | Networks | settlements, plants, load per capita | power plants by fuel, hydro, sun and coast; grid links following the route graph and load; water works by source; telecom by density |
| 11 | Building stock | pops, jobs, step 9 | housing per household, workplaces per job, public buildings by service level; interiors are generated on approach ([world simulation](world-simulation.md) §12) |
| 12 | Pops | profiles ([0034](../decisions/0034-nation-demographics.md)), steps 5–11 | people with ages, education, jobs filled from workplace rosters, households in housing |
| 13 | Relations | factions, borders | standing, treaties, claims, and starting subject relations the user allows |
| 14 | Audit | everything | checks (below) and the realism report |

**Audit invariants (a world that fails is not shipped):** people add up; every employed pop fills a workplace slot and no slot exceeds capacity; every settlement is connected; every plant has its inputs or a declared shortage; every grid node has a source or a declared blackout; every supply chain closes or states its import; deposit stocks plus extracted equal the endowment; every faction's sectors are contiguous or connected by a declared route; every placement has a cause record.

### 4.1 Sectors and zones: the spatial hierarchy (Decided, [0037](../decisions/0037-sectors-zones-and-world-creation-answers.md); model Proposal)

The world is divided on purpose, so that borders, placement and play all use the same units, as when starting a city in Cities: Skylines.

| Level | What it is | What it is used for | Detail |
| --- | --- | --- | --- |
| World cell | A patch of the planet's surface (an equal-area cell of the global grid) carrying the fields of steps 1–4 | Terrain, climate, soil, flora, fauna, deposits, capacity K | The whole planet, coarse |
| **Sector** | A connected group of world cells around a settlement and its hinterland; **dozens at the start of a world, designed to scale to hundreds** | The control unit: owner, controller, claims, borders (a nation's territory is a set of sectors); the target of macro orders | Whole planet; a sector holds pops, deposits, buildings and route nodes |
| **Zone** | A land-use area inside a sector: residential, commercial, industrial, farmland, forest, reserve, park, extraction, and so on | Where cities and buildings are **placed**: zoning decides what may be built where, and demand (Cities: Skylines style) fills it | Exists only where people build: settlement footprints and worked land; wilderness stays at world-cell level |
| Plot | A building lot inside a zone, on the 0.5 m build grid | One building, its interior generated on approach ([world simulation](world-simulation.md) §8, §12) | Local scale |

- **Borders follow sectors:** two nations meet where their sectors meet, and a border never cuts a sector. Moving a border is moving a sector's owner.
- **Zoning is how a city grows:** a zone sets what may go up; pops and companies build it as jobs and material demand are answered ([world simulation](world-simulation.md) §7); the building stock is the sum of what is built.
- **The generator fills zones** where the user has not: the settlement step (5) draws a footprint, step 11 zones and fills it.
- **Sectors are what the earlier docs call a sector:** the named area macro orders aim at ([world simulation](world-simulation.md) §13).
- **Layer mapping:** Macro sees sectors and borders; City sees zones and plots; Individual sees plots and interiors ([macro layer](macro-layer.md) §3.3).
- **Size and grid** (an equal-area global grid, sector sizes, zone resolution) are decisions for the first generator spec.

## 5. Factions and control

A **faction** is a state actor ([macro layer](macro-layer.md) §7) that holds sectors. The control model, in Hearts of Iron and Stellaris terms:

- **Sector:** the control unit (a Hearts of Iron province), a settlement and its hinterland (cells, pops, deposits, buildings, route nodes). It has an **owner**, a **controller** (differs under occupation), a list of **claims**, and a development and unrest state. Sectors are the unit macro orders aim at ([world simulation](world-simulation.md) §13); zooming in opens the layers below.
- **Territory** is the set of owned sectors; a **border** is where owners meet; **reach** (message speed × tolerated delay) limits what a centre can hold without a delegated tier ([world numbers](world-numbers.md) §4.1).
- **Relations:** standing with reasons, treaties, alliances, rivalry, as in [macro layer](macro-layer.md).
- **Subjection:** a faction can be a **subject** of another (vassal, protectorate, tributary, puppet), with its own government, an autonomy level, a tribute share of revenue and an obligation (levies, access, trade terms). Subjects can rebel, be absorbed, or be released. Lowest-autonomy subjection is the reading of "become slaves" for factions ([0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md)).
- **Expansion:** by settling empty or thin land when pop pressure ([world numbers](world-numbers.md) §2.2) exceeds capacity and routes allow; by diplomacy (subjection, union); by conquest.
- **Conquest:** war resolved in aggregate from forces, supply and terrain, with its cost priced in pops, wages and revenue ([sandbox parameters](sandbox-parameters.md) §5.1). Occupation brings unrest and a time to integrate; the occupied pops keep their identity ([pop continuity](pop-continuity.md)) and change owner by a roster event, never by being recreated.
- **Shrinking:** loss in war, secession when legitimacy falls or reach is exceeded, collapse to a lower tier ([world numbers](world-numbers.md) §4.1), always with the same pops and buildings.
- **AI factions use the same actions as the player** (parity, [vision](../vision.md)): orders, edicts, projects, treaties; the AI only chooses.

Rules, rates and costs for subjection, conquest and integration are Proposals in section 10.4; none is decided here.

## 6. State → pop → state, system by system

The full audit of every jump between layers, with its status and proposed rule, is in [layer jumps](layer-jumps.md) ([0039](../decisions/0039-fill-the-layer-jumps.md)); this table is the macro-to-pop slice of it.

The coupling contract ([spec 28](../tier0/specs/28-pop-levels.md), [pop continuity](pop-continuity.md)): a macro order becomes a command; the command becomes events on rosters and stocks (region events, workplaces, projects); micro entities act under their own rules (pops, establishments); their results add up into the macro numbers; every transfer carries the reason, back to the order.

| Domain | Macro state (a sum) | Down: what an order does | Up: what the micro result adds up to |
| --- | --- | --- | --- |
| Economy | Sector output, jobs, shortages | An edict (tax, rationing, price rule, conscription) changes establishments' staffing, inputs and hours | Output is the sum of establishments' production; revenue follows |
| Processing | Chain throughput, stocks | A build order or priority starts a plant project; allocation changes input shares | Throughput is each plant's output given its inputs, workers and power |
| Construction | Building stock, projects | A programme posts jobs and material demand that pops and companies answer | Stock is the sum of finished buildings; capability readout rises |
| Networks | Capacity, load, outages | A build or maintenance order, a load-shedding rule | Capacity is the sum of sound components; an outage is a failed component |
| Resources, flora, fauna | Stock, harvest rate, depletion | A licence, quota or ban changes who may extract | Stock falls by what is actually taken; regrowth follows the laws |
| Pops | Counts by age, work, wealth, loyalty | Region events on rosters ([spec 28](../tier0/specs/28-pop-levels.md)) | Counts are sums over pops, never stored apart |
| Faction | Revenue, army, officials, stability, reach | Policy and orders | Capability is drawn from pops and stocks ([world numbers](world-numbers.md) §4.2) |
| Territory | Owned sectors, borders, subjects | Treaty, war, secession | Ownership changes by event; pops and buildings keep their identities |

## 7. What the user sets at creation

Presets, auto-fill or custom for every field, as already decided ([sandbox parameters](sandbox-parameters.md) §1.3), each with realistic and sandbox bands. Groups:

| Group | Examples |
| --- | --- |
| World | size, land share, climate, relief, water, resource richness, flora and fauna richness |
| Civilisation | number of factions and how unequal in size, urbanisation, development level and how it varies between factions, population density, inequality |
| Politics | government types and their shares, how many factions start as subjects, aggression, stability, how many are in conflict |
| Economy | trade openness, industrial share, resource scarcity, energy mix, infrastructure level |
| Culture | number of cultures and how mixed |
| Per faction | override anything above for one faction (as in Stellaris empire creation; the screen and the inheritance of culture and beliefs are in [macro layer](macro-layer.md) §6.5, [0026](../decisions/0026-country-creation-inheritance.md)), or paint territory on the generated map |
| Outbreak and hazards | per [sandbox parameters](sandbox-parameters.md) §12 |
| Time | per [0032](../decisions/0032-real-time-reference-and-generational-runs.md) |

The user can **edit the generated map before starting** (Decided, [0037](../decisions/0037-sectors-zones-and-world-creation-answers.md)), as when starting a new city in Cities: Skylines: paint **nation borders** over sectors, paint **zones** inside a sector and place cities and buildings with a brush ([0016](../decisions/0016-sandbox-setup-and-layer-flow.md)), move or rename a faction, or leave any part to the generator. An edited world is tagged and **re-audited** (section 4): an edit that breaks an invariant (a city with no water, a plant with no inputs) is refused or flagged with the reason.

## 8. Specs to write, and the order

Dependencies only; nothing here blocks the pop work already built. After [world numbers](world-numbers.md) §12:

1. **World fields and terrain generator** (steps 1–2), then **geology and deposits** (3).
2. **Capacity and settlements** (4–5), then **routes** (6).
3. **Sectors and the faction territory model** (7–8), with the control, claim and subject records.
4. **Economy core** (sectors, establishments as rosters of workers, stocks, jobs) and **processing chains** (9).
5. **Networks** (10) and **building stock and construction** (11).
6. **Pops from the world** (12) and **relations** (13), then the **audit and realism report** (14).
7. **Faction dynamics:** expansion, subjection, conquest, shrink; **war in aggregate**, last.

A first playable slice for the loop: a **single continent of a dozen or so of the dozens of sectors, and three to five factions**, one resource chain (ore → steel → tools), one grid, one trade route graph, run through the yearly step with the loop closed. It tests the contract before the full world.

## 9. Questions

### Answered (2026-10-06, [0037](../decisions/0037-sectors-zones-and-world-creation-answers.md))

1. **Forced labour:** a possibility for later, not in the plan; it needs its own decision before any design.
2. **Granularity:** dozens of sectors, designed to scale.
3. **Map shape and realism:** worlds are realistic to physics and math, not necessarily 1:1 with Earth.
4. **Uneven development at the start:** yes, read from the capabilities each faction holds.
5. **Map editing before the run:** yes, with the world divided into sectors (borders) and zones (placement), re-audited after edits.

### Answered (2026-10-06, [0038](../decisions/0038-grid-sector-zone-sizes-accepted.md))

- **Global grid:** equal-area cells of about 5,000 km² at Earth size (about 100,000 cells); a flat metric grid only for local maps.
- **Sectors and zones:** a sector is a few hundred to a few thousand cells (dozens per world); zone cells are 100 to 500 m, inside settlement footprints only.
- **Real Earth:** later.

### Open

1. **Research honesty.** The notes are second-hand search summaries; a real data pipeline ([world numbers](world-numbers.md) §10) is the way to firm numbers up.
2. **The first generator spec** (global grid, terrain and climate fields, sectors, zones) is the next unit to write; see section 8.

## 10. Laws and anchors for the new systems

Each is a **Proposal** with a realistic band and a wider sandbox band; the research notes hold the full equations, sources and confidence. "Design" means we chose it.

### 10.1 Economy (sector level)

| Law | Form | Anchors |
| --- | --- | --- |
| Production | `Y = A K^a L^(1−a) × min(1, energy available ÷ needed)`; `A` read from the capabilities the sector has | Capital share 0.30–0.45 (textbook band, design here) |
| Structure | Sector shares move with log income (agriculture falls, industry humps, services rise) | Low income about 59% of jobs and 27% of GDP in agriculture; rich about 75% of jobs in services |
| Input and output | Leontief closure over six sectors (agriculture, mining and energy, manufacturing, construction, services, government) | Manufacturing about 66% intermediate inputs, whole economy about 45%; other coefficients not found, to come from WIOD or OECD tables |
| Capital | `K' = (1−d)K + sY`; steady state `K/Y = s/(g+d)` | Investment 25.9% of world GDP; K/Y 3–4; depreciation buildings 1.5–2.3%, machinery 11% |
| Demand | Food share falls with income, housing flat | Food about 13% of EU budgets against 40–55% in poor countries; housing 24–27% |
| Prices and shelves | Damped adjustment `p' = p(1 + k·clamp((D−S)/S))` with stocks; a shop is empty when stock plus supply is below demand at the posted price | The equation is design; only the qualitative price-ceiling result is sourced |
| Trade | Gravity `Y_i Y_j / d^δ`, δ 0.7–1.0, rescaled to an openness target | Openness 25–38% for large economies, 83–400% for small; services about 25% of trade |
| Process recipes | Inputs, energy and labour per tonne | Steel 10.2 GJ/t (scrap-EAF) to 24.0 (blast furnace); clinker 3.5 GJ/t plus 0.1 MWh/t cement; refinery own use 3–8%; milling 60–100 kWh/t. Labour per tonne not found: derive from labour share of value added |
| Money | Inflation and credit bands by regime | Credit to GDP 0.2–1.5; inflation 0–4% stable, up to 13.5% (US 1980) |

### 10.2 Flora, fauna, soil and sea

| Law | Form | Anchors |
| --- | --- | --- |
| Stocks | Logistic growth with harvest, `dB/dt = rB(1−B/K) − H`, for wood, wild plants, grass | Plantation growth 3–25 m³/ha/yr; fuelwood 0.4 m³ per person a year (Africa 0.67); 7–29 m³ of wood per house |
| Wood balance | Demand against sustainable supply; stress above 1 drains the stock | Net forest loss about 5 Mha a year |
| Recovery | Exponential approach, about 29-year time constant | Secondary forest 122 Mg/ha at 20 years; median 66 years to 90% of old-growth biomass |
| Soil | Fertility regenerates under fallow and is used by crops; erosion by cover | Fallow 7–10 years (about 20 for full recovery); tolerable erosion about 1 t/ha/yr, ploughed fields 10–100 times more; fertiliser feeds 48% of people |
| Animal density | Damuth, density ∝ body mass^−0.75, scaled to edible production at about 10% per trophic step | 10,000 kg of prey supports about 90 kg of predator; transfer 2–24% |
| Hunting and fishing | Same logistic; sustainable yield rK/4; warn below 0.3 K; collapse flag at catch under 10% of peak | Fish per person 9.1 kg (1961) to 20.7 kg (2022); 37.7% of stocks overfished; aquaculture 51% of output |
| Livestock | Animal units from forage; competes with crops | 77% of farmland for 18% of calories; about 6–7 ha per cow on dry range |
| Pests and blight | A stochastic yield loss | 17–30% of crop yield |
| Zoonosis | `hazard = base × wild contact × livestock density` | About 60% of new pathogens come from animals |
| Forage | kcal per hour of hunting or gathering falls as local density falls | About 830 kcal/h hunting, 850–1,200 kcal/h starchy plants |

### 10.3 Resources, processing and networks

| Law | Form | Anchors |
| --- | --- | --- |
| Where deposits go | Geological setting decides what a cell can hold: shields (iron, nickel sulphide, gold), arcs and orogens (porphyry copper, gold), sedimentary basins (coal, oil, gas, salt, phosphate), warm wet ultramafic lowland (laterite nickel, bauxite), rivers and coasts (placers), recharge gravels (aquifers) | Banded iron on shields over 60% of iron reserves; about 70% of nickel is laterite; under 40 supergiant oil fields, two thirds in one basin. The table is a generator rule; only some associations are sourced |
| Deposit size and grade | Lognormal tonnage and grade, Pareto tail for fluids | Porphyry copper median 250 Mt at 0.39% copper; spreads not found (design 1.0–1.8) |
| Reserves | Deposits whose cost per tonne of metal is under the price; ore per tonne of metal = 100/grade %; the cutoff grade falls with price and technology (Lasky) | Copper cutoff about 0.20–0.25% |
| Processing | Energy and inputs per tonne (above); plants sited by input: smelting near cheap firm power, cement near limestone, steel near coal, iron and a port, concentrators near the mine | Aluminium 13–15 MWh/t; alumina about 10 GJ/t; 2–2.5 t bauxite per t alumina |
| Power | Demand from income; capacity from `energy ÷ (8760 × capacity factor)`; mix by what exists (hydro head and flow, basin fuel, wind and sun cells, nuclear only with capability); plants sized from real distributions; transmission as a minimum spanning tree plus loops, distribution as radial trees | World grid loss 8.3% (range 2–19%); capacity factors solar 12–25%, nuclear about 74%; mean plant sizes coal 380 MW, gas combined cycle 500–820 MW, nuclear about 900 MW; high-income electricity use 67 times low |
| Water | Demand `pop × per-capita × (1 + non-revenue water)`; intake, treatment, storage, mains tree with loops only in the core | Non-revenue water 30–35% (10–60%); safely managed water 74% globally (2024) |
| Transport | Paved road per capita by income, capped by area; rail only where freight or density justify it; spanning tree plus loops until detour ratio is low, then hubs | Paved road per million people 170 / 1,660 / 10,110 km (low, middle, high income); rail density 0.02–0.07 km per km² |
| Telecom and pipelines | Tower spacing from density; a pipeline from field to nearest refinery, port or load by minimum cost | Radius 1–2 km urban to 10–32 km sparse; pipeline rule is design |

Every placement keeps a reason string ("aluminium smelter beside the cheapest firm power"), as the pipeline requires.

### 10.4 Factions, subjects and conquest

| Law | Form | Anchors |
| --- | --- | --- |
| Control unit | A sector holds pops, buildings and resources with owner, controller, claimants and integration in 0–1; faction totals are sums weighted by integration and control | Victoria 3 states and Crusader Kings 3 counties; CK3 loses 1% tax and 0.5% levy per missing point of control |
| Reach | Projected power decays with distance from the supplied base, `exp(−d/L)`; roads, rail and ships raise `L` | Decay length not found: set so a pre-industrial state projects half power at 1–2 weeks of march |
| Battle | Strength `N × quality`; effective ratio with terrain, fortification, supply, morale; losses between linear and square law (exponent about 1.5, design); a side breaks on morale or cut supply before annihilation | History fits neither law well; terrain examples from Hearts of Iron |
| Siege | The shorter of starvation and assault; a relieving army becomes a battle | Most strongholds that fell did so within about two weeks |
| War score | Value of enemy sectors occupied plus battle wins; exhaustion rises until a side accepts peace | Constants not found |
| Occupation | Resistance falls and compliance grows with time; yield `base × compliance`; garrison drains the occupier's own manpower; integration 5–30 years for taxes and levies, 100–150 years culturally | Gaul about 100–150 years (low confidence); a failure case if locals are excluded (Dacia) |
| Subject contract | One structure: tribute share, levy share, autonomy, protection; presets tributary, vassal, puppet, protectorate | CK3 levy shares 16.6 / 25 / 33.3 / 50%; Stellaris tributaries at least 30%; Ming tribute a net transfer to the subject. Real revenue shares mostly not found: game numbers only |
| Loyalty | Liberty desire from ethnic distance, burdens, protection and overlord strength sets the yearly chance of rebellion; absorb when indirect rule costs more than direct | Assyria annexed vassals into sectors (743–732 BC) |
| Expansion | Settle when local pops exceed capacity and a neighbour has room; conquer when value over cost beats a threshold; bounded land raises the cost of leaving | Carneiro |
| Secession and collapse | Yearly chance rises with unrest, falls with control and garrison; collapse when pay cannot be met or legitimacy is low | Empire lifetime about 215 years (a newspaper summary of Turchin); Europe about 500 states (1500) to 25–30 (1900); half of the 1816 states gone; civil wars start about 2.3 a year since 1945 |
| Diplomacy | Balance the strongest neighbouring threat; opinion as dated modifiers with decay; no abstract diplomacy currency | Walt's balance of threat |

War resolution stays secondary ([0031](../decisions/0031-restoring-capabilities-real-time-playable-age.md)), but the laws above price it in pops, wages and revenue.

## 11. Data licences for these systems

Reported, from search summaries; confirm on each publisher's page before shipping any table.

- **Usable (public domain or CC BY):** USGS (public domain), Natural Earth (public domain), Global Power Plant Database (CC BY), FAOSTAT (CC BY 4.0), TRY plant traits (CC BY 4.0), GBIF (filter to CC0 or CC BY), World Bank, Penn World Table, WIOD, OECD (content from July 2024) and EU KLEMS (all reported CC BY 4.0).
- **Not usable:** Seshat (CC BY-NC-SA), Sea Around Us (CC BY-NC-SA, use FAO capture data instead), Correlates of War (custom terms), Fragile States Index (CC BY-NC-SA).
- **Needs a decision:** OpenStreetMap (ODbL, share-alike; the repo already restricts it to the home region as a separate database, [world simulation](world-simulation.md) §3).
- **Unchecked or restricted:** IEA, Energy Institute, worldsteel, Eurostat, HYDE's licence (reported CC BY 4.0, not confirmed in the factions note).
