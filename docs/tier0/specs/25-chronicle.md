# 25 · Chronicle and cause of death

**Status:** Proposal. **Stage:** 7 (cause chains and death sentences, needed by the AI-only runs of stage 8), 9 (day summaries and death screen). **Needs:** [04](04-hazard-event.md), [06](06-save-replay.md), and every system that writes events.

## Purpose

After a death the player must know which ignored thing killed them (fun question from [concept v2 §9](../../research/concept-v2.md#9-revision-after-pichus-answers-2026-10-05)). The chronicle turns the event log's cause chains into plain sentences.

## State

| Field | Notes |
| --- | --- |
| cause chain | for any event, walk causes back to the root |
| danger tags | fixed list: bleeding, infected wound, dehydration, bad water, spoiled food, starvation, exhaustion, overload, noise, light leak, hidden bite, walker attack, shock (falls join when a system produces them) |
| day summary | per day: notable events with significance score |

## Rules

1. On a death, find the proximate cause (blood loss, sepsis, dehydration, walker bite then turning...) and walk back to the root events and the missed counters (wound never cleaned, fridge without power for 18 h, generator ran all night).
2. Write one or two plain sentences: "Marco died of sepsis on day 9. It started as a cut on his left hand while salvaging a window on day 6. It was never cleaned."
3. The death screen shows the sentence, the chain as a short timeline, and the danger tags.
4. The chronicle only reads the log; it never changes state.

## Invariants

- Every death has a proximate cause and at least one root event.

## Tests

- Scripted deaths for each danger tag produce the expected sentence. (A1)
- Fuzz: 50 AI-only runs, every death has a non-empty chain. (A1)

## Calibration

None. Readability is checked in playtests (fun question 5 and 7).

## Stage 9 additions (Proposal 2026-10-06)

- **Danger tags** are derived from the death's cause chain and the log, never stored: walker attack (a bite or grab up the chain), hidden bite (the bitten pop lived at least 5 minutes and never reported it), bleeding (died of blood loss), infected wound, dehydration, starvation, noise (the walker that bit arrived while the generator was running).
- **Missed counters** add one clause: "the bite was never told to the group", "the wound was never cleaned", "the generator was running when it arrived".
- **Day summary:** per game day, the events with a score (death 10, someone joined 6, broadcast heard 8, breach 4, grid failed 5, out of fuel 5, bite 3, a walker put down 1); the top five in time order.

## Not in Tier 0

The after-death truth replay and the macro chronicle of the full game.

## Model as implemented (Proposal, 2026-10-06)

Code: `tier0::death_sentence` in `engine/sim/src/tier0.cpp`.

- One sentence per `Death` event: name, age, when, cause, then the first bite or strike up the cause chain ("You, 30, died day 2 10:48: the infection from a bite. Bitten on the neck by David Silva, day 1 05:03."). Bleeding deaths link to the wound that bled most. Walkers read "was put down".

**Stage 9:** `danger_tags`, `missed_counters` and `day_summary` as above; the headless run prints the death screen and the day lines.

**Known gaps:** tags exhaustion and overload, light leak and bad water are not derived yet (no deaths come from them in Tier 0 runs); no timeline view (stage 5).
