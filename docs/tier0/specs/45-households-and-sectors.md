# 45 · Households and sectors

**Status:** Implemented and tested, 2026-10-08 (`tools/data/global_tables.py`, base countries, `test_global_tables.cpp`; MSVC). **Stage:** [0052](../../decisions/0052-one-executable-one-simulation.md) item 4, third slice after [43](43-global-tables.md) and [44](44-child-mortality.md). **Needs:** [0034](../../decisions/0034-nation-demographics.md) (nation profiles).

## Purpose

The base countries' households and employment by sector were placeholder shapes. This spec sources them for the world and the four income groups, so the generators and the economy build towns whose homes and jobs have real proportions.

## Sources

- **Households:** UN DESA Population Division, *Database on Household Size and Composition 2022* (CC BY 3.0 IGO, stated in the workbook): average size and shares of households of 1, 2–3, 4–5 and 6+ members, per country and survey (DHS, MICS, IPUMS, DYB, LFS), 1960–2021. Downloaded through the built-in browser (the site refuses scripted downloads).
- **Sectors:** ILO modelled estimates (ILOEST) of employment in agriculture, industry and services, 2023, for the World Bank income groups, as the World Bank's open API serves them (WDI `SL.AGR.EMPL.ZS`, `SL.IND.EMPL.ZS`, `SL.SRV.EMPL.ZS`; CC BY 4.0). ILOSTAT's own bulk site sits behind a bot check we do not pass.
- **Grouping:** the World Bank's country income classification (its API, `incomeLevel`); country populations from WPP 2024.

## Rules

1. **Household groups.** A profile's `[households]` now has the UN's four groups (1, 2–3, 4–5, 6+) and an optional published `mean_size`; `mean_household_size` returns it when given, else the groups at 1, 2.5, 4.5 and 7 members (a stated approximation). Splitting the UN groups into single sizes would be invention, so the format follows the data.
2. **Aggregation.** For each country the latest survey; a group's shares are the countries' shares weighted by their number of households (2023 population ÷ average size); its mean is people ÷ households over the countries with data. The coverage (share of the group's people in countries with data) and the median survey year are recorded.
3. **Sectors** are the ILO aggregates as published, normalised to sum to 1 (they add to 100 % within rounding).
4. **Written to** the global tables (`household_size_shares`, `household_mean_size`, `household_coverage`, `household_median_year`, `employment_by_sector` per group) and the base countries' `[households]` and `[sectors]`, each with its source and year. Education, labour, occupations and settlement stay placeholders.

## Results

| Group | Households 1 / 2–3 / 4–5 / 6+ (%) | Mean size | Coverage | Median survey year | Employment agriculture / industry / services (%) |
| --- | --- | --- | --- | --- | --- |
| World | 13.9 / 39.2 / 31.3 / 15.6 | 3.66 | 98 % | 2015 | 26.6 / 23.5 / 49.9 |
| High | 29.5 / 48.3 / 19.1 / 3.1 | 2.46 | 95 % | 2011 | 3.1 / 22.6 / 74.3 |
| Upper-middle | 10.4 / 44.1 / 34.7 / 10.7 | 3.50 | 99 % | 2016 | 20.9 / 27.5 / 51.6 |
| Lower-middle | 6.9 / 28.3 / 37.3 / 27.6 | 4.56 | 100 % | 2017 | 41.1 / 21.9 / 37.0 |
| Low | 5.9 / 23.1 / 30.8 / 40.2 | 5.23 | 100 % | 2018 | 60.7 / 10.0 / 29.3 |

## Not in this spec

- **Using them:** the town generator's own household-size mix (`data/gen/populations/`) and the economy's jobs do not read the base country yet; that wiring comes with the generator's sectors and zones.
- **Survey years:** households are the latest survey per country (median 2011–2018), not 2023; household size falls slowly over time, so these slightly overstate it.
- Education, labour participation, occupations and urbanisation: further downloads, asked for by name.
