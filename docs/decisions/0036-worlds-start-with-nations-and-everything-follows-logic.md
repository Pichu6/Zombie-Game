# 0036 · Worlds start with nations in place; everything placed follows logic; factions hold territory

- **Date:** 2026-10-06
- **Status:** Decided by the user (intent below, from their message in chat). The generation pipeline and models are Proposals in [world systems](../design/world-systems.md).
- **Extends:** [0018](0018-sandbox-answers.md) (the world is complete at the start), [0035](0035-realistic-world-numbers-and-the-pop-nation-loop.md), [0022](0022-macro-layer-intent.md).

## Decisions

1. **When a world starts, its nations already exist**, placed in logical places, as are its cities, their sizes, supply chains, grid networks and everything else. Nothing is dropped on the map without a reason that the simulation can state.
2. **The user sets the starting parameters** in the sandbox and creation options; the world follows from them.
3. **Factions hold areas of control**, as in Hearts of Iron or Stellaris. They can expand, shrink, be subjugated (become subjects of another), or become conquerors.
4. **The loop runs both ways: macro → micro → macro, state → pop → state.** A decision at state level reaches pops and buildings; what pops and buildings do adds up to the state's capability.
5. **The simulation must cover more than it does:** economy, flora, fauna, resources, processing, building, and the rest of what a world is made of.

## Open (needs the user)

"Become slaves" is read as **subjugation of a faction** (a subject state: vassal, puppet, tributary). Whether the game also models **enslaved or forced-labour populations** inside a state is a separate, sensitive design choice and is not assumed; see [world systems](../design/world-systems.md) section 9.
