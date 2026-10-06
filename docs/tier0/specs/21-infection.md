# 21 · Zombie infection

**Status:** Proposal. **Stage:** 7. **Needs:** [04](04-hazard-event.md), [11](11-body-composition.md), [12](12-body-wounds.md).

## Purpose

How a living body becomes a walker, with the default origin: a hidden bite, 1–2 days to turn ([0014](../../decisions/0014-open-questions-answered.md)). The infected friend inside the group must be possible and must not be shown to the player by the UI.

## State (per body)

| Field | Notes |
| --- | --- |
| pathogen dose | From bites and scratches ([20](20-damage.md)); 0 if never exposed |
| stage | none → incubating → prodrome → dying → dead → walker |
| stage times | Drawn once at exposure, from the virus settings |
| symptoms | fever, chills, fatigue, pallor, wound inflammation: shown as observable tells |
| hiding | whether the carrier hides the wound (pop AI decision, [24](24-pop-ai.md)) |

## Rules

1. Exposure: infection is established with a probability by route (bite high, scratch lower, splash lowest) × dose, drawn once.
2. Times to prodrome, death and reactivation are drawn up front and scheduled ([04](04-hazard-event.md)); default total from bite to death 1–2 days, reactivation within an hour of death (tune).
3. Symptoms raise core temperature (fever cost in [11](11-body-composition.md)), lower consciousness and exertion; they are visible as tells (sweating, pallor, shivering, a dressed wound), never as an "infected" label.
4. At death, the body keeps everything: masses, wounds, clothes, items. It becomes a walker with the walker brain ([23](23-walker-brain.md)) and a walker profile: no sleep, no pain, no fear, no tiring by default, poorer coordination.
5. People who die without the pathogen stay dead (default origin). The origin setting can change this ("everyone reanimates") without new rules.
6. All numbers come from the virus settings; Tier 0 ships the default preset only.
7. The debug inspector shows the truth; the player inspector does not.

## Invariants

- Stages only move forward; a walker never returns to living.
- Census conservation holds through turning ([22](22-census.md)).

## Tests

- 100 seeded bite exposures: time to reactivation inside the preset band. (A2)
- A pop bitten on day 3 with no treatment rises as a walker with the same wounds and items. (A1)
- Player inspector never shows the stage field. (A1)

## Calibration

| Value | Source |
| --- | --- |
| Default preset (hidden bite, 1–2 days) | Decided ([0014](../../decisions/0014-open-questions-answered.md)) |
| Fever and prodrome feel | Clinical courses of rabies and sepsis as analogues (tone, not numbers) |

## Not in Tier 0

Long incubation (weeks), cures and treatments, airborne routes, SEIRZ spread across the town.

## Model as implemented (Proposal, 2026-10-06)

Code: `Walkers::bite`, `Walkers::step_turnings`, `Walkers::rise`; `Bodies::kill`, `Bodies::reanimate`.

- A bite establishes the infection once per person: 0.5 if skin-deep, 0.9 deeper (default preset; 74 of 100 test bites). Times are drawn at the bite: prodrome 6–12 h, death 24–48 h, rising 10–60 min after death (spec 21 band, test over 100 bites).
- At the drawn time an infected body still alive dies (`Pathogen`). Any infected body that dies of anything but a destroyed brain rises: same body, wounds and items, the walker flag set, `Reanimated`, census back to walker on the map.
- A corpse whose brain was destroyed before it rose (`Walkers::put_down`) stays dead; the group's AI does this for its own dead (spec 24).
- `Walkers::turning_of` is the debug truth; nothing in the player view shows it.

**Known gaps:** bites only (no scratch or splash route, dose unused); prodrome is a recorded time with no symptoms or tells yet; the walker profile is only "doesn't bleed to death, isn't driven by the pop executor"; no settings beyond the default preset.
