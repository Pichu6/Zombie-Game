# 0054 · Research notes: findings with IDs, links both ways, checked

- **Date:** 2026-10-08
- **Status:** Decided by the user (the goal and the plan); the exact format is a default taken by Claude ([0052](0052-one-executable-one-simulation.md) item 6) and can be changed.
- **Changes:** how `docs/research/` is written and used. It doesn't change what a research note is: dated, and not a decision ([CLAUDE.md](../../CLAUDE.md)).

## Why

On 2026-10-08 the folder held 42 notes (about 600 KB). 14 were linked from no design, decision or spec, and 13 more from only one; 4 were missing from the index; a quarter had no status line; each marked confidence its own way; findings were buried in prose; and open questions sat at the end of notes that nobody rewrites. Research was read once, when it was written. The numbers that went into `data/` with their sources (demography, countries) are the exception: the simulation uses them, so they stay alive.

## Decision

1. **One header for every note**: `Date`, `Status`, `Feeds`, `Evidence` and `IDs`. `Feeds` links the design docs, specs or decisions the note serves, or says `background` when it serves none on purpose. `IDs` is the note's short prefix, unique in the folder.
2. **A findings table in every note**: `| ID | Finding | Confidence | Source |`, one row per finding, numbered `PREFIX-1`, `PREFIX-2`… IDs are never reused or renumbered; a finding that turns out wrong stays, marked wrong, and a new row replaces it. A verbatim record (the user's own vision) says so instead of a table.
3. **Four confidence words only**: `sourced` (the primary source was read), `reported` (from search summaries or secondary sources), `derived` (our arithmetic from other findings, which it names), `estimate` (our judgement).
4. **Cite findings by ID**: a design doc, spec or decision cites `[PZ-3](../research/project-zomboid.md)`, not "see the Zomboid note".
5. **Links both ways**: a design doc a note feeds links back to that note, ideally in a `Research` section.
6. **Numbers the simulation uses live in `data/`** with their sources, as the demography and country tables do; the note points to the file.
7. **Open questions belong to the design doc**, in its Open section, where they get answered. A note may list the questions it raised, but each one is also carried to the design doc it feeds.
8. **The index is grouped by topic**, not by file name.
9. **`tools/docs/check_research.py` checks all of it** (headers, findings, confidence words, unique prefixes and IDs, index, links, links back, cited IDs exist) and runs in CI on every push.

## Consequences

- A finding can be found by ID, cited, checked and corrected without rewriting a note.
- Research that feeds nothing shows up as a failing check, unless it says `background`.
- Writing a note takes a little longer: the findings table is the part that matters, the prose explains it.
