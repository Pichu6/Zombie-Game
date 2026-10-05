# 22 · Stub census

**Status:** Proposal. **Stage:** 7. **Needs:** [03](03-stock-transfer.md), [04](04-hazard-event.md), [11](11-body-composition.md), [21](21-infection.md).

## Purpose

About 60 neighbours from a fixed table, so every walker is a specific former person, and the block's dead come back on a schedule. Finite: nobody respawns ([0005](../../decisions/0005-finite-world-parity.md)).

## State

| Field | Notes |
| --- | --- |
| person | id, name, age, sex, household, home (on map or off-map street), job, body seed, clothes and items |
| fate | drawn once at run start: alive and fled, dead, turned (with time and place), or "found survivor" slot |
| location | on map (embodied) or off-map pool |
| off-map attention | accumulator fed by noise and light reaching the map border ([18](18-noise-field.md), [19](19-light-field.md)) |

## Rules

1. The table is a data file. Bodies are generated from each person's seed using real anthropometry distributions, so the block has thin, fat, old and young bodies.
2. Fates are drawn up front from the virus preset and the scenario, then scheduled; some neighbours are already walkers in their homes when the run starts.
3. Off-map walkers enter from the map border when the arrival hazard fires: rate = base drift + k × off-map attention, limited by how many off-map walkers remain. Attention decays slowly. So loud nights bring bigger waves, and quiet days are rare chances.
4. A neighbour who arrives as a walker carries their body, clothes and items. The view names them when seen up close ("Mrs. Ortega, from house A").
5. Three census slots are the found survivors ([26](26-run.md)); they are full pops from the start.
6. Conservation: table total = alive (on map + fled) + dead + walkers (on map + off map) + destroyed walkers, checked by the ledger every tick.

## Invariants

- Each person has exactly one fate state and one location.
- Totals constant.

## Tests

- Census total conserved over a 14-day AI-only run. (A1)
- More generator hours at night → more arrivals, over 50 seeded runs. (A3)
- A walker's body matches its person's seed. (A1)

## Calibration

| Value | Source |
| --- | --- |
| Household sizes and ages | National census tables (e.g. US Census ACS or the home country's) |
| Body measurements | NHANES (body mass, height) and ANSUR II anthropometry |
| Arrival and attention rates | No real source; tuned in playtests against the pressure arc |

## Not in Tier 0

Records, lifepaths, SEIRZ cells, warm neighbours who act, the human generator.
