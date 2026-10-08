# 49 · Real-country profiles under parody names

**Status:** Implemented and tested, 2026-10-08 (`tools/data/countries.py`, `data/setup/nations/real/`, `data/setup/demography/real/`, `data/setup/nations/parody_names.toml`, `load_real_profiles`; `test_global_tables.cpp`, `test_world_people.cpp`; MSVC). **Stage:** [0053](../../decisions/0053-real-country-data-with-parody-names.md). **Needs:** [43](43-global-tables.md)–[48](48-people-sectors-countries.md).

## Purpose

The planet's countries were the four income-group averages. Now each is a real country's data under a parody name (Mussia, Argentonga, Mile), so countries differ the way real ones do: Mussia's men live 12 years less than its women, Nigerre has six children per woman, Mile has four in five people in towns.

## Rules

1. **Profiles.** `python tools/data/countries.py` writes, for every country with UN WPP 2024 and ILO modelled data and a World Bank income group (183), a demography preset (Siler mortality and age-specific fertility fitted to its own tables, specs 43–44) and a nation profile with every table of spec 46 from its own data. A table it lacks takes its income group's aggregate, and its source says so: 19 countries take the household table that way (none lacks another). Each profile carries a `[country]` table with its income group and 2023 population.
2. **Names.** `data/setup/nations/parody_names.toml`: one hand-written name per ISO3 code, recognisable and not the real name in English or in other languages we checked (Japonia is Polish for Japan, so it is Japanoo). The loader test checks that no name equals the real name in its own sources.
3. **On the planet** (`populate_world` with profiles): countries, largest first, take the unused profile of their own income group nearest in people (the profile's people scaled by this planet's people over Earth's); with none left, a country stays its group's base country.
4. **The one world** takes the block's country's real profile for its registry and shows its name; `Place` carries the profile and the name. The stage 9 golden hash is re-pinned on MSVC.

## Results

- Fits over the 183 countries: life expectancy at birth within 1.2 years of WPP for every country and sex (worst: Venezuela); TFR within 0.05.
- Planet 1: Mussia, United States of Amerigo, Chyna, Indus, Brazilo, Mexicana, Indonasia, Ethiopa, Nigeriana, Pakistun, Bangladosh, Egypta, Tanzanica, Kenja, Grand Congoland, Myanmor. The block stands in a town of 11,149 in Chyna (upper-middle income).

## Not in this spec

- Matching by climate or geography (no per-country climate in the tables); names for towns from the country's culture; real borders (real Earth, later).
