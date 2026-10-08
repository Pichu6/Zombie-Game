# Cameras, modes and controls

Cameras are lenses over a single simulation: changing view never changes rules, damage or state. There are three modes. Third person (own body), isometric tactical (one character, their perception) and management (the team, vision shared by radio). The player is independent of the inhabitant they control. They can take over another one without resetting its state. Third person and isometric are built first, on the same character. Management, RTS and co-op wait for multiplayer (Proposal from the research).

Labels: **Decided** (the user confirmed it in the prototype), **Proposal** (not confirmed), **Open** (pending question).

## Principles

| Rule | Status |
| --- | --- |
| Same simulation in every mode; a single combat and damage system. | Proposal |
| The player is independent of the controlled inhabitant. | Decided |
| Macro orders in management, isometric tactics and immersive direct control. | Decided |
| NPCs act autonomously when nobody controls them. | Decided |
| Flow from management to third person, also indoors and underground. | Decided |
| Tactical and management have different roles; zoom and freedom do not make them equal. | Decided |
| Inventory, exertion and symptoms persist across changes of control, camera or order. | Decided |
| Gamepad and keyboard/mouse are both complete, comfortable paths. | Decided |
| Start with third person + isometric on one character. | Proposal |
| Defer commander/RTS mode and city manager. | Proposal |

## The modes

| Mode | Focus | Visible information | Role |
| --- | --- | --- | --- |
| Third person | Own body and immediate surroundings | Own perception | Move, aim, interact, fight |
| Tactical (isometric) | Chosen character and surroundings | Own perception only; never the team union | Position and decide individual actions |
| Management | Team operation | Union of what allies with radios report | Coordinate sectors, assign tasks to NPCs |
| City manager | Districts, coverage, supply | "Cold" simulation state | Deferred (Proposal) |

Tactical shows only the chosen character's perception and stays centered on them. Management gathers the team's vision. Decided.

User references (Decided as reference, not as a copy of systems): third person like GTA, Ghost Recon Breakpoint and Gears of War; tactical like Diablo and Door Kickers. They do not imply a military setting or cover-based combat.

## Camera parameters

Developer test values, not user decisions. Proposal.

| Mode | Tested parameters |
| --- | --- |
| Third person | ~1.85 m from the pivot, 12 cm lateral offset, 62° FOV, shortens on obstacles |
| Tactical | Target at character + 0.9 m; fixed 55° tilt; orthographic 10–17 m (initial 13); free rotation, no pan |
| Management | Tilt 65–80° (initial 72°); orthographic 30–60 m (initial 44); pan limited to the scene |

- The scroll wheel does not change mode. Switching is explicit and each mode keeps its framing. Proposal.
- Conflict: the GDD proposed a continuous 2.5–60 m zoom that went from micro to macro. The later prototype dropped it. I prefer explicit modes. Proposal.
- Selecting a character in management does not drag the management camera. Proposal.
- Camera follow and impact kick: smoothing recipe in [research/second-order-dynamics](../research/second-order-dynamics.md). Proposal.

## Interiors and floors

- Third person: walls and ceilings between camera and character turn semi-transparent (dithering), without snapping the camera closer. Proposal.
- High views: floors above the character's floor are hidden (floor cut). Proposal.
- Remove roofs only when management needs it; restore the architecture when walking through it. Proposal.
- Changing floor must not reveal hidden information by accident. Proposal.
- Transparency never changes perception. Proposal.

## Perception and fog

Each observer has their own perception. Camera height, pan or zoom do not grant vision through walls. Decided (binoculars and lookout); the rest Proposal.

| Component | What it does | Status |
| --- | --- | --- |
| Detection | 3D line of sight, obstacles, light, range and attention. Detecting is not identifying. | Proposal |
| Acuity | Curve by distance and contrast; equipped lenses correct it. | Proposal |
| Memory | Last observation, age and confidence. Moving things do not update inside the fog. | Proposal |
| Stability | Gaze tremor and accuracy separate from the visual effect. Camera shake optional. | Proposal |

- Fog: visible in color, remembered in gray, unknown in black. Black is ignorance, not night. Proposal.
- Tested values: 110° cone, 12 m range; lookout 24 m. Proposal.
- In third person, blur can express acuity. In tactical and management, show contact uncertainty, not screen blur. Proposal.
- Never move or blur the interface. Proposal.

## Co-op

Reference scenario confirmed by the user. It is direction, not a scripted mission. Decided.

1. PC1 fights in third person; PC2 in tactical; they coordinate by radio.
2. PC3 watches from a tower in management, selects NPCs by area and orders them to loot.
3. NPCs with radios contribute their vision cones as reports.
4. PC3 uses binoculars and pans farther from the height; warns of reinforcements by radio.
5. PC1 and PC2 cross paths on the stairs and talk by local voice.
6. PC3 orders an evacuation at a jog; an overloaded NPC complains and the order stands.
7. Once safe, a player takes over that NPC and finds muscle pain. They can treat it or delegate it to another NPC.

Derived rules:

| Rule | Status |
| --- | --- |
| Two voice channels: radio at a distance and local proximity voice. | Decided |
| Radio allows voice and sharing cones as an abstraction of what is communicated. | Decided |
| Hearing a warning does not show the enemy through walls in tactical or third person. | Decided |
| The coordinator can evacuate their body while staying in management. | Decided |
| Area selection and group orders; destination and pace are separate data. | Decided |
| A complaint does not cancel the order; separate warning, will and physical capacity. | Proposal |
| Everything tied to the character simulation: menus, items, transfers, orders. | Decided |
| Orders to other players are suggestions they accept; to NPCs, by permissions. | Proposal |
| Separate own character, selection for orders and camera focus. | Proposal |
| Group membership and leader role govern NPC control. | Decided |
| Radio reports carry sender and age; without connection, they age. | Proposal |
| Radio range, obstruction, batteries and latency. | Open |
| Speech recognition to turn talk into orders. | Open (not confirmed) |

## Control and interaction

- Contextual mouse selection; aim-based selection in third person; group selection, waypoints and patrols. Decided.
- Contextual animations at walls, corners and doors in combat stance. Decided.
- Shared actions: the menu, manual transfer and an order to an NPC use the same rules. Proposal.
- Debug tools always distinguishable from player actions. Decided.
- Movement relative to the active camera. Proposal.

Base scheme from the GDD (Proposal; final shortcuts Open):

| Action | Gamepad | Keyboard and mouse |
| --- | --- | --- |
| Move | Left stick | WASD |
| Aim / look | Right stick | Cursor |
| Combat mode | Left trigger | Right click (hold) |
| Attack / shove | Right trigger | Left click |
| Rotate camera | LB / RB | Q / E or middle button |
| Interact | X / Square | E / F |
| Run / crouch | L3 / A | Shift / Ctrl |
| Inspect body and clothing | D-pad up | H |

## Scouting before entering

From the GDD. Proposal.

- Peeking through a gap: the field of view grows as the eye gets closer (θ = 2·arctan(w/2d)). At 1 m, ~2°; pressed against it, >50°.
- In third person the camera moves to the eyes and the room's sound opens up. In high views, a wedge reveals the fog.
- Flashlight and scent also cross the gap: the risk goes both ways.
- Ear to the wall: drywall carries footsteps and voices; brick only knocks; insulation almost nothing.
- Your own pulse masks what you hear. The listener loses rear vision.
- A knock on the wall triggers a startle: panic, a step back, possibly dropping the held item.
- 3–5 cm gaps in barricades allow attacking from inside; the weapon can get stuck.

## Time

- Time acceleration is the hard part, not the cameras. Proposal.
- Single player: accelerate only with no threats nearby. Proposal.
- Multiplayer: a single speed. Proposal.
- The simulation clock does not depend on UI or camera. Proposal.
- Does isometric allow acceleration? Open.

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [concept-pichu-original-vision](../research/concept-pichu-original-vision.md): Pichu's original vision (2026-10-05, verbatim, lightly cleaned)
- [concept-v2](../research/concept-v2.md): Juego Zombie · Concept v2
- [second-order-dynamics](../research/second-order-dynamics.md): Second-order dynamics for procedural motion
- [world-in-conflict](../research/world-in-conflict.md): World in Conflict, as a reference for command
- [renderer](../research/renderer.md): The renderer, at engineering level
- [view-readability](../research/view-readability.md): Readability of the 2D top-down view: state, selection, alerts, overlays, and what our window lacks

## Prototype sources

- GDD_Survival_3D.md (sections 2, 16, 19, 20, 21)
- wiki/gameplay-cooperativo.md
- wiki/decisiones-experiencia-control.md
- wiki/decisiones-mundo-aprendizaje.md
- wiki/bases-produccion-visual.md
- art-direction/multi-team-blockout/README.md
- engine-research.txt

## Open

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- Is "call support" a commander-layer verb that shows the asset, its travel time and its ammunition? ([world-in-conflict](../research/world-in-conflict.md))
- Does direct control cap time at 1:1 (1.35 cells per real second) or 3x, and on WASD only or on any approach by the controlled pop? (Today the window's "1x" is 24x game time.) ([view-readability](../research/view-readability.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** X3 while walking or WASD held, Real in a fight.
- Does the follow camera hard-lock (OpenTTD, OpenRCT2) or lag with a dead zone (Godot, Mindustry), and which key recentres? ([view-readability](../research/view-readability.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** smoothed follow (tau 0.1 s) with a 20 % dead zone; F or Home recentres.
- Which events raise a pausing Pacer alert at normal speed (spec 40)? ([view-readability](../research/view-readability.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** a walker in sight of a group member, a bite, a group member down.
