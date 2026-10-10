# Activities and mini-games

Intent Decided ([0019](../decisions/0019-activities-and-delegation.md)); everything else Proposal. Research: `research/global-sim/loop/25-activities-minigames.md`.

## What an activity is

An activity is an existing timed action (see [items and crafting](items-crafting.md), [interaction](interaction.md)) plus one or two optional **hands-on moments**: the critical step (the first board, the risky nail, the stitch), not every hour of work. The game clock never pauses. Each activity has a real effect in the simulation, gated by knowledge, skill, tools and quality, and pops do it through the same action.

A good activity has stakes, a choice before the input, visible progress, and an exit once it gets repetitive. Most trade speed against noise or risk, so each one is a small version of the core choice.

## Parity: skill buys tolerance, hands buy attention

- Skill, tool quality and body state (pain, cold, fear, fatigue) set how wide the success window is, how much it drifts and what readings the player sees.
- The player's input, or the pop's draw, lands a performance value inside that window.
- The result never exceeds `min(skill, Q_tool, method ceiling, Q_station)` ([0014](../decisions/0014-open-questions-answered.md)).
- **Auto is exactly the pop's draw** and is the default ([0020](../decisions/0020-activity-answers.md)); playing by hand is optional and never required to get a fair result. A test checks that player and pop results match in distribution.

## Eight shared interaction types

| Type | Feel | Example |
| --- | --- | --- |
| Steady | Hold still against drift | Aiming, stitching, pouring fuel |
| Stroke | Rhythm and timing | Sawing, hammering, scrubbing |
| Procedure | Right steps in the right order | First aid, starting a generator, radio setup |
| Trace | Follow a path | Cutting along a line, wiring |
| Load | Balance weight and space | Packing, carrying, loading a vehicle |
| Inspect | Find the fault by looking and listening | Diagnosing an engine, searching a room quietly |
| Tune | Find a value by feedback | Radio frequency, carburettor, lockpicking |
| Exchange | Offer and read the other side | Trade, negotiation, cards |

Each one reads values from the simulation and returns a performance value plus physical events (noise, slips, snaps). All work in isometric and over-the-shoulder cameras.

## Families

Body care, medicine, cooking and food preservation, repair and mechanics, electrical, plumbing, carpentry and fortification, scavenging and dismantling, farming, foraging and hunting, radio and communication, driving and hauling, social and trade, leisure and morale. The catalogue with effects, gates and pop versions is in the research file.

**First candidates** (six of the eight types cover them): barricading; refuelling and starting the generator; washing and bandaging wounds; boiling water and cooking; quiet searching; carrying and loading; radio tuning.

## Answered ([0020](../decisions/0020-activity-answers.md))

1. Hands never beat skill.
2. Automatic by default; hands-on the first time as a tutorial, then optional. Proposal for how a hands-on moment pays off without breaking parity: it can land in the upper part of the character's own window (for example a press-to-build-faster rhythm, War Dogs style), a part a skilled pop also reaches; it never raises the ceiling.
3. Simple, easy and varied (GTA, Shenmue, War Dogs building), in both cameras.
4. Items carry their owners' lives: a zombie's pockets come from its census record, and its house key opens its real house (the building ledger in [world simulation](world-simulation.md) section 12).

## Open

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- Is "ask a pop what they know" an Individual-layer action from the start, reading spec 39 beliefs? ([shenmue](../research/shenmue.md))
- Should a town's establishments show opening hours from their staff rosters? ([shenmue](../research/shenmue.md))
- From [cognitive-load](../research/cognitive-load.md): the first hands-on run of an activity (0020) as a worked example, then completion (the player does the last steps), then free; faded per player and skipped for anyone who has done it, since guidance hurts the experienced ([CLT-40](../research/cognitive-load.md), [CLT-17](../research/cognitive-load.md)).

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [gta](../research/gta.md): GTA, as a reference for the Individual layer
- [shenmue](../research/shenmue.md): Shenmue, as a reference for the Individual layer
- [cognitive-load](../research/cognitive-load.md): Cognitive load: how much a player can take in, and how deep games teach
