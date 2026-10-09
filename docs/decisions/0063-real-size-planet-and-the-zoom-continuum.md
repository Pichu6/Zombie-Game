# 0063 · A real-size planet stored at Earth-size cells, and one zoom from planet to street

- **Date:** 2026-10-09
- **Status:** Decided by the user (2026-10-09): "yes, do whats good for good foundations. so, like in google earth, or map we should be able to go from planet to street view with ease. but with gameplay on top." This confirms the [zoom continuum](../design/zoom-continuum.md) proposal of 2026-10-07 in its main lines and moves the one world onto the grid 0038 chose.
- **Builds on:** [0038](0038-grid-sector-zone-sizes-accepted.md) (about 5,000 km² per cell at Earth size), [0052](0052-one-executable-one-simulation.md) (one world), [0062](0062-the-game-loop-attention-possession-and-late-starts.md) (attention sets detail), [spec 30](../tier0/specs/30-world-grid.md) (the nested grid), [spec 31](../tier0/specs/31-terrain-climate.md) and [spec 52](../tier0/specs/52-land-water-capacity.md) (generators that no longer depend on the grid).

## Decision

1. **Real size.** The planet is Earth's size by default (radius 6,371 km, 510 million km²) and is never shrunk to fit the screen; the player gets around by zooming and by travel with time passing.
2. **One stored grid.** The one world is generated and stored on the frequency-101 grid: 102,012 cells of about 5,000 km², about 70 km apart (`kOneWorldFrequency`). Everything finer is computed from it when approached and saved once visited (zoom continuum tricks T2-T4): children add up to their parent, so zooming in never changes what was seen from above.
3. **One camera, layers on top.** One continuous camera from the globe to a street and a room, as in a globe viewer; the gameplay layers sit on that zoom by camera height (Macro at globe and region heights, City at city and district heights, Individual at street height), with enter and leave heights apart so they do not flicker, and only the hard jumps the zoom note lists (taking a body, materialising a new place, opening an office).
4. **Generators in km, not cells.** Every generator rule is stated in km or km², never in grid steps, so the same seed gives the same world at any grid frequency; fast tests may run on a coarse planet (`kTestFrequency` = 24) for rules that do not depend on the grid, and the checks against Earth run on the one world's grid.

## Consequences

- World creation takes about 6.4 s for the whole pipeline at frequency 101 (MSVC release, seed 1: plates to countries, people and towns, plus a 2,048 × 1,024 map); the window starts after it.
- Goldens of the land, deposits and people are pinned on the frequency-101 planet; the deposit medians and the country history are calibrated there (spec 52).
- The window's views (spec 16) and the renderer must reach every height of the zoom; the stored fields must be evaluable below a cell (zoom continuum section 4), which is the next generator work after the country card.
- Cameras-modes' preference for explicit modes (zoom continuum section 5, point 1) is superseded for the layer views: the camera is continuous, the modes are what each height allows.
