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

- **Have:** the decided concept, design docs distilled from the prototype, recorded decisions, and the clothing research (tools tested on a synthetic body).
- **Don't have:** any engine code. The repo is not on GitHub yet (the user's account is locked by a 2FA issue); history is in local git.

## Next step

1. Plan Tier 0: module layout of the sim core, first systems (body, room graph, power and noise), and the 2D debug view.
2. Push the repo to GitHub once the account is back.

## Waiting on the user

- Engine stack ([0002](decisions/0002-engine-stack.md)).
- Clothing via GPU wrap, and whether tight clothing hides the skin under it ([0003](decisions/0003-clothing-wrap.md)). Not needed until Tier 1–2.
- Quality formula ([0006](decisions/0006-crafting-quality.md)).
- Region (real or fictional) and tone ([vision](vision.md), [0011](decisions/0011-setting-scale-lenses.md)).
