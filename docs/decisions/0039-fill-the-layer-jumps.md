# 0039 · Fill the gaps in the jumps between layers

- **Date:** 2026-10-06
- **Status:** Decided by the user (intent below, from their message in chat). The audit and the proposed rules are Proposals in [layer jumps](../design/layer-jumps.md).
- **Extends:** [0012](0012-world-simulation-layers.md), [0036](0036-worlds-start-with-nations-and-everything-follows-logic.md); implements contract rules 8 and 12 of [world simulation](../design/world-simulation.md) §2.

## Decisions

1. **The simulation must try to fill in its gaps, in particular the "jumps" between layers** (Macro, City, Individual): every place where one layer's state is summed up, split down, switched to another level of detail, caught up in time, or passed as an order or a report.
2. **Each jump gets a declared pair of functions, an invariant and tests**, as the contract already says, instead of being left implicit.

## Notes (not decisions)

- The audit found 22 domains with jumps; pops are built for lazy and record levels, while the pop's **embodied** level (the link to the body simulation), stocks, money, establishments, networks above building scale, coarse fields, weather, flora and fauna, travel, ownership and time arbitration are gaps ([layer jumps](../design/layer-jumps.md) §3).
- Order of work follows dependency: embodied pops, a generic jump test harness, stocks, money, establishments, then the rest.
