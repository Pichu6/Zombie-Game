# World systems

Everything a generated world is made of, how the parts depend on each other, in what order a starting world is built so that **everything placed follows logic**, how factions hold and change territory, and how a decision at state level reaches pops and buildings and comes back as the state's capability. This is the map; each system gets its own spec before its code ([CLAUDE.md](../../CLAUDE.md)).

Intent **Decided** ([0032](../decisions/0032-worlds-start-with-nations-and-everything-follows-logic.md), [0031](../decisions/0031-realistic-world-numbers-and-the-pop-nation-loop.md), [0018](../decisions/0018-sandbox-answers.md)): nations exist when a world starts, placed in logical places, with their cities, sizes, supply chains and grids; the user sets the starting parameters; factions hold areas of control and can expand, shrink, be subjugated or conquer; the loop runs macro → micro → macro, state → pop → state; the simulation must cover economy, flora, fauna, resources, processing, building and the rest. The model below is a **Proposal**. Quantitative laws: [world numbers](world-numbers.md). Research for the new systems is being written to `docs/research/world-systems-*.md` (economy, ecology, resources and networks, factions); until it is in, every number for those systems is open.

## 1. The contract that makes it logical

1. **A thing exists only because of its inputs.** A city exists where land, water and surplus allow it; a steel mill where ore, fuel or power, workers and a route allow it; a grid link where there is load to serve. A generator step reads only what earlier steps made.
2. **Every placement records why** (a cause chain: which fields and which earlier placements). The inspector can answer "why is this here?", the same field the ledger uses for transfers ([macro layer](macro-layer.md) §5).
3. **Conservation:** people, goods, money, fuel and resource stocks move between owners; nothing is created. A deposit's stock is `endowment − extracted`; an item's mass comes from its inputs ([0013](../decisions/0013-production-ladder.md)).
4. **Macro numbers are sums over micro entities** ([pop continuity](pop-continuity.md) §3): a province's output is the sum of its establishments' output; its workers are rosters; its stocks are stocks. Nothing at state level is stored apart from what lies beneath it.
5. **Settings change numbers, never rules** (Decided). The user's starting parameters feed the same laws.
6. **A nation never has a stat that is not a head count, a stock or a flow of something real.**

## 2. What the world is made of: coverage map

| Domain | What it is | Designed already | Missing |
| --- | --- | --- | --- |
| Terrain and climate | Elevation, sea, rivers, rain, temperature, biome | World fields and generator pipeline ([world simulation](world-simulation.md) §12) | The generator itself |
| Geology and soil | Deposits (ores, fuels, stone, salt), aquifers, soil fertility | Soil as a field | Deposit generation, stocks, grades |
| Flora | Forests, grassland, crops, fuelwood, timber | Agriculture and food chain at one survivor's scale ([world survival](world-survival.md)) | Region-scale stocks, growth, harvest, depletion |
| Fauna | Wildlife, livestock, fisheries, pests, reservoirs | Wildlife and hunting locally; zombies ([threats](threats.md)) | Region-scale populations, fisheries, livestock, zoonoses |
| Pops | People: bodies, ages, education, jobs, wealth | [Spec 28](../tier0/specs/28-pop-levels.md), [0030](../decisions/0030-nation-demographics.md), [world numbers](world-numbers.md) §3 | Household formation, wealth transmission |
| Settlements | Cities and towns, sizes, spacing | Laws in [world numbers](world-numbers.md) §2.3; town and pop generators | The world-scale settlement generator |
| Economy | Production, jobs, trade, money, prices, shortages | Stock and flow per region, scarcest-input rule ([world simulation](world-simulation.md) §5) | Sectors, input-output structure, markets, trade flows, money |
| Processing chains | Extraction → refining → manufacturing → distribution | Production ladder and quality at item scale ([items and crafting](items-crafting.md)) | Chain model at region scale, plant siting |
| Building and construction | Housing, workplaces, plants, public works; building projects | Assemblies and Projects ([world simulation](world-simulation.md) §7) | Building stock per settlement, construction cost and time, upkeep |
| Networks | Power, water, roads, rail, ports, telecom, pipelines | Power and water inside a building and block ([infrastructure](infrastructure.md)) | Regional and national topology, sizing, losses, failure cascades |
| Energy and materials | Flows by sector and per person | Anchors in [world numbers](world-numbers.md) §2.2 | Balance per province, fuel trade |
| Factions and territory | Who controls what, borders, claims | Country actor, standing, assembly ([macro layer](macro-layer.md)) | Provinces, control and occupation, expansion, subjection, conquest |
| Culture, religion, language | How groups differ | [Sandbox parameters](sandbox-parameters.md) §6 | Generation and spread |
| Technology and capability | What a nation can do | Readout of what exists ([0025](../decisions/0025-leader-records-foxhole-tech-readout.md), [0026](../decisions/0026-pop-continuity-and-capability-loss.md)) | Capability list and its links to yield, cost and output |
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
| 7 | Provinces | settlements, routes | control units: cell clusters around a settlement and its hinterland |
| 8 | Factions and borders | provinces, circumscription, surplus, reach | states seeded in circumscribed high-surplus regions and grown by cost-distance from capitals until reach, competition or geography stops them; count and sizes follow the laws ([world numbers](world-numbers.md) §4.1); cultures assigned with drift |
| 9 | Economy and chains | pops, deposits, flora, fauna, energy, routes | extraction sites at deposits, processing and factories where inputs, workers, power and an outlet meet, farms and fisheries by capacity; sized to demand and supply |
| 10 | Networks | settlements, plants, load per capita | power plants by fuel, hydro, sun and coast; grid links following the route graph and load; water works by source; telecom by density |
| 11 | Building stock | pops, jobs, step 9 | housing per household, workplaces per job, public buildings by service level; interiors are generated on approach ([world simulation](world-simulation.md) §12) |
| 12 | Pops | profiles ([0030](../decisions/0030-nation-demographics.md)), steps 5–11 | people with ages, education, jobs filled from workplace rosters, households in housing |
| 13 | Relations | factions, borders | standing, treaties, claims, and starting subject relations the user allows |
| 14 | Audit | everything | checks (below) and the realism report |

**Audit invariants (a world that fails is not shipped):** people add up; every employed pop fills a workplace slot and no slot exceeds capacity; every settlement is connected; every plant has its inputs or a declared shortage; every grid node has a source or a declared blackout; every supply chain closes or states its import; deposit stocks plus extracted equal the endowment; every faction's provinces are contiguous or connected by a declared route; every placement has a cause record.

## 5. Factions and control

A **faction** is a state actor ([macro layer](macro-layer.md) §7) that holds provinces. The control model, in Hearts of Iron and Stellaris terms:

- **Province:** the control unit, a settlement and its hinterland (cells, pops, deposits, buildings, route nodes). It has an **owner**, a **controller** (differs under occupation), a list of **claims**, and a development and unrest state. Provinces are the unit macro orders aim at ([world simulation](world-simulation.md) §13); zooming in opens the layers below.
- **Territory** is the set of owned provinces; a **border** is where owners meet; **reach** (message speed × tolerated delay) limits what a centre can hold without a delegated tier ([world numbers](world-numbers.md) §4.1).
- **Relations:** standing with reasons, treaties, alliances, rivalry, as in [macro layer](macro-layer.md).
- **Subjection:** a faction can be a **subject** of another (vassal, protectorate, tributary, puppet), with its own government, an autonomy level, a tribute share of revenue and an obligation (levies, access, trade terms). Subjects can rebel, be absorbed, or be released. Lowest-autonomy subjection is the reading of "become slaves" for factions ([0032](../decisions/0032-worlds-start-with-nations-and-everything-follows-logic.md)).
- **Expansion:** by settling empty or thin land when pop pressure ([world numbers](world-numbers.md) §2.2) exceeds capacity and routes allow; by diplomacy (subjection, union); by conquest.
- **Conquest:** war resolved in aggregate from forces, supply and terrain, with its cost priced in pops, wages and revenue ([sandbox parameters](sandbox-parameters.md) §5.1). Occupation brings unrest and a time to integrate; the occupied pops keep their identity ([pop continuity](pop-continuity.md)) and change owner by a roster event, never by being recreated.
- **Shrinking:** loss in war, secession when legitimacy falls or reach is exceeded, collapse to a lower tier ([world numbers](world-numbers.md) §4.1), always with the same pops and buildings.
- **AI factions use the same actions as the player** (parity, [vision](../vision.md)): orders, edicts, projects, treaties; the AI only chooses.

Rules, rates and costs for subjection, conquest and integration wait for the faction research note; none is decided here.

## 6. State → pop → state, system by system

The coupling contract ([spec 28](../tier0/specs/28-pop-levels.md), [pop continuity](pop-continuity.md)): a macro order becomes a command; the command becomes events on rosters and stocks (region events, workplaces, projects); micro entities act under their own rules (pops, establishments); their results add up into the macro numbers; every transfer carries the reason, back to the order.

| Domain | Macro state (a sum) | Down: what an order does | Up: what the micro result adds up to |
| --- | --- | --- | --- |
| Economy | Province output, jobs, shortages | An edict (tax, rationing, price rule, conscription) changes establishments' staffing, inputs and hours | Output is the sum of establishments' production; revenue follows |
| Processing | Chain throughput, stocks | A build order or priority starts a plant project; allocation changes input shares | Throughput is each plant's output given its inputs, workers and power |
| Construction | Building stock, projects | A programme posts jobs and material demand that pops and companies answer | Stock is the sum of finished buildings; capability readout rises |
| Networks | Capacity, load, outages | A build or maintenance order, a load-shedding rule | Capacity is the sum of sound components; an outage is a failed component |
| Resources, flora, fauna | Stock, harvest rate, depletion | A licence, quota or ban changes who may extract | Stock falls by what is actually taken; regrowth follows the laws |
| Pops | Counts by age, work, wealth, loyalty | Region events on rosters ([spec 28](../tier0/specs/28-pop-levels.md)) | Counts are sums over pops, never stored apart |
| Faction | Revenue, army, officials, stability, reach | Policy and orders | Capability is drawn from pops and stocks ([world numbers](world-numbers.md) §4.2) |
| Territory | Owned provinces, borders, subjects | Treaty, war, secession | Ownership changes by event; pops and buildings keep their identities |

## 7. What the user sets at creation

Presets, auto-fill or custom for every field, as already decided ([sandbox parameters](sandbox-parameters.md) §1.3), each with realistic and sandbox bands. Groups:

| Group | Examples |
| --- | --- |
| World | size, land share, climate, relief, water, resource richness, flora and fauna richness |
| Civilisation | number of factions and how unequal in size, urbanisation, development level and how it varies between factions, population density, inequality |
| Politics | government types and their shares, how many factions start as subjects, aggression, stability, how many are in conflict |
| Economy | trade openness, industrial share, resource scarcity, energy mix, infrastructure level |
| Culture | number of cultures and how mixed |
| Per faction | override anything above for one faction (as in Stellaris empire creation), or paint territory on the generated map |
| Outbreak and hazards | per [sandbox parameters](sandbox-parameters.md) §12 |
| Time | per [0028](../decisions/0028-real-time-reference-and-generational-runs.md) |

The user may also **edit the generated map** (move a border, rename, place a faction) before starting; edited worlds are tagged and re-audited.

## 8. Specs to write, and the order

Dependencies only; nothing here blocks the pop work already built. After [world numbers](world-numbers.md) §12:

1. **World fields and terrain generator** (steps 1–2), then **geology and deposits** (3).
2. **Capacity and settlements** (4–5), then **routes** (6).
3. **Provinces and the faction territory model** (7–8), with the control, claim and subject records.
4. **Economy core** (sectors, establishments as rosters of workers, stocks, jobs) and **processing chains** (9).
5. **Networks** (10) and **building stock and construction** (11).
6. **Pops from the world** (12) and **relations** (13), then the **audit and realism report** (14).
7. **Faction dynamics:** expansion, subjection, conquest, shrink; **war in aggregate**, last.

A first playable slice for the loop: a **single continent of about a dozen provinces and three to five factions**, one resource chain (ore → steel → tools), one grid, one trade route graph, run through the yearly step with the loop closed. It tests the contract before the full world.

## 9. Open questions

1. **"Slaves."** Read as subjugation of a faction (subject statuses, section 5). Does the game also model **enslaved or forced-labour populations** inside a state? It is a sensitive design area, a fact of history that some games model; it is not assumed here and needs your explicit decision.
2. **Province size and count.** A world of 100–300 provinces (Hearts of Iron scale) or a few dozen (Stellaris sectors)? Recommendation: pick by the start slice (a dozen), design for hundreds, as the pop levels already scale.
3. **Map shape.** One generated continent, several, or the real Earth (the default is Earth, [0028](../decisions/0028-real-time-reference-and-generational-runs.md))? Earth is a separate data pipeline; generated worlds use the laws above.
4. **Unequal development at start.** Real nations differ widely. Recommendation: yes, development (hence yield, tax ceiling, transport cost) varies by faction as the capabilities its provinces actually hold.
5. **Hand-editing the map** before the run (section 7). Recommendation: yes, with a re-audit and a tag on the save.
6. **Research honesty.** The existing notes are second-hand search summaries; the new ones will be too. A real data pipeline ([world numbers](world-numbers.md) §10) is the way to firm numbers up.
