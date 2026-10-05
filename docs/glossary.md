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
