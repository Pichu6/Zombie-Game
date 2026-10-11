# The Atlas: the front door of `zs.exe`

What opens when you double-click `zs.exe` with no flags: a map of the project and of the world. One screen to see where everything stands, read what was written, press a button to test what was just built, open any country, town, pop or house and see what it has and what it still lacks, and watch the compute and breakthrough side of the project. The same screens are the first sandbox: once a card can be edited, it is the setup screen for that thing.

Labels: **Decided**, **Proposal**, **Implemented**, **Validated** (see CLAUDE.md). What the Atlas is, that it becomes the player's app, and that no build ships the docs are Decided ([0073](../decisions/0073-the-atlas-is-the-front-of-the-engine-no-docs-in-builds.md)); the pages, the manifest and the slices below are Proposals. Requested by the user on 2026-10-11 ("a visual representation of our project ... I click execute an .exe and there I can test, see docs, research, the game in each part ... the beginning of the sandbox options"). Builds on [0052](../decisions/0052-one-executable-one-simulation.md) (one executable), [sandbox parameters](sandbox-parameters.md) §1.3 (Auto, Preset, Custom), [content tools](content-tools.md) §2 (the tool set), the [compute budget](compute-budget.md) and the [ranked improvements](improvements-ranked.md).

## 1. Rules

1. **Not a second program.** The Atlas is a mode of `zs.exe` ([0052](../decisions/0052-one-executable-one-simulation.md) rule 1) and the default one: `zs.exe` alone opens the Atlas; `zs.exe --play` (or the Play button) opens the Tier 0 window as today. Every flag and `gen` command keeps working, and every one of them is also a button.
2. **Nothing written for the Atlas.** Everything it shows comes from a file the repo already maintains: `docs/status.md`, `docs/design/`, `docs/decisions/`, `docs/research/`, `docs/tier0/specs/`, `data/`, the generators and the tests. The one new file is the manifest of runnables (§3). If a page would need text written only for the Atlas, that text goes into the doc it belongs to, and the Atlas reads it there. This is what keeps it from rotting: when a doc changes, the Atlas changes.
3. **Engine pages and workshop pages** ([0073](../decisions/0073-the-atlas-is-the-front-of-the-engine-no-docs-in-builds.md)). Engine pages are for everyone and stay: the parts of the world, the cards, the playable modes, setup and creation. Workshop pages (the docs reader, decisions, research, tests, backlog, the breakthrough board) exist only when a repo sits next to the executable, and are expected to go or be replaced as their part is finished. **No build ships the docs:** `docs/` is never copied into `data/`, the `play` folder or a release; without a repo the workshop pages are absent and the engine pages work alone.
4. **Headless parity.** Every button runs the same executable with the same flags a terminal would (a child process whose output is read as it arrives), so the text it shows is the text the flag prints. The Atlas is a view over `engine/sim`, `engine/gen` and the modes; it owns no simulation state (CLAUDE.md, the core does not depend on the renderer).
5. **The labels are real.** Decided, Proposal, Implemented, Validated appear as badges, and each badge is read from the doc (a decision's status line, a spec's state, a `ctest` result), never set by hand in the Atlas.
6. **Cards before editing.** An entity card (§5) first only shows. Editing arrives field by field, under the sandbox rules (settings change numbers, never rules; Auto, Preset, Custom; one validator), and only once the record it edits is read by the simulation.

## 2. The map: the parts and their pages

The home screen is the project as a map of parts. Each part is one page, assembled from the docs that already exist. The same part list is the index of the design docs, so adding a design doc adds it to the map.

| Part | What the page assembles | Today |
| --- | --- | --- |
| **World** (Macro) | [world simulation](world-simulation.md), [world systems](world-systems.md), [world numbers](world-numbers.md), [macro layer](macro-layer.md), specs 30, 31, 48, 49, 52; `zs gen world` with the map drawn on screen (today a `.bmp`); the country cards | Grid, terrain, land, countries built; towns named and capacity next |
| **City** (the mid layer) | [infrastructure](infrastructure.md), [layer jumps](layer-jumps.md), [zoom continuum](zoom-continuum.md), specs 33, 34, 35, 47, 51; `zs gen town`, `zs gen houses`; the town and house cards | Establishments, projects, networks, dwellings built; streets and plots next |
| **Individual** (the immersive layer) | [body](body.md), [inhabitants](inhabitants.md), [activities](activities.md), [interaction](interaction.md), [cameras and modes](cameras-modes.md), [sensors and soundscape](sensors-and-soundscape.md), specs 02-27, 53, 54; Play; the pop card | The Tier 0 run; the window; the calendar; schedules next |
| **Simulation** | [engine architecture](engine-architecture.md), [focus and compression](focus-and-compression.md), [pop continuity](pop-continuity.md), specs 05, 06, 28, 40; time, saves, determinism; the test list | One world, wake ticks, pacer, saves; per-tick hashes and the harness next |
| **Generators** | [content tools](content-tools.md) §2 as a live table: one row per generator, its state, its data packs, a Run button with its options, the output in a panel | Names, pops, towns, world, land, countries, houses |
| **Stories** | [story explorer](story-explorer.md), [cases and living lore](cases-and-living-lore.md), [story blocks](story-blocks.md), [pop lore](pop-lore-and-custom-pops.md); `zs diary <pop>` when built | Designed |
| **Threats** | [threats](threats.md), [body trauma](body-trauma.md), spec 38; `--stage7`, `--group` | Outbreak built aggregate and entity |
| **Sandbox** | [sandbox parameters](sandbox-parameters.md) as a tree of cards: session, world, nations, cultures, pops, households, businesses, government, economy, infrastructure, outbreak; each field with its kind (setup, sandbox rule, policy, derived) and its band | Catalogue designed; demography preset built |
| **Compute and breakthroughs** | §6 | Measured 2026-10-10 |
| **Status and backlog** | `docs/status.md` rendered; [improvements ranked](improvements-ranked.md) as a sortable table; decisions as a timeline; research notes by ID with their confidence | All exist as docs |

How the parts tie together is one diagram on the home screen, drawn from the "Builds on" lines of the decisions and the cross-links of the design docs: a graph of docs, parts as clusters. Click a node, open the page.

## 3. The Try panel: "you can test it" is a button

The manifest `data/atlas/tries.toml` lists everything runnable. One entry per runnable:

```toml
[[try]]
id = "calendar-window"
part = "individual"
spec = "53"
title = "The calendar in the window"
what = "The date, the holidays and whether schools are open, in the time line"
run = ["--play", "--seed", "1"]
kind = "window"          # window | text | image | test
added = 2026-10-11

[[try]]
id = "world-map"
part = "world"
spec = "52"
title = "The planet with rivers and countries"
run = ["gen", "world", "--seed", "1", "--bmp", "{out}/world.bmp", "--political"]
kind = "image"
added = 2026-10-09

[[try]]
id = "sim-tests"
part = "simulation"
title = "Every simulation test"
run = ["ctest", "--preset", "release", "-R", "zs_sim_tests"]
kind = "test"
needs = "build"           # workshop entry: only shown when a repo and a build directory exist next to the exe
```

Rules:

1. **When a session ends with "you can test it", it adds an entry** (or updates one) with the date. The Atlas sorts the Try panel newest first, so the top of the list is what was just built. The hand-off file points at the entry's id.
2. **Kinds.** `window` opens the window mode in the same process; `text` runs the mode in a worker thread and shows its output in a panel, with the same text the flag prints; `image` runs it and shows the file it wrote; `test` runs `ctest` as a child process and streams the lines, green or red by the result.
3. **The status table's "Try it" column** (`docs/status.md`) is generated from this file, not the other way round, so the two cannot disagree.
4. **Seeds and options are editable** in the panel before running, with the entry's values as defaults. Each run is logged with its command line, so a result can be reproduced from the flag.
5. **Nothing runs on open.** Opening the Atlas costs nothing; the planet is generated when a card or a Try needs it, then cached for the session.

## 4. The reader

Docs are Markdown rendered in the window (Dear ImGui plus a small Markdown renderer: headings, lists, tables, links, code; `imgui_markdown` or our own, decided at the spec). Rules:

1. Links between docs navigate inside the Atlas; links to specs open the spec with its state badge; links to a decision open it with its date and status; a finding ID (`CK-247`) opens the research note at that finding and shows its confidence.
2. Every page has a "Where it lives" strip: the doc's path, its last commit date and message (read from `git log` when a repo is present), and the decisions that build on it.
3. Search across every doc, by text and by ID.
4. A Spanish toggle is not here: the repo is in English (CLAUDE.md). The Atlas is a reader of the repo.

## 5. Cards: everything a country needs

A card is one entity of the world at one layer, shown as **what it has, what it lacks, and where that is decided**. The fields come from the record the generator or the simulation already produces; the list of fields a thing *needs* comes from the design doc's table for it, so a card is a diff between the design and the build.

| Card | Opens from | Fields from | "Needs" list from |
| --- | --- | --- | --- |
| **Country** | the world map, the country list, a pop's nationality | spec 48 and 49 records, spec 43-46 tables, spec 52 endowment and income, spec 53 calendar | [macro layer](macro-layer.md) §6, [sandbox parameters](sandbox-parameters.md) nations; this is the country card the status step 6 already lists (CF-53) |
| **Town** | a country card, the map | spec 47, 48, 51: directory, households, businesses, dwellings | [world simulation](world-simulation.md) §12, [services by settlement size](../research/services-by-settlement-size.md) |
| **Pop** | a town's roster, the run's group, the window's selection | the pop record and dossier (`zs gen town` prints it today), the body, the lifepath, beliefs (spec 39), today's schedule (spec 54 when built) | [inhabitants](inhabitants.md), [content tools](content-tools.md) §2 full human generator row |
| **Household and house** | a pop, a street | spec 51: address, members, plan, rooms, furniture, food | [content tools](content-tools.md) interior and building rows |
| **Establishment** | a town, a pop's workplace | spec 33, 34, 35: recipe, staff, power, stalls | [infrastructure](infrastructure.md) |
| **The world** | home | spec 30, 31, 52: grid size, sliders, land share, runoff, people fed | [world numbers](world-numbers.md) |
| **The simulation** | the Simulation part, the Tour's stop 2 | spec 40 and 41: systems in wake order, their next wake tick, the save version, the hash; built 2026-10-11 ([spec 55](../tier0/specs/55-atlas.md) rule 15) with step, jump and the cold-world check | [engine architecture](engine-architecture.md) |

Each field row shows: value for the current seed, the spec that produces it, its badge, and (later) its sandbox kind. A field in the design table with no producer yet shows as **lacking**, with a link to where it is designed. Counting the lacking rows per card gives an honest "how complete is a country" number nobody has to maintain.

The card is the seed of the sandbox: the same country card, with editing on, is the Nations setup screen (Auto fills it, a preset replaces it, Custom edits a field and marks it with a dot, the validator colours the band). Editing is a later slice (§8), field by field, and only for fields the simulation reads.

## 5a. Worlds: create, open, fork, delete

Decided as intent, [0074](../decisions/0074-worlds-create-save-edit-fork-delete.md). The Worlds page is the first engine page a player meets after the Tour, and the first thing slice 1 builds, because "keep testing on an existing seed" needs it.

| Action | What happens | Where |
| --- | --- | --- |
| **New** | name, seed (random shown, or typed), settings Auto, a preset, or cards edited (slice 5); writes `worlds/<name>/world.toml`; nothing generated yet | `zs world new` |
| **Open** | from the start, or from any save in `worlds/<name>/saves/`; Play and every Try entry run on the open world | `zs --world <name> [--from <save>]` |
| **Save** | F5 and the daily autosave write into the world's `saves/`, named by game date and tick | spec 06 |
| **Edit** | name and notes always; sandbox rules always, tagged; setup fields freely until the first save | the cards |
| **Fork** | a setup edit on a world with saves makes a new world: new name, same seed by default, the edited settings, the original untouched | `zs world fork` |
| **Compare** | two worlds with the same seed listed side by side with the fields that differ (later slice) | |
| **Delete** | confirmed, then to the recycle bin | `zs world delete` |

The list shows for each world: name, seed, created, realism tag, touched fields count, last save's game date, and the badge of the build that made it (commit), so a world made by an older build says so. A world made by an older save format loads through spec 06's upgrade steps or says why it cannot.

## 6. Compute and breakthroughs

Two pages, both read from docs with a fixed table shape so the Atlas can render them as boards rather than prose.

**Compute** ([compute budget](compute-budget.md)): the measured table as it stands, plus a **Measure again** button per row that runs the same benchmark (`--bench`, `--pop-bench`, `--run` timed) on this PC and shows the new number beside the recorded one, with the commit each came from. The budget per tick table becomes a gauge: at each speed, how much of the per-tick budget today's block uses.

**Breakthroughs**: a new doc, `docs/design/breakthroughs.md`, one row per claim. The Atlas shows each as a card with its evidence badge:

| Claim (what nobody else did, as far as we know) | Evidence | Barrier | The next push | Status |
| --- | --- | --- | --- | --- |
| One simulation under three layers: a nation's output is the sum of what its pops' establishments produce, exact | specs 32-35 tests; [0012](../decisions/0012-world-simulation-layers.md) | Travel and ownership rows of [layer jumps](layer-jumps.md) still a gap | Rows 18 and 19 | Built in part |
| Cost is attention × speed, not world size: reading a lazy pop costs the same at 10 million as at 1 million | `--pop-bench` B2 | Chunks and threads not yet built; wake spikes unmeasured | Compute budget §6 measurements | Measured |
| A world that jumps a week in a few steps with the same hash as stepping | spec 40 tests | Per-tick hashes to debug divergence (backlog item 1) | Item 1 | Built |
| Coarse layers derived from fine rules, kept random, validated on held-out seeds | [EF](../research/coarse-layers-from-fine-rules.md), [TC](../research/two-contagions.md) | Not built; needs specs 38 and 39 only | EF-test 1 | Proposal |
| The simulation running ahead of the view, rolled back by orders | [RA](../research/perception-and-run-ahead.md) §5 | Needs items 1, 2, 4 of the backlog | A decision and a spec | Proposal |
| Stories found, never written: cases and diaries derived from what happened | [0067](../decisions/0067-stories-are-found-never-written.md) | Nothing built | `zs diary <pop>` | Decided |
| Days from real time-use data, depth bought by attention | [0069](../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md), [0071](../decisions/0071-days-from-time-use-data-dispositions-nudge.md) | Spec 54 not built | Spec 54 | Decided |

The rows above are a first draft for that doc; the doc owns them from then on, and the "Status" column must agree with the badges the Atlas reads elsewhere.

## 6a. The Tour: showing it to a friend

The first use of the Atlas (user, 2026-10-11): sit a friend down and say "this is what I am doing, this is how it works, here you test its current state, this is my breakthrough and why I think it is new, and this is where I am." The Tour is an engine page, not a workshop page: a player will take the same tour. It is five stops, and every stop is something the app shows or runs, never a slide.

| Stop | The friend hears | The app shows or runs |
| --- | --- | --- |
| 1. **What it is** | one world, one simulation, from the planet to one person's day; with or without an outbreak | `gen world --bmp --political` drawn on screen: the planet with rivers and countries; click a country, its card opens; click its town, a pop, their house |
| 2. **How it works** | every layer is the same rules seen from farther away; a nation's output is the sum of its pops' establishments; a cold week costs a few steps and gives the same hash as stepping | the Simulation card: systems in wake order; a Try that jumps a week cold and then steps it, both hashes side by side |
| 3. **Test it now** | make a world with a seed, press Play, or watch the 14-day run as text; reopen it later where it stopped | the Try panel's engine entries: Play (`--play --seed N`), the run with its chronicle (`--run`), a town with every dossier (`gen town`) |
| 4. **The breakthrough** | cost is attention × speed, not world size; reading a person costs the same at 10 million as at 1 million; coarse layers derived from fine rules; the simulation can run ahead of the view | the Breakthroughs board (§6) with Measure again live on the friend's question: "a million people?" press, 0.4 s |
| 5. **Where I am** | what is built, what is designed, what is next | the map of parts with its badges and lacking counts per card; the status page's "Next step" (workshop only; the engine version is the badges) |

**The question ladder.** A friend asks at two heights, and the Tour must answer both from the same screens:

| Broad | Answered by | Granular | Answered by |
| --- | --- | --- | --- |
| "How big is the world?" | the world card: 102,012 cells, 70 km apart, 92 countries | "What is this country's income, and why?" | the country card: endowment, access, history (spec 52) |
| "How do people live?" | the Individual part: body, needs, jobs, the calendar | "What is this pop doing at 3 pm on a Tuesday, and where?" | the pop card's schedule (spec 54 when built; the dossier today) |
| "What happens in an outbreak?" | the Threats part, spec 38's reproduction number | "Why did this pop die on day 9?" | the run's chronicle and death screen (spec 25, 26), opened from the Try panel |
| "Is it fast?" | the Compute board, the budget gauge | "How long does one tick take with a million people?" | Measure again on that row |
| "What is new about it?" | the Breakthroughs board | "Show me the test that proves the week jump matches stepping" | the test entry in the Try panel (workshop), or its two hashes (engine) |

Rule: every granular answer is a card, a run or a measurement, so the friend can push one step further ("and that pop's house?") and the app still answers. If a question has no screen, that is a lacking row, and it goes on the backlog.

## 7. Where it lives in the code

| What | Where | Note |
| --- | --- | --- |
| The mode | `apps/zs/atlas.cpp`, entered from `main.cpp` when no flag is given | SDL3 + Dear ImGui as the window today; the 2D window is `--play` (built, [spec 55](../tier0/specs/55-atlas.md)) |
| The reader | `apps/zs/atlas_md.*` | Our own small renderer (headings, lists, tables, code, links); links resolved against `docs/` |
| The docs | `docs/` when a repo is found next to the exe (walk up to the repo root); otherwise no workshop pages | Never copied into a build ([0073](../decisions/0073-the-atlas-is-the-front-of-the-engine-no-docs-in-builds.md)) |
| The manifest | `data/atlas/tries.toml` | Read with tomlplusplus like every pack |
| Cards | `apps/zs/atlas_cards.*` (slice 3) | Read-only views over `engine/gen` and `engine/sim` records; no state of their own |
| Runs | `apps/zs/runner.*` | every kind as a child process (the executable itself, or `ctest` in the repo), its output read through a pipe; the command line shown with the output |

No new engine code. Fields a card shows that the engine does not expose yet get a small accessor in the engine, specified like any other unit.

## 8. Slices, in order

1. **Built 2026-10-11 ([spec 55](../tier0/specs/55-atlas.md)). The front door, the Worlds page, the Tour and the Try panel.** `zs.exe` opens the Atlas; the Worlds page (§5a: list, new with a seed, open from start or save, fork, delete; saves move into the world folder); the map of parts; the Tour's five stops (§6a) over what exists today: the planet drawn on screen with countries clickable, the engine Try entries (`window`, `text`, `image` kinds) and the Breakthroughs board read from its doc; the workshop reader with links, badges and search; the status table's Try column generated. Spec first. Small to medium: the planet on screen and the country click are the only new drawing.
2. **Built 2026-10-11 (spec 55 rule 11). Tests and the dev strip.** The `test` kind, the Where-it-lives strip with git dates, the build detection (`needs = "build"`). Small.
3. **Built 2026-10-11 (spec 55 rule 12; the simulation card waits). Cards, read-only.** Country, town, pop, house, establishment, world, simulation; the lacking rows from the design tables; the question ladder's granular column answered end to end. This is also CF-53, the country card status step 6 already lists. Medium.
4. **Built 2026-10-11 (spec 55 rule 13). Compute and breakthroughs.** The Breakthroughs board came with slice 1 (its rows live in `data/atlas/breakthroughs.toml`, engine data rather than a doc, so a build without the repo shows them too: 0073); slice 4 adds the Compute board from `data/atlas/compute.toml`, Measure again per benchmark and the budget gauge. Small.
5. **First slice built 2026-10-11 (spec 55 rule 14: the world's settings, presets, fork when locked; country and pop editing wait for the simulation to read those fields). Editing on the cards.** Field by field, under the sandbox rules; a preset saved from an edited card; the validator's bands shown. This is the start of the Sandbox setup screens, and the first slice of the no-outbreak game's settings. Medium, after the simulation reads the fields.

Slices 1 and 2 need no generator or simulation work and do not re-pin any golden; they can go between spec 54 and the next generator step without blocking either.

## Open

- Markdown renderer: `imgui_markdown` (single header, MIT) covers headings, links, emphasis and images; tables are not in it. Our docs lean on tables. Either extend it or write our own small one at the spec.
- The parts list of §2 is grouped in `data/atlas/parts.toml` (engine data, so a player sees the parts too), not in the docs' front matter: slice 1 did not touch thirty design docs for it. A `part:` line per doc can replace it later if the data file drifts.
