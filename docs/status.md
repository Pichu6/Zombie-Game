# Status

Updated: 2026-10-08.

## What we're building

A realistic simulation of a world and the people in it: **Macro** (nations and their sectors), **City** (zones, buildings, supply) and **Individual** (one pop, played), one simulation under all three ([vision](vision.md), [0012](decisions/0012-world-simulation-layers.md)), with or without an outbreak ([0027](decisions/0027-no-outbreak-game-and-parameter-catalogue.md)). The engine's core is `engine/sim`, a library with no window; everything else reads it and sends it commands.

**One executable, one simulation** ([0052](decisions/0052-one-executable-one-simulation.md)): `zs.exe` is the only program (the window by default, headless modes by flag, `zs gen` for the generators). Work is local on Windows with MSVC. Nothing is left waiting on the user: Claude takes a default, records it as a decision, and keeps going.

## Where we are

**Built and tested** (MSVC release and debug, `ctest`; the window shows only the Tier 0 block so far):

| Part | What | Try it |
| --- | --- | --- |
| Tier 0 stages 1-4 | foundations, things and places, the body, actions and items | `--map`, `--body-lab`, `--bench`, `--scripted-pop` |
| Tier 0 stages 6-9 | power, noise, light; the dead; the group; the 14-day run | `--stage6`, `--stage7`, `--group`, `--run` |
| Content tools | seeded names, pops and towns, six culture packs, lifepath backgrounds built from places that exist in the town (`data/gen/lifepath.toml`) | `zs gen town` |
| Pop levels ([spec 28](tier0/specs/28-pop-levels.md)) | one pop at every layer: ids, random-access lives, births, cohorts and rosters, region-wide events, versioned saves, a world system with commands | `--pop-bench`, `--town` |
| World grid ([spec 30](tier0/specs/30-world-grid.md)) | the icosahedral hex-and-pentagon grid of world cells (102,012 at Earth size, areas within 1.42x), neighbours, distances, lookup; first unit of the world generator | `ctest` (`zs_gen_tests`) |
| Terrain and climate ([spec 31](tier0/specs/31-terrain-climate.md)) | plates, elevation, sea level, temperature, rain, biome on the grid, from sliders (land share, temperature, relief, moisture) and a seed | `zs gen world --seed 1` (text map) |
| Stocks, goods and money ([spec 32](tier0/specs/32-stocks-goods-and-money.md)) | owners by scope (country, region, building, pop), goods as bulk at a distance and units up close, spoilage that jumps exactly, money as a ledger resource with purchases, wages and taxes, panels per [0051](decisions/0051-money-and-resources-shown-follow-the-selection.md); a world system with visit, leave, purchase, pay and ship commands, saved and replayable | `ctest` (`zs_sim_tests`) |
| Establishments and chains ([spec 33](tier0/specs/33-establishments-and-chains.md)) | recipes as declared conversions with labour and power, establishments whose staff is the workplace roster, production that jumps exactly, the same recipe by hand, chains by shipment, Macro output as the sum under an owner | `ctest` (`zs_sim_tests`) |
| Projects ([spec 34](tier0/specs/34-projects.md)) | a build order escrows its budget on a site, stages are built by a recipe as materials arrive, a contractor is paid per stage, stalls say why (staff, power, money, materials), the finished building is one unit the Macro stock sums, cancel returns the escrow | `ctest` (`zs_sim_tests`) |
| Networks above building scale ([spec 35](tier0/specs/35-networks.md)) | a tree of grids (nation, district, building) with one node record at every level, solved on change: demand up, supply down in a fixed order, a fault below is an outage above, a repair restores exactly, load shedding above is unserved load below | `ctest` (`zs_sim_tests`) |
| Fields and weather ([spec 36](tier0/specs/36-fields-and-weather.md)) | coarsen and refine with keyed zero-mean variation (refine then coarsen is the identity), a climate cell downscaled to an hour's temperature and a day's rain with the year's mean and total exact, ambient noise summed from sources and the audible range above it | `ctest` (`zs_sim_tests`) |
| Flora and fauna ([spec 37](tier0/specs/37-flora-and-fauna.md)) | a species is a good whose unit is one tree or animal; a place's stock grows by the logistic closed form as declared inflows with an exact jump; harvest is a transfer; on approach the stock is a keyed roster of individuals, a kill removes one and leaves the rest; sustainable yield and depletion for Macro; a take command in the world | `ctest` (`zs_sim_tests`) |
| Outbreak ([spec 38](tier0/specs/38-outbreak.md)) | one keyed encounter rule (speed, warning, bite, kill, conversion) measured into the rates a region's compartments use; aggregate and entity-level runs of the same town agree in distribution; the reproduction number comes from the measured rates: a shambler among the warned fizzles, a runner among the unwarned takes off | `ctest` (`zs_sim_tests`) |
| Orders and belief ([spec 39](tier0/specs/39-orders-and-belief.md)) | levy, tax and staffing edicts that act only through the ordinary commands below them, with the signing as root cause, so any transfer traces to its edict and the edict sums its effects; beliefs per pop with source, age, hops and confidence, broadcasts that need communications, retelling that distorts, a lens that shows belief not truth | `ctest` (`zs_sim_tests`) |
| Time ([spec 40](tier0/specs/40-time.md)) | each system names its next wake tick and the world steps only those, so a cold world jumps a week in a few steps with the same hash as stepping while an attended body forces every tick; a pacer turns real seconds into ticks on the speed ladder of 0050, capped by what is attended with hysteresis, paused by alerts | `ctest` (`zs_sim_tests`) |
| Nation demographics ([0034](decisions/0034-nation-demographics.md)) | a country's own age, education, work, households as data (placeholder profile only) | `data/setup/nations/` |

**The window** ([spec 16](tier0/specs/16-view-2d.md)): the Tier 0 run seen from above. The user checked it on screen: it works; WASD movement is poor and it is hard to tell what is going on.

**Designed, not built** (Proposals unless a decision says otherwise): [sandbox parameters](design/sandbox-parameters.md), [world numbers](design/world-numbers.md), [world systems](design/world-systems.md), [zoom continuum](design/zoom-continuum.md), [focus and compression](design/focus-and-compression.md), [layer jumps](design/layer-jumps.md) (rows 18 travel and 19 ownership still a gap), [pop lore and custom pops](design/pop-lore-and-custom-pops.md).

**Decisions** run up to [0052](decisions/0052-one-executable-one-simulation.md).

## Next step

1. ~~One executable~~: done (2026-10-08), `apps/zs`.
2. ~~One world and one population~~, first slice done (2026-10-08, [spec 41](tier0/specs/41-one-world.md)): the run is the one world; its 61 people are the registry's founders; the economy's owners, the grid tree and orders run in it; the stage 9 hash re-pinned on MSVC. Next slice (spec 42): the block placed on a cell of the generated world grid, its climate feeding weather to bodies; then the outbreak aggregate and flora and fauna around it.
3. **Global tables and base countries** (0052 item 4), spec first: age structure, life tables, fertility, households, work by sector by development level, then fictional base countries validated against published aggregates.
4. Then travel and ownership (layer jumps rows 18 and 19) and the generator (sectors and zones on the grid), each added to the one world as it is built; the window's WASD and legibility when it is next touched.
