# 0046 · Realistic numbers always; free data decides the route, not the target

- **Date:** 2026-10-07
- **Note:** written on 2026-10-07 in a local session with a lower number and renumbered on merge.
- **Status:** Decided by the user. Clarifies [0045](0045-open-questions-answered-2.md) item 11; the same intent as [0035](0035-realistic-world-numbers-and-the-pop-nation-loop.md) (realistic world numbers), seen from the data-licence side.

## Decision

The target is always realistic data. Using only free data that needs no permission ([0045](0045-open-questions-answered-2.md)) changes how a number is obtained, never whether it is realistic. When a dataset cannot be used, the same number is derived another way:

1. from a dataset with an explicit licence that measures the same thing (a national statistics office, WorldPop, the UN World Population Prospects, ILOSTAT, OECD);
2. from facts published in open papers and reports, which are not the restricted microdata themselves (a household size distribution printed in a CC BY paper is a fact we may use);
3. from a model calibrated on what we have (the synthetic household route fitted to the age pyramid and the home contact matrix, checked against published aggregates);
4. as a last resort, from a documented estimate with its source and uncertainty, marked in the data files like every other calibration number.

## Consequences

- Every number in `data/` still names its source; a derived number names the method and the inputs.
- The human generator's fallback route ([research/human-generator-data.md](../research/human-generator-data.md) §3 and §4) is a derivation, not a lower standard: its output is validated against published country aggregates (household size, age structure, employment by occupation).
- Health rates: NHANES scaled by national mortality, with published country prevalence figures from open papers where they exist, so the US calibration is corrected, not just labelled.
