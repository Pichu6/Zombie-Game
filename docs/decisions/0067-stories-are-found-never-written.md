# 0067 · Stories are found, never written: the story explorer, cases from many sources, blocks for every kind of story, a diary per pop

- **Date:** 2026-10-10
- **Status:** Decided by the user as intent (2026-10-09 and 2026-10-10); the ground rules below are defaults taken by Claude ([0052](0052-one-executable-one-simulation.md)). The user's words (lightly cleaned):
  - "I want the user to be able to navigate through the world it simulated to see and find things, like the people who use Google Earth that way… intriguing stories one could tie by knowing who knew who and did what. Like watching or reading a show… all tied with coherence with the world and places."
  - "I would like the user to be able to find mysteries or solve cases from different sources of information: a note on character A and another on character B… seeing a crime being committed and following the criminal, and that ends up leading to a conspiracy."
  - "I take the lore as background mostly, but don't know how it could be procedural during the sim, so that when zoomed in enough there's logic between everything. That discovery feeling."
  - "Not only detective, crime and conspiracy stories: it's for everything: love, thrill, sad, action, adventure… building blocks for different types of things or contexts."
  - "Every pop has a diary where the story of that pop up to the moment is. Maybe a daily report?"
- **Builds on:** [0061](0061-the-world-runs-without-the-player.md) (watching changes nothing), [0062](0062-the-game-loop-attention-possession-and-late-starts.md), [0048](0048-realistic-time-default-commit-on-observation-and-pins.md) (commit on observation), [0066](0066-player-states-embodied-detached-observer.md) (detachment levels), the macro layer's "events are detected, never injected", and the designs [story explorer](../design/story-explorer.md), [cases and living lore](../design/cases-and-living-lore.md) and [story blocks](../design/story-blocks.md), with research [library-of-babel-and-sacred-geometry](../research/library-of-babel-and-sacred-geometry.md), [procedural-mysteries](../research/procedural-mysteries.md), [story-building-blocks](../research/story-building-blocks.md) and [lore-made-and-generated-worlds](../research/lore-made-and-generated-worlds.md).

## Decision

1. **Found, never written.** Every story the game shows is made of facts the simulation holds: events, derived facts (pure functions of seed and id) and beliefs. Nothing adds an event, a person or a motive for drama. A **librarian** (story sifting, LB-16, LB-18) finds stories as they form; the **explorer** lets the player navigate the world like a globe viewer to read, watch and follow them.
2. **Every fact has a place and a time**, and the past is shown at the detail it was lived: a place that ran cold shows counts with the named people derived behind them, not scenes. Dramatising *how* is optional and labelled; *who* and *what* are never invented. No story shows the future.
3. **Truth first, traces from the truth.** A case is a real chain of secret acts; every clue is a projection of it (physical, document, witness, ledger, infrastructure). Nothing is placed to make a case fair; a case with three independent trace routes per key fact is fair, one with fewer is cold (MY-1, MY-19, MY-20).
4. **Blocks write intentions and words, never outcomes.** Pre-written blocks (seeds, wants, places, shapes, lines, scenes, broadcasts, objects that remember) cast real pops and places; they change the world only through actions pops choose under the normal rules. Tone is computed from each pop's arc, not written (SB-13, SB-15, LR-20..LR-22).
5. **Lore is layered.** The premise (what the outbreak is and its rules) is written and kept as parameters, with optional authored origin presets; history, lives and events are generated and simulated (LR-23). Lifepath hooks (debts, old flames, grudges, secrets) wake only under the simulation's pressure.
6. **Every pop has a diary,** one page per day, rendered on demand from facts and beliefs, so it costs nothing until read. Two voices: the **report** (what happened) and the **diary** (what the pop believed and felt, which can be wrong). Quiet days fold. Only physical diaries kept by pops with a writing habit are stored, as items and as traces.
7. **Visibility follows the detachment level** (0066): free sees the truth; knowledge sees what your pop or side knows; off sees only your own diary.

## Consequences

- First slices need nothing new built: a per-pop diary page from the Tier 0 run (`zs diary`), and the first case inside Tier 0, "who took the fuel" (improvements 51 and 53).
- Content tools §4.2 now follows pop continuity: clicking a name reads that person and never promotes them.
- The world's own story spots (a deterministic share of the detail budget chosen from world state) are a Proposal for spec 27, to be measured against the compute budget.
