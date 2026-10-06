# 0038 · Global grid, sector and zone sizes, and real Earth later: recommendations accepted

- **Date:** 2026-10-06
- **Status:** Decided by the user ("lets go with what you recommend"). Sizes are starting values, tuned in the first generator spec.
- **Answers:** the open questions of [world systems](../design/world-systems.md) section 9.

## Decisions

1. **Global grid:** an equal-area grid on the sphere (an icosahedral hexagon-and-pentagon grid), about **5,000 km² per cell at Earth size (about 100,000 cells)**, so areas and distances stay honest at any planet size. A flat metric grid only for local maps.
2. **Sectors and zones:** a **sector** is a few hundred to a few thousand world cells, so a world has **dozens**; **zone cells are 100 to 500 m**, inside settlement footprints only.
3. **Real Earth** is a later, separate data pipeline; generated worlds come first.

Names and levels: [0037](0037-sectors-zones-and-world-creation-answers.md).
