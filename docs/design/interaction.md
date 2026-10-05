# World interaction

Players and NPCs act on the world under the same rules. Each object offers interactions based on its capabilities; a common executor validates them, runs them in simulation time and applies their effects. The interface only expresses intents: act in person, order one's own character, or delegate to an authorised unit. There are three control modes (third person, individual tactical and management), and all reach the same executor. Keyboard, mouse and gamepad are each complete input paths.

Labels: **Decided** (the user confirmed it in the prototype), **Proposal** (not confirmed), **Open** (pending question).

## Player–AI parity

Decided: every human survives, moves, works, equips, eats, rests, fights or dies with the same actions and rules, whether a player or an AI controls them.

- The AI picks high-level intents. It does not mimic key presses or bypass the common executor. Decided.
- Player and AI go through the same requirements, reservations, times, consumption and effects. Proposal.
- The AI gets no free attacks or special shortcuts. Proposal.
- An off-camera action does not succeed automatically. It keeps route, load, phase, risks and interruptions. Proposal.
- Changing who controls an inhabitant does not change times, costs or results. Proposal.

## Interaction offers (Smart Objects)

Proposal, with a partial base tested in the prototype.

- An object's definition declares its interactions by identifier (e.g. `transfer`).
- Another type that declares the same capability gets the same behaviour, without its own code.
- An unimplemented interaction produces no fake actions.
- Each offer contains: identifier, label, executor, target, availability, block reason, phases and described effect.
- An offer can be shown as blocked, with its reason.
- The identifier includes source and destination. A stale menu does not act on an object that has moved.
- The offer describes a possibility; the validated process produces the consequences.
- No direct effects like "energy +80" that bypass the body simulation.

### Knowledge filtering

Decided (Haven & Hearth model, 2026-09-10):

- Offers are not universal. They depend on the actor's knowledge and profession.
- An electrician knows how to replace a transformer fuse cartridge; a surgeon, how to do vascular sutures.
- Without knowledge, only clumsy high-risk actions appear, such as ripping out cables by force.
- Tool Q affects speed, wear and success rate.

Proposal: everyday actions are available from starting knowledge; not everything requires prior study.

### Querying is not executing

- The preview uses the same checks as execution and changes nothing. Proposal.
- Checks: actor available, reservation, source, capacity, access and path. Proposal.
- On execution, the system validates everything again. Proposal.
- Effects happen in the executor, never in the menu or by playing an animation. Proposal.
- The caller only passes targets the character knows or perceives. Proposal.

## Timed actions

Proposal. Every action has requirements, duration, resources, phases, interruption and effect.

- One action per executor and one reservation per instance.
- Ownership changes only on completion.
- Cancelling does not refund time spent.
- Pause stops progress and the body.
- Saving keeps access, reservations and progress.
- If a loaded action has lost its requirements, the next validation interrupts it.
- Interruptions: death, lack of physical reserve, loss of reach, changed source, lack of capacity.
- Excessive distance triggers an approach first; never a silent remote transfer.

### Transfers

Proposed phases: access the source → extract and handle → access the destination → fit and close.

- Before extraction, the object is still in the source. During handling it is in the hand.
- An interruption does not duplicate or vanish the object, nor complete a whole batch.
- One-off access cost (opening the backpack) is separate from per-unit cost.
- Size, mass, quantity, grip, free hands, posture and distance all matter.
- Moving from a pocket to a closed backpack on one's back does not cost the same as to a box.

Prototype test coefficients, not calibrated:

| Parameter | Value |
| --- | --- |
| Work | source s + destination s + 0.5 + 0.5 per kg |
| Box access | 1 s |
| Personal container access | 3 s |
| Maximum reach to each end | 1.5 m |
| Minimum body effort during the action | 0.15 |
| Modulation | Effort and pain reserve, with a minimum speed while still capable |

### Body and capacity

- Fatigue, injury, pain and nervousness affect only the relevant phases. Decided (relevance); rules are a Proposal.
- There is no single "health" multiplier that penalises everything. Proposal.
- A thin body is not incapable by appearance: strength, composition, training and state count. Proposal.
- A transfer's duration depends on the object, source, destination and body. Decided.

## Selection, targeting and control

Decided: interaction with objects and containers; querying each character's state and items.

Decided: base scheme for the three modes.

| Mode | Select | Quick action / move | Extended options |
| --- | --- | --- | --- |
| Third person | Aim at a visible object | Interact button; direct locomotion | Hold interact or a dedicated button |
| Individual tactical | Left click on target | Right click on ground moves the character | Right click on object opens its menu |
| Management | Left click or area drag | Right click sets destination for selected units | Middle click opens orders, poses and actions |

- Assigning area selection and group orders to management is a provisional reading. Proposal.
- The "store object" action gives the same result in all three modes. Proposal.
- Four things are kept separate: controlled character, selected units, target object and camera focus. Proposal.
- Selecting a box does not change the controlled character. Proposal.
- Inspecting a companion does not authorise touching their inventory. Proposal.
- The target is fixed when the menu opens and revalidated on execution. Proposal.
- Reach, access, visibility and knowledge are distinct constraints. Proposal.
- Show target, executor and block reason. Proposal.
- Open: control permissions and cooperative trading.

### Gamepad and inputs

Decided: gamepad and keyboard/mouse supported by design.

Proposal:

- Abstract actions: select, extend selection, confirm, contextual, cancel, change focus, inventory, queue, posture.
- Game rules never live in physical button handlers.
- Gamepad: assisted selection among valid targets, radial menu with a list alternative.
- Every essential operation can be completed without mouse, hover, drag or middle click.
- Remappable keys, an alternative to holding, visible focus, cancel goes back.
- Open: final physical layout; requires a gamepad prototype.

## Inventory and status interface

Proposal. One shared presentation, opened from any mode:

- Functional state: activity, fatigue, pain, known conditions. What is perceptible is separate from detailed diagnosis.
- Body and gear: hands, worn clothing, storage points.
- Containers: pockets, backpack, bags, with real contents.
- Transfer: source, destination, quantity, space, resulting load, estimated duration.
- Two-inventory view, or select object → choose destination → confirm.
- Action queue: executor, object, destination, progress, interruption, block reason.
- The menu requests actions; it never grants experience directly.
- Knowledge feedback: "no compatible tool", "procedure known, no practice", "unknown component".

## Movement, waypoints and patrols

Decided: movement orders, waypoints and possible patrols.

Proposal:

- Waypoints express intent; navigation respects collisions, doors, stairs and connectivity.
- Distinguish replacing the destination from queueing points.
- A route can be viewed, edited, cancelled and resumed. An impossible leg warns; it never teleports.
- Patrol: reusable route with repetition, waits and facing.
- Open: formations, priorities and response to threats.

## Posture, environment and combat

Decided: animations aware of walls, corners and doors, especially in combat stance. Reference: Ghost Recon Breakpoint.

Proposal:

- Separate environment detection, posture decision and animation.
- Do not snap the character to every nearby wall. Combine intent, facing, free space and body state.
- Animation represents a viable action; it does not authorise passing through collisions.
- Holding a button enters combat mode: slower, torso toward the threat, ready to attack or defend.
- Separate input, intent, resolution and representation.
- Objects declare grips and uses, not just "one hand / two hands".
- Changing grip, regaining balance or freeing a stuck tool takes time.
- Distances: out of reach, weapon reach, contact and grappling are distinct states.
- Aiming does not guarantee a hit. Assistance shows the focus and never corrects through walls.
- Third person: free reticle against body parts, affected by recoil, fatigue, pain and breathing.
- Isometric: zone selector (head, torso, legs, arms) and a spread cone that narrows as aim steadies.
- The cone depends on skill, weapon Q and calm. Moving, shaking or taking a hit widens it.
- Open: bindings, list of techniques, hold versus toggle.

## Build order

Proposal: inhabitant state → objects and inventories → action executor → selection and controls → navigation and orders → autonomy and co-op.

## Prototype sources

- interaccion-control-inventario.md
- ofertas-interaccion.md
- base-funcional-habitantes-contratos-acciones.md
- aprendizaje-especializacion.md
- fundamentos-mundo-personajes-produccion-aprendizaje-escala.md
- fundamentos-mundo-personajes-produccion-habitantes-objetos.md
