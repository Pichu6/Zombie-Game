# 08 · Component and assembly

**Status:** Proposal. **Stage:** 2. **Needs:** [03](03-stock-transfer.md), [07](07-condition.md).

## Purpose

One data model for everything built, from a nail to a wall to the generator ([0010](../../decisions/0010-concept-v2.md), "everything built is scavengable and craftable"). Salvage returns the real parts; making builds the graph.

## State

| Field | Type | Conserved | Notes |
| --- | --- | --- | --- |
| definition | id, family, mass (g), slots, capabilities, recipes | — | Data file |
| instance | id, definition, Q (0–100, fixed), condition ([07](07-condition.md)), location | Identity yes | One owner |
| slots | slot name → component instance or empty | — | e.g. generator: engine, alternator, tank, spark plug, air filter, oil |
| joint | per slot: screwed / nailed / welded / glued / loose | — | Sets which salvage process applies |
| contents | stack stocks ([03](03-stock-transfer.md)) | Yes | Fuel in a tank is contents, not structure |
| functional properties | derived | — | e.g. generator output W, door hold force N |

## Rules

1. Mass of an assembly = sum of mounted parts + contents, each counted once.
2. A functional property names the part that governs it (door hold force ← strike plate and hinges; generator output ← engine and alternator). It reads that part's Q and condition, not the average.
3. Q of a made or repaired assembly: weighted mean of the governing parts' Q, capped by min(skill, Q_tool, method ceiling, Q_station) ([0014](../../decisions/0014-open-questions-answered.md)). Tier 0 has only salvage and hand methods and no stations, so Q_station does not cap yet.
4. A missing or failed required part stops the function. The average never lets an incomplete object work.
5. Salvage moves the real parts out through the ledger, minus a declared loss that drops with skill and tool Q. It never rolls new loot. Disassembling and reassembling never raises Q.
6. Composition is fixed when the instance is created (from the map file or a seed) and saved.

## Invariants

- Mass is conserved through salvage and assembly, except the declared loss, which goes to a sink.
- Every instance has one location; slots and containers form a tree.

## Tests

- Property: salvage then reassemble gives the same parts and never more Q. (A1)
- A generator with no spark plug produces 0 W. (A1)
- Q example from the design doc: Q40 head and Q90 handle at 70/30 → Q55 before caps. (A1)

## Calibration

Masses of Tier 0 items from manufacturer spec sheets (generator ~45 kg for a 3 kW unit, solid-core door ~25 kg); listed in the data file with their source.

## Not in Tier 0

Workshop, machine shop and plant methods; process stability spread; learning that changes skill.
