# 0052 · One executable, one simulation, nothing left waiting

- **Date:** 2026-10-08
- **Status:** Decided by the user. Extends [0047](0047-one-line-of-work.md) (one line of work) from branches to programs and worlds. Answers the three items `docs/status.md` listed as waiting on the user.

## Decisions

1. **One executable.** `zs.exe` is the only program. It opens the window by default; the same file runs headless with command-line flags (batch runs, benchmarks, generator output). `zs_headless` and `zs_gen` are folded into it and removed. The simulation stays a library without a window (`engine/sim`, `engine/gen`); the executable is a thin shell over it.
2. **One simulation.** One world assembly builds everything that exists: the world grid and its fields, the country and its regions, the pop registry, the economy, networks, outbreak, orders and time, with the Tier 0 block as the detailed site inside it. Every built system is added to that one world in one fixed order. The stage scenarios (stages 1 to 9, the body lab) stay only as test fixtures.
3. **One population.** The Tier 0 block's neighbours are the registry's founders: the same pops at every layer. Tier 0's golden hashes are regenerated once for it.
4. **Data.** Global tables first (age structure, life tables, fertility, household size, work by sector, by development level), then a few fictional base countries built from them, validated against published aggregates. Realistic maths always ([0046](0046-realistic-numbers-from-free-data.md)). A real country is later just another preset.
5. **Local on Windows.** Work is done on the user's PC with MSVC. Golden hashes are pinned on MSVC; the CI build stays as an extra check, not the reference.
6. **Nothing waits on the user.** When a choice comes up, Claude takes a sensible default, records it in a decision marked "default taken by Claude", and keeps going. The user can overturn any of them later. `docs/status.md` has no "waiting on the user" list.

## Notes

- The stage 5 sandbox was checked on screen by the user before 2026-10-08: it works; WASD movement is poor and it is hard to tell what is going on. Both are fixed in the one executable.
- Forced-labour populations stay out ([0037](0037-sectors-zones-and-world-creation-answers.md)); that is a decision, not a pending item.

## Order of work

1. One executable (item 1).
2. One world and one population (items 2 and 3), spec first.
3. Global tables and base countries (item 4), spec first.
4. Then the layer-jump rows left (travel, ownership) and the generator, each added to the one world as it is built.
