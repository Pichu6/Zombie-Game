# 38 · Outbreak across levels

**Status:** Implemented and tested, headless (the encounter rule, the measured rates, the compartments and the entity-level run; not yet wired to the registry's records or to the Tier 0 walkers). **Stage:** not Tier 0; the [layer jumps](../../design/layer-jumps.md) audit row 14. **Needs:** [02](02-draw.md), [21](21-infection.md), [36](36-fields-and-weather.md) (keyed draws), the jump harness (`engine/sim/tests/jumps.h`). Design: [world simulation](../../design/world-simulation.md) §5 (SEIRZ per region, calibration as a requirement), [threats](../../design/threats.md). Decisions: [0014](../../decisions/0014-open-questions-answered.md) (hidden bite, 1–2 days to turn), [0039](../../decisions/0039-fill-the-layer-jumps.md).

## Purpose

The outbreak is compartments per region at Macro, infected people in rosters at City, a bite and a turned body at Individual. The design's requirement is that the aggregate's rates are **built from what the simulation already has**, never set apart: how fast the zombie is, how much people know, whether they fight back, whether a bite infects. This spec gives the one encounter rule, measures it into rates, steps a region's compartments with those rates, runs the same town entity by entity, and tests that the two agree in distribution, so the preview a player sees at Macro and the fight at Individual are the same outbreak.

## What the code forces

- **Keyed draws only** ([02](02-draw.md)): an encounter is a pure function of (seed, zombie, draw), so an entity-level run can be replayed and compared seed by seed.
- **The harness compares distributions** (layer jumps §5 step 2): an aggregate step cannot equal an entity run tick for tick; the agreement level is A2, tested with Anderson–Darling, mean agreement and the share of runs that took off.
- **Tau-leap, not exact jumps.** The aggregate steps by day with binomial draws; a coarser step would be a different approximation, so no partition-independence is claimed here (unlike spoilage or growth). A day is the step; a longer jump is a loop of days.

## State

| Field | Notes |
| --- | --- |
| `Micro` | the entity-level rules: zombie and human speed, `warning` (0–1, how much people know), bite chance once caught, kill skill of warned people, conversion per bite ([21](21-infection.md) rule 1), mean days to turn ([0014](../../decisions/0014-open-questions-answered.md): 1–2), contacts per zombie per day |
| `Rates` | measured: bite per encounter, destroyed per encounter, conversion, contacts per day; `r0()` derived |
| `Compartments` | per region: `s` susceptible, `e` exposed (bitten, not turned), `z` zombies, `r` destroyed zombies |
| `Entities` | the same town one entity each: a state and, for the exposed, nothing more (turning is a daily chance, as in the aggregate) |

## Rules

1. **The encounter.** A zombie meets a person. Caught with chance `min(1, zombie speed ÷ human speed) × (1 − 0.7 × warning)`; bitten, if caught, with `bite_base`; the zombie destroyed with `warning × kill_skill`. Three keyed draws; four outcomes. This is the only place the physics of the meeting is written.
2. **Measurement.** `measure(micro, seed, n)` runs `n` keyed encounters and returns the bite and destruction shares, with the conversion and contact rate carried over. These are the aggregate's only inputs. They match the closed-form chances within 2 % at 200,000 encounters (A2).
3. **The reproduction number.** `r0 = bite × convert × contacts`, where `contacts` is the encounters a zombie gets before destruction, `1 ÷ destroyed`, capped at a 60-day horizon when nobody destroys it. It is computed from the measured rates, never set: a slow shambler among warned people has `r0` well under 0.5 (the design's "each infects about 0.3 when people fight back"); a runner among the unwarned has `r0` above 5 and converts the town.
4. **The aggregate day.** The exposed turn with chance `1 ÷ turn_days` (keyed binomial). Each zombie has `contacts_per_day` encounters with the living (a keyed binomial over 100 slots per zombie); a share `s ÷ (s + e)` of them meet a susceptible; of those, `bite × convert` become exposed; of all encounters, `destroyed` end the zombie. Counts never go negative and never exceed what is there.
5. **The entity day.** Every exposed entity turns by the same daily chance; every zombie draws its contacts, each with a person drawn from the living and an outcome from the encounter rule; a susceptible bitten and infected becomes exposed; a zombie destroyed is removed at the end of its day (so its day's encounters happen, as in the tau-leap). The compartments of an entity run are a sum over the entities, never stored (A1).
6. **Agreement (A2).** For the same town, rates and horizon, the final infected-ever count over many seeds has the same distribution in both runs (Anderson–Darling at 5 %), the same mean within four standard errors, and the same share of runs that took off within three. Where the aggregate overcounts (two bites on one person in one day, a zombie destroyed early in its day) the bias is below the test's power at town scale; at city scale it is a known approximation of tau-leaping, to be tightened with shorter steps if a test shows it.
7. **Keys.** Field numbers 201–207, append only; the day goes into `k`; entity ids are `region × 1000003 + index`.

## Invariants

- `s + e + z + r` is constant in both runs; no compartment is negative.
- Entity counts equal the sum over entities every day.
- The same seed gives the same run; the measured rates are deterministic for a seed.

## Tests (`test_outbreak.cpp`)

- **Encounter and rates (A1, A2):** keyed and seed-sensitive; measured bite and destruction within 2 % of the closed forms; faster bites more, warned are bitten less and destroy more; the unwarned never destroy.
- **Keyed binomial (A2):** pure; edges (n = 0, p = 0, p = 1); mean and variance of 4,000 draws at (50, 0.2) within 3 % and 12 %; the large-n fallback in range.
- **Enumeration (A1):** 20 days of an entity run conserve 303 and the counts equal the sum by hand; the aggregate conserves too.
- **Agreement (A2):** a town of 400 with 3 zombies over 30 days under a middling preset, 300 aggregate seeds against 60 entity seeds: Anderson–Darling does not reject, means and took-off shares agree; the same for a sure-fizzle preset.
- **The design's claims (A3):** `r0` under 0.5 for the warned shambler and over 5 for the unwarned runner; over 40 days the shambler takes off in under 15 % of 200 runs and the runner in over 85 %, converting more than 350 of 400 on average.

## As built

- `engine/sim/include/zs/outbreak.h`, `src/outbreak.cpp`, namespace `zs::outbreak`: `Micro`, `encounter`, `catch_chance`, `bite_chance`, `kill_chance`, `Rates` (`r0`), `measure`, `Compartments`, `keyed_binomial` (inversion with a normal fallback for large n p), `aggregate_day`, `aggregate_days`, `Entities` (`run_day`, `counts`).

## Calibration

| Value | Source |
| --- | --- |
| Speeds | [0010](../../decisions/0010-concept-v2.md): body-driven, mostly slow walkers; a runner about 4 m/s is the preset's upper case |
| Conversion per bite 0.9, turn in 1–2 days | [21](21-infection.md) rule 1, [0014](../../decisions/0014-open-questions-answered.md); the GDD's bite 100 % is the upper bound |
| Warning, kill skill, contacts per day | Placeholders; to be measured from the Tier 0 runs (bites per walker per day from `--group` and `--run` batches) |

## Not in this spec

Wiring the record level ("infection of a record changes the counts": an `EffectKind` on rosters, so a city's exposed are listed people); measuring the encounter rule from the Tier 0 walker fights instead of a closed form (the real purpose of rule 2: replace the formula by the sim's own numbers); the hidden-bite social rules (people hiding bites, misdiagnosis, [threats](../../design/threats.md) §Spread); airborne presets; regions coupled by travel (row 18); hospitals and morgues as hotspots.

## Open

1. **Step length.** A day is coarse for a fast outbreak in a dense city. Recommendation: step by hour when `z × contacts` per day exceeds a tenth of the living, and re-run the agreement test at that scale.
2. **Who is "warned".** Here it is one number per region. The real source is the belief store (row 16): the share of people who know, from reports and sightings. Recommendation: `warning` becomes a lens over beliefs when orders and belief are built.
