# Fixes from the engineering research (2026-10-08)

A work list for a **local session on Windows** (MSVC), built from six research notes and decided by default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md). Each package names the spec to update first (a unit's spec is written before its code), the code with line numbers, the finding IDs behind it, and how to check it. Line numbers are at commit `8295162` (code identical to `origin/main` `2646536`); re-check them before editing.

**Status of everything below: Proposal, not Implemented.** The spec wording of packages 1, 3 and 4 was written on 2026-10-08 (specs 01, 02, 05, 06, 16, 40 and engine architecture §5, each marked "not built yet"); no code has been changed. Work in order; push after each package; tick the box in this file when a package is Implemented and say how it was Validated.

## Order

| # | Package | Why first | Size |
| --- | --- | --- | --- |
| 1 | [Window: WASD and readability](#1-window-wasd-and-readability) | The user's two complaints ([status](../status.md)) | Large, view-side only |
| 2 | [Saves written safely](#2-saves-written-safely) | Can lose a save today | Small |
| 3 | [Determinism guards](#3-determinism-guards) | Goldens checked only on MSVC; two latent traps | Small to medium |
| 4 | [Save format 4 and zstd](#4-save-format-4-and-zstd) | 10x smaller saves, spec 06 rule 4 | Medium |
| 5 | [Replay viewer without thinning](#5-replay-viewer-without-thinning) | Long runs lose frames today | Small |
| 6 | [Library pins and notices](#6-library-pins-and-notices) | Bug fixes upstream; licences | Small |
| 7 | [Later, when the system needs it](#7-later-when-the-system-needs-it) | Not needed by a scenario yet | — |

## 1. Window: WASD and readability

- [x] **Implemented 2026-10-08** (`apps/zs/window.cpp`; the walker-arrived line reworded in `tier0.cpp` and `headless.cpp`). Validated: release and debug `ctest` on MSVC with every golden unchanged; `--shot` frames before and after at ticks 600, 28800, 57600 and 100800 with the overlays. Not yet validated by the user playing it. Two estimates: need bands at 0.33/0.6/0.85, and 30 % blood lost as severe bleeding. Not done: cold or heat in the state column (bodies do not read the weather yet).
- **Spec first** (written 2026-10-08: spec 16 rules 4-13, spec 40 rule 10): [spec 16](specs/16-view-2d.md): add the follow camera, the state column, map key, shapes and palette, WASD wall rule and cancel, and the walking cap (0055 §2). [spec 40](specs/40-time.md): the window uses the `Pacer`; the alerts list (0055 §2.7).
- **Code:** `apps/zs/window.cpp`. All fixes are view-side: the window keeps no game state and only sends intents (spec 16 rule 1). Full table with reasons: [view-readability](../research/view-readability.md) "What this means for us".

| Fix | Where | Change | Findings |
| --- | --- | --- | --- |
| 1.1 Time | `:35-36`, `:149-168`, `:447` | Drive time with `Pacer`; ladder from `Pace::Real`; labels from `pace_name`; cap `walking` at X3 while WASD is held or the controlled pop approaches, Real in a fight; show the limiting cap | VR-1..3 |
| 1.2 Walls | `:409-412` | Require `map->can_walk(SiteMap::edge_between(p, to))`; diagonal only if both orthogonal edges pass, else the passable axis; flash the blocking edge | VR-4..6 |
| 1.3 Smooth bodies | `:278`, `:288-293` | Draw at `pos + (path[path_next] - pos) * step_m / length` while approaching (read-only), as OpenRCT2's tweener | VR-9..11, RN |
| 1.4 Walkers apart | `:275-281` | Offset walkers sharing a cell as pops are, or a count badge | RN |
| 1.5 Follow camera | `:59-60`, `:687-693`, `:377-380` | Target in metres; `k = 1 - exp(-dt / 0.1)`; dead zone 20 %; pan breaks; F or Home recentres; WASD resumes; clamp to map | VR-12, 14-22 |
| 1.6 Floor follows | `:94`, `:122`, `:667`, `:289` | While following, `a.level` = the pop's `z`; PgUp/PgDn break the follow | VR-13 |
| 1.7 Paused or busy | `:408-409` | While paused show "Paused: Space to run"; while busy a WASD press submits `Cancel` then the move | VR-7 |
| 1.8 No duplicate moves | `:409` | Send the next move only after `clock.tick` passes the tick the last one was sent at | VR-8 |
| 1.9 Labels | `:243-247` | Per-kind 2-3 letter labels with a "Map key" panel; drop item spawn markers; outline object squares | VR-28, 33 |
| 1.10 Shapes | `:274-281`, `:293-297` | Walkers diamonds, pops discs; active walkers a line to their target; controlled pop thick ring and arrow; selection a second ring | VR-30, 31 |
| 1.11 Palette | `:190` | Tol "bright" (blue, cyan, yellow, purple); keep initials | VR-31, 32 |
| 1.12 Initials | `:295` | Centre with `ImGui::CalcTextSize`; disc radius at least 0.42 cell, one letter under 24 px | VR-29 |
| 1.13 Bars and state column | `:485-489`, `:519-525` | Colour bars by their thresholds with a word; invert morale; state column of the 3-5 worst states by the pop and in the Group panel | VR-35, [PZ-13](../research/project-zomboid.md) |
| 1.14 Log | `:577-596`, `:529-533` | Coalesce identical lines within a game minute as "x N"; group wounds by region; reword "arrived, dead" | VR-25, 35 |
| 1.15 Overlays | `:235-240` | Legend with scale and background dB; "nothing above background" when empty; light shows only cells above outdoor ambient | VR-34 |
| 1.16 Targets | `:381-397` | Outline hovered cell and the menu's cell; draw the controlled pop's path dotted | VR-36 |
| 1.17 Map size | `:688-692` | Fit below the Time window's real height and left of panels; start at about 32 px per cell with follow | VR-27 |
| 1.18 Alerts | `:140-146` | Route alert lines to `Pacer::alert`; an alert column naming threat, distance, direction | VR-26, 38 |

- **Check:** `ctest --preset release` passes with every golden unchanged (view-only change). Take `zs --shot before.bmp --ticks 600` and `zs --shot after.bmp --ticks 600` (and `--overlay noise`, `--overlay light`) and compare. Then the user plays it: WASD feels like walking, the camera follows, a walker is told apart from a pop at a glance. A passing test is not visual validation; the user's check is.

## 2. Saves written safely

- [x] **Implemented 2026-10-08.** The window's F5 goes through `zs::write_file`; `write_file` writes with `fopen("wbc")` on Windows (the CRT commit flag makes the flush durable) and `fsync` plus a directory `fsync` on POSIX. Validated: release and debug `ctest` on MSVC, goldens unchanged. Not yet checked: killing the process mid-write.
- **Spec:** [spec 06](specs/06-save-replay.md) rules 5 and 13 already require it; no change.
- `apps/zs/window.cpp:99-104` — `save_game` writes with a plain `std::ofstream`, bypassing `zs::write_file` (no temp file, read-back or rename). Call `zs::write_file(save_path(a), bytes)` in a try/catch that sets the status message. ([DC](../research/data-compression.md))
- `engine/sim/src/archive.cpp:42-56` — `write_file` never flushes to disk before the rename, nor the directory after. Flush and `FlushFileBuffers` (Windows) or `fsync` (POSIX) the temp file before rename; `fsync` the directory on POSIX. (DC)
- **Check:** `test_save_replay.cpp` still passes; F5 then F9 in the window round-trips; a save interrupted (kill the process mid-write) leaves the old file intact.

## 3. Determinism guards

- [x] **Implemented 2026-10-08**, except the negative check (goldens built with contraction on must fail), which waits for a CI job that builds that way. `.github/workflows/linux.yml` runs the goldens on GCC and Clang (green on the Linux CI decides it; not seen from this PC); clang-cl gets `/clang:-ffp-contract=off` and MSVC older than 2022 stops at configure; `detmath` `sin` and `cos` return NaN beyond 2^30 and for infinities, `round_to_int` asserts its range (`test_detmath.cpp`); `World::step` restores MXCSR and counts it in `fp_env_restored` (`test_clock.cpp`); `ArchiveWriter::f64` asserts no NaN (never fired in the debug suite); `TOML_FLOAT_CHARCONV=1` on GCC 12+. Validated: release and debug `ctest` on MSVC, goldens unchanged.
- **Spec first** (written 2026-10-08: spec 01 rule 4, determinism target and tests; spec 02 rule 3; spec 05 rule 7; engine architecture §5 items 5-6): [spec 01](specs/01-quantity.md) rule 4: name clang-cl and ARM64, require MSVC 2022+, make the float path "required, measured on GCC 13 and Clang 18", add the negative check "goldens fail with contraction on". [spec 02](specs/02-draw.md): `std::` engines only in tests; forbid `std::shuffle` and `std::sample`. [spec 05](specs/05-clock.md) rule 7: ban `std::sort` without a total order and `std::hash` in anything saved or hashed. [engine architecture](../design/engine-architecture.md) §5: per-worker FP environment, Jolt flags, fixed reduction order. ([DET](../research/determinism.md) §5)
- `.github/workflows/` — add a Linux job: GCC and Clang, `cmake --preset release -DZS_BUILD_WINDOW=OFF`, `cmake --build --preset release`, `ctest --preset release` (install Ninja). Measured passing on GCC 13.3 and Clang 18.1 at `8295162`. (DET)
- `CMakeLists.txt:15-16` — clang-cl takes the `if(MSVC)` branch and keeps contraction on. Inside it, when `CMAKE_CXX_COMPILER_ID` is `Clang`, add `/clang:-ffp-contract=off`. (DET-16, measured: contraction breaks 7 goldens)
- `engine/sim/src/detmath.cpp:21-23, 121-123, 134-136` — `round_to_int` casts unbounded doubles (`sin(1e19)` = -1.9e34, `sin(inf)` = inf). Return NaN for non-finite input; assert `|x| <= 2^30` in `sin`/`cos`; assert `|t| < 2^62` in `round_to_int`; add those cases to `test_detmath.cpp`. (DET-8)
- `engine/sim/src/world.cpp:35` (`step`, also `jump`) — no FP-environment check. A small RAII guard (like Jolt's `FPControlWord`) checks MXCSR rounding and FTZ/DAZ and restores the default; count or throw in debug. (DET-40)
- `engine/sim/src/archive.cpp:10` (`ArchiveWriter::f64`) — assert no NaN is saved (0055 §3.4). (DET-37)
- `engine/sim/CMakeLists.txt:50` and `engine/gen/CMakeLists.txt` — define `TOML_FLOAT_CHARCONV=1` on GCC 12+. (DET-38)
- **Check:** all goldens unchanged on MSVC; the Linux job green on GCC and Clang.

## 4. Save format 4 and zstd

- [ ] **Library ready 2026-10-09; the save code is not written.** zstd 1.5.7 is vendored in `third_party/zstd` (the official release tarball, checksum checked, library only), built as `zs_zstd` single-threaded without the assembly decoder; `engine/sim/tests/test_zstd.cpp` round-trips a save at levels 1, 3 and 19 and refuses a corrupted frame. Still to do: save format version 5 (spec 06 rule 16; version 4 went to spec 50), writing and reading compressed saves, the autosave history of rule 18.
- [ ] **Spec first** (written 2026-10-08: spec 06 rules 15-20): [spec 06](specs/06-save-replay.md): format version 4 header (codec, uncompressed size, binary hash, data hash, seed, settings); zstd levels and frame rules (0055 §4); one full save per game day plus deltas in the autosave history; goldens hash uncompressed bytes.
- Vendor zstd (a pinned release, BSD licence) in `third_party/` and list it in `third_party/README.md`.
- `engine/sim/src/world.cpp:164-175` — write the version 4 header; keep the version 3 reader.
- Compress with zstd level 3 (saves) or 1 (autosaves, traces), single-threaded, checksum on.
- **Check:** a version 3 save still loads; save size at day 14 on seed 1 drops from about 213 KB to about 21 KB (DC measured); `test_save_replay.cpp` extended for version 4.

## 5. Replay viewer without thinning

- [ ] `tools/viewer/embed.py:29-36` — instead of dropping frames to fit `--max-mb`, embed the gzip of the whole trace in base64 (about 360 KB for the 14-day run) and decode it in `tools/viewer/replay.html` with `DecompressionStream("gzip")`. Keep `--max-mb` as a fallback. Later: store frames as delta-coded columns (180 KB to 19 KB, DC).
- **Check:** `zs --run --seed 1 --trace trace.json`, embed, open in a browser, scrub to the last frame.

## 6. Library pins and notices

- [x] **Implemented 2026-10-09** with the user's permission to download: SDL release-3.4.18 (`829a65d7`), Dear ImGui v1.92.9b (`f1cc2ae1`), doctest 2.5.3 vendored; `third_party/README.md` lists the vendored and the fetched libraries with their licences, ready for a notices file. Validated: release and debug `ctest` on MSVC (see the commit). The notices file itself is not generated yet.
- [ ] `apps/zs/CMakeLists.txt:16` — SDL `release-3.4.8` to `release-3.4.18` (GPU, D3D12 and Vulkan renderer fixes). ([ES](../research/engine-stack.md))
- `apps/zs/CMakeLists.txt:20` — ImGui `v1.92.9` to `v1.92.9b` (two regressions fixed). (ES-37)
- `third_party/README.md` — doctest 2.4.11 to 2.5.3 when convenient; list the fetched SDL3 and ImGui with their licences, so a notices file can be generated (0055 §1.8).
- **Check:** Windows CI build green; the window opens and `--shot` matches.

## 7. Later, when the system needs it

- Storage ([ECS](../research/ecs-memory-layout.md)): `engine/sim/include/zs/pop_registry.h:183` (`records_` `std::map`, about 25 ms per pass at 100k records) to a record slot table; `roster.h:64` (`stints_`) to a sorted vector of `(PopId, joined, left)`; `ledger.h:94` (`balances_`) behind a Tracy zone first. Rerun the ECS benchmark on MSVC before writing the layout into a spec.
- Renderer ([RN](../research/renderer.md)): when a scenario needs 3D, follow RN's frame structure, crowd tiers and pitfall list.

## Housekeeping for the next session

- ~~Branch `claude/intelligent-wright-4dliah`~~: merged 2026-10-08; its note is in the 0054 format (IDs DMO) under "Engine and rendering".
- Every design doc touched by the research now has a "From research" list in its Open section; answer or close items there as packages land.
