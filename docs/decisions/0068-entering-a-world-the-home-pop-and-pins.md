# 0068 · Entering a world: free cam starts at the world view, a pop starts in third person; leaving the body; the home pop and pinned pops

- **Date:** 2026-10-10
- **Status:** Decided by the user (2026-10-10). The user's words (lightly cleaned):

  > "When entering a world, I think it should be as free cam or with a pop. Free cam always starts the world in world view. Starting or entering the world with a pop starts the simulation on the other end, as a pop in third person. Detaching could be like leaving the body, an astral projection kind of analogy. A player should be able to pin pops, favourite them, with always a default or original one, which is the one used to enter the game. Only entering in free cam has no default character, but you should still be able to pin or favourite pops."

  Rule 5 and the numbers are defaults taken by Claude ([0052](0052-one-executable-one-simulation.md)): a later decision can change them.
- **Builds on:**
  - [0048](0048-realistic-time-default-commit-on-observation-and-pins.md) (pins);
  - [0061](0061-the-world-runs-without-the-player.md) (watching changes nothing);
  - [0062](0062-the-game-loop-attention-possession-and-late-starts.md) ("start as", one player per pop, late starts);
  - [0066](0066-player-states-embodied-detached-observer.md) (embodied, detached, observer; pinned windows; detachment levels);
  - [cameras and modes](../design/cameras-modes.md) (third person);
  - the [demo flow](../design/demo-flow.md);
  - research [simulation-and-free-will](../research/simulation-and-free-will.md) (FW-70 the player as the outside will; FW-77 pins and the replay).

## Decision

1. **Two ways in.** Entering a world (new, loaded or joined) asks how: **with a pop** or **free cam**.
   - **Free cam** enters the observer state (0066) at the **world view**: the Macro layer, the globe. It does so every time.
   - **With a pop** enters **embodied**, at the other end of the zoom: the Individual layer, in third person behind that pop, wherever it is at that moment. The pop comes from the scenario's "start as" choice (0062): a chosen pop, a created survivor, a random survivor or an office holder.
2. **The home pop.** The pop you enter with is your **home** (the "original").
   - It is always first among your pins and cannot be unpinned while it lives.
   - "Return to body" goes back to it from anywhere.
   - Entering free cam gives no home.
3. **Detaching is leaving the body.** Detaching (0066) is presented like astral projection: the camera rises out of the pop and away, and returning sinks back into it.
   - The body stays where it is and goes on living on its own AI and your standing orders, as 0066 already says.
   - While you are out, its pinned window stays open, and an alert about it calls you back (0066).
   - Possession is the outside will entering a pop; leaving the body hands the pop back to its own reasons (FW-70).
4. **Pins are favourites.** Every player can pin pops: with a home or without one, embodied or out of the body. Places, buildings and units can be pinned as well (0048).
   - The pin list lets you move the camera to a pin (0066 rule 5), open its window, or possess it if the mode allows.
   - What a pin shows follows your detachment level (0066 rule 3). In knowledge mode that is the pop as your side last knew it, with ages.
   - **Pinning shows; it never changes the world** (0061).
5. **Defaults for the edges** (Claude):
   - **Size.** The pin list has no fixed size. At most **4 pinned windows** are open at once (0066 says 3–4), and the home's window is one of them while you are out of the body.
   - **Only open windows count as attention** (0066). A pin whose window is closed is a bookmark; it raises no detail.
   - **Free cam needs the free level.** Entering free cam is offered where your detachment level is **free** (sandbox; the admin). In **knowledge** or **off** games you enter with a pop, because an observer with no pop has no side to know through.
   - **Off** (hardcore) cannot leave the body, so it has no free cam and no rising out. Pins still list the pops your pop knows.
   - **Possessing another pop does not move home.** Your home stays pinned and "return" goes back to it. You can make the pop you are in your home ("make home"); the old home stays as an ordinary pin. A free-cam player who possesses someone can make that pop home the same way.
   - **Multiplayer.** While you are out of the body, your home stays reserved to you (0062 rule 2): others can watch it but not take it. Any other pop you possessed is released when you leave it.
   - **When the home dies,** you are left out of the body above it, and its pin stays, marked dead. You choose a new home:
     - free level: any pop;
     - knowledge or off: a pop your side knows, or a random survivor (0062 rule 3).

     Until you choose, you stay out of the body.

## Consequences

- **Setup and entry.** The scenario setup's "start as" choice (0062) gains **free cam**. Loading or joining a world asks "with my pop or free cam", and offers free cam only where rule 5 allows it.
- **Saves.** The save keeps, per player, the home, the pin list (in order) and which windows are open, so a loaded world opens the way it was left.
- **Demo flow.** The [demo flow](../design/demo-flow.md) is the free-cam path (globe first). Entering with a pop is the other path, straight to a street corner in third person.
- **Detail and replay.** Rule 5 settles which pins count as attention: only open windows. Whether those windows change drawn outcomes stays the open question of FW-77 (focus and compression, Open). If they do, opening and closing a window is logged in the replay like an order.
- **Possession switch.** Spec 29's possession switch (not yet specified) carries the home, the "make home" command and the reservation of the home in multiplayer.
