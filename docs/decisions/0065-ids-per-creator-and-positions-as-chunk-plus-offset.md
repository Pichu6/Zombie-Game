# 0065 · Ids are minted per creator; positions are a chunk plus a local offset

- **Date:** 2026-10-09 (recorded 2026-10-10)
- **Status:** Defaults taken by Claude ([0052](0052-one-executable-one-simulation.md)); the user answered "whatever is best" (2026-10-09). Both are decided now, before chunks exist, because they are cheap to change today and costly after.
- **Builds on:** [0041](0041-threading-and-partition.md) (ids monotonic, never recycled), [0063](0063-real-size-planet-and-the-zoom-continuum.md) (a real-size planet), [spec 27](../tier0/specs/27-detail-levels.md) (chunks), research [perception-and-run-ahead](../research/perception-and-run-ahead.md) (RA-114) and [star-engine](../research/star-engine.md) (SE-26).

## Decision

1. **Ids per creator.** An entity id is minted by whoever creates it, from its own counter: for example (chunk, tick, sequence). Ids stay unique and are never recycled (0041). Today one global counter (`World::new_entity`, 5 call sites) makes two chunks' creations depend on each other's order, which breaks local rollback and parallel chunks (RA-114).
2. **Positions as chunk plus local offset**, or integers at 0.5 m cells, never one big float. A 32-bit float resolves only about 4 m at planet scale (SE-26); Star Citizen moved to 64-bit positions for the same reason (SE-1).

## Consequences

- Both go into spec 27 (chunks) before any chunk code; the five call sites of `World::new_entity` move to the new scheme in that work.
- Saves and replays carry ids in the new form; the save format version records the change (spec 06).
