# The engine stack behind decision 0002: licences, versions, determinism, alternatives

**Date:** 2026-10-08. **Status:** research note, not a decision; it reviews [0002](../decisions/0002-engine-stack.md), which is still a Proposal. **Feeds:** [0002](../decisions/0002-engine-stack.md), [engine architecture](../design/engine-architecture.md), [0023](../decisions/0023-engine-architecture-threads.md), [0001](../decisions/0001-own-engine.md), [spec 01](../tier0/specs/01-quantity.md), [spec 06](../tier0/specs/06-save-replay.md), [spec 16](../tier0/specs/16-view-2d.md), [threats](../design/threats.md). **Evidence:** every library in 0002, plus enkiTS and Tracy (added by 0023) and the three vendored test and data libraries, was read at its latest tag from a shallow git clone on 2026-10-08 (LICENSE, README, docs, headers, CMake files); tag dates come from `git for-each-ref` on blobless clones. Our own code was read and grepped at commit 8295162. One measurement was run on this cloud container (Ubuntu 24.04, g++ 13.3, clang 18.1, libstdc++); MSVC was not available here. Library websites (wiki.libsdl.org, jrouwe.github.io, miniaud.io, valvesoftware.github.io) were blocked; their content was read from the repos instead. Users of Jolt and the Steam Audio licence change come from web-search summaries. **IDs:** ES.

0002 names eleven libraries and nothing in the repo backs the choice. This note checks each one at the primary source: what its licence asks of a commercial game, how alive it is, what it needs from our build, and whether it can sit inside a deterministic simulation with golden hashes. It also looks at SDL3's own GPU API as an alternative to raw Vulkan.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| ES-1 | Of the 0002 stack, the repo uses only SDL3 and Dear ImGui, fetched at configure time and pinned to `release-3.4.8` (commit d9d55367, 2026-05-01) and `v1.92.9` (01380c57, 2026-07-25); the window uses the SDL 2D renderer, no Vulkan. Vendored in `third_party/`: doctest 2.4.11, toml++ 3.4.0, RapidCheck master 2c3c4365. | sourced | `apps/zs/CMakeLists.txt`, `third_party/README.md` @ 8295162; tags checked with `git ls-remote` |
| ES-2 | Latest tags on 2026-10-08: vk-bootstrap v1.4.365 (2026-10-06), VMA v3.4.0 (2026-06-05), meshoptimizer v1.3 (2026-09-25), SDL release-3.4.18 (2026-10-02), Jolt v5.6.0 (2026-07-11), ozz-animation 0.17.0 (2026-08-01), Recast v1.6.0 (2023-05-21), cgltf v1.15 (2025-02-09), fastgltf v0.9.1 (2026-09-27), Dear ImGui v1.92.9b (2026-07-31), miniaudio 0.11.25 (2026-03-04), Steam Audio v4.8.1 (2026-02-11), enkiTS v1.12 (2026-07-04), Tracy v0.14.1 (2026-08-22). | sourced | `git for-each-ref --sort=-creatordate refs/tags` on each repo |
| ES-3 | Recast/Detour has made no release since v1.6.0 (May 2023), but main has 367 commits since, the last on 2026-02-27 (a crash fix for large navmesh builds), with CMake modernised and unit tests added in Feb 2026. We would pin a commit, not a tag. | sourced | github.com/recastnavigation/recastnavigation, `git log v1.6.0..HEAD` |
| ES-4 | MIT licence (keep the copyright and permission notice "in all copies or substantial portions"): vk-bootstrap, VMA, meshoptimizer, Jolt, ozz-animation, cgltf, fastgltf, Dear ImGui, doctest, toml++. A shipped game needs a third-party notices file with each text. | sourced | each repo's `LICENSE*` at the tags in ES-2 |
| ES-5 | zlib licence: SDL3, Recast/Detour, enkiTS, SDL_shadercross. Binaries need no notice (acknowledgement "would be appreciated but is not required"); altered source must be marked as altered. | sourced | `SDL/LICENSE.txt` @ release-3.4.18, `recastnavigation/License.txt` @ v1.6.0, `enkiTS/License.txt` @ v1.12 |
| ES-6 | miniaudio is dual public domain (Unlicense) or MIT No Attribution: nothing is required of us. | sourced | `miniaudio/LICENSE` @ 0.11.25 |
| ES-7 | Steam Audio is Apache 2.0 (core and plug-ins; the core was opened in February 2024 with v4.5.2). Apache 2.0 needs the licence text shipped and modified files marked; there is no NOTICE file in the repo. `TRADEMARK_RIGHTS.md` grants no licence to "Steam Audio" or other Valve marks beyond describing the code's origin, and forbids implying endorsement. | sourced | `steam-audio/LICENSE.md`, `TRADEMARK_RIGHTS.md` @ v4.8.1; open-sourcing date: phoronix.com/news/Steam-Audio-SDK-Fully-Open (reported) |
| ES-8 | Steam Audio's third-party list: Intel IPP (binary only, Intel Simplified Software License: reproduce the notice in documentation, no reverse engineering), Embree and Google SH (Apache 2.0), FFTS, PFFFT and MySOFA (BSD), CIPIC HRTF database (keep the copyright notice), Radeon Rays and TrueAudio Next (MIT). IPP is optional: without it the build falls back to built-in vector math and PFFFT. | sourced | `steam-audio/core/THIRDPARTY.md`, `core/doc/build-instructions.rst` @ v4.8.1 |
| ES-9 | Tracy is BSD 3-clause (binary redistribution reproduces the notice), but its CMake option `TRACY_ENABLE` defaults to OFF in v0.14.1, so a release build without it ships nothing of Tracy. RapidCheck (BSD 2-clause) is test-only and never shipped. | sourced | `tracy/LICENSE`, `tracy/CMakeLists.txt:111` @ v0.14.1; `third_party/README.md` |
| ES-10 | Jolt is deterministic when the API calls that change the world come in the same order and the same binary runs; CPU vendor does not matter. | sourced | `JoltPhysics/Docs/Architecture.md` "Deterministic Simulation" @ v5.6.0 |
| ES-11 | With the CMake option `CROSS_PLATFORM_DETERMINISTIC` Jolt is about 8 % slower but gives the same result across compilers (MSVC 2022, clang, gcc, emscripten), configurations, OSes, architectures and word size; CI checks 14 targets including Windows MSVC x64 AVX2 and Linux gcc/clang x64. The same source and the same defines are required on every platform. | sourced | `Docs/Architecture.md` @ v5.6.0 |
| ES-12 | `CROSS_PLATFORM_DETERMINISTIC` also drops FMA: `JPH_USE_FMADD` and `-mfma` are only set when it is off. | sourced | `Jolt/Jolt.cmake:963-1025` @ v5.6.0 |
| ES-13 | Even in deterministic mode: broad-phase queries are not deterministic (fattened bounds updated from several threads), narrow-phase queries return the same hits in varying order, contact, activation and step listeners are called from several threads in no fixed order, and `GetActiveBodies` order varies. Anything the sim reads from Jolt must be re-filtered and sorted. | sourced | `Docs/Architecture.md` @ v5.6.0 |
| ES-14 | Jolt's rules for the host application: precise FP mode (`/fp:precise`, `-ffp-model=precise`), `-ffp-contract=off`, consistent rounding and DAZ/FTZ across threads, no standard `sin`/`cos`, no `std::sort`, no `std::push_heap`/`pop_heap` with equal elements, no `std::hash`. | sourced | `Docs/Architecture.md` @ v5.6.0 |
| ES-15 | Our build already follows ES-14: `zs_strict_fp` sets `/fp:precise` on MSVC and `-ffp-contract=off -fno-fast-math -msse2 -mfpmath=sse` elsewhere; transcendental functions go through `zs::detmath`; `engine/sim` and `engine/gen` contain no `unordered_*` or `std::hash`; every `priority_queue` key is a tuple ending in a cell or entity index, and every `std::sort` comparator breaks ties by id, so unstable library sorts and heaps cannot change results. | sourced | `CMakeLists.txt:13-22`, `engine/sim/src/{noise,walkers,site_map}.cpp`, `engine/gen/src/{world_fields,world_people,world_grid,catalog}.cpp` @ 8295162 (grep) |
| ES-16 | Jolt saves and restores physics state per step (`SaveState`/`RestoreState` with a filter); it records only what the step changes, body ids must match, and `CreateBodyWithID` fixes ids. Constraint order matters unless `mConstraintPriority` is unique. | sourced | `Docs/Architecture.md` "Rolling Back a Simulation", "Being Sloppy" @ v5.6.0 |
| ES-17 | Jolt in float mode is accurate within about 5 km of the origin; `JPH_DOUBLE_PRECISION` costs 5-10 %; the broad phase stays float, so at 10,000 km its resolution is 1 m. Several `PhysicsSystem`s can run side by side sharing only shapes (one global factory, allocator and default material). | sourced | `Docs/Architecture.md` "Big Worlds", "Multiple Physics Systems" @ v5.6.0 |
| ES-18 | Jolt's `JobSystem` is an abstract interface (CreateJob, QueueJob(s), barriers); `JobSystemThreadPool`, `JobSystemWithBarrier` and `JobSystemSingleThreaded` ship with it, so it can run on enkiTS or on one thread in headless tests. All allocations can be routed through `Allocate`/`Free`/`AlignedAllocate`. | sourced | `Jolt/Core/JobSystem.h:20-68`, `Jolt/Core/JobSystemSingleThreaded.h` @ v5.6.0 |
| ES-19 | Jolt needs C++17 and CMake 3.20; exceptions and RTTI are off by default; `USE_STATIC_MSVC_RUNTIME_LIBRARY` defaults ON and sets `CMAKE_MSVC_RUNTIME_LIBRARY` to the static runtime, which clashes with our default `/MD` (MSVC LNK2038) unless turned off. v5.6.0 added a GPU compute interface (DX12, Vulkan, Metal, CPU) and a work-in-progress GPU hair system, both on by default (`JPH_USE_VK` etc.); a sim-side build turns them off. | sourced | `Build/CMakeLists.txt:1,119,153`, `Jolt/Jolt.cmake:2`, `Docs/ReleaseNotes.md` v5.6.0 |
| ES-20 | Jolt replaced a commercial engine in Guerrilla's Decima for Horizon Forbidden West (less memory, double the simulation rate with less CPU); its README lists Death Stranding 2. | reported | guerrilla-games.com/read/architecting-jolt-physics-for-horizon-forbidden-west; Jolt `README.md` |
| ES-21 | Physics alternatives: PhysX 5 is BSD 3-clause (2008-2025 NVIDIA); Box2D v3.1.1 is MIT and cross-platform deterministic but 2D only ([TP-14](threading-and-partition.md)). Neither documents a cross-compiler determinism mode as complete as Jolt's (ES-11). | sourced | `NVIDIA-Omniverse/PhysX LICENSE.md` @ main, `erincatto/box2d LICENSE` @ main |
| ES-22 | SDL3 ships a patch release roughly monthly (3.4.10 to 3.4.18 between 2026-05-31 and 2026-10-02). Our pin 3.4.8 is 390 commits behind 3.4.18, which include GPU, D3D12 and Vulkan renderer fixes (`SDL_RenderReadPixels` validation error, render-target pass reuse, D3D12 fence read under lock). | sourced | `git log release-3.4.8..release-3.4.18` on github.com/libsdl-org/SDL |
| ES-23 | `SDL_PollEvent` and `SDL_PumpEvents` may only be called on the main thread (the thread that initialised video), as engine-architecture §2 assumes. `SDL_Vulkan_GetInstanceExtensions` and `SDL_Vulkan_CreateSurface` give the surface vk-bootstrap needs. | sourced | `SDL/include/SDL3/SDL_events.h:1099,1271,1302`, `SDL_vulkan.h:200,226` @ release-3.4.18 |
| ES-24 | SDL_gpu runs on Vulkan, D3D12 and Metal. On Vulkan it needs 1.0 plus `VK_KHR_swapchain`, `VK_KHR_maintenance1` and six core features; since 3.4.0 `SDL_GPUVulkanOptions` can request a higher API version, a feature chain and extra extensions. | sourced | `SDL/include/SDL3/SDL_gpu.h` header comment and lines 2400-2424 @ release-3.4.18 |
| ES-25 | SDL_gpu's binding model is fixed: SPIR-V descriptor sets 0-3 are assigned by resource kind and stage; per stage at most 16 samplers, 8 storage textures, 8 storage buffers, 4 uniform buffers; at most 3 frames in flight. There are no unbounded descriptor arrays (bindless). | sourced | `SDL_gpu.h:2606-2741`, `src/gpu/SDL_sysgpu.h:29-40` @ release-3.4.18 |
| ES-26 | SDL_gpu has compute passes, storage buffers and textures, indirect draws with a draw count, indirect dispatch, buffer and texture downloads with fences, and command buffers that may be recorded on several threads (each used on the thread that acquired it). | sourced | `SDL_gpu.h` header comment, lines 538, 3623-3650, 3845 @ release-3.4.18 |
| ES-27 | SDL_gpu has no timestamp or occlusion queries, no queue choice (no dedicated transfer or async-compute queue), no draw-indirect-count, and no access to the native `VkDevice`/`VkCommandBuffer` (device properties are name and driver strings only). Its FAQ says mesh shaders and ray tracing are not "near future" items. | sourced | `SDL_gpu.h` FAQ and lines 2590-2595; absence checked by grep for query, timestamp, queue, count buffer @ release-3.4.18 |
| ES-28 | SDL_gpu takes a different shader format per backend (SPIR-V, DXBC, DXIL, MSL, metallib). SDL_shadercross (zlib, no release tags, last commit 2026-09-03) translates SPIR-V or HLSL, depends on SPIRV-Cross, SPIRV-Tools and a DXC fork, and its README advises shipping SPIRV-Cross and DXIL DLLs for runtime use. | sourced | `SDL_gpu.h` header comment; github.com/libsdl-org/SDL_shadercross `README.txt`, `.gitmodules` @ 1ff05be |
| ES-29 | Dear ImGui has backends for both paths: `imgui_impl_sdlgpu3` and `imgui_impl_vulkan` next to the `imgui_impl_sdlrenderer3` we use. | sourced | `imgui/backends/` @ v1.92.9b |
| ES-30 | vk-bootstrap (MIT, C++17, CMake 3.22) builds the instance, picks the physical device by criteria, creates the device, queues (including a dedicated transfer queue) and swapchain, and can take our own `vkGetInstanceProcAddr` (for volk). Its tags follow the Vulkan header version and come out about weekly. | sourced | `vk-bootstrap/README.md`, `src/VkBootstrap.h:253,352`, `CMakeLists.txt:102` @ v1.4.365 |
| ES-31 | VMA is one C++14 header with no STL, RTTI or exceptions, thread-safe for allocations that share a block, uses `VK_EXT_memory_budget` when present, and can defragment. | sourced | `VulkanMemoryAllocator/README.md` @ v3.4.0 |
| ES-32 | meshoptimizer has a C-compatible header and C++ sources needing no special flags; it covers vertex-cache, overdraw and fetch optimisation, simplification, meshlets with cone bounds, and the gltfpack tool; it reached v1.0 in Dec 2025. | sourced | `meshoptimizer/README.md` @ v1.3 |
| ES-33 | ozz-animation's runtime needs only C++17 and the standard library (no OS code); its converters read glTF and, through the FBX SDK, FBX; allocations go through `SetDefaulAllocator`. Its top `CMakeLists.txt` requires CMake 3.30, above our 3.25. Alternatives: ACL (MIT, v2.1.0) compresses clips but does not blend or do IK; Jolt itself has `Skeleton`, `SkeletalAnimation` and `Ragdoll`. | sourced | `ozz-animation/README.md`, `CMakeLists.txt:1`, `include/ozz/base/memory/allocator.h:48`, `CHANGES.md` @ 0.17.0; `nfrechette/acl LICENSE` @ develop |
| ES-34 | Recast and Detour take custom allocators (`rcAllocSetCustom`, `dtAllocSetCustom`); the README recommends adding the source folders (Recast, Detour, DetourCrowd, DetourTileCache) to our own build; Detour offers tiled navmeshes for streaming. | sourced | `recastnavigation/README.md`, `Recast/Include/RecastAlloc.h:52`, `Detour/Include/DetourAlloc.h:47` @ v1.6.0 |
| ES-35 | cgltf is one C99 header with memory callbacks (`cgltf_memory_options`) and is used by gltfpack, Filament, bgfx and raylib. fastgltf needs C++17 (optional C++20 module) and simdjson, which its CMake downloads at configure time (simdjson 4.6.11) unless one is already found. | sourced | `cgltf/README.md`, `cgltf.h:134` @ v1.15; `fastgltf/README.md`, `cmake/dependencies.cmake` @ v0.9.1 |
| ES-36 | A Dear ImGui context may not be used from several threads in parallel; a separate render thread must stage `ImDrawData` (the FAQ points to `ImDrawDataSnapshot` in imgui_club). Allocations go through `SetAllocatorFunctions`. | sourced | `imgui/docs/FAQ.md:750-758`, `imgui.h:288` @ v1.92.9b |
| ES-37 | ImGui v1.92.9b fixes two regressions of our pinned v1.92.9 (Ctrl+click on a Drag/Slider flicks to text edit; `ImDrawData::CmdListsCount` always 0). Our window uses neither: no Drag/Slider widgets, and the SDL renderer backend iterates `CmdLists`. | sourced | `imgui/docs/CHANGELOG.txt` @ v1.92.9b; `apps/zs/window.cpp`, `backends/imgui_impl_sdlrenderer3.cpp:177` |
| ES-38 | miniaudio is C in one file, with WASAPI and DirectSound on Windows, a high-level engine with mixing, a node graph and optional 3D spatialisation, WAV/FLAC/MP3 decoding, and `ma_allocation_callbacks`. | sourced | `miniaudio/README.md`, `miniaudio.h:4382` @ 0.11.25 |
| ES-39 | Steam Audio is a C API (`phonon`) built with CMake 3.17 at C++14, shared by default, with its own static C runtime; its scene can use the default ray tracer, Embree, Radeon Rays or `IPL_SCENETYPE_CUSTOM` (our own closest-hit and any-hit callbacks); direct occlusion is raycast or volumetric. | sourced | `steam-audio/core/CMakeLists.txt:15-117`, `core/src/core/phonon.h:720-737,3683-3738` @ v4.8.1 |
| ES-40 | enkiTS (zlib, C++11) does not allocate while scheduling, has up to 5 priorities, pinned tasks that can wait for new work (for an I/O thread), external threads and a custom allocator. v1.12 (2026-07) came after a four-year gap since v1.11 (2022-03); main had commits up to 2026-09-29. | sourced | `enkiTS/README.md:13-75` @ v1.12; tag dates |
| ES-41 | Vendored test and data libraries: doctest is at v2.5.3 (2026-07-07) against our 2.4.11 (2023-03); toml++ has no release since v3.4.0 (2023-10-13) but 50 commits on master since, including parser fixes (a precondition violation on `[[[` headers, quoted-key error text); our RapidCheck commit is still its HEAD (it has no tags). | sourced | `doctest/CHANGELOG.md` @ v2.5.3; `git log v3.4.0..HEAD` on marzer/tomlplusplus; `git ls-remote` emil-e/rapidcheck |
| ES-42 | toml++ 3.4.0 parses doubles with `std::from_chars` on MSVC but forces a `std::stringstream` fallback on GCC and Clang (`TOML_FLOAT_CHARCONV 0`). On libstdc++ both paths gave bit-identical doubles for all 22,603 decimal literals in `data/**/*.toml` and for 2,000,000 random decimals of 1-17 digits, with g++ 13.3 and clang 18.1. libc++ was not tested (no `<charconv>` on this machine). | sourced | `third_party/tomlplusplus/toml.hpp:1106-1127,14358-14398`; measured: `parse.cpp` comparing both paths, `g++ -std=c++20 -O2` and `clang++ -std=c++20 -O2`, Ubuntu 24.04 cloud container, 2026-10-08 |
| ES-43 | CMake minimums: ozz 3.30, vk-bootstrap 3.22, Jolt 3.20, SDL 3.16, Steam Audio 3.17, miniaudio 3.10. Only ozz exceeds our 3.25; we either raise ours or build the ozz runtime sources in our own target. | derived | ES-19, ES-30, ES-33, ES-39; each `CMakeLists.txt` |
| ES-44 | Verdict on 0002: every choice holds on licence, maintenance and build. Changes: fold enkiTS and Tracy (0023) into the table; pick cgltf for the offline converter (no download at configure, one header); pin Recast to a commit; make Steam Audio "later" (ES-46); state where physics lives (ES-45). | estimate | ES-2 to ES-43 |
| ES-45 | Physics in the simulation needs Jolt built with `CROSS_PLATFORM_DETERMINISTIC`, `JobSystemSingleThreaded` (or ordered jobs), sorted results from every query and listener (ES-13), and body ids from our entity ids (ES-16). Cheaper default: the sim keeps its own room and grid geometry for movement, sight and noise, and Jolt runs view-side only (ragdolls, debris, props) where its results never feed back. A rule that needs physics (a thrown object hitting someone) is then a sim request with a sim-side answer. | estimate | ES-10 to ES-18, [spec 01](../tier0/specs/01-quantity.md), [contract rule 8](../design/world-simulation.md#2-the-simulation-contract-proposal) |
| ES-46 | Steam Audio is not needed for the first 3D slice: the sim already computes sound path loss through the map (`noise.cpp` `path_loss`; [threats](../design/threats.md): sound travels through the room graph), and miniaudio's spatialiser plus a low-pass driven by that loss gives occlusion. Steam Audio adds HRTF and reflections later, through `IPL_SCENETYPE_CUSTOM` on our own geometry; its licence then needs ES-7 and ES-8 notices. | estimate | ES-38, ES-39, `engine/sim/src/noise.cpp` |
| ES-47 | SDL_gpu would cover the first 3D renderer (compute skinning and clothing wrap, clustered forward lights in storage buffers, indirect draws) but blocks things engine-architecture §8 and 0009's visual realism will want: GPU timestamps for Tracy, bindless materials, a transfer queue fed by the I/O thread, draw-indirect-count for GPU culling, and later mesh shaders; with no native handles there is no escape hatch. Keep raw Vulkan with vk-bootstrap and VMA ([0001](../decisions/0001-own-engine.md)); SDL stays the window and input layer. | estimate | ES-24 to ES-31, [0009](../decisions/0009-visual-direction.md) |

## The stack, one row per library

| Library | Licence | Latest tag (date) | Language, build | Allocator hook | Holds? |
| --- | --- | --- | --- | --- | --- |
| vk-bootstrap | MIT | v1.4.365 (2026-10-06) | C++17, CMake 3.22 | uses Vulkan callbacks | Yes. Alternative: hand-written init (about 500 lines, per its README); no gain. |
| Vulkan Memory Allocator | MIT | v3.4.0 (2026-06-05) | C++14 header | Vulkan callbacks | Yes. No serious alternative. |
| meshoptimizer | MIT | v1.3 (2026-09-25) | C header, C++ sources | `meshopt_setAllocator` | Yes. Offline converter only. |
| SDL3 | zlib | release-3.4.18 (2026-10-02) | C, CMake 3.16 | `SDL_SetMemoryFunctions` | Yes, already in use; bump the pin. GLFW would cover only the window and input, and SDL is already in. |
| Jolt Physics | MIT | v5.6.0 (2026-07-11) | C++17, CMake 3.20 | `Allocate`/`Free` | Yes; the only one with a documented cross-compiler determinism mode. PhysX 5 (BSD-3) is the alternative. |
| ozz-animation | MIT | 0.17.0 (2026-08-01) | C++17, CMake 3.30 | `SetDefaulAllocator` | Yes, view-side. ACL compresses only. |
| Recast and Detour | zlib | v1.6.0 (2023-05-21), main 2026-02-27 | C++, sources added to our build | `rcAllocSetCustom`, `dtAllocSetCustom` | Probably; pin a commit. The sim's own grid paths may be enough until open 3D ground. |
| cgltf / fastgltf | MIT / MIT | v1.15 (2025-02-09) / v0.9.1 (2026-09-27) | C99 header / C++17 + simdjson | memory options / custom | Pick cgltf. |
| Dear ImGui | MIT | v1.92.9b (2026-07-31) | C++, no build system | `SetAllocatorFunctions` | Yes, already in use. |
| miniaudio | Unlicense or MIT-0 | 0.11.25 (2026-03-04) | C, one file | `ma_allocation_callbacks` | Yes. Alternatives (SoLoud, FMOD, Wwise) add licences or fees. |
| Steam Audio | Apache 2.0 + third party | v4.8.1 (2026-02-11) | C API, C++14, shared lib | — | Later, not first (ES-46). |
| enkiTS (0023) | zlib | v1.12 (2026-07-04) | C/C++11 | custom allocator | Yes. |
| Tracy (0023) | BSD-3 | v0.14.1 (2026-08-22) | C++ | — | Yes; compiled out of release builds. |

## Determinism: what Jolt asks, and where we stand

Jolt's documentation is the most precise statement any library in the stack makes about determinism, and it lines up with what our sim already does ([spec 01](../tier0/specs/01-quantity.md), engine-architecture §5):

| Jolt asks (ES-14) | Our code (ES-15) |
| --- | --- |
| Precise FP, no FP contraction | `zs_strict_fp`: `/fp:precise`; `-ffp-contract=off -fno-fast-math -msse2 -mfpmath=sse` |
| No platform `sin`/`cos` | `zs::detmath` for `exp`, `log` and the rest; only `sqrt` (correctly rounded by IEEE 754) calls the standard library |
| No `std::sort`, `push_heap`, `pop_heap` with ties | Every comparator and heap key ends in an id, so there are no ties |
| No `std::hash` | No unordered containers in `engine/sim` or `engine/gen` |
| Same defines everywhere | One CMake tree; Jolt's `PUBLIC` defines propagate |

What Jolt cannot give, even in its deterministic mode, is a fixed order for queries and callbacks across threads (ES-13). That is the same rule as our phase model: results are requests, sorted before they are applied. If Jolt ever enters the sim, its outputs go through phase 6 (Resolve) like any other request.

A note on floats in data files: toml++ reads doubles through two different code paths depending on the compiler (ES-42). On the standard library we and CI use, they agree to the bit on all our data, so the golden hashes are not at risk from this today. If a macOS or libc++ build is ever added, this is the first thing to recheck.

## SDL_gpu against raw Vulkan

| Need (engine-architecture §8, 0003, 0009) | SDL_gpu 3.4.18 | Vulkan + vk-bootstrap + VMA |
| --- | --- | --- |
| Compute skinning, morphs, clothing wrap | Yes: compute passes, storage buffers (8 per stage) | Yes |
| Clustered forward lights | Yes, light lists in storage buffers | Yes |
| 2-3 frames in flight | Yes, at most 3 | Yes, any |
| Recording on several threads | Yes, one command buffer per thread | Yes, secondary buffers too |
| Transfer queue fed by the I/O thread | No queue choice | Yes, vk-bootstrap selects a dedicated transfer queue |
| GPU timings in Tracy | No timestamp queries | Yes (`vkCmdWriteTimestamp`; Tracy has a Vulkan collector) |
| Many materials without rebinding (bindless) | No unbounded arrays; fixed sets | Yes (descriptor indexing, core in 1.2) |
| GPU-driven culling | Indirect draws, but no indirect count | Yes |
| Mesh shaders, ray tracing | Not near future (FAQ) | Extensions |
| Escape to native Vulkan | No native handles | n/a |
| Portability | Vulkan, D3D12, Metal from one code path | Vulkan only (Windows and Linux; macOS through MoltenVK) |
| Shaders | One format per backend, SDL_shadercross for the rest | SPIR-V only |
| Code we write | Less: no barriers, layouts or memory types | More; vk-bootstrap and VMA remove most boilerplate |

SDL_gpu is the right tool for a portable game with a modest renderer. Ours is Windows-first and aims at visual realism (0009). The features SDL_gpu lacks are the ones a realistic renderer grows into, and it gives no way to drop down to Vulkan for them. Choosing it now would mean rewriting the renderer later. The Tier 0 window ([spec 16](../tier0/specs/16-view-2d.md)) can keep the SDL 2D renderer until the Vulkan renderer starts.

## What this means for us (Proposal)

1. **Keep 0002 as it stands, with five edits** (ES-44): add enkiTS and Tracy; choose cgltf; pin Recast to a commit; move Steam Audio to "later"; say that physics is view-side unless a rule needs it, and then deterministic (ES-45).
2. **Renderer: raw Vulkan with vk-bootstrap and VMA, not SDL_gpu** (ES-47). SDL3 stays the platform layer.
3. **Jolt build settings when it lands:** `CROSS_PLATFORM_DETERMINISTIC=ON` if anything feeds the sim; `USE_STATIC_MSVC_RUNTIME_LIBRARY=OFF`; `JPH_USE_VK`, `JPH_USE_DX12`, `JPH_USE_MTL`, `JPH_USE_CPU_COMPUTE` off in the sim-side build; a golden-hash test with Jolt in the loop on MSVC and GCC.
4. **Ship a `THIRD_PARTY_NOTICES` file** with every MIT, BSD and Apache text (ES-4, ES-7 to ES-9), generated from the `third_party/README.md` table, and add the fetched libraries to that table.
5. **Maintenance bumps, low priority:** SDL to release-3.4.18 and ImGui to v1.92.9b (ES-22, ES-37), doctest to 2.5.x (ES-41). Re-pin MSVC golden hashes only if a bump changes them (none of these touch the sim).
6. **CMake 3.30** if ozz is built through its own CMake (ES-43); otherwise list its runtime sources in our target.

## Open questions

- engine-architecture: does any simulation rule need rigid-body physics, or does Jolt stay view-side (ES-45)?
- engine-architecture: does the sim's grid pathing scale to open 3D ground, or do we add Detour, and if so does a Detour path feed the sim (float results across compilers)?
- 0002: is the renderer raw Vulkan (ES-47), confirmed by the user?
- threats: is audio occlusion driven by the sim's noise path loss (ES-46), so that what a player hears matches what a zombie hears?
- 0002: is a macOS build ever a target? If so, recheck toml++ float parsing on libc++ (ES-42) and SDL_gpu's portability argument.

## Sources

Repos cloned at the tag named, read on 2026-10-08:

- github.com/charles-lunarg/vk-bootstrap @ v1.4.365: `LICENSE.txt`, `README.md`, `CMakeLists.txt`, `src/VkBootstrap.h`
- github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator @ v3.4.0: `LICENSE.txt`, `README.md`
- github.com/zeux/meshoptimizer @ v1.3: `LICENSE.md`, `README.md`, `CMakeLists.txt`
- github.com/libsdl-org/SDL @ release-3.4.18: `LICENSE.txt`, `README.md`, `include/SDL3/SDL_gpu.h`, `SDL_events.h`, `SDL_vulkan.h`, `src/gpu/SDL_sysgpu.h`; `git log release-3.4.8..release-3.4.18`
- github.com/libsdl-org/SDL_shadercross @ 1ff05be: `LICENSE.txt`, `README.txt`, `.gitmodules`
- github.com/jrouwe/JoltPhysics @ v5.6.0: `LICENSE`, `README.md`, `Docs/Architecture.md`, `Docs/ReleaseNotes.md`, `Build/CMakeLists.txt`, `Jolt/Jolt.cmake`, `Jolt/Core/JobSystem.h`, `Jolt/RegisterTypes.h`
- github.com/guillaumeblanc/ozz-animation @ 0.17.0: `LICENSE.md`, `README.md`, `CHANGES.md`, `CMakeLists.txt`, `build-utils/cmake/compiler_settings.cmake`, `include/ozz/base/memory/allocator.h`
- github.com/recastnavigation/recastnavigation @ v1.6.0 and main: `License.txt`, `README.md`, `Recast/Include/RecastAlloc.h`, `Detour/Include/DetourAlloc.h`; `git log v1.6.0..HEAD`
- github.com/jkuhlmann/cgltf @ v1.15: `LICENSE`, `README.md`, `cgltf.h`
- github.com/spnda/fastgltf @ v0.9.1: `LICENSE.md`, `README.md`, `CMakeLists.txt`, `cmake/dependencies.cmake`
- github.com/ocornut/imgui @ v1.92.9b: `LICENSE.txt`, `docs/FAQ.md`, `docs/CHANGELOG.txt`, `imgui.h`, `backends/`
- github.com/mackron/miniaudio @ 0.11.25: `LICENSE`, `README.md`, `CHANGES.md`, `miniaudio.h`
- github.com/ValveSoftware/steam-audio @ v4.8.1: `LICENSE.md`, `TRADEMARK_RIGHTS.md`, `README.md`, `core/THIRDPARTY.md`, `core/CMakeLists.txt`, `core/doc/build-instructions.rst`, `core/src/core/phonon.h`
- github.com/dougbinks/enkiTS @ v1.12: `License.txt`, `README.md`
- github.com/wolfpld/tracy @ v0.14.1: `LICENSE`, `CMakeLists.txt`
- github.com/doctest/doctest @ v2.5.3: `LICENSE.txt`, `CHANGELOG.md`; github.com/marzer/tomlplusplus @ v3.4.0 and master log; github.com/emil-e/rapidcheck (`git ls-remote`)
- Licence files read through raw.githubusercontent.com: NVIDIA-Omniverse/PhysX `LICENSE.md` @ main, erincatto/box2d `LICENSE` @ main, nfrechette/acl `LICENSE` @ develop
- This repo @ 8295162: `CMakeLists.txt`, `apps/zs/CMakeLists.txt`, `apps/zs/window.cpp`, `third_party/README.md`, `third_party/tomlplusplus/toml.hpp`, `engine/sim/src/*.cpp`, `engine/gen/src/*.cpp`
- Web-search summaries: guerrilla-games.com/read/architecting-jolt-physics-for-horizon-forbidden-west (Jolt in Horizon Forbidden West); phoronix.com/news/Steam-Audio-SDK-Fully-Open (Steam Audio Apache 2.0, February 2024)
