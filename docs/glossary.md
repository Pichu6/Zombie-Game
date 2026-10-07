# Glossary

Shared project vocabulary for design documents and new-engine code. It keeps the prototype terms still in use, minus those tied to Godot or specific assets, and adds terms from the engine research. Definitions describe design intent; each rule's status (Decided, Proposal, Open) lives in its design document.

## World

- **Conservation**: starting population = alive + dead + converted + located escapees. The same applies to object mass.
- **Deferred simulation**: a dormant inhabitant or sector advances through scheduled events, not ticks.
- **Finite persistent world**: no respawn of people, loot or zombies. Each city has a finite starting census.
- **Room graph**: rooms and corridors as containers joined by openings; it carries water, gas, smoke, sound and smell.
- **Sector**: a portion of the map loaded, saved and simulated as one unit.
- **Simulation level (hot / warm / cold)**: full detail near the player, network calculation in the district, statistics elsewhere.
- **Utility network** (red de servicios): power and water as graphs of nodes and edges, recalculated only when they change.
- **Lazy census**: everyone in the world who has never been touched by the simulation; regenerated from the world seed on demand, costing no memory.
- **Record**: a compact saved person, made when the simulation first touches someone (met, hired, bitten).
- **Embodied bubble**: the people near the player's body who run the full body simulation.
- **Macro / City / Individual layers**: the three gameplay layers over one world simulation (globe, isometric city, one person).
- **Edict**: a law or order that may only change a rate, capacity, allocation, condition, use-of-force threshold or broadcast; checked by the same validator as scenarios and sandbox sliders.
- **Project**: the record created by a build order; it posts jobs and material requests and holds the escrowed money.
- **Lifepath**: a pop's past, sampled term by term backwards from who they are today, TTRPG style.
- **Dossier**: the inspector seen through a character's knowledge; each fact shows its source and age and can be wrong.
- **Projection**: the written, field-by-field rule that turns a body into a record on demotion, saying what is kept as integers and what is dropped as a readout.
- **Memorisation blob**: what a record stores from the last demotion that the record itself cannot represent (position in a room, a mid-action state), so promotion can restore it.
- **Kept reason**: a flag that keeps a record from merging back into a census row (player tie, story tie, witnessed, possessed, never cull).
- **Quantum**: the visible unit of an aggregate shown to the player (a household, a pack of walkers), so a cold count stays readable.
- **Chunk header**: per saved chunk, its schema and generator versions, generation status, caught-up tick, level and checksum.
- **Jump form**: a rule's `advance(state, from, to)` that applies it over an interval in one step; an exact event loop when rules inside the interval are coupled.
- **Archetype**: a household template (ages, sexes, relationships) that a census cell holds an integer count of; the human generator maps a slot to its archetype by cumulative count, so totals are exact.
- **Outbox / commit**: in the two-phase tick, the keyed messages a chunk writes during the parallel pre-update, applied serially in id order afterwards.

## Objects

- **Assembly graph**: an object's list of components, each with material, Q and condition.
- **Condition**: wear, damage or corrosion. Changes with use and repair.
- **Container**: an object or place that holds instances; its capacity is distinct from the carrier's.
- **Definition**: shared data for an object type (mass, capabilities, materials, processes).
- **Effective Q**: the Q an action yields after being capped by skill and tool.
- **Instance** (ejemplar): a concrete object with unique ID, Q, condition, location and real components.
- **Material**: what a component is made of.
- **Q (quality)**: how well something was made. Fixed except through a real process.
- **Recipe / process**: inputs, tools, knowledge, time, outputs, losses and Q rules.

## Body

- **Biological mass**: the naked body (fat + muscle + structural).
- **BodyParts3D**: anatomical atlas (CC BY 4.0) used as reference for the detailed medical view.
- **Capacities**: moving, manipulation, consciousness, etc., calculated from body parts.
- **Clo**: unit of clothing thermal insulation.
- **Clothing layers**: inner, middle and outer, CDDA-style.
- **Compartments (kg)**: the body stores masses in kg: fat, muscle and structural. Percentages are derived.
- **Encumbrance** (estorbo): movement or accuracy penalty from thick, rigid or loaded clothing.
- **Moving mass** (masa móvil): biological mass plus clothing and load. Governs calorie expenditure and fatigue.

## Inhabitants

- **Assessment**: what an inhabitant believes about an instance, with confidence and date.
- **Background (lifepath)**: chain of prior stages that leaves real contacts, practice, goods and debts.
- **Faction / operational group / role**: social membership; those who act together; leadership and delegations.
- **Inhabitant** (habitante): persistent individual with identity, body, knowledge, practice, inventory, relationships and membership.
- **Knowledge**: which procedure an inhabitant understands.
- **Novelty key**: identifies what was learned, so the same thing is not rewarded twice.
- **Player–AI parity**: player and AI use the same actions and rules; the AI only picks intents.
- **Practice / skill**: accumulated ability to perform a procedure.
- **World fact / memory / account**: what happened; what each person perceived; what circulates as rumour.

## Interaction

- **Combat mode**: a held stance, slower, ready to attack or defend.
- **Common executor**: the system that validates, times and applies every action, from player or AI.
- **Offer (Smart Object)** (oferta): an action an object exposes to an executor, with availability and block reason.
- **Reservation**: a lock on an instance while an action uses it.
- **Third person / tactical / management**: three control modes over the same world and the same rules.

## Cameras and presentation

- **Cutaway (roofs and walls)**: hides the roof or front walls when the character is inside.
- **Fog of war**: hides what the character cannot see in tactical view and leaves a grey memory.
- **LOD / proxies**: simplified geometry at distance to save render cost.
- **Overlay**: visual layer for the power, water or gas network in isometric view.

## Prototype sources

- glosario.md
- arquitectura-sistemas.md
- aprendizaje-especializacion.md
- historias-relaciones-sandbox.md
- ofertas-interaccion.md
- fundamentos-mundo-personajes-produccion-habitantes-objetos.md
- engine-research.txt (engine research)
