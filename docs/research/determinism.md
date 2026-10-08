# Cross-compiler and cross-platform determinism: pitfalls and fixes

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [spec 01](../tier0/specs/01-quantity.md), [spec 02](../tier0/specs/02-draw.md), [spec 05](../tier0/specs/05-clock.md), [spec 06](../tier0/specs/06-save-replay.md), [engine architecture](../design/engine-architecture.md), [0002](../decisions/0002-engine-stack.md), [0041](../decisions/0041-threading-and-partition.md). **Evidence:** primary sources read from git (Jolt Physics, Box2D v3, libfixmath, the C++ working draft, the MSVC `/fp` page in MicrosoftDocs/cpp-docs, the Clang users' manual and option table, GCC's `invoke.texi`, the `<random>` and `std::hash` code of MSVC STL, libstdc++ and libc++); our code audited by grep at commit 8295162 (simulation code identical to `origin/main` 2646536); measurements run by Claude on Linux x86-64 (4 cores, AVX2 and FMA present), GCC 13.3.0 and Clang 18.1.3. Libm accuracy, Factorio and the new MSVC math library come from search summaries (`reported`). Not tested: MSVC itself (our goldens are pinned there), ARM64, macOS. **IDs:** DET.

Our golden hashes are pinned on MSVC and [spec 01](../tier0/specs/01-quantity.md) asks other compilers to reproduce them. This note collects what breaks bit-identical results across compilers, standard libraries and CPUs, how Jolt and Box2D defend against it, and audits our code against that list, with a direct measurement on GCC and Clang.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| DET-1 | GCC 13.3 (release preset, `-DZS_BUILD_WINDOW=OFF`) builds the tree and passes every test: 269 of 269 simulation test cases (1,716,538 assertions) and all generator tests, including every golden hash pinned on MSVC (body, demo, detmath, stages 4, 6, 7, 8, 9, map, grid, fields, town). | sourced | `cmake --preset release -DZS_BUILD_WINDOW=OFF -DCMAKE_CXX_COMPILER=g++ && cmake --build --preset release && ctest --preset release` on an export of 8295162; Linux x86-64 |
| DET-2 | Clang 18.1.3 (Release, same flags) passes the same tests and goldens. | sourced | same build with `-DCMAKE_CXX_COMPILER=clang++` |
| DET-3 | The detmath golden (`0x7fccc6b0bd223228`, 500,000 outputs) is reproduced by g++ and clang++ at `-O2 -ffp-contract=off`, with and without `-march=haswell` (FMA instructions available). | sourced | harness `fmaprobe.cpp` linking `engine/sim/src/detmath.cpp`, same inputs as `test_detmath.cpp` |
| DET-4 | With contraction allowed the detmath hash changes: g++ `-march=haswell -ffp-contract=fast` gives `0x3c726d0ec8d5582c`, clang++ `-march=haswell` with its default contraction gives `0x8e757a0b3341708d`. FMA contraction alone breaks the goldens as soon as the target has FMA. | sourced | same harness |
| DET-5 | `-ffast-math` changes the detmath hash (`0xde102884579a9188`) and makes 88% of `detmath::exp` outputs differ from the strict build. | sourced | same harness, `g++ -O3 -ffast-math` |
| DET-6 | detmath and glibc disagree in the last bits often: `exp` in 10,162 of 100,000 inputs, `log` 525, `sin` 19,144. Replacing one `detmath::` call by `std::` (or back) changes the hashes. | sourced | same harness |
| DET-7 | A whole-suite build with FMA contraction on (Clang, `-march=haswell -ffp-contract=on` placed after our flags, which is what clang-cl's `/fp:precise` amounts to) fails 7 of 269 simulation test cases, all of them golden hashes (detmath, body, stages 4, 6, 7, 8, 9); the generator goldens still pass. The control build with `-march=haswell` and our `-ffp-contract=off` last passes them all. | sourced | `build-fmaon` and `build-fma` in the scratchpad, `ctest`; see §4 for the failing list |
| DET-8 | `detmath::sin` and `cos` convert `x · 2/π` to `int64` without a range check: above about 1.4e19 that is undefined behaviour (UBSan flags `detmath.cpp:22`), `sin(1e19)` returns -1.9e34 and `sin(inf)` returns inf instead of NaN. Accuracy against glibc is 1.3e-16 at 1e3, 2.5e-9 at 1e8, 2.6e-7 at 1e10. | sourced | `bigsin.cpp`, `midsin.cpp` probes, g++ 13.3 with and without `-fsanitize=float-cast-overflow` |
| DET-9 | UBSan (`-fsanitize=undefined,float-cast-overflow`, GCC 13.3) over the whole test suite reports no undefined behaviour: 0 runtime errors in 269 simulation and 42 generator test cases (2.09 million assertions). The detmath cast of DET-8 is latent: no simulation input reaches it. | sourced | `build-ubsan`, `zs_sim_tests` and `zs_gen_tests` run directly, stderr searched for `runtime error` |
| DET-10 | MSVC `/fp:precise` does not generate contractions by default from Visual Studio 2022; earlier versions could. `/fp:contract` is new in VS 2022 and `/fp:fast` enables it. | sourced | github.com/MicrosoftDocs/cpp-docs `docs/build/reference/fp-specify-floating-point-behavior.md` @ 281a0a0 |
| DET-11 | MSVC `/fp:precise` rounds to source precision at assignments, casts, call arguments and returns, and does no reassociation or distribution unless the result is bitwise identical. | sourced | same file |
| DET-12 | MSVC's default floating-point environment: exceptions masked, round to nearest, subnormals preserved. | sourced | same file, "default floating point environment" |
| DET-13 | GCC's default is `-ffp-contract=fast` (fuse wherever the target can), except for C in a strict ISO mode; `on` fuses within one expression only. | sourced | github.com/gcc-mirror/gcc `gcc/doc/invoke.texi` @ 9b2bbe9 |
| DET-14 | GCC rounds excess precision away at casts and assignments (`-fexcess-precision=standard`) by default under a strict `-std=c++NN`, which `CMAKE_CXX_EXTENSIONS OFF` gives us; on x86-64 scalar maths is SSE2 by default, so x87 does not arise. | sourced | same file, `-fexcess-precision` and `-mfpmath` |
| DET-15 | Clang's default model `precise` has contraction on (`-ffp-contract=on`); `-fno-fast-math` alone resets contraction to the default, but a later or earlier explicit `-ffp-contract` wins. | sourced | github.com/llvm/llvm-project `clang/docs/UsersManual.md` @ 4a428b1 |
| DET-16 | clang-cl maps `/fp:precise` to `-ffp-model=precise`, so contraction stays on; Box2D passes `/clang:-ffp-contract=off` for clang-cl because the plain flag is dropped there. | sourced | llvm-project `clang/include/clang/Options/Options.td` line 9445 @ 4a428b1; github.com/erincatto/box2d `CMakeLists.txt` @ ac7c751 |
| DET-17 | `-ffast-math` links `crtfastmath.o`, whose static constructor sets FTZ and DAZ in MXCSR for the whole program, every library included. | sourced | Clang users' manual, "A note about crtfastmath.o" |
| DET-18 | The C++ standard lets floating results be held in greater precision and range than their type; that sentence is what licenses x87 excess precision and contraction. | sourced | github.com/cplusplus/draft `source/expressions.tex` [expr.pre] @ 9438b11 |
| DET-19 | The algorithms of the standard random distributions are implementation-defined; engines are fully specified (the 10,000th `mt19937` output is 4123659995 everywhere). | sourced | cplusplus/draft `source/numerics.tex` [rand.dist.general], [rand.predef] @ 9438b11 |
| DET-20 | The three standard libraries really differ: all use Marsaglia's polar method for `normal_distribution` but each calls its own `std::log`, and MSVC adds a rescaling branch for tiny radii; `uniform_int_distribution` is Lemire's method in libstdc++, independent bits with rejection in libc++, and `_Rng_from_urng_v2` in MSVC STL. | sourced | microsoft/STL `stl/inc/random` @ da52dc0; gcc `libstdc++-v3/include/bits/random.tcc`, `uniform_int_dist.h` @ 9b2bbe9; llvm-project `libcxx/include/__random/` @ 4a428b1 |
| DET-21 | `std::sort` is not stable and implementations differ, so equal keys may land in different orders; `stable_sort` is specified stable. Jolt also bans `std::push_heap`/`pop_heap` for the same reason. | sourced | cplusplus/draft `source/algorithms.tex` [sort], [stable.sort]; JoltPhysics `Docs/Architecture.md` @ 8c605f1 |
| DET-22 | A `std::hash` value need only be stable within one execution. MSVC hashes bytes with FNV-1a; libstdc++ returns integers unchanged and hashes strings with Murmur, so unordered iteration order differs between them. | sourced | cplusplus/draft `source/lib-intro.tex` (Cpp17Hash); microsoft/STL `stl/inc/type_traits`; gcc `libstdc++-v3/libsupc++/hash_bytes.cc`, `bits/functional_hash.h` |
| DET-23 | Undefined behaviour that compilers resolve differently: signed overflow ([expr.pre]), float-to-integer conversion out of range ([conv.fpint]), shifts by a negative amount or at least the width ([expr.shift]); `char`'s underlying type is implementation-defined (signed on x86, unsigned on ARM). | sourced | cplusplus/draft `source/expressions.tex`, `source/basic.tex` |
| DET-24 | Jolt's `CROSS_PLATFORM_DETERMINISTIC` costs about 8% and is deterministic across compiler (MSVC 2022, Clang, GCC, Emscripten), configuration, OS, architecture and word size, checked on every build on 14 platform combinations. | sourced | JoltPhysics `Docs/Architecture.md` "Deterministic Simulation" @ 8c605f1 |
| DET-25 | Jolt's own rules for applications: precise mode, contraction off, the same FPU state on every platform and thread (rounding to nearest, DAZ and FTZ set consistently), its own `Sin`/`Cos`, no `std::sort`, heap functions or `std::hash`. In that mode it drops FMA and its approximate reciprocal square root. | sourced | same doc; `Jolt/Jolt.cmake`, `Jolt/Math/Math.h`, `Build/CMakeLists.txt` @ 8c605f1 |
| DET-26 | Even deterministic, Jolt calls its listeners from several threads in no fixed order, returns active bodies in no fixed order and does not make broadphase queries deterministic; callers must sort. | sourced | same doc |
| DET-27 | Box2D v3 is deterministic across thread counts and 64-bit platforms by creation-order simulation, `/fp:precise` on MSVC, contraction off on GCC and Clang, and its own `atan2`, `cos`, `sin`; one expected hash (`0x36564C94`, sleep step 295) is checked for every platform and worker count on each pull request. | sourced | erincatto/box2d `docs/simulation.md`, `shared/determinism.h`, `test/test_determinism.c` @ ac7c751 |
| DET-28 | Box2D's replay contract enforces struct layout, pointer width, endianness and format version, and says the floating-point environment must match; `-ffast-math` is unsupported. | sourced | box2d `docs/recording.md` @ ac7c751 |
| DET-29 | libfixmath is Q16.16: range ±32768 at 1.5e-5 resolution, its sine accurate to about 2.1%; fixed point buys determinism with range and accuracy we would not accept for body chemistry or fields. | sourced | github.com/PetteriAimonen/libfixmath `README.md` @ ed9391c |
| DET-30 | Neither glibc nor the MSVC CRT promises correctly rounded `exp`, `log`, `sin`; both aim at about 1 ulp and differ in last bits; CORE-MATH's correctly rounded functions are entering glibc one function at a time. | reported | learn.microsoft.com CRT floating-point support; patchwork.sourceware.org CORE-MATH patches; Zimmermann's libm accuracy reports |
| DET-31 | Microsoft announced (September 2026) an experimental math library for constexpr `<cmath>` built on LLVM libc, off by default, that can change accuracy. | reported | devblogs.microsoft.com/cppblog/msvc-c23-constexpr-cmath-with-llvm-libc/ |
| DET-32 | Factorio 1.1.95 added cross-platform checks for x86 and ARM floating-point conversions after multiplayer desyncs; a forum report also warns that software FMA fallbacks mishandle subnormals. | reported | forums.factorio.com/viewtopic.php?p=595966, ?p=703180 |
| DET-33 | Our CMake strict-FP target (`/fp:precise` on MSVC; `-ffp-contract=off -fno-fast-math` on GCC and Clang; `-msse2 -mfpmath=sse` on x86) is the Jolt and Box2D recipe, and it reaches `zs_gen`, `zs` and the tests through `zs_sim`'s PUBLIC link. | sourced | `CMakeLists.txt:14-22`, `engine/sim/CMakeLists.txt:52`, `engine/gen/CMakeLists.txt` |
| DET-34 | `engine/sim` and `engine/gen` call no libm transcendental; the `<cmath>` they use is exact by IEEE 754 (`sqrt`, `floor`, `round`, `lround`, `llround`, `abs`, `fabs`); there is no `float` and no `long double`. | sourced | grep over `engine/sim/{src,include}`, `engine/gen/{src,include}` @ 8295162 |
| DET-35 | No `<random>` distribution, unordered container, `std::hash` or thread exists in `engine/sim` or `engine/gen`; the one `std::mt19937_64` drives a property test's command sequence (an engine, fully specified). | sourced | grep; `engine/sim/tests/test_embodied_pops.cpp:213` |
| DET-36 | Every `std::sort` sorts unique keys or has an id tie-break (`walkers.cpp:185`, `world_fields.cpp:152`, `:257`, `world_people.cpp:132`, `catalog.cpp:104-106`, `world_grid.cpp:190`); ties elsewhere use `stable_sort`; directory listings are sorted before use. | sourced | grep and reading each call site |
| DET-37 | Saves and hashes are portable by construction: little-endian fixed-width fields, doubles as their bits, strings as u32 length plus bytes, FNV-1a; generator string hashes cast `char` to `unsigned char`. NaN payloads and signs are stored as found. | sourced | `engine/sim/include/zs/archive.h`, `engine/sim/src/archive.cpp:10`, `engine/gen/include/zs/gen/keyed.h:23-27` |
| DET-38 | toml++ 3.4.0 parses data-file floats with `std::from_chars` on MSVC but with a classic-locale `std::stringstream` on GCC and Clang: two code paths for every calibration number, equal today (DET-1). | sourced | `third_party/tomlplusplus/toml.hpp:1106-1121`, `:14358-14398` |
| DET-39 | CI builds and runs the goldens only on Windows MSVC; no GCC, Clang or ARM job exists, though [spec 06](../tier0/specs/06-save-replay.md) asks for golden hashes "checked in CI per compiler". | sourced | `.github/workflows/windows.yml` @ origin/main |
| DET-40 | The window steps the simulation on the main thread next to SDL's renderer and graphics driver (`apps/zs/window.cpp:155`), and nothing sets or checks the floating-point environment before a tick. | sourced | grep for `_mm_setcsr`, `_controlfp`, `fesetround`: none |
| DET-41 | Today's results are portable for the right reasons (DET-1 to DET-3 and DET-33 to DET-37); the open risks are a clang-cl or ARM64 build that contracts (DET-4, DET-7, DET-16), an FP environment changed by a driver or library (DET-17, DET-40), out-of-range float-to-int conversions (DET-8, DET-32) and the untested second toolchain in CI (DET-39). | derived | from DET-1 to DET-40 |
| DET-42 | Fixes, in order of value: a Linux GCC and Clang CI job; clang-cl flags; a range guard in detmath's reduction; an FP-environment guard per step; NaN canonicalised in the archive; one float-parsing path in toml++; the threaded determinism test of [0041](../decisions/0041-threading-and-partition.md) when workers arrive. | estimate | this note, §5 |

## 1. Answer in one page

1. **Measured: our goldens hold on GCC 13 and Clang 18 on x86-64.** All 269 simulation test cases and the generator tests pass, every golden pinned on MSVC included (DET-1, DET-2). The float path is cross-compiler today, which [spec 01](../tier0/specs/01-quantity.md) only called "a goal".
2. **It holds because the code already follows the Jolt and Box2D rules**: strict FP flags on every target (DET-33), no libm transcendental (DET-34), our own samplers on Philox, no `<random>` distribution, no unordered container, no threads (DET-35), sorts with tie-breaks (DET-36), a byte-exact little-endian archive (DET-37).
3. **The one thing that would break it silently is FMA contraction.** On a target with FMA, contraction changes the detmath hash and the simulation goldens (DET-4, DET-7). GCC and Clang get `-ffp-contract=off` from us; MSVC 2022 does not contract under `/fp:precise` (DET-10). **clang-cl does** (DET-16), and our CMake gives clang-cl only `/fp:precise`, because `if(MSVC)` is true for it. ARM64 has FMA in the base instruction set, so the same applies to any ARM build that loses the flag.
4. **Second risk: the floating-point environment.** Rounding mode and FTZ/DAZ live in a per-thread register that any library or driver in the process can change (DET-17, DET-25). The window ticks the simulation on the same thread as the graphics driver (DET-40). A per-step guard costs one register read.
5. **Third: undefined behaviour in conversions.** `detmath::sin` and `cos` cast an unbounded double to `int64` (DET-8). x86 gives `INT64_MIN` for out-of-range values, other CPUs saturate, and Factorio shipped a fix for exactly this x86/ARM difference (DET-32). Inputs today are small; a guard makes it safe for good.
6. **What we must keep not doing**: `std::*_distribution` (DET-19, DET-20), `std::exp`/`log`/`sin` (DET-6, DET-30, DET-31), `std::sort` on ties, `std::hash` and unordered iteration (DET-21, DET-22), `-ffast-math` or `/fp:fast` anywhere in the process (DET-5, DET-17), `long double`, and pointer values in any key.
7. **Fixed point is not needed.** Box2D and Jolt get cross-platform determinism in floating point with flags and their own trigonometry (DET-24, DET-27); libfixmath's Q16.16 would cost range and accuracy (DET-29). Our split (integers for conserved quantities, strict doubles for continuous ones) stays.

## 2. The pitfalls, one table

| Pitfall | What changes | Defence used by Jolt / Box2D | Ours |
| --- | --- | --- | --- |
| FMA contraction | `a*b+c` rounded once instead of twice | contraction off; Jolt drops its FMA path | GCC/Clang `-ffp-contract=off`; MSVC 2022 off by default; **clang-cl not covered** |
| Fast-math | reassociation, reciprocal, FTZ via `crtfastmath.o` | precise model only | `-fno-fast-math`, `/fp:precise` |
| x87 excess precision | 80-bit intermediates | SSE2 (`-mfpmath=sse`) | x64 only; `-msse2 -mfpmath=sse` on x86 |
| Denormals (FTZ/DAZ) | subnormals become 0 | same MXCSR on every thread | default environment, **never checked** |
| Rounding mode | every operation | assert round-to-nearest | **never checked** |
| libm transcendentals | last bits differ by libm and version | own `Sin`, `Cos`, `Atan2` | `detmath` from `+ − × ÷ √` |
| `<random>` distributions | algorithm per library | own samplers | own samplers on Philox ([spec 02](../tier0/specs/02-draw.md)) |
| `std::sort`, heaps | order of equal keys | own `QuickSort`, `BinaryHeapPush` | tie-breaks, `stable_sort` |
| `std::hash`, unordered containers | iteration order | own `Hash` | none used; `std::map` and sorted vectors |
| Thread completion order | order of merges and sums | creation order, sorted events | single thread; [0041](../decisions/0041-threading-and-partition.md) for later |
| Float-to-int out of range | `INT64_MIN` on x86, saturation on ARM | not stated | **detmath reduction unguarded** |
| Signed overflow, shifts | optimiser assumptions | not stated | UBSan over the suite: none found (DET-9) |
| `char` signedness | hashes of text | not stated | cast to `unsigned char` |
| Endianness, word size | save bytes | layout hash, endianness check | explicit little-endian, fixed widths |
| Number parsing | decimal to double | not stated | correctly rounded on both paths; two paths |
| NaN bits | NaN sign and payload differ by CPU | not stated | stored as found |

## 3. How the two physics engines do it

**Jolt** offers a compile-time switch. Without it Jolt is deterministic only for the same binary; with it, for any compiler, configuration, OS, CPU architecture and word size, at about 8% cost, and its CI checks 14 platform combinations for the same results (DET-24). The switch turns off FMA and the approximate reciprocal square root (DET-25). Its advice to applications is the checklist we already follow, plus one item we do not: keep the FPU state the same on every thread, rounding and denormal flags included. When Jolt arrives ([0002](../decisions/0002-engine-stack.md)), three of its outputs come in thread order (listeners, active bodies, broadphase queries) and must be sorted before the simulation reads them (DET-26).

**Box2D v3** makes determinism the only mode. It uses compiler flags (precise on MSVC, contraction off on GCC and Clang, forced through `/clang:` for clang-cl), its own `atan2`, `cos` and `sin`, and simulation order by creation order so thread count does not matter. One expected hash is asserted for every platform and worker count on each pull request (DET-27): the same shape as our goldens. Its replay files carry a layout hash and refuse a build whose layout, pointer width or endianness differs (DET-28). Our archive is field by field, so it is portable rather than refused.

## 4. Measurements on our code

All runs on an export of commit 8295162 (simulation code identical to `origin/main` 2646536), Linux x86-64, 4 cores, AVX2 and FMA present.

| Build | Result |
| --- | --- |
| GCC 13.3, release preset, window off | all tests pass; every MSVC-pinned golden matches (DET-1) |
| Clang 18.1.3, Release, window off | all tests pass (DET-2) |
| Clang 18.1.3, `-march=haswell`, our `-ffp-contract=off` last | all tests pass (control for DET-7) |
| Clang 18.1.3, `-march=haswell -ffp-contract=on` last | 7 of 269 simulation test cases fail, exactly the goldens: detmath, body, stages 4, 6, 7, 8, 9; generator tests pass (DET-7) |
| GCC 13.3, `-fsanitize=undefined,float-cast-overflow` | all tests pass; 0 UBSan reports in 311 test cases (DET-9) |

detmath alone (100,000 inputs per function, the inputs of `test_detmath.cpp`):

| Compiler and flags | Hash | Outputs differing from glibc (exp / log / sin) |
| --- | --- | --- |
| g++ or clang++ `-O2 -ffp-contract=off` | `0x7fccc6b0bd223228` (the golden) | 10,162 / 525 / 19,144 |
| same with `-march=haswell` | `0x7fccc6b0bd223228` | same |
| g++ `-march=haswell -ffp-contract=fast` | `0x3c726d0ec8d5582c` | 5,976 / 485 / 18,172 |
| clang++ `-march=haswell` (default contraction) | `0x8e757a0b3341708d` | 5,976 / 525 / 18,172 |
| g++ `-O3 -ffast-math` | `0xde102884579a9188` | 88,103 / 41,113 / 91,096 |

`detmath::sin` at large arguments: relative error to glibc 1.3e-16 at 1e3, 0 at 1e5 and 1e6, 2.5e-9 at 1e8, 2.6e-7 at 1e10; `sin(1e19)` = -1.9e34 and `sin(inf)` = inf, with UBSan reporting the cast at `detmath.cpp:22` (DET-8).

## 5. What this means for us (Proposal)

Concrete fixes in our code, none applied:

1. **`.github/workflows/`: add a Linux job** (Ubuntu, GCC and Clang, `cmake --preset release -DZS_BUILD_WINDOW=OFF`, `ctest --preset release`), and when convenient an `ubuntu-24.04-arm` job. This makes [spec 06](../tier0/specs/06-save-replay.md)'s "golden hashes checked in CI per compiler" true and catches contraction on ARM64, where FMA is in the base ISA (DET-39).
2. **`CMakeLists.txt:15-16`: clang-cl takes the MSVC branch** (`if(MSVC)` is true for it) and gets only `/fp:precise`, which leaves contraction on (DET-16); with `/arch:AVX2` the goldens would break (DET-4, DET-7). Fix: inside that branch test the compiler, not the platform: `if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang") target_compile_options(zs_strict_fp INTERFACE /clang:-ffp-contract=off)`, as Box2D does. On MSVC itself keep `/fp:precise` and require VS 2022 or newer (DET-10).
3. **`engine/sim/src/detmath.cpp:21-23, 121-123, 134-136`: unguarded float-to-int conversion** in `round_to_int` (DET-8). Fix: in `sin` and `cos` return NaN for non-finite `x`, and either reject `|x| > 2^30` (assert, the simulation never needs it) or reduce with a Payne-Hanek step; in `round_to_int` assert `|t| < 2^62`. Add `sin(inf)` and a 1e19 case to `test_detmath.cpp`.
4. **`engine/sim/src/world.cpp:35` (`World::step`): no floating-point environment guard** (DET-40). Fix: at the top of `step()` and `jump()`, on x86 read MXCSR and on ARM FPCR; if rounding is not to-nearest or FTZ/DAZ is set, restore the default and count it in a debug counter (or throw in debug builds). A small RAII class like Jolt's `FPControlWord` does it.
5. **`engine/sim/src/archive.cpp:10` (`ArchiveWriter::f64`): NaN stored as found** (DET-37). x86 produces a negative default NaN and ARM a positive one, so a NaN in state would hash differently across CPUs. Fix: write every NaN as `0x7ff8000000000000`, or treat a NaN in saved state as a bug and assert.
6. **`engine/sim/CMakeLists.txt:50` and `engine/gen/CMakeLists.txt`: one number parser.** Define `TOML_FLOAT_CHARCONV=1` for GCC 12+ (libstdc++ has floating `from_chars`), so all compilers parse calibration numbers through `from_chars` as MSVC does (DET-38). Both paths are correctly rounded today; one path is one less thing to trust.
7. **[spec 01](../tier0/specs/01-quantity.md) rule 4** should name clang-cl and ARM64, say "MSVC 2022 or newer", and move the float path from "a goal" to "required, measured on GCC 13 and Clang 18" (DET-1, DET-2). Its test list should add "the goldens fail with contraction on" as a negative check (DET-7), so the flags cannot quietly disappear.
8. **[spec 02](../tier0/specs/02-draw.md)** is right to forbid `std::*_distribution` (DET-19, DET-20); add that a `std::` engine is allowed only in tests, and that `std::shuffle` and `std::sample` are forbidden for the same reason.
9. **[spec 05](../tier0/specs/05-clock.md) rule 7** should also ban `std::sort` without a total order and `std::hash` in anything saved or hashed (DET-21, DET-22); the code already complies (DET-35, DET-36).
10. **[engine architecture](../design/engine-architecture.md) §5** should add: each worker thread sets the same FP environment at start (DET-25); Jolt is built with `CROSS_PLATFORM_DETERMINISTIC` and its thread-ordered outputs are sorted (DET-24, DET-26); reductions keep the fixed order of [0041](../decisions/0041-threading-and-partition.md), with the 1-, 2- and 8-worker golden test.
11. **Nothing else to adopt.** No fixed point (DET-29); no external correctly rounded libm needed while detmath covers our functions (DET-30, DET-31).

## Open questions

- [spec 01](../tier0/specs/01-quantity.md): does the float path become "required across compilers" now that GCC 13 and Clang 18 reproduce every MSVC golden (DET-1, DET-2)?
- [spec 01](../tier0/specs/01-quantity.md): which platforms are in scope: x64 only, or also ARM64 (Windows on ARM, Apple Silicon) where FMA is in the base ISA?
- [spec 06](../tier0/specs/06-save-replay.md): should a NaN in saved state be canonicalised or treated as a bug?
- [engine architecture](../design/engine-architecture.md): where does the FP-environment guard live once worker threads exist: per step on the main thread, per job, or per worker at start?

## Sources

Primary, read from git:

- Jolt Physics, github.com/jrouwe/JoltPhysics @ 8c605f1 (2026-10-08): `Docs/Architecture.md` "Deterministic Simulation"; `Jolt/Jolt.cmake`; `Build/CMakeLists.txt`; `Jolt/Math/Math.h`; `Jolt/Core/FPFlushDenormals.h`.
- Box2D v3, github.com/erincatto/box2d @ ac7c751 (2026-10-04): `docs/simulation.md` "Determinism"; `docs/recording.md` "Determinism contract"; `docs/questions.md`; `CMakeLists.txt`; `shared/determinism.h`; `test/test_determinism.c`.
- libfixmath, github.com/PetteriAimonen/libfixmath @ ed9391c: `README.md`.
- C++ working draft, github.com/cplusplus/draft @ 9438b11: `source/numerics.tex` ([rand.dist.general], [rand.predef], [rand.util.canonical]); `source/algorithms.tex` ([sort], [stable.sort]); `source/lib-intro.tex` (Cpp17Hash); `source/expressions.tex` ([expr.pre], [conv.fpint], [expr.shift]); `source/basic.tex` (underlying type of `char`).
- MSVC `/fp`, github.com/MicrosoftDocs/cpp-docs @ 281a0a0: `docs/build/reference/fp-specify-floating-point-behavior.md`.
- Clang, github.com/llvm/llvm-project @ 4a428b1: `clang/docs/UsersManual.md`; `clang/include/clang/Options/Options.td`; `libcxx/include/__random/normal_distribution.h`, `uniform_int_distribution.h`.
- GCC, github.com/gcc-mirror/gcc @ 9b2bbe9: `gcc/doc/invoke.texi`; `libstdc++-v3/include/bits/random.tcc`, `uniform_int_dist.h`, `functional_hash.h`; `libstdc++-v3/libsupc++/hash_bytes.cc`.
- MSVC STL, github.com/microsoft/STL @ da52dc0: `stl/inc/random`, `stl/inc/algorithm`, `stl/inc/type_traits`.
- toml++ 3.4.0 as vendored: `third_party/tomlplusplus/toml.hpp`.
- Our code @ 8295162: `CMakeLists.txt`, `engine/sim/src/detmath.cpp`, `draw.cpp`, `philox.cpp`, `archive.cpp`, `world.cpp`, `engine/gen/include/zs/gen/keyed.h`, `apps/zs/window.cpp`, `.github/workflows/windows.yml`.

Measurements (Claude, 2026-10-08, Linux x86-64, GCC 13.3.0, Clang 18.1.3): release builds and `ctest` as in DET-1, DET-2, DET-7, DET-9; detmath harnesses `fmaprobe.cpp`, `bigsin.cpp`, `midsin.cpp` (kept in the session scratchpad, not in the repo).

Reported (search summaries):

- Microsoft CRT floating-point support (accuracy "within ±1 ulp"), learn.microsoft.com.
- CORE-MATH functions in glibc, patchwork.sourceware.org/project/glibc; P. Zimmermann et al., accuracy of mathematical functions, homepages.loria.fr/PZimmermann/papers/.
- MSVC constexpr `<cmath>` with LLVM libc, devblogs.microsoft.com/cppblog/msvc-c23-constexpr-cmath-with-llvm-libc/.
- Factorio forum: 1.1.95 x86/ARM float conversion checks, forums.factorio.com/viewtopic.php?p=595966; FMA fallbacks, forums.factorio.com/viewtopic.php?p=703180.
