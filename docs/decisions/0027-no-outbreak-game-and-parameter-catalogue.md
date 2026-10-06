# 0027 · The game must run without an outbreak; one parameter catalogue

- **Date:** 2026-10-06
- **Status:** Decided by the user (intent below, from their message in chat). The catalogue in [sandbox parameters](../design/sandbox-parameters.md) is a Proposal.
- **Extends:** [0016](0016-sandbox-setup-and-layer-flow.md), [0018](0018-sandbox-answers.md), [0022](0022-macro-layer-intent.md). Makes the "no virus" option of 0018 a design requirement for every system, not a switch at the end.

## Decisions

- **No-outbreak is a full game:** the simulation and gameplay must run without an outbreak, as a politics, grand-strategy and Sims-like game, on the same world and the same rules.
- **Everything is configurable at setup:** sandbox options, world creation, nations, businesses, pops, corporations, groups and more. The outbreak becomes one parameter group among them.
- **Parameters follow the existing rule:** settings change numbers, never rules.

## Proposals recorded with it (not decided)

- Four kinds of parameter (setup, sandbox rule, policy, derived), three bands (realistic, sandbox, rejected), TOML presets under `data/setup/`.
- The group, business and corporation records, the shared ideology axes, the long time steps, and the no-outbreak presets.
- Six open questions (births, war, late outbreak trigger, technology, build order, player in a corporation) in [sandbox parameters](../design/sandbox-parameters.md) §15.

## Why

The user wants the world to be a laboratory and a game in its own right: the zombies are one of the things that can happen to it, not the reason it exists.
