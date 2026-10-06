# 28 · Pop identity and levels

**Status:** Proposal. **Stage:** not Tier 0. The benchmark (below) can run headless on the Tier 0 core, so it is the first step of the post-Tier 0 work. **Needs:** [02](02-draw.md), [03](03-stock-transfer.md), [06](06-save-replay.md), [22](22-census.md), [27](27-detail-levels.md). Design: [pop continuity](../../design/pop-continuity.md). Research: [sim scale in other games](../../research/sim-scale-in-other-games.md). Decided intent: [0025](../../decisions/0025-leader-records-foxhole-tech-readout.md), [0026](../../decisions/0026-pop-continuity-and-capability-loss.md).

## Purpose

A pop is the same person at every layer and every zoom, from a row in a Macro count to a body in Individual. This spec fixes the identity, the three levels (lazy, record, embodied), how a pop moves between them, and how aggregate counts and individual pops stay in agreement. [27](27-detail-levels.md) does the same for **places**; this one is for **people**. Without it, leaders, followed pops, births and the Macro layer cannot be built.

## The core technique (Proposal)

Two tools make "lazy but exact" possible, both already in our stack (counter-based draws, [02](02-draw.md)):

1. **Random-access life.** Every draw about a pop is keyed by `(pop_id, purpose, time bucket)`, never by order. So any pop's state at any time can be computed on demand, without stepping the million others. A lifetime is one uniform draw `u(pop_id, "death")` turned into a death time by the inverse of the cumulative hazard for that pop (age, sex, health record); likewise age at marriage, first child, retirement.
2. **Rank by permutation.** When an aggregate event hits k of n pops of a cohort (a shop lays off 40 of 120, a district loses 3% of its jobs), the count k is drawn at aggregate level, and **which** pops is "the ones whose rank is below k", where rank is a keyed pseudo-random permutation of the cohort (a small Feistel cipher over the cohort index, no storage). Count and membership then agree exactly: Macro reads k, a promoted pop checks `rank(pop) < k`. A rule that has a real ranking (seniority, skill, an edict's order) uses that ranking instead of the permutation.

## State

| Field | Notes |
| --- | --- |
| PopId | 64 bits, never reused. Census people ("founders"): the slot id. Born pops: an **invertible** id, so a lazy birth can always be found again: top bit, 3 bits of generation (1..7), the path of child numbers from the founder (4 bits each), then the founder's slot. Limits: 7 generations below a founder and 15 children per mother. (A hash of household and birth index was the first idea; it cannot be walked back to the mother, so a lazy born pop could not be found.) Templates and imports: a new id at world creation ([sandbox parameters](../../design/sandbox-parameters.md) §7.3) |
| Cohort | A fixed set defined by a generator rule and a snapshot time (the workers of one business on a date; the pops of a district born in one year). Members are indexes `0..n-1` of the generation order, so rank and membership need no list |
| Level | lazy, record or embodied; per pop, never per camera |
| Lazy state | Nothing stored. `derive(pop_id, t)` = base person from the seed (the human generator) + aggregate events applied (cohort ranks) + own life-event draws up to `t` |
| Record | Compact stored state: id, birth and death time, household, job and office, ties (sparse), beliefs, wounds, items, plans, next-event time, plus the **exceptions list** (see Rules 3) |
| Embodied | The record plus the full body, perception and executor ([11](11-body-composition.md)–[14](14-action-executor.md)) |
| Next-event time | The time of the pop's next scheduled life event; records sit in a queue keyed by it, so a long step touches only the pops that have something happening |

Record size is an **estimate**: a few hundred bytes to 1 KB. Not found in any other game's published numbers; it is to be measured (Benchmark).

## Rules

1. **One identity.** A pop's id, history, office and ties never change with level. Level changes how much is stored and simulated, not what is true.
2. **Promotion triggers** (the only ways up from lazy; looking at a pop is a pure read and never one):

   | To | When |
   | --- | --- |
   | Record | holds an office or is a candidate for one; is followed, met or possessed by the player; is hired, bitten, treated, taught or otherwise touched by an interaction with a record or embodied pop; is a researcher or a leader of a group the player can see; a scenario or story ties them |
   | Embodied | within sense range of a player-controlled body, plus one ring ([27](27-detail-levels.md)) |

3. **Demotion.** Embodied → record when the chunk cools. Record → lazy only if the pop holds no office, is not followed, and their state equals `derive` **or** the difference is saved as a short exceptions list (what they did that the seed could not say). Nothing the pop did is lost. An officeholder or followed pop is never lazy (Decided, [0025](../../decisions/0025-leader-records-foxhole-tech-readout.md)).
4. **Counts are sums, never stored apart.** A region's counts by age, job, wealth, health and loyalty are the sum over its cohorts: lazy pops via cohort counts, records via their own state, minus those promoted. Macro never keeps a second number.
5. **Aggregate events pick members by rank** (technique 2). The count is drawn once and recorded with the event; any pop asked later answers from its rank. Events are logged with a reason id ([macro layer](../../design/macro-layer.md) §5).
6. **Jumps.** Life events (age, births, marriage, illness, job moves, death) have a jump form ([27](27-detail-levels.md) rule 2): `advance(pop, from, to)` applies the same rates in one step. A rule that cannot jump (a fight, a push on a door) only runs embodied.
7. **Births and deaths.** A birth is a conserved event creating a new PopId from the parents' records or cohort slots; its body comes from the generator. A death removes the pop from every group, office and job and writes one chronicle record (cause, time) ([25](25-chronicle.md)). The dead are not regenerated.
8. **Ledger.** Counts of people by owner (region, household, office, alive, dead, away) move by transfers ([03](03-stock-transfer.md)). Promotion and demotion are function pairs that move owners and never create or destroy people (contract rule 5 of [27](27-detail-levels.md)).
9. **Update order.** Records update event-driven (by next-event time) and on interaction; embodied pops tick; lazy pops cost nothing until read. Same fixed order as the rest ([05](05-clock.md)).
10. **Time steps.** Long steps (day to year per second, Macro and City) run only jump forms. Embodied pops require a speed at which their rules are valid; the clock refuses long steps while any embodied pop is in a fight.
11. **Cohort changes.** A cohort is a snapshot. If members join or leave (a business hires), a new cohort version starts; old events keep their old cohort. Pops moving between cohorts carry their earlier outcomes in their record or in `derive`. (Risk: see Open.)

## Invariants

- Totals of people, jobs, money and stock are identical before and after any promotion, demotion or jump.
- `derive(pop_id, t)` is a pure function: same seed, inputs and time give the same pop, in any order of evaluation.
- For any aggregate event, the number of pops whose rank is below k is exactly k.
- A pop's id, birth, ties and office are unchanged by any level change.
- An officeholder or followed pop is never lazy.

## Tests

- **Derive purity:** evaluate 10,000 pops at 20 times in random order and in time order; results identical. (A1)
- **Rank agreement:** for 200 events over cohorts of size 10 to 100,000, count pops with `rank < k` by enumeration; equals k, no duplicates, all in the cohort. (A1)
- **Jump agreement:** one 50-year jump vs 100 stepped runs for a cohort of 1,000 (deaths, births, marriages, job moves); the stepped distribution contains the jump result at 90%. Deterministic parts equal. (A2, A1)
- **Round trip:** lazy → record → embodied → record → lazy for a pop with an exception; ledger totals equal, the exception survives, the id and history are unchanged. (A1)
- **Aggregate vs enumeration:** a Macro count of "unemployed in region R" equals the number of promoted pops that report being unemployed, for any subset promoted. (A1)
- **Save/load:** a save with a region lazy and the same save with it embodied replay to the same hash once both are embodied. (A1)
- **Office rule:** kill the holder; succession picks from real candidates; the office's work stalls when there are none. (A3)

## Benchmark (the first thing to build)

A headless command, `zs_headless --pop-bench`, using the Tier 0 core and the generator, with fixed seeds. Numbers below are **targets to be confirmed by the first run**, not facts; the first run only measures and the targets are then set from it.

| Case | Measure |
| --- | --- |
| B1 records at 10k, 100k, 1M | bytes per record, total memory, time to build, time per game year of event-driven updates |
| B2 lazy census 1M, 10M | time of one `derive`, time of an aggregate count, memory (should be ~0 plus cohort tables) |
| B3 promotion and demotion | latency lazy→record, record→embodied, and back; per-frame spread when promoting 100 at once |
| B4 long steps | game years per real second at 1M lazy pops with 10k records, and with 100k |
| B5 embodied | frame cost at 50, 150, 300 embodied pops (extends the existing `--bench`: 4 pops + 60 walkers for a day takes about 0.2 s) |
| B6 generations | 1M-person region, no outbreak, 60 game years: wall time, record count growth, and the realism report ([sandbox parameters](../../design/sandbox-parameters.md) §13.1) |

Starting budget to test (estimates from the research note): 50–300 embodied, 10,000–100,000 records, lazy unbounded.

### First measurement (2026-10-06, `zs_headless --pop-bench`, release build, one thread, a cloud container; GCC)

Measured, not estimated. The records are **thin** (only office, followed flag and exceptions), so the memory figure is a floor: real records with ties, beliefs, wounds and items will be several times larger.

| Case | Result |
| --- | --- |
| B1 records, 1,000,000 | built in 0.45 s (0.45 µs each); 72 bytes per record of process memory (std::map nodes included); 17 bytes per record saved |
| B2 lazy snapshot, 1M to 10M pops | 1.6–2.2 µs per pop, flat with census size; with 10,000 aggregate effects in the log 2.1 µs |
| B2 region count (1M pops, 10,000 effects) | 6.8 µs; 10M pops and 100 effects: 4.4 µs |
| B3 promotion | lazy → record 0.7 µs; record → lazy 0.4 µs |
| B4, B6 | not yet (need P5: life-event jumps) |
| B5 | existing `--bench`: 4 pops + 60 walkers for a game day in about 0.2 s |

What it shows:

- **Records are not the limit at this size.** A million thin records is 72 MB and 0.45 s. The limit will be what a record holds and how often it is updated, which P5 and the systems that add fields decide. The research estimate of 10,000–100,000 records is conservative for thin records.
- **Lazy is genuinely cheap:** about 2 µs a pop to read, flat as the census grows tenfold.
- **The first version replayed the whole effect log on every query** (27 µs a snapshot and 377 µs a region count at 10,000 effects). An index per (cohort, kind) and per region fixed it (2.1 µs and 6.8 µs). This is the kind of cost the benchmark exists to find.
- **Not measured yet:** long steps, life-event updates, realistic record size, several threads, other machines, MSVC. The numbers are one container's; treat them as orders of magnitude.

## P5 result: the jump (2026-10-06)

A woman's births are a pure function of her id, like her death: a childless draw, a first birth at 18 plus an exponential delay, then gaps of 3 years plus an exponential, none from 45, at most 15. A pop's whole descendant tree is therefore known at any time without stepping anyone. `jump(founders, from, to)` walks the trees and counts births, deaths and who is alive.

- **Agreement with stepping (A2):** against an independent week-by-week simulation of the same rates, 30 runs each, 400 founders over 40 years: births 143.4 vs 143.2, deaths 215.4 vs 215.5, alive at the end 328.0 vs 327.6; all far inside 4 standard errors. Splitting a jump in two windows adds up exactly; people are conserved (alive at end = alive at start + births - deaths).
- **Speed (release build, one thread, GCC):** 1,000,000 founders over 60 game years in 0.48 s (about 0.3 µs per person ever in the trees, 1.5 million people visited); 100,000 founders over 150 years in 0.06 s. A long step costs the people visited, not the length of time.
- **Not realistic yet, and this is the report's job:** the benchmark's founders are aged 18–64 with no children or elderly, so the first decades show a shrinking population (x0.60 over 60 years; 8 births and 15 deaths per 1,000 per year) that is an artefact of that age structure plus placeholder fertility and mortality, not a finding about people. The census generator will supply real age pyramids; the placeholders get fitted to real tables (Calibration).
- **Limits found:** the 7-generation id format is reached after about 150 years (generation 7 had 104 living pops at 150 years, and they cannot have children), so runs longer than that need a wider id before they are meaningful. Marriage and fathers do not exist yet: a child has only a mother. Effects (laid off, moved, ill) reach founders only; a born pop has no cohort until births create cohort versions (rule 11, still the biggest open risk).
- **Cost to watch:** `project_demography` re-walks every tree from the start of the run for each call; caching a tree's state at a year, per founder, is the next step before using it every frame.

## Calibration

| Value | Source |
| --- | --- |
| Lifetimes, fertility, marriage, labour | Real demography and labour tables for the world's nation ([sandbox parameters](../../design/sandbox-parameters.md) §7, §13.1) |
| Record size, update rate, budgets | None: measured by the benchmark |

## Not in this spec

Offices and their work, the economy, the UI of following and possessing, war ([sandbox parameters](../../design/sandbox-parameters.md) §5). They read this layer.

## Open

1. **Cohort versioning** (rule 11): businesses and households change over time, so cohorts snapshot; check the number of live cohort versions stays small. First suspect for complexity.
2. **Interactions between a lazy and a record pop** (a record pop's ties to people they've never met): ties are created only by promoting the other pop (rule 2); confirm it does not cascade.
3. **Per-region series** (what the lazy side reads from the region) are defined by the economy and outbreak specs, not here.
4. **Estimate checks:** record size, budgets and long-step throughput come from the benchmark.

## Implementation order

P1 PopId, `derive`, the permutation, with the purity and rank tests (**Implemented 2026-10-06**: `engine/sim/include/zs/pop.h`, `src/pop.cpp`, `tests/test_pop.cpp`; mortality is a placeholder, not yet calibrated; not yet Validated) → P2 cohort events and count agreement (**Implemented 2026-10-06**) → P3 record store, promotion and demotion, round-trip and save tests (**Implemented 2026-10-06**: `engine/sim/include/zs/pop_registry.h`, `src/pop_registry.cpp`, `tests/test_pop_registry.cpp`; a standalone registry, not yet part of the World, the ledger or the state hash; records hold only office, followed and exceptions so far; embodied level, cohort versions and non-census ids not yet; not yet Validated) → P4 `--pop-bench` and its first measurement → P5 jump forms for life events (**Implemented 2026-10-06** for births and deaths: `engine/sim/include/zs/demography.h`, `src/demography.cpp`, `tests/test_demography.cpp`; marriage and job moves wait for households and businesses) → P6 realism report.
