# 0064 · Sound and sky defaults: the grid's death is heard ten times farther, walkers moan, Earth's sky

- **Date:** 2026-10-09 (recorded 2026-10-10)
- **Status:** Defaults taken by Claude ([0052](0052-one-executable-one-simulation.md)). Asked whether to change the numbers or the claim "ten times farther when the grid dies", the user answered "whatever is best" (2026-10-09). Moaning and the sky values were taken as defaults in the same research round.
- **Builds on:** [spec 18](../tier0/specs/18-noise-field.md) rule 6 ("the generator, not the shout, brings the horde", Decided as intent), [spec 19](../tier0/specs/19-light-field.md), [0063](0063-real-size-planet-and-the-zoom-continuum.md) (an Earth-size, Earth-like planet), research [hordes-sound-sky-and-testing](../research/hordes-sound-sky-and-testing.md) (HST-138..HST-140, HST-146, HST-148) and [perception-and-run-ahead](../research/perception-and-run-ahead.md) (RA-111).

## Decision

1. **Keep the claim, change the numbers.** "Voices carry about ten times farther when the grid dies" needs a 20 dB drop in background; spec 18's 55/45 → 40/30 gives 15 dB, about 5.6× (HST-138).
   - Grid-dead background becomes **35 dB by day and 27 at night** (was 40 / 30). The drops are then 20 dB by day (10×) and 18 dB at night (about 8×).
   - **Two hearing thresholds** replace the single +3 dB: **notice** at signal minus background ≥ 0 dB, **place** the sound at +10 dB (HST-139).
   - The open-frame **generator is 85 dB at 1 m** (was 89), matching a 68–70 dBA-at-7 m unit (HST-140).
2. **Walkers moan.** Moans make the horde turn as a wave and relay sound from walker to walker (HST-146, RA-111). The level at 1 m is not sourced: start at **about 65 dB** (an estimate) and tune it.
3. **Earth's sky.** Axial tilt 23.44°, year 365.24 days, day 24 h, synodic month 29.53 days, kept as generator parameters so another world can change them (HST-148).

## Consequences

- Spec 18 changes in the same commit as its code, because the numbers re-pin the golden hashes; until then spec 18 states what is implemented. **Rules 1 and 2 built 2026-10-11** (background 35/27, the two thresholds, the generator at 85 dB, moans at 65 dB; the stage 7 golden re-pinned). Rule 3, the sky's parameters, waits for the daylight-by-latitude slice of spec 42.
- The walker relay speed, once moans exist, sets the rollback cone of the run-ahead proposal (engine architecture, Open).
- Sun and moon come from formulas on these parameters (spec 19), shared with the planet work.
