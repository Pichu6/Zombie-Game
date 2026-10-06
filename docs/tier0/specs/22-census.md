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

## Model as implemented (Proposal, 2026-10-06)

Code: `engine/sim/src/census.cpp`, table `data/tier0/census.toml` (60 people, made by `tools/census/make_census.py`).

- **Ledger:** resource `people`, one owner per fate (fled, dead, survivor slot, walker off the map, walker on the map, destroyed, alive in the group). Every change is a transfer, so conservation is the ledger check; the player joins as one more person.
- **Fates** at setup: people living on the map (house_a, house_b) are walkers at home with 0.5, else fled; off-map neighbours are walkers 0.35, dead 0.15, fled otherwise; three rows are survivor slots.
- **Bodies** for every walker: NHANES-like height, BMI 27.5 × e^(0.18 z), Deurenberg body fat, VO2max by age and sex.
- **Arrivals:** attention gains (metres the generator's audible range and lit windows' seen distance reach past the border) / 1000 per hour and halves every 4 h; arrivals per hour = 0.05 + 0.35 × attention, from a random outdoor border cell. A day with the generator brought about twice the arrivals of a quiet one (test, 6 seeds).

- **Survivors (stage 8):** in a group run the survivor slots are embodied at setup next to the player (`Census::embody`), census fate alive in the group; names are unique within the table.

**Known gaps:** survivor slots outside a group run have no bodies yet (stage 9); arrivals carry body only, no clothes or items; names show in the event log, not up close in a view yet.
