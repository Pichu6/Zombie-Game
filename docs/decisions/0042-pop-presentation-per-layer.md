# 0042 · How pops appear in each layer

- **Date:** 2026-10-07
- **Note:** written on 2026-10-07 in a local session with a lower number and renumbered on merge.
- **Status:** Decided by the user.
- **Extends:** [0012](0012-world-simulation-layers.md) (one fact, every layer), [world simulation §8 and §13](../design/world-simulation.md).

## Decisions

1. **Macro (cold):** a pop is a number or a statistic. Only important pops in places of power are shown individually, as an info card, possibly with a thumbnail.
2. **City (medium):** pops are small figures, Cities: Skylines style. Clicking a pop shows its personal info; clicking a building shows general info about the pops in it.
3. **Individual (hot):** a pop is fully rendered.

## Consequences

- Presentation follows the layer; simulation detail follows the player's body ([research/detail-levels.md](../research/detail-levels.md)). A City figure looks the same whether its chunk is warm or hot.
- Pops in places of power are records from world creation with a kept reason (office), so Macro can show them without promoting anyone.
- Personal info in City and Macro is the dossier: what the viewing character believes, with source and age, and it can be wrong ([0012](0012-world-simulation-layers.md)).
- Clicking never changes simulation state (looking is a pure read).
