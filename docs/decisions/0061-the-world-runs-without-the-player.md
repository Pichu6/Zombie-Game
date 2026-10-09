# 0061 · The world runs without the player; playing is changing inputs; watching changes nothing

- **Date:** 2026-10-09
- **Status:** Decided by the user (2026-10-09): "not getting involved in the simulation should be like in some football games where you can just simulate the match. The simulation should carry on independent of the players. Player involvement is just like changing parameters live; the world should react realistically. It does not matter which zoom or layer we are watching."
- **Builds on:** [0012](0012-world-simulation-layers.md) (one fact, every layer), [0022](0022-macro-layer-intent.md) ("events don't wait": the simulation decides and carries on), [0042](0042-pop-presentation-per-layer.md) (clicking never changes state), [0048](0048-realistic-time-default-commit-on-observation-and-pins.md), [0058](0058-offices-open-layers-single-player-is-multiplayer-with-ai.md) (AI in every chair), [0060](0060-one-fight-three-views-behaviour-from-jobs-and-skills.md) rule 2.

## Decision

1. **The world needs no player.** Every role (citizen, mayor, governor, president, commander) is filled by a pop with its own AI; a world with no player at all runs to the same kind of outcomes as one with players. "Simulate the match" is always available: a fight, a crisis or a whole period can be left to run.
2. **Playing is changing inputs.** A player acts only through the role they hold: an order, a law, a budget, a move, a choice. That changes the inputs of the simulation, the same inputs an AI in that role would change, and the world responds by its own rules. There is no player-only lever that bypasses the rules.
3. **Watching changes nothing.** The layer, the zoom and the view decide how much is shown and at what detail it is computed, never what happens: exact jump forms where they exist (spec 40, A1), the same expected outcome where draws are involved (0060 rule 2, A2). Looking away and looking back, or never looking, gives the same world.

## Consequences

- A standing test for every system: a run with no player and with the views closed matches the watched run (A1 where the forms are exact, A2 where draws are involved). The window and the headless run already share one state hash (spec 16, A1).
- Every AI role must be good enough to run its part unattended; a role only a player can fill is a gap.
