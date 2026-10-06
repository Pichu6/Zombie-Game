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
- **No-outbreak game** ([0023](decisions/0023-no-outbreak-game-and-parameter-catalogue.md), Decided intent): the world must also run as a politics, grand-strategy and Sims-like game with no outbreak. [Sandbox parameters](design/sandbox-parameters.md) (Proposal) lists every setting for session, world, nations, cultures, pops, groups, businesses, corporations and economy, [0024](decisions/0024-births-military-mystery-tech-leaders.md) (Decided) adds births, ageing and succession of leaders and researchers, military power, a mystery late outbreak with an off option, a Stellaris-style tech tree (shape tbd) and officeholders you can follow and play. The hard part is one person at every zoom level (open question 1 in the doc; model in [pop continuity](design/pop-continuity.md), spec [28](tier0/specs/28-pop-levels.md): lazy / record / embodied pops, benchmark first, [0026](decisions/0026-pop-continuity-and-capability-loss.md): one pop at every layer, templates, hidden mystery, capability loss both ways). [0025](decisions/0025-leader-records-foxhole-tech-readout.md): leaders stay records, Foxhole to research for war, tech tree reads from what exists in Layers 2 and 1.
- **Don't have:** the window, power/noise/light, zombies or AI yet. Built so far: foundations (stage 1), things and places (stage 2) the body (stage 3) and actions and items (stage 4). The repo is not on GitHub yet (the user's account is locked by a 2FA issue); history is in local git.

## Next step

1. Tier 0 [plan](tier0/plan.md) and [specs](tier0/specs/README.md) written (Proposal). Implemented on 2026-10-05, tests green on GCC and Clang with identical golden hashes (Windows/MSVC not yet checked):
   - **Stage 1:** foundations (quantities, own deterministic maths, Philox draws, ledger, hazards and events, clock, save/load/replay).
   - **Stage 2:** condition, assemblies with salvage that conserves mass, and the Tier 0 block map with its room graph (`zs_headless --map`).
   - **Stage 3:** the body (composition, energy, water, blood, sleep, exertion, 17 regions, wounds, infection, healing, capabilities). `zs_headless --body-lab` runs 14 calibration scenarios, all inside their bands. `zs_headless --bench` runs one game day with 4 pops and 60 walkers in about 0.2 s.
   - **Stage 4:** the compressed item and action model (materials, properties, stocks vs things; body primitives plus data processes that match by property), the action executor and a scripted pop that drinks, eats, disinfects and dresses a wound, breaks a chair into planks and boards a window (`zs_headless --scripted-pop`). Player and AI runs of the same intents hash the same.
   - **Stage 6:** the generator problem, headless. A power network (mains and generator behind a transfer switch, breakers, surge and overload stalls), fuel through the ledger, a grid failure drawn in days 2-5, fridge temperature and spoilage, a noise field through walls and doors, and light with window leaks, curtains and boards; dark slows fine work. `zs_headless --stage6` runs the grid failure, refuel, fuel-out and spoilage, with a night snapshot of noise and light. The window overlays wait for stage 5.
   - **Stage 7:** the dead, headless. A census of 60 named neighbours with drawn fates and real-sized bodies, counted through the ledger; off-map walkers arrive faster while the generator's noise and lit windows reach past the block; walkers hear, see, follow, push doors and windows part by part, grab and bite; bites can carry the infection, the bitten die on a drawn schedule and rise with their wounds and items; crowbar strikes by body strength; one-sentence death causes. `zs_headless --stage7 [--trace]` runs it and the replay viewer shows the walkers. The scripted pop is a stand-in: it only fights what is next to it.
   - **Stage 8:** the group, headless. Four pops driven by a small utility AI from their own bodies: fight or flee from fear, wounds, thirst, hunger, sleep, orders, jobs (keep the generator going by day, board windows, keep watch), refusals with reasons (dark rooms, open ground with walkers in sight), honest or secretive about bites, helping a pop in a fight, putting down their own dead before they rise; possession hands a pop to the player and back. `zs_headless --group [--seeds 50] [--trace]`; the replay viewer shows the group. AI-only, 14 days, 50 seeds: someone alive in 27 runs, deaths mostly from bite infection (measured with the generator run 08-18; stage 9 moved it to 10-16).
   - **Stage 9:** the run, headless. "The generator problem" over 14 days: the player starts alone; a neighbour (short of water) and a technician wait in their houses to be found and invited; a stranger reaches the gate on day 3-5, bitten and hiding it in half the seeds; only the technician can repair the radio, which needs power and plays an evacuation broadcast from day 9-12 at noon; control passes to the next member when the controlled one dies; the run ends at day 14 or when nobody is left. Death screen with danger tags and what was missed; a day-by-day summary. `zs_headless --run [--seeds 12] [--careless] [--trace]`. AI-only, 48 seeds: someone reaches day 14 in 23 (8 of them hear the broadcast); the fuel runs out between day 8 and 12 in 11; careless play: 8 of 48 reach day 14.
   - **Content tools** ([design](design/content-tools.md)): seeded name, pop and town generator with six culture packs (`zs_gen town`); item, interior and building generators are design only.
   - **Stage 5 (built, not yet run on the PC):** the sandbox window, SDL3 + Dear ImGui: the run on a top-down map lit by the simulation, with noise and light overlays, WASD and right-click actions greyed with reasons, inspector, group panel (control any member or let the AI drive), event log, time controls with rest, save/load. Build steps in spec 16.
   - **Windows:** GitHub Actions (`.github/workflows/windows.yml`) builds with MSVC on every push to main, runs the sim and gen tests (all golden hashes match Linux, first passed 2026-10-06) and uploads a zip (`juego-zombie-windows`) with the sandbox, the headless app and the data.
   - **Next:** Pichu plays the sandbox from the zip; then stage 10 (kill or keep: real player runs).
2. Push the repo to GitHub once the account is back.

## Waiting on the user

- Nothing. All design questions are answered (up to [0022](decisions/0022-macro-layer-intent.md)).
