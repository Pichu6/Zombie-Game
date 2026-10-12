# 18 · Noise field

**Status:** Proposal. **Stage:** 6. **Needs:** [09](09-site-map.md).

## Purpose

Every sound is an event with a level that travels through the room graph and wakes listeners. One rule for all hearing: signal minus background at the listener ([world simulation §9](../../design/world-simulation.md#9-communication-decided-as-intent-model-proposal)). It is the "noticed" half of the core choice.

## State

| Field | Notes |
| --- | --- |
| sound event | source position, level at 1 m (dB), duration (impulse or continuous), cause |
| continuous sources | generator, radio speaker, running power tool |
| background level | per area and hour: urban day 55 dB, urban night 45 dB; after the grid fails, 35 by day and 27 at night (decision 0064) |
| portal loss | per portal from the assembly ([08](08-assembly.md)) and its open fraction |

## Model as implemented (Proposal, 2026-10-05)

Code: `engine/sim/src/noise.cpp`. Stateless: callers evaluate it when something changes.

- **Best path** is a least-loss Dijkstra over cells (not rooms): crossing an edge costs its assembly's `sound_loss_db` when closed, 0 when air passes (no edge, open, breached), +3 dB per nailed board; stairs cost 0. Ties break by steps then cell index.
- **Level** at a cell = L(1 m) − 20·log10(max(1, straight-line distance, 3 m per level)) − path loss; sources add energetically.
- **Background:** 55 dB day / 45 night while the grid lives, 35 / 27 after it fails (night 22:00–07:00): drops of 20 and 18 dB, so a voice carries about ten times farther ([0064](../../decisions/0064-sound-and-sky-defaults.md), HST-138).
- **Carry outdoors:** audible range = 10^((L − best loss to any outdoor cell − background) / 20) (the notice threshold is 0 dB). An open-frame generator (85 dB at 1 m, a 68–70 dBA-at-7 m unit, HST-140) carries 100 m at night before the grid fails and about 790 m after.
- **Sources** now: the running generator, each pop's current primitive or footsteps, a walker shoving a door (75 dB), and a walker's moan (65 dB at 1 m, an estimate to tune) while it pursues, bashes or attacks ([0064](../../decisions/0064-sound-and-sky-defaults.md) rule 2): the moan carries to other walkers, so a horde turns as a wave and relays the sound walker to walker (HST-146, RA-111).

**Known gaps:** spreading uses the straight-line distance, not the length of the best path (a sound around corners carries a bit too far); no direction or certainty from the margin; no cache.

## Rules

1. Level at a listener = source level − 20·log10(distance in m) along the best path − sum of portal losses on that path. Best path = least total loss, by Dijkstra over the room graph with portal positions.
2. Two thresholds on signal minus background ([0064](../../decisions/0064-sound-and-sky-defaults.md), HST-139): a listener **notices** a sound at 0 dB and turns toward it (a walker takes up to six steps that way), and **places** it at +10 dB and goes to the spot. Hearing capability ([13](13-capabilities.md)) scales the margin.
3. Continuous sources are re-evaluated only when something changes (source on/off, a portal changes, background changes), not per tick.
4. Sound that reaches the map border above the background adds to the off-map attention of the census ([22](22-census.md)): loud nights bring bigger waves.
5. The overlay shows the last loud events and the current field, so the player can see how far the generator carries.
6. When the grid dies the background drops, so the same generator is heard much farther (Decided as intent: "the generator, not the shout, brings the horde").

## Invariants

- Level never increases along a path.
- A portal's loss is ≥ 0; an open door or broken window is ~0.

## Tests

- An open-frame generator (85 dB at 1 m) is heard by a walker 200 m away outside at night after grid failure (background 27 dB, about 790 m), but not before (background 45 dB, 100 m). (A3)
- A walker that pursues moans at 65 dB, and a second walker 3 m away at night hears the moan and turns toward it. (A3)
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
