# Inhabitants

Each inhabitant is a persistent individual with a body, knowledge, practice, inventory, relationships and membership. The city has a finite starting population and nobody respawns. Inhabitants learn by studying and practising, without rigid classes. Their stories come from shared events, differing memories and asymmetric bonds. Away from the player they are simulated in less detail and by events, without losing identity or conservation.

Labels: **Decided** (the user confirmed it in the prototype), **Proposal** (not confirmed), **Open** (pending question).

## Identity, group and authority

Decided:

- The player starts with their character and can find survivors or bandits.
- They can form groups, join others or recruit.
- Controlling NPCs depends on group or faction membership and a leadership role.
- Individual inhabitants with traits, states and customisation. References: Sims/pops with XCOM elements.

Proposal:

| Concept | Meaning |
| --- | --- |
| Faction | Social membership and relationships. Being friendly does not give control. |
| Operational group | Those who travel or work together and share local orders. |
| Role | Leader and explicit delegations within the group. |

- A recruited NPC is the same individual, with their objects and prior state.
- Leading NPCs gives no control over other players' characters.
- Open: leader succession, transfer of command, desertion, loyalty, disobedience.

## Finite population

Decided:

- People, loot and zombies are finite; no respawn.
- Each city has a logical, finite starting population.
- Demanding survival: one or two enemies can already be dangerous.
- Conversion, flight and death move or transform existing people.

Proposal:

- Population comes from dwellings, households, jobs and outbreak context, not from a technical maximum.
- Conservation: starting = alive + dead + converted + located escapees.
- The world keeps the real census; the character only knows what was observed or communicated.
- Trade does not regenerate goods. Natural reserves are finite quantities.
- Open: births, crops and growth. They are not an automatic exception to "no respawn".

## Needs and body

Proposal:

- Needs (hunger, thirst, sleep, temperature, pain) come from the body simulation, not from standalone bars.
- Object offers describe how to satisfy them; the body applies the real effect.
- Moving mass (body + clothing + load) governs calorie expenditure and fatigue.
- Body model from the research: tree of parts with tissue layers → capacities → actions.
- Fatigue or injury affect effective capacity; they never erase knowledge.
- Open: nervousness and panic model; utility evaluation to choose which need to address.

## Learning and specialisation

Decided (2026-09-07):

- Each player steers their character's specialisation through what they study and practise.
- Studying takes the character's time.
- Understanding a procedure, associating a tool and performing it are distinct advances.
- The first novel actions give experience; repetition gives less or none.
- Another instance of a known type can teach through differences, parts or faults.
- Applies to all items according to their capabilities, not to one specific type.

Decided (2026-09-10, Haven & Hearth model): knowledge and profession filter which actions each inhabitant sees. Skill is a soft cap on the Q obtained. See `items-crafting.md`.

Proposal:

- Open specialities, with no classes that lock out other areas.
- Related knowledge transfers between compatible procedures.
- Studying never raises any object's Q.
- A mistake teaches only if it reveals something new. Repeating empty failures is not rewarded.

### Four separate states

| State | Belongs to | Example |
| --- | --- | --- |
| Knowledge | Inhabitant | Understands a threaded fastener |
| Practice | Inhabitant and domain | Has taken apart many latches of that family |
| Assessment | Inhabitant, instance and property | Believes this part is damaged (with date, method and confidence) |
| Effective capacity | Executor when acting | An injured hand or a bad tool make the task harder |

### Proposed cycle

1. The object exposes observable properties, without revealing hidden parts.
2. The inhabitant recognises what they know and starts study if they have access and means.
3. The action advances in simulation time and records milestones.
4. Knowledge associates tools and procedures.
5. Performing gives a real result and evidence of practice.
6. A novelty key decides what was learned, with diminishing returns.

- Only the simulation validates milestones; the UI never grants experience. Proposal.
- The novelty record prevents double rewards on reload or camera change. Proposal.
- A manual or mentor can give knowledge, not equivalent practice. Proposal.

Open: domain taxonomy; starting knowledge by background; durations and curves; forgetting (not confirmed, do not introduce); Q visibility; autonomous NPC learning; knowledge after death or recruitment.

## Stories and relationships

Proposal (research, not confirmed as a milestone).

A good creation answers whom you affected, where, what changed and what remained unresolved.

### Fact, memory and account

| Layer | Content |
| --- | --- |
| World fact | What happened, when, where, who; stored once and referenced by everyone |
| Personal memory | What each person perceived, with confidence, detail, emotion and errors |
| Evidence | Objects, wounds, corpses, records, places, testimonies |
| Circulating account | Rumour or official version; it mutates without changing the truth |


### Bonds

- Sparse graph: an edge exists only if there was kinship, cohabitation, contact or information.
- A's view of B: familiarity, affinity, trust, respect, fear, resentment, dependence, optional attraction.
- Also: known facts, notable memories, what A wants from B and what stands in the way.
- Bilateral and asymmetric: Ana trusts Bruno and fears him; Bruno loves her and believes she betrayed him.
- Healing raises trust or debt, not affection. Intimidating gives obedience, fear and resentment.
- Giving the optimal gift does not buy friendship. Under scarcity, need, cost, intent and provenance matter.
- A bond gives no telepathy; news requires observation, reporting or inference.
- Player and AI use the same social actions: observe, ask, promise, lie, help, threaten.
- From Stardew Valley: rhythm of living together, discoverable tastes, relationship milestones, social calendar.
- Relationship milestones change something actionable (access, help, confession), not just dialogue.

### Background without rigid class

Chain: home → formative experience → trade → crisis → decisive bond → prior incident → outbreak → current goal.

- Each step leaves verifiable residue: starting practice, real contacts, access, goods, debts, injuries, an open question.
- A profession never gives perfect mastery. "Was a paramedic" is not "medicine 80".
- Every benefit carries a coherent cost: time, obligation, injury, age.

### Outbreak origin

- The causal truth of the outbreak is fixed in the simulation, though hidden from the player.
- Grammar: cause → first incident → witnesses → cover-up → public version → failures → personal experiences.
- No mandatory main quest. If the only virologist dies, no replacement appears; her notes remain.

### Generation

From larger to smaller: city → households and organisations → individual skeleton → detail on entering focus.

- Deterministic expansion; every revealed fact stays fixed. "Not detailed" does not mean "contradictable".
- Off-screen, upcoming events are scheduled, not frame-by-frame conversations.

## Scale and deferred simulation

Proposal. One identity passes through three levels of detail; the research calls them hot, warm and cold.

| Level | Where | What runs |
| --- | --- | --- |
| Full (hot) | ~50 m from a player, or in conflict | Body, perception, navigation, animation, actions in fine steps |
| Sector (warm) | Loaded district | Real actions, aggregate movement, body at longer intervals |
| Dormant (cold) | Rest of the world | No tick; phased plans, next threshold, analytic evolution |

- Per-inhabitant state: resolved time, compact state, current phase, next wake-up, sector, seed.
- The world clock is the authority; dropping a level schedules the next event.
- Rising a level resolves due events in order; never tick by tick. Bleeding, critical hunger or encounters wake an inhabitant.
- Promotion starts before becoming visible and respects a per-frame budget.
- Prototype benchmark: 1000 full bodies at 10 Hz cost ~30 ms per step. The deferred level is needed.
- Prototype save: ~28 KB per inhabitant. Separate definition from state and split by sector.
- First goal: dozens of materialised humans and thousands of persistent identities with exact conservation.

Proposal (user intent, figures not confirmed): large map, journeys of hours on foot, Arma/GTA as references. The research leaves size open (town, district or city). Open: size, simultaneous population, hardware and time acceleration.

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [gta](../research/gta.md): GTA, as a reference for the Individual layer
- [inzoi](../research/inzoi.md): inZOI, as a reference for the Individual layer
- [pop-simulation-and-persistence](../research/pop-simulation-and-persistence.md): Pop simulation, lenses per zoom, and saving the world
- [pop-behaviour](../research/pop-behaviour.md): Pop behaviour from job, skills, traits and state: games and real life
- [procedural-mysteries](../research/procedural-mysteries.md): Procedural mysteries: cases that come from a simulation
- [story-building-blocks](../research/story-building-blocks.md): Story building blocks: how games and story theory build variety from parts
- [lore-made-and-generated-worlds](../research/lore-made-and-generated-worlds.md): Lore in made and generated worlds: Project Zomboid, other games, and tools outside games

## Prototype sources

- aprendizaje-especializacion.md
- historias-relaciones-sandbox.md
- base-funcional-habitantes.md
- base-funcional-habitantes-contratos-acciones.md
- fundamentos-mundo-personajes-produccion.md
- fundamentos-mundo-personajes-produccion-habitantes-objetos.md
- fundamentos-mundo-personajes-produccion-aprendizaje-escala.md
- arquitectura-sistemas.md
- engine-research.txt (engine research)

## Open

### From research (pop-behaviour, 2026-10-09)

The note proposes a pop record (hidden dispositions shown as trait words with signed effects, skills learned on exponential curves with trade time constants, a job with duties and an action set, attachments, needs, fear and slow stress) and a dual-utility choice with ranks (survival, urgent body, duty, household, needs and helping, wait and see), with pass bands from disaster research (panic under 5 %, police 5-20 % and health staff 20-50 % off duty, evacuation 40-75 %). Its questions:

- How many hidden dispositions: six numbers, full Big Five plus stress traits, or DF-style dozens of facets? Fewer is cheaper to save per pop (pop storage notes). ([pop-behaviour](../research/pop-behaviour.md))
- Should a player see a stranger's traits at once, or learn them by observing (FM scouting, [PB-23](../research/pop-behaviour.md))? ([pop-behaviour](../research/pop-behaviour.md))
- Does the player's own character get traits by the same draw, or chosen at creation with points (PZ)? ([pop-behaviour](../research/pop-behaviour.md))
- Role conflict needs to know where family is: does the AI use the pop's belief (what it knows) or the truth? Spec 24 has no belief store in Tier 0. ([pop-behaviour](../research/pop-behaviour.md))
- Does stress change dispositions over months (DF memories, [PB-19](../research/pop-behaviour.md))? It would make long play deeper but break the calibration over time. ([pop-behaviour](../research/pop-behaviour.md))
- Looting and crime: how much comes from need (rare, [PB-49](../research/pop-behaviour.md)) and how much from pre-existing criminals in the census? Needs a crime source. ([pop-behaviour](../research/pop-behaviour.md))
- Soldier units: do they get unit cohesion as its own number, or is it bonds plus training? ([pop-behaviour](../research/pop-behaviour.md))
- Short-skill time constants (first aid, firearms, driving) are guesses; they need sources before going to `data/`. ([pop-behaviour](../research/pop-behaviour.md))

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- Should the selected pop show a "why" line (current goal, the need behind it, what it believes) to make the window readable? ([inzoi](../research/inzoi.md))
- Off-screen life changes must be legible and consentable (Sims 3 lesson); records browsable by filters with additive breakdowns. Confirm? ([pop-simulation-and-persistence](../research/pop-simulation-and-persistence.md))
- From [borrowed-from-other-fields](../research/borrowed-from-other-fields.md) §4: pop knowledge as 16 tracks: enough, which kinds, which half-lives (XD-128, XD-129)? Do pops share bearings to cross-fix sounds, or is that a skill (XD-131)?
