# Threading and partition model

**Date:** 2026-10-07. **Status:** research note, not a decision; its recommended model was decided by the user the same day as [0041](../decisions/0041-threading-and-partition.md). **Feeds:** [engine architecture](../design/engine-architecture.md), [0041](../decisions/0041-threading-and-partition.md), [spec 05](../tier0/specs/05-clock.md), [spec 06](../tier0/specs/06-save-replay.md), [spec 27](../tier0/specs/27-detail-levels.md), [spec 40](../tier0/specs/40-time.md). **Evidence:** primary pages (Factorio Friday Facts, Paradox ACCU slides and dev diaries, engine docs, Box2D, Age of Empires article) read unless marked (S) secondary or (W) weak in the sources; CK3 dev diary 36 was blocked; Songs of Syx's statements could not be found independently and are unverified; the ACCU and GDC videos were not watched. **IDs:** TP.

Follows [pop-simulation-and-persistence.md](pop-simulation-and-persistence.md): Songs of Syx's one big loop and Dwarf Fortress's single thread are both ceilings their developers regret, and both were hard to retrofit. We want the model fixed before the first chunk code ([spec 27](../tier0/specs/27-detail-levels.md)), without giving up replay by hash ([spec 06](../tier0/specs/06-save-replay.md)) or the fixed update order ([spec 05](../tier0/specs/05-clock.md)).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TP-1 | Factorio keeps its main update single-threaded and parallelises only belt groups that never interact, merging wake-up requests on the main thread deterministically by group order number. | sourced | https://factorio.com/blog/post/fff-364 |
| TP-2 | Factorio's belt threading cut the belt update from 4 ms to 1.6 ms and gave 20 to 40 % on megabases, then saturated memory: "limited by memory access", not cores. | sourced | https://factorio.com/blog/post/fff-364 |
| TP-3 | Factorio's 2017 attempt to run trains, electric networks and belts as three concurrent tasks was slower than serial because threads invalidated each other's cache. | sourced | https://factorio.com/blog/post/fff-215 |
| TP-4 | Factorio's 2016 plan: 3×3 chunk groups updated in 9 sequential passes, each thread keeping its own deltas merged and applied in one thread afterwards. | sourced | https://factorio.com/blog/post/fff-151 |
| TP-5 | Paradox (CK3) splits each system into a parallel read phase on a frozen state writing only private per-entity data and a serial write phase in deterministic id order. | sourced | https://accu.org/conf-docs/PDFs_2023/XMultiThreadingModelinParadoxGamesPastPresentandFuture.pdf |
| TP-6 | CK3 averages about 5 of 16 cores busy; 32-core and 8-core machines tick about the same; succession got 3× faster; script volume is the remaining cost. | sourced | https://devtrackers.gg/crusader-kings/p/0fbce8e3-dev-diary-187-performance-optimization |
| TP-7 | Victoria 3 splits employment into a parallel and a serial phase with a sort step between them to keep ordering deterministic across clients. | sourced | https://www.paradoxinteractive.com/games/victoria-3/news/dev-diary-76-performance |
| TP-8 | Stellaris stores production outputs separately and consolidates afterwards instead of locking; its "Blorg shot first" desync came from shared state. | sourced | https://devtrackers.gg/stellaris/p/25864744-stellaris-dev-diary-181-threading-and-loading-times |
| TP-9 | Dyson Sphere Program parallelises by task type in phases with barriers and work stealing: logic frame 22 ms to 11.7 ms, phase hand-off 6.5 µs. | sourced | https://devtrackers.gg/dyson-sphere-program/p/81e22586-dev-log-the-new-multithreading-framework |
| TP-10 | Noita updates 64×64 pixel chunks in a four-pass checkerboard, each pass updating every other chunk plus a 32-pixel margin, with no locks. | reported | https://80.lv/articles/noita-a-game-based-on-falling-sand-simulation/ |
| TP-11 | Minecraft Folia gives each region its own tick loop with no main thread and no determinism; regions must merge and split, and it needs 16 cores. | sourced | https://docs.papermc.io/folia/reference/overview |
| TP-12 | Unity's EntityCommandBuffer.ParallelWriter records commands with a sort key and plays them back sorted on one thread, making the order deterministic. | sourced | https://docs.unity3d.com/Packages/com.unity.entities@1.0/manual/ecs-workflow-optimize-systems.html |
| TP-13 | Bevy's parallel system order is nondeterministic unless edges are explicit (with an ambiguity error mode); flecs keeps the world read-only during progress and flushes queued changes at sync points. | sourced | https://www.flecs.dev/flecs/Systems.html |
| TP-14 | Box2D v3 is deterministic across x64 and ARM, MSVC, GCC and Clang, multithreaded, by graph-colouring the solver, no FMA contraction, no fast-math and its own sinf, cosf, atan2f. | sourced | https://box2d.org/posts/2024/08/determinism/ |
| TP-15 | Rapier's cross-platform determinism cannot be enabled together with its SIMD and parallel features. | sourced | https://rapier.rs/docs/user_guides/rust/determinism/ |
| TP-16 | oneTBB parallel_deterministic_reduce performs the same split and join operations for any thread count (static partitioners only). | sourced | https://oneapi-spec.uxlfoundation.org/specifications/oneapi/latest/elements/onetbb/source/algorithms/functions/parallel_deterministic_reduce_func |
| TP-17 | Age of Empires ran lockstep with the identical simulation on all clients and a separate RNG for cosmetics; desyncs kept slipping through. | sourced | https://www.gamedeveloper.com/programming/1500-archers-on-a-28-8-network-programming-in-age-of-empires-and-beyond |
| TP-18 | Dwarf Fortress, RimWorld and Oxygen Not Included thread rendering only; Klei says ONI's simulation is almost entirely cache/memory fetching. | reported | https://kleiforums.com/forums/topic/145779-multi-core-support/ |
| TP-19 | Oxide's Nitrous engine, the one that went all in on free jobs, gave up determinism. | reported | https://www.gamingonlinux.com/2015/03/ashes-of-the-singularity-strategy-game-from-stardock-oxide-should-see-a-linux-release/comment_id=34377 |
| TP-20 | Nobody parallelises the update loop freely and keeps determinism; the gains are modest and memory-bound, so for us the value is headroom and clean asynchronous work, not throughput (from TP-1 to TP-19). | estimate | this note |
| TP-21 | Recommended model: two-phase tick over chunk islands: parallel pre-update on a frozen world writing own entities and a keyed outbox, then a serial commit in chunk and entity id order. | estimate | this note |
| TP-22 | Fields are double-buffered and read the previous tick across chunk borders (ghost cells), a 0.5 s latency nobody notices. | estimate | https://edoras.sdsu.edu/~mthomas/f17.705/topics/iter_solv/gropp_iter_solv/kjoldtad_snir_2010_ghostcell.pdf |
| TP-23 | Asynchronous work (paths, generation, cold jumps, saves) runs from immutable snapshots and commits at request tick + fixed latency L; the main thread waits if it is not ready; wall-clock never enters. | estimate | this note |
| TP-24 | At 0.5 s hot ticks the budget is about 30× Factorio's 16.7 ms tick; the serial commit should stay under a tenth of the tick. | derived | this note |
| TP-25 | Determinism test: the same seed and input log with 1, 2, 3, 7 and 16 workers must give identical per-tick hashes, plus scheduler and async-latency fuzzing and an MSVC/GCC/Clang matrix. | estimate | this note |
| TP-26 | Rejected: independent region clocks (Folia), a free job graph as the gameplay model (Bevy, DOTS), Noita's checkerboard for entities, and locks or atomics on shared state. | estimate | this note |
| TP-27 | Ids are a monotonic counter, never recycled; every container iterated in an update is id-ordered. | estimate | https://www.flecs.dev/flecs/Systems.html |

## 1. Answer in one page

1. **Nobody parallelises the update loop freely and keeps determinism.** Factorio keeps the main update single-threaded and parallelises only islands (belt groups that provably never interact), merging their wake-up requests on the main thread "in a deterministic way based on the group update order number". Paradox's current model (Crusader Kings 3) splits every system into a parallel read phase on a frozen state and a serial write phase in id order: "every parallel thread sees the same visible game state, like it's frozen". The one engine that went all in on free jobs (Oxide's Nitrous) gave up determinism.
2. **The gains are modest and memory-bound.** Factorio's belt threading gave 20 to 40 % on huge bases and then saturated the cache ("Factorio is not limited by cores; it's limited by memory access"); Crusader Kings 3 averages about 5 of 16 cores busy; Klei says Oxygen Not Included's simulation "is almost entirely cache/memory fetching". The value of a parallel model for us is headroom and clean asynchronous work (pathfinding, generation, saves, cold jumps), not raw throughput.
3. **Our shape fits the Factorio 2016 plan exactly:** chunks compute in parallel from a snapshot of the previous tick, writing only their own entities and an outbox of messages; a serial commit applies the messages in a fixed key order. Fields cross borders by reading the previous tick's buffer (ghost cells), which hides a 0.5 s latency nobody can notice.
4. **Asynchronous work is committed at a tick chosen from state, never from completion time.** A path or a cold jump requested at tick t is applied at t + L; if it is not done, the main thread waits. Wall-clock never enters the simulation (Age of Empires, Factorio, Paradox all state this).
5. **Determinism is tested, not assumed:** the same seed and input log with 1, 2, 3, 7 and 16 workers must give the same per-tick hash sequence, with a debug executor that reorders tasks and delays asynchronous results.
6. **Rejected:** independent region clocks (Minecraft's Folia: no main thread, no determinism, most of its complexity is merging and splitting regions); a free job graph with read/write declarations as the gameplay model (Bevy, Unity DOTS, Paradox's future plan: "explicit data usage declaration makes design iteration more expensive"); Noita's four-pass checkerboard for entities (good for fields, bakes a spatial order into gameplay); locks and atomics on shared state (Stellaris's "Blorg shot first" desync).

## 2. How others do it

| Source | What is parallel | How determinism is kept | Result |
| --- | --- | --- | --- |
| **Factorio, FFF-151 (2016 plan)** | 3×3 chunk groups; all chunks with the same group number update in parallel, groups 1 to 9 in sequence | "Every thread keeps its own list of changes to be applied later, we call these deltas. These deltas are merged at the end of the multithreaded phase, and applied in a single thread" | Plan only |
| **Factorio, FFF-215 (2017)** | Trains, electric networks and belts as three concurrent tasks | Not the problem; "the threads are invalidating each other's cache all the time" | Slower than serial; per-task allocators needed, no time before 1.0 |
| **Factorio, FFF-364 (2021, shipped)** | Belts grouped into lines that never interact | Threads never wake entities; wake-up requests go to a list merged on the main thread by group order | Belt update 4 ms to 1.6 ms; 20 to 40 % on megabases; then cache-bound |
| **Factorio forum (Rseding91)** | | "Single-core performance has been optimized to the point where it's almost always waiting for RAM" | Hacked threading of the top five costs estimated under 10 % |
| **Paradox, ACCU 2023 (Ropert) and CK3 dev diary 187** | Old model: each subsystem a `parallel_for` over entities, combat and units stayed sequential. New model: parallel pre-update writes only private per-entity data; serial write phase in deterministic id order | "We need a deterministic order of observable changes so all clients have the same game state"; Stellaris stores production outputs separately and consolidates afterwards instead of locking | Succession 3× faster; about 5 of 16 cores used; 32-core and 8-core machines tick about the same; script volume is the remaining cost |
| **Victoria 3 dev diary 76** | Tick as tasks with dependency info; employment split into a parallel and a serial phase | A sort step between the phases "to ensure deterministic ordering across game clients" | Tasks still cannot run in parallel in general |
| **Dyson Sphere Program dev log (2025)** | By task type and entity, not by region; phases with barriers; work stealing inside a phase | Not discussed (single player); each entity writes its own slot | Logic frame 22 ms to 11.7 ms; phase hand-off 6.5 µs |
| **Noita (GDC 2019)** | 64×64 pixel chunks in a four-pass checkerboard, each pass updating every other chunk plus a 32-pixel margin | Non-overlapping writes per pass; no locks | Falling sand at scale |
| **Minecraft Folia (PaperMC)** | Regions of nearby chunks each with its own tick loop and clock, no main thread | None; regions must merge when adjacent and split when disconnected; global state "considered broken" | Needs 16 cores; helps only when players are spread out |
| **Unity DOTS (Cities: Skylines 2)** | Jobs over entities with Burst | `EntityCommandBuffer.ParallelWriter` records commands with a sort key; playback sorts by key on one thread, "the sorting makes the playback order deterministic" | No published determinism claim for the game |
| **Bevy, flecs** | Systems in parallel from declared read and write sets | Bevy: order is "nondeterministic" unless edges are explicit, with an ambiguity error mode; flecs: world read-only during progress, changes queued and flushed at sync points, systems ordered by id | Pattern: declare, stage, defer structural changes, replay in key order |
| **Age of Empires (1500 archers)** | None in the simulation | Lockstep: "all of the users executing exactly the same simulation"; separate RNG for cosmetics; "there was always one more thing that slipped just under the radar" | The desync-hunting lesson |
| **Box2D v3 (Catto 2024)** | Solver by graph colouring so each colour touches disjoint bodies | No FMA (`-ffp-contract=off`), no fast-math, own `sinf`, `cosf`, `atan2f`; workers write thread-local bit arrays merged by bitwise OR on the main thread | Deterministic across x64 and ARM, MSVC, GCC and Clang, multithreaded |
| **Rapier** | | Cross-platform determinism "cannot be enabled at the same time as the simd and parallel features" | Determinism and free parallelism conflict |
| **oneTBB `parallel_deterministic_reduce`** | Reductions | "Executes the same set of split and join operations no matter how many threads participate"; only static partitioners | The formal "fixed reduction tree" |
| **Dwarf Fortress, RimWorld, Oxygen Not Included** | Rendering only (DF 50.09 added experimental line-of-sight threading) | | Tarn Adams: "we are certainly at the edge of what we can currently support"; Klei: memory-bound, more compute would not help |

Songs of Syx's own statements (one loop, 30 frames per second under load, path threads added later) are in [pop-simulation-and-persistence.md §2.2](pop-simulation-and-persistence.md#22-songs-of-syx), from the developer's Steam replies; the threading pass could not find them independently and marks them as unverified.

## 3. Recommended model

**Two-phase tick over chunk islands, with messages committed in key order.** Chunks are the partition ([spec 27](../tier0/specs/27-detail-levels.md)). The hot tick (0.5 s) becomes:

1. **Begin (serial).** Apply this tick's input commands; apply asynchronous results whose commit tick is now; build the work list as hot chunks sorted by chunk id.
2. **Fields (parallel, read-only on state).** Noise, light and smoke are double-buffered: read tick t−1 for every chunk including the neighbours' border cells (the ghost-cell pattern, Kjolstad and Snir), write tick t into the other buffer. Each chunk owns its cells; cross-border reads are reads of the old buffer.
3. **Pre-update (parallel, frozen world).** Each chunk updates its entities in id order, reading anything (own chunk, neighbours, fields at t) and writing only its own entities' state and an **outbox** of messages: a strike, a sound emitted, an entity transfer, a door opened, a spawn request. Message key = (destination chunk, source chunk, source entity id, sequence). Draws come from the keyed Philox streams ([spec 02](../tier0/specs/02-draw.md)), which are scheduler-independent by construction. No shared-heap allocation in this phase: per-chunk arenas, structure-of-arrays per chunk (Factorio's cache lesson).
4. **Commit (serial, id order).** For each chunk by id, apply its inbox sorted by key (Factorio's wake-up list, Unity's sort-key playback); then structural changes in the same order: entity creation and destruction, chunk transfers, promotion and demotion. Entity and chunk ids are a monotonic counter, never recycled (flecs's warning).
5. **Asynchronous results (serial, same place).** Pathfinding, generation on approach, cold-region jumps ([catch-up-jump-forms.md](catch-up-jump-forms.md)) and save snapshots run on workers from **immutable snapshots** taken at request time, and are consumed only at their commit tick = request tick + a fixed latency L. If not ready, the main thread waits. Save: copy the committed state (or fork on POSIX, as Factorio does) and serialise the copy on a worker; the copy is the only serial part.
6. **Warm and cold (serial, or parallel by region with the same outbox rule).** Hourly and daily updates are bigger tasks with the same discipline; sums across regions are folded serially in id order or with a fixed reduction tree.
7. **Hash (serial).** State hash every N ticks, with per-system sub-hashes (entities, fields, inboxes, draw counters) so the first divergent tick names the system (Age of Empires' lesson).

What stays serial: input, commit, id allocation, promotion and demotion, global statistics, the save copy.

**Fit with the current specs.** [Spec 05](../tier0/specs/05-clock.md) rule 2 already reads start-of-tick values where order would matter and forbids hash-container iteration (rule 7); this model generalises "read the previous tick" to "pre-update reads a frozen world" and adds the outbox and commit. Nothing in Tier 0 (one hot chunk) changes; the phases exist with one chunk and one worker, which is how the thread-count test starts.

**Expected numbers.** At 0.5 s hot ticks the budget is about 30× Factorio's 16.7 ms, so the parallel phases are for headroom; the serial commit should stay under a tenth of the tick if messages are compact. Expect Factorio-like gains (tens of percent) from chunk parallelism, and the real win from asynchronous work no longer stalling the loop.

## 4. Decided

Recorded as [0041](../decisions/0041-threading-and-partition.md) on 2026-10-07 with these items:

- The simulation runs as a two-phase tick: parallel pre-update of chunk islands on a frozen world, serial commit of keyed messages in chunk and entity id order.
- Fields are double-buffered and read the previous tick across borders.
- Asynchronous work (paths, generation, cold jumps, saves) computes from immutable snapshots and commits at a tick chosen from state; the loop waits rather than reading wall-clock.
- Entity and chunk ids are monotonic and never recycled; every container iterated in an update is id-ordered.
- The replay hash is identical for any worker count; this is a standing CI test from the first chunk split.
- Floating-point hygiene even with our own maths: no FMA contraction, no fast-math, own transcendental functions, the FP control word asserted each tick on Windows.

## 5. Invariants and tests

Invariants (asserted in debug, enforced by API):

1. No system reads wall-clock, thread id, pointer values or hash-map order inside the simulation.
2. Pre-update writes are confined to the chunk's own entities and its outbox; a debug write barrier tags chunk-owned memory with the chunk id and asserts writer = owner.
3. Every message carries (source chunk, source entity, sequence); inbox order is a pure function of those keys.
4. Ids are monotonic and never recycled within a run.
5. Draw streams are keyed per entity and purpose ([spec 02](../tier0/specs/02-draw.md)); cosmetic randomness is a separate stream.
6. Asynchronous results apply only at their commit tick.
7. The field phase reads buffer t−1 and writes buffer t only.
8. Reductions across parallel work use a fixed tree or a serial id-ordered fold.

Tests (CI, every commit, from the first chunk split):

- **Worker sweep:** 1, 2, 3, 7 and 16 workers, same seed and input log, identical per-tick hashes (odd and prime counts perturb the partition).
- **Compiler and platform matrix:** MSVC, GCC and Clang on Windows and Linux (ARM64 when a target exists), as Box2D runs.
- **Scheduler fuzz:** a debug executor that delays and reorders tasks within a phase; hashes unchanged.
- **Asynchronous-latency fuzz:** random completion delays on workers; hashes unchanged.
- **Save and load round trip** at tick k, continued to k+n, equal to the uninterrupted run (exists at day granularity, [spec 06](../tier0/specs/06-save-replay.md)).
- **Ambiguity table:** a static table of phase → (reads, writes) per system; a test fails on overlapping writes or a write/read overlap within a parallel phase (Bevy's ambiguity error, by hand).
- **Border stress:** a scripted replay with bodies, noise and light straddling chunk borders and promotion and demotion every tick.

## 6. Sources

Primary unless marked (S = secondary, W = weak).

- Factorio: https://factorio.com/blog/post/fff-151 (chunk groups and deltas); https://factorio.com/blog/post/fff-215 (parallel tasks slower, cache invalidation); https://factorio.com/blog/post/fff-364 (belt groups, deterministic wake-ups, 20 to 40 %); forum posts by Rseding91 https://forums.factorio.com/viewtopic.php?p=306371 , https://forums.factorio.com/viewtopic.php?p=243339 , https://forums.factorio.com/viewtopic.php?p=537333 ; non-blocking save (S) https://forums.factorio.com/viewtopic.php?p=341647 ; 60 updates per second (S) https://wiki.factorio.com/Time
- Paradox: Ropert, "Multi-threading model in Paradox games: past, present and future", ACCU 2023 slides https://accu.org/conf-docs/PDFs_2023/XMultiThreadingModelinParadoxGamesPastPresentandFuture.pdf and video page https://accu.org/video/spring-2023-day-3/ropert ; CK3 dev diary 187 (mirror) https://devtrackers.gg/crusader-kings/p/0fbce8e3-dev-diary-187-performance-optimization ; Victoria 3 dev diary 76 https://www.paradoxinteractive.com/games/victoria-3/news/dev-diary-76-performance ; Stellaris dev diary 181 (mirror) https://devtrackers.gg/stellaris/p/25864744-stellaris-dev-diary-181-threading-and-loading-times ; CK3 dev diary 36 https://forum.paradoxplaza.com/forum/threads/ck3-dev-diary-36-gotta-go-fast.1408620/ (blocked; the "+75 %" figure quoted elsewhere is unverified)
- Dyson Sphere Program dev log (mirror) https://devtrackers.gg/dyson-sphere-program/p/81e22586-dev-log-the-new-multithreading-framework ; AMD repost (S) https://gpuopen.com/learn/dyson-sphere-program-new-multithreading/
- Noita, Nolla interview https://80.lv/articles/noita-a-game-based-on-falling-sand-simulation/ ; GDC 2019 "Exploring the Tech and Design of Noita" (video, via the wiki index, S) https://noita.wiki.gg/wiki/Official_Interviews_and_Videos
- Folia: https://docs.papermc.io/folia/reference/overview and https://github.com/PaperMC/Folia
- Unity entity command buffer sort keys: https://docs.unity3d.com/Packages/com.unity.entities@1.0/manual/ecs-workflow-optimize-systems.html ; Cities: Skylines 2 on DOTS (S) https://80.lv/articles/cities-skylines-2-doesn-t-have-limit-for-people-it-can-track
- Bevy ambiguity: https://github.com/bevyengine/bevy/blob/main/examples/ecs/nondeterministic_system_order.rs and https://dev-docs.bevyengine.org/bevy/ecs/schedule/enum.ScheduleBuildError.html ; flecs systems and pipelines https://www.flecs.dev/flecs/Systems.html
- Age of Empires lockstep: https://www.gamedeveloper.com/programming/1500-archers-on-a-28-8-network-programming-in-age-of-empires-and-beyond
- Floating point: https://gafferongames.com/post/floating_point_determinism/ and https://gafferongames.com/post/deterministic_lockstep/ ; Box2D v3 determinism https://box2d.org/posts/2024/08/determinism/ ; Rapier https://rapier.rs/docs/user_guides/rust/determinism/
- Task systems: oneTBB deterministic reduce https://oneapi-spec.uxlfoundation.org/specifications/oneapi/latest/elements/onetbb/source/algorithms/functions/parallel_deterministic_reduce_func ; enkiTS https://github.com/dougbinks/enkiTS ; Taskflow (S) https://taskflow.github.io/
- Parallel discrete-event simulation and ghost cells: Fujimoto, null messages https://sigsim.acm.org/conf/mskr/Courseware/Fujimoto/Slides/FujimotoSlides-06-NullMessages.pdf ; Kjolstad and Snir, "Ghost cell pattern" https://edoras.sdsu.edu/~mthomas/f17.705/topics/iter_solv/gropp_iter_solv/kjoldtad_snir_2010_ghostcell.pdf
- Single-thread ceilings: Tarn Adams https://stackoverflow.blog/2021/07/28/700000-lines-of-code-20-years-and-one-developer-how-dwarf-fortress-is-built/ ; DF frame-time causes (S) https://dwarffortresswiki.org/index.php/v0.34:Maximizing_framerate ; DF 50.09 experimental threading (S) https://ui.lemm.ee/post/510166 ; RimWorld 1.5 pawn rendering thread (S) https://tildes.net/~games/1ewx/rimworld_anomaly_expansion_and_update_1_5_announced ; Klei on Oxygen Not Included (S, snippet only) https://kleiforums.com/forums/topic/145779-multi-core-support/ ; Supreme Commander (W) https://forum.faforever.xyz/post/53458 ; Nitrous not deterministic (W) https://www.gamingonlinux.com/2015/03/ashes-of-the-singularity-strategy-game-from-stardock-oxide-should-see-a-linux-release/comment_id=34377
