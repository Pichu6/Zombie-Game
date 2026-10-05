# 14 · Action executor

**Status:** Proposal. **Stage:** 4. **Needs:** [03](03-stock-transfer.md), [08](08-assembly.md), [13](13-capabilities.md).

## Purpose

The single place where anything is done to the world, by the player or any AI, under the same rules ([interaction](../../design/interaction.md#player-ai-parity)). Parity is enforced here: the AI picks intents; it never has a shortcut.

## State

| Field | Notes |
| --- | --- |
| offer | Declared by an object's capabilities: id, label, target, requirements, phases, effects |
| action | Running instance: actor, offer, target, phase, progress, reservations |
| reservation | One per instance in use |
| queue | Per actor, ordered intents |

## Rules

1. Five gates decide what an actor sees and can do ([world simulation §9](../../design/world-simulation.md#9-communication-decided-as-intent-model-proposal)): **knowledge** hides an offer; **skill** sets the odds and Q cap; missing **items** or **access** grey it out with the reason; **quality** changes the result; **capability** ([13](13-capabilities.md)) blocks with the reason.
2. Preview runs the same checks as execution and changes nothing. Execution checks again.
3. An action is timed: duration = base × work rate × tool and Q factors. It runs in phases (approach → prepare → work → finish). Effects land only in the phase that causes them.
4. Each phase costs the body (MET level for [11](11-body-composition.md)) and emits its sound ([18](18-noise-field.md)).
5. Interruptions: death, unconsciousness, lost reach, target changed, capability below need, a slow-down event the actor reacts to. Progress is kept for resumable work (barricading, repairing).
6. Transfers move items through the ledger in phases (access source → handle → access destination → fit). An interruption never duplicates or loses an item.
7. Player input becomes intents for the controlled actor: move to, use offer, cancel. The same intents come from the AI ([24](24-pop-ai.md)).
8. Direct control (WASD) sets a movement intent each tick; movement still uses the body's speeds.

## Invariants

- One running action per actor; one reservation per instance.
- No effect without a completed phase.

## Tests

- The same scripted intents for a player-controlled and an AI-controlled pop give the same state hash. (A1)
- Interrupting a transfer at each phase leaves the item in exactly one place. (A1)
- Knowledge gate: a pop without electronics knowledge does not see "repair radio board". (A1)

## Calibration

Durations per action in the data files, each with a source or marked estimate (e.g. nailing one board ~1–2 min by hand).

## Not in Tier 0

Learning from practice, novelty keys, social actions (ask, promise, lie), trading.
