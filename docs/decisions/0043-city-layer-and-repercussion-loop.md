# 0043 · City layer gameplay and the repercussion loop

- **Date:** 2026-10-07
- **Note:** written on 2026-10-07 in a local session with a lower number and renumbered on merge.
- **Status:** Decided by the user (intent). The model is a Proposal in [world simulation §7 and §13](../design/world-simulation.md).
- **Extends:** [0012](0012-world-simulation-layers.md), [0013](0013-production-ladder.md), [0016](0016-sandbox-setup-and-layer-flow.md), [0042](0042-pop-presentation-per-layer.md).

## Decisions

1. **City is Cities: Skylines style management, RTS-like:** building options, city management, dealing with the city and its establishments, infrastructure.
2. **What can be built depends on the country:** its development, its politics and its "tech tree". Recorded as: the country's real present (industries, grid, hospitals, machinery, knowledge, institutions) plus the projects it runs during play, and its laws. Not a research-points tree: the production ladder ([0013](0013-production-ladder.md)) and "settings change numbers, never rules" stay in force.
3. **Situations and states of the selected city** are shown and acted on in City (the city's status, what limits it, its edicts in force).
4. **The repercussion loop runs both ways, 1 to 3 and 3 to 1.** Political decisions in Macro inflict things on City (building options, supplies, staff, movement, use of force); City decisions and Macro decisions reach Individual (what a pop's day looks like, what is on the shelves, who guards the road). In the other direction, what individuals do aggregates into City and Macro (noise and light, work done, infections hidden, jobs taken, samples carried, orders refused).

## Why

The user sees City as the bridge: Macro sets the rules, City is where they become buildings, jobs and shortages, and Individual is where a person lives them. The loop in both directions is the selling point of "one world, three layers" ([0012](0012-world-simulation-layers.md)).

## Consequences

- Build orders stay Projects with real jobs and materials ([world simulation §7](../design/world-simulation.md)); City picks plots and priorities, Macro sets programmes and edicts, Individual takes the jobs.
- The country's development level is data in the world fields and the census (which industries, grid class, hospital capacity, machinery), set at world creation and changed only by Projects during play.
- Every City building option states what gates it: a country's development, a law, a material, a machine or a knowledge, so the reason is shown when it is greyed out (the same five gates as actions, [spec 14](../tier0/specs/14-action-executor.md) rule 1).
- Upward effects need no new rule: they are the sums City and Macro already read (counts, stocks, fields, the ledger). What is new is making them visible: a city status that names the individual causes behind a number when they are known.
