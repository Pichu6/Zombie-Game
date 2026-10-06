# Vision

**How long can you really survive?** Everyone thinks they would survive a zombie outbreak like in the movies. Real life has hundreds of things they ignore that would kill them: a scratch that gets infected, bad water, dehydration, no sleep, cold, a sprained ankle, carrying too much, the noise you make, food that spoils, panic. Each one is a body or world system, and each one is a believable way to die.

Under it runs **one outbreak simulation of the whole world**, real Earth by default. A virus starts somewhere, the news arrives, supply chains keep running until they break, and about 7 days later it reaches your town. You can watch and steer it from the globe (Macro), manage its local effects as a city (City), or live it as one person (Individual), in real time, as in Project Zomboid. Every person, yours or not, is a simulated human body under the same rules as you. The dead are not generic monsters: each zombie is a specific former person, with their body, wounds and clothes. Some used to be yours.

Labels: **Decided** (the user confirmed it), **Proposal** (not confirmed), **Open** (pending question). Full concept and its history: [concept v2](research/concept-v2.md). Decision records: [0010](decisions/0010-concept-v2.md), [0012](decisions/0012-world-simulation-layers.md). World layer design: [world simulation](design/world-simulation.md).

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
| One world, three layers | Macro (globe: watch, laws and edicts), City (isometric RTS over the local zone) and Individual (play one pop) are views over one simulation. A hand wound or a power plant exists once and shows in every layer. See [0012](decisions/0012-world-simulation-layers.md). | Decided |
| Play style chosen in setup | No main mode: micro → macro (start as one survivor, the XCOM-like commander layer opens with others), macro → micro, one layer only, or spectator. | Decided |
| Entangled and calibrated | Infection reaches people, jobs, supply chains and infrastructure. Outbreaks can fail; the default is calibrated on real pandemics and how people react. | Decided |
| Finite world | People, loot and zombies don't respawn. | Decided |
| Everything built is scavengable and craftable | Walls, floors, houses, pipe networks, power lines, car engines, vehicles, tools and items are all assemblies of real parts that can be scavenged and crafted, all tied by one quality (Q) system. Q comes from parts, capped by skill and tool. What you can see and do depends on job knowledge (electrician, plumber, mechanic). Everything takes real time (Haven & Hearth). Slow and hard, nothing easy; infrastructure fails unless pops maintain it. Salvage is easy; making engines, concrete or grid parts is capped by machinery, and top quality needs a whole chain running well, so manufacturing is late-game teamwork and trade. See [0010](decisions/0010-concept-v2.md), [0013](decisions/0013-production-ladder.md). | Decided |
| Living infrastructure | Power, water, light and noise as real networks that keep bodies alive and leak signals. | Proposal |
| Real behaviour | People talk by voice, radio and phone with real propagation. Options depend on knowledge, items and their quality. Every item and resource has quality, condition and states. Doors, walls and fortifications break part by part. | Decided |
| Single state per survivor | Pain, fear, load and job persist when you possess or release a body. | Decided |
| Realism | Visual realism and fluid animation; readable physiology. | Decided |

## How you play

- **Real time, no turns.** Decided.
- **Time scale:** 1 real hour per game day at 1× (1 game hour = 2.5 real minutes), plus 3×, 10× and rest-until-event. One global clock. Combat forces 1×. Events (alarm, attack, injury, breakdown, new symptom) drop to 1× and can pause; seeing a distant zombie does not. Also a sandbox setting. Decided (event rule: Proposal).
- **Command layer:** squad orders (go, hold, follow, attack, retreat) and a job board with priorities. Proposal.
- **Possession:** click a survivor to play them directly. Tier 0: direct WASD control; Tier 1: iso zoom-in; Tier 2: over-the-shoulder 3rd person. Camera and control code is mode-agnostic from day one. Possession gives control, not power: the body still sets speed, strength and stamina. Proposal.
- **Why possess:** the AI is cautious and average on purpose, and panics or refuses in specific situations (a dark basement, an open yard with dead in sight, a live wire). Proposal.
- **Setup:** create the world from parameters (real Earth by default; generated worlds in sandbox, where pops settle where conditions are best and towns, roads and trade follow), then the outbreak in a Plague Inc style virus editor with sliders (or no virus, which turns the game into city building, world management, politics and a Sims-like life), the scenario and the character. Decided ([0016](decisions/0016-sandbox-setup-and-layer-flow.md)).
- **Character:** pick a pop or create one, Project Zomboid style (civilian, police, military, nurse, custom). Starting weapons follow real custody rules by default. Every pop has a TTRPG-style lifepath behind them. Decided.
- **Warning week:** about 7 days from first news to arrival (realistic for the virus settings), all live play by default: work, buy, prepare, hear the news. Decided.
- **Fog:** playing an individual, the layers above show what that character believes, and a pop's dossier can be wrong. Decided.
- **Activities:** plenty of Shenmue-style hands-on activities in the Individual layer, each one a real action pops also do, automatic by default and simple when played by hand; a zombie's house key opens its real house. Decided ([0019](decisions/0019-activities-and-delegation.md), [0020](decisions/0020-activity-answers.md)); model in [activities](design/activities.md).
- **Layer flow:** a Macro order aimed at a sector (power plant in sector 3, blockade in sector 4) becomes a site in City, where pops gather, build and staff it on their own; click the plant, pick a worker and play as them. A possessed pop's tasks and routine become optional objectives. Decided ([0016](decisions/0016-sandbox-setup-and-layer-flow.md)).
- **Orders follow role:** an individual takes jobs at a power plant; only someone with authority orders one. A build order creates real jobs and material demand that pops answer, like zoning in Cities: Skylines. Decided.
- **Map:** the world is a sandbox too, with a zoom from the **globe** down to the **local zone** (Infection Free Zone style). Every building enterable; breachable walls. Timing decided (2026-10-05): hand-made map for Tier 0–1, a template-based generator in Tier 2, real-world globe pick in Tier 3; the global outbreak layer, being only numbers, can come early. The map data is designed from the start so a generator can fill it.
- **Sandbox settings** (Zomboid style): zombie speed, time scale, population, utility shutoff, loot, and **the virus itself** (Plague Inc style): transmission routes and their chances, incubation time, symptoms, lethality, time from death to reactivation, and whether a cure exists. Presets: a realistic default ("necrobiotic syndrome", see [threats](design/threats.md)) plus faster or harsher variants. Decided (2026-10-05). **Settings change numbers, never rules**, so player and survivor parity always holds. Decided.
- **Zombie speed default:** body-driven (mostly slow; fresh lean bodies faster; fat or broken ones slower). Options: always slow, body-driven, fast. Zombies don't tire by default. Decided.
- **Origins:** selectable, Plague Inc style. Origins that change rules, not only numbers, are DLC. Decided.
- **Multiplayer:** the end goal; single player first, then co-op, then multiplayer. Decided.

## Setting and scale

- **Era:** present day, modern world. Decided (2026-10-05).
- **World:** real Earth by default; the local zone is a real region. Decided (2026-10-05).
- **Local scale:** it must *feel* big enough for big-picture play, without being a 1:1 real city. Decided as intent; numbers are a Proposal: the final target is one small town, compressed (real building sizes, fewer of them), roughly a few hundred buildings and a few thousand residents in the census. Each tier grows the map: a block (Tier 0), a few buildings (Tier 1), a neighbourhood (Tier 2), the town (Tier 3).
- **Three layers** (supersedes "three lenses" in [0011](decisions/0011-setting-scale-lenses.md)): Macro, City and Individual, with isometric and over-the-shoulder cameras in Individual. Taking a different body uses a GTA V-style switch that hides the loading. Decided (switch style: Proposal). Tiers: iso in Tier 0–1, City grows with map size, 3rd person in Tier 2.

## Scope by tier

Nothing is deleted; every idea is scheduled. Details inside each tier are Proposals.

| Tier | What | Order |
| --- | --- | --- |
| **0 · "The generator problem"** | 2D simulation, SDL3 + ImGui, no Vulkan. One 2-floor building with a basement, yard and 2 houses. Start with 1 survivor; find 3 more. Power, noise, light (water as stretch). Body in kg, ~17 regions with wounds, needs, weekly drift. 6 jobs, squad orders, direct control. Zombies made from dead survivors and a cold census of ~60 neighbours; hidden incubation. Goal: survive 14 days. | First. **Decided:** before any renderer work |
| **1 · "One building, five bodies"** | Tier 0 sim in 3D grey box: Vulkan renderer, iso cutaways, floor switching, one camera rig, skinned mannequins with 3 morph axes, Jolt collision, power/water/noise/light/smoke, ~30 items, 3–6 survivors, ~300 census, 2–3 hand-made buildings. | After Tier 0 |
| **2 · "The neighbourhood"** | 3rd-person possession, visible body change, long infection drama, recruitment and relationships, a district with a sewer, floods/fire/collapse, ~150 items, seasons. | — |
| **3 · "The town"** | Co-op (commander + possessed players), photoreal bodies, physics-based movement, vehicles, thousands of persistent lives. | — |

## Inspirations

Named by the user (2026-10-05), "and plenty more": Haven & Hearth, Project Zomboid, Infection Free Zone, Cities: Skylines, The Sims, Thunder Tier One, Spark in the Dark, Ghost Recon Breakpoint, GTA, The Last of Us, XCOM, Plague Inc.

## Target player

Proposal: players of systemic survival games (Project Zomboid, RimWorld, Dwarf Fortress, XCOM) who want to understand causes and consequences more than follow a script.

## Open questions

Earlier questions were answered in [0014](decisions/0014-open-questions-answered.md), [0015](decisions/0015-tone-realistic-damage.md), [0018](decisions/0018-sandbox-answers.md), [0019](decisions/0019-activities-and-delegation.md) and [0020](decisions/0020-activity-answers.md).

1. Local zone: a cropped piece of the real town, or the whole town with filler buildings removed? Recommended: cropped, thinning only filler if needed, never moving key buildings.
2. Bodies keep real walking speed while the calendar runs 24×? Recommended: yes, and size the zone for it.
3. One XCOM-style collapse-pressure bar per region on the globe? Recommended: yes, computed from the simulation's forecast, fogged while playing an individual.

Details: [world simulation](design/world-simulation.md) section 8 (scale across layers).

Also still open: ceiling values per production method, and how many production steps to model ([production ladder](design/items-crafting.md#production-ladder)).
