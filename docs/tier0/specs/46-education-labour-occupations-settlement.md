# 46 · Education, labour, occupations and settlement

**Status:** Implemented and tested, 2026-10-08 (`tools/data/global_tables.py`, `tools/data/rds.py`, base countries, `test_global_tables.cpp`; MSVC). **Stage:** [0052](../../decisions/0052-one-executable-one-simulation.md) item 4, last slice after [43](43-global-tables.md)–[45](45-households-and-sectors.md). **Needs:** [0034](../../decisions/0034-nation-demographics.md).

## Purpose

The base countries' last placeholder tables are sourced, so each base country is a complete, real income-group profile (`status = "sourced"`). With this, every table of a nation profile names its source and year.

## Sources

| Table | Source | Licence | How it was obtained |
| --- | --- | --- | --- |
| Education: attainment by age group | Wittgenstein Centre Human Capital Data Explorer, WIC2023, SSP2 (K.C. et al. 2024, IIASA WP-24-003), `pop-age-edattain.rds` | none stated: aggregated shares used as published facts ([0046](../../decisions/0046-realistic-numbers-from-free-data.md) route 2); the raw file stays outside the repo | the `wcde` package's public data repository (GitHub), read by `tools/data/rds.py` |
| Labour: participation by sex and age, unemployment | ILO modelled estimates (`EAP_2WAP_SEX_AGE_RT`, `UNE_2EAP_SEX_AGE_RT`), 2023 | CC BY 4.0 | the ILO SDMX API (no bot check, unlike the bulk site) |
| Occupations | ILO modelled estimates (`EMP_2EMP_SEX_OCU_NB`), 2023 | CC BY 4.0 | the ILO SDMX API |
| Settlement: urban share | UN World Urbanization Prospects via World Bank WDI `SP.URB.TOTL.IN.ZS`, 2023 | CC BY 4.0 | the World Bank API |

## Rules

1. **Income groups.** The ILO and the World Bank publish the four income-group aggregates themselves (ILO codes X02–X05, X01 the world); the education shares are summed over the group's countries by the World Bank classification (WIC counts people, so the sum is the group's own distribution), interpolated to 2023 between WIC's 2020 and 2025.
2. **Education levels.** None = no education + incomplete primary; primary; secondary = lower + upper secondary; tertiary = WIC's post-secondary (short post-secondary + bachelor + master and higher). Age groups 25–34, 35–64, 65+.
3. **Occupations follow the data:** the ILO's modelled estimates have eight groups (ISCO-08 1, 2, 3, 4, 5, 7, 8, and 6 with 9 combined; armed forces are spread over the others), and they add up to total employment. `kOccupationGroups` is 8; splitting the combined group would be invention.
4. **Labour.** Participation for 15–24, 25–54, 55–64 and 65+ by sex and the 15+ unemployment rate, as published. The retirement age has no open source by income group: 65, written as an estimate in the table's source.
5. **The `.rds` reader** (`tools/data/rds.py`): R's XDR serialization for data frames of numbers, strings and factors, gzip-compressed, standard library only.

## Results (2023)

| Group | Tertiary, 25–34 | No schooling, 25–34 | Participation 25–54 men / women | Unemployment | Urban |
| --- | --- | --- | --- | --- | --- |
| World | 27.0 % | 15.1 % | 92.1 / 64.1 % | 4.9 % | 57.3 % |
| High | 47.5 % | 1.0 % | 92.1 / 80.1 % | 4.3 % | 80.0 % |
| Upper-middle | 31.8 % | 4.7 % | 90.7 / 71.0 % | 5.6 % | 67.4 % |
| Lower-middle | 18.2 % | 23.1 % | 94.5 / 50.0 % | 4.3 % | 41.2 % |
| Low | 6.9 % | 50.9 % | 86.9 / 60.8 % | 5.3 % | 34.3 % |

## Not in this spec

- The generators and the economy reading the base country (education of generated pops, jobs by sector and occupation, urban share for settlements): with the generator's sectors and zones.
- Retirement ages by country (OECD and national pension tables have no open licence we have confirmed).
- Real countries: the same tool with one country's code instead of an income group.
