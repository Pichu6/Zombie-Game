# Tier 0 unit specs

One page per unit, written before its code ([contract rule 10](../../design/world-simulation.md#2-the-simulation-contract-proposal)). Everything here is a **Proposal** until the user confirms it or code and tests make it **Implemented** / **Validated**. Numbers are starting values to tune; each one names its source.

The order of work is in [the Tier 0 plan](../plan.md).

## Template

Each spec has: **Purpose**, **State** (field, unit, conserved?, owner), **Rules**, **Invariants**, **Tests** (each with its agreement level), **Calibration** (value, source) and **Not in Tier 0**.

Agreement levels ([contract rule 11](../../design/world-simulation.md#2-the-simulation-contract-proposal)):

- **A1 identical:** bit-identical numbers for the same binary, data, seed and inputs.
- **A2 distribution:** same distribution over N ≥ 50 seeded runs (90 % intervals). For jump forms, where the cheap side can run thousands of samples, use the stronger form: 50 hot against about 2,000 cold with Anderson–Darling and energy distance ([catch-up-jump-forms.md](../../research/catch-up-jump-forms.md) §5); the 90 % band alone only catches gross errors.
- **A3 relationship:** same direction of effect, or inside a calibration band.

## Units

| # | Spec | Units (world-simulation names) | Plan stage |
| --- | --- | --- | --- |
| 01 | [Quantity](01-quantity.md) | Quantity | 1 |
| 02 | [Draw](02-draw.md) | Draw | 1 |
| 03 | [Stock and transfer](03-stock-transfer.md) | Stock, Transfer | 1 |
| 04 | [Hazard and event](04-hazard-event.md) | Hazard, Event | 1 |
| 05 | [Clock and update order](05-clock.md) | World (time) | 1 |
| 06 | [Save, replay and event log](06-save-replay.md) | World (persistence) | 1 |
| 07 | [Condition](07-condition.md) | Condition | 2 |
| 08 | [Component and assembly](08-assembly.md) | Component, Assembly | 2 |
| 09 | [Site map and room graph](09-site-map.md) | Cell grid, room graph | 2 |
| 10 | [Fortification](10-fortification.md) | Assembly (structural) | 2, 7 |
| 11 | [Body: composition and needs](11-body-composition.md) | Body | 3 |
| 12 | [Body: regions and wounds](12-body-wounds.md) | Body part | 3 |
| 13 | [Capabilities](13-capabilities.md) | Body (readouts) | 3 |
| 14 | [Action executor](14-action-executor.md) | Action | 4 |
| 15 | [Tier 0 items and recipes](15-items-recipes.md) | Component, Assembly, Action | 4 |
| 16 | [2D view](16-view-2d.md) | (reads only) | 5 |
| 17 | [Power network](17-power-network.md) | Network | 6 |
| 18 | [Noise field](18-noise-field.md) | Field | 6 |
| 19 | [Light field](19-light-field.md) | Field | 6 |
| 20 | [Damage resolver](20-damage.md) | (shared damage) | 7 |
| 21 | [Zombie infection](21-infection.md) | Hazard, Event, Body | 7 |
| 22 | [Stub census](22-census.md) | Agent (census form) | 7 |
| 23 | [Walker brain](23-walker-brain.md) | Agent (policy) | 7 |
| 24 | [Pop AI](24-pop-ai.md) | Agent (policy) | 8 |
| 25 | [Chronicle and cause of death](25-chronicle.md) | Event | 7, 9 |
| 26 | [The run: "The generator problem"](26-run.md) | Scenario | 9 |
| 27 | [Detail levels and chunks](27-detail-levels.md) | World (levels, chunks) | 8 rules; Tier 1 split |
| 28 | [Pop identity and levels](28-pop-levels.md) | Agent (identity, levels), Belief store | after Tier 0; benchmark first |
| 29 | [Embodied pops](29-embodied-pops.md) | Agent (embodied level), Body | after Tier 0; first layer jump |
| 30 | [World grid](30-world-grid.md) | Cell grid (planet) | after Tier 0; generator step 1 |
| 31 | [Terrain and climate fields](31-terrain-climate.md) | Fields (elevation, climate, biome) | after Tier 0; generator steps 1-2 (implemented through biome) |
| 32 | [Stocks, goods and money across levels](32-stocks-goods-and-money.md) | Ledger (owner tree, goods, spoilage, money), panels | after Tier 0; layer jumps rows 5 and 6 |
| 33 | [Establishments and chains](33-establishments-and-chains.md) | Recipe, Establishment (staff, power, production jump) | after Tier 0; layer jumps rows 7 and 8 |
| 34 | [Projects](34-projects.md) | Plan, Project (escrow, stages, stalls, finish) | after Tier 0; layer jumps row 9 |
| 35 | [Networks above building scale](35-networks.md) | Grid tree (sources, switches, loads, child grids) | after Tier 0; layer jumps row 10 |
| 36 | [Fields and weather](36-fields-and-weather.md) | Coarsen and refine; weather from climate; ambient noise | after Tier 0; layer jumps rows 11 and 12 |
| 37 | [Flora and fauna](37-flora-and-fauna.md) | Population (logistic stock, roster of individuals, take) | after Tier 0; layer jumps row 13 |
| 38 | [Outbreak across levels](38-outbreak.md) | Encounter rule, measured rates, compartments, entities | after Tier 0; layer jumps row 14 |
| 39 | [Orders and belief](39-orders-and-belief.md) | Edict (levy, tax, staffing), cause trace, Beliefs (message, retell, lens) | after Tier 0; layer jumps rows 15 and 16 |
| 40 | [Time across levels](40-time.md) | World::jump (wake ticks), Pacer (speed ladder, caps, real time) | after Tier 0; layer jumps row 17 |
| 41 | [The one world](41-one-world.md) | make_run as the one world: census rows as registry founders, owners and money, grid tree, orders audience | decision 0052 |
| 42 | [The block's place](42-place.md) | Place, PlaceSystem (weather at the cell, indoor temperature), choose_place, gen::make_world | decision 0052 |
| 43 | [Global tables and base countries](43-global-tables.md) | WPP 2024 tables, fitted mortality, age-specific fertility, base countries | decision 0052 |
| 44 | [Child mortality](44-child-mortality.md) | Siler hazard (early childhood), fitted on qx weighted by deaths | decision 0052 |
| 45 | [Households and sectors](45-households-and-sectors.md) | UN household sizes and ILO employment by sector per income group; four household groups | decision 0052 |
| 46 | [Education, labour, occupations and settlement](46-education-labour-occupations-settlement.md) | WIC2023 education, ILO participation, unemployment and occupations, urban share; base countries fully sourced | decision 0052 |
| 47 | [Towns generated for a country](47-country-towns.md) | the town generator draws households, schooling, work and occupations from a base country | decision 0052 |
| 48 | [People, sectors, countries and towns](48-people-sectors-countries.md) | suitability, population, sectors, countries with income groups, rank-size towns; the block's town and country | decision 0052 |
| 49 | [Real-country profiles under parody names](49-real-country-profiles.md) | 183 real-data profiles, parody names, matched to the planet's countries | decision 0053 |
| 50 | [Movement](50-movement.md) | 8-direction paths, blocking furniture, the `Walk` intent (steering), doors on the way | user playtest 2026-10-08 |
| 51 | [Dwellings](51-dwellings.md) | the dwelling ledger per household, the house plan (corridor, rooms, doors, windows, lot) and its interior from the household | content tools; user request 2026-10-08 |
| 52 | [Land](52-land-water-capacity.md) | runoff, rivers, lakes, fertility, arable land, capacity (slice 1); forests, game, fish, deposits, countries (slices 2-4) | decision 0059 |
| 53 | [The calendar](53-calendar.md) | CivilTime (dates from a start date; the weather reads it), CountryCalendar (weekends, short days, holidays, school days), CalendarSystem | decision 0072; fourth demo flow slice 1 |
| 54 | [Daily schedules](54-schedules.md) | a pop's day as a function of the pop, the day and its country's time-use table: participation then duration, a skeleton drawn once, the template chain, where a pop is at a tick; `zs --day` | decisions 0069, 0070, 0071; fourth demo flow slice 1 |
| 56 | [Probes and the first divergence](56-probes-and-divergence.md) | a hash and a timer per system, never in the state; `zs --diverge` names the first divergent tick and system between a stepped world and a jumped or reloaded one | ranked items 1 and 67 |
| 55 | [The Atlas](55-atlas.md) | the front door of zs.exe: worlds, the tour, cards, the simulation card, the try panel, the compute and breakthrough boards, the workshop reader | decisions 0073, 0074 |
