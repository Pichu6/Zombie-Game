# Juego Zombie · Concept v2

**Date:** 2026-10-05. **Status:** research note, not a decision; its direction was decided by Pichu on 2026-10-05 (sections 8 and 9, recorded in [0010](../decisions/0010-concept-v2.md)), and where section 9 differs from sections 1-7, section 9 wins; details inside the tiers are still proposals. **Feeds:** [0010 concept v2](../decisions/0010-concept-v2.md), [0012 world simulation layers](../decisions/0012-world-simulation-layers.md), [world simulation](../design/world-simulation.md), [infrastructure](../design/infrastructure.md), [threats](../design/threats.md), [cameras and modes](../design/cameras-modes.md), [spec 24](../tier0/specs/24-pop-ai.md), [spec 25](../tier0/specs/25-chronicle.md), [spec 26](../tier0/specs/26-run.md). **Evidence:** the result of a proposer/critic refinement loop (working files kept outside the repo, in the project files under `research/concept/loop/`), built on Pichu's own words ([concept-pichu-original-vision.md](concept-pichu-original-vision.md)); no outside sources were read. Findings marked sourced are Pichu's own answers; the rest are the loop's proposals. **IDs:** CV.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CV-1 | The hero is the player's own survivor, as in Project Zomboid; the game asks "how long can you really survive?" and its content is the realistic ways to die that movies and games ignore | sourced | Pichu, 2026-10-05 (§8.1, §9) |
| CV-2 | Start alone; an XCOM-like commander layer unlocks when you find or recruit other survivors, and you can drop into any of them to play directly | sourced | Pichu, 2026-10-05 (§8.2, §9) |
| CV-3 | Real time, no turns | sourced | Pichu, 2026-10-05 (§8.2) |
| CV-4 | Default time scale: 1 real hour per game day at 1x (1 game hour = 2.5 real minutes), with 3x (20 min/day), 10x (6 min/day) and rest-until-event (about 1-2 min/day); also a sandbox setting | sourced | Pichu, 2026-10-05 (§8.3); speeds from §3 |
| CV-5 | Map: small and hand-made, every building enterable, breachable walls; Tier 0 is one building plus 2 houses; seeded variants wait for Tier 3 | sourced | Pichu, 2026-10-05 (§8.4) |
| CV-6 | Tier 0 is a 2D simulation (SDL3 + ImGui, no Vulkan) that must answer the fun questions before any renderer work | sourced | Pichu, 2026-10-05 (§8.5) |
| CV-7 | Zombie speed is a sandbox setting (always slow, body-driven, fast); default body-driven: mostly slow, fresh lean bodies faster, fat or broken ones slower | sourced | Pichu, 2026-10-05 (§8.6) |
| CV-8 | Parity rule: every pop can survive exactly like the player and the player can do everything any pop can; one body model, one action set, one set of survival rules; the AI only chooses actions | sourced | Pichu, 2026-10-05 (§9) |
| CV-9 | Sandbox settings change numbers, never rules, so player-pop parity always holds | sourced | Pichu, 2026-10-05 (§9) |
| CV-10 | Everything built (walls, floors, houses, pipes, power lines, engines, vehicles, tools, items) can be scavenged into real parts and crafted, tied by one quality system; crafting and repair depend on job knowledge and skill | sourced | Pichu, 2026-10-05 (§9) |
| CV-11 | Consequence of CV-10: one assembly data model for every built thing, from a screw to a building; structures are assemblies, not static level geometry | estimate | this note (§9) |
| CV-12 | Consequence of CV-8: every pop runs the full survival sim, so simulation LOD is essential (full detail near the player, cheaper scheduled updates far away, same rules and conserved quantities) | estimate | this note (§9) |
| CV-13 | The core choice, repeated every few minutes: comfort and progress versus being noticed, paid for with fragile bodies; every system feeds it | estimate | this note (§2) |
| CV-14 | Every light, engine and hammer blow leaks signals (noise, light, smell) that draw the dead; example: a generator makes 70 dB | estimate | this note (§2) |
| CV-15 | Each zombie is a specific former person who keeps their body record, wounds and clothes (a broken leg makes a crawler); a body that dies infected comes back | estimate | this note (§1, §4) |
| CV-16 | Pressure arc so "survive N days" is not flat: fuel runs short around day 10, waves grow with the noise made, hidden infection breaks out inside the group, and a goal (repair the radio) needs power, which means noise | estimate | this note (§2) |
| CV-17 | Possession gives control, not power: the player brings timing, aim and judgement; the body still sets speed, strength and stamina | estimate | this note (§3) |
| CV-18 | The AI is cautious and average on purpose and panics or refuses in specific situations (dark basement, open yard with dead in sight, cutting a live wire); possessing is how you get through them | estimate | this note (§3) |
| CV-19 | Unpossessing never resets anything: the pop keeps its pain, fear, load and job and the AI takes over | estimate | this note (§3) |
| CV-20 | One global clock; possession does not lock the game to 1x, combat does; slow down on events (alarm, attack, injury, breakdown, new symptom), not on perception (the RimWorld rule) | estimate | this note (§3) |
| CV-21 | Haven & Hearth slowness is felt in game hours compressed by delegation, not in real minutes of waiting; a 6-hour repair is 15 min at 1x, 5 min at 3x, 45 s at 10x | derived | this note (from CV-4) |
| CV-22 | Tier 0 contents: 4 pops (body in kg of fat, muscle and structure, fitness, about 17 body regions with wounds and bleeding), 6 jobs (haul, refuel, repair, cook, guard, barricade), squad orders, power with about 10 receivers, noise and light, a cold census of about 60 neighbours, hidden incubation; goal: survive 14 days | estimate | this note (§5) |
| CV-23 | After Pichu's answers, Tier 0 starts with 1 survivor and the other 3 pops are found during the run (a neighbour, a stranger who may be infected) | sourced | Pichu, 2026-10-05 (§9) |
| CV-24 | New Tier 0 fun question: dying must feel fair and taught; after a death the player should know which ignored thing killed them | sourced | Pichu, 2026-10-05 (§9) |
| CV-25 | Test of realism: run the sim with no player; if AI pops die of the same ignored things a careless player does, the realism works; the AI must survive alone for 14 days with player actions only | estimate | this note (§9) |
| CV-26 | Catalogue per tier: Tier 0 about 20 items, Tier 1 about 30 item families plus pipes and wiring, Tier 2 about 150 items, Tier 3 vehicles and engines | estimate | this note (§4, §9) |
| CV-27 | "Fluids" in Tiers 0-2 means a water level per room with simple visuals, not visibly flowing water; off-screen physics is frozen | estimate | this note (§4) |
| CV-28 | Biggest hidden costs of Tier 1 are pop AI (job bugs), command UI and integration/tuning, about 25% of the total | estimate | this note (§5) |
| CV-29 | Every simulated value needs a visible tell (overlay, status icon or log line), or the sim feels random | estimate | this note (§6) |
| CV-30 | Inspirations named by Pichu: Haven & Hearth, Project Zomboid, Infection Free Zone, Cities: Skylines, The Sims, Thunder Tier One, Spark in the Dark, Ghost Recon Breakpoint, GTA, The Last of Us, XCOM, Plague Inc. | sourced | Pichu, 2026-10-05 (§9) |

## 1. The game in one paragraph

A zombie survival game where you lead a small group of survivors holding one building in a dying neighborhood. Every person is a simulated human body: fat, lean, strong, fit, tired, hungry, wounded. Those facts decide how fast they run, how much they carry and how loud they are. You command the group like an RTS or colony game: jobs, orders, squads. When it matters, you step into one of them, Dungeon Keeper style, and play that person directly. The building runs on real networks (power, water, light, noise) that keep the bodies alive, and each one leaks signals that draw the dead. The dead are not generic monsters. Each zombie is a specific former person, with their body, their wounds and their clothes. Some of them used to be yours. Everything takes real time, as in Haven & Hearth, so the hours of your people are the main thing you spend.

**The fantasy in one sentence:** *keep a handful of fragile, real human bodies alive by deciding how much comfort you can afford, knowing every light, engine and hammer blow tells the dead where you are.*

## 2. The core choice, minute to minute

One decision repeats every few minutes: **comfort and progress vs being noticed, paid for with fragile bodies.**

- Run the generator tonight so the fridge stays cold and Ana can finish the radio? It makes 70 dB, and the block's dead will come.
- Light the workshop so the fine repair goes faster? The window leaks light into the street.
- Send Marco to scavenge fuel? He is the fastest runner, but he is also the leanest, he is starving first, and he has an open cut.
- Hammer barricades now, which is loud, or quietly at dawn, which leaves you weaker tonight?

Every system feeds that one choice:

| System | What it gives you (comfort) | What it costs (noticed / bodies) |
| --- | --- | --- |
| **Body** | Strong bodies carry more, fit bodies run further | Bodies need food, sleep and warmth. Wounds slow them, and every one you lose is permanent |
| **Infrastructure** | Cold food, clean water, light to work, heat | Noise, light and smell leak out. Networks break at night |
| **Pops** | Hands for jobs. Specialists get better with practice | Each one eats. Each one can be hiding an infection |
| **Crafting (H&H time)** | Better tools and repairs, stronger barricades | Hours of someone's day. Work is often loud and needs light |
| **Zombies** | (none, they are the price) | Drawn by signals. Each is a former person, with their weight and wounds |

```
   bodies spend HOURS ─────────► infrastructure + crafting
        ▲                              │ (power, light, water, tools)
        │ food, water, heat, rest      │
        │                              ▼  leaks SIGNALS (noise, light, smell)
   bodies stay alive ◄── fights, wounds, infection ◄── the dead gather
        │                                                  ▲
        └──── a body that dies infected comes back ────────┘
```

**The pressure arc.** "Survive N days" alone gives a flat middle, so each run gets an arc. Fuel runs short around day 10. Waves grow with the noise you have made. Hidden infection breaks out inside the group. A goal (repair the radio and call for rescue) needs power, and power means noise.

## 3. How the player plays

**Main identity: the commander who can become any one of them.** You spend most of your time above the group and drop into a body for the moments that matter.

- **Command (RTS layer).** High isometric view. You have two tools, kept side by side:
  - **Squad orders**, as in an RTS: go there, hold, follow, attack, retreat. Select one or several pops, then right-click.
  - **Job board / day plan**, as in a colony game: who refuels, who cooks, who guards, who repairs, with priorities.
- **Possession, step 1 (Tier 0–1): iso zoom.** Click a portrait and the same camera zooms in on that pop. You move them with WASD and do the action with the mouse. There is no second camera to build.
- **Possession, step 2 (Tier 2): 3rd person.** Over the shoulder, as in The Last of Us or Ghost Recon, on the same pop with the same rules. Camera and control code is written mode-agnostic from day one so this can be added without a rewrite.
- **Possession gives control, not power.** You bring timing, aim and judgement. The body still sets speed, strength and stamina.
- **Why you possess.** The AI is cautious and average on purpose. It also **panics or refuses** in specific situations: a dark basement, crossing an open yard with dead in sight, cutting a live wire. Possessing is how you get through those. Routine work is better left to the AI.
- **Unpossessing never resets anything.** The pop keeps its pain, fear, load and job, and the AI takes over.

**Time scale (proposal):**

| Speed | Real time per game day | Example: a 6-hour repair |
| --- | --- | --- |
| 1× | 60 min (1 game hour = 2.5 real min) | 15 min |
| 3× | 20 min | 5 min |
| 10× | 6 min | 45 s |
| "Rest until event" (night, everyone safe) | ~1–2 min | — |

- There is **one global clock**. Possession does not lock the game to 1×. Combat does.
- **Slow-down on events, not on perception** (the RimWorld rule). An alarm, an attack, an injury, a breakdown, a new symptom or a scavenger in trouble drops the game to 1× and pauses it (the pause is optional in settings). After that the player can speed up again. A zombie that is merely visible in the distance does not slow the game.
- H&H slowness is felt in **game hours compressed by delegation**, not in real minutes of waiting. You plan long jobs, then speed through them.

## 4. Every idea Pichu had, and where it lives

Tier 0 = 2D prototype. Tier 1 = first 3D playable. Tier 2 = the neighborhood. Tier 3 = the dream. **Nothing is deleted**, only scheduled.

| Pichu's idea | Tier 0 (2D prototype) | Tier 1 (3D first playable) | Tier 2 | Tier 3 |
| --- | --- | --- | --- | --- |
| RTS: command troops, give orders | Squad orders + 6 jobs | + day plan, priorities, zones | Larger groups, expeditions | City-manager view over districts |
| Dungeon Keeper possession | Direct WASD control of any pop | Iso zoom-in possession | Trust between pops and leader affects who obeys | Co-op: one commander, others possessed |
| Isometric (Zomboid, Diablo) | Top-down 2D | Iso 3D grey box, cutaways, floor switching | Final art | — |
| 3rd person (TLOU, Ghost Recon, GTA) | — (code kept mode-agnostic) | — | Over-the-shoulder, peeking, cover | Polished |
| Every NPC a pop (Sims) | 4 pops with needs + simple mood | 3–6 pops, household data stored | Recruitment, relationships, complaints, bandits, trade | Thousands of persistent lives with stories |
| Every NPC a pop (SimCity census) | Cold list of ~60 neighbours who die and return | ~300-person census for the block | Warm neighbours who are alive and act | Town census, outbreak origin |
| Body: fat, slim, muscled, fit, traits | kg of fat/muscle, fitness. Traits shown as labels | Same + 3 visual morph axes (fat, muscle, height) | Full tissue layers, surgery | Photoreal bodies, organs |
| Bodies that change | **Slow weekly drift** from diet and work | Same, visible on morphs | Training, atrophy, stomach | — |
| Zombie = infected human body | Dead pop keeps its record and gets a walker brain. Inherits wounds (broken leg → crawler) | Same, 3D, inherits clothes and gear | Long hidden incubation, misdiagnosis drama. Zombies decay over weeks | Swappable threat types |
| Infected friend inside the group | **Hidden incubation timer + symptoms** | Same | Full prodrome drama | — |
| Body drives movement/animation | Speed, carry and noise come from the body | Stride, limp, crawl, load posture | Capability-driven animation + IK | Physics-based movement limited by real joints |
| Grids (power, water, others) | Power + noise + light. Water as a stretch | Power, water, noise, light, smoke | Substations, sector blackouts, rerouting | Rebuild power from scratch |
| Fluids | Water level per room (a number) | Same + simple visuals | Floods, freezing pipes, CO, electrified water | Richer visuals |
| Physics | Simple collision | Jolt: collision, doors | Ragdolls, dropped items, structural collapse | Vehicles, active ragdoll |
| Map not fake: walls you can break | Breach, barricade, cell grid | Same in 3D, multi-floor | Collapse, building new structures | Town scale |
| Zomboid map: fixed, all buildings enterable | Hand-made building + 2 houses, all enterable | 2–3 hand-made buildings + simple editor | Hand-made district | Seeded variants (optional) |
| H&H: everything takes time | Real durations, resumable jobs | ~30 items, quality from parts | ~150 items, learning by study, mentors | Generational play, crops |

**Said plainly:** "fluids" in Tiers 0–2 means water levels per room, shown with simple visuals. It does not mean visibly flowing water. "Physics" off-screen is frozen. That is how one person can afford it.

## 5. The tiers

**Fact first: no engine code exists yet.** The repo today holds docs and decisions only. Every estimate below includes building the engine.

### Tier 0: "The generator problem" (2D, SDL3 + ImGui, no Vulkan)

Exact contents:
- **Map:** one hand-made 2-floor building with a basement, a yard and 2 neighbour houses. Cell grid with walls, doors, windows, barricades and breachable walls.
- **Room graph:** power (generator, fuel, breakers, ~10 receivers), noise, light leak through windows. Water (tank, leak into basement) is a stretch goal.
- **4 pops:** fat/muscle/structure in kg, fitness, ~17 body regions with wounds and bleeding, hunger/sleep/energy. Capabilities set speed, carry and noise. Slow weekly body drift. Simple mood. Panic/refusal in 3 situations.
- **6 jobs:** haul, refuel, repair, cook, guard, barricade. **Squad orders:** go, hold, follow, attack.
- **Direct control** of any pop with WASD (the stand-in for possession).
- **Zombies:** dead pops with a walker brain, drawn by noise and light. A cold census of ~60 neighbours who die and come back on a schedule. Hidden incubation (timer + symptoms).
- **Time:** the scale from section 3, with event-based slow-down.
- **Tooling:** deterministic simulation, save/load, event log, per-pop inspector, overlays for noise, light and power.
- **Goal:** survive 14 days with a fuel crunch, noise-driven waves and a radio to repair.

**Questions it must answer** (after 5+ full runs, ideally some by other players):
1. Does the comfort-vs-noticed choice come up and feel interesting **every 5–10 minutes**?
2. Do players use direct control, and when? Do the panic/refusal moments make it feel needed rather than a chore?
3. Does the infected friend land emotionally? Do players recognise the zombie as "Ana"?
4. Does sped-up time feel like play or like waiting?
5. Can the player always tell **why** something happened (who starved, why the horde came)?
6. Do different bodies (fat hauler, lean scout) lead to different decisions?

If an answer is no, change the design **before** writing a renderer. The sim code is kept for Tier 1.

### Tier 1: "One building, five bodies" (after Tier 0)

This is the Tier 0 sim in 3D grey box. It adds a Vulkan renderer with iso cutaways and floor switching, one camera rig (zoom out for command, zoom in for possession), skinned mannequins with 3 morph axes, pathfinding that updates when walls are breached, Jolt collision, power + water + noise + light + smoke, ~30 items, 3–6 pops, a census of ~300, two simulation detail levels (near and far) and 2–3 hand-made buildings with a minimal building editor. It has no warm neighbours, no trust system and no seeding. The biggest hidden costs are pop AI (job bugs), command UI and integration/tuning, which takes about 25% of the total.

### Tier 2: "The neighborhood" (brief)
3rd-person possession, bodies that change visibly, long infection drama, recruitment and relationships, warm neighbours, a district with a sewer, floods/fire/collapse, ~150 items, seasons.

### Tier 3: "The town" (dream, brief)
Co-op (commander + possessed players), photoreal bodies, physics-based movement, town scale with vehicles, thousands of persistent lives, rebuilding civilisation.

## 6. Top risks and mitigations

| Risk | Mitigation |
| --- | --- |
| The engine does not exist, so renderer work could crowd out the game | Tier 0 is pure sim in 2D. The renderer starts only after Tier 0 passes |
| No clear fun loop (watching bars move) | One core choice (section 2), a pressure arc, and the Tier 0 kill/keep questions |
| Possession gets ignored (as in Dungeon Keeper) or becomes a chore | AI panic/refusal moments, risky phases where timing matters, measured in Tier 0 |
| Sped-up time keeps dropping back to 1× | Slow down on events, not on perception. The player can speed up again |
| Pop AI bugs (hauling, reservations, interrupts) | Few jobs, data-driven phases, deterministic replay from Tier 0 |
| The sim feels random because its state is invisible | Every simulated value gets a visible tell: overlay, status icon or log line. UI budget is planned |
| Too much content (animations, sounds) for one person + AI | Placeholder art that still reads clearly. Jobs share a small pose set |
| Multi-floor iso visibility | Cutaways and floor switching are designed in Tier 1, not bolted on later |
| AI agents drift apart in conventions | Strong interfaces, tests and decision docs before parallel work |
| Hard to debug an emergent sim | Determinism, event log, inspectors and time scrub from Tier 0 |

## 7. Where we disagreed

The critique was right on most points, and this concept accepts them: that the old plan was far too big, Tier 0 first, cutting trust gating, warm neighbours, the belief map and seeding from Tier 1, keeping hidden infection and body drift early, adding direct squad orders, and slow-down on events. Here is where we pushed back to protect Pichu's vision:

- **Possession is not optional flavour.** The critic warned that Dungeon Keeper players mostly ignored possession. It is half of Pichu's fantasy, so it is in Tier 0 from day one and gets measured, not trimmed. Also, possession is not only a fix for AI failure. It is also the reward: the moment you feel a body's limits from the inside.
- **The body is the heart, not a stat sheet.** The critic suggested "the group" as the hero. We agree and phrase it as **a group of real bodies**. The body model stays the single shared data source for pop, zombie and movement, even in 2D.
- **Fluids stay visible in the plan.** The critic made water optional. Pichu named fluids, so water is a Tier 0 stretch goal and certain in Tier 1. It also feeds the core choice, because the pump is loud.
- **Commander first, but 3rd person is not "later, maybe".** It is a committed Tier 2 feature, and the code stays mode-agnostic to keep that promise cheap.

## 8. Decisions (all answered by Pichu on 2026-10-05)

1. **What is the hero?** **Decided by Pichu (2026-10-05):** the player's own survivor, as in Project Zomboid. The question the game asks is "how long can you really survive?" Everyone thinks they would survive like in the movies or games, but real life has hundreds of things they ignore that would kill them. See "Revision after Pichu's answers" below.
2. **Main identity?** **Decided by Pichu (2026-10-05):** a commander who can play as the units, "like XCOM in that sense". Read as: base/group management above, and dropping into named, permadeath-style units for the moments that matter. Real time, no turns (confirmed by Pichu).
3. **Time scale?** **Decided by Pichu (2026-10-05):** default 1 real hour per game day at 1× (1 game hour = 2.5 real minutes), with 3×, 10× and rest-until-event; also a sandbox setting.
4. **Map?** **Decided (Pichu: "something doable", 2026-10-05):** a small hand-made map where every building can be entered, with breachable walls. Tier 0: one building + 2 houses. Seeded variants stay in Tier 3.
5. **Tier 0 before any renderer work?** **Decided by Pichu (2026-10-05):** yes. A 2D simulation (SDL3 + ImGui, no Vulkan) with the kill/keep questions, before any renderer work.
6. **Zombie speed?** **Decided by Pichu (2026-10-05):** a sandbox setting, Zomboid style. Default: body-driven (mostly slow; fresh lean bodies faster; fat or broken ones slower). Options: always slow, body-driven, fast.

## 9. Revision after Pichu's answers (2026-10-05)

Pichu's answers move the centre of the game from "the group" to **one survivor**, with command growing out of it.

- **The pitch:** *how long can you really survive?* The game is about the hundreds of things movies and games ignore: a scratch that gets infected, bad water, dehydration, no sleep, cold, a sprained ankle, carrying too much, the noise you make, food that spoils, panic. Each one is a body or world system, and each one is a believable way to die.
- **Start alone.** You play one survivor from minute one, in real time, Zomboid style. The commander layer (XCOM-like) unlocks when you find or recruit other survivors (decision 0005 already says: start with one character, then form or join groups).
- **What changes in Tier 0:** the run starts with 1 survivor. The other 3 pops are found during the run (a neighbour, a stranger who may be infected), so Tier 0 tests both "survive alone" and "the first time you command someone". The survival checklist of realistic threats becomes the main content list for Tier 0 and Tier 1.
- **What stays:** the body as the core data, comfort versus being noticed, zombies made from former people, the building as the stage, all tiers.
- **New Tier 0 fun question:** does dying feel fair and *taught*? After a death, the player should know which ignored thing killed them ("you died of an infected cut you never cleaned").
- **Rule, decided by Pichu (2026-10-05): parity between player and pops.** Every pop must be able to survive exactly like the player, and the player can do everything any pop can. One body model, one action set, one set of survival rules; the AI only chooses what to do, the player chooses for the body they control. This confirms decision 0005 ("same rules for everyone").
  - *Consequence for the engine:* every pop runs the full survival sim (body, needs, wounds, infection), so the sim LOD from section 4 becomes essential: full detail near the player, cheaper scheduled updates far away, with the same rules and conserved quantities.
  - *Consequence for Tier 0:* the AI must be able to survive alone for 14 days using only player actions. A useful test: run the sim with no player and see how long AI pops last. If they die of the same ignored things a careless player does, the realism works.
- **Sandbox settings (Pichu, 2026-10-05).** Like Project Zomboid, key world rules are player settings rather than fixed design: zombie speed, time scale, and later others (population, infection timing, utility shutoff, loot). Defaults follow the realistic recommendations above. Design rule: settings change numbers, never the rules, so player and pop parity always holds.
- **Rule, decided by Pichu (2026-10-05): everything built is scavengable and craftable, tied by quality.** Everything built in the game (walls, floors, houses, doors, furniture, pipe networks, power lines, car engines, vehicles, tools, items) can be scavenged (dismantled into real parts) and crafted, and all of it shares one quality (Q) system. This generalises the earlier infrastructure "golden rule" (every grid element can be repaired or built) to the whole world. Crafting and repair depend on job-related knowledge and skill (Haven & Hearth model), so professions matter: an electrician, a plumber and a mechanic see and do different things.
  - *Consequence for the engine:* one assembly data model for every built thing, from a screw to a building. Structures (walls, floors) are assemblies too, not static level geometry.
  - *Scope per tier:* the rule holds from Tier 0, the catalog grows by tier. Tier 0: walls, doors, windows, barricades, the generator and its parts, ~20 items. Tier 1: ~30 item families plus pipes and wiring. Tier 2: ~150 items, building new structures. Tier 3: vehicles and engines.
- **Inspirations named by Pichu (2026-10-05):** Haven & Hearth, Project Zomboid, Infection Free Zone, Cities: Skylines, The Sims, Thunder Tier One, Spark in the Dark, Ghost Recon Breakpoint, GTA, The Last of Us, XCOM, Plague Inc. ("and plenty more").
