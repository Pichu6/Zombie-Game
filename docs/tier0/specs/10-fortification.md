# 10 · Fortification

**Status:** Proposal. **Stage:** 2 (data), 7 (attacks). **Needs:** [08](08-assembly.md), [09](09-site-map.md); attacks use [20](20-damage.md).

## Purpose

Doors, windows, walls, fences and barricades are assemblies that fail part by part at their weakest point, under the summed push of real bodies ([world simulation §10](../../design/world-simulation.md#10-items-resources-and-fortifications)).

## State

| Field | Notes |
| --- | --- |
| failure chain | Ordered parts per assembly type: door = lock/strike plate → hinges → panel → frame; window = glass → frame; barricade = fasteners → boards |
| part strength | Hold force (N) or energy to break (J), from Q and condition |
| damage | Condition of each part ([07](07-condition.md)) |
| state | intact, damaged, breached (passable), destroyed (rubble) |

## Rules

1. Push load on an edge = sum over attacking bodies of mass × g × posture factor × (1 − decay factor). Each body's factor comes from its body ([11](11-body-composition.md)).
2. Each tick, the load is applied to the first intact part of the chain; it loses condition at a rate that grows with load above its hold force. When it fails, the next part takes the load.
3. Breach time therefore falls smoothly with horde size; there is no "N zombies break a door" constant.
4. Every hit or push is a sound event ([18](18-noise-field.md)); the level grows with impulse.
5. Barricades add parts in front of the chain: boards nailed across a door must fail before the door's own chain. Barricading is loud (hammering).
6. Breaching an edge marks the room graph dirty ([09](09-site-map.md)).

## Invariants

- Parts fail in chain order.
- A breached edge is passable; an intact one is not.

## Tests

- Breach time decreases monotonically with the number of pushers. (A3)
- A boarded door outlasts the bare door under the same load. (A3)
- Ten seeded runs with 5 walkers on a standard interior door land inside the calibration band. (A2)

## Calibration

| Value | Source |
| --- | --- |
| Door resistance classes | EN 1627 (burglar resistance RC1–RC6) |
| Glass resistance | EN 356 (P1A–P8B) |
| Crowd push forces | ASTM F3038 (timed forced-entry tests); crowd pressure studies (Fruin) |
| Typical residential door | Fails at strike plate first (field reports on kick-ins); tune |

## Not in Tier 0

Load graph for walls and floors (collapse), crushing bodies under rubble.
