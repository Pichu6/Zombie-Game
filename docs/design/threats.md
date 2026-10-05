# Threats

The base threat is a slow walker: a corpse moved by an infectious colony. The colony prepared the body for weeks before killing it. The outbreak spreads through infected people who still travel, not through zombie speed. The zombie inherits the body, clothing and gear of who it was. It perceives by sight, hearing and smell. The world is finite: nothing respawns. Later, the sandbox may swap the threat for others (living infected, parasites, aliens). It does so by composing modules, not by writing whole species.

Labels: **Decided** (the user confirmed it in the prototype), **Proposal** (not confirmed), **Open** (pending question).

## General rules

| Rule | Status |
| --- | --- |
| People, loot and zombies are finite and do not respawn. | Decided |
| Hard, long survival; combat is dangerous even against few. | Decided |
| Other survivors and bandits also exist as encounters. | Decided |
| Turning an inhabitant keeps their identity, bonds and inventory; modules and capabilities change. | Proposal |
| The AI does not special-case species: it perceives, weighs drives and requests actions from the body. | Proposal |

## Base origin: necrobiotic syndrome

User concept, developed as a Proposal. Elements from the user: long incubation, death before reanimation, slow walkers, transmission through blood/saliva or suspended material, researchable cure, eating restores energy and fluid without regenerating limbs.

Causal model (Proposal):

- Over weeks, the agent installs a metabolic network in muscles, fascia, marrow and peripheral nerves.
- The person dies: conscious identity is lost. The colony stays active and uses the corpse as a frame.
- The agent prepares the body before death; it does not rebuild old corpses.
- Moving and perceiving consume energy, water and tissue.
- A cure can save the living host; neutralizing the zombie does not bring the person back.

Walker physiology (Proposal):

- A dark carrier fluid instead of blood. Eating provides water and substrate; it does not repair muscles or limbs.
- If it loses fluid through wounds faster than it absorbs, it gets stuck eating without ever being sated.
- It guards its food: distant noise does not distract it; a living person nearby does. Without an abdomen, eating is useless.
- It does not run: poor coordination, incomplete balance, little power. Fresh ones shove, grab or speed up for a short stretch.

## Virus as a sandbox setting

Decided (2026-10-05): the player can set the virus parameters before a game, Plague Inc style. Necrobiotic syndrome (above) is the default preset. Parameters are numbers on the same model, never new rules: transmission routes (bite, scratch, blood, saliva, airborne particles) and their chances, incubation range, symptom timing, lethality, time from death to reactivation, walker speed (see below) and whether a cure exists. Player and survivors face the same virus. Later, swapping the threat for other kinds (living infected, parasites) stays a module composition, as stated at the top.

## Course of infection

Timings vary by individual; example of "about a month". Proposal.

| Stage | State | Gameplay |
| --- | --- | --- |
| Exposure | Mucosa, wound or particles | Dose and route are recorded; not revealed |
| Silent incubation | Weeks; few signs | The carrier keeps traveling and infecting |
| Prodrome | General symptoms | Doubt, isolation, misdiagnosis |
| Neurological failure | High contagion | Medical and social decisions; a treatment window still exists |
| Host death | Identity lost | Bodies, hospitals and morgues are hotspots |
| Reactivation | The network takes over limited locomotion | Slow walker with the inherited body state |

Conflict: the GDD gave bite 100 %, scratch 30 %, prodrome in 6–12 h, death in 24–48 h and amputation in <180 s. That clashes with the long incubation of the user's concept. I prefer the long version. Open: how emergency amputation fits.

Conflict: the GDD had "fast and strong" fresh zombies. The user's later concept asks for slow walkers. I prefer slow. Proposal.

Resolved (2026-10-05, [0010](../decisions/0010-concept-v2.md)): zombie speed is a sandbox setting (always slow, body-driven, fast). The default is body-driven: mostly slow walkers, fresh lean bodies faster, fat or broken ones slower. Decided.

## Spread and contagion

- The first wave comes from people in incubation: presymptomatic contagion, misdiagnosis, multiple hotspots. Proposal.
- Organized groups can contain walkers in open ground; institutions are not incompetent by default. Proposal.
- Blood and saliva infect through mucosa or wounds; nearby droplets and deposited material keep some risk. Proposal.
- Airborne contagion at a distance is another preset: it changes ventilation, masks and shelters. Proposal.
- Without presymptomatic contagion, the preset is a local outbreak, not a global apocalypse. Proposal.

## Cure and research

Four configurable outcomes. Proposal.

| Outcome | Effect |
| --- | --- |
| Prophylaxis | Prevents an exposure from establishing the colony |
| Early treatment | Removes or slows the agent before neurological damage |
| Stabilization | Infected person alive and non-contagious, perhaps on ongoing treatment |
| Post-mortem neutralization | Stops the zombie; does not restore the person |

- The cure does not come from a crafting bench. Steps: recognize cases, samples, diagnosis, target, safety, batches, distribution. Proposal.
- Experts, reagents, power and machines are finite. Evidence accumulates; no bar fills up. Proposal.

## Human-derived zombie

From the GDD. Proposal.

It keeps height, build, clothing and the gear of its profession:

| Origin | Trait | Possible loot |
| --- | --- | --- |
| Police / military | Vest: −70 % bullet damage to the chest | Pistol, ammo |
| Firefighter | Fire-resistant jacket, helmet | Axe |
| Doctor / nurse | Gown | Bandages, alcohol, pills |
| Athlete | Long stride, intact footwear | — |

Minor conflict: the "faster" athlete zombie must respect the slow walker's speed cap. Proposal.

## Localized damage

From the GDD. Proposal.

| Destroyed zone | Result |
| --- | --- |
| Leg or knee | Falls and crawls |
| Arms | Cannot grapple or grab |
| Jaw | Cannot bite; only clumsy blows |
| Skull | Permanent neutralization |

In the necrobiotic preset, neutralization attacks the network (central nervous system and active tissue). Open: exact rules.

## Perception and attraction

| Stimulus | Rule | Status |
| --- | --- | --- |
| Sight | Affected by darkness; lack of light changes perception for zombies and players. | Proposal |
| Hearing | With occlusion by walls; sound travels through the room graph. | Proposal |
| Smell | Player emission index dispersed by wind; travels through the room graph. | Proposal |
| Alarms | Battery alarms go off when the grid fails and attract zombies. | Proposal |
| Radio | A speaker attracts; headphones are silent but mask ambient hearing. | Proposal |
| Light | A flashlight visible through gaps alerts light-sensitive zombies. | Proposal |

Scent sources from the GDD (Proposal):

| Source | Intensity | Persistence | Removed by |
| --- | --- | --- | --- |
| Fresh blood on clothing/skin | 100 | High (60 dry) | Washing or changing |
| Cigarette smoke | 90 | Short (5 min) | Not smoking nearby |
| Open wound / discharge | 85 | Until healed | Disinfect and dress |
| Hot food / raw meat | 50 | Medium | Airtight container |
| Sweat / dirt | 45 | Medium | Bathing, washing with soap |

## Corpses

From the GDD. Proposal.

| Phase | Time | Effect |
| --- | --- | --- |
| Fresh | 0–24 h | Blood attracts wild dogs and infected |
| Bloated | 1–7 days | Stench carried by wind: nausea and low morale; cholera risk near water |
| Skeleton | 2–12 weeks | Toxic soil; months later, dense regrowth of weeds |

Corpses soon switch from ragdoll to a static mesh with decomposition progress. Proposal.

## Population at scale

Proposal from the engine research.

- Near (~50 m): individuals with full AI. Loaded district: groups. Elsewhere: migrating density.
- Moving between levels preserves the count: the world stays finite.
- Each threat module declares its off-screen summary. A swarm is summarized per sector; an infiltrator needs identities.

## Threat sandbox

User idea: at world creation, pick the zombie type, or swap it for aliens, monsters or other variants. Proposal; does not commit content.

Combinable axes (Proposal):

| Axis | Values |
| --- | --- |
| Substrate | Living human, corpse, parasitized host, construct, alien, anomaly |
| Identity | Preserved, fragmented, erased, copied, collective |
| Agency | Reflex, animal, habitual, tactical, sapient |
| Control | Self, external master, parasite, hive, signal, compulsion |
| Propagation | None, wound/fluids, air/spores, death, ritual, assimilation, nest |
| Locomotion | Crawl, walk, run, climb, flight, burrowing |
| Senses | Sight, hearing, smell, heat, vibration, shared network |
| Drive | Hunger, transmission, command, territory, curiosity, reproduction |
| Neutralization | Trauma, CNS, immobilize, starvation, antidote, break signal, destroy nest |
| Evolution | Stable, decay, recovery, stages, mutation, learning |

Candidate families by cost (Proposal):

- Low–medium: fast living infected with a fictional "rabies".
- Medium: epidemic corpse (base), controlled and rescuable zombie, revenant or curse.
- Medium–high: conscious/relational (remembers, resists drives, keeps company).
- High: parasitic/fungal host, hive with a node, alien infiltrator, alien fauna.
- High to very high: mutants, anomalies that alter memory, space or time.

Cost bands: parameters and presentation (cheap); new module on a humanoid (medium); new body (expensive); new world rule (very expensive). Proposal.

- Separate the origin profile (truth, timeline, evidence, cure ceiling) from the threat profile (body, behavior, contagion). Proposal.
- Coherent presets first, advanced mode later. Incompatible combinations are rejected or explain what is missing. Proposal.
- World creation sets a causal timeline, not a script. NPCs know fragments according to profession and place. Proposal.
- Test presets: necrobiotic, living infected, conscious/relational; no alternative one in the first slice. Proposal.

## Prototype sources

- GDD_Survival_3D.md (sections 8, 11, 18, 20, 22)
- wiki/amenazas-sandbox.md
- wiki/amenazas-sandbox-taxonomia.md
- wiki/amenazas-sandbox-origen-necrobiotico.md
- wiki/decisiones-mundo-aprendizaje.md
- engine-research.txt
