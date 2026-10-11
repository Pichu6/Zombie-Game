# 55 · The Atlas, slice 1: the front door, worlds, the tour, the try panel, the reader

**Status:** Implemented on MSVC (2026-10-11), written with its code; checked by offscreen shots of every page and one on-screen run. **Stage:** after Tier 0; the first slice of the [Atlas](../../design/atlas.md) design. **Needs:** [06](06-save-replay.md), [16](16-view-2d.md), [42](42-place.md), [52](52-land-water-capacity.md). **Decisions:** [0052](../../decisions/0052-one-executable-one-simulation.md), [0073](../../decisions/0073-the-atlas-is-the-front-of-the-engine-no-docs-in-builds.md), [0074](../../decisions/0074-worlds-create-save-edit-fork-delete.md).

## Purpose

`zs.exe` opened the Tier 0 window and nothing else. The Atlas is what opens now with no flags: a place to pick a world, take a friend through the project in five stops, press a button to run what was just built, and, when the repo sits next to the executable, read its docs. It owns no simulation state and writes nothing the engine reads; it is a view over the modes, the generators and the world folders.

## Rules

1. **The front door.** `zs.exe` with no arguments opens the Atlas. `--play`, or any window flag (`--seed`, `--watch`, `--shot`, `--world`, `--from`), opens the Tier 0 window as before. Headless flags and `gen` are unchanged. `zs world list|new|fork|delete` manages world folders from a terminal. `--atlas` opens the Atlas explicitly and takes `--shot out.bmp [--page P] [--stop N] [--try ID]` for checks without a screen.
2. **No docs in a build; workshop pages only next to a repo** ([0073](../../decisions/0073-the-atlas-is-the-front-of-the-engine-no-docs-in-builds.md)). The Atlas looks for a repo (a folder with `docs/status.md` and `CLAUDE.md`) above the data folder it was built from, then above the executable. With one, the nav shows Status and Docs and the Try panel shows the test entries; without one, those are absent and nothing else changes. `docs/` is never copied anywhere.
3. **Worlds as folders** ([0074](../../decisions/0074-worlds-create-save-edit-fork-delete.md)). `worlds/<name>/world.toml` next to the executable holds name, seed, created, built (the build's date), notes and `forked_from`; `saves/` holds the saves. A name is letters, digits, spaces, dots, dashes and underscores. The Atlas lists worlds newest first; New takes a name and a seed (random shown); Fork makes a new folder with the same seed and leaves the saves behind; Delete asks once and sends the folder to the recycle bin (to `worlds/_trash/` where there is none). With no world at all, "World 1" with seed 1 is made so nothing waits. The window, opened with `--world NAME`, takes the world's seed, writes F5 saves to `saves/<civil date>-tick<N>.sav` and loads the newest on F9; `--from FILE` loads that save at start.
4. **The Try panel reads `data/atlas/tries.toml`**: id, part, spec, title, what, `run` (the arguments after the program), `kind` (`window`, `text`, `image`, `test`), `added` and `workshop`. Entries sort newest first. `{seed}` is the panel's seed, `{world}` the selected world's name, `{out}` a folder for files (`atlas_out/` next to the executable). Every entry runs as a child process: the same executable with those arguments for `window`, `text` and `image`; `ctest` (the one the release build was configured with, from its cache) in the repo for `test`. Its output is read through a pipe and shown as it arrives, with the exit code and the seconds; an `image` entry also shows the file it wrote. Nothing runs on open.
5. **The reader** draws Markdown: headings, paragraphs, bullet and numbered lists with their indents, fenced code, tables, bold, code spans and links. Links between docs open in the reader (relative to the doc, `#anchors` dropped); web links open in the browser. A list of every `.md` under `docs/` with a search over names and contents; a back button. The map of parts links each part's design docs and specs (by number) into the reader.
6. **The map of parts reads `data/atlas/parts.toml`** (id, title, line, today, docs, specs): engine data, so a player sees the parts too. Grouping docs under parts in data rather than in the docs' front matter keeps this slice from touching thirty design docs.
7. **The Breakthroughs board reads `data/atlas/breakthroughs.toml`**: title, claim, evidence, barrier, push, status and an optional `try` whose button is "Measure again" (or "Show" for a window). Statuses: measured, built, built in part, decided, proposal.
8. **The Tour reads `data/atlas/tour.toml`**: stops with a title, what to say and what to show (`planet`, `page:<id>` or `try:<id>`). The planet stop generates the selected world's planet in a thread (grid 101, the generator's defaults, the same `CountryParams` as `zs gen world`), draws it with one colour per country, names the country under the cursor and opens a card on click: area, people, income group, endowment, coast, periods its capital was held, sectors and towns.
9. **One raster for the file and the screen.** `zs gen world --bmp` and the Atlas draw the planet through the same `render_planet`, so what the file shows is what the screen shows.
10. **Big textures are drawn in tiles** (16 × 8): one textured quad of 1,200 px overflows the software renderer's fixed-point triangle setup used by the offscreen shots, and the tiles cost nothing on a GPU.

11. **The strip (slice 2, 2026-10-11).** An open doc shows its path, the date and subject of the last commit that touched it (`git log -1`, through the same runner; nothing when git is not there) and "linked from": every doc whose text links to it, as buttons. Test entries need the release build next to the repo (`build/release/CMakeCache.txt`), not only the repo.

12. **The cards (slice 3, 2026-10-11).** A Cards page, and a double-click (or "Open its card") on the Tour's planet. Each card is a diff between the design and the build: `data/atlas/cards.toml` lists, per kind (world, country, town, pop, house), the rows the design asks for with the spec that produces each; the card fills a row from the generator's record, shows "not read yet" for a produced row the Atlas does not read, and "lacking" for a row with no producer, and counts the three. Values: the world from the planet's grid, people and history; a country from specs 48, 49 and 52 plus its calendar file (spec 53: rest days, holidays in 2027, the school year); a town from the planet's settlement (people, rank, cell, position) plus a town generated for its country's profile with 120 people (spec 47, seed = the world seed mixed with the cell), its streets, businesses, households and dwellings (spec 51), said on the card to be a stand-in until spec 48's towns carry their own people; a pop from the generated record and its dossier; a house from its dwelling and the site map `house_toml` writes. The simulation card waits for the Atlas to open a world in-process (slice 4 or later). `--card N --pick` with `--shot` selects a country, its first town, first pop and their house for a check.

## State

The Atlas keeps only view state: the page, the selected world, the Try seed, the running child processes and their output, the loaded textures, the open doc and its history, the tour stop. Worlds live in their folders; the output of a run lives in the child process until it ends.

## Code

`apps/zs/atlas.cpp` (the window and its pages), `atlas_md.cpp` (the reader), `runner.cpp` (child processes with their output), `worlds.cpp` (the folders and `zs world`), `planet_raster.cpp` (shared with `gen_cli.cpp`); `main.cpp` routes the flags; `window.cpp` takes `--play`, `--world` and `--from` and writes its saves into the world folder. Data: `data/atlas/{parts,tries,breakthroughs,tour}.toml`. Build: `build/msvc_atlas.bat` builds `zs` alone in release.

## Checks

- `zs world new "Test A" --seed 7`, `fork`, `list`, `delete`: the folder, its `world.toml`, the fork's `forked_from`, the recycle bin (2026-10-11).
- `zs --play --world "Test A"` then F5: `worlds/Test A/saves/Mon-11-Jan-2027-08-03-tick436.sav` (152,573 bytes) (2026-10-11).
- `zs --atlas --shot` of every page, of the tour's planet stop, of a `text` Try (`--bench`, 0.2 s, its line shown) and of an `image` Try (`gen world --bmp`, 9.0 s, the map shown) (2026-10-11).
- The Atlas on screen with the GPU renderer: the planet of seed 7 drawn whole; the generator's own count for seed 7 (348 countries) matches the Atlas's summary line (2026-10-11).
- The test Try `gen-tests` through the panel: ctest found from the build's cache, 1 of 1 passed in 39 s, its lines shown (2026-10-11, slice 2).
- Cards page shots: the world card (seed 7: 9 produced, 1 not read, 2 lacking), a country card with its calendar row and towns (11 produced, 0 not read, 5 lacking), and the picked chain down to a house with its site map (2026-10-11, slice 3).
- `zs gen world --seed 1 --freq 24 --bmp` still writes a 6,291,510-byte file through the shared raster (2026-10-11).

## Open

- The status table's "Try it" column is still written by hand; the generator from the manifest (atlas design §3 rule 3) is a repo tool to write when that table is next rewritten, so as not to rewrite `status.md` under another session.
- The simulation card (systems in wake order, next wake tick, save version, hash) needs a world open in-process; it waits for the compute board slice.
- `imgui_markdown` was not used: our docs lean on tables, which it lacks; the reader is our own and covers what the docs use.
