# 0040 · Pop lore shapes the world, custom pops, and player slots

- **Date:** 2026-10-07
- **Status:** Decided by the user (intent below, from their messages in chat). The mechanisms are Proposals in [pop lore and custom pops](../design/pop-lore-and-custom-pops.md).
- **Extends:** [0036](0036-worlds-start-with-nations-and-everything-follows-logic.md), [0039](0039-fill-the-layer-jumps.md); builds on [spec 28](../tier0/specs/28-pop-levels.md) and [spec 29](../tier0/specs/29-embodied-pops.md).

## Decisions

1. **A pop's background and traits give the world around it story and lore depth.** The simulation adapts around the pop's story, not the reverse.
2. **The user can make a custom pop and add it to a new world,** like XCOM's character pool: it can be there from the start of the run or appear later (for example as a recruit, a survivor or a stranger). It must make lore sense in the simulation.
3. **A disembodied pop can serve as a multiplayer player slot.** A slot links an account to a pop; the body is borrowed from the pool.

## Notes (not decisions)

- Adapting must never contradict committed state (ledger, death records, saved bodies). That rule, hooks, the entry paths and the player-slot rules are Proposals.
