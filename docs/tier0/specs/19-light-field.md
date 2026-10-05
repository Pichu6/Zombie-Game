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
