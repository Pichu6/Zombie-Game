# Vision

A 3D zombie survival game on a custom engine (C++/Vulkan). The player embodies an inhabitant of a finite world. That body is a simulated human: it tires, gets hurt, learns and carries real weight. The city is simulated too: power, water and air are networks that break, get exploited and get repaired. The game is played in third person and in isometric view on the same character. Group management and co-op come later. Every object can be disassembled and crafted. Its quality depends on parts, tool and skill.

Labels: **Decided** (the user confirmed it in the prototype), **Proposal** (not confirmed), **Open** (pending question).

## Fantasy

Survive a long time in a city that is shutting down. Every fight is dangerous, even against few enemies. What you find never respawns. To last, you must understand the body, objects and infrastructure, and organize others.

## Pillars

| Pillar | What it means | Status |
| --- | --- | --- |
| Finite world | People, loot and zombies do not respawn. Long, hard survival. | Decided |
| Simulated body | Injuries by zone, metabolism, body composition and real load. No generic health bar. | Proposal (reduced version) |
| Everything is disassembled and crafted | Objects as assemblies; Q inherited from parts, capped by skill and tool. | Decided |
| Knowledge and profession | Available actions depend on knowledge, profession and tool Q (Haven & Hearth model). | Decided |
| Living infrastructure | Coupled power grid, water and air; breaking a pole blacks out a sector. | Proposal (hero system) |
| One world, several lenses | Third person, tactical and management on the same world (Decided). Identical rules in all (Proposal). | Decided / Proposal |
| Single state per inhabitant | Inventory, exertion and pain persist across changes of control, camera or order. | Decided |
| Realism | Visual realism and fluid animation; readable physiology. | Decided |

## What makes it different

- **Living infrastructure in 3D.** 2D games (Oxygen Not Included, Space Station 13) do it; almost no 3D survival game does. Project Zomboid cuts power and water on a timer. Proposal.
- **Room graph.** Water, smoke, gas, sound and zombie scent travel between rooms through doors, stairs and drains. Proposal.
- **Quality that feeds physical systems.** A Q80 pipe holds more pressure than a Q30 one. Proposal.
- **A body that changes silhouette and clothing fit.** When you lose weight, your trousers get loose. Proposal.
- **Asymmetric co-op.** One player coordinates from management while others fight and loot. Decided as intent; deferred by the research (Proposal).

Demo scenario (Proposal): a grenade in the basement pump room breaks pipes. The water rises and flows down the stairs to the floor below. Blowing up a pole leaves the block dark. Repair it with a crafted part; its Q decides how long it lasts.

## Target player

Proposal: players of systemic survival games (Project Zomboid, Dwarf Fortress, RimWorld, Oxygen Not Included) who want that in 3D with direct action. They enjoy understanding causes and consequences more than following a script.

## Play mode

- Final goal is multiplayer. Starting single player is accepted, then co-op, then multiplayer. Decided.
- Start with one character, find survivors or bandits, form or join groups. Decided.
- Scale of inhabitants, zombies and players. Open.

## Scope: cut, defer, keep

Table from the engine research. **Proposal**: the user has not confirmed it yet.

| Layer | Decision | Why |
| --- | --- | --- |
| Third person + isometric on one character | Keep | Core identity |
| Graph power grid with sectors | Keep | Cheap, visible; pole demo |
| Water network + room-graph flooding | Keep | Grenade demo |
| Layered body + metabolism | Keep, reduced | Part tree and composition |
| Clothing that follows body morphs | Keep, now | Blocks every character asset |
| Disassembly + quality on ~30 items | Keep | Tests the H&H loop without a huge catalog |
| Commander / RTS mode | Defer | Needs multiplayer |
| City manager mode | Defer | Needs a whole city of content |
| Multiplayer | Defer | Multiplies the cost of every system |
| Fire, temperature, weather | Defer | Will sit on the room graph |
| Full cloth simulation | Defer | Only polish near the camera |
| Voxel destruction, world-scale fluids | Cut | Excessive cost for the gain |

First vertical slice (Proposal): one block with 3–4 buildings, one with a basement pump room; a substation and ~20 poles; one character with both cameras.

## Open questions

- Is the hero system the infrastructure or the body? Open.
- Co-op central from day one or later? Open (the user accepted starting single player).
- World size: a town, a district or a city? Open.
- Does the isometric view allow speeding up time? Open.
- Era, region, tone and level of graphic violence. Open.

## Prototype sources

- GDD_Survival_3D.md
- wiki/decisiones-experiencia-control.md
- wiki/decisiones-mundo-aprendizaje.md
- wiki/gameplay-cooperativo.md
- wiki/guia-de-arte.md
- wiki/bases-produccion-visual.md
- engine-research.txt (engine research, most recent)
