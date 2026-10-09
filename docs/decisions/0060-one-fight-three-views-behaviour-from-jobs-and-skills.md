# 0060 · One fight, shown three ways; it happens whether anyone plays it; behaviour from jobs and skills; what a pop shows per layer

- **Date:** 2026-10-09
- **Status:** Decided by the user (2026-10-09): "a fight in the world view should be seen as a TEG or Hearts of Iron fight; in layer 2 as an RTS (World in Conflict, Infection Free Zone); in layer 1 isometric (Thunder Tier One, Project Zomboid, The Sims) or third person (The Last of Us, Project Zomboid's mods). A fight should be carried out whether a player takes control at some level or not. Jobs, skills and professions play an important role in how a pop behaves. In layer 1 I should see a granular status of a pop (traits, body); in layer 2 basic info (happiness, basic status); in layer 3 only country data."
- **Builds on:** [0012](0012-world-simulation-layers.md) (one fact, every layer), [0042](0042-pop-presentation-per-layer.md), [0044](0044-information-scope-per-layer.md), [cameras and modes](../design/cameras-modes.md) (third person and isometric tactical, Decided), research [ZL-43..47](../research/zoom-and-layer-two.md), [world-in-conflict](../research/world-in-conflict.md), [war-in-other-games](../research/war-in-other-games.md), [LJ-15](../research/layer-jumps-multiresolution.md) (an auto-resolved battle is felt as a different model).

## Decision

1. **A fight looks like its layer.**
   - Macro (layer 3): fronts, regions and counters, as in TEG or Hearts of Iron IV.
   - City (layer 2): squads and units on the city map, real-time strategy, as in World in Conflict or Infection Free Zone; one map, one clock, the command tools beside the management tools ([ZL-43, ZL-46](../research/zoom-and-layer-two.md)).
   - Individual (layer 1): bodies, seen isometric (Thunder Tier One, Project Zomboid, The Sims) or in third person (The Last of Us).
2. **A fight happens whether or not anyone plays it.** It is one fight in the one simulation, resolved at the detail the attention allows: bodies tick by tick where someone is attended, squads where a City view watches, counts and real rates where nobody looks. The three resolutions must agree on average (A2): the same troops, weapons, terrain and morale give the same expected outcome at every level, so zooming in or out never changes who is likely to win, and a fight entered mid-way is materialised from its current state.
3. **Behaviour comes from who the pop is.** Job, skills, profession, traits and state decide what a pop does and how well: a soldier holds, a nurse treats, a farmer flees or fights depending on traits and fear. AI pops and officeholders act by the same rules whether or not a player is above them (0058).
4. **What a pop shows follows the layer** (0042, 0044):
   - Individual: the full pop: body by region, needs and states, traits with their pluses and minuses, skills, mind, things carried.
   - City: a basic card: name, job, mood or happiness, health in a word, what it is doing.
   - Macro: country data only (counts, shares, stocks); individuals only for people in office.

## Consequences

- A fight model needs three resolutions with a shared calibration test, like the jump forms of spec 40: a War or Combat spec before any combat code above the Tier 0 bodies.
- The pop record must carry job, skills and traits in a form both the AI and the three views read (spec 24 pop AI, spec 28 pop levels, the generated pops of spec 47).
