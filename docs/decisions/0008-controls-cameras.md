# 0008 · Controls, cameras and co-op

- **Original date:** recorded in the prototype (2026-09); brought into the new project on 2026-10-05
- **Status:** Decided by the user in the prototype. Revalidate if anything changed with our own engine.

## Decisions

- The end goal is multiplayer. Starting single player, then co-op, then multiplayer, is accepted.
- Flow from management/RTS to third person, including interiors and underground. References: GTA, Ghost Recon Breakpoint, Gears of War.
- Tactical view shows only the character's own perception and stays centred on them. Management view merges the team's vision.
- Radio with voice and shared vision cones, plus local voice. A lookout with binoculars who can't see through walls.
- Area selection, paced orders; the coordinator can order an evacuation.
- Gamepad and keyboard/mouse are both full input paths, designed in from the start. Contextual and group selection, waypoints, patrols, contextual animations.
- Three control modes with their click scheme.
- A transfer's duration depends on the item, source, destination and body; fatigue and condition count.
- Animation reacts to walls, corners and doors (reference: Ghost Recon Breakpoint).

Design: [cameras and modes](../design/cameras-modes.md), [interaction](../design/interaction.md).
