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

Rules from the engineering research (added 2026-10-08, [0055](../../decisions/0055-defaults-for-fixes-from-engineering-research.md) §3-4; rules 15 and 19 built 2026-10-08, rules 16-18 and 20 not yet, [research fixes](../research-fixes.md) packages 2-4):

15. **Every save goes through `zs::write_file`**, the window's F5 included (today it writes with a plain stream, [DC-39](../../research/data-compression.md)), and `write_file` flushes the temp file to disk before the rename (`FlushFileBuffers` on Windows, `fsync` and a directory `fsync` on POSIX, [DC-40](../../research/data-compression.md)), as rule 13 asks.
16. **Format version 4** adds to the header, in one step: the codec, the uncompressed size, and rule 4's binary hash, data hash, seed and settings ([DC-38](../../research/data-compression.md)). The version 3 reader stays.
17. **Compression:** zstd, one frame per blob, single-threaded, content size and checksum on; level 3 for saves, level 1 for autosaves and traces, level 19 only for archives ([DC-22, DC-27, DC-29](../../research/data-compression.md)). No trained dictionaries for now. **Hashes and goldens are taken over the uncompressed bytes only**, because compressed bytes change with the library version and thread mode ([DC-5](../../research/data-compression.md)).
18. **Autosave and rewind history:** one full save per game day, with deltas against the previous save in between ([DC-24](../../research/data-compression.md)).
19. **A NaN in saved state is a bug:** the archive asserts on it rather than canonicalising it, since x86 and ARM write different NaN bits ([DET-37](../../research/determinism.md)).
20. **Founder specs are regenerated** from the generator, its version and data hash, not stored (rule 7, [DC-37](../../research/data-compression.md)); pop record death and effect years move to fixed point when the record format next changes ([DC-35](../../research/data-compression.md)).

Rules for the chunked world (added 2026-10-07 from [pop-simulation-and-persistence.md](../../research/pop-simulation-and-persistence.md) §5; apply from the Tier 1 chunk split, [27](27-detail-levels.md)):

7. **Store inputs and deltas, never regenerable output.** The save holds the seed, generator version and data-definition hash, then only what diverged from generation: visited layouts, touched items, records. Each chunk carries a generation status (unvisited, layout generated, visited), so a half-generated chunk is a valid save.
8. **Per-chunk header:** schema version, generator version, status, caught-up tick, level at save time, uncompressed size, CRC-64. The tick is authoritative; any wall-clock timestamp is for tools.
9. **Iteration order is state.** Any list whose order affects results is saved and restored in order (extends [05](05-clock.md) rule 7 to the file). Pending scheduled events are saved with their chunk; for a cold chunk they are its whole simulation.
10. **One global person table**, keyed by a stable 64-bit id; chunks and groups hold ids. A person is never writable in two forms at once; the hot body is materialised from the record and projected back; cell counts are derived caches stamped with the tick they were derived at.
11. **Three version numbers:** container format (global), per-chunk data version upgraded lazily on load, and one monotonic version per subsystem. The save records the list of applied migrations and the name → id mapping of data definitions, so renamed definitions remap on load. The migration window is bounded; older saves go through a converter tool outside the game.
12. **Text twin:** any save can be dumped as a complete human-readable tagged text, so two runs can be diffed at the first hash mismatch.
13. **Verify before rename:** write a temp file, fsync, read it back and check the hash, rename, fsync the directory (Dwarf Fortress on a full disk "pretends to save correctly"; rename without fsync lost data on ext4 in 2009).
14. **Container from Tier 1 (Decided, [0045](../../decisions/0045-open-questions-answered-2.md)): one SQLite file per world (chunks and records as blobs keyed by packed ids, zstd at the levels of rule 17, partial writes, crash safety; Vintage Story, Luanti and Cataclysm: Bright Nights precedents) or region files (32×32 chunks, 4 KiB sectors, two header tables; Minecraft). Either way the deterministic full-order dump stays the reference for hashing.

## Invariants

- save → load → save produces identical bytes. (A1)
- Replaying the input log reproduces the per-day state hashes. (A1)

## Tests

- Golden hashes for a fixed scripted run, checked in CI per compiler. (A1)
- Load at day 5, play to day 6, compare with uninterrupted run. (A1)
- Upgrade test: a saved file from each past format version still loads. (A1)
- Version 4: save, load, save gives identical uncompressed bytes; the day-14 save of seed 1 drops from about 213 KB to about 21 KB ([DC-22](../../research/data-compression.md)); a save killed mid-write leaves the old file intact. (A1)
- Demote a chunk to cold at tick T, save, load in a fresh process, wake it at T+k, continue the input log: same result as the never-demoted run at the level each jump rule claims (A1 for exact rules, A2 for drawn ones). From Tier 1.

## Calibration

None.

## Not in Tier 0

Time scrub (rewind) in the view: replay from the nearest save covers it for now.
