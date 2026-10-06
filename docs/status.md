# Status

Updated: 2026-10-06.

## What we're building

A realistic simulation of a world and the people in it: **Macro** (nations and their sectors), **City** (zones, buildings, supply) and **Individual** (one pop, played), one simulation under all three ([vision](vision.md), [0012](decisions/0012-world-simulation-layers.md)). It runs with or without an outbreak ([0023](decisions/0023-no-outbreak-game-and-parameter-catalogue.md)): survival, or a politics, grand-strategy and Sims-like game. Worlds start complete, with nations, cities, supply chains and grids placed by logic ([0032](decisions/0032-worlds-start-with-nations-and-everything-follows-logic.md)), on realistic numbers ([0031](decisions/0031-realistic-world-numbers-and-the-pop-nation-loop.md)).

**Tier 0, "The generator problem"** ([0010](decisions/0010-concept-v2.md)): the 2D survival prototype, SDL3 + ImGui, no Vulkan; a 2-floor building plus 2 houses, a body model in kg with ~17 regions, power/noise/light, zombies from a census of ~60 neighbours, goal: survive 14 days.

## Where we are

**Built and tested, headless** (GCC and Clang, identical golden hashes; run `ctest --preset debug`):

| Part | What | Try it |
| --- | --- | --- |
| Tier 0 stages 1-4 | foundations, things and places, the body, actions and items | `--map`, `--body-lab`, `--bench`, `--scripted-pop` |
| Tier 0 stages 6-9 | power, noise, light; the dead; the group; the 14-day run | `--stage6`, `--stage7`, `--group`, `--run` |
| Content tools | seeded names, pops and towns, six culture packs | `zs_gen town` |
| Pop levels ([spec 28](tier0/specs/28-pop-levels.md)) | one pop at every layer: ids, random-access lives, births, cohorts and rosters, region-wide events, versioned saves, a world system with commands | `--pop-bench`, `--town` |
| Nation demographics ([0030](decisions/0030-nation-demographics.md)) | a country's own age, education, work, households as data (placeholder profile only) | `data/setup/nations/` |

**Stage 5, the sandbox window:** built, not yet run on a PC. **Windows/MSVC:** CI builds `main` and uploads a zip; the pop work lives on the working branch and has not been through MSVC yet.

**Designed, not built** (all Proposals unless a decision says otherwise): [sandbox parameters](design/sandbox-parameters.md), [world numbers](design/world-numbers.md), [world systems](design/world-systems.md) (sectors and zones, the generation pipeline, factions, subjects), built from research notes in `docs/research/` whose numbers are second-hand search summaries.

**Decisions** run up to [0034](decisions/0034-grid-sector-zone-sizes-accepted.md): equal-area global grid (~5,000 km² cells), dozens of sectors, zones of 100-500 m, uneven development at the start, map editing before the run, real Earth later.

## Next step

1. **Write the first generator spec** (spec 29: global grid, terrain and climate fields, sectors, zones; tests with agreement levels), then build it.
2. Then, in order ([world systems](design/world-systems.md) section 8): geology and deposits, capacity and settlements, routes, sectors and faction territory, economy core and chains, networks, building stock, the audit and realism report.
3. Tier 0: play the sandbox from the Windows zip; then stage 10 (kill or keep: real player runs).

## Waiting on the user

- **Real data:** a data tool that fetches licensed datasets and writes sourced presets, and a first real country (or global tables). Today every profile is a placeholder.
- **Tier 0 census and pops:** make the block's 60 neighbours the same pops as the registry's founders? It changes Tier 0's pinned golden hashes.
- **Merging the working branch** into `main` (35+ commits ahead): not done; needs your go-ahead.
- Forced-labour populations stay out of the plan unless you decide otherwise ([0033](decisions/0033-sectors-zones-and-world-creation-answers.md)).
