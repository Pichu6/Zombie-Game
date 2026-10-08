# Research notes

Dated notes that feed decisions and designs. **They are not decisions** ([CLAUDE.md](../../CLAUDE.md)). Most were written from web-search summaries because the proxy blocks many sites; each note says so and which numbers to re-check.

## How a note is written ([0054](../decisions/0054-research-notes-findings-and-links.md))

Run `python3 tools/docs/check_research.py` before pushing; CI runs it too.

1. **Header**, right under the title: `**Date:**` (YYYY-MM-DD), `**Status:**`, `**Feeds:**` (links to the design docs, specs or decisions it serves, or `background`), `**Evidence:**` (how it was gathered and what was not read), `**IDs:**` (the note's prefix, 2-4 capital letters, unique in this folder).
2. **`## Findings`**: one table, one row per finding. IDs are never reused or renumbered; a wrong finding stays, marked wrong, and a new row replaces it.
3. **Confidence**, one of four words: `sourced` (the primary source was read), `reported` (search summaries or secondary sources), `derived` (our arithmetic; name the findings it comes from), `estimate` (our judgement).
4. **Cite by ID** elsewhere: `[PZ-3](../research/project-zomboid.md)`.
5. **Links both ways**: a design doc in `Feeds` links back to the note, ideally in a `Research` section.
6. **Numbers the simulation uses** go to `data/` with their sources; the note points to the file.
7. **Open questions** are carried to the Open section of the design doc they concern; the note may keep the list.
8. **Index**: add the note to its topic below.

```markdown
# <Title>

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [body](../design/body.md), [spec 21](../tier0/specs/21-infection.md). **Evidence:** web-search summaries; the wiki was blocked. **IDs:** PZ.

<Why this note exists, in two lines.>

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| PZ-1 | <one claim, with its number and unit> | reported | <URL or reference> |

## <Explanation, tables, what we take and what we don't>

## Sources
```

## Index

Grouped by topic. Each note's header says which design docs, specs and decisions it feeds.

### Vision

| Note | Title | Date |
| --- | --- | --- |
| [concept-pichu-original-vision.md](concept-pichu-original-vision.md) | Pichu's original vision (2026-10-05, verbatim, lightly cleaned) | 2026-10-05 |
| [concept-v2.md](concept-v2.md) | Juego Zombie · Concept v2 | 2026-10-05 |

### Simulation engine: detail, time, threads, storage

| Note | Title | Date |
| --- | --- | --- |
| [detail-levels.md](detail-levels.md) | Detail levels: one simulation, read and run in chunks | 2026-10-06 |
| [catch-up-jump-forms.md](catch-up-jump-forms.md) | Catch-up: jump forms for drawn and competing rules | 2026-10-07 |
| [layer-jumps-multiresolution.md](layer-jumps-multiresolution.md) | Layer jumps: what multi-resolution simulation research and games say | 2026-10-06 |
| [threading-and-partition.md](threading-and-partition.md) | Threading and partition model | 2026-10-07 |
| [pop-simulation-and-persistence.md](pop-simulation-and-persistence.md) | Pop simulation, lenses per zoom, and saving the world | 2026-10-07 |
| [pop-storage-and-lenses.md](pop-storage-and-lenses.md) | Pop simulation, saving data, and "lenses" across zoom | 2026-10-07 |
| [sim-scale-in-other-games.md](sim-scale-in-other-games.md) | Simulation scale in other games | 2026-10-06 |
| [low-resource-city-simulation.md](low-resource-city-simulation.md) | Low-resource city simulation: "ASCII City" and the games that fake or tame big populations | 2026-10-07 |

### World: generation, numbers and systems

| Note | Title | Date |
| --- | --- | --- |
| [world-generation-prior-art.md](world-generation-prior-art.md) | World generation: prior art | 2026-10-07 |
| [world-numbers-capacity.md](world-numbers-capacity.md) | World numbers: how many people can a world hold? | 2026-10-06 |
| [world-numbers-demography.md](world-numbers-demography.md) | World numbers: demography, structure, education, labour, inequality, production | 2026-10-06 |
| [world-numbers-nations.md](world-numbers-nations.md) | World numbers for nations: how they arise, how big they are, what they can do | 2026-10-06 |
| [world-numbers-settlement.md](world-numbers-settlement.md) | World numbers: how people spread over land, and how towns and cities arise | 2026-10-06 |
| [world-systems-ecology.md](world-systems-ecology.md) | World systems: flora, fauna, soils and fisheries in numbers | 2026-10-06 |
| [world-systems-economy.md](world-systems-economy.md) | World systems: how a real economy works in numbers | 2026-10-06 |
| [world-systems-factions.md](world-systems-factions.md) | World systems for factions: territory, subjects, expansion, conquest, collapse | 2026-10-06 |
| [world-systems-resources-networks.md](world-systems-resources-networks.md) | World systems: where resources are, and how grids and networks are laid out | 2026-10-06 |

### People and bodies

| Note | Title | Date |
| --- | --- | --- |
| [human-generator-data.md](human-generator-data.md) | The human generator: data, licences and the generation pipeline | 2026-10-07 |
| [clothing-wrap.md](clothing-wrap.md) | Clothing that follows the body, in our own engine | 2026-10-05 |
| [second-order-dynamics.md](second-order-dynamics.md) | Second-order dynamics for procedural motion | 2026-10-06 |

### Outbreak and war

| Note | Title | Date |
| --- | --- | --- |
| [global-sim.md](global-sim.md) | Global outbreak simulation research | 2026-10-05 |
| [war-in-other-games.md](war-in-other-games.md) | War in other games, as a reference | 2026-10-06 |
| [foxhole.md](foxhole.md) | Foxhole, as a reference for war and logistics | 2026-10-06 |

### Reference games

| Note | Title | Date |
| --- | --- | --- |
| [reference-games-by-layer.md](reference-games-by-layer.md) | Reference games by layer: which mechanic, from which game, for which layer | 2026-10-08 |
| [project-zomboid.md](project-zomboid.md) | Project Zomboid, as a reference for the Individual layer | 2026-10-08 |
| [haven-and-hearth.md](haven-and-hearth.md) | Haven and Hearth: a reference game for long real-time play | 2026-10-07 |
| [gta.md](gta.md) | GTA, as a reference for the Individual layer | 2026-10-08 |
| [shenmue.md](shenmue.md) | Shenmue, as a reference for the Individual layer | 2026-10-08 |
| [the-sims.md](the-sims.md) | The Sims, as a reference for the Individual layer | 2026-10-08 |
| [inzoi.md](inzoi.md) | inZOI, as a reference for the Individual layer | 2026-10-08 |
| [second-life.md](second-life.md) | Second Life, as a reference for ownership | 2026-10-08 |
| [simcity.md](simcity.md) | SimCity, as a reference for the City layer | 2026-10-08 |
| [cities-skylines.md](cities-skylines.md) | Cities: Skylines, as a reference for the City layer | 2026-10-08 |
| [world-in-conflict.md](world-in-conflict.md) | World in Conflict, as a reference for command | 2026-10-08 |
| [stellaris.md](stellaris.md) | Stellaris, as a reference for the Macro layer | 2026-10-08 |
| [hearts-of-iron.md](hearts-of-iron.md) | Hearts of Iron IV, as a reference for the Macro layer | 2026-10-08 |
| [teg.md](teg.md) | TEG, as a reference for the Macro layer | 2026-10-08 |

### Engine and rendering

| Note | Title | Date |
| --- | --- | --- |
| [renderer.md](renderer.md) | The renderer, at engineering level | 2026-10-08 |
| [engine-stack.md](engine-stack.md) | The engine stack behind decision 0002: licences, versions, determinism, alternatives | 2026-10-08 |
| [view-readability.md](view-readability.md) | Readability of the 2D top-down view: state, selection, alerts, overlays, and what our window lacks | 2026-10-08 |

### The old prototype

| Note | Title | Date |
| --- | --- | --- |
| [prototype-audit.md](prototype-audit.md) | Docs and agent tooling: what to keep for the new project | 2026-10-05 |
| [prototype-sim-code.md](prototype-sim-code.md) | Prototype simulation code: what to keep, what to avoid | 2026-10-05 |

## Reference games: where each is covered

Games we study and what the notes say about them. Add a row when a note covers a new game. The overview by layer is [reference-games-by-layer](reference-games-by-layer.md).

| Game | What we look at | Notes |
| --- | --- | --- |
| Haven and Hearth | Real durations, timed actions, progress bars, quality softcaps, long-wait play | [haven-and-hearth](haven-and-hearth.md) |
| Dwarf Fortress | World generation fields, tracked individuals versus abstract population, where the simulation cost goes | [sim-scale](sim-scale-in-other-games.md), [world-generation-prior-art](world-generation-prior-art.md), [low-resource-city-simulation](low-resource-city-simulation.md) |
| Cities: Skylines 1 and 2 | Citizen, unit and agent split; traffic agents; population scale; district policies, travelling services, death waves | [cities-skylines](cities-skylines.md), [sim-scale](sim-scale-in-other-games.md), [low-resource-city-simulation](low-resource-city-simulation.md) |
| Songs of Syx | Tens of thousands of individually simulated citizens | [sim-scale](sim-scale-in-other-games.md), [low-resource-city-simulation](low-resource-city-simulation.md) |
| Project Zomboid | Zombie counts per cell, virtual outside the loaded area; moodles, health panel, infection by wound, skills by use | [project-zomboid](project-zomboid.md), [sim-scale](sim-scale-in-other-games.md) |
| Cataclysm: DDA | Abstract overmap, detail generated on discovery | [world-generation-prior-art](world-generation-prior-art.md) |
| WorldEngine, mapgen4, Songs of the Eons | Terrain and climate generators | [world-generation-prior-art](world-generation-prior-art.md) |
| Kingdom Come: Deliverance 2, Assassin's Creed Unity, Hitman | AI level of detail, pools that swap cheap and real agents, thousands of NPCs with routines | [pop-storage-and-lenses](pop-storage-and-lenses.md) |
| Factorio, Minecraft, No Man's Sky | Save size, chunked storage, seeds instead of stored worlds | [pop-storage-and-lenses](pop-storage-and-lenses.md) |
| Foxhole | Logistics, war, player-run supply | [foxhole](foxhole.md) |
| GTA | Wanted level from witnesses, ambient population, side activities, the phone | [gta](gta.md) |
| Shenmue | People and shops on schedules, weather from records, asking around, jobs and pastimes | [shenmue](shenmue.md) |
| The Sims | Motives, smart objects and advertisements, autonomy | [the-sims](the-sims.md), [sim-scale](sim-scale-in-other-games.md) |
| inZOI | Karma from one person to a city, reading a character's thoughts | [inzoi](inzoi.md) |
| Second Life | Item permissions (next owner), creator kept, regions and parcels | [second-life](second-life.md) |
| SimCity | Zoning and demand, coverage, the 2013 agents without homes | [simcity](simcity.md) |
| World in Conflict | Reinforcements from a pool, tactical aid, command points, roles | [world-in-conflict](world-in-conflict.md) |
| Stellaris | Empire creation, tech as weighted cards, factions from pops, governors | [stellaris](stellaris.md), [sim-scale](sim-scale-in-other-games.md) |
| Hearts of Iron IV | Supply over rail and hubs, conscription laws, battle plans | [hearts-of-iron](hearts-of-iron.md) |
| TEG | Secret objectives, pacts, a simple map of 50 countries | [teg](teg.md) |
| Many others (RimWorld, CK3, Victoria 3, Stellaris, Bannerlord, Kenshi, The Sims, Football Manager) | How many full records a simulation holds, and what is aggregated | [sim-scale](sim-scale-in-other-games.md), [war-in-other-games](war-in-other-games.md) |
