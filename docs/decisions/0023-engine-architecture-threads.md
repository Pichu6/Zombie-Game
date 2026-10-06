# 0023 · Engine architecture and CPU threading

- **Date:** 2026-10-06
- **Status:** Proposal (Claude, technical lead; not yet confirmed by the user)
- **Design:** [engine-architecture](../design/engine-architecture.md)

## Context

Pichu asked how the engine works at the hardware level and how work is assigned to CPU cores. [0002](0002-engine-stack.md) lists libraries but not threads, timing or budgets. Vulkan is only the GPU API, not the engine.

## Proposal

1. Fixed thread set: main (input), workers (cores − 2), render, I/O, audio. No threads created during play.
2. A job system (enkiTS, zlib licence) with a phase scheduler: each sim tick runs Commands, Fields, Sense, Think, Act, Resolve, Warm/Cold, Publish, with barriers between phases.
3. Parallel phases read the previous tick and write only their own entity or per-worker request buffers; a short serial Resolve applies requests sorted by (system order, entity id). Results must not depend on worker count (golden-hash test with 1, 2 and 8 workers).
4. Fixed sim tick (0.5 game s, about 20 ms wall at 1×), render at display rate from interpolated snapshots.
5. Per-tick budgets by detail level: hot ≤ 60 %, warm ≤ 15 % (round-robin), cold/macro ≤ 10 % (deadline jobs), slack ≥ 15 %. Over budget slows game time; rules are never skipped.
6. Data-oriented layout per chunk (component arrays, per-worker scratch allocators), adopted system by system when Tracy shows it is hot.
7. Order: Tier 0 stays single-threaded but follows the phase rules; then job system, then sim/render split, then streaming, then layer budgets.

## Consequences

- Adds enkiTS and Tracy to the [0002](0002-engine-stack.md) stack.
- New systems must be written as phase jobs with request buffers, not as code that edits other entities directly.
