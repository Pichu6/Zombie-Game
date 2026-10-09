# 0057 · The country economy: zones limited by geography, placement chosen and justified, growth driven by jobs and housing

- **Date:** 2026-10-09
- **Status:** Decided by the user (2026-10-08/09): the Stellaris and Hearts of Iron style of choosing where things go, with "things that make sense like in real life: a coal mine where a coal mine should be, materials taken to be processed", and zone capacity from geography rather than a fixed number of slots (answered "yes").
- **Builds on:** [0035](0035-realistic-world-numbers-and-the-pop-nation-loop.md), [0036](0036-worlds-start-with-nations-and-everything-follows-logic.md), [0043](0043-city-layer-and-repercussion-loop.md), [world systems](../design/world-systems.md) (contract rules 1-2, the dependency graph, the sector, zone and plot hierarchy), research [ST-6, ST-8, ST-11](../research/stellaris.md), [HOI-9..12](../research/hearts-of-iron.md).

## Decision

1. **A sector's zones are limited by its geography, not by a slot count.** Farmland as large as its good soil and water allow, extraction zones only where deposits are, housing on buildable land, ports on usable coast, plants where their fuel, water or site allow. Stellaris districts in spirit, with every limit coming from the world.
2. **The player or the AI chooses** what to zone and what to fund within those limits (a programme and budget from Macro, a plot from the City layer, 0043). Every placement records why it is possible (world systems rule 2); a choice the geography does not allow is refused with the reason.
3. **Chains are real** (spec 33): a mine ships ore to processing, processing to manufacturing or power; each step creates jobs where it stands.
4. **Growth and migration follow jobs and housing** as well as the land's capacity: jobs draw people, missing housing and unemployment push them away and lower births. The Stellaris loop of pops, jobs and housing, with real causes.
5. Deposits (world systems step 3) come first: nothing can be placed for a reason until the reasons exist.

## Consequences

- Deposit generation, capacity per sector and a site planner become the next generator work.
- A country's difficulty comes from its land, not from modifiers (how countries form and how a start is chosen: under discussion, 2026-10-09).
