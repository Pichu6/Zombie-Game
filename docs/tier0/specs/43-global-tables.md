# 43 · Global tables and base countries

**Status:** Implemented and tested, 2026-10-08 (`tools/data/global_tables.py`, `data/setup/world/wpp2024.toml`, `data/setup/demography/wpp2024_*.toml`, `data/setup/nations/base_*.toml`; `test_global_tables.cpp`; MSVC). **Stage:** [0052](../../decisions/0052-one-executable-one-simulation.md) item 4, first slice. **Needs:** [28](28-pop-levels.md) (pop lives), [0034](../../decisions/0034-nation-demographics.md) (nation profiles), [0046](../../decisions/0046-realistic-numbers-from-free-data.md) (realistic numbers from free data).

## Purpose

Every pop number was a placeholder. This spec replaces the demography with real tables: the age structure, life tables and fertility of the world and of the four World Bank income groups from the **UN World Population Prospects 2024** (CC BY 3.0 IGO), and fits the simulation's own models to them, so the pops the simulation draws live and have children the way real populations do. Four fictional **base countries** carry those numbers; the one world's country is one of them.

## What the data forced

- **Mortality fits the existing model.** A constant plus a Gompertz hazard (spec 28) fitted to log death rates at ages 15–89 gives life expectancy at 15 within 0.6 years of WPP in every group and sex. At birth the model is 0.1 to 3 years too high, because it has no child-mortality term (low-income infant mortality is 43 per 1000).
- **Fertility did not fit the existing model.** An exponential delay to the first birth and exponential gaps cannot make the real shape (rising to a peak at 25–29, then falling): the best fit put the mean age at childbearing three years late. So the model gains an **age-specific** form (rule 3), which reproduces the rates exactly; the old form stays for old saves and the stage scenarios.

## Rules

1. **The tool.** `python tools/data/global_tables.py [RAW_DIR] [--year 2023]` reads the four WPP 2024 CSV files (population by age and sex, abridged life tables, fertility by age, demographic indicators) from a folder outside the repo (default `../data_raw/wpp2024`), keeps the World, High-, Upper-middle-, Lower-middle- and Low-income aggregates for one year (2023, the last estimate year of the life tables), and writes the three kinds of file below. Only derived tables enter the repo; every file names its source and year.
2. **Global tables** (`data/setup/world/wpp2024.toml`): per group, age shares in 20 five-year bands (95+ merged) for both sexes and each sex, abridged life table `qx` by sex, age-specific fertility 15–49, total fertility, mean age at childbearing, sex ratio at birth, life expectancy by sex, infant and under-five mortality, median age.
3. **Age-specific fertility.** `Fertility::hazard_by_band`: a birth hazard per year, constant within each five-year band from 15 to 50; after a birth none for `gap_min` (1 year, an estimate: pregnancy plus the shortest postpartum interval). A birth is drawn by inverting the band-wise cumulative hazard at a keyed exponential draw (closed form, so lives stay random-access). The tool fits the hazards so the model's rates equal WPP's (fixed-point iteration on the renewal equation `b(a) = h(a)(1 − births in (a − gap, a])`). Empty hazards keep the spec 28 model.
4. **Demography presets** (`data/setup/demography/wpp2024_<group>.toml`): mortality by sex (Gompertz–Makeham fit), the fertility hazards, the male share at birth from the sex ratio, the age pyramid, `source` and `year`, and a `[check]` table with the model's life expectancy at 15 (read only by tests).
5. **Base countries** (`data/setup/nations/base_<group>.toml`): the group's sourced age pyramid and preset; education, labour, occupations, sectors, households and settlement are still the placeholder shapes, so their status stays `placeholder` until those tables are sourced.
6. **The one world's country** is `base_upper_middle` (default taken by Claude, [0052](../../decisions/0052-one-executable-one-simulation.md) item 6: the group closest to the block's setting). Its preset is the registry's parameters; the census founders keep their own ages.
7. **Save.** The registry writes version 6 when its parameters carry fertility hazards (the hazards after the pyramid, the embodiment trailer always); older versions load as before. `fixtures/popreg_v6.bin` is frozen.

## Results (2023)

| Group | e15 m / f, model | e15 m / f, WPP | e0 m / f, model | e0 m / f, WPP | TFR model / WPP | Mean age at childbearing model / WPP | Never a mother |
| --- | --- | --- | --- | --- | --- | --- | --- |
| World | 58.9 / 64.1 | 58.9 / 64.1 | 72.7 / 78.2 | 70.6 / 75.9 | 2.24 / 2.25 | 28.3 / 28.2 | 8 % |
| High | 64.5 / 70.1 | 64.3 / 69.5 | 79.0 / 84.9 | 78.8 / 84.1 | 1.47 / 1.47 | 30.8 / 30.8 | 21 % |
| Upper-middle | 59.1 / 65.2 | 59.2 / 65.2 | 73.2 / 79.7 | 72.9 / 79.0 | 1.47 / 1.48 | 28.4 / 28.3 | 21 % |
| Lower-middle | 56.5 / 60.3 | 56.4 / 60.3 | 70.3 / 74.3 | 67.6 / 71.6 | 2.55 / 2.55 | 28.0 / 28.0 | 6 % |
| Low | 53.1 / 57.3 | 53.2 / 57.3 | 65.6 / 70.5 | 62.6 / 67.3 | 4.52 / 4.53 | 29.3 / 29.2 | 0.4 % |

TFR differences are WPP's births at 10–14 and 50–54, outside the model's ages. The simulation's own draws (200,000 lives per case, `test_global_tables.cpp`) give life expectancy at 15 within 0.05 years of the fitted model and TFR within 0.1 %.

## Not in this spec

- **Child mortality**: done in [44](44-child-mortality.md) (the Siler model, refitted on all ages); the mortality numbers in the table above are spec 43's fit.
- **Childlessness by choice and sterility**: the low-income group's 0.4 % never-mothers is too low (primary sterility alone is a few per cent); a share of women with zero hazard would fix it.
- **The other tables**: households (UN household size and composition), labour and sectors (ILOSTAT, CC BY 4.0), education, settlement. Each needs its own download, asked for by name and size.
- Migration, and the change of rates over time (the tables are one year).
