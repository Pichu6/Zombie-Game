# 0044 · Information scope per layer

- **Date:** 2026-10-07
- **Note:** written on 2026-10-07 in a local session with a lower number and renumbered on merge.
- **Status:** Decided by the user.
- **Extends:** [0012](0012-world-simulation-layers.md) (fog), [0042](0042-pop-presentation-per-layer.md), [0043](0043-city-layer-and-repercussion-loop.md).

## Decisions

1. **Individual** shows only the character's own info and resources: body, mind, things carried and owned, knowledge, ties, beliefs.
2. **City** shows only the resources and info of the city or place the player is at: its stocks, buildings, staff, infrastructure, edicts in force. Not the country's.
3. **Macro** shows the country's info and resources: population, politics, standings, stocks and capacity by region.

## Consequences

- What a city can build is still gated by what the country has ([0043](0043-city-layer-and-repercussion-loop.md)), but City shows the gate as a reason ("no transformer available in the region", "banned by edict"), not the country's inventory.
- A resource moves between scopes only by a real transfer through the ledger (a shipment, a requisition, a pop carrying it), so a city never sees or spends what has not arrived.
- Fog stays on top: in micro → macro play each scope shows what the character believes, with source and age ([0012](0012-world-simulation-layers.md)); in macro → micro or spectator play the scopes show the truth of their unit.
- The same rule applies to pops ([0042](0042-pop-presentation-per-layer.md)): a pop's full info is Individual scope; City shows what the place knows about its people; Macro shows counts and the people in office.
