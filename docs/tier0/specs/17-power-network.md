# 17 · Power network

**Status:** Proposal. **Stage:** 6. **Needs:** [03](03-stock-transfer.md), [08](08-assembly.md), [09](09-site-map.md).

## Purpose

The building's power as a small graph: mains or generator → transfer switch → panel → breakers → ~10 receivers. It keeps food cold and lights on, and every watt from the generator costs fuel and noise. This is the generator problem.

## State

| Field | Notes |
| --- | --- |
| nodes | sources (mains feed, generator), switches (transfer switch, main breaker, circuit breakers), receivers |
| edges | cables (assemblies with Q and condition, carry limit in W) |
| source | max W (from engine and alternator Q and condition), running, fuel stock (mL) |
| switch | open/closed, trip rating (W) |
| receiver | demand W (running and start surge), on/off, powered yes/no |
| mains | present until the grid-failure event, then dead |

## Rules

1. Solved only when dirty (a switch, receiver, cable or source changes): find connected components from each live source over closed switches, sum demand, compare with capacity. Cost is a few hundred operations; never per tick.
2. Over a breaker's rating: it trips (event, slow-down). Over the generator's capacity: brownout, then the generator stalls (event).
3. Generator fuel use per hour = idle use + (full-load use − idle use) × load fraction; fuel moves to `world.burned` through the ledger.
4. A running generator is a continuous noise source ([18](18-noise-field.md)); its level depends on its type and enclosure (indoors is quieter outside but brings carbon monoxide, Tier 1).
5. Grid failure: in Tier 0 a scheduled world event (drawn between day 2 and day 5) kills the mains, a stand-in for staff loss in the full sim ([world simulation §5](../../design/world-simulation.md#5-entanglement-decided-as-a-requirement-loop-details-proposal)).
6. Working on a live circuit (panel, cable) is a hazard (shock injury). The AI refuses it ([24](24-pop-ai.md)); the player may do it.
7. Model in watts per connected component only; no voltage or current.

## Invariants

- A receiver is powered only if a closed path reaches a running source.
- Fuel never negative; total fuel conserved through the ledger.

## Tests

- Switching on the hot plate with fridge and lamps on a 3 kW generator: inside capacity; adding a 1.4 kW saw stalls it. (A1)
- Fuel use at 50 % load over 10 h lands in the spec-sheet band. (A3)
- Grid failure turns every mains-fed receiver off in one solve. (A1)

## Calibration

| Value | Source |
| --- | --- |
| Portable generator 3–3.5 kW: capacity, fuel use, tank size | Manufacturer spec sheets (open-frame and inverter types) |
| Receiver demand: LED lamp ~10 W, fridge ~100–200 W running with a start surge, hot plate ~1–1.5 kW, radio ~10–30 W | Appliance labels; US DOE / Energy Star data |
| Shock hazard | Electrical safety references (OSHA); tune as an injury chance |

## Not in Tier 0

Substations, poles and street grid, voltage and current, cable overheating fires, solar or batteries, water pump (stretch with the water tank).
