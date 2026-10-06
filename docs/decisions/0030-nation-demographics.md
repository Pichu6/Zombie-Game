# 0030 · A country has its own demographics

- **Date:** 2026-10-06
- **Status:** Decided by the user (intent below, from their message in chat). The profile format and its use are Proposals in [sandbox parameters](../design/sandbox-parameters.md) section 5.2 and [spec 28](../tier0/specs/28-pop-levels.md).
- **Extends:** [0023](0023-no-outbreak-game-and-parameter-catalogue.md), [0028](0028-real-time-reference-and-generational-runs.md) (real Earth data is the default for everything).

## Decisions

1. **Each country has its own demographics, like real life:** the level of education of its pops, what work they do, and the rest of its demographic structure. Countries differ from each other the way real ones do.

## Notes (not decisions)

- A profile is data (`data/setup/nations/<id>.toml`): age structure, education by age group, labour participation and unemployment, occupations and sectors, household sizes, urbanisation, plus the mortality and fertility preset it names.
- **No invented numbers presented as real.** A profile is `placeholder` (a plausible shape) or `sourced`; the loader refuses `sourced` without a source and a year on every table. The only profile so far is a placeholder.
- Real tables need real sources and licence checks (the licence list in [world simulation](../design/world-simulation.md) section 3). Candidate sources, unchecked: UN World Population Prospects, UNESCO Institute for Statistics and Barro-Lee (education), ILO modelled estimates (labour, sectors, occupations), World Bank (urbanisation), national census tables (households).
