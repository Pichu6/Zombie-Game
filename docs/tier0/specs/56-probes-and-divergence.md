# 56 · Probes and the first divergence: per-system hashes and timers, and the tool that names the tick and the system

**Status:** Implemented and tested on MSVC (2026-10-11), written with its code. **Stage:** after Tier 0; ranked improvements items 1 and 67, the first of the "cheapest and unblocking" steps of `docs/status.md` step 8. **Needs:** [06](06-save-replay.md), [40](40-time.md). **Design:** [sensors and soundscape](../../design/sensors-and-soundscape.md) §1 (probes never feed the simulation), [engine architecture](../../design/engine-architecture.md). **Research:** [hordes-sound-sky-and-testing](../../research/hordes-sound-sky-and-testing.md) HST-126 (Factorio checksums every tick and finds the first desync tick), HST-127 (Age of Empires most wished it had built per-system sync tags early).

## Purpose

Every later piece, chunks, threads and run-ahead, will one day make two copies of the world disagree, and the question will be: at which tick, and in which system. Today a disagreement is one state hash, found a day late. This spec gives the world a hash per system and a timer per system, costing nothing when off, and a headless tool that steps one copy and jumps or reloads another, comparing system by system at every check, and prints the first divergent tick and system.

## Rules

1. **A hash per system.** `World::system_hashes()` returns "core" first (the clock, the entity counter, the rng counters, the ledger, the locations, the event log, the scheduler, the input log and the pending commands: everything `save_state` writes before the systems) and then every system in update order, each the hash of that system's own `save` bytes. The whole state's hash is unchanged; `save_state` is now `save_core` followed by the systems.
2. **The first divergence** between two worlds is the first pair of equal names with different hashes, in that order: a clock that moved shows as "core" first, a system that computed differently shows under its name.
3. **Probes** (`World::probes`, one per system in update order, grown by `step`): the count of `update` calls, and the wall-clock nanoseconds inside `update` when `probe_timing` is on. Off by default; the window and the Atlas turn it on. Probes are never saved, never hashed and never read by a system: timing a world changes nothing in its state (tested).
4. **The tool**, `zs --diverge [--seed N] [--hours H] [--every TICKS] [--mode jump|saveload|both]`: builds the one world and steps it; at every check tick, `jump` jumps a second copy to the same tick and compares, `saveload` saves the stepped world and loads it into a fresh setup and compares, `both` does both. It prints the first divergent tick (with its day and hour) and system and exits 1, or "agree" with the number of checks; then the probes: updates, milliseconds, share of the time and hash per system, and the core hash.
5. **In test builds, every hour.** The test steps a stage 8 world through a day and jumps another, comparing every system each hour; a save loaded into a fresh setup compares equal system by system; probes count every update and time only when asked.
6. **The simulation card** ([55](55-atlas.md)) shows each system's share of the time since the open and its hash, and the core hash.

## As built (2026-10-11)

- `zs --diverge --seed 1 --hours 6 --mode both`: agree, 6 checks over 43,200 ticks, every system, in 26 s (most of it the seven builds of the one world for the reloads). The probes: pop_ai 40.7 % of the update time, walkers 22.5 %, bodies 11.0 %, actors 8.7 %, census 8.0 %; every other system under 2 %.
- Tests: core first and the systems in order; two fresh worlds agree and a stepped one differs at "core"; stepping and jumping agree system by system at every hour of a day; save, load into a fresh setup, equal; probes count 100 updates each, 0 ns off and more than 0 ns on, the state hash unchanged by timing.

## Not in this spec

- Per-chunk probes and timers (there are no chunks yet), a debug overlay in the window, a log file, a frame profiler (item 67's remainder).
- Hashing every tick in the tool (every hour by default; `--every 1` does it).
- The testing harness: each seed run twice and diffed, `SIM_BUGGIFY` injection sites (item 4).

## Open

- `--mode saveload` rebuilds the world per check: a save of the setup to reload from would make it cheap, which is also what spec 06's load into a candidate wants.
