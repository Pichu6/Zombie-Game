# 0053 · Real country data under parody names

- **Date:** 2026-10-08
- **Status:** Decided by the user (the idea and the names); the details marked "default taken by Claude" follow [0052](0052-one-executable-one-simulation.md) item 6 and can be overturned.
- **Extends:** [0034](0034-nation-demographics.md) (nation profiles), [0046](0046-realistic-numbers-from-free-data.md) (realistic numbers), specs [43](../tier0/specs/43-global-tables.md)–[48](../tier0/specs/48-people-sectors-countries.md).

## Decision

The default countries use **real countries' data** under **names that echo the real ones without being them**: Argentonga (Argentina), Mussia (Russia), Mile (Chile). A player recognises Mussia as a Russia-like country, and its people, ages, schooling, work and households are Russia's real numbers; the name, and everything that is not data, is fictional.

## Defaults taken by Claude

1. **One profile per real country** with data, written by `tools/data/countries.py` from the same open sources as the base countries (UN WPP 2024, UN household database, ILO modelled estimates, Wittgenstein Centre, World Bank). A table a country lacks falls back to its income group's aggregate, and its source says so.
2. **Names** are hand-written in one data file (ISO3 code to parody name), one per country, each recognisable and none the real name; no flags, emblems or real leaders.
3. **On a generated planet** each country takes a real profile of its income group, the one nearest in population (scaled to the planet), and no profile is used twice. (Climate is not matched: the tables have no climate per country.) Its name is that profile's parody name. A real-Earth map stays later ([0038](0038-grid-sector-zone-sizes-accepted.md) item 3).
4. The four base countries stay as the income-group averages, for worlds with more countries than profiles and for tests.
