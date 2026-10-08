# 37 · Flora and fauna across levels

**Status:** Implemented and tested, headless (the library, and the wiring in the economy world system). **Stage:** not Tier 0; the [layer jumps](../../design/layer-jumps.md) audit row 13. **Needs:** [03](03-stock-transfer.md), [27](27-detail-levels.md) rule 2, [32](32-stocks-goods-and-money.md), [36](36-fields-and-weather.md) (keyed draws). Design: [world systems](../../design/world-systems.md) §10.2 (stocks, recovery, hunting and fishing). Decision: [0039](../../decisions/0039-fill-the-layer-jumps.md).

## Purpose

A region's forest and game as stocks, growth and harvest at Macro; a cell's woods and herds at City; one tree to cut and one deer to hunt at Individual. This spec makes them one stock on the ledger that grows by a law with an exact jump form, is harvested by transfers, and becomes individuals only when approached, by keyed serials, so that a kill removes exactly one and leaves the others as they were. It is the pattern of the census ([22](22-census.md)): a count per cell, bodies only near the player.

## What the code forces

- **Growth creates mass.** The ledger mints only at setup ([03](03-stock-transfer.md)); living stocks grow. So growth enters as a **declared inflow** with its cause, the same audited door a body's tissue uses when a pop is embodied ([29](29-embodied-pops.md)), and decline above capacity leaves as a declared outflow. The check stays exact.
- **A species is a good** ([32](32-stocks-goods-and-money.md)): bulk is biomass in kg, the unit is one individual with `bulk_per_unit` its typical mass. A harvest of 3 t of wood is a transfer of bulk; a felled tree is a transfer of that tree's mass.
- **Rates jump exactly** ([27](27-detail-levels.md) rule 2): the logistic law has a closed form, so a lot (stock at a start tick) gives the stock at any later tick without stepping.

## State

| Field | Notes |
| --- | --- |
| `Population` | `owner` (the place's account); `species` (the good); `capacity` K (bulk); `rate_per_day` r; the lot (`start_amount`, `start_tick`, `caught_up`); `next_serial` (individuals ever shown); `taken` (serials removed) |
| `Individual` | generated, never stored: `serial`, `mass_share` (0.5–1.5 of the unit mass), `age_share`, position offsets inside the place, sex |

## Rules

1. **Logistic growth.** `grown(B0, K, r, t) = K ÷ (1 + (K ÷ B0 − 1) e^(−r t))` with the deterministic exponential, floored when growing (never past K) and ceiled when declining (never below K); nothing grows from nothing. `advance(to_tick)` moves each lot's `grown(caught_up) → grown(to_tick)` difference in or out of the ledger. Stepping every hour and one jump give identical balances (A1). A harvest or a kill resets the lot to what the place holds now.
2. **Harvest is a transfer.** Wood or meat leaves the place's account for the taker's by the ledger ([32](32-stocks-goods-and-money.md) rule 1): a shipment, a kill, a cut. Nothing else lowers a stock.
3. **Individuals on approach.** The roster of a population is the first `n` serials not in `taken`, where `n = stock ÷ unit mass`; the remainder stays bulk (saplings, not a tree). Each individual is a pure keyed function of (seed, owner, species, serial): the same serial is the same deer whenever asked for, in any order (A1). `approach` records how far the serials have been shown; growth after it appends new serials at the end, so what was seen stays.
4. **Take.** `BiotaTake` (subject = taker, population, serial): the stock is caught up first; if the serial is on the roster, its mass (unit mass × its share) moves to the taker's account and the serial joins `taken`; otherwise refused (`NotOnRoster`). After a take the roster is the old one minus that individual (A1).
5. **Macro readouts.** The sustainable yield is `r K ÷ 4` per day ([world systems](../../design/world-systems.md) §10.2); a stock below 30 % of K is `depleted`. Both are computed, never stored.
6. **In the world.** Lots start at the end of setup (`start_populations`); the economy system advances every population once a game hour after spoilage and production. Takes emit `BiotaTaken` (taker, population, mass). Populations are saved after the projects (the lots, serials shown and taken; species and capacities are setup).

## Invariants

- The ledger check passes every tick; every change of a stock is a transfer or a declared flow with a cause.
- A growing stock never exceeds K; a declining one never falls below K; a stock of zero stays zero.
- The roster after a take equals the roster before without that serial; the roster after growth equals the roster before plus new serials at the end.
- Balances after any partition of an interval equal the single jump's.

## Tests (`test_biota.cpp`)

- **Growth (A1):** 200 days stepped hourly against one jump: identical balances; the stock matches the closed form; above K it declines to K; nothing from nothing. Property: random partitions agree with one jump.
- **Yield (A3):** a forest at K/2 harvested at r K/4 a day for a year stays within 10 % of K/2 and is not depleted; at twice the yield for three more years it is depleted.
- **Approach and kill (A1):** the same stock gives the same 50 deer twice; another species draws differently; both sexes appear; killing deer 17 moves its mass to the hunter, a second kill of 17 and a kill of 999 are refused, the 49 others are unchanged; growth appends serial 50 after them. Property: leave and come back after up to nine cuts shows the same trees minus the cut ones.
- **In the world (A1):** a herd grows by the hour and two kills move mass to the hunter, two refusals name their reason; save, load, save identical; replay and a mid-run resume reproduce the hash.

## As built

- `engine/sim/include/zs/biota.h`, `src/biota.cpp`: `Population`, `Individual`, `Populations` (`grown`, `reset`, `advance`, `roster`, `individual`, `approach`, `take`, `sustainable_yield_per_day`, `depleted`, save and load). Keyed draws through `fields::keyed_unit` and `keyed_normal` ([36](36-fields-and-weather.md)), field numbers 101–105.
- `EconomySystem`: `populations()`, `start_populations()`; command `BiotaTake` (0x200b); refusals `NoPopulation`, `NotOnRoster`; event `BiotaTaken` (58).

## Calibration

| Value | Source |
| --- | --- |
| r, K per species and biome | [world systems](../../design/world-systems.md) §10.2: plantation growth 3–25 m³/ha/yr, recovery time constant about 29 years, Damuth density for animals; the tests use fast placeholders |
| Unit masses | Placeholders (an oak 2 t, a deer 60 kg) |

## Not in this spec

Individuals that move (a herd's position is its cell; animal AI is the walker brain's cousin); predators and trophic chains (one logistic per species here); seasons in growth (r could follow [36](36-fields-and-weather.md)); soil, crops and fallow; fishing's collapse flag at catch under 10 % of peak; zoonosis hazard; forage kcal per hour; saved exceptions beyond `taken` (an injured animal, a marked tree); the cutting action itself ([14](14-action-executor.md), [15](15-items-recipes.md)).

## Open

1. **The `taken` set grows forever.** Serials below the roster window could be forgotten once growth has passed them, but then a serial could be reused by a later roster. Recommendation: keep the set; prune when a population's `next_serial` is reset at a world epoch, which does not exist yet.
2. **Mass shares and the unit.** A kill moves the individual's keyed mass, so `n` can change by one tree for the next roster when shares are heavy. Recommendation: accept; the alternative, uniform masses, loses the big tree.
