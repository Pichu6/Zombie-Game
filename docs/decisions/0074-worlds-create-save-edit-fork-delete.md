# 0074 · Worlds: create, save, edit before play, fork after, delete; test on a new seed or keep testing on an old one

- **Date:** 2026-10-11
- **Status:** Decided by the user as intent ("I want to be able to create, save, edit?, delete world seeds, so I can test sandbox options and new worlds too, and keep testing in an existing seed"); the rules below are defaults taken by Claude ([0052](0052-one-executable-one-simulation.md)).
- **Builds on:** [spec 06](../tier0/specs/06-save-replay.md) (saves carry seed and settings; autosave per day), [sandbox parameters](../design/sandbox-parameters.md) §1.2 (setup is locked at the start; sandbox rules change tagged) and §1.5 (a run records its resolved settings and seed), [0062](0062-the-game-loop-attention-possession-and-late-starts.md), [0073](0073-the-atlas-is-the-front-of-the-engine-no-docs-in-builds.md).

## Decision

1. **A world is a folder.** `worlds/<name>/` next to the executable (user data, never in the repo): `world.toml` (name, seed, created, notes, the resolved settings copied from the presets at creation, the realism tag, which fields were touched) and `saves/` (manual saves and the per-day autosaves of spec 06 rule 18). Nothing else identifies a world; the seed alone does not, because the same seed with other settings is another world.
2. **Create.** Name, seed (random and shown, or typed) and settings: Auto by default, a preset, or fields edited on the cards as they become editable. Creating writes `world.toml` only; the planet is generated when the world is first opened, because generation is a pure function of seed and settings.
3. **Save.** F5 and the autosave write into the world's own `saves/`, named by game date and tick. A world opens from its start or from any of its saves, so testing continues on an existing seed exactly where it stopped. Today's `sandbox_seed<N>.sav` files are replaced by this.
4. **Edit.** Name and notes: always. Sandbox rules: always, tagged in the save and the chronicle (sandbox parameters §1.2). Setup parameters: freely while the world has no save; once it has one, editing a setup field **forks**: a new world folder with a new name, the same seed by default, the edited settings, and the original untouched. So a sandbox option is tested by forking a known world and changing one field.
5. **Delete.** Confirmed in the app, then the folder goes to the system recycle bin (not erased), so a slip is recoverable.
6. **Headless parity.** `zs --world <name> [--from <save>]` opens a world; `zs world new|list|fork|delete` do the rest from the command line; the Atlas's Worlds page uses the same code.
7. **Test fixtures stay fixtures.** The stage scenarios and goldens do not become worlds; `make_run(seed)` keeps building the one world for tests. A world's `world.toml` is what the Atlas's Play and Try entries pass to the same assembly.

## Consequences

- [Atlas](../design/atlas.md) gains §5a, the Worlds page, in slice 1 (list, create with seed, open from start or save, fork, delete) with the settings cards arriving in slice 5.
- Spec 06 gains the world folder and the save naming; the window's save path changes (a spec change, no golden moves).
