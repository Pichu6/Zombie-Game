# 25 · Chronicle and cause of death

**Status:** Proposal. **Stage:** 9. **Needs:** [04](04-hazard-event.md), [06](06-save-replay.md), and every system that writes events.

## Purpose

After a death the player must know which ignored thing killed them (fun question from [concept v2 §9](../../research/concept-v2.md#9-revision-after-pichus-answers-2026-10-05)). The chronicle turns the event log's cause chains into plain sentences.

## State

| Field | Notes |
| --- | --- |
| cause chain | for any event, walk causes back to the root |
| danger tags | fixed list: bleeding, infected wound, dehydration, bad water, spoiled food, starvation, exhaustion, overload, noise, light leak, hidden bite, walker attack, fall, shock |
| day summary | per day: notable events with significance score |

## Rules

1. On a death, find the proximate cause (blood loss, sepsis, dehydration, walker bite then turning...) and walk back to the root events and the missed counters (wound never cleaned, fridge without power for 18 h, generator ran all night).
2. Write one or two plain sentences: "Marco died of sepsis on day 9. It started as a cut on his left hand while salvaging a window on day 6. It was never cleaned."
3. The death screen shows the sentence, the chain as a short timeline, and the danger tags.
4. In a run with a group, the death of the controlled pop moves control to another group member; the run ends when nobody is left or the goal is met ([26](26-run.md)).
5. The chronicle only reads the log; it never changes state.

## Invariants

- Every death has a proximate cause and at least one root event.

## Tests

- Scripted deaths for each danger tag produce the expected sentence. (A1)
- Fuzz: 50 AI-only runs, every death has a non-empty chain. (A1)

## Calibration

None. Readability is checked in playtests (fun question 5 and 7).

## Not in Tier 0

The after-death truth replay and the macro chronicle of the full game.
