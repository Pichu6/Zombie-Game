# 13 · Capabilities

**Status:** Proposal. **Stage:** 3. **Needs:** [11](11-body-composition.md), [12](12-body-wounds.md).

## Purpose

What a body can do right now, computed from its state: the only body numbers that the action executor, the AI, movement and the UI read. Strength and endurance come from the body ([0007](../../decisions/0007-body.md)); labels like "fat hauler" or "lean scout" only describe it.

## State (all derived, never stored)

| Capability | Range | Inputs |
| --- | --- | --- |
| consciousness | 0–1 | blood loss, sleep pressure, pain, fever, dehydration |
| moving | 0–1 | legs and feet wounds and fractures, pain, exertion reserve |
| manipulation (left, right) | 0–1 | arms and hands wounds, pain |
| sight | 0–1 | head wounds; light level is applied by perception |
| hearing | 0–1 | head wounds; recent loud exposure (stretch) |

## Readouts

| Readout | Rule |
| --- | --- |
| walk and run speed (m/s) | base from height (leg length) and fitness, × moving, × load factor (moving mass / body mass), × stairs factor |
| carry limit (kg) | from muscle mass (arms and trunk) and moving; above it, speed drops and exertion rises fast |
| work rate | × manipulation, × consciousness, × exertion |
| strike force and push force | from muscle mass and body mass ([10](10-fortification.md), [20](20-damage.md)) |
| footstep noise (dB at 1 m) | from body mass, speed, floor type ([18](18-noise-field.md)) |
| stamina drain | per action, scaled by fitness |
| trait labels | "overweight", "lean", "strong", "unfit": thresholds on the state, display only |

## Rules

1. No single "health" multiplier: each capability takes only its own inputs.
2. Any capability at 0 blocks the actions that need it, with a reason the UI shows.
3. Consciousness below a threshold = unconscious (falls, drops held items).
4. Readouts are pure functions: the same state always gives the same values.

## Invariants

- All capabilities in [0, 1]; speeds ≥ 0.

## Tests

- A fresh 60 kg lean body outruns a 110 kg body with the same fitness. (A3)
- A fractured shin drops moving and walk speed; a hand wound does not. (A1)
- Carrying 30 kg slows a light body more than a strong one. (A3)

## Calibration

| Value | Source |
| --- | --- |
| Comfortable walking speed ~1.2–1.4 m/s by age | Bohannon and Andrews (2011) |
| Running speeds | Recreational runner data; tune |
| Load effect on speed | Military load-carriage studies (Knapik et al. 2004) |
| Footstep levels | Measured footfall noise ~40–60 dB at 1 m (estimate; tune) |

## Not in Tier 0

Capability-driven animation, breathing and blood-pumping capabilities as separate organ systems.
