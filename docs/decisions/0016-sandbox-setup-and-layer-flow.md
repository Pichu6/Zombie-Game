# 0016 · Sandbox world options, virus editor, generated settlements, layer flow and possession

- **Date:** 2026-10-06
- **Status:** Decided by the user (intent below). Models in [world simulation](../design/world-simulation.md) sections 12–13 are Proposals.
- **Extends:** [0012](0012-world-simulation-layers.md). Layer numbering is unchanged: 1st = Individual, 2nd = City, 3rd = Macro.

## Decisions

- **World options in setup:** natural resources, temperature, climate, flora, fauna, soil type and fertility, biome and population.
- **Virus editor, Plague Inc style:** the player customises the outbreak (traits, symptoms) to test how deadly it is, or whether the simulated pops contain or cure it. There is an option for no virus.
- **Generated worlds** (sandbox; real Earth stays the default): pops gather where conditions are best, and towns, cities, roads, commerce and trade follow from that, as in real life.
- **City building from prefab kits** ("Lego"): a brush or spawner places enterable structures procedurally, with culture styles (European, Japanese, Indian and others).
- **Macro (3rd layer):** watch the simulation Plague Inc style and take some decisions (Stellaris, Hearts of Iron, OGame), aimed at a place, e.g. a power plant in sector 3 or a blockade in sector 4, with its stats.
- **City (2nd layer):** zooming into that sector shows it in Manor Lords / Cities: Skylines / Infection Free Zone style. Decisions here are management (buildings, stores, supply chains, grid), not laws or edicts. Clicking the plant or blockade shows the resources and pops it needs; pops gather, build and staff it on their own.
- **From building to body:** click a building, pick a pop working there, see their info, and play as them.
- **Individual (1st layer):** isometric (Project Zomboid, Alien Swarm, Thunder Tier One) or over the shoulder (Ghost Recon Breakpoint, The Last of Us).
- **Any combination:** only watch, or decide in any layer, play one layer or all three, and see the consequences of decisions from above.
- **Possession gives objectives:** a possessed pop's current tasks and routine (building, shopping, going home or to work) become optional objectives, so there is always something to do.

## Why

The user wants the sandbox to be a laboratory: build a world and a virus, then see whether people survive it, at any scale.

Research: `/mnt/project-files/research/global-sim/loop/22a`–`23` (see [research/global-sim.md](../research/global-sim.md)).
