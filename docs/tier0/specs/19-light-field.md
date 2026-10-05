# 19 · Light field

**Status:** Proposal. **Stage:** 6. **Needs:** [09](09-site-map.md), [17](17-power-network.md).

## Purpose

Light lets people work and see, and leaks out of windows at night where the dead can see it. Darkness also changes what pops and walkers perceive ([threats](../../design/threats.md#perception-and-attraction)).

## State

| Field | Notes |
| --- | --- |
| ambient | by hour: day ~10 000+ lux outdoors, twilight ~1–10, moonlit ~0.1–0.3, moonless ~0.001; indoors a fraction through windows |
| sources | lamps (powered), flashlight (cone, battery), stove flame, fire (later) |
| room light | illuminance per room from its sources (lumens / floor area, simple) |
| window leak | per window: room light × glass transmission × (1 − cover by curtain or boards) |

## Model as implemented (Proposal, 2026-10-05)

Code: `Power::lux_at`, `Power::window_leaks` in `engine/sim/src/power.cpp`.

- **Ambient** by hour (tick 0 is 08:00): 0.1 lux at night, log-linear through twilight (05–07, 19–21), 10 000 lux by day. Indoor cells get 2 % of it.
- **Lamp light** = lumens of powered lamps in the cell's walled space / its cells (1 m² each). A walled space is cells joined by edges with nothing built on them, so light doesn't pass doors or windows.
- **Window leak** = brightest inner-side lamp light × transmission (0.85 glass, 1 open or broken) × (1 − cover). Cover = 1 − (1 − curtain)(1 − boards / plank slots); a cloth of ≥ 1 m² hung on the window (`hang_curtain`) is 0.97.
- **Seen from** d = sqrt(leak × 1.2 m² / π / (0.02 × outdoor ambient + 0.0002)): the lit pane as a small diffuse source whose light at the observer must beat 2 % of ambient plus a floor. This is an attention distance (what pulls a walker), not the limit of sight, which is far longer. The kitchen lamp's window shows at ~53 m at night, curtained ~9 m, boarded never, by day never.
- **Work:** fine, strike, pry and cut work runs at 0.5 below 10 lux, rising log-linearly to 1 at 100 lux.

**Known gaps:** the attention constants are estimates; light is recomputed on every query (cheap now; spec rule 5 wants on change); no flashlights or flames; light doesn't spill through open doors; windowless rooms still get the 2 % daylight share; sight (rule 2) waits for walkers and pop AI; a hung curtain can't be taken down yet.

## Rules

1. A cell's light = ambient share + room light (+ flashlight cone if it falls there).
2. Seeing: a target is seen if in line of sight, inside range, and its light is above the observer's threshold (scaled by sight capability). In the dark, sight range collapses to what light allows.
3. Work in poor light is slower and riskier (fine repairs most), which pushes the player to light the workshop.
4. A lit window at night is visible from outside up to the distance where the leak still stands out from ambient; walkers that see it treat it as a stimulus ([23](23-walker-brain.md)). In daylight it never stands out.
5. Recomputed on change (lamp on, curtain drawn, hour step), not per tick, except moving flashlights.

## Invariants

- Light ≥ 0; covering a window never increases its leak.

## Tests

- A lit, uncovered window at night is visible to a walker at 50 m; a curtained one only up close; a boarded one not at all. (A3)
- Repair time in a dark room is longer than in a lit one. (A3)

## Calibration

| Value | Source |
| --- | --- |
| Outdoor illuminance by sky condition | Standard photometry tables (IES Lighting Handbook) |
| Indoor task lighting ~300–500 lux | IES / EN 12464-1 recommendations |
| LED bulb ~800 lm at ~10 W; flashlight 100–1000 lm | Product data |

## Not in Tier 0

Shadows, colour, vision cones on the map beyond simple line of sight, light-sensitive walker variants.
