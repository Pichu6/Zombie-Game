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
- **Don't have:** any engine code. The repo is not on GitHub yet (the user's account is locked by a 2FA issue); history is in local git.

## Next step

1. Tier 0 specs and plan are written (Proposal): [plan](tier0/plan.md), [specs](tier0/specs/README.md). Next: stage 0 (build skeleton), then stage 1 (foundations).
2. Push the repo to GitHub once the account is back.

## Waiting on the user

- Nothing blocking. All open questions are answered ([0014](decisions/0014-open-questions-answered.md), [0015](decisions/0015-tone-realistic-damage.md)); only tuning numbers remain.
