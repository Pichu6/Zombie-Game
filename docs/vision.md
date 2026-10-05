# Vision

**How long can you really survive?** Everyone thinks they would survive a zombie outbreak like in the movies. Real life has hundreds of things they ignore that would kill them: a scratch that gets infected, bad water, dehydration, no sleep, cold, a sprained ankle, carrying too much, the noise you make, food that spoils, panic. Each one is a body or world system, and each one is a believable way to die.

You play one survivor in real time, as in Project Zomboid. When you find or recruit others, a commander layer opens up, like XCOM: you manage the group and the building from above, and drop into any survivor to play them directly. Every person, yours or not, is a simulated human body under the same rules as you. The dead are not generic monsters: each zombie is a specific former person, with their body, wounds and clothes. Some used to be yours.

Labels: **Decided** (the user confirmed it), **Proposal** (not confirmed), **Open** (pending question). Full concept and its history: [concept v2](research/concept-v2.md). Decision record: [0010](decisions/0010-concept-v2.md).

## The core choice

One decision repeats every few minutes: **comfort and progress versus being noticed, paid for with fragile bodies.** Run the generator so the fridge stays cold? It makes 70 dB and the block's dead will come. Light the workshop so the repair goes faster? The window leaks light into the street. Hammer barricades now, loudly, or quietly at dawn? (Proposal)

| System | What it gives you | What it costs |
| --- | --- | --- |
| Body | Strong bodies carry more, fit bodies run further | Food, sleep and warmth. Wounds slow you. Every loss is permanent |
| Infrastructure | Cold food, clean water, light, heat | Noise, light and smell leak out. Networks break |
| Other survivors | Hands for jobs; specialists improve with practice | Each one eats. Each one may hide an infection |
| Crafting (H&H time) | Better tools, repairs and barricades | Hours of someone's day. Work is often loud and needs light |
| Zombies | Nothing: they are the price | Drawn by signals. Each is a former person |

**Pressure arc** (Proposal): fuel runs short around day 10, waves grow with the noise you made, hidden infection breaks out inside the group, and the goal (repair the radio, call for rescue) needs power, which means noise.

## Pillars

| Pillar | What it means | Status |
| --- | --- | --- |
| Realistic survival | The content is the realistic ways to die that other games ignore. After a death you know which ignored thing killed you. | Decided |
| One body model | Player, every survivor and every zombie share one body model, one action set and one set of survival rules. The AI only chooses what to do. | Decided |
| Survivor first, commander later | Start alone in real time; the XCOM-like commander layer unlocks with other survivors. | Decided |
| Finite world | People, loot and zombies don't respawn. | Decided |
| Everything built is scavengable and craftable | Walls, floors, houses, pipe networks, power lines, car engines, vehicles, tools and items are all assemblies of real parts that can be scavenged and crafted, all tied by one quality (Q) system. Q comes from parts, capped by skill and tool. What you can see and do depends on job knowledge (electrician, plumber, mechanic). Everything takes real time (Haven & Hearth). See [0010](decisions/0010-concept-v2.md). | Decided |
| Living infrastructure | Power, water, light and noise as real networks that keep bodies alive and leak signals. | Proposal |
| Single state per survivor | Pain, fear, load and job persist when you possess or release a body. | Decided |
| Realism | Visual realism and fluid animation; readable physiology. | Decided |

## How you play

- **Real time, no turns.** Decided.
- **Time scale:** 1 real hour per game day at 1× (1 game hour = 2.5 real minutes), plus 3×, 10× and rest-until-event. One global clock. Combat forces 1×. Events (alarm, attack, injury, breakdown, new symptom) drop to 1× and can pause; seeing a distant zombie does not. Also a sandbox setting. Decided (event rule: Proposal).
- **Command layer:** squad orders (go, hold, follow, attack, retreat) and a job board with priorities. Proposal.
- **Possession:** click a survivor to play them directly. Tier 0: direct WASD control; Tier 1: iso zoom-in; Tier 2: over-the-shoulder 3rd person. Camera and control code is mode-agnostic from day one. Possession gives control, not power: the body still sets speed, strength and stamina. Proposal.
- **Why possess:** the AI is cautious and average on purpose, and panics or refuses in specific situations (a dark basement, an open yard with dead in sight, a live wire). Proposal.
- **Map:** small and hand-made; every building enterable; breachable walls. Decided.
- **Sandbox settings** (Zomboid style): zombie speed, time scale, population, utility shutoff, loot, and **the virus itself** (Plague Inc style): transmission routes and their chances, incubation time, symptoms, lethality, time from death to reactivation, and whether a cure exists. Presets: a realistic default ("necrobiotic syndrome", see [threats](design/threats.md)) plus faster or harsher variants. Decided (2026-10-05). **Settings change numbers, never rules**, so player and survivor parity always holds. Decided.
- **Zombie speed default:** body-driven (mostly slow; fresh lean bodies faster; fat or broken ones slower). Options: always slow, body-driven, fast. Decided.
- **Multiplayer:** the end goal; single player first, then co-op, then multiplayer. Decided.

## Setting and scale

- **Era:** present day, modern world. Decided (2026-10-05).
- **Scale:** it must *feel* big enough for big-picture play, without being a 1:1 real city. Decided as intent; numbers are a Proposal: the final target is one small town, compressed (real building sizes, fewer of them), roughly a few hundred buildings and a few thousand residents in the census. Each tier grows the map: a block (Tier 0), a few buildings (Tier 1), a neighbourhood (Tier 2), the town (Tier 3).
- **One zoom, three lenses:** the player moves continuously from **manager** (big picture: the town or district map, census, sectors, group and base, as in Infection Free Zone or Cities: Skylines) through **isometric** (tactical, Zomboid style) to **immersion** (2nd/3rd person, as in Ghost Recon or The Last of Us), on the same world and the same survivor. Decided (2026-10-05). Tiers: iso in Tier 0–1, manager view grows with map size, 3rd person in Tier 2.

## Scope by tier

Nothing is deleted; every idea is scheduled. Details inside each tier are Proposals.

| Tier | What | Size |
| --- | --- | --- |
| **0 · "The generator problem"** | 2D simulation, SDL3 + ImGui, no Vulkan. One 2-floor building with a basement, yard and 2 houses. Start with 1 survivor; find 3 more. Power, noise, light (water as stretch). Body in kg, ~17 regions with wounds, needs, weekly drift. 6 jobs, squad orders, direct control. Zombies made from dead survivors and a cold census of ~60 neighbours; hidden incubation. Goal: survive 14 days. | ~3 months. **Decided:** before any renderer work |
| **1 · "One building, five bodies"** | Tier 0 sim in 3D grey box: Vulkan renderer, iso cutaways, floor switching, one camera rig, skinned mannequins with 3 morph axes, Jolt collision, power/water/noise/light/smoke, ~30 items, 3–6 survivors, ~300 census, 2–3 hand-made buildings. | ~20–30 person-months |
| **2 · "The neighbourhood"** | 3rd-person possession, visible body change, long infection drama, recruitment and relationships, a district with a sewer, floods/fire/collapse, ~150 items, seasons. | — |
| **3 · "The town"** | Co-op (commander + possessed players), photoreal bodies, physics-based movement, vehicles, thousands of persistent lives. | — |

## Inspirations

Named by the user (2026-10-05), "and plenty more": Haven & Hearth, Project Zomboid, Infection Free Zone, Cities: Skylines, The Sims, Thunder Tier One, Spark in the Dark, Ghost Recon Breakpoint, GTA, The Last of Us, XCOM, Plague Inc.

## Target player

Proposal: players of systemic survival games (Project Zomboid, RimWorld, Dwarf Fortress, XCOM) who want to understand causes and consequences more than follow a script.

## Open questions

- Region: a real place (like Infection Free Zone's real maps) or a realistic fictional town modelled on a real region? Recommendation: fictional town modelled on a real region, which fits the hand-made map.
- Tone and level of graphic violence.
