# 27 · Detail levels and chunks

**Status:** Proposal. **Stage:** rules apply from stage 8; first chunk split at the start of Tier 1. **Needs:** [03](03-stock-transfer.md), [04](04-hazard-event.md), [06](06-save-replay.md), [22](22-census.md). Research: [detail-levels.md](../../research/detail-levels.md).

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
2. Every system writes each rule once, as a rate over time, and provides its jump form: `advance(state, from_tick, to_tick)` that applies the same rate in one step. Rules that cannot jump (a fight, a push on a door) say so and only run hot.
3. The hot set is the chunks within sense range of a player-controlled body, plus one ring. Their neighbours are at least warm, so fields cross the border.
4. Waking a chunk: load it, `advance` it from `caught_up_tick` to now, then promote its contents. Sleeping a chunk: demote, save, record the tick. Promotion is spread over a few ticks when large.
5. Promotion and demotion are function pairs per unit (contract rule 12). They move owners in the ledger, never create or destroy people, mass or fuel.
6. Draws stay keyed by entity and purpose, so waking early or late gives the same result for the same elapsed time.
7. Views only read; changing layer or camera never changes level. Detail follows bodies the player controls, not the camera.

## Invariants

- Ledger totals are the same before and after any promotion, demotion or jump.
- A save made with a chunk cold and the same save with it hot replay to the same hash once both are hot again.

## Tests

- Jump agreement: a fridge without power, fuel burn, spoilage, healing and arrivals over 18 h, one jump vs 50 seeded hot runs. (A2)
- Wake/sleep round trip keeps ledger totals and item identities. (A1)
- A running generator in a hot chunk is still heard in the warm chunk next door. (A3)

## Calibration

None: the jump forms use the same constants as the hot rules.

## Not in Tier 0

Tier 0's block is one hot chunk plus the cold census (off-map walkers already promote on arrival and demote when destroyed, [22](22-census.md)). Splitting the map into chunks, region-level cold models and streaming from disk start in Tier 1.
