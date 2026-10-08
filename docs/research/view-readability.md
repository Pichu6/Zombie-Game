# Readability of the 2D top-down view: state, selection, alerts, overlays, and what our window lacks

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [spec 16](../tier0/specs/16-view-2d.md), [interaction](../design/interaction.md), [cameras and modes](../design/cameras-modes.md), [art direction](../design/art-direction.md), [engine architecture](../design/engine-architecture.md), [spec 40](../tier0/specs/40-time.md). **Evidence:** our own `apps/zs/window.cpp` read in full at commit `8295162`; the window built offscreen on Linux (GCC 13, SDL3 3.4.8 with `-DSDL_X11=OFF -DSDL_WAYLAND=OFF -DSDL_UNIX_CONSOLE_BUILD=ON`, since the container has no X11/Wayland dev headers) and eight `zs --shot` frames taken at 1440x900 and looked at; two scratch harnesses linked against `zs_sim`/`zs_gen` that replay the window's frame loop and WASD block headlessly; palette contrast and colour-vision simulation measured with colorspacious 1.1.2 (Machado et al. 2009 model); camera and UI code read in OpenTTD, OpenRCT2, Cataclysm: DDA, Mindustry, Godot and Unity's C# reference (commits in Sources); RimWorld, Prison Architect and Zomboid UI from web-search summaries only (`reported`). Nothing was changed in the repo's code. **IDs:** VR.

The user checked the window: "WASD movement is poor and it is hard to tell what is going on" ([status](../status.md)). This note measures why, reads how open-source top-down games move cameras and show state, and gives prioritised fixes with `window.cpp` line numbers.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| VR-1 | The window's "1x" is 48 ticks per real second = 24 game s per real s (one real hour per game day); the spec 40 ladder's 1:1 (`Pace::Real`) is not offered, and the window does not use the `Pacer` | sourced | `apps/zs/window.cpp:35-36,447` @ 8295162; `engine/sim/include/zs/pacer.h`; spec 40 header |
| VR-2 | A pop walks 1.35 m/s (height, age, load scaled) and cells are 1 m, so at the window's "1x" a walking pop crosses 32 cells (the whole 32x28 m block) in about 1 real second | derived | VR-1; `engine/sim/src/body.cpp:613` (`walk_speed`); `data/tier0/map/README.md` (1 m cells) |
| VR-3 | Measured: holding D for 3 real s at "1x" moves the pop 76 cells at 60 fps and 58 at 144 fps (19-25 cells per real second); at 3x 164-233 cells; at 60x and 360x 2.4-12 game hours pass in those 3 seconds while the pop still steps at most one cell per frame | sourced | harness `wasd.cpp` (scratch), replaying `window.cpp:149-168` and `:401-413`, seed 1, Linux GCC 13 |
| VR-4 | WASD sends `MoveTo` to the neighbouring cell whenever that cell is walkable, without checking the edge between; `MoveTo` path-finds, so pressing towards a wall with floor behind it walks the pop around through doors | sourced | `window.cpp:409-412`; `engine/sim/src/actions.cpp:599-610` (`find_path`) |
| VR-5 | Measured on level 0 of the block: 372 of 3,464 orthogonal WASD steps to a walkable neighbour (10.7 %) are behind a wall or fence; the detour they trigger is 16.5 cells on average and up to 47 cells | sourced | harness `detour.cpp` (scratch) over `SiteMap::can_walk`/`find_path`, seed 1 |
| VR-6 | W+A (any diagonal) sends a diagonal goal to a 4-neighbour path finder; 161 of 837 diagonal presses (19 %) produce a path longer than two cells | sourced | harness `detour.cpp`; `engine/sim/src/site_map.cpp` (4 directions) |
| VR-7 | WASD does nothing, silently, while paused or while the pop is busy (any action or queued command), because the block is gated on `moving_time` and `idle()` | sourced | `window.cpp:408-409`; `engine/sim/include/zs/actions.h:216` |
| VR-8 | At 144 fps and "1x" (0.33 ticks per frame) 39 of 59 WASD commands were sent before the previous one had been applied (no tick ran in between); each lands in the input log and replay | sourced | harness `wasd2.cpp` (scratch); `engine/sim/src/world.cpp:18-21` |
| VR-9 | The map draws every body at its integer cell; the sub-cell progress the sim keeps (`Action::step_m` toward `path[path_next]`) is not used, so movement is drawn as jumps of a whole cell | sourced | `window.cpp:278,288-292`; `engine/sim/src/actions.cpp:835-845` |
| VR-10 | Our own architecture says the renderer draws from the last two snapshots, interpolated; the window does not | sourced | [engine architecture §3](../design/engine-architecture.md#3-time-fixed-sim-tick-free-frame-rate); VR-9 |
| VR-11 | OpenRCT2 interpolates people and vehicles between ticks for display only: positions before and after each tick are stored, the frame lerps with alpha = accumulator / tick length, and the true positions are restored before the next tick; zoomed-out viewports are skipped | sourced | github.com/OpenRCT2/OpenRCT2 `src/openrct2/entity/EntityTweener.cpp:97-160`, `src/openrct2/Context.cpp:1347-1374` @ 33aacdeb |
| VR-12 | The camera never follows anyone: `origin` is set once on the first frame and moves only by middle-drag and wheel zoom | sourced | `window.cpp:59-60,377-380,687-693` |
| VR-13 | The view's level changes only on start, load and `--shot`; a pop that takes the stairs vanishes from the drawn level | sourced | `window.cpp:94,122,667,289` |
| VR-14 | OpenTTD smooth scroll moves the camera so that 0.75^(dt/30 ms) of the distance remains (frame-rate independent), capped at 16 tiles per 30 ms, keeping the minor axis proportional so it heads straight at the target; smooth scroll is on by default | sourced | github.com/OpenTTD/OpenTTD `src/viewport.cpp:1963-1989,1991-2045`; `src/table/settings/gui_settings.ini:133-135` @ fab8089e |
| VR-15 | That OpenTTD rule halves the remaining distance every 72 ms | derived | VR-14: 30 ms x ln 0.5 / ln 0.75 |
| VR-16 | OpenTTD's follow-vehicle mode is a hard lock (camera = vehicle each frame); keyboard scrolling or edge scrolling cancels the follow; edge scrolling (15 px margin) is off by default | sourced | OpenTTD `src/viewport.cpp:1995-2000`; `src/window.cpp:2781-2815,2871-2890`; `gui_settings.ini:95-98` @ fab8089e |
| VR-17 | OpenTTD zooms to the cursor by scrolling to the tile under the cursor and then zooming | sourced | OpenTTD `src/main_gui.cpp:137-151` @ fab8089e |
| VR-18 | OpenRCT2 "scroll to location" closes 1/8 of the remaining distance per update (rounded up) until it lands; follow-sprite is a hard lock; keyboard scrolling unfollows; edge scrolling only at the outermost pixel and not while Shift or Ctrl is held | sourced | OpenRCT2 `src/openrct2/interface/Viewport.cpp:572-600,611-630`; `src/openrct2-ui/input/MouseInput.cpp:1614-1647`; `src/openrct2-ui/input/InputManager.cpp:240-276` @ 33aacdeb |
| VR-19 | Mindustry: WASD moves the player's unit (normalised, so diagonals are not faster); the camera lerps to the player with factor 0.08 per 1/60 s when "smooth camera" is on (1 when off); any move key cancels free panning and returns the camera to the player | sourced | github.com/Anuken/Mindustry `core/src/mindustry/input/DesktopInput.java:47,262-290` @ 17012d61 |
| VR-20 | Mindustry's 0.08 per 1/60 s gives a half-life of 0.14 s | derived | VR-19: ln 0.5 / ln 0.92 / 60 |
| VR-21 | Godot's Camera2D smoothing is `pos += (target - pos) * speed * delta` with speed 5 by default (a frame-rate dependent form whose source comment admits "haphazard results"), and a drag margin of 20 % of the half-screen on each side inside which the target moves without moving the camera | sourced | github.com/godotengine/godot `scene/2d/camera_2d.cpp:195-208`, `scene/2d/camera_2d.h:68,79` @ 65e8d169 |
| VR-22 | Unity's `SmoothDamp` (from Game Programming Gems 4, 1.10) is a critically damped spring with a velocity state: omega = 2 / smoothTime, a cubic approximation of exp(-omega dt), a max-speed clamp, and an overshoot guard; it is frame-rate independent. Unity's reference source is read-only licensed: cite the formula, do not copy code | sourced | github.com/Unity-Technologies/UnityCsReference `Runtime/Export/Math/Mathf.cs:276-306` @ d5a388db |
| VR-23 | Cataclysm: DDA's sidebar is data-driven widgets: a value shown as a number, a text, or a graph of N cells and symbols ("graph" style) with a colour spectrum across the value range and optional `breaks` (percent thresholds); a variable with a "normal" range shows white while normal | sourced | github.com/CleverRaven/Cataclysm-DDA `doc/WIDGETS.md` (Graph style, colors, breaks) @ 3defba19 |
| VR-24 | CDDA's UI guide: colour is not read by screen readers, so information carried only by colour must also be in text (e.g. "Acorns (fresh)" vs "(old)"), and disabled options should say so | sourced | CDDA `doc/USER_INTERFACE_AND_ACCESSIBILITY.md` ("Text color") @ 3defba19 |
| VR-25 | CDDA coalesces a repeated message of the same type within 3 turns into one line with "x N" | sourced | CDDA `src/messages.cpp:141-170` @ 3defba19 |
| VR-26 | CDDA "safe mode": when a hostile comes within a set distance it stops auto-travel and long activities and asks to stop, ignore or whitelist ("Spotted X 7 tiles to the east -- safe mode is on!"), naming the threat, its distance and direction | sourced | CDDA `src/game.cpp:1499-1530,7380-7402`; `src/options.cpp:1673-1690` @ 3defba19 |
| VR-27 | Measured from the 1440x900 shot: the map takes 651x509 px, 25.6 % of the screen; the cell is 20.4 px; a pop disc is 14.7 px across, an item dot 4 px; the Time window covers the map's top row; the event log panel (1080x220 px) is mostly empty | sourced | `zs --shot` frames (seed 1, ticks 1); `window.cpp:690-692` |
| VR-28 | Map objects are drawn as a dark square with the first letter of their kind: the block's 15 kinds collapse to 8 letters (`s` is sink, saw, stockpile and stove; `p` pantry and power panel; `h` home and hot plate), and the 12 `item` spawn markers draw an `i` square next to the real item's dot; `l` (lamp) and `i` look alike in the default font | sourced | `window.cpp:243-247`; `data/tier0/map/block.toml`; shot `ticks_1` |
| VR-29 | Initials are drawn at a fixed offset (-6, -7) without measuring the text, so two-letter initials ("AM", "MR") overflow the 14.7 px disc | sourced | `window.cpp:295`; shot `crop_house2` |
| VR-30 | Walkers and pops differ only by colour (both are filled discs); an idle walker is dark red (150,50,50) and was found standing on an object square, half hiding it | sourced | `window.cpp:279,293`; shot `crop_house2` |
| VR-31 | Under simulated deuteranomaly (severity 100) the four group colours have a minimum pairwise CAM02-UCS distance of 6.0 (orange-green 6.7, blue-pink 6.0) against 24.1 with normal vision; an active walker against a door is 3.3, a pop's orange against a barricade 3.9 | sourced | `cvd.py` (scratch), colorspacious 1.1.2 Machado 2009 matrices, colours from `window.cpp:176-180,190,261-279` |
| VR-32 | The same measure gives Tol's "bright" (blue, cyan, yellow, purple) 16.2 deutan / 15.6 protan / 28.7 tritan, Tableau Colorblind10's first four 24.7 / 23.6 / 21.4, Tol "high contrast" 28.9 / 29.7 / 24.3; ours is 6.0 / 7.7 / 8.8 | sourced | `cvd2.py` (scratch); tol-colors 2.2.0 `colors.json`; matplotlib 3.11.2 `stylelib/tableau-colorblind10.mplstyle` |
| VR-33 | WCAG contrast: dark initials on the four pop colours 7.4-9.9 (good); on a dead pop's grey 3.6; item dot on concrete 2.45, on wood 2.97; object squares on dirt 1.93 and grass 2.17; an idle walker on asphalt at night 2.43 | sourced | `cvd.py` (scratch), WCAG 2 relative luminance |
| VR-34 | The noise overlay drew nothing at day 0 16:00 (0 pixels differ from no overlay) and has no legend, so "no sound above background" and "overlay broken" look the same; the light overlay at noon washes every outdoor and indoor cell yellow (alpha from log10 lux, capped 180), hiding the light leak it exists to show | sourced | shots `ticks_57600[_overlay_noise]`, `ticks_28800_overlay_light`; `window.cpp:235-240` |
| VR-35 | Need bars use one colour for "high is bad" (hunger 65 %) and "high is good" (morale 57 %); a dead player's 23 bites are listed one line each; 23 near-identical "X bit You (region)" lines fill the log; "Andres Castillo arrived, dead" reads as a corpse rather than a walker arriving | sourced | `window.cpp:485-489,529-533,585-592`; shots `night`, `day2` |
| VR-36 | Nothing on the map marks the hovered cell, the cell a context menu refers to, or a pop's current goal or path; the only feedback is a text tooltip | sourced | `window.cpp:381-397`; shot `ticks_1_menu_10_12` |
| VR-37 | Project Zomboid shows the body as a column of moodle icons with up to four levels and a sentence on hover, no numbers | reported | [PZ-2](project-zomboid.md) |
| VR-38 | RimWorld: a colonist bar of portraits across the top with state icons (mental state, fleeing, sleeping, medical rest), a column of alerts on the right and a stack of letters for events; a popular mod adds mood-risk outlines (yellow minor, red major, pulsing extreme), so vanilla is reported not to show break risk on the bar | reported | rimworldwiki.com/wiki/Category:Images_-_Colonist_Bar; rimworldwiki.com/wiki/Mood; Owl's Colonist Bar mod pages (search summaries) |
| VR-39 | Prison Architect: a Utilities view shows wiring and pipes over the plan; its overall danger is a "thermometer" in the top bar, not a map overlay | reported | prison-architect.fandom.com/wiki/Utilities (search summary) |
| VR-40 | Root of "WASD movement is poor", in order: (1) direct control runs at 24x, so a step lasts 1/30 of a second (VR-1..3); (2) presses into walls walk long detours (VR-5); (3) moves are drawn as cell jumps and the camera does not follow (VR-9, VR-12); (4) WASD silently fails while paused or busy (VR-7) | estimate | VR-1..VR-13 |
| VR-41 | Root of "hard to tell what is going on": no distinct shapes (pops, walkers, objects are discs and lettered squares), ambiguous letters, no state on the map (no moodles, no alert column, no goal lines), overlays without legends, a log that repeats, and a quarter of the screen used for the map | estimate | VR-27..VR-36 |

## WASD: what happens on a key press

The WASD block (`window.cpp:401-413`) runs once per frame after `advance()`:

1. If the controlled pop is idle and time is running, send `MoveTo(neighbour)`.
2. The next frames run ticks; the pop path-finds to that cell and walks 1 m at 1.35 m/s, about 1.5 ticks.
3. When it arrives it is idle again and the next frame sends the next step.

At the window's "1x" (24 game s per real s) each step lasts about 30 ms of real time, so the pop shoots across the block. At higher speeds the frame rate, not the body, sets the pace, and hours pass during a short walk (VR-3). If the next cell is floor behind a wall, `MoveTo` walks around the house (VR-5). Diagonals are routed by a 4-neighbour path finder (VR-6). While paused or busy nothing happens and nothing says why (VR-7).

What the open games do:

| Game | Unit movement | Camera |
| --- | --- | --- |
| Mindustry | WASD drives the unit directly, normalised | Lerps to the unit (half-life 0.14 s); moving cancels free pan (VR-19, VR-20) |
| OpenTTD | (no avatar) | Smooth scroll to a target, half-life 72 ms, capped speed; follow is a hard lock; arrow keys and edge scroll cancel follow (VR-14..17) |
| OpenRCT2 | Entities tweened between ticks for display (VR-11) | Scroll-to closes 1/8 per update; follow is a hard lock; keyboard scroll unfollows (VR-18) |
| Godot Camera2D | | Drag margin 20 %, smoothing speed 5 (VR-21) |
| Unity SmoothDamp | | Critically damped spring with velocity, max speed (VR-22); [SO-2](second-order-dynamics.md) gives the general form |

The pattern in every one: the simulation position is exact; the drawn position is interpolated; the camera follows the controlled body with a short, frame-rate independent smoothing, and any manual pan breaks the follow until a recentre key or a move key.

## Legibility: what the shots show

Eight frames from `zs --shot` (seed 1; ticks 1, 28,800, 57,600, 108,000 and 230,000; overlays none, noise, light; a context menu):

- **Morning, ticks 1.** Three buildings on grass, the player as an orange disc with "Y" and a white ring, two survivors as discs with initials, a dark red disc inside the south house that is an idle walker standing on an object. Fourteen dark squares with single letters; five of them are `i` next to a yellow dot (the spawn marker beside the item it spawned). Without hovering, nothing says what `s`, `p` or `h` is (VR-28). The right third of the screen and most of the log are empty (VR-27).
- **Night, 23:00.** Floors are dimmed by light; four dark red walkers stand around the main building and the yard. They read well against the dark (CAM02-UCS distance 25-41 normal vision) but are the same shape as the survivors (VR-30). The log shows three red lines "X arrived, dead" (VR-35).
- **Day 1, 15:56.** The player was bitten 23 times and rose; the inspector lists 23 bites one per line and the log 23 "bit You" lines. The ring is now on a red disc. The time bar says "The run is over: nobody left" in small orange text.
- **Overlays.** Noise at 16:00: nothing drawn, no legend (VR-34). Light at noon: every cell yellow.
- **Context menu.** "Walk here" floats at the cell's right edge; the cell itself is not outlined (VR-36).

Colour: the group's four colours are fine for normal vision and close to indistinguishable in pairs for a red-green deficient player (VR-31); swapping to a measured palette quadruples the margin (VR-32). The art direction already says "never colour alone" for pain, infection and fracture ([art direction](../design/art-direction.md)); the map breaks it for pops vs walkers and idle vs active walkers.

## State, selection and alerts in other games

- **Body state as a column** (Zomboid moodles, VR-37; CDDA graph widgets with thresholds, VR-23): few states, each with levels, colour plus symbol plus a sentence. Our inspector has bars for every need but no thresholds, and the same colour for good and bad (VR-35).
- **The group as a bar** (RimWorld, VR-38): one portrait per person with a state icon (sleeping, fleeing, hurt), visible all the time. Our Group panel is a text list with "in the group / not found yet".
- **Alerts as a column, events as a log** (RimWorld alerts and letters, VR-38; CDDA safe mode, VR-26): the alert names the threat, distance and direction and stops long actions. Our "Rest until something happens" stops on alert and death lines (`window.cpp:143`), but at normal speed nothing slows or pauses; the `Pacer` already has `alert()` (spec 40 rule 7).
- **Log hygiene** (CDDA, VR-25): repeat lines collapse to "x N".
- **Overlays with a legend** (Prison Architect utilities view, VR-39; OpenTTD smallmap legends): an overlay always says what its colour means and its scale.

## What this means for us (Proposal)

Fixes in `apps/zs/window.cpp` (line numbers at commit `8295162`), in priority order. None are applied.

| # | Where | Problem | Proposed fix |
| --- | --- | --- | --- |
| 1 | `:35-36`, `:149-168`, `:447` | Direct control runs at 24x (VR-1..3); "1x" is mislabelled | Drive time with the `Pacer` (spec 40): the ladder from `Pace::Real`, labels from `pace_name`, and a named cap `pacer.cap("walking", Pace::Real or Pace::X3)` while a WASD key is held or the controlled pop is approaching, released on key-up (the cap's dwell gives hysteresis). Show "capped by walking" in the time bar (`Pacer::limiting`). Default taken by Claude to record: X3 while walking, Real in a fight |
| 2 | `:409-412` | Presses into walls walk detours (VR-4, VR-5); diagonals take odd routes (VR-6) | Before submitting, require `t.map->can_walk(SiteMap::edge_between(p, to))`; for a diagonal, submit only if both orthogonal edges are passable, else try the single axis that is; if neither, flash the blocking edge |
| 3 | `:278`, `:288-293` | Bodies jump a whole cell (VR-9, VR-10) | Draw at `pos + (path[path_next] - pos) * step_m / length` while `phase == Approach` (read-only), as OpenRCT2's tweener does (VR-11); later from two snapshots per §3 |
| 4 | `:59-60`, `:687-693`, `:377-380` | Camera never follows (VR-12) | Keep a camera target in world metres; each frame move toward the followed pop's drawn position with a frame-rate independent smoothing, `k = 1 - exp(-dt / tau)` with tau about 0.1 s (between OpenTTD's 72 ms and Mindustry's 0.14 s half-lives, VR-15, VR-20), or SmoothDamp (VR-22); a dead zone of about 20 % (VR-21); middle-drag breaks the follow; F or Home recentres; any WASD press re-follows (VR-19). Clamp to the map |
| 5 | `:94`, `:122`, `:667`, `:289` | The level does not follow the pop (VR-13) | Each frame, while following, set `a.level` to the followed pop's `z`; PgUp/PgDn break the follow like a pan |
| 6 | `:408-409` | WASD silently fails while paused or busy (VR-7) | While paused, show "Paused: Space to run" near the pop; while busy, a WASD press cancels the current action (submit `Cancel`, then the move), as in direct-control games, or shows "busy: C cancels" |
| 7 | `:409` | Commands sent twice before a tick applies them (VR-8) | Remember the tick a move was sent at and send the next only after `clock.tick` has advanced past it |
| 8 | `:243-247` | Lettered squares are ambiguous (VR-28) | A per-kind label table (two or three letters or a glyph: `Frg`, `Gen`, `Snk`, `Stv`, `Lmp`...) with a legend in a "Map key" panel; do not draw `item` spawn markers (the item dot is the item); draw object squares with a light outline (contrast VR-33) |
| 9 | `:274-281`, `:293-297` | Pops and walkers are the same shape; idle vs active only by brightness (VR-30, VR-31) | Walkers as a diamond or X, pops as circles; active walkers get a short line to their target or facing; the controlled pop a thick ring plus a small arrow; the selected entity a dashed or second ring |
| 10 | `:190` | Group colours confuse under red-green deficiency (VR-31, VR-32) | Replace with Tol "bright" blue/cyan/yellow/purple or Tableau Colorblind10; keep initials as the redundant cue |
| 11 | `:295` | Initials overflow the disc (VR-29) | Centre with `ImGui::CalcTextSize`; scale disc radius to at least 0.42 cell or show one letter below 24 px per cell |
| 12 | `:485-489`, `:519-525` | Bars do not say good or bad (VR-35) | Colour each bar by its own thresholds (CDDA `breaks`, VR-23) with a word ("hungry", "very tired"), and invert morale; add a moodle-style column of the 3-5 worst states on the map next to the controlled pop and in the Group panel ([PZ-13](project-zomboid.md)) |
| 13 | `:577-596`, `:529-533` | Log repeats; wound list unbounded (VR-35) | Coalesce identical lines within a game minute into "x N" (VR-25); group wounds by region with a count; reword "arrived, dead" to "a walker (Andres Castillo) arrived" |
| 14 | `:235-240` | Overlays without legend; light saturates by day (VR-34) | A legend strip with the scale and "background N dB"; "nothing above background" text when empty; for light, show only indoor cells or cells whose lux exceeds the outdoor ambient, so the leak shows |
| 15 | `:381-397` | No hover or menu target on the map (VR-36) | Outline the hovered cell; keep the menu's cell outlined while the menu is open; draw the controlled pop's path (`action.path`) as a dotted line |
| 16 | `:688-692` | Map uses a quarter of the screen, top row hidden (VR-27) | Fit the map to the space left of the side panels and below the Time window's real height (`ImGui::GetWindowHeight` from the previous frame), or let it fill the screen behind semi-transparent panels; with follow (fix 4), start at a zoom of about 32 px per cell |
| 17 | `:140-146` | Alerts do not slow time at normal speed | Route "alert" event lines into `Pacer::alert` (pause until acknowledged) and an alert column on the map edge naming threat, distance and direction (VR-26, VR-38) |

Fixes 1-4 address "WASD is poor"; 8-12 and 16 address "hard to tell". All are view-side: the window keeps no game state and only sends intents (spec 16 rule 1). Fix 2 changes what WASD sends but not the sim. A sim-side "step in a direction" intent would be cleaner than `MoveTo` to a neighbour; that is a change to spec 14, left as an open question.

Carried to design docs:

- [spec 16](../tier0/specs/16-view-2d.md): the A1 hash check is untouched by these fixes, but fix 1 moves the window to the `Pacer`, which spec 40 already lists as pending.
- [cameras and modes](../design/cameras-modes.md): tactical mode is "centered on the character"; VR-14..22 give the follow parameters to try.
- [art direction](../design/art-direction.md): "never colour alone" should extend from the body view to the map (shape for pops vs walkers, VR-30).

## Open questions

- design/cameras-modes.md: Which speed caps direct control: 1:1 (realistic, 1.35 cells per real second) or 3x (4 cells per second)? Does the cap apply to WASD only or to any approach of the controlled pop?
- design/interaction.md: Should WASD send a "step in a direction" intent (refused at a wall, no path-finding) instead of `MoveTo` to the neighbour cell, and does a WASD press cancel the current action?
- design/cameras-modes.md: Does the follow camera hard-lock (OpenTTD, OpenRCT2) or lag with a dead zone (Godot, Mindustry), and which key recentres?
- design/art-direction.md: Which colour-vision-safe palette for group members and map states, and which shapes separate pops, walkers, objects and items at 20-32 px per cell?
- tier0/specs/16-view-2d.md: Is a moodle-style state column (which states, which thresholds from the body) part of the Tier 0 window, and where (by the pop on the map, in the Group panel, or both)?
- tier0/specs/40-time.md: Which events raise a `Pacer` alert that pauses at normal speed (a walker in sight, a bite, a group member down)?

## Sources

Own code and measurements (repo at `8295162`):

- `apps/zs/window.cpp`, `engine/sim/src/actions.cpp`, `engine/sim/src/body.cpp`, `engine/sim/src/world.cpp`, `engine/sim/include/zs/pacer.h`, `data/tier0/map/block.toml`.
- Offscreen build: `cmake -S . -B <scratch>/build-vr -G Ninja -DCMAKE_BUILD_TYPE=Release -DZS_BUILD_WINDOW=ON -DSDL_X11=OFF -DSDL_WAYLAND=OFF -DSDL_KMSDRM=OFF -DSDL_UNIX_CONSOLE_BUILD=ON` (Linux container, GCC 13, 4 cores); without the last flags SDL's configure fails for lack of X11/Wayland dev packages.
- Shots: `zs --seed 1 --shot f.bmp --ticks N [--overlay noise|light] [--menu 10,12]` for N = 1, 28,800, 57,600, 108,000, 230,000 (1440x900).
- Scratch harnesses (not in the repo): `wasd.cpp`/`wasd2.cpp` replay `advance()` and the WASD block at 60 and 144 fps for 3 real seconds per speed; `detour.cpp` checks every level-0 cell's WASD neighbours with `SiteMap::can_walk` and `find_path`; `cvd.py`/`cvd2.py` measure CAM02-UCS distances under colorspacious 1.1.2's Machado et al. (2009) simulation at severity 100 and WCAG 2 contrast.

Code read (all `sourced` rows):

- OpenTTD, github.com/OpenTTD/OpenTTD @ fab8089e6bed2be570b6b0a2cb9f19a5db16a7da: `src/viewport.cpp`, `src/window.cpp`, `src/main_gui.cpp`, `src/table/settings/gui_settings.ini`.
- OpenRCT2, github.com/OpenRCT2/OpenRCT2 @ 33aacdeb0d9e22c1622acf071e6698f946365453: `src/openrct2/entity/EntityTweener.cpp`, `src/openrct2/Context.cpp`, `src/openrct2/interface/Viewport.cpp`, `src/openrct2-ui/input/MouseInput.cpp`, `src/openrct2-ui/input/InputManager.cpp`.
- Cataclysm: DDA, github.com/CleverRaven/Cataclysm-DDA @ 3defba1905e8b3c77fd204afc7973d5cde33a85f: `doc/WIDGETS.md`, `doc/USER_INTERFACE_AND_ACCESSIBILITY.md`, `src/messages.cpp`, `src/game.cpp`, `src/options.cpp`.
- Mindustry, github.com/Anuken/Mindustry @ 17012d61d9e4c3cbd9820d87e81c24495486ac53: `core/src/mindustry/input/DesktopInput.java`.
- Godot, github.com/godotengine/godot @ 65e8d16951d6963cb3984c090e45f40d1ba5f704: `scene/2d/camera_2d.cpp`, `scene/2d/camera_2d.h`.
- Unity C# reference (reference-only licence), github.com/Unity-Technologies/UnityCsReference @ d5a388dbe5253100d5778cccb467fdb42af6a643: `Runtime/Export/Math/Mathf.cs`.
- Palettes: tol-colors 2.2.0 (PyPI) `colors.json`; matplotlib 3.11.2 `mpl-data/stylelib/tableau-colorblind10.mplstyle`.

Reported (web-search summaries):

- RimWorld colonist bar icons: https://rimworldwiki.com/wiki/Category:Images_-_Colonist_Bar ; mood thresholds: https://rimworldwiki.com/wiki/Mood ; mod outlines: https://catalogue.smods.ru/archives/160179 ; alerts and inbox: https://mail.rimworldwiki.com/wiki/Console_version/1.11
- Prison Architect utilities: https://prison-architect.fandom.com/wiki/Utilities
- Project Zomboid moodles: [project-zomboid](project-zomboid.md) (PZ-2).
