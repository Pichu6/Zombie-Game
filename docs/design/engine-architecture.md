# Engine architecture and CPU threading

**Status:** Proposal ([0023](../decisions/0023-engine-architecture-threads.md)). Builds on [0002](../decisions/0002-engine-stack.md) (libraries), the [simulation contract](world-simulation.md#2-the-simulation-contract-proposal) and [spec 27](../tier0/specs/27-detail-levels.md) (detail levels).

## 1. What Vulkan is, and what the engine is

Vulkan is only the language the program uses to give work to the graphics card. The GPU driver already ships the Vulkan runtime; the Vulkan SDK adds headers, validation layers and the shader compiler for development. Vulkan draws nothing by itself and knows nothing about time, input, files, sound or zombies. The engine is everything around it:

| Module | Job | Ours or library |
| --- | --- | --- |
| Platform | Window, input, timers, threads, files | SDL3 |
| Job system | Splits each tick's work across CPU cores | enkiTS (Proposal), our phase scheduler on top |
| Simulation core | The game: body, items, power, noise, light, AI, the dead, layers | Ours (`engine/sim`), no window |
| Data layout | Entities as flat arrays per component, so cores stream through memory | Ours |
| Assets | Converts glTF and textures offline to our format; streams them at runtime | cgltf/fastgltf, meshoptimizer |
| Renderer | Builds GPU command lists from a sim snapshot; skinning and clothing wrap in compute | Ours, on vk-bootstrap + VMA |
| Physics, navigation, animation | Collision, paths, skeletons | Jolt, Recast/Detour, ozz-animation |
| Audio | Mixing, occlusion | miniaudio, Steam Audio |
| Save | Deterministic state + replay | Ours (spec 06) |
| Tools | Inspectors, overlays, profiler | Dear ImGui, Tracy |

## 2. Threads

A thread is one line of work the OS runs on a core. The engine creates a fixed set at start-up and never creates threads during play.

| Thread | Count | Does | Never does |
| --- | --- | --- | --- |
| Main | 1 | OS events and input (SDL needs the main thread), turns input into sim commands, starts each tick and frame | Heavy work |
| Workers | cores − 2 (min 1) | Run jobs: sim phases, AI, fields, culling, animation, asset decoding | Block on files or the GPU |
| Render | 1 | Reads the latest sim snapshot, records Vulkan command buffers, submits and presents | Write sim state |
| I/O | 1 | Reads and writes files: asset streaming, chunk wake/sleep, saves | Game logic |
| Audio | 1 | Owned by miniaudio; mixes from a queue of sound events | Read sim state directly |

The main and render threads also run jobs while they wait. Workers steal jobs from each other when idle.

## 3. Time: fixed sim tick, free frame rate

- The sim advances in fixed ticks of 0.5 game s (Tier 0 default). At the usual speed of 1 real hour = 1 game day, that is 48 ticks per real second (the speed is the player's choice, [0049](../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md); at 1:1 a tick is 0.5 s of wall time; the budget that matters is fast-forward), so one tick has about **20 ms** of wall time at 1×. Fast-forward runs more ticks per second, so the budget shrinks.
- The renderer draws at the display's rate from the last two snapshots, interpolated. A slow frame never slows the sim; a slow tick drops interpolation smoothness, not correctness.
- A snapshot is an immutable copy of what views need (positions, poses, lights, UI facts), written at the end of a tick. Views only read ([contract rule 8](world-simulation.md#2-the-simulation-contract-proposal)).
- View-side smoothing of snapshot values (camera, procedural animation, UI) runs on the frame time and never feeds the sim: [research/second-order-dynamics](../research/second-order-dynamics.md).

## 4. One tick, in phases

Each phase is a set of jobs; a phase starts only when the previous one is done (a barrier). Parallel phases read only the **previous tick's** state and write only their **own** entity's state or a per-worker output buffer (contract rule 5).

| # | Phase | Parallel over | Writes |
| --- | --- | --- | --- |
| 1 | Commands | — (serial) | Player and possession commands, scheduled events due this tick |
| 2 | Fields | rooms / cells | Noise and light fields, power network solve per network |
| 3 | Sense | pops and walkers | What each one heard and saw |
| 4 | Think | pops and walkers | Chosen intent (utility AI, walker brain) |
| 5 | Act | bodies, executor | Own body state; transfer and damage **requests** to the worker's buffer |
| 6 | Resolve | — (serial) | Requests sorted by (system order, entity id), applied to the ledger and events (contract rule 3) |
| 7 | Warm and cold | chunks, regions | This tick's slice of warm chunks and macro work (section 6) |
| 8 | Publish | — | Snapshot for render, audio and UI; debug ledger check |

The serial phases are short: they only apply sorted results. Everything costly (fields, AI, bodies) is parallel.

## 5. Determinism with threads

Saves, replays and golden hashes need the same result for the same seed on any machine ([spec 06](../tier0/specs/06-save-replay.md)). Threads would break that if the order of work leaked into results, so:

1. Jobs never share writable state. Cross-entity effects are requests, merged in a fixed sort (phase 6), never applied by the job that made them.
2. Draws are keyed by entity and purpose (Philox), not by which thread runs first.
3. Sums over many entities (a noise total) use integers or a fixed reduction order, never "whichever worker finishes first".
4. **Test:** the same run with 1, 2 and 8 workers gives the same golden hash. This test lands with the job system.

## 6. Budgets for the three layers

Individual, City and Macro are views over one simulation; cost follows detail level, not camera ([spec 27](../tier0/specs/27-detail-levels.md)). Starting budgets per tick at 1×, to be measured and tuned with Tracy:

| Detail | What runs | When | Budget |
| --- | --- | --- | --- |
| Hot | Chunks near controlled bodies: phases 2-6 every tick | Every tick | ≤ 60 % |
| Warm | Neighbour chunks: needs, schedules, room graph | Hourly per chunk, spread round-robin so 1/N runs each tick | ≤ 15 % |
| Cold and Macro | Census rows, region stocks and flows, the outbreak model | One jump when read; macro step daily, split into low-priority jobs with a deadline at the day boundary | ≤ 10 % |
| Slack | Spikes, chunk wake/promotion, previews on idle cores | — | ≥ 15 % |

If hot work goes over budget, warm and macro slices wait (they have deadlines, not fixed slots); if a deadline is about to be missed, the sim slows game time rather than skip rules. Forecast previews (macro "estimate from N runs") run only on idle cores at lowest priority.

## 7. Data layout

- Entities are integer ids. Each component (body, position, inventory, AI memory) is a dense array; a system loops over the arrays it needs. No per-entity virtual calls or heap objects in hot loops.
- Arrays live per chunk, so waking or sleeping a chunk moves whole blocks and a worker's job touches one chunk's memory.
- Each worker has a linear scratch allocator, reset every tick: no `malloc` in the tick.
- Tier 0 code uses simple structs and `std::function` hooks (stage 6); that is fine single-threaded. It moves to this layout when profiling says a system is hot, one system at a time.

## 8. GPU side

- The render thread records with 2-3 frames in flight, so the CPU prepares frame N+1 while the GPU draws N.
- Workers can record secondary command buffers in parallel (one per worker per pass) when draw counts need it.
- Skinning, morphs and the clothing wrap run as compute on the GPU ([0003](../decisions/0003-clothing-wrap.md)); the CPU sends poses, not vertices.
- Memory through VMA; uploads through a transfer queue fed by the I/O thread.

## 9. Order of work

1. **Now (Tier 0):** single-threaded sim, but every system already follows the phase rules (read previous tick, requests, sorted resolve). Seed batches run as separate processes in parallel.
2. **Job system:** add enkiTS and the phase scheduler; parallelize fields, sense, think and act; the worker-count determinism test; Tracy zones per phase.
3. **Sim/render split:** snapshot and render thread, when the Vulkan renderer starts.
4. **Streaming:** I/O thread for chunk wake/sleep and assets, with the Tier 1 chunk split.
5. **Layer budgets:** warm round-robin and macro deadlines, when warm and macro systems exist.

## Open

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- ECS storage as archetypes or sparse sets? Stable ids favour sparse sets. ([pop-storage-and-lenses](../research/pop-storage-and-lenses.md))
- Two frames in flight (as niagara and Sascha Willems' examples use) or "2-3"? The research recommends 2. ([renderer](../research/renderer.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** two frames in flight.
- Minimum GPU: Vulkan 1.3 with buffer device address, descriptor indexing, dynamic rendering and synchronization2; mesh shaders optional? ([renderer](../research/renderer.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** yes, mesh shaders optional.
- Does any simulation rule need rigid-body physics, or does Jolt stay view-side ([ES-45](../research/engine-stack.md))? ([engine-stack](../research/engine-stack.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** physics is view-side unless a game rule needs it.
- Does the sim's grid pathing scale to open 3D ground, or do we add Detour, and if so do its float-based paths feed the sim? ([engine-stack](../research/engine-stack.md))
- Raw Vulkan with vk-bootstrap and VMA rather than SDL_gpu ([ES-47](../research/engine-stack.md)), to be confirmed in a decision superseding 0002. ([engine-stack](../research/engine-stack.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** raw Vulkan; SDL3 for window and input.
- Will a macOS build ever be a target? If so, recheck toml++ float parsing on libc++ and SDL_gpu's portability case. ([engine-stack](../research/engine-stack.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** x64 Windows and Linux only for now.
- Do the id-set and record-layout ratios hold on MSVC and a desktop CPU? Rerun the benchmark before the layout goes into a spec. ([ecs-memory-layout](../research/ecs-memory-layout.md))
- Which pop record fields are hot and which cold (spec 28)? Needs the realistic record that benchmark B1 waits for. ([ecs-memory-layout](../research/ecs-memory-layout.md))
- Should spec 01's float path become "required across compilers", now that GCC 13 and Clang 18 reproduce every MSVC golden? ([determinism](../research/determinism.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** yes, required and checked in CI on GCC and Clang.
- Are only x64 platforms in scope, or also ARM64, where FMA is in the base instruction set? ([determinism](../research/determinism.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** x64 only for now.
- Should a NaN in saved state be canonicalised, or treated as a bug (spec 06)? ([determinism](../research/determinism.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** a bug; the archive asserts.
- Where does the floating-point environment guard live once there are worker threads: per step, per job, or per worker at start? ([determinism](../research/determinism.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** per step now, per worker at start later.

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [clothing-wrap](../research/clothing-wrap.md): Clothing that follows the body, in our own engine
- [detail-levels](../research/detail-levels.md): Detail levels: one simulation, read and run in chunks
- [second-order-dynamics](../research/second-order-dynamics.md): Second-order dynamics for procedural motion
- [threading-and-partition](../research/threading-and-partition.md): Threading and partition model
- [renderer](../research/renderer.md): The renderer, at engineering level
- [engine-stack](../research/engine-stack.md): The engine stack behind decision 0002: licences, versions, determinism, alternatives
- [view-readability](../research/view-readability.md): Readability of the 2D top-down view: state, selection, alerts, overlays, and what our window lacks
- [ecs-memory-layout](../research/ecs-memory-layout.md): Entity storage and memory layout: archetypes, sparse sets, id sets, profiling
- [determinism](../research/determinism.md): Cross-compiler and cross-platform determinism: pitfalls and fixes
- [draw-call-and-mesh-optimizations](../research/draw-call-and-mesh-optimizations.md): Draw-call and mesh optimizations, from a voxel-engine video
