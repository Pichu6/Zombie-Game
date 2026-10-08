# 0055 · Defaults for the fixes found by the engineering research

- **Date:** 2026-10-08
- **Status:** Defaults taken by Claude ([0052](0052-one-executable-one-simulation.md) item 6), so that the [research fixes](../tier0/research-fixes.md) can be built without waiting. Each item can be overturned by the user.
- **Builds on:** the research notes of 2026-10-08: [engine stack](../research/engine-stack.md) (ES), [renderer](../research/renderer.md) (RN), [determinism](../research/determinism.md) (DET), [data compression](../research/data-compression.md) (DC), [entity storage](../research/ecs-memory-layout.md) (ECS), [view readability](../research/view-readability.md) (VR).
- **Changes:** confirms [0002](0002-engine-stack.md) (a Proposal until now) with the edits in item 1; adds to [spec 01](../tier0/specs/01-quantity.md), [spec 06](../tier0/specs/06-save-replay.md), [spec 16](../tier0/specs/16-view-2d.md) and [spec 40](../tier0/specs/40-time.md) through the fix plan, which updates those specs before their code.

## Defaults

### 1. Engine stack (0002 confirmed, with edits)

1. **Raw Vulkan 1.3** with vk-bootstrap and VMA for the 3D renderer, not SDL3's GPU API, which has no bindless, timestamps, queue choice or native handles ([ES-47](../research/engine-stack.md), [RN](../research/renderer.md)). SDL3 stays the window and input layer. Minimum GPU: Vulkan 1.3 with buffer device address, descriptor indexing, dynamic rendering and synchronization2; mesh shaders optional. Two frames in flight.
2. **The Tier 0 2D window stays on SDL's renderer with ImGui draw lists** until a scenario needs 3D (RN).
3. **cgltf, not fastgltf** (fastgltf downloads simdjson at configure time, [ES-35](../research/engine-stack.md)).
4. **enkiTS and Tracy** join the table (from [0023](0023-engine-architecture-threads.md)); Tracy is off in release builds.
5. **Recast/Detour pinned to a commit** (no release since v1.6.0, [ES-3](../research/engine-stack.md)).
6. **Steam Audio later**; miniaudio first ([ES-46](../research/engine-stack.md)).
7. **Physics is view-side** unless a game rule needs it ([ES-45](../research/engine-stack.md)); when Jolt enters the simulation it is built with `CROSS_PLATFORM_DETERMINISTIC`, its thread-ordered outputs are sorted, `USE_STATIC_MSVC_RUNTIME_LIBRARY` is OFF and its GPU backends are off.
8. **A notices file** ships with the game, generated from `third_party/README.md`, which lists every vendored and fetched library with its licence.
9. **zstd** is vendored for saves, autosaves and traces (DC).

### 2. Direct control and the window

1. **Time while the player controls a pop:** the window drives time with the spec 40 `Pacer`. While a WASD key is held or the controlled pop is walking to a target, a named cap `walking` holds speed at **3x**; during a fight (a walker within reach of the controlled pop) the cap is **Real (1:1)**. The time bar shows which cap is limiting. "1x" means real time.
2. **WASD refuses walls:** a press moves only if the edge to the neighbour cell is walkable; a diagonal only if both orthogonal edges are; otherwise the one passable axis, or nothing (the blocking edge flashes). It stays a `MoveTo` to the neighbour for now; a sim-side "step" intent waits for a spec 14 change.
3. **A WASD press cancels the controlled pop's current action** (as in direct-control games); while paused it shows "Paused: Space to run".
4. **Follow camera:** follows the controlled pop with frame-rate-independent smoothing (tau 0.1 s) and a dead zone of about 20 % of the view; a middle-drag pan or PgUp/PgDn breaks the follow; **F or Home** recentres and resumes it; any WASD press resumes it. The drawn floor follows the pop.
5. **Palette and shapes:** group colours from Paul Tol's "bright" palette, with initials kept as a second cue; pops are discs, walkers diamonds, objects squares with two- or three-letter labels and a map key. Colour is never the only cue on the map ([art direction](../design/art-direction.md)).
6. **A state column** ("moodles") is part of the Tier 0 window: the three to five worst states of the controlled pop, by the pop on the map and in the Group panel. States: hunger, thirst, fatigue, cold or heat, pain, bleeding, fear, and sickness from visible symptoms only (infection itself stays hidden, [spec 21](../tier0/specs/21-infection.md)). Thresholds come from the body's existing bands.
7. **Pacer alerts that pause at normal speed:** a walker comes into sight of a group member, a bite, a group member down.

### 3. Determinism

1. **The float path is required across compilers**, measured: GCC 13.3 and Clang 18.1 reproduce every MSVC golden ([DET](../research/determinism.md)). CI runs the goldens on Linux with GCC and Clang as well as on Windows.
2. **x64 only for now.** ARM64 (FMA in the base instruction set) is out of scope until a target needs it.
3. **clang-cl gets `/clang:-ffp-contract=off`**; MSVC 2022 or newer is required.
4. **A NaN in saved state is a bug:** the archive asserts on it rather than canonicalising it.
5. **The floating-point environment is checked and restored per step** on the main thread now; per worker at start once workers exist.
6. **One number parser:** `TOML_FLOAT_CHARCONV=1` on GCC 12+ so all compilers parse data floats with `from_chars`.

### 4. Saves and traces

1. **Save format version 4** adds the codec, the uncompressed size, and spec 06 rule 4's binary hash, data hash, seed and settings in one step; the version 3 reader stays.
2. **zstd level 3** for saves, **level 1** for autosaves and traces, level 19 only for archives; one frame per blob, single-threaded, checksum on. Golden tests hash uncompressed bytes only.
3. **One full save per game day** in the autosave and rewind history, with deltas against the previous save in between.
4. **No trained dictionaries** for now (a 16 % gain, [DC](../research/data-compression.md)).
5. **The replay viewer embeds the whole trace compressed** instead of thinning frames.
6. **Founder specs are regenerated** from the generator, not stored; pop record death and effect years move to fixed point when the record format next changes.

### 5. Storage

1. **No ECS library.** One slot table per entity kind with stable 64-bit ids, columns indexed by a dense slot, hot fields apart from cold ones ([ECS](../research/ecs-memory-layout.md)). Rosters are sorted vectors; Roaring only if a spec ever stores region-scale sets.
2. The `std::map` stores named in the fix plan move to slot tables or sorted vectors **when their system gets real load**, not before.

## Consequences

- The fix plan in [research-fixes](../tier0/research-fixes.md) can be built in order by a local session without asking.
- Specs 01, 02, 05, 06, 16 and 40 get the wording changes listed in the plan, before the code.
