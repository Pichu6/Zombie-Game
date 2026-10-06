# Status

Updated: 2026-10-05.

## What we're building

**Tier 0, "The generator problem"** (Decided, see [0010](decisions/0010-concept-v2.md) and [vision](vision.md)): a 2D simulation prototype, SDL3 + ImGui, no Vulkan.

- One hand-made 2-floor building with a basement and yard, plus 2 houses. Every building enterable; walls breachable.
- Start with 1 survivor; find 3 more during the run. Same body model and rules for player and survivors.
- Body in kg (fat, muscle, structure), ~17 regions with wounds and bleeding, hunger, sleep, energy, weekly drift.
- Power, noise and light leaking from the building (water as a stretch goal).
- Walls, doors, barricades and the generator can be scavenged and crafted with quality; ~20 items.
- Zombies made from dead survivors and a census of ~60 neighbours; hidden incubation.
- Goal: survive 14 days. Deterministic sim, save/load, event log, inspectors.

It must answer the fun questions in [concept v2](research/concept-v2.md) (section 5 and section 9) before any renderer work.

## Where we are

- **Have:** the decided concept, design docs distilled from the prototype, recorded decisions, the clothing research, and the world simulation design ([0012](decisions/0012-world-simulation-layers.md), [world simulation](design/world-simulation.md)): one outbreak simulation of real Earth, played as Macro, City or Individual, with the play style chosen in setup.
- **Don't have:** the window, power/noise/light, zombies or AI yet. Built so far: foundations (stage 1), things and places (stage 2) the body (stage 3) and actions and items (stage 4). The repo is not on GitHub yet (the user's account is locked by a 2FA issue); history is in local git.

## Next step

1. Tier 0 [plan](tier0/plan.md) and [specs](tier0/specs/README.md) written (Proposal). Implemented on 2026-10-05, tests green on GCC and Clang with identical golden hashes (Windows/MSVC not yet checked):
   - **Stage 1:** foundations (quantities, own deterministic maths, Philox draws, ledger, hazards and events, clock, save/load/replay).
   - **Stage 2:** condition, assemblies with salvage that conserves mass, and the Tier 0 block map with its room graph (`zs_headless --map`).
   - **Stage 3:** the body (composition, energy, water, blood, sleep, exertion, 17 regions, wounds, infection, healing, capabilities). `zs_headless --body-lab` runs 14 calibration scenarios, all inside their bands. `zs_headless --bench` runs one game day with 4 pops and 60 walkers in about 0.2 s.
   - **Stage 4:** the compressed item and action model (materials, properties, stocks vs things; body primitives plus data processes that match by property), the action executor and a scripted pop that drinks, eats, disinfects and dresses a wound, breaks a chair into planks and boards a window (`zs_headless --scripted-pop`). Player and AI runs of the same intents hash the same.
   - **Stage 6:** the generator problem, headless. A power network (mains and generator behind a transfer switch, breakers, surge and overload stalls), fuel through the ledger, a grid failure drawn in days 2-5, fridge temperature and spoilage, a noise field through walls and doors, and light with window leaks, curtains and boards; dark slows fine work. `zs_headless --stage6` runs the grid failure, refuel, fuel-out and spoilage, with a night snapshot of noise and light. The window overlays wait for stage 5.
   - **Stage 7:** the dead, headless. A census of 60 named neighbours with drawn fates and real-sized bodies, counted through the ledger; off-map walkers arrive faster while the generator's noise and lit windows reach past the block; walkers hear, see, follow, push doors and windows part by part, grab and bite; bites can carry the infection, the bitten die on a drawn schedule and rise with their wounds and items; crowbar strikes by body strength; one-sentence death causes. `zs_headless --stage7 [--trace]` runs it and the replay viewer shows the walkers. The scripted pop is a stand-in: it only fights what is next to it.
   - **Next:** stage 5 (first window, a sandbox: SDL3 + ImGui) on the user's Windows PC, which is also the first MSVC check; stage 8 (pop AI) can go ahead headless.
2. Push the repo to GitHub once the account is back.

## Waiting on the user

- Three questions on scale across layers and the Macro pressure bar ([vision](vision.md#open-questions)), each with a recommendation.
