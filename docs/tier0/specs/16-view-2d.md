# 16 · 2D view

**Status:** Implemented (stage 5 base; rules 4-13 on 2026-10-08, checked by screenshots only, not yet played by the user). **Stage:** 5 (base), then grows each stage. **Needs:** [09](09-site-map.md), [13](13-capabilities.md), [14](14-action-executor.md).

## Purpose

The Tier 0 window: SDL3 + Dear ImGui, top-down 2D, no Vulkan ([0010](../../decisions/0010-concept-v2.md)). It reads the simulation and sends it intents; it never changes state itself (contract rule 8). It must make every simulated value readable, because "can the player tell why?" is a kill/keep question.

## Screens and panels

| Part | What it shows or does |
| --- | --- |
| Map | Cells, edges (walls, doors, windows, barricades with damage), fixtures, items on floor, pops, walkers. One level at a time; switch with keys. Outdoors darkens at night |
| Perception fog | Areas the controlled pop cannot see are grey memory (tactical rule, [cameras](../../design/cameras-modes.md)). Debug toggle shows all |
| Overlays | Noise (heat map with the last loud events), light leak, power (powered/unpowered, load vs capacity) |
| Pop bar | Portraits of the group with key needs and wound icons; click to select, double click to control |
| Inspector | Body (a 17-region silhouette coloured by the worst state per region, a marker per wound, click a region to list its wounds, one "how I feel" line; masses and needs), Things (inventory), Doing (current action and queue), Mind (mood, fear). A separate debug inspector shows truth (hidden infection) |
| Context menu | Offers on the clicked target, greyed with reasons ([14](14-action-executor.md)) |
| Event log | Filterable; each line links to its cause chain ([25](25-chronicle.md)) |
| Time bar | Pause, 1×, 3×, 10×, rest-until-event; clock and day |
| Job board | Pops × 6 jobs with priorities ([24](24-pop-ai.md)) |

## Controls

- Direct control: WASD moves the controlled pop; mouse targets; E uses the default offer.
- Command: left click or drag selects pops; right click gives go; keys for hold, follow, attack.
- The Decided click scheme ([interaction](../../design/interaction.md#selection-targeting-and-control)) is kept alongside WASD: in direct control, right click on ground moves and right click on an object opens its menu; in command, middle click opens orders.
- Keyboard and mouse first; gamepad as a full path is designed in (abstract actions) but not tested in Tier 0.

## Rules

1. The view keeps no game state; closing and reopening it changes nothing.
2. A headless runner app runs the same simulation with no window (batch runs, AI-only runs, tests).
3. Placeholder art: coloured shapes and text; placeholders are marked as such.

Rules from the readability research (added 2026-10-08, [0055](../../decisions/0055-defaults-for-fixes-from-engineering-research.md) §2, [view-readability](../../research/view-readability.md); built the same day, [research fixes](../research-fixes.md) package 1). They answer the user's two complaints: the window is hard to read and WASD feels poor. All are view-side; rule 1 holds.

4. **Time from the pacer.** The window drives time with spec 40's `Pacer`: the ladder from `Pace::Real`, labels from `pace_name`, "1x" means real time. While a WASD key is held or the controlled pop walks to a target, a cap named `walking` holds speed at 3x; while a walker is within reach of the controlled pop, a cap `fight` holds it at Real. The time bar names the cap that limits ([VR-1..3](../../research/view-readability.md)).
5. **WASD walks.** WASD sends a `Walk` heading when the keys change and `-1` on release; the simulation walks cell to cell without stopping, slides along walls and refuses them ([50](50-movement.md) rules 5 and 6, which replaced the window-side step rule on 2026-10-08). The window flashes the edge that stopped the pop.
6. **WASD takes over.** A press while the controlled pop is busy sends `Cancel` and then the move; while paused the window shows "Paused: Space to run" ([VR-7](../../research/view-readability.md)).
7. **Follow camera.** The camera follows the controlled pop with frame-rate-independent smoothing (`k = 1 - exp(-dt / 0.1 s)`) and a dead zone of about 20 % of the view, clamped to the map; a middle-drag pan or PgUp/PgDn breaks the follow; F or Home recentres and resumes it; any WASD press resumes it. While following, the drawn level is the pop's level ([VR-12..22](../../research/view-readability.md)).
8. **Bodies move smoothly.** Pops and walkers are drawn between cells by their progress along the path (read-only); walkers sharing a cell are offset as pops are, or shown with a count ([VR-9..11](../../research/view-readability.md), [RN-40](../../research/renderer.md)).
9. **Shapes and colours, never colour alone.** Pops are discs with centred initials (radius at least 0.42 cell), walkers diamonds (active ones with a line to their target), objects outlined squares with two- or three-letter labels explained in a "Map key" panel; the controlled pop has a thick ring and an arrow, the selection a second ring. Group colours from Paul Tol's "bright" palette ([VR-28..33](../../research/view-readability.md), [art direction](../../design/art-direction.md)).
10. **State column.** The three to five worst states of the controlled pop are shown by it on the map and in the Group panel: hunger, thirst, fatigue, cold or heat, pain, bleeding, fear, and sickness from visible symptoms only (infection stays hidden, [21](21-infection.md)). Thresholds come from the body's existing bands; bars are coloured by those thresholds with a word, and morale is inverted so that worse is always the same way ([VR-35](../../research/view-readability.md), [PZ-13](../../research/project-zomboid.md)).
11. **Alerts pause.** A walker coming into sight of a group member, a bite and a group member down go to `Pacer::alert` and pause at normal speed; an alert column names the threat, its distance and direction ([VR-26, VR-38](../../research/view-readability.md)).
12. **Readable log and overlays.** Identical log lines within a game minute coalesce as "x N", wounds group by region; overlays carry a legend with scale and background, say "nothing above background" when empty, and light shows only cells above outdoor ambient. The hovered cell and the menu's cell are outlined; the controlled pop's path is drawn dotted ([VR-25, VR-34..36](../../research/view-readability.md)).
13. **Map size.** The map fits below the time bar and left of the panels and starts at about 32 px per cell, following ([VR-27](../../research/view-readability.md)).
14. **Action bar** (added 2026-10-08 after the user's playtest, [PZ-17](../../research/project-zomboid.md), [0049](../../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md)). A pop at work shows a bar over its head filled by `progress_s / total_s`; the controlled pop's bar is larger and carries the action's name and the game time left. The Inspector shows the same bar under "Doing". Approaching a job shows the job's name with an arrow, no bar. Read-only: the bar reads the action the simulation already keeps.

## Tests

- The headless runner and the windowed app reach the same state hash from the same inputs. (A1)
- Screenshot check per stage (manual): the user can read each new system on screen.
- Rules 4-13 change no golden (view only); `zs --shot` before and after, with `--overlay noise` and `--overlay light`, then the user plays it: WASD feels like walking, the camera follows, a walker is told from a pop at a glance. A passing test is not visual validation.

## Model as implemented (stage 5)

`apps/zs` (`zs`), built when `ZS_BUILD_WINDOW` is on (default on Windows). SDL3 3.4.8 and Dear ImGui 1.92.9 are fetched by CMake at pinned commits; the map is drawn with ImGui's draw lists over the SDL renderer. It starts the run (spec 26) for `--seed N` with the controlled pop possessed.

- Map: fills the space below the time bar, left of the panels and above the log, at 32 px per cell to start; floors darkened by the light falling on them (night outdoors is near black), walls, doors, windows, fences, barricades. Pops are discs in Tol "bright" colours with centred initials (one letter under 24 px), the controlled pop with a thick ring, an arrow and its path dotted, the selection with a yellow ring; walkers are red diamonds, bright with a line to their goal when not idle; bodies are drawn along their path between cells and spread apart when they share a cell. Objects are outlined squares with two- or three-letter labels (item spawn markers are not drawn), explained in a collapsible "Map key" panel. The hovered cell and the menu's cell are outlined; hovering lists what is there in words.
- Camera: follows the controlled pop (or the run's pop while the AI drives) with `k = 1 - exp(-dt / 0.1 s)` and a 20 % dead zone, clamped to the map, and the drawn level follows it; a middle-drag pan or PgUp/PgDn breaks the follow, F, Home or any WASD press resumes it.
- Overlays: noise margin over background, light only where brighter than outdoors (refreshed every 5 game minutes), each with a legend line that says so when nothing shows.
- Time bar: Pause, 1:1, 3x, 10x, 1 hour = 1 day (the start), 60x, 360x through the `Pacer`, with the speed actually run and the cap that holds it (`walking` 3x, `fight` 1:1), or the alert that paused it; rest until an alert or eight hours; place, grid, generator and fuel.
- Alerts: a walker coming into sight of a group member (again after five unseen game minutes), a bite on a member, a member down: they pause until Space and stay for a game hour in a column at the map's top right, with distance and direction.
- Controls: WASD steps to a neighbour only across a walkable edge (a diagonal needs both orthogonal edges, else the passable axis, else the wall flashes), one move per applied tick; a fresh press while busy cancels and steps; while paused a press shows "Paused: Space to run". Right click lists every target in the cell with its offers, greyed with the reason; "walk here"; C cancels; left click selects; wheel zooms to the cursor; Space pauses, continues or acknowledges an alert; 1-6 set speed; F5/F9 save (through `zs::write_file`) and load a file per seed.
- Inspector: name, current action, the AI's intent; need bars coloured by band with the word (fine, mild, bad, severe; "higher is worse" for every bar, morale shown as low morale); wounds grouped by region with a count (infection only when visible, or with the debug truth toggle, which also shows a turning), carried items.
- State column: the three to five worst states (thirst, hunger, fatigue, pain, bleeding, fear, sickness from visible symptoms) by the controlled pop on the map and under each member in the Group panel. Bands at 0.33, 0.6 and 0.85 are an estimate until the body names its own.
- Group panel: each member's state and worst states; "Control" possesses one; "Let the AI drive everyone" releases.
- Event log: the chronicle lines (`tier0::event_line`, shared with the headless app), coloured by kind, identical lines within a game minute coalesced as "x N", with a text filter.
- `--shot file.bmp [--ticks N] [--watch] [--menu x,y] [--overlay noise|light]` renders one frame offscreen, for checks without a screen.

**Known gaps:** no perception fog (everything is shown); no body silhouette (wounds are a list); no job board or orders (command mode); log lines do not link to cause chains; the A1 hash check between the window and the headless app is not automated (the window only uses `World::submit`, like the tests).

## Build on Windows

Needs Visual Studio 2022 (Desktop C++), CMake 3.25+ and Git. From the repo folder:

```
cmake -S . -B build
cmake --build build --config Release
build\engine\sim\tests\Release\zs_sim_tests.exe
build\apps\zs\Release\zs.exe --seed 5
```

The tests are the first MSVC check: the golden hashes must match Linux.

## Not in Tier 0

Isometric 3D, cutaways, animation, sound playback beyond simple cues.
