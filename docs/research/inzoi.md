# inZOI, as a reference for the Individual layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Why:** [0025](../decisions/0025-sandbox-engine-first-game.md) names it with The Sims for the life-game end of the range; the user listed it again. Part of [reference games by layer](reference-games-by-layer.md).

**Evidence caveat:** checked by web search (fan wiki, news on NVIDIA ACE). The game is in early access, so its systems change; I found nothing reliable on disease spreading through its cities, so that is left out.

## What it is

Krafton's life simulation on Unreal Engine 5, in early access since March 2025. Its people are called Zois; the player can edit the city as well as the household.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Karma** | Each Zoi's moral standing on a seven-step scale from its actions. It changes how others treat them, which dialogue and careers are open, and what happens after death. The city's total karma affects the city. A community reputation system is planned for 2026 | Beliefs about a pop held by other pops ([spec 39](../tier0/specs/39-orders-and-belief.md)): a reputation that comes from who saw what, not one global score |
| **Smart Zoi** | An optional mode where a small language model running on the player's GPU (NVIDIA ACE) drives a Zoi; the player can read its thoughts and set its personality and life goals | Our AI is deterministic and replayable ([spec 06](../tier0/specs/06-save-replay.md)), so a language model can't drive choices. Reading a pop's "thoughts" can still be a lens: its current goal, its strongest need, what it believes |
| **City editing** | Change a city's look and details from the city view | City layer zones ([world systems](../design/world-systems.md)) |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| Reputation that adds up from one person to a city | One global karma number per person |
| A "thoughts" lens: why a pop is doing what it does, read from its AI state | A language model choosing actions: it breaks determinism and golden tests |

## Questions this leaves (candidates for decisions)

1. Should the selected pop show a "why" line (current goal, the need behind it, what it believes) as part of making the window readable?
2. Is reputation per community (what this town believes about you) a Macro or City readout as well?

## Sources

- Karma: https://thegameswiki.com/inzoi/wiki/karma-and-reputation-system
- Smart Zoi and NVIDIA ACE: https://www.gamestar.de/artikel/inzoi-nvidia-ace-ki-meinung,3431176.html ; https://www.smartworld.it/news/nvidia-ace-inzoi-gdc-2025.html
