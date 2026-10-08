# Entity storage and memory layout: archetypes, sparse sets, id sets, profiling

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [engine architecture](../design/engine-architecture.md), [focus and compression](../design/focus-and-compression.md), [spec 28](../tier0/specs/28-pop-levels.md), [spec 29](../tier0/specs/29-embodied-pops.md), [spec 27](../tier0/specs/27-detail-levels.md), [spec 05](../tier0/specs/05-clock.md). **Evidence:** primary sources read from shallow git clones (EnTT, flecs, Bevy's `bevy_ecs`, CRoaring, Tracy; commits in Sources); Unity's Entities manual was not reachable and is `reported` from search summaries; our own code read at `origin/main` 2646536; a standalone micro-benchmark written for this note and run in a cloud container (method in section 4; the code is not in the repo). The benchmark was not run on MSVC. **IDs:** ECS.

Two questions were open: "ECS storage as archetypes or sparse sets? Stable ids favour sparse sets" ([engine architecture](../design/engine-architecture.md#open)) and "Do Roaring-style compressed id sets beat plain sorted arrays at our roster sizes? Needs a measurement" ([focus and compression](../design/focus-and-compression.md#open)). This note reads how the main ECS libraries store entities, measures the id-set and record-layout choices at our sizes, and recommends storage per detail level.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| ECS-1 | EnTT is a sparse-set ECS: every component type has its own pool (a sparse array from entity to index plus packed arrays of entities and components); the registry does not need the component list in advance. | sourced | github.com/skypjack/entt docs/md/entity.md "Type-less and bitset-free", "Storage" @ e0f4c37 (v4.1.0) |
| ECS-2 | EnTT's sparse arrays are paged (4,096 entries per page by default, `ENTT_SPARSE_PAGE`) and packed component arrays are paged (1,024, `ENTT_PACKED_PAGE`) for pointer stability on growth; a sparse page is allocated for any id that lands in it, so memory follows the id range, not the count. | sourced | entt src/entt/config/config.h; src/entt/entity/sparse_set.hpp `assure_at_least` @ e0f4c37 |
| ECS-3 | EnTT's default entity is 32 bits: 20 bits of index and 12 bits of version (`entity_mask = 0xFFFFF`, `version_mask = 0xFFF`); released ids are recycled with the version bumped. 64-bit ids are supported (32+32). | sourced | entt src/entt/entity/entity.hpp lines 44-56; docs/md/entity.md "The Registry, the Entity and the Component" @ e0f4c37 |
| ECS-4 | EnTT removes by swap-and-pop by default, so a pool's packed order depends on the history of adds and removes; order can be restored with `sort`, and an in-place (tombstone) policy gives stable pointers, but owning groups refuse to compile with it. | sourced | entt docs/md/entity.md "Pointer stability", "In-place delete", "Sorting: is it possible?"; sparse_set.hpp `swap_and_pop`, `in_place_pop` @ e0f4c37 |
| ECS-5 | EnTT views iterate the smallest pool and look the others up through the sparse arrays; full-owning groups reorder the owned pools so several components are iterated as aligned packed arrays "with no jumps nor branches", at the cost of slower add and remove for those types. | sourced | entt docs/md/entity.md "Iteration order", "Groups", "Full-owning groups" @ e0f4c37 |
| ECS-6 | EnTT's registry is not thread-safe; different component sets may be iterated or changed in parallel, and one set may be iterated by many threads if nothing is added or removed meanwhile. | sourced | entt docs/md/entity.md "Multithreading" @ e0f4c37 |
| ECS-7 | flecs is archetype-based: entities with the same component set share a table, which gives contiguous component arrays to systems; Unity DOTS, Unreal Mass and Bevy are named as other archetype implementations. | sourced | github.com/SanderMertens/flecs docs/FAQ.md "What is an archetype?" @ 9e874bc (v4.1.6) |
| ECS-8 | A flecs entity id is 64 bits: the low 32 bits identify it, the high 32 bits hold a liveness count incremented when an id is recycled; deleted ids are reused by default. Id ranges can confine new and recycled ids to an interval. | sourced | flecs docs/EntitiesComponents.md (entity ids, deleting, "Ranges") @ 9e874bc |
| ECS-9 | flecs is a hybrid: a component can be marked `Sparse` (stored outside tables, stable pointers, still an archetype change on add/remove) or `DontFragment` (sparse storage that does not split tables), meant for components that are rare or change often. | sourced | flecs docs/ComponentTraits.md "Sparse trait", "DontFragment trait" @ 9e874bc |
| ECS-10 | In flecs, each `(ChildOf, parent)` pair is like a component, so children of different parents live in different tables; "the performance difference between a system that iterates over 1 vs. 1000 tables can be over an order of magnitude", and the degenerate case is one child per table. A non-fragmenting `Parent` storage exists for this. | sourced | flecs docs/HierarchiesManual.md lines 427-452, 463; docs/Relationships.md "Fragmentation" @ 9e874bc |
| ECS-11 | flecs refuses table changes during iteration (`LOCKED_STORAGE`); structural changes are deferred between `defer_begin` and `defer_end`. | sourced | flecs docs/FAQ.md (LOCKED_STORAGE) @ 9e874bc |
| ECS-12 | Bevy stores components in tables by default ("fast and cache friendly iteration, but slower adding and removing") and optionally in sparse sets ("fast adding and removing, but slower iteration"), chosen per component type. | sourced | github.com/bevyengine/bevy crates/bevy_ecs/README.md "Component Storage"; src/storage/mod.rs @ 59fed6a (0.20.0-dev) |
| ECS-13 | A Bevy `Entity` is a 32-bit index plus a 32-bit generation; the docs warn that generations wrap and can alias, so two different entities can compare equal. | sourced | bevy crates/bevy_ecs/src/entity/mod.rs `EntityIndex`, `EntityGeneration` "Aliasing" @ 59fed6a |
| ECS-14 | Unity Entities stores each archetype in fixed 16 KiB chunks; how many entities fit depends on the archetype's total component size. | reported | docs.unity3d.com/Packages/com.unity.entities@1.4/manual/concepts-archetypes.html (search summary) |
| ECS-15 | Our code stores entities by 64-bit id in `std::map`: bodies (`Bodies::bodies_`), walkers, the ledger's balances and parent links, beliefs, records, kept exceptions, rosters and each roster's stints. Iteration is in id order, as [spec 05](../tier0/specs/05-clock.md) rule 7 requires; no ECS library is used. | sourced | engine/sim/include/zs/body.h:253, dead.h:182, ledger.h:94 and 114, beliefs.h:54, pop_registry.h:177-188, roster.h:64 @ 2646536 |
| ECS-16 | `PopId` is 64 bits, never reused, and structured (top bit, 3 bits of generation, 4 bits per child number, founder slot), so pop ids are sparse in a 64-bit space; only founder slots are dense. | sourced | [spec 28](../tier0/specs/28-pop-levels.md) State table; engine/sim/include/zs/pop.h:13 |
| ECS-17 | The embodied set is already a fixed pool of body slots reserved at setup, with fixed ids, sized as the hot-set cap (recommendation 256). | sourced | [spec 29](../tier0/specs/29-embodied-pops.md) State and Open |
| ECS-18 | CRoaring splits 32-bit values into 2^16-value containers; a container is a sorted array up to 4,096 values (`DEFAULT_MAX_SIZE`), else an 8 KiB bitset; `run_optimize` converts to run containers where smaller. Licence Apache-2.0 or MIT; it dispatches to AVX2/AVX-512 at run time. | sourced | github.com/RoaringBitmap/CRoaring include/roaring/containers/array.h:38, bitset.h:41, include/roaring/roaring.h:615-621, src/containers/bitset.c:72, LICENSE @ f2b1a55 (v5.2.3) |
| ECS-19 | CRoaring has a 64-bit variant (`roaring64.h`) and a portable serialised format with a published specification shared by its Java, Go and Python implementations. | sourced | CRoaring README.md lines 79, 183-245, 481 @ f2b1a55 |
| ECS-20 | Measured, id sets of 100 random ids from a 10M universe: a sorted `std::vector<uint32_t>` beats CRoaring on everything: 4 vs 7.4 bytes per id, intersection 0.15 vs 1.65 µs, union 0.5 vs 7.8 µs, iteration 0.4 vs 6.7 ns per id. A 10M-bit bitset costs 1.25 MB per set. | sourced | this note's benchmark, section 4 (GCC 13.3 -O3; Clang 18 within 2x) |
| ECS-21 | Measured, 10,000 random ids (density 0.1%): CRoaring is half the memory of a 32-bit vector (2.1 vs 4 bytes per id) and faster to combine: intersection 11 µs vs 93 µs (branchy `std::set_intersection`) or 53 µs (branchless merge); union 30 vs 116 µs; membership 38 ns vs 87 ns (branchy) or 14 ns (branchless); iteration 0.49 vs 0.22 ns per id. | sourced | benchmark, section 4 (GCC) |
| ECS-22 | Measured, 1,000,000 random ids (density 10%): CRoaring 1.25 bytes per id (same as the bitset) vs 4; intersection 1.2 ms vs 5.7-10.6 ms (vector) vs 0.15 ms (bitset); union 0.18 ms vs 15 ms vs 0.12 ms; membership 33 ns vs 214 ns vs 1.8 ns. | sourced | benchmark, section 4 (GCC) |
| ECS-23 | Measured, ids in contiguous blocks of 1,000 (cohort slot blocks), after `run_optimize`: CRoaring takes 0.01 bytes per id; intersection of two 1M sets 16 µs vs 1.3 ms for the vector; of two 10k sets 0.08 µs vs 10 µs. | sourced | benchmark, section 4 (GCC) |
| ECS-24 | Measured, a small roster against a big set: 100 ids in 1M by a plain merge (`std::set_intersection`) 650 µs; by galloping or `lower_bound` 3 µs; CRoaring `and` 4 µs, `and_cardinality` 0.6 µs; probing a bitset of the big set 0.13 µs. 10,000 in 1M: galloping 590 µs, CRoaring 28 µs (cardinality 15 µs), bitset probe 10 µs. | sourced | benchmark, section 4 (GCC) |
| ECS-25 | Measured, a needs step (3 float updates and a threshold count) over pop-like records of 136 bytes (40 hot, 96 cold): at 1M records, `std::map` 296 ns per record, array of structs 14.8 ns, a 16-byte hot struct 2.6 ns, struct of arrays 0.7 ns. At 300 records: 5.1, 1.0, 0.9 and 0.5 ns. | sourced | benchmark, section 4 (GCC; Clang 271, 14.1, 1.9, 0.5 ns at 1M) |
| ECS-26 | Measured, random lookup by 64-bit id at 1M records: `std::map::find` 2.4 µs (GCC) or 0.68 µs (Clang); binary search on a sorted id column 0.25-0.36 µs; an index already resolved to a slot 1.7-1.9 ns. At 10,000 records: 122/57 ns, 79/36 ns, 1.3 ns. | sourced | benchmark, section 4 |
| ECS-27 | Measured, a sparse-set style join (iterate component B on half the entities, write A through B's entity list) at 1M: with A a float column, 2.7 ns per entity in swap-and-pop (shuffled) order, 0.7 ns sorted by slot, 0.29 ns as an aligned table; with A the 136-byte record, 33 ns shuffled and 15 ns sorted. At 10,000 all are under 1.3 ns. | sourced | benchmark, section 4 (GCC; Clang 2.3, 0.65, 0.29, 23.5, 12.4 ns) |
| ECS-28 | Tracy records a zone (start and end) for about 2.25 ns on its own benchmark; it has nanosecond timers via `rdtsc`, plots (`TracyPlot`), separate named frame sets (`FrameMarkNamed`), memory events (`TracyAlloc`/`TracyFree`), lock markup, fibers and an on-demand mode; a session is limited to 2^47 ns (1.6 days). Licence BSD-3. | sourced | github.com/wolfpld/tracy manual/tracy.tex sections "Performance impact" (line 298), "Marking frames", "Plotting data", "Memory profiling", limits (line 825); LICENSE @ afb16f6 (0.14.2) |
| ECS-29 | Tracy's call-stack capture costs tens of nanoseconds to microseconds per capture (34 ns to 30 µs in its table, by depth and CPU), so call stacks belong on rare zones, not per-entity ones. | sourced | tracy manual/tracy.tex "Collecting call stacks", table lines 2032-2038 @ afb16f6 |
| ECS-30 | A full pass over 100,000 records kept in a `std::map` built in promotion order costs about 25 ms (247 ns × 100k), more than the 20 ms tick at 1x; at 1M it is about 0.3 s. A struct-of-arrays pass over 1M costs about 0.7 ms. | derived | ECS-25; tick budget from [engine architecture](../design/engine-architecture.md) §3 |
| ECS-31 | A pass over the 256-slot body pool in a `std::map` costs about 1.3 µs (5 ns × 256), under 0.01% of a 20 ms tick: the hot set does not need a new layout for speed now. | derived | ECS-17, ECS-25 |
| ECS-32 | A bitset over slots beats a sorted 32-bit vector in memory once density exceeds 1/32 (1/(8d) bytes per id against 4), and beats CRoaring in speed at any density but not in memory below about 6% density, where CRoaring stores arrays. | derived | ECS-18, ECS-20 to ECS-22 |
| ECS-33 | A `std::map<uint64_t, Rec>` node holds 32 bytes of tree links plus the key and value, so a 136-byte record takes 176 bytes plus allocator overhead, about 1.3-1.4x the array size, and nodes allocated in promotion order are scattered, which is what makes ECS-25 and ECS-26 slow. | derived | libstdc++ `_Rb_tree_node` layout; ECS-25 |
| ECS-34 | General ECS libraries recycle ids (EnTT, flecs, Bevy), which conflicts with our rule that ids are never reused (threading note TP-27, [spec 28](../tier0/specs/28-pop-levels.md) rule 1); keeping their default needs a separate stable id anyway. | derived | ECS-3, ECS-8, ECS-13, ECS-16 |
| ECS-35 | An archetype store would fragment our groups the way flecs's `ChildOf` does: a household, workplace or chunk as a relationship gives one table per group, and our groups are many and small (workplace-sized rosters, spec 28 rule 11). | derived | ECS-10, [spec 28](../tier0/specs/28-pop-levels.md) rule 11 |
| ECS-36 | For our sizes the layout matters far more than the ECS model: hot/cold split and struct of arrays give 6-20x at 1M records (ECS-25), keeping a sparse-set's packed order sorted by slot gives 2-4x (ECS-27), and the table-versus-sparse-set gap is within 3x for columns. | derived | ECS-25, ECS-27 |
| ECS-37 | Recommendation: do not adopt EnTT, flecs or a general archetype ECS. Store each entity kind as a slot table: dense columns indexed by a 32-bit slot, hot fields split from cold, iterated in slot order, with a `PopId` or `EntityId` column and a lookup index that is never iterated. | estimate | ECS-30 to ECS-36 |
| ECS-38 | Recommendation for id sets: sorted vectors of slots for workplace and household rosters (≤10,000), galloping for small-in-large intersections, CRoaring (or our own Roaring-style container) for region-scale or block-structured sets over 32-bit slots, a plain bitset for dense hot-set masks. | estimate | ECS-20 to ECS-24, ECS-32 |
| ECS-39 | Run-to-run spread in this container was up to 2x for single operations and up to 3.5x between GCC and Clang for `std::map::find`; the ratios above are orders of magnitude, not budgets, until repeated on MSVC on a Windows machine. | sourced | benchmark, section 4 (three runs per compiler) |

## 1. How the three libraries store entities

| | EnTT (sparse sets) | flecs (archetypes, hybrid) | Bevy (tables, optional sparse sets) |
| --- | --- | --- | --- |
| Entity id | 32-bit, 20 index + 12 version, recycled (ECS-3) | 64-bit, 32 id + 32 liveness, recycled; ranges (ECS-8) | 32 index + 32 generation, can alias (ECS-13) |
| Component storage | One pool per type: paged sparse array → packed entities and packed components (ECS-1, ECS-2) | One table per component set; `Sparse` and `DontFragment` traits per type (ECS-7, ECS-9) | Tables by default, sparse set per type (ECS-12) |
| Iterating several components | View: smallest pool, lookups in the others; owning group: aligned arrays (ECS-5) | Per matching table, contiguous arrays (ECS-7) | Per table |
| Add or remove a component | O(1) in one pool | Moves the entity between tables | Moves between tables (sparse set: O(1)) |
| Order | Swap-and-pop history unless sorted (ECS-4) | Table order | Table order |
| Groups and relationships | User's own | Each `(ChildOf, parent)` splits tables (ECS-10) | Relationships as components |
| Structural change while iterating | Not on the iterated set (ECS-6) | Deferred (ECS-11) | Deferred (commands) |

**What this means for the open question.** "Stable ids favour sparse sets" is half right. A sparse set needs a dense index to be cheap (ECS-2): its sparse array is sized by the id range, so it cannot be indexed by our sparse 64-bit `PopId` (ECS-16). None of the three gives ids that are never reused (ECS-34). The useful part of the sparse-set model is the slot: a dense 32-bit index into packed columns, which is what our body pool already is (ECS-17). Archetypes pay off when many entities share few component sets and iterate together; our households, workplaces and chunks would each become a table (ECS-35).

## 2. What our code does today

Every container of entities in `engine/sim` is a `std::map` keyed by id (ECS-15): ordered, so iteration follows [spec 05](../tier0/specs/05-clock.md) rule 7, and simple. [Engine architecture](../design/engine-architecture.md) §7 already says Tier 0 keeps simple structs and moves a system to flat arrays "when profiling says a system is hot". The measurements say where that line is:

- **Hot pool (256 bodies):** a map pass is about 1.3 µs (ECS-31). No change needed for speed.
- **Records (10,000 to 1M, [spec 28](../tier0/specs/28-pop-levels.md)):** a map pass over 100,000 records is about 25 ms (ECS-30) and a random lookup at 1M is 0.7-2.4 µs (ECS-26). Event-driven updates through the next-event queue are fine; any hourly or daily pass over all records is not.
- **Rosters:** `Roster::stints_` is a map from pop to a vector of stints, roughly 100 bytes per member (estimate: a map node plus a heap vector), against 4-8 bytes per id for a sorted vector. Spec 28 already lists `members()` walking the stint map as a known cost.

## 3. Id sets: Roaring against sorted arrays, answered

At workplace and household sizes (100 to a few thousand), sorted arrays win on every count, including memory (ECS-20). From about 10,000 random ids, CRoaring halves memory and combines sets 3-8x faster, while membership and iteration stay comparable (ECS-21). At region scale (1M) or when ids come in blocks (cohort slots, region slot ranges), CRoaring wins by one to two orders of magnitude in both memory and set algebra (ECS-22, ECS-23). A plain bitset over slots is the fastest when the set is dense, and the worst when it is small (ECS-20, ECS-32). Whatever the representation, never intersect a small set with a large one by a linear merge: galloping or probing is 200x faster (ECS-24).

Two caveats for us: CRoaring's 32-bit bitmaps need 32-bit values, so a region set must be over slots (founder slots, record slots), not `PopId`s, or use `roaring64` (ECS-19); and before Roaring bytes go into a save, the format's determinism (same set, same bytes, whatever the history of `run_optimize`) needs a test, because the save hash depends on it ([spec 06](../tier0/specs/06-save-replay.md)).

## 4. Method of the measurement

- **Machine:** cloud container, Intel Xeon @ 2.80 GHz, 4 vCPUs with AVX-512, L2 4 MiB total, L3 33 MiB; Ubuntu 24.04. One thread. Not MSVC, not a desktop.
- **Compilers and flags:** GCC 13.3.0 and Clang 18.1.3 (both with libstdc++), `-std=c++20 -O3 -DNDEBUG` (CMake's Release default for GCC and Clang, which our `release` preset uses), no `-march` (CRoaring picks AVX2/AVX-512 at run time). CRoaring v5.2.3 compiled from its `src/*.c` with the same flags into a static library; its amalgamation script was not run.
- **Timing:** `std::chrono::steady_clock`; each operation repeated until 30 ms, best of 3 trials; each program run 3 times and the median taken. Numbers in the findings are GCC medians unless a row says otherwise.
- **Id sets:** universe of 10M ids (a 10M-person census); sets of 100, 10,000 and 1M ids drawn uniformly, plus sets of 10,000 and 1M in blocks of 1,000 consecutive ids. Operations: membership (65,536 queries, half hits), intersection and union of two independent sets of equal size (result materialised), iteration (sum of ids), small-in-large intersection. Sorted vector (`std::binary_search`, `std::set_intersection`, `std::set_union`, plus a branchless search and merge), CRoaring (`contains`, `and`, `or`, `and_cardinality`, bulk iterator read), bitset of 10M bits (word AND/OR over the whole universe, `ctz` iteration). Memory: vector 4 bytes per id; CRoaring `portable_size_in_bytes`; bitset 1.25 MB.
- **Records:** a 136-byte record (64-bit id, 4 floats of needs and health, position, region, flags, 96 bytes of cold data) for 300, 10,000, 100,000 and 1M sparse 64-bit ids; the step adds a rate to three floats and counts a threshold. Layouts: `std::map<uint64_t, Rec>` inserted in shuffled order (as promotions happen), a `std::vector<Rec>`, a 16-byte hot struct, four float columns (four passes and one fused pass). Lookups: `map::find`, `lower_bound` on a sorted id column, a precomputed slot. Join: B on a random half of the entities, A read and written through B's entity list in shuffled order, slot order, or as an aligned column.

| GCC -O3, median | 100 random | 10k random | 1M random | 10k blocks | 1M blocks |
| --- | --- | --- | --- | --- | --- |
| Bytes per id: vector / Roaring / bitset | 4 / 7.36 / 12,500 | 4 / 2.12 / 125 | 4 / 1.25 / 1.25 | 4 / 0.01 / 125 | 4 / 0.01 / 1.25 |
| Membership ns: vector (branchless) / Roaring / bitset | 32 (5.9) / 25 / 1.3 | 87 (14) / 38 / 1.4 | 214 (292) / 33 / 1.8 | 49 (13) / 14 / 1.4 | 155 (90) / 48 / 1.5 |
| Intersection µs: vector (branchless) / Roaring / bitset | 0.15 (0.55) / 1.65 / 166 | 93 (53) / 11.3 / 137 | 10,644 (5,653) / 1,205 / 146 | 10.3 (40) / 0.08 / 163 | 1,314 (6,069) / 15.7 / 139 |
| Union µs: vector / Roaring / bitset | 0.5 / 7.8 / 135 | 116 / 30 / 134 | 15,318 / 182 / 121 | 41 / 0.8 / 139 | 5,641 / 22 / 128 |
| Iteration ns per id: vector / Roaring / bitset | 0.4 / 6.7 / 1,531 | 0.22 / 0.49 / 23 | 0.19 / 3.5 / 2.4 | 0.21 / 0.34 / 17.6 | 0.21 / 0.33 / 1.0 |

| GCC -O3, median, ns per record | map | array of structs | hot struct (16 B) | columns, 4 passes | columns, fused | lookup: map / sorted ids / slot |
| --- | --- | --- | --- | --- | --- | --- |
| 300 | 5.1 | 1.0 | 0.9 | 0.7 | 0.5 | 40 / 45 / 1.2 |
| 10,000 | 31 | 1.3 | 0.9 | 0.6 | 0.4 | 122 / 79 / 1.3 |
| 100,000 | 247 | 5.4 | 0.9 | 0.6 | 0.4 | 930 / 120 / 1.3 |
| 1,000,000 | 296 | 14.8 | 2.6 | 1.3 | 0.7 | 2,421 / 252 / 1.7 |

The branchless variants swing with the compiler (Clang's branchy search was faster than its branchless one), and a branchless merge loses to the branchy one on block-structured sets, where branches predict well; neither changes the conclusions.

## 5. Profiling with Tracy

Tracy is already the planned profiler ([engine architecture](../design/engine-architecture.md) §1 and §9). What the manual gives us (ECS-28, ECS-29): a zone per phase and per chunk job is affordable (2.25 ns), but not per entity; the sim tick should be its own named frame set (`FrameMarkNamed("tick")`), separate from render frames, so tick time and frame time are read apart; counts that explain cost (records updated, embodied bodies, roster events, set sizes) go to `TracyPlot`; `TracyAlloc`/`TracyFree` in the per-worker allocators show any allocation inside the tick, which §7 forbids; on-demand mode keeps it compiled in without a connected viewer. Long fast-forward sessions must stay under 1.6 days of wall time per capture.

## What this means for us (Proposal)

Storage per detail level ([spec 27](../tier0/specs/27-detail-levels.md), [spec 28](../tier0/specs/28-pop-levels.md)):

| Level | Size | Store | Sets over it | Why |
| --- | --- | --- | --- | --- |
| Hot (embodied bodies, walkers near the player) | 50-300 | The existing fixed slot pool ([spec 29](../tier0/specs/29-embodied-pops.md)); keep `std::map` until Tracy shows it; when the job system lands, columns per slot, hot and cold split, iterated in slot order | Bitset or `uint64_t` masks over slots (alive, in fight, in chunk) | ECS-31; slots are dense and fixed, so a mask is a few words |
| Warm and record pops | 10,000-1M | One slot table per kind: columns indexed by a 32-bit record slot, a sorted `PopId` column, hot fields (needs, next-event time, chunk) apart from cold ones (ties, beliefs, exceptions); free slots reused but ids never; a lookup index (`PopId` → slot) never iterated | Rosters as sorted vectors of slots; galloping for small-in-large | ECS-25, ECS-26, ECS-30; a full pass over 1M is about 1 ms in columns |
| Region-scale sets (who is in a region, infected, employed) | 100,000-10M | Not stored per pop: counts are sums ([spec 28](../tier0/specs/28-pop-levels.md) rule 4) | CRoaring or a Roaring-style container over founder or record slots when a list is truly needed; blocks of cohort slots compress to almost nothing | ECS-22, ECS-23 |
| Lazy pops | unbounded | Nothing: `derive(pop_id, t)` and cohort ranges, as now | Cohort ranges (already intervals) | Spec 28 technique |

Answers to the two open questions: **neither archetypes nor a general sparse-set ECS**; per-kind slot tables, which are the dense half of a sparse set without id recycling and with slot-order iteration (ECS-37). **Roaring does not beat sorted arrays at roster sizes** (up to a few thousand); it does from about 10,000 random ids and decisively at region scale or with block-structured ids (ECS-38). Do not add CRoaring to [0002](../decisions/0002-engine-stack.md) until a region-scale set exists in a spec; until then, sorted vectors.

Code changes to make when the system that needs them is built, not before (none is a bug):

1. `engine/sim/include/zs/pop_registry.h:183` `records_` (`std::map<PopId, Record>`): move to a record slot table when records gain periodic updates or exceed about 100,000 (ECS-30).
2. `engine/sim/include/zs/roster.h:64` `stints_` (`std::map<PopId, std::vector<Stint>>`): a sorted vector of `(PopId, joined, left)` rows, which also removes the stint-map walk spec 28 lists as a known cost.
3. `engine/sim/include/zs/ledger.h:94` `balances_` (`std::map<std::pair<OwnerId, ResourceId>, int64_t>`): every body flow looks it up; put a Tracy zone around transfers before changing it, then consider a dense owner × resource table once owners are slots.

## Open questions

- [Engine architecture](../design/engine-architecture.md): do these ratios hold on MSVC and a desktop CPU? Rerun the benchmark there before writing the layout into a spec (ECS-39).
- [Spec 28](../tier0/specs/28-pop-levels.md): which record fields are hot (touched by every periodic pass) and which cold? The split needs the realistic record that benchmark B1 is waiting for.
- [Focus and compression](../design/focus-and-compression.md): if a region-scale id set is ever stored, is it over founder slots, record slots or 64-bit `PopId` (`roaring64`)? And is CRoaring's portable serialisation byte-identical for the same set whatever its history, as a save hash needs?

## Sources

- EnTT, `github.com/skypjack/entt` @ e0f4c37 (2026-09-17, version 4.1.0 in `src/entt/config/version.h`): `docs/md/entity.md`, `docs/md/config.md`, `src/entt/entity/sparse_set.hpp`, `src/entt/entity/entity.hpp`, `src/entt/config/config.h`, `LICENSE` (MIT).
- flecs, `github.com/SanderMertens/flecs` @ 9e874bc (2026-09-12, v4.1.6): `docs/FAQ.md`, `docs/EntitiesComponents.md`, `docs/ComponentTraits.md`, `docs/HierarchiesManual.md`, `docs/Relationships.md`, `LICENSE` (MIT).
- Bevy, `github.com/bevyengine/bevy` @ 59fed6a (2026-10-08, `bevy_ecs` 0.20.0-dev): `crates/bevy_ecs/README.md`, `crates/bevy_ecs/src/storage/mod.rs`, `crates/bevy_ecs/src/entity/mod.rs`.
- CRoaring, `github.com/RoaringBitmap/CRoaring` @ f2b1a55 (v5.2.3): `README.md`, `include/roaring/roaring.h`, `include/roaring/containers/array.h`, `include/roaring/containers/bitset.h`, `src/containers/bitset.c`, `LICENSE`. Format specification: https://github.com/RoaringBitmap/RoaringFormatSpec (not read).
- Tracy, `github.com/wolfpld/tracy` @ afb16f6 (version 0.14.2 in `public/common/TracyVersion.hpp`): `manual/tracy.tex`, `LICENSE` (BSD-3).
- Unity Entities manual, archetypes: https://docs.unity3d.com/Packages/com.unity.entities@1.4/manual/concepts-archetypes.html (search summary only).
- Our code at `origin/main` 2646536: `engine/sim/include/zs/body.h`, `dead.h`, `ledger.h`, `beliefs.h`, `pop.h`, `pop_registry.h`, `roster.h`, `draw.h`.
- Earlier notes: [pop storage and lenses](pop-storage-and-lenses.md) (PS-8, PS-10, the questions this note answers), [threading and partition](threading-and-partition.md) (TP-27, ids never recycled).
