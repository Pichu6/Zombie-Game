# 0041 · Two-phase tick over chunk islands; asynchronous work committed by tick

- **Date:** 2026-10-07
- **Status:** Decided by the user ("yes, what is best" to the recommended model in [research/threading-and-partition.md](../research/threading-and-partition.md) §4).
- **Note:** written as 0022 on 2026-10-07 in a local session and renumbered on merge; it confirms and supersedes the threading part of [0042](0023-engine-architecture-threads.md) (a Proposal), which keeps its fixed thread set and job system as the implementation route.
- **Extends:** [0001](0001-own-engine.md), [0005](0005-finite-world-parity.md) (replay by hash), [spec 05](../tier0/specs/05-clock.md), [spec 27](../tier0/specs/27-detail-levels.md).

## Decisions

1. **Two-phase tick.** Every tick is a parallel pre-update of chunk islands on a frozen copy of the previous tick, where a chunk writes only its own entities and an outbox of keyed messages, followed by a serial commit that applies messages and structural changes in chunk and entity id order.
2. **Fields cross borders through the previous tick.** Noise, light and smoke are double-buffered; a chunk reads its neighbours' border cells from the old buffer and writes only its own cells.
3. **Asynchronous work commits by tick, never by wall-clock.** Pathfinding, generation on approach, cold-region jumps and save snapshots run on workers from immutable snapshots and are applied at a commit tick chosen from state; if not ready, the loop waits.
4. **Ids are monotonic and never recycled** within a run; every container iterated in an update is id-ordered.
5. **The replay hash is independent of the worker count.** The same seed and input log with 1, 2, 3, 7 and 16 workers give the same per-tick hashes; this is a standing CI test from the first chunk split, with scheduler and asynchronous-latency fuzzing.
6. **Floating-point hygiene** in the simulation library even with our own deterministic maths: no FMA contraction, no fast-math, own transcendental functions, the FP control word asserted each tick on Windows.
7. **Rejected:** independent region clocks (Folia), a free job graph with read/write declarations as the gameplay model (Bevy, Unity DOTS), a checkerboard update order for entities (Noita), and locks or atomics on shared simulation state.

## Why

Factorio and Paradox are the only precedents that parallelise and keep determinism, and both do it this way: islands or systems compute in parallel from a frozen state, and observable changes are merged on one thread in a fixed order. The gains are modest and memory-bound (Factorio 20 to 40 % then cache-bound; Crusader Kings 3 about 5 of 16 cores), so the value for us is headroom and asynchronous work that no longer stalls the loop. Songs of Syx and Dwarf Fortress show the cost of deciding late.

## Consequences

- Tier 0 is unchanged in behaviour: the phases exist with one chunk and one worker ([spec 05](../tier0/specs/05-clock.md) rule 8), which is where the worker-count test starts.
- Chunk data is laid out per chunk (structure of arrays, chunk-local arenas) from the first chunk split, so the parallel phase is not slower than serial.
- Every new system states its phase and its read and write sets; a static table checks that no two systems in the same parallel phase overlap.
- Details, invariants and tests: [research/threading-and-partition.md](../research/threading-and-partition.md) §3 and §5.
