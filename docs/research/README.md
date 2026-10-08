# Research notes

Dated notes that feed decisions and designs. **They are not decisions** ([CLAUDE.md](../../CLAUDE.md)). Most were written from web-search summaries because the proxy blocks many sites; each note says so and which numbers to re-check.

## Index

| Note | Title | Date |
| --- | --- | --- |
| [clothing-wrap.md](clothing-wrap.md) | Clothing that follows the body, in our own engine | 2026-10-05 |
| [concept-pichu-original-vision.md](concept-pichu-original-vision.md) | Pichu's original vision (2026-10-05, verbatim, lightly cleaned) | 2026-10-05 |
| [concept-v2.md](concept-v2.md) | Juego Zombie · Concept v2 | 2026-10-05 |
| [detail-levels.md](detail-levels.md) | Detail levels: one simulation, read and run in chunks | 2026-10-06 |
| [foxhole.md](foxhole.md) | Foxhole, as a reference for war and logistics | 2026-10-06 |
| [global-sim.md](global-sim.md) | Global outbreak simulation research | 2026-10-05 |
| [haven-and-hearth.md](haven-and-hearth.md) | Haven and Hearth: a reference game for long real-time play | 2026-10-07 |
| [layer-jumps-multiresolution.md](layer-jumps-multiresolution.md) | Layer jumps: what multi-resolution simulation research and games say | 2026-10-06 |
| [low-resource-city-simulation.md](low-resource-city-simulation.md) | Low-resource city simulation: "ASCII City" and the games that fake or tame big populations | 2026-10-07 |
| [pop-storage-and-lenses.md](pop-storage-and-lenses.md) | Pop simulation, saving data, and "lenses" across zoom | 2026-10-07 |
| [prototype-audit.md](prototype-audit.md) | Docs and agent tooling: what to keep for the new project | 2026-10-05 |
| [prototype-sim-code.md](prototype-sim-code.md) | Prototype simulation code: what to keep, what to avoid | 2026-10-05 |
| [second-order-dynamics.md](second-order-dynamics.md) | Second-order dynamics for procedural motion | 2026-10-06 |
| [sim-scale-in-other-games.md](sim-scale-in-other-games.md) | Simulation scale in other games | 2026-10-06 |
| [war-in-other-games.md](war-in-other-games.md) | War in other games, as a reference | 2026-10-06 |
| [world-generation-prior-art.md](world-generation-prior-art.md) | World generation: prior art | 2026-10-07 |
| [world-numbers-capacity.md](world-numbers-capacity.md) | World numbers: how many people can a world hold? | 2026-10-06 |
| [world-numbers-demography.md](world-numbers-demography.md) | World numbers: demography, structure, education, labour, inequality, production | 2026-10-06 |
| [world-numbers-nations.md](world-numbers-nations.md) | World numbers for nations: how they arise, how big they are, what they can do | 2026-10-06 |
| [world-numbers-settlement.md](world-numbers-settlement.md) | World numbers: how people spread over land, and how towns and cities arise | 2026-10-06 |
| [world-systems-ecology.md](world-systems-ecology.md) | World systems: flora, fauna, soils and fisheries in numbers | 2026-10-06 |
| [world-systems-economy.md](world-systems-economy.md) | World systems: how a real economy works in numbers | 2026-10-06 |
| [world-systems-factions.md](world-systems-factions.md) | World systems for factions: territory, subjects, expansion, conquest, collapse | 2026-10-06 |
| [world-systems-resources-networks.md](world-systems-resources-networks.md) | World systems: where resources are, and how grids and networks are laid out | 2026-10-06 |

## Reference games: where each is covered

Games we study and what the notes say about them. Add a row when a note covers a new game.

| Game | What we look at | Notes |
| --- | --- | --- |
| Haven and Hearth | Real durations, timed actions, progress bars, quality softcaps, long-wait play | [haven-and-hearth](haven-and-hearth.md) |
| Dwarf Fortress | World generation fields, tracked individuals versus abstract population, where the simulation cost goes | [sim-scale](sim-scale-in-other-games.md), [world-generation-prior-art](world-generation-prior-art.md), [low-resource-city-simulation](low-resource-city-simulation.md) |
| Cities: Skylines 1 and 2 | Citizen, unit and agent split; traffic agents; population scale | [sim-scale](sim-scale-in-other-games.md), [low-resource-city-simulation](low-resource-city-simulation.md) |
| Songs of Syx | Tens of thousands of individually simulated citizens | [sim-scale](sim-scale-in-other-games.md), [low-resource-city-simulation](low-resource-city-simulation.md) |
| Project Zomboid | Zombie counts per cell, virtual outside the loaded area | [sim-scale](sim-scale-in-other-games.md) |
| Cataclysm: DDA | Abstract overmap, detail generated on discovery | [world-generation-prior-art](world-generation-prior-art.md) |
| WorldEngine, mapgen4, Songs of the Eons | Terrain and climate generators | [world-generation-prior-art](world-generation-prior-art.md) |
| Kingdom Come: Deliverance 2, Assassin's Creed Unity, Hitman | AI level of detail, pools that swap cheap and real agents, thousands of NPCs with routines | [pop-storage-and-lenses](pop-storage-and-lenses.md) |
| Factorio, Minecraft, No Man's Sky | Save size, chunked storage, seeds instead of stored worlds | [pop-storage-and-lenses](pop-storage-and-lenses.md) |
| Foxhole | Logistics, war, player-run supply | [foxhole](foxhole.md) |
| Many others (RimWorld, CK3, Victoria 3, Stellaris, Bannerlord, Kenshi, The Sims, Football Manager) | How many full records a simulation holds, and what is aggregated | [sim-scale](sim-scale-in-other-games.md), [war-in-other-games](war-in-other-games.md) |
