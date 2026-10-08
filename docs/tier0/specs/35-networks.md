# 35 · Networks above building scale

**Status:** Implemented and tested, headless (the library and a world system). **Stage:** not Tier 0; the [layer jumps](../../design/layer-jumps.md) audit row 10 (networks). **Needs:** [06](06-save-replay.md). Relates to [17](17-power-network.md) (the building's own network, Tier 0). Design: [world systems](../../design/world-systems.md) §3 and §10.3, [world simulation](../../design/world-simulation.md) §8 ("Power plant: one assembly record"). Decision: [0039](../../decisions/0039-fill-the-layer-jumps.md).

## Purpose

A national grid's capacity, load and outages at Macro; a district's substation, breakers and buildings at City; a building's panel, circuits and appliances at Individual: one network. This spec makes them a **tree of grids with one node record at every level**, so a fault in a building is unserved load in its district and its nation by the same numbers, an order above (open a breaker, cut a plant) is lost load below, and a repair restores the exact state. Power first; water, roads and telecom are the same tree with other units.

## What the code forces

- **No time.** A network is solved on change ([17](17-power-network.md) rule 1); its state is a function of its switches, faults, capacities and demands. So there is no jump form to write and no partition to test: stepping and jumping are the same solve.
- **Saves hold state, setup holds shape** ([06](06-save-replay.md)): the grids, nodes and edges are built by setup; what a command can change (closed, sound, capacity, a leaf load's demand) is saved. The solve is recomputed on load.

## State

| Field | Notes |
| --- | --- |
| `Grid` | name; `parent` grid (−1 = root); `feed` (the node of this grid through which the parent's supply enters: a Hub or a Source, never a Load); nodes; edges; results: `demand_w` (connected demand), `served_w`, `capacity_w` (sound sources inside plus the feed's allotment) |
| `GridNode` | name; kind (Source, Switch, Load, Hub); `capacity_w` (Source); `demand_w` (Load; for a child grid's load, the child's connected demand, written by the solve); `closed` (Switch); `sound` (any node; a fault removes it); `child` (a Load that is a child grid, −1 = leaf); results: `connected`, `served_w` (a Load: what it got; a Source: what it gave) |
| `NodeId` | `(grid << 32) | node`: one id space, so a command names any node of any level |

## Rules

1. **Islands.** Within a grid, nodes that are sound (and, for switches, closed) are joined by the edges into islands (union-find). An unsound node and an open switch are not in any island; nothing passes through them.
2. **Demand, bottom-up.** A child grid's load carries the child's **connected demand**: the demand of the child's leaf loads and child grids in the island of its feed node. A root grid's demand is the demand of its loads in any island. Demand that is cut off is not drawn: a building whose panel failed asks its district for nothing, so district and nation report both less demand and less served load, and the difference is the watts lost.
3. **Supply, top-down.** For each island: supply = the sound sources' capacity in it, plus the feed's allotment if the feed node is in it. Loads in the island are served **in node order** until the supply runs out (a fixed, deterministic shedding order: setup puts hospitals before saws); a child grid's load passes what it got down as its feed. A load not in any supplied island is not connected. Sources report what they gave, the feed's allotment counted first, then the sources in node order.
4. **Capacity and summary (enumeration).** `summary(grid)` sums, over the grid and everything under it, the sound sources' capacity, the leaf loads' demand and served load; `unserved = demand − served`. The nation's outage is the sum of its districts', which is the sum of their buildings'. Nothing is stored apart.
5. **Faults and orders.** A fault is `sound = false` on any node; a repair sets it back; the state after the repair equals the state before the fault, node by node (A1). A load-shedding order is `closed = false` on a switch above: everything behind it is unserved and disconnected. A plant cut is a lower `capacity_w`: the nation serves in node order and the shortfall is exactly the unserved load.
6. **Commands** (`GridCommand`, 0x3001–0x3004): `GridSwitch` (node, 1 close or 0 open; Switch nodes), `GridFault` (node, 1 sound or 0 faulted; any node), `GridSetCapacity` (Source nodes), `GridSetDemand` (leaf Load nodes). Refusals (`GridRefused`, with `GridRefusal`): unknown node, wrong kind, nothing to change, bad value. Every applied command solves the tree and emits `GridChanged` (served, unserved) for each grid whose served or unserved load changed.
7. **One tree, many networks.** Power is the first. A water network is the same tree with litres per hour; a road network's "capacity" is a different law and waits for the travel jump (row 18).

## Invariants

- Served ≤ demand for every load and every grid; served ≤ capacity for every island.
- A load is served only if connected; a child grid gets exactly what its load in the parent was served.
- The summary of a grid equals the sum of the summaries of its parts.
- Solving an unchanged tree changes nothing; save, load, solve gives the same results.

## Tests (`test_grid.cpp`)

- **Sums (A1):** a nation (10 MW plant, two districts), districts with breakers, buildings A (fridge, lamps, saw, a 3 kW generator behind an open transfer switch), B and C (a 6 MW factory): demand sums up the tree, all served, the plant gives what is drawn and the generator idles.
- **Fault up, repair exact (A1):** building A's panel fails: A serves 0, district 1 and the nation lose exactly A's watts, the plant gives that much less; a faulted breaker leaves C connected to nothing; after both repairs every node of A and the served load of every grid equal the state before.
- **Orders down (A1):** opening the nation's breaker to district 1 cuts A and B (unserved, not connected) and leaves C whole; A on its generator serves its loads in order; a saw bigger than the generator gets what is left after the fridge and lamps; the plant at 5 MW serves district 1 first and gives district 2 the rest, the nation's unserved being exactly the shortfall.
- **Idempotence and save (A1):** solving twice reports no change; the state saves and reloads to the same results.
- **In a world (A1):** commands at known ticks (a fault, a breaker, two refused commands, a repair, a plant cut, the breaker back) emit `GridChanged` per affected grid and `GridRefused` with reasons; the final summary is right; save, load, save are identical bytes and replaying the input log reproduces the hash.

## As built

- `engine/sim/include/zs/grid.h`, `src/grid.cpp`: `GridTree` (`add_grid`, `add_node`, `add_edge`, `attach`, `solve`, `summary`, save and load).
- `engine/sim/include/zs/grid_system.h`, `src/grid_system.cpp`: `GridSystem` (commands, `start()` at the end of setup), events `GridChanged` (56), `GridRefused` (57).

## Calibration

| Value | Source |
| --- | --- |
| Capacities and demands | [world systems](../../design/world-systems.md) §10.3 (grid capacity, load, outage anchors); the tests use round numbers |
| Shedding order | Setup's node order; a policy (hospitals first) is an edict later |

## Not in this spec

Making the Tier 0 building network ([17](17-power-network.md)) a leaf of this tree (its `mains_alive` would be the district's allotment to it); cables with limits and losses; plants as establishments ([33](33-establishments-and-chains.md): fuel in, watts out) and the energy law of production; faults drawn by hazards over an interval (the only time-dependent part, which belongs to the hazard system); water, roads and telecom as instances.

## Open

1. **Partial service.** A load served below its demand is "partly served" here (the saw gets 2,790 of 3,500 W). Real appliances brown out or trip. Recommendation: a leaf load is all-or-nothing when its grid is a building (the building's own rules, [17](17-power-network.md)), proportional only for aggregate loads; decide when the Tier 0 network joins the tree.
