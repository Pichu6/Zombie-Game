# 0006 · Dismantling, crafting and quality

- **Original date:** recorded in the prototype (2026-09); brought into the new project on 2026-10-05
- **Status:** Decided by the user in the prototype. Revalidate if anything changed with our own engine.

## Decisions

- Every item can be dismantled and crafted; almost all spawn already assembled.
- Every item has a quality (Q), tied to its parts, its physical data and its look.
- Haven & Hearth is the reference: knowledge filters which actions each person sees; quality comes from the parts and skill caps it.
- Higher quality gives more speed (up to 50 %), less wear and fewer critical failures.
- Learning applies to all items. Studying takes time, and the first new actions teach more than repetition.

## Open

The exact formula. The prototype used a soft cap of √(Q×S); the research proposes a weighted mean of the parts capped by min(skill, tool quality). See [items and crafting](../design/items-crafting.md).
