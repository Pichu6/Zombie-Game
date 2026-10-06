# 0026 · Country creation like Stellaris species creation; traits pass from parents to children

- **Date:** 2026-10-06
- **Status:** Decided by the user (intent below). Model in [macro layer](../design/macro-layer.md) §6.5 is a Proposal; research `loop/28`.
- **Extends:** [0022](0022-macro-layer-intent.md).

## Decisions

- **Create a country the way Stellaris creates a species and empire:** population, politics, culture, religion and more, and these parameters shape the pops it generates.
- **Children inherit from their parents:** a child of parents with a given ancestry, religion or political stance starts with those; parents of different backgrounds produce a child with a mix, and over time mixes can lead to something new.

## Realism rule (Proposal, adopted as the default)

Genes pass only the body (looks, height potential, some health risks). Culture, language, religion and politics are learned from parents first, then school, peers and events, so a child starts from the family's and can change. Ancestry never gives ability or behaviour bonuses or penalties; the schema has no field that could tie them, and the validator rejects one.

On real Earth, real religion names are labels and data only, with no bonuses; generated worlds use invented faiths (default taken, research `loop/28`).
