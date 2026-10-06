# Tier 0 plan: "The generator problem"

**Status:** Proposal (2026-10-05). Scope is Decided ([0010](../decisions/0010-concept-v2.md), [status](../status.md)); this file only orders the work. Stages are ordered by dependency; no stage has a date or a size estimate.

Every unit has a one-page spec in [specs/](specs/README.md), written before its code. Each stage ends with something you can run and check, so a wrong turn shows up early.

## Engineering defaults (Proposal)

| Topic | Default | Why |
| --- | --- | --- |
| Language and build | C++20, CMake, Ninja | Proposal; builds on Windows (MSVC) and Linux (GCC or Clang) |
| Libraries | SDL3 and Dear ImGui (Decided in [0014](../decisions/0014-open-questions-answered.md)); doctest (unit tests), RapidCheck (property tests), toml++ (data files) | The last three are Proposals: proven, permissive licences; nothing else until a spec needs it |
| Randomness | Own Philox-4x32 and samplers | Contract rule 4 ([02](specs/02-draw.md)) |
| Floating point | Strict: `/fp:precise`, `-ffp-contract=off`, no `-ffast-math`; own `detmath` for exp, log, sin, cos | Same result on every compiler ([01](specs/01-quantity.md)) |
| Data | Text files in `data/tier0/` (items, map, census, calibration numbers with sources) | No numbers buried in code |
| Where it runs | Simulation and tests build and run headless in the cloud session; the window (stage 5 on) runs on the user's PC | The sim core never needs a window |

## Layout

```
engine/sim/        simulation core library (no SDL, no ImGui)
engine/sim/tests/  unit, property and golden tests
apps/headless/     command-line runner: batch runs, AI-only runs, replays
apps/view2d/       SDL3 + ImGui window
data/tier0/        definitions, map, census table, presets
docs/tier0/        this plan and the specs
```

## Stages

| # | Stage | Specs | Needs | Done when |
| --- | --- | --- | --- | --- |
| 0 ✓ | **Skeleton** | — | — | Empty sim library, headless app and test target build on Linux and Windows; one test passes; "Build and test" section in CLAUDE.md filled in |
| 1 ✓ | **Foundations** | [01](specs/01-quantity.md) [02](specs/02-draw.md) [03](specs/03-stock-transfer.md) [04](specs/04-hazard-event.md) [05](specs/05-clock.md) [06](specs/06-save-replay.md) | 0 | Property tests green; a scripted dummy run saves, loads and replays to the same golden hash; ledger check runs every debug tick |
| 2 ✓ | **Things and places** | [07](specs/07-condition.md) [08](specs/08-assembly.md) [09](specs/09-site-map.md) [10](specs/10-fortification.md) (data only) | 1 | The Tier 0 map loads from its text file; the headless app prints the room graph; salvage and reassembly conserve mass |
| 3 ✓ | **The body** | [11](specs/11-body-composition.md) [12](specs/12-body-wounds.md) [13](specs/13-capabilities.md) | 1 | A headless "body lab" reproduces the calibration bands (starvation, dehydration, bleeding, wakefulness, load); **body-cost benchmark** shows rest-until-event stays fast with 4 pops and ~60 walkers |
| 4 ✓ | **Doing things** | [14](specs/14-action-executor.md) [15](specs/15-items-recipes.md) | 2, 3 | A scripted pop eats, drinks, dresses a wound, salvages a chair and barricades a window; player-controlled and AI-controlled runs of the same intents hash the same |
| 5 (built, check on PC) | **First window** | [16](specs/16-view-2d.md) | 4 | On the user's PC: the map, one pop under WASD control, inspector, context menu with greyed reasons, event log, time controls |
| 6 ✓ | **The generator problem itself** | [17](specs/17-power-network.md) [18](specs/18-noise-field.md) [19](specs/19-light-field.md) | 4 (5 for overlays) | Grid fails, the generator powers fridge and lamps, fuel drains, food spoils without power; noise, light and power overlays show how far the generator carries and which windows leak |
| 7 ✓ | **The dead** | [20](specs/20-damage.md) [21](specs/21-infection.md) [22](specs/22-census.md) [23](specs/23-walker-brain.md) [10](specs/10-fortification.md) (attacks) [25](specs/25-chronicle.md) (cause chains, death sentences) | 3, 6 | A generator night after grid failure draws named neighbours who push in the weakest door; a bitten pop turns with its own wounds and items; census totals hold; every death prints its cause sentence |
| 8 ✓ | **The group** | [24](specs/24-pop-ai.md) | 7 | Command view with job board and squad orders; the three refusals; **AI-only run**: 4 pops survive or die of explainable causes over 50 seeds |
| 9 ✓ | **The run** | [25](specs/25-chronicle.md) (day summaries, death screen) [26](specs/26-run.md) | 8 | A full 14-day run is playable start to end, with found survivors, fuel crunch, waves, radio goal, death sentences and telemetry |
| 10 | **Kill or keep** | — | 9 | At least 5 full runs (some by other players), on seeds both with and without the stranger's hidden bite, answer the seven fun questions; results written in `docs/tier0/playtest.md`; the design changes before any renderer work if an answer is no |

✓ = Implemented (tests green on GCC and Clang; Windows check pending).

Stage 3 can run alongside stage 2 (both need only stage 1). Stage 5 can start as soon as stage 4 lands and then grows with every later stage.

**Stretch, after stage 9 if the core works:** water (rooftop tank, basement leak by room level), a shotgun as a loud comfort item, carbon monoxide from an indoor generator.

## Working rules for this plan

- Every engine task names the stage it serves; if none needs it, it waits (CLAUDE.md).
- A spec changes before its code does; code that disagrees with its spec is a bug in one of them, and the fix says which.
- Each stage's "done when" is shown with evidence (test output, a run log, a screenshot), and labelled Implemented or Validated.
