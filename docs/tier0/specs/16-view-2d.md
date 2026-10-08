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

## Tests

- The headless runner and the windowed app reach the same state hash from the same inputs. (A1)
- Screenshot check per stage (manual): the user can read each new system on screen.

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
build\apps\sandbox\Release\zs.exe --seed 5
```

The tests are the first MSVC check: the golden hashes must match Linux.

## Not in Tier 0

Isometric 3D, cutaways, animation, sound playback beyond simple cues.
