# 14 · Action executor

**Status:** Implemented (stage 4; numbers are Proposal). **Stage:** 4. **Needs:** [03](03-stock-transfer.md), [08](08-assembly.md), [13](13-capabilities.md).

## Purpose

The single place where anything is done to the world, by the player or any AI, under the same rules ([interaction](../../design/interaction.md#player-ai-parity)). Parity is enforced here: the AI picks intents; it never has a shortcut.

## State

| Field | Notes |
| --- | --- |
| offer | Declared by an object's capabilities: id, label, target, requirements, phases, effects |
| action | Running instance: actor, offer, target, phase, progress, reservations |
| reservation | One per instance in use |
| queue | Per actor, ordered intents |

## Model: primitives and processes (Proposal, 2026-10-05)

Pichu's constraint: items will number in the thousands and actions must stay compressed. Code holds only the **primitives**; every contextual action is a **process** in data that matches items by property, never by name ([15](15-items-recipes.md#model-materials-properties-stocks-and-things)).

| Layer | What it is | Where it lives |
| --- | --- | --- |
| Primitive | One body verb: move (walk, run, crawl, climb), hold/carry, strike, push/pull, cut, pry, fine work, consume, rest/sleep, exercise, search, wait | Code. Each has a body cost (MET, which regions move), the capability it needs ([13](13-capabilities.md)) and a noise ([18](18-noise-field.md)) |
| Process | A primitive applied to a target with requirements and effects, e.g. "pry window" = pry + tool `lever >= 0.5` | Data (`data/tier0/processes.toml`) |
| Requirement | A property test on a tool, an input or the target: `lever >= 0.5`, `cloth_area >= 0.2`, `water_safe` stock `>= 200 mL` | Data |
| Effect op | One of a fixed set: transfer stock, convert stock, change condition, mount, unmount, add wound, treat wound, set state | Code (the whole effect vocabulary) |
| Phase | approach → prepare → work → finish; each phase lists its primitive and its effect ops | Data |

- **Speed from properties.** Work time = base time ÷ (tool property ÷ the process's reference value), × work rate ([13](13-capabilities.md)). A crowbar (lever 1.0) pries faster than a screwdriver (lever 0.3); both qualify for a reference of 0.25.
- **Knowledge** is a set of known process ids per person, plus one skill number per trade (first aid, carpentry, mechanics, electronics). Everyday processes (eat, drink, open, carry) are known by everyone.
- **Exercise** is a primitive with a MET and training stimulus; the body already counts heavy minutes ([11](11-body-composition.md)).
- New items join old processes automatically when their properties qualify; a new process needs no code unless it needs a new effect op.

**As implemented (stage 4):** `engine/sim/src/actions.cpp`, `data/tier0/processes.toml`.
- 16 primitives with their body costs.
- 17 processes, matching tools and inputs by property.
- 9 effect ops: eat, drink, fill, use stock, treat, board, salvage, take, sleep.
- Phases are approach then work, with effects at finish.
- Player commands (`World::submit`) and AI intents (`World::queue_intent`) take the same path. Only the player's are kept in the input log, so parity compares `World::world_hash()`, which leaves the input log out.
- Floors are fixed entity ids per cell, so dropped items need no extra saved state.
- `zs_headless --scripted-pop [--ai]` runs the stage 4 scenario.

## Rules

1. Five gates decide what an actor sees and can do ([world simulation §9](../../design/world-simulation.md#9-communication-decided-as-intent-model-proposal)): **knowledge** hides an offer; **skill** sets the odds and Q cap; missing **items** or **access** grey it out with the reason; **quality** changes the result; **capability** ([13](13-capabilities.md)) blocks with the reason.
2. Preview runs the same checks as execution and changes nothing. Execution checks again.
3. An action is timed: duration = base × work rate × tool and Q factors. It runs in phases (approach → prepare → work → finish). Effects land only in the phase that causes them.
4. Each phase costs the body (MET level for [11](11-body-composition.md)) and emits its sound ([18](18-noise-field.md)).
5. Interruptions: death, unconsciousness, lost reach, target changed, capability below need, a slow-down event the actor reacts to. Progress is kept for resumable work (barricading, repairing).
6. Transfers move items through the ledger in phases (access source → handle → access destination → fit). The item stays at its origin, reserved, until extracted; then it is in the actor's hands (limited by the carry limit); then it is placed. An interruption drops it on the actor's cell. It is never duplicated or lost. An offer's identity includes its target's location, so a stale menu never acts on a moved item.
7. Arriving is not working: the work phase starts on the tick after the approach ends.
8. Player input becomes intents for the controlled actor: move to, use offer, cancel. The same intents come from the AI ([24](24-pop-ai.md)).
9. Direct control (WASD) sets a movement intent each tick; movement still uses the body's speeds.

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
