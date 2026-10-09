# 0058 · Offices open the layers; orders go down the chain; single player is multiplayer with AI in the other chairs

- **Date:** 2026-10-09
- **Status:** Decided by the user (2026-10-08/09): a president at a desk sees the world layer, a governor the region or city they control; the president orders a power plant, the governor decides where; the logic should hold in single player too (answered "yes"); a server world's speed is set at world creation.
- **Builds on:** [0008](0008-controls-cameras.md) (multiplayer is the end goal), [0018](0018-sandbox-answers.md) (orders limited to the character's role), [0028](0028-births-military-mystery-tech-leaders.md) (officeholders are real pops), [0040](0040-pop-lore-custom-pops-and-player-slots.md) (a player slot is a pop), [0044](0044-information-scope-per-layer.md) (what each layer knows), [0014](0014-open-questions-answered.md) (orders need communications), [spec 39](../tier0/specs/39-orders-and-belief.md) (edicts).

## Decision

1. **The office decides which layers open.** A citizen plays their own life (Individual); a mayor sees their city; a governor their sector or region; a president or minister the country (Macro). The view shows what that office would know (0044).
2. **The desk is the way in.** A higher layer opens when the officeholder sits at their desk (an office, a command post, a radio room). Their body stays there: it gets hungry and tired and can be reached by what happens in the building.
3. **Orders go down the chain.** The president signs a programme and budget ("a power plant in this region"); it reaches the governor through working communications; the governor (or the AI in that chair) picks the site; pops take the jobs and build it. Each link is a pop with an office; an AI link uses the planner.
4. **Single player is multiplayer with AI in every other chair.** The player holds one office; the others are AI pops. Whether the player may change body or office follows the play style (0018). One logic, written once.
5. **A server world's speed is chosen at world creation** and is the same for everyone in it (one clock, spec 40); single player keeps the speed ladder. Durations stay real (0049), so a slow server makes Macro projects long: the choice belongs to whoever creates the world.

## Not decided here

The server itself, who may pause, join and leave rules for offices, and per-layer speed ranges in single player (proposed on 2026-10-08: caps when zooming in, a remembered speed per layer, symbolic flow tokens for planes and ships on the world map).
