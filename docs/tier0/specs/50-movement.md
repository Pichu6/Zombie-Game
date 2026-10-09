# 50 · Movement

**Status:** Implemented and tested on MSVC (2026-10-08), written before its code; not yet played by the user. **Stage:** Tier 0, after stage 9. **Needs:** [09](09-site-map.md), [14](14-action-executor.md), [23](23-walker-brain.md). **Changes:** [09](09-site-map.md) rule 5 (paths), [14](14-action-executor.md) (a `Walk` intent; doors on the way), [16](16-view-2d.md) rule 5 (WASD sends `Walk`).

## Purpose

The user played the reworked window on 2026-10-08 and found four faults, all in the simulation, not the view:

1. Mouse orders walk "like the snake game": paths use only four directions, so every route is a staircase of straight lines.
2. WASD moves in jumps and lurches: each step is a separate `MoveTo` to the next cell, so the pop stops at every cell and waits for the next order, and a diagonal alternates between its two axes.
3. Crossing a closed door feels like a tug: the pop stands still for a fixed 2 s while it opens the door.
4. A pop walked through a door with a stove in front of it: map objects take no room.

This spec fixes them in the simulation, so the player, the pop AI, the walkers and a replay all move by the same rules.

## State

| Field | Where | Notes |
| --- | --- | --- |
| `blocking` | map file, top level | the object kinds that fill their cell (furniture) |
| `SiteMap::passable(cell)` | derived | walkable floor without a blocking object |
| `Actor::heading` | saved | the direction of a `Walk` intent: one of 8, or none |

## Rules

1. **Eight directions.** Paths (pops through `SiteMap::find_path`, walkers through their own planner) may step diagonally. A diagonal step is allowed only across an open corner: both orthogonal neighbours are passable and the four edges around the corner have no wall, door, window, fence or gate. Doors and windows are therefore always crossed straight. Costs are octile: 10 straight, 14 diagonal, plus the existing door cost.
2. **Diagonal length.** A diagonal step is √2 m of walking (cells are 1 m); straight steps stay 1 m and stairs 4 m.
3. **Blocking objects.** Objects whose kind is listed under `blocking` fill their cell: no path, step or walker enters it. The cell stays floor for light, noise and items. A body standing on one (an old save) may step off. Default list: fridge, stove, hot plate, pantry, sink, stockpile, generator (estimate: the furniture a person cannot walk through; lamps, radios, panels, tools and markers do not block).
4. **Working at a blocking object.** Its reach cells (spec 14) are its passable 4-neighbours with a crossable edge to it, instead of its own cell; items lying on it (a stockpile's tools) are reached the same way.
5. **`Walk` intent** (command 4, `a` = direction 0-7 clockwise from north, or -1 to stop). It takes over: the queue is cleared and any other action is interrupted. While a heading is set, the actor walks cell to cell without stopping: when a step completes, the next cell is the one in the heading if rule 6 allows it, and the walking progress beyond the cell carries over. `-1` lets the current step finish and then the actor is idle.
6. **Steering past walls.** From a cell, a heading leads to: its own neighbour if that step is allowed (rules 1 and 3, and the edge can be walked); for a diagonal heading that is not allowed, the passable one of its two axes (sliding along a wall); otherwise nowhere, and the actor stops with its heading kept, so it moves again as soon as the way is open. `SiteMap::steer(from, dx, dy)` is the one function for this; the window uses it only to flash the wall that stopped a step.
7. **Doors on the way.** Opening a closed, unlocked door while walking takes 0.5 s (estimate: a push of the handle), down from 2 s; an open door is crossed without stopping.
8. **One rule for all.** The pop AI, a replay and the player produce the same movement from the same intents; `Walk` is an ordinary command in the input log.

## Invariants

- No body ends a step on a blocking cell or crosses a corner that has an edge.
- A heading never changes the state of a body that is not walking (`Walk` on a dead or unconscious body is refused like `MoveTo`).
- Every walkable, non-blocking cell of the block that was reachable before stays reachable from the pop start (the map is adjusted where a blocking object closed the only way).

## Tests (`test_movement.cpp`)

- A path across an open room is diagonal and shorter in steps than the 4-direction path; no diagonal crosses a corner with an edge or passes a door. (A1)
- A blocking object is never entered by `find_path`, `steer` or a walker; working at the fridge happens from a neighbour. (A1)
- `Walk` east for 3 game seconds moves a pop about 4 cells without an idle tick between cells; `Walk` north-east against a wall slides along it; `-1` stops at the next cell. (A1)
- Opening a closed door on the way costs 0.5 s: one or two ticks more than an open door. (A1)
- Every cell reachable before is still reachable from the pop start. (A1)
- The goldens of the stage tests are re-pinned on MSVC once, in the same commit, with the reason. (A1)

## As built

- `SiteMap`: `blocking` read from the map file, `blocked`, `passable`, `step_allowed`, `steer`, `step_length`; `find_path` relaxes the four straight steps, then the four diagonals, in a fixed order.
- `Actors`: `CommandType::Walk`, `Actor::heading`, `Action::steering`, `start_steer` (drops the heading of a body that cannot walk); `step_move` checks `step_allowed`, walks √2 m on a diagonal, opens doors in 0.5 s and extends a steering path one cell at a time with the progress carried; an interruption clears the heading; any other order replaces it. Furniture is worked at from a free 4-neighbour.
- `Walkers::plan_path`: the same diagonals and `passable`; a goal on furniture ends beside it.
- Save format version 4 holds the heading and steering (`ArchiveReader::format`); version 3 saves load with no heading. The compressed format of [06](06-save-replay.md) rule 16 becomes version 5.
- Map: the kitchen's fridge, pantry, stove, sink and hot plate stand along its top wall; the sink no longer stands in front of the door at (5, 10).
- Window: WASD sends `Walk` when the keys change and -1 on release; it flashes the edge only when `SiteMap::steer` finds nothing.
- Goldens of stages 4, 6, 7, 8 and 9 re-pinned on MSVC. A 20-seed batch of the 14-day run: 9 to 14 runs reached day 14, deaths from bleeding out 11 to 2 (old movement against new, same seeds); not studied further.

## Not in this spec

Any-angle movement (positions between cells, string-pulled paths): the next step if the 8-direction paths still look stiff. Pushing or dragging furniture (a sofa against a door as a barricade). Walker crowds and body collisions.

## Open

1. Should walkers keep 4 directions (shambling) and pops get 8? Default: both get 8, one rule for all bodies ([0055](../../decisions/0055-defaults-for-fixes-from-engineering-research.md) spirit); revisit when walkers are tuned.
