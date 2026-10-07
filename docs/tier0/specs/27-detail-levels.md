# 27 · Detail levels and chunks

**Status:** Proposal. **Stage:** rules apply from stage 8; first chunk split at the start of Tier 1. **Needs:** [03](03-stock-transfer.md), [04](04-hazard-event.md), [06](06-save-replay.md), [22](22-census.md). Research: [detail-levels.md](../../research/detail-levels.md); rules 8–12 and the last two tests come from [pop-simulation-and-persistence.md](../../research/pop-simulation-and-persistence.md) (2026-10-07).

## Purpose

One simulation, run at the cheapest detail that still gives the right answer near the player, stored in chunks that wake only when needed. The gameplay layers (Individual, City, Macro) are views over it, never separate simulations ([world-simulation §2](../../design/world-simulation.md#2-the-simulation-contract-proposal)).

## State

| Field | Notes |
| --- | --- |
| chunk | a building with its lot, or a street segment; owns its ledger owners, items and edges |
| level | cold, warm or hot, per chunk; people and things take their chunk's level |
| caught_up_tick | the tick the chunk's state is true for |

## Rules

1. **Cold:** stocks and counts only (census rows, kg, litres); updated in one jump when read. **Warm:** room graph, item lists, people as needs and schedules; hourly or on events. **Hot:** cells, fields, bodies, executor; every tick.
2. Every system writes each rule once, as a rate over time, and provides its jump form: `advance(state, from_tick, to_tick)` that applies the same rate in one step. Rules that cannot jump (a fight, a push on a door) say so and only run hot. (Refined 2026-10-07, [catch-up-jump-forms.md](../../research/catch-up-jump-forms.md).) The jump of a chunk whose rules are coupled is a next-reaction event loop inside the interval ([04](04-hazard-event.md) rule 9); rules independent inside the interval use closed forms (Poisson counts, Newton cooling, binomial consumption). Hot uses exact per-tick factors (1 − e^(−hΔt), e^(−kΔt)) so the two paths agree.
3. The hot set is the chunks within sense range of a player-controlled body, plus one ring. Their neighbours are at least warm, so fields cross the border.
4. Waking a chunk: load it, `advance` it from `caught_up_tick` to now, then promote its contents. Sleeping a chunk: demote, save, record the tick. Promotion is spread over a few ticks when large.
5. Promotion and demotion are function pairs per unit (contract rule 12). They move owners in the ledger, never create or destroy people, mass or fuel.
6. Draws stay keyed by entity and purpose, so waking early or late gives the same result for the same elapsed time.
7. Views only read; changing layer or camera never changes level. Detail follows bodies the player controls, not the camera.

8. **Projection is written field by field** (added 2026-10-07, from Dwarf Fortress, whose record has no bleeding so leaving a site heals you): demoting a body to a record keeps every primary stock, wound clock and infection clock as integers, and lists explicitly what it drops (readouts only). What the record cannot hold but promotion needs (position in the room, a mid-action state) goes in a versioned **memorisation blob** on the record, or the rule says it is dropped.
9. **Records have kept-reason flags and a merge path.** A record is kept while any flag is set (player tie, story tie, witnessed by a pop, possessed, never cull). With no flags and a body summary equal to what its census row regenerates, it drops back to a row and the cell count absorbs it. Promotion without a merge path grows without bound (Dwarf Fortress history, Victoria 3 before 1.6).
10. **Promotion from a count respects the hot layer.** People or walkers realised from a cell count are placed by the building's seed and checked against walls and doors (Project Zomboid's meta zombies inside sealed houses is the bug to avoid).
11. **Importance is not only distance.** A record stays at least warm while the player watches it from City, while it is tied to a pop, or while it is in an action that cannot jump (a fight, a breach), wherever it is.
12. **Groups are nodes.** A horde, household, squad or convoy is one record with a key, a count, a goal and a next event time until it is within sense range; then it expands one level to member records, and to bodies only in the hot set.

## Invariants

- Ledger totals are the same before and after any promotion, demotion or jump.
- A person exists in exactly one form (row, record or body) and one chunk or group at a time.
- A save made with a chunk cold and the same save with it hot replay to the same hash once both are hot again.

## Tests

- Jump agreement: a fridge without power, fuel burn, spoilage, healing and arrivals over 18 h; 50 seeded hot runs against about 2,000 seeded cold jumps, compared with Anderson–Darling per quantity and energy distance on the joint vector, plus the deterministic invariants on every sample. (A2; the "90 % band" check is only a smoke test.) Cases whose jump keeps one draw per event (infection clocks, causes of death, the fridge) are checked bit-identical instead. (A1)
- Wake/sleep round trip keeps ledger totals and item identities. (A1)
- A running generator in a hot chunk is still heard in the warm chunk next door. (A3)
- Standing harness (from Tier 1): the same seed and inputs run hot everywhere and with level switching; counts per cell, deaths and their causes, and stocks agree at the level each jump rule claims (A1 for exact rules, A2 for drawn ones). Navarro et al. validate multi-level simulations this way; X4's in-sector versus out-of-sector complaints are what skipping it looks like.
- Demote a body to a record and promote it again: every primary stock, wound and clock is identical; only listed readouts are recomputed. (A1)

## Calibration

None: the jump forms use the same constants as the hot rules.

## Not in Tier 0

Tier 0's block is one hot chunk plus the cold census (off-map walkers already promote on arrival and demote when destroyed, [22](22-census.md)). Splitting the map into chunks, region-level cold models and streaming from disk start in Tier 1.
