# 51 · Dwellings: the ledger, the house plan and its interior

**Status:** Implemented and tested on MSVC (2026-10-08), written before its code. Not yet used by the run: the block is still hand-made. **Stage:** after Tier 0; the first slice of the building and interior generators of [content tools](../../design/content-tools.md). **Needs:** [47](47-country-towns.md) (the town's households), [09](09-site-map.md) (the map format), [50](50-movement.md) (blocking furniture). **Design:** [world simulation](../../design/world-simulation.md) §8 and §12, [zoom continuum](../../design/zoom-continuum.md) (T3), [0017](../../decisions/0017-slow-endgame-and-generation-on-the-go.md).

## Purpose

A generated town is records today: households, people and businesses, with an address that is a random string (two households can share one). The world under the camera must be the same world the records describe: a household of five on a low income lives in a small, crowded house with a little food; a high-income couple has room to spare. This spec makes the first link down: every household gets a **dwelling record** at town creation, and a **house plan with its interior** is generated from that record, in the site-map format the simulation loads.

The rule of [world simulation §12](../../design/world-simulation.md): the record is the truth, made at world creation; the placement is a pure function of the record and a seed, generated when needed, and never adds or removes what the record holds.

## State

| Field | Notes |
| --- | --- |
| `Dwelling` | household index, street, number, address; floor area (m²); footprint width (frontage) and depth in 1 m cells; lot width and depth; rooms (type, rectangle, occupants); food in the pantry and fridge (kJ) |
| `data/gen/dwellings.toml` | floor area per person by income group, minimum area, pantry days by income group, room minimums, lot margins |
| `data/gen/interiors.toml` | furniture kit per room type, in placement order; which kinds block |

## Rules

1. **One dwelling per household**, made after the town (`plan_dwellings`), with its own keyed draws, so the town itself and its golden do not change. Addresses become real: households are numbered along the town's streets, odd and even sides, unique per street; `assign_addresses` writes them back into the households.
2. **Floor area** = members × area per person of the town's income group × (1 ± 15 % keyed spread), at least the minimum (estimates: low 12, lower-middle 20, upper-middle 30, high 40 m² per person, minimum 24 m²; high income anchored on [WEC-18](../../research/world-systems-economy.md), 35-49 m² in Finland, Lithuania and urban China).
3. **Rooms** from the household: a kitchen, a living room, a bathroom (not in low-income dwellings, estimate) and bedrooms: one for the head and partner, one per two other members, at most four. Below 36 m² the kitchen and living room are one main room. Every room is at least 2 × 2 cells.
4. **Plan (one storey in this slice).** The footprint is width × depth cells, width from the area (aspect about 1.3, 5 to 14 cells). A 1-cell corridor runs from the front door to the back door; rooms stack front to back in the two bands beside it, the living room and kitchen at the front. **Every room has a door onto the corridor**, so every room is reachable by construction. Windows: one on each exterior side of each room. The front door faces the street.
5. **Lot.** The house stands on a lot with a 3 m front yard (a dirt path from the street to the door), 2 m side yards and a 5 m back yard, fenced on the sides and back. The lot's first row is the street (asphalt).
6. **Interior from the room type** (`data/gen/interiors.toml`): furniture along the walls, never on a cell in front of a door, placed in kit order; a piece that would cut any free cell of the room off from its door is skipped (the reachability check of [50](50-movement.md)). One bed per occupant of a bedroom (beds do not block). Kitchens hold a fridge (not in low-income dwellings, estimate), a stove, a sink and a pantry.
7. **Contents follow the household:** the pantry holds members × 10,000 kJ × the income group's pantry days (estimates: low 3, lower-middle 5, upper-middle 7, high 10); the fridge two more days. A `home` object carries the household's census home id.
8. **Output** is a site-map TOML ([09](09-site-map.md)) with the `blocking` list, one level, the lot and the house; it loads with `SiteMap::load_file`. `zs gen houses` prints the ledger of a town; `zs gen house --household N --out f.toml` writes one house and prints its plan.
9. **Pure functions:** the same town, seed and household give the same bytes.

## Invariants

- Every household has exactly one dwelling; no two dwellings share an address.
- Every walkable, non-blocked cell of a generated house is reachable from the street.
- The beds in a dwelling equal its members; the food in it equals rule 7's amount.

## Tests (`engine/gen/tests/test_dwellings.cpp`)

- One dwelling per household, unique addresses written back into the households. (A1)
- Floor area within rule 2's band; per person, low-income dwellings are smaller than high-income ones over a town of each. (A1)
- Each generated house loads as a `SiteMap`; every free cell is reachable from the street; every room has a door; beds equal members; the pantry holds rule 7's food. (A1)
- The same seed gives the same TOML; golden hash of one house pinned. (A1)

## As built

- `engine/gen/include/zs/gen/dwellings.h`, `src/dwellings.cpp`: `DwellingRules` (`load_dwelling_rules` reads `data/gen/dwellings.toml` and `data/gen/interiors.toml`), `income_group`, `Dwelling` and `Room`, `plan_dwellings`, `assign_addresses`, `house_toml`.
- The plan: width from the area (aspect 1.3, 5-14 cells); rooms go to a band (living or main left, kitchen and bathroom right, bedrooms to the lighter band); each band is as wide as its rooms' share of the area, at least 2 cells; a room is as deep as its share, at least 2 cells and enough cells for its door, furniture and a way between (main 10, kitchen 7, bedroom occupants + 2, others 4); the house is as deep as the deeper band and the last room of each band takes what is left. The kitchen and bathroom floors are tiled.
- The interior: beds first in bedrooms (on any free cell), then the kit in order; a blocking piece goes on a wall cell, starting at a keyed position, and is kept only if every free cell of the room still reaches the door. The pantry comes first in kitchen kits, so it is always placed; the kitchen sink carries the mains water.
- `zs gen houses` prints every dwelling; `zs gen house --household N [--out f.toml]` prints and writes one.
- `engine/gen/tests/test_dwellings.cpp`: 4 cases (unique addresses written back; area by members and income; every house of a 120-person town at each income loads as a `SiteMap` with every free cell reachable from the street, beds equal to members, rule 7's food, one pantry, a fridge only when the income has one; same seed same bytes, golden pinned on MSVC).
- Measured on seed 1 (60 people, upper-middle): 27 dwellings from 36 m² (one person) to 238 m² (seven people). One person on a low income gets 30 m² with no fridge and no bathroom; on a high income, 48 m².

## Not in this spec

Apartments and multi-storey houses; business premises and public buildings (from the establishment and business records); streets laid out in space and plots placed in a town; the window or the run loading a generated house in place of the hand-made block (a decision of its own: it re-pins the goldens); culture and era kits (the plan and kit are one style for now); damage from the outbreak's history; items beyond food (tools, clothes, valuables per occupation).

## Open

1. The area per person and pantry days per income group are estimates; a research note on housing by income (UN-Habitat, census tables) should replace them.
