# 06 · Save, replay and event log

**Status:** Proposal. **Stage:** 1. **Needs:** [01](01-quantity.md)–[05](05-clock.md).

## Purpose

Any run can be saved, loaded, replayed exactly and inspected. This is how an emergent simulation stays debuggable ([concept v2](../../research/concept-v2.md), risks table).

## State

| Field | Notes |
| --- | --- |
| save header | format version, binary hash, data hash, seed, settings, tick |
| world state | every system's state, written in a fixed order |
| input log | every player command with its tick |
| event log | every event ([04](04-hazard-event.md)) with cause |
| state hash | 64-bit hash of world state, per tick on demand |

## Rules

1. Save format is versioned from the first commit. Loading an older version runs explicit upgrade steps; unknown versions refuse to load.
2. Save = full state. Replay = start state + seed + input log. Both must reach the same hash.
3. The event log is append-only during a run and saved with it. The view filters it; the simulation never reads it.
4. Every output (save, log, test report) carries binary hash, data hash, seed and settings.
5. Loading goes into a fresh candidate world built by the same setup; the running world is replaced only if the load succeeded. Saving writes a temporary file, reads it back, then renames it. Only primary state is saved; readouts are recomputed.
6. Data (item definitions, map, census table, calibration numbers) lives in text files under `data/tier0/`, never in code.

## Invariants

- save → load → save produces identical bytes. (A1)
- Replaying the input log reproduces the per-day state hashes. (A1)

## Tests

- Golden hashes for a fixed scripted run, checked in CI per compiler. (A1)
- Load at day 5, play to day 6, compare with uninterrupted run. (A1)
- Upgrade test: a saved file from each past format version still loads. (A1)

## Calibration

None.

## Not in Tier 0

Time scrub (rewind) in the view: replay from the nearest save covers it for now.
