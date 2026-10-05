# 0010 · Concept v2: realistic survival, one body model, Tier 0 first

- **Date:** 2026-10-05
- **Status:** Decided by the user
- **Supersedes:** the open questions in the earlier vision (hero system, world scale for the first build) and the earlier vertical slice in `status.md` (pole, grenade in the pump room, both cameras).

## Decisions

- **Hero:** the player's own survivor, as in Project Zomboid. The game asks "how long can you really survive?": its content is the realistic ways to die that movies and games ignore.
- **Identity:** start alone. When you find or recruit others, an XCOM-like commander layer opens: manage the group and building from above, and drop into any survivor to play them directly.
- **Real time**, no turns.
- **Parity:** every survivor can survive exactly like the player, and the player can do everything any survivor can. One body model, one action set, one set of survival rules; the AI only chooses actions. (Confirms [0005](0005-finite-world-parity.md).)
- **Everything built is scavengable and craftable**, all tied by one quality system: walls, floors, houses, pipe networks, power lines, car engines, vehicles, tools and items. Crafting and repair depend on job-related knowledge and skill. This generalises the infrastructure "golden rule" to the whole world and extends [0006](0006-crafting-quality.md).
- **Time scale:** 1 real hour per game day at 1×, plus faster speeds; also a sandbox setting.
- **Map:** small and hand-made; every building enterable; breachable walls.
- **Sandbox settings**, Zomboid style, for zombie speed, time scale and later others. Settings change numbers, never rules. Zombie speed default: body-driven.
- **Tier 0 first:** a 2D simulation (SDL3 + ImGui, no Vulkan) that must answer the fun questions before any renderer work.

## Why

The user felt the previous vision was spread across too many ideas and too ambitious for a custom engine. A proposer/critic review found the old first playable far too big for one person with AI. These decisions keep every idea but schedule it by tier, and prove the fun in 2D first.

## Consequences

- The simulation core must run every survivor's full survival sim, so simulation level of detail (near vs far) is needed early.
- Structures are assemblies in the item data model, not static level geometry.
- 3rd-person, clothing fit, co-op and vehicles stay in the plan, in later tiers.

Details: [concept v2](../research/concept-v2.md), [vision](../vision.md).
