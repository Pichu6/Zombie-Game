# Status

Updated: 2026-10-06.

## What we're building

A realistic simulation of a world and the people in it: **Macro** (nations and their sectors), **City** (zones, buildings, supply) and **Individual** (one pop, played), one simulation under all three ([vision](vision.md), [0012](decisions/0012-world-simulation-layers.md)). It runs with or without an outbreak ([0027](decisions/0027-no-outbreak-game-and-parameter-catalogue.md)): survival, or a politics, grand-strategy and Sims-like game. Worlds start complete, with nations, cities, supply chains and grids placed by logic ([0036](decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md)), on realistic numbers ([0035](decisions/0035-realistic-world-numbers-and-the-pop-nation-loop.md)).

**Tier 0, "The generator problem"** ([0010](decisions/0010-concept-v2.md)): the 2D survival prototype, SDL3 + ImGui, no Vulkan; a 2-floor building plus 2 houses, a body model in kg with ~17 regions, power/noise/light, zombies from a census of ~60 neighbours, goal: survive 14 days.

## Where we are

**Built and tested, headless** (GCC and Clang, identical golden hashes; run `ctest --preset debug`):

| Part | What | Try it |
| --- | --- | --- |
| Tier 0 stages 1-4 | foundations, things and places, the body, actions and items | `--map`, `--body-lab`, `--bench`, `--scripted-pop` |
| Tier 0 stages 6-9 | power, noise, light; the dead; the group; the 14-day run | `--stage6`, `--stage7`, `--group`, `--run` |
| Content tools | seeded names, pops and towns, six culture packs, lifepath backgrounds built from places that exist in the town (`data/gen/lifepath.toml`) | `zs_gen town` |
| Pop levels ([spec 28](tier0/specs/28-pop-levels.md)) | one pop at every layer: ids, random-access lives, births, cohorts and rosters, region-wide events, versioned saves, a world system with commands | `--pop-bench`, `--town` |
| Nation demographics ([0034](decisions/0034-nation-demographics.md)) | a country's own age, education, work, households as data (placeholder profile only) | `data/setup/nations/` |

**Stage 5, the sandbox window:** built; the user's on-screen check is pending (`play/zs_sandbox.exe` on the PC). **Windows/MSVC:** CI builds `main` and uploads a zip; on 2026-10-07 the user's PC built this `main` (28c5a7e) with MSVC 2022 from a clean tree, release and debug, and `ctest --preset debug` passed with the Linux-pinned golden hashes (Validated: cross-compiler A1).

**Designed, not built** (all Proposals unless a decision says otherwise): [sandbox parameters](design/sandbox-parameters.md), [world numbers](design/world-numbers.md), [world systems](design/world-systems.md) (sectors and zones, the generation pipeline, factions, subjects), built from research notes in `docs/research/` whose numbers are second-hand search summaries.

**Layer jumps** ([0039](decisions/0039-fill-the-layer-jumps.md), Decided intent; [audit](design/layer-jumps.md), Proposal): 22 domains audited for how Macro, City and Individual connect; first gap closed: embodied pops ([spec 29](tier0/specs/29-embodied-pops.md), headless, tested). Next: a generic jump test harness, then stocks and money.

**Pop lore and custom pops** ([0040](decisions/0040-pop-lore-custom-pops-and-player-slots.md), Decided intent; [design](design/pop-lore-and-custom-pops.md), Proposal): a pop's background shapes the world around it without contradicting committed state; authored pops enter at the start or as recruits; a pop can be a player slot.

**Merged on 2026-10-07 from a local session** (its stage 5 and stage 9 code was dropped in favour of this line's; its documents stayed): research on [pop simulation and persistence](research/pop-simulation-and-persistence.md), [catch-up maths](research/catch-up-jump-forms.md), [threading](research/threading-and-partition.md) and [the human generator's data](research/human-generator-data.md); decisions [0041](decisions/0041-threading-and-partition.md)–[0046](decisions/0046-realistic-numbers-from-free-data.md) (threading decided, pop presentation per layer, the City layer loop, information scope per layer, ten open questions, realistic numbers from free data) and [0047](decisions/0047-one-line-of-work.md) (one line of work); the [pop states and actions](design/pop-states-actions.md) catalogue; the [playtest](tier0/playtest.md) template; Proposal rules in specs 02, 04, 05, 06 and 27.

**Decisions** run up to [0047](decisions/0047-one-line-of-work.md); 0038 is the last of the world design ones (0023, 0025, 0026 are engine architecture, the sandbox engine as first game, and country creation; mine start at 0027; 0024 is unused): equal-area global grid (~5,000 km² cells), dozens of sectors, zones of 100-500 m, uneven development at the start, map editing before the run, real Earth later.

## Next step

1. **Fill the layer jumps** ([layer jumps](design/layer-jumps.md) §5): embodied pops are done; next a generic jump test harness, then body time while demoted, stocks and money.
2. **Write the first generator spec** (global grid, terrain and climate fields, sectors, zones; tests with agreement levels), then build it.
3. Then, in order ([world systems](design/world-systems.md) section 8): geology and deposits, capacity and settlements, routes, sectors and faction territory, economy core and chains, networks, building stock, the audit and realism report.
4. Tier 0: play the sandbox from the Windows zip; then stage 10 (kill or keep: real player runs).

## Waiting on the user

- **Real data:** a data tool that fetches licensed datasets and writes sourced presets, and a first real country (or global tables). Today every profile is a placeholder.
- **Tier 0 census and pops:** make the block's 60 neighbours the same pops as the registry's founders? It changes Tier 0's pinned golden hashes.
- Forced-labour populations stay out of the plan unless you decide otherwise ([0037](decisions/0037-sectors-zones-and-world-creation-answers.md)).
