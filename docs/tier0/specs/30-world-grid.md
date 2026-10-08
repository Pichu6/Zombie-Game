# 30 · World grid

**Status:** Implemented and tested (headless, GCC and Clang, same golden hash; see As built). Not yet Validated on Windows/MSVC. **Stage:** not Tier 0; first unit of the world generator ([world systems](../../design/world-systems.md) §4 step 1, §8 item 1). **Needs:** [02](02-draw.md), [01](01-quantity.md) (determinism rule). Decision: [0038](../../decisions/0038-grid-sector-zone-sizes-accepted.md) (**Decided**: icosahedral hexagon-and-pentagon grid, about 5,000 km² per cell at Earth size).

## Purpose

One shared set of world cells that every later generator step reads and writes ([spec 31](31-terrain-climate.md) fields, then geology, settlements, routes, sectors). It gives each cell a place on the sphere, an area, and its neighbours, so distances, borders and flows stay honest at any planet size. It is a library with no window and no dependence on the renderer.

## State

| Field | Type | Unit | Notes |
| --- | --- | --- | --- |
| `frequency` n | `int` | | Subdivision of the icosahedron. Cells = 10 n² + 2 (12 pentagons, the rest hexagons) |
| `radius_km` | `double` | km | Planet radius; Earth 6371. Setup parameter, never changed after creation |
| `centre[i]` | unit vector (x, y, z) | | Cell centre on the unit sphere; `z` is the sine of latitude, the polar axis is through two icosahedron vertices |
| `area_km2[i]` | `double` | km² | Cell area; the sum is the sphere's area |
| `neighbours[i]` | list of `uint32` | | 5 or 6 cell ids, in a fixed order (by id) |

Cell ids are `0 .. 10 n² + 1`, assigned by a fixed construction order, so the same `n` always gives the same ids on every platform. The grid is **derived, not saved**: it is rebuilt from `(n, radius_km)`, which are saved with the world. Fields on top of it are saved.

## Rules

1. **Construction.** The 12 vertices of a regular icosahedron (golden-ratio coordinates, normalised, rotated so one vertex is at the north pole). Each of its 20 faces is cut into n² small triangles by barycentric interpolation, with each weight warped to `sin(w θ) / sin θ` (θ = the angle between icosahedron vertices) so cells near a face's centre are not oversized; each lattice point is projected onto the sphere. Without the warp the largest cell is about 2.2 times the smallest; with it, about 1.42. Each unique point is one cell (the dual of the triangle mesh is the hex-and-pentagon grid). Points on shared edges and corners are deduplicated by their integer lattice key, never by comparing floats.
2. **Only `+ - * /`, `sqrt` and `detmath::sin`** ([01](01-quantity.md) rule 5): never `<cmath>` trigonometry, so the grid is bit-identical across compilers.
3. **Neighbours** are the cells joined by a triangle edge. Symmetric by construction.
4. **Area** of a cell is one third of the flat area of its incident triangles (the barycentric dual), then every area is scaled by one common factor so the sum equals `4 π radius²`. This is near-equal-area, not exact: areas differ by a bounded ratio (see Tests). *Stated plainly:* decision 0038 said "equal-area"; an icosahedral geodesic grid is equal-area only approximately, and the spread is reported, not hidden.
5. **Distance** between cells is the great-circle distance `radius × angle`, with the angle from a deterministic arctangent (argument halving, then a Taylor series) on `|a × b|` and `a · b` ([01](01-quantity.md)); and **graph distance** is the number of neighbour steps. Cost-distance for routes is a later spec and uses these.
6. **Lookup:** `nearest_cell(point)` is a greedy walk over neighbours from any start, stopping when no neighbour is closer; tested against brute force; `cell_at(lat, lon)` uses it.
7. **Size presets:** `frequency_for_cells(target)` picks the n whose cell count is nearest. Earth size and 5,000 km² per cell is n = 101 (102,012 cells). Tests and smaller worlds use n from 4 up.

## Invariants

- Nested: every cell centre at frequency n is a cell centre at 2n (tested; the zoom ladder, [zoom continuum](../../design/zoom-continuum.md)).
- Cell count = 10 n² + 2; exactly 12 cells have 5 neighbours, all others 6.
- Neighbour relation is symmetric and irreflexive; the graph is connected.
- Euler's formula holds for the triangle mesh (V − E + F = 2, with E = 30 n², F = 20 n²).
- All centres are unit vectors to within 1e-12.
- Sum of areas = `4 π radius²` to 1e-9 relative.

## Tests (agreement levels)

- Counts, 12 pentagons, symmetry, connectivity, Euler, for n in {1, 2, 3, 4, 8, 16}. (A1)
- Area spread: max ÷ min cell area under 1.5 (measured 1.39–1.42 for n from 8 to 101) and the mean within 1e-12 of `sphere area ÷ cells`, for n in {8, 16, 32}. (A3, calibration band)
- Area sum equals the sphere's area. (A1 within tolerance)
- `nearest_cell` of each cell's own centre is that cell; of random points agrees with brute force over all cells. (A1)
- Golden: hash of the neighbour table and the centres rounded to 1e-9, for n = 8, pinned on Linux; Windows must reproduce it. (A1 across compilers)
- Metamorphic: n = 2m has about 4 times the cells of n = m and about a quarter the mean area. (A3)

## Calibration

| Value | Source |
| --- | --- |
| Earth radius 6371 km, area 510.1 million km² | standard value |
| About 5,000 km² per cell, n = 101 | decision 0038 (design) |

## Not in this spec

Rotation of the planet, tides, real Earth import (a later data pipeline, 0038 item 3), local flat metric grids, sectors and zones (they sit on top of the cells; spec to come), a renderer mesh.

## As built

- `engine/gen`: `WorldGrid` (`zs/gen/world_grid.h`, `src/world_grid.cpp`): construction, `centre`, `area_km2`, `neighbours`, `distance_km`, `nearest_cell`, `cell_at`, `graph_distance` (multi-source breadth first), `frequency_for_cells`. Tests in `engine/gen/tests/test_world_grid.cpp`.
- Earth size (n = 101) builds 102,012 cells in about 0.15 s; areas run 3,866 to 5,492 km² around the 5,000 km² mean.
- Cell 0 is the north pole and cell 11 the south pole (the 12 pentagons are cells 0 to 11 and, for n ≥ 2, only those).
- *Found by building it:* decision 0038 says "equal-area"; the grid is near-equal-area with a 1.42 spread, which is the spec's stated reading. Exact equal-area would need an optimised or Lloyd-relaxed grid; not needed for the fields and sectors built on it.
