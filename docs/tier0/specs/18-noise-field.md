# 18 · Noise field

**Status:** Proposal. **Stage:** 6. **Needs:** [09](09-site-map.md).

## Purpose

Every sound is an event with a level that travels through the room graph and wakes listeners. One rule for all hearing: signal minus background at the listener ([world simulation §9](../../design/world-simulation.md#9-communication-decided-as-intent-model-proposal)). It is the "noticed" half of the core choice.

## State

| Field | Notes |
| --- | --- |
| sound event | source position, level at 1 m (dB), duration (impulse or continuous), cause |
| continuous sources | generator, radio speaker, running power tool |
| background level | per area and hour: urban day ~55 dB, urban night ~45 dB; after the grid fails, quiet night ~30 dB |
| portal loss | per portal from the assembly ([08](08-assembly.md)) and its open fraction |

## Rules

1. Level at a listener = source level − 20·log10(distance in m) along the best path − sum of portal losses on that path. Best path = least total loss, by Dijkstra over the room graph with portal positions.
2. A listener hears a sound if level − background − hearing threshold > 0, with a starting threshold of 3 dB; the margin sets how sure it is of the direction and place. Hearing capability ([13](13-capabilities.md)) shifts the threshold.
3. Continuous sources are re-evaluated only when something changes (source on/off, a portal changes, background changes), not per tick.
4. Sound that reaches the map border above the background adds to the off-map attention of the census ([22](22-census.md)): loud nights bring bigger waves.
5. The overlay shows the last loud events and the current field, so the player can see how far the generator carries.
6. When the grid dies the background drops, so the same generator is heard much farther (Decided as intent: "the generator, not the shout, brings the horde").

## Invariants

- Level never increases along a path.
- A portal's loss is ≥ 0; an open door or broken window is ~0.

## Tests

- An open-frame generator (~89 dB at 1 m) is heard by a walker 200 m away outside at night after grid failure (background 30 dB), but not before (background 45 dB, audible only to ~110 m). (A3)
- Closing a door lowers the level in the next room by that door's loss. (A1)
- Hammering a barricade carries farther than footsteps. (A3)

## Calibration

| Value | Source |
| --- | --- |
| Generator ~65–75 dBA at 7 m (open-frame), ~52–60 dBA (inverter) | Manufacturer spec sheets |
| Hammering ~90–100 dB at 1 m, shout ~80–90, talk ~60, footsteps ~40–60 | Occupational noise references (NIOSH); estimates |
| Wall and door losses: interior stud wall ~33–38, hollow door ~20, solid door ~30, single glazing ~25, masonry ~45–50 dB | ASTM E413 STC ratings for typical assemblies |
| Background levels | WHO environmental noise guidelines; US EPA levels document (1974) |

## Not in Tier 0

Frequency bands, reverberation, wind, radio and phone channels (the radio in Tier 0 only receives a broadcast).
