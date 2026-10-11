# 0073 · The Atlas is the front of the engine: it evolves into the player's app, and no build ships the docs

- **Date:** 2026-10-11
- **Status:** Decided by the user. The user's words (lightly cleaned): "I would not like docs to be released with any build. I just want the app to evolve to be how the user uses the engine itself; things now will surely go when finished, others newly made. The app should evolve to have the sandbox options, creation, test, etc." The name "Atlas" is fine for now and can change as it evolves.
- **Builds on:** [0052](0052-one-executable-one-simulation.md) (one executable), [sandbox parameters](../design/sandbox-parameters.md) §1.3 (Auto, Preset, Custom), the [Atlas](../design/atlas.md) design.

## Decision

1. **The Atlas is the front door of `zs.exe`** and the thing that becomes the player's way of using the engine: sandbox options, world and entity creation, testing a world, playing it. It is not an internal dashboard that is thrown away later; it is the first shape of the final app.
2. **Two kinds of pages, one window.** *Engine pages* are for everyone and stay: the map of parts as the world's parts, cards, the Try panel's runnable modes, setup and creation. *Workshop pages* exist only while the repo sits next to the executable: the docs reader, the decisions and research, the tests, the backlog, the breakthrough board. A workshop page is expected to disappear or be replaced when its part is finished.
3. **No build ships the docs.** `docs/` is never copied into `data/` or into the `play` folder or any release. The Atlas reads `docs/` only when it finds a repo next to the executable; without one, the workshop pages are simply absent and the engine pages work alone.
4. **Names are provisional.** "Atlas" and the names of its pages may change as the app evolves; the decision is about what it is, not what it is called.

## Consequences

- [Atlas](../design/atlas.md) rules 2 and 3 and its Open section are updated: no `data/docs/`, one reader with one source.
- The Try manifest (`data/atlas/tries.toml`) is engine data: it lists playable modes for everyone and marks test and workshop entries, which the Atlas hides without a repo.
- The status table's "Try it" column is still generated from the manifest; that generation is a repo tool, not something the exe does.
