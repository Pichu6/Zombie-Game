# 16 · 2D view

**Status:** Implemented (stage 5 base; not yet run on the user's PC). **Stage:** 5 (base), then grows each stage. **Needs:** [09](09-site-map.md), [13](13-capabilities.md), [14](14-action-executor.md).

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

Rules from the readability research (added 2026-10-08, [0055](../../decisions/0055-defaults-for-fixes-from-engineering-research.md) §2, [view-readability](../../research/view-readability.md); not built yet, [research fixes](../research-fixes.md) package 1). They answer the user's two complaints: the window is hard to read and WASD feels poor. All are view-side; rule 1 holds.

4. **Time from the pacer.** The window drives time with spec 40's `Pacer`: the ladder from `Pace::Real`, labels from `pace_name`, "1x" means real time. While a WASD key is held or the controlled pop walks to a target, a cap named `walking` holds speed at 3x; while a walker is within reach of the controlled pop, a cap `fight` holds it at Real. The time bar names the cap that limits ([VR-1..3](../../research/view-readability.md)).
5. **WASD refuses walls.** A press moves only if the edge to the neighbour cell is walkable; a diagonal only if both orthogonal edges are, else along the one passable axis, else nothing, and the blocking edge flashes. It is a `MoveTo` to the neighbour; the next move is sent only after the clock passes the tick the last one was sent at ([VR-4..6, VR-8](../../research/view-readability.md)).
6. **WASD takes over.** A press while the controlled pop is busy sends `Cancel` and then the move; while paused the window shows "Paused: Space to run" ([VR-7](../../research/view-readability.md)).
7. **Follow camera.** The camera follows the controlled pop with frame-rate-independent smoothing (`k = 1 - exp(-dt / 0.1 s)`) and a dead zone of about 20 % of the view, clamped to the map; a middle-drag pan or PgUp/PgDn breaks the follow; F or Home recentres and resumes it; any WASD press resumes it. While following, the drawn level is the pop's level ([VR-12..22](../../research/view-readability.md)).
8. **Bodies move smoothly.** Pops and walkers are drawn between cells by their progress along the path (read-only); walkers sharing a cell are offset as pops are, or shown with a count ([VR-9..11](../../research/view-readability.md), [RN-40](../../research/renderer.md)).
9. **Shapes and colours, never colour alone.** Pops are discs with centred initials (radius at least 0.42 cell), walkers diamonds (active ones with a line to their target), objects outlined squares with two- or three-letter labels explained in a "Map key" panel; the controlled pop has a thick ring and an arrow, the selection a second ring. Group colours from Paul Tol's "bright" palette ([VR-28..33](../../research/view-readability.md), [art direction](../../design/art-direction.md)).
10. **State column.** The three to five worst states of the controlled pop are shown by it on the map and in the Group panel: hunger, thirst, fatigue, cold or heat, pain, bleeding, fear, and sickness from visible symptoms only (infection stays hidden, [21](21-infection.md)). Thresholds come from the body's existing bands; bars are coloured by those thresholds with a word, and morale is inverted so that worse is always the same way ([VR-35](../../research/view-readability.md), [PZ-13](../../research/project-zomboid.md)).
11. **Alerts pause.** A walker coming into sight of a group member, a bite and a group member down go to `Pacer::alert` and pause at normal speed; an alert column names the threat, its distance and direction ([VR-26, VR-38](../../research/view-readability.md)).
12. **Readable log and overlays.** Identical log lines within a game minute coalesce as "x N", wounds group by region; overlays carry a legend with scale and background, say "nothing above background" when empty, and light shows only cells above outdoor ambient. The hovered cell and the menu's cell are outlined; the controlled pop's path is drawn dotted ([VR-25, VR-34..36](../../research/view-readability.md)).
13. **Map size.** The map fits below the time bar and left of the panels and starts at about 32 px per cell, following ([VR-27](../../research/view-readability.md)).

## Tests

- The headless runner and the windowed app reach the same state hash from the same inputs. (A1)
- Screenshot check per stage (manual): the user can read each new system on screen.
- Rules 4-13 change no golden (view only); `zs --shot` before and after, with `--overlay noise` and `--overlay light`, then the user plays it: WASD feels like walking, the camera follows, a walker is told from a pop at a glance. A passing test is not visual validation.

## Model as implemented (stage 5)

`apps/zs` (`zs`), built when `ZS_BUILD_WINDOW` is on (default on Windows). SDL3 3.4.8 and Dear ImGui 1.92.9 are fetched by CMake at pinned commits; the map is drawn with ImGui's draw lists over the SDL renderer. It starts the run (spec 26) for `--seed N` with the controlled pop possessed.

- Map: floors darkened by the light falling on them (night outdoors is near black), walls, doors, windows, fences, barricades, map objects as lettered squares, items as dots, walkers (bright when not idle), the group in member colours with initials and a ring on the controlled pop. Hovering a cell lists what is there in words. Levels with PgUp/PgDn.
- Overlays: noise margin over background, light (refreshed every 5 game minutes).
- Time bar: pause, 1× (one real hour = one game day), 3×, 10×, 60×, 360×, rest until an alert or eight hours; grid, generator and fuel.
- Controls: WASD steps the controlled pop when it is free; right click lists every target in the cell (objects, items, bodies, edges on its sides, the floor) with its offers, greyed with the reason; "walk here"; C cancels; left click selects; wheel zooms; middle drag pans; Space and 1-6 set speed; F5/F9 save and load a file per seed.
- Inspector: name, current action, the AI's intent, needs, blood lost, fear, morale, wounds (infection only when visible, or with the debug truth toggle, which also shows a turning), carried items.
- Group panel: each member's state; "Control" possesses one; "Let the AI drive everyone" releases.
- Event log: the chronicle lines (`tier0::event_line`, shared with the headless app), coloured by kind, with a text filter.
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
