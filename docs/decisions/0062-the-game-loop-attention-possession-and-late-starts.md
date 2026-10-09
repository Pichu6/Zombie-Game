# 0062 · The game loop: create, watch, jump into pops; one player per pop; late starts; attention is not decision

- **Date:** 2026-10-09
- **Status:** Decided by the user (2026-10-09): "you create a world, set its parameters and start the simulation; you can watch it, speed up time, etc. You can select a president or a pop of interest to access gameplay decisions, jumping from pop to pop if you want to administer something personally, reaching the designated station that opens the upper-level management, RTS or god-mode map. In multiplayer, a pop possessed by a player is restricted for others. You can play a random survivor in a 'ten years after' scenario, or pay no attention to the upper levels, focusing on activities: hunting, studying flora and fauna, farming, fishing. The upper levels keep running because they bring repercussions below, but our focus is elsewhere: that is where our consciousness is, what it watches and hears, regardless of decision making."
- **Builds on:** [0016](0016-sandbox-setup-and-layer-flow.md) (any combination of layers; from building to body; possession gives objectives), [0019](0019-activities-and-delegation.md) (activities), [0040](0040-pop-lore-custom-pops-and-player-slots.md) (a player slot is a pop), [0058](0058-offices-open-layers-single-player-is-multiplayer-with-ai.md) (the desk opens the layer), [0061](0061-the-world-runs-without-the-player.md) (the world runs without the player), [focus and compression](../design/focus-and-compression.md) (attention sets detail).

## Decision

1. **The loop.** Create a world (its parameters), start it, watch it at any speed. Select any pop (a president, a mayor, a survivor) to see it and, by possessing it, to act through its role; jump from pop to pop at will. Reaching that pop's station (desk, command post, radio room) opens its upper-layer view: management, RTS, or the world map (0058).
2. **One player per pop.** In multiplayer a possessed pop belongs to that player until released; others can watch it within their own scope but cannot take it.
3. **Late starts.** A scenario may begin years into the world (for example "ten years after" the outbreak): the world is created at its start date and run forward, cold and exact where the jump forms allow (spec 40), before control is handed to the player, who may start as a random survivor.
4. **Attention is not decision.** Where the player's consciousness is (the body or place it watches and hears) sets what is detailed and shown. Deciding is a separate thing, done only through a possessed pop's role. A player may spend the whole game on activities (hunting, farming, fishing, studying plants and animals) and never decide anything above; the upper layers keep running and their consequences still arrive.

## Consequences

- The scenario setup gains a start date relative to the outbreak and a "start as" choice (a chosen pop, a random survivor, an office).
- Running the world forward for years before play is a performance target for the jump forms (spec 40 Open 2) and for the macro systems.
- Activities need to be full play in themselves (0019), not filler between decisions.
