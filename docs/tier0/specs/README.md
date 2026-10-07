# Tier 0 unit specs

One page per unit, written before its code ([contract rule 10](../../design/world-simulation.md#2-the-simulation-contract-proposal)). Everything here is a **Proposal** until the user confirms it or code and tests make it **Implemented** / **Validated**. Numbers are starting values to tune; each one names its source.

The order of work is in [the Tier 0 plan](../plan.md).

## Template

Each spec has: **Purpose**, **State** (field, unit, conserved?, owner), **Rules**, **Invariants**, **Tests** (each with its agreement level), **Calibration** (value, source) and **Not in Tier 0**.

Agreement levels ([contract rule 11](../../design/world-simulation.md#2-the-simulation-contract-proposal)):

- **A1 identical:** bit-identical numbers for the same binary, data, seed and inputs.
- **A2 distribution:** same distribution over N ≥ 50 seeded runs (90 % intervals).
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
| 31 | [Terrain and climate fields](31-terrain-climate.md) | Fields (elevation, climate, biome) | after Tier 0; generator steps 1-2 (Proposal, not built) |
