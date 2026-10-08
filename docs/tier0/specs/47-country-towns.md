# 47 · Towns generated for a country

**Status:** Implemented and tested, 2026-10-08 (`zs::gen::set_country`, `Mix::country`, `data/gen/jobs.toml` groups; `test_country_town.cpp`; MSVC). **Stage:** [0052](../../decisions/0052-one-executable-one-simulation.md), after the base countries ([43](43-global-tables.md)–[46](46-education-labour-occupations-settlement.md)). **Needs:** the content tools ([content tools](../../design/content-tools.md) §4).

## Purpose

The town generator drew household sizes, schooling and jobs from fixed estimates of a US town. With a country set, it draws them from that country's sourced tables, so a town in the low-income base country has large households, little schooling and mostly farm and elementary work, and a town in the high-income one the reverse. Names, cultures, streets and lifepaths are unchanged.

## Rules

1. **The country.** A population mix may name `country = "<id>"` (a profile in `data/setup/nations/`), or `zs gen town --country <id>` sets it. Without one the generator draws exactly as before (same seed, same town).
2. **Household sizes.** A group from the country's shares (1, 2–3, 4–5, 6+); within 2–3 and 4–5 the upper size with one share `q`, fitted so the expected size equals the published mean; 6+ is 6 to 9 evenly (mean 7.5, an estimate). With the last household cut to the town's size the mean comes out within 1–2 %.
3. **Schooling.** Each adult draws an attainment level (none, primary, secondary, tertiary) from the country's shares for their age group (18–34 the 25–34 shares, 35–64, 65+); `Pop::attainment` keeps it. The generator's own education names follow: none and primary give "none", secondary "high school" (or "trade school" for a trade job), tertiary "college" (or the job's higher degree).
4. **Work.** In the labour force with the country's participation for the pop's age group and sex (18–24, 25–54, 55–64, 65+); in it, out of work with the unemployment rate; out of it, a college student (under 25 with secondary or more), retired (55 and over, with a last job) or keeping house.
5. **Occupations.** Every job has its ILO modelled group (`group` in `jobs.toml`, 0–7; soldiers counted with services, as the ILO's modelled groups have no armed forces). A group is open to a schooling level if one of its jobs is (none and primary: jobs needing none; secondary: up to trade; tertiary: all). Each group's weight is the country's share times a factor fitted once per town by iterative proportional fitting over the workers' schooling (25–34 and 35–64 shares, 35:65), so that the groups come out in the country's shares while a degree is still needed for a professional's job. Then a job of the group by the jobs' own shares.
6. **New jobs.** IT technician, lab technician and sales representative (group 2), so technicians have more than the paramedic. This changes the default town's draws; the town golden hash is re-pinned on MSVC.

## Results (20,000 people, seed 11)

| | High income: generated (table) | Low income: generated (table) |
| --- | --- | --- |
| Mean household size | 2.43 (2.46) | 5.23 (5.23) |
| Tertiary, ages 25–34 | 47.4 % (47.5 %) | 6.6 % (6.9 %) |
| Participation 25–54, men / women | 92.0 / 80.5 % (92.1 / 80.1 %) | 87.4 / 60.7 % (86.9 / 60.8 %) |
| Unemployment | 4.2 % (4.3 %) | 5.0 % (5.3 %) |
| Occupations, largest gap | 1.6 points (professionals) | 2.1 points (agriculture and elementary) |

## Not in this spec

- **The Tier 0 block's census** (`data/tier0/census.toml`) is still the fixed table of spec 22; regenerating it from the one world's country would change the run's people and its found survivors (named in `run.toml`).
- Ages from the country's pyramid (the generator builds ages from household roles), sectors of the jobs (occupation is drawn, sector follows the job), urban or rural towns from the urban share: with sectors and zones on the world grid.
