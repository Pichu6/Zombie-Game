# 41 · The one world

**Status:** Implemented and tested, 2026-10-08 (`engine/sim/src/one_world.cpp`, `test_one_world.cpp`; MSVC). **Stage:** the one simulation of [0052](../../decisions/0052-one-executable-one-simulation.md) items 2 and 3. **Needs:** [26](26-run.md) (the run), [22](22-census.md) (census), [28](28-pop-levels.md) (registry), [32](32-stocks-goods-and-money.md) (owners and money), [35](35-networks.md) (grid tree), [39](39-orders-and-belief.md) (orders), [40](40-time.md) (wake ticks).

## Purpose

Until now each part was assembled on its own: the Tier 0 run in `tier0::make_run`, the registry, economy, grids and orders only inside their tests. This spec puts them in **one world**: the run that `zs.exe` opens and `zs --run` plays is the same `World` that holds the pop registry, the owners and their money, the grid tree and the orders. The block's neighbours are registry pops, not a separate list.

## What the code forces

- **The census has its own people.** A census `Person` has no `PopId`; it is keyed by its index. The registry already reserves founder ids for it: `census_pop_id(slot) == slot` ([28](28-pop-levels.md) id table), and founders can carry an explicit age and sex (`set_founder_specs`). So the census rows become the registry's founder cohort with no new id scheme.
- **The world hash covers every system.** Adding systems to the run changes its golden hash; the stage 4, 6, 7 and 8 scenarios do not get the new systems and keep theirs.
- **`engine/sim` cannot call `engine/gen`.** The world grid and terrain fields live in the generator library, so placing the block on the world grid is the next slice (below), not this one.

## Rules

1. **One assembly.** `tier0::make_run(seed)` builds the one world. After the Tier 0 systems (items, map, actors, power, census, walkers, bodies, AI, run) it adds, in this order: **pops** (`PopSystem`), **economy** (`EconomySystem`), **orders** (`OrdersSystem`), **grids** (`GridSystem`). Cold systems update after the hot ones, so an hourly economy step sees the bodies' tick. `Tier0` gains pointers to the four; the stage scenarios leave them null.
2. **One population.** Every census row, the player included, is a registry founder: cohort 1, region 1, slot = census index, founder spec = the row's age and sex. `Person::pop` is `census_pop_id(index)`, derived on load, never saved. Each census household is a household roster (`kind Household`, index by first appearance). A census fate and the registry agree: a person whose fate is dead, a walker or destroyed is dead in the registry from the tick the census sets it (`PopSystem::record_death`, called by the census; derived from state, so replay repeats it); those dead before the run have death year 0. The registry may still end an off-map life by its own drawn death year.
3. **One set of owners.** The economy's owner tree: the **country** (1), the **region** (the block's town), the three **buildings** of the map (the main building, house A, house B) under the region, and one **pop owner** per founder under the region. Money is minted at setup as a placeholder wallet per living adult, a region budget and a country budget (constants marked placeholder in `one_world.cpp`) until the data spec (0052 item 4) gives real numbers; the panels of [0051](../../decisions/0051-money-and-resources-shown-follow-the-selection.md) read these owners.
4. **One grid tree.** Nation grid → the region's district grid → one node per building on the map. The block's own `Power` stays the building-level model; the district's outage is the Tier 0 grid failure. In this slice the tree is solved at setup and the two are not yet coupled (open point 1).
5. **Orders reach the registry.** The orders audience is the registry's living founders, not a hand-made list.
6. **Time.** The world steps every tick while a body is attended, as [40](40-time.md) rule 4 already does; the cold systems' wakes make `World::jump` exact when nothing is attended.
7. **Save and hash.** The one world saves and loads like any world; its golden hash is pinned anew in `test_stage9` (MSVC), once.

## Invariants

- Every census person has exactly one registry pop with the same age (whole years) and sex, and the same id as its slot.
- Every census death is a registry death from the same tick.
- Ledger conservation holds with all systems in (the per-tick check).
- Save, load and replay of the one world give the same hash.

## Tests

- `test_one_world.cpp`: census ↔ registry agreement (ids, age, sex, households) at setup; after a seeded 2-day run every census death is a registry death; the owner tree and balances; the grid tree solves; orders audience equals the living founders; save/load/replay round trip.
- `test_stage9`: the run's golden hash re-pinned once; stages 4, 6, 7 and 8 unchanged.

## Not in this spec

- **The place on the world grid** (next slice, spec 42): the block in one land cell of the generated world, its climate cell feeding weather ([36](36-fields-and-weather.md)) to bodies. Needs the world grid and fields reachable from the assembly (moved into `engine/sim` or the assembly moved up).
- **Real numbers**: wallets, budgets and age structure come from the data spec (0052 item 4).
- **The outbreak aggregate** ([38](38-outbreak.md)) for the off-map town, and flora and fauna stocks ([37](37-flora-and-fauna.md)) around the block: added when the place exists.
- The window showing the new systems (money panel, region panel).

## Open (defaults taken, 0052 item 6)

1. Coupling the district grid with the block's `Power`: default, the district outage drives the Tier 0 grid failure once the pacer and grid tree are wired; until then they are parallel.
2. Placeholder money: default, a flat wallet per living adult and region and country budgets, marked placeholder in the code.
