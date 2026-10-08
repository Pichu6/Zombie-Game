# 0051 · Money and resources shown per layer: the pop at Individual, the selection at City, the country at Macro

- **Date:** 2026-10-07
- **Status:** Decided by the user.
- **Extends:** [0044](0044-information-scope-per-layer.md) (information scope per layer) and [0042](0042-pop-presentation-per-layer.md) item 2 (clicking a pop or a building at City). Gives the money and stocks jumps ([layer jumps](../design/layer-jumps.md) rows 5 and 6) their display rule before they are built.

## Decisions

1. **Individual (layer 1):** the money and resources shown are the played pop's own: wallet, what it carries and owns. Nothing else.
2. **City (layer 2):** what is shown depends on what is selected.
   - A **pop** selected: that pop's money and resources (its personal info, as in [0042](0042-pop-presentation-per-layer.md)).
   - A **building** selected: that building's money and resources (its accounts and inventory, and general info about the pops in it).
   - **Nothing** selected: only the region's information: the stocks, buildings, staff and infrastructure of the city or place the player is at ([0044](0044-information-scope-per-layer.md) item 2). Never the country's.
3. **Macro (layer 3):** the money and resources shown are the country's: budget, revenue, debt, stocks and capacity by region.

## Consequences

- **Every owner is a ledger account.** The pop, the building (establishment, household, warehouse), the region and the country each hold money and stock on the same ledger ([layer jumps](../design/layer-jumps.md) §4 items 2 and 3). The display rule is then one rule: show the account of the selected owner, where the layer fixes the default owner (the pop at Individual, the region at City, the country at Macro) and a City selection overrides it.
- **No sums are shown that are not the selected owner's.** A building's panel does not show the region's stock; the region's panel does not show the country's. What a place can do may still be gated by what the country has, shown as a reason, not as the country's inventory ([0044](0044-information-scope-per-layer.md)).
- **A selection does not change what is true,** only what is displayed. Fog still applies on top in micro → macro play ([0012](0012-world-simulation-layers.md), [0044](0044-information-scope-per-layer.md)).
- **For the stocks and money build** (next in [layer jumps](../design/layer-jumps.md) §5): the ledger needs owner ids at all four scopes from the start, and a query "account of owner X" that the panels at every layer share. The jump tests (conservation across a visit and a round trip, A1) hold per owner and in total.
