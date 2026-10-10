# 0066 · Player states: embodied, detached, observer; stations gate deciding, not looking; detachment is a permission

- **Date:** 2026-10-09 (recorded 2026-10-10)
- **Status:** Decided by the user as intent (2026-10-09); the three states, the defaults and the save mark are defaults taken by Claude ([0052](0052-one-executable-one-simulation.md)). The user's words (lightly cleaned):
  - "In world view, it should be more of a god view, like a Google Earth board game kind of thing… I click a country, I see who the president is, I click to see where he is and what he's doing, I click to play as him. I create a survivor, I start as a pop; when reaching the requirements… I can access higher levels."
  - "The issue I see is from micro to macro. I understand that going up needs a station, but that is not required from macro to micro. If I make a simulation and just watch it, I can go and choose whatever level of gameplay or pop I want to play, or not, like a god just looking at what's happening… there's a level of detachment I feel is missing."
  - "Survivor/free could be like a single-player admin in multiplayer kind of thing? Survivor/knowledge and hardcore/off could lead to good role play."
- **Builds on:** [0019](0019-activities-and-delegation.md) (standing orders), [0042](0042-pop-presentation-per-layer.md), [0044](0044-information-scope-per-layer.md), [0048](0048-realistic-time-default-commit-on-observation-and-pins.md) (pins), [0058](0058-offices-open-layers-single-player-is-multiplayer-with-ai.md) (stations), [0060](0060-one-fight-three-views-behaviour-from-jobs-and-skills.md), [0061](0061-the-world-runs-without-the-player.md), [0062](0062-the-game-loop-attention-possession-and-late-starts.md), research [playing-knowing-commanding-moving](../research/playing-knowing-commanding-moving.md) (PKC-1..PKC-35, PKC-145, PKC-146).

## Decision

1. **Stations gate deciding, not looking.** A station (desk, command post, radio room) opens what a pop can know and order in the layers above; it never limits where the camera may go.
2. **Three player states, plus pinned windows:**
   - **Embodied:** you are the pop and decide its actions; its station, if it has one, opens its office.
   - **Detached:** you step out of the pop at any time, with no station needed. The camera is free on every layer and you decide nothing. The pop goes on living on its own AI and your standing orders (0019, 0061); an alert about your body calls you back.
   - **Observer:** no pop at all. God view; you may possess anyone the mode allows.
   - **Pinned windows:** small live isometric views of your own body (always shown while detached or at a station) and of any pop, building or unit you pin. Each pinned place counts as attention, so it is computed in detail; about 3–4 at most. In knowledge mode only what your side can watch can be pinned.
3. **What a detached or observing player sees is a detachment level:** **free** (the truth), **knowledge** (what your pop or side knows, stale and with ages) or **off** (only the body's senses, like Project Zomboid). Live eyes elsewhere come only from the body's senses or diegetic devices (radio, CCTV, binoculars, drones).
4. **The level is a permission per player, not one setting per world.**
   - **free** is the admin or game-master role: the server owner and moderators in multiplayer; in single player you are the admin of your own world.
   - **knowledge** and **off** are the role-play levels and the defaults for players on a server. Defaults: sandbox free; survivor and multiplayer knowledge.
   - Watching never changes anything (0061). An admin acts only through sandbox tools, which are inputs and are logged in the replay.
   - **Lowering** the level is allowed at any time. **Raising** it to free in a knowledge or off game marks the save, like Paradox's Ironman (PKC-30), so role-play and challenge runs stay honest. Levels are presets over separate permissions, and the save records which were used and when (PKC-145).
5. **Moving the camera** to a far pop or place goes zoom out, pan, zoom in, cutting directly when close (PKC-35, PKC-146).

## Consequences

- Every view reads the belief store in knowledge mode, never the truth (PKC-144); search, counts and pins must not leak hidden things (LR-26).
- Open, pinned by the user for later (cameras-modes Open): achievements or a public integrity mark, or only the private record in the save (PKC-145).
- The story explorer's visibility rules follow these levels ([story explorer](../design/story-explorer.md) §2 rule 6).
