# 09 · Site map and room graph

**Status:** Proposal. **Stage:** 2. **Needs:** [08](08-assembly.md).

## Purpose

The hand-made Tier 0 block: one 2-floor building with a basement and yard, plus 2 houses. Every building enterable; walls are assemblies that can be breached. The room graph it produces carries sound, light and (later) water and smoke ([infrastructure](../../design/infrastructure.md#room-graph-fluids-through-spaces)).

## State

| Field | Type | Notes |
| --- | --- | --- |
| levels | basement (−1), ground (0), first (1) | Houses use ground only |
| cells | 1 m × 1 m, per level: floor type (concrete, wood, grass, asphalt), indoor/outdoor | Walkable or not |
| edges | between neighbouring cells: none, wall, door, window, fence, hole | Each non-empty edge is an assembly instance ([08](08-assembly.md)) |
| stairs | cell pairs across levels | Portals in the room graph |
| rooms | derived: flood fill of cells bounded by closed edges | Recomputed when an edge opens, closes or breaks |
| portals | derived: each open or leaky edge between rooms, with area and open fraction | Room graph edges |
| receivers and fixtures | positions of lamps, outlets, fridge, stove, panel | Linked to [17](17-power-network.md) |
| stockpiles | marked areas or shelves for the haul job | [24](24-pop-ai.md) |

## Rules

1. The map is a TOML file under `data/tier0/map/`: one ASCII grid per level at double resolution (cells on odd rows and columns, edges between them, legend in `data/tier0/map/README.md`), plus stairs and an object list. It is hand-edited (a helper script drew the first version) and loaded at start. It is designed so a generator could fill the same format later (vision: map data ready for a generator).
2. Rooms and portals are rebuilt only when an edge changes state (dirty flag), never per tick.
3. Outdoors is one room per connected outdoor area; the map border connects to the off-map world (census arrivals, [22](22-census.md)).
4. A closed door or intact wall is still a portal for sound, with that assembly's transmission loss ([18](18-noise-field.md)).
5. Pathfinding: A* on cells with door costs; doors that are locked or barricaded are blocked for those without the means to open them. Re-planned on edge change.

## Invariants

- Every walkable cell belongs to exactly one room.
- Every portal joins two different rooms.

## Tests

- Golden room graph for the Tier 0 map file. (A1)
- Breaking a wall merges two rooms and adds no others. (A1)
- A path exists from every pop start to the yard. (A1)

## Calibration

Building sizes from real floor plans: rooms 9–20 m², 2.5–2.7 m ceilings, 0.9 m interior doors, 1.0 m exterior doors.

## Not in Tier 0

Roofs as walkable spaces, sewer, vehicles, the generator of maps.
