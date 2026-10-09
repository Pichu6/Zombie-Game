# 48 · People, sectors, countries and towns on the world grid

**Status:** Implemented and tested, 2026-10-08 (`zs/gen/world_people.h`, `choose_town`, `Place` town fields; `test_world_people.cpp`; MSVC). **Stage:** the world generator, steps 5, 7 and 8 of [world systems](../../design/world-systems.md) §4 in a first form; [0052](../../decisions/0052-one-executable-one-simulation.md). **Needs:** [30](30-world-grid.md) (grid), [31](31-terrain-climate.md) (terrain and climate), [42](42-place.md) (the block's place), [43](43-global-tables.md)–[46](46-education-labour-occupations-settlement.md) (base countries). Decisions: [0037](../../decisions/0037-sectors-zones-and-world-creation-answers.md), [0038](../../decisions/0038-grid-sector-zone-sizes-accepted.md), [0036](../../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md).

## Purpose

A generated planet has terrain and climate but nobody on it. This spec puts the present-day population on it where land suits people, divides the land into sectors, groups sectors into countries with an income level each, and places towns by rank-size. The block then stands in a real town of a real (generated) country, and that country's income level picks the base country the one world uses.

## What the design forces, and what this slice skips

- Capacity K (soil, water, deposits: steps 2–4) is not built, so population follows a **suitability** of climate, water, height and coast, not K. Routes (step 6) are not built, so sectors grow by terrain cost, not by route reach.
- Present day ([0036](../../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md)): the world's population is today's Earth's scaled by land area, and each income group holds its real share of it.

## Rules

1. **Suitability** of a land cell: `s = f_T · f_P · f_H · f_C`, sea and ice 0.
   - `f_T = exp(−((T − 13)/7)²)`, T the annual mean (°C): people concentrate where the annual mean is about 13 °C (the human climate niche, Xu et al. 2020, PNAS); the width is an estimate.
   - `f_P = P / (P + 500)`, P the annual rain (mm): water, saturating (estimate).
   - `f_H = exp(−h / 1500)`, h the elevation (m) (estimate).
   - `f_C = 1.5` within one cell of the sea, else 1 (about 40 % of people live within 100 km of a coast; the bonus is an estimate).
2. **Population.** The world total is 8,091,734,930 (WPP 2024, 2023) times this planet's land area over Earth's 148.9 million km². A land cell's people are proportional to `area · s^α`, α = 1.5 (estimate: concentration), in whole people.
*Since [0063](../../decisions/0063-real-size-planet-and-the-zoom-continuum.md) the one world is generated at frequency 101, not 24. Rules 3-5 are superseded by [spec 52](52-land-water-capacity.md) slice 4 (2026-10-09): countries form from the land by a contest history, sectors are cut inside each country with rule 3's method, and income groups are filled from endowment, access and history instead of suitability. Kept below as built on 2026-10-08.*

3. **Sectors.** Target area 3 million km² (Earth: about 50 sectors, "dozens", [0038](../../decisions/0038-grid-sector-zone-sizes-accepted.md)); count = max(1, round(land area / target)). Seeds: the most populous land cells, each at least `0.7 · √target` from earlier seeds. Every land cell joins the seed it reaches cheapest by multi-source shortest paths over the grid: a land step costs its distance times (1 + |Δh| / 500 m); a sea step 4 times its distance (estimates). Sea cells belong to no sector.
4. **Countries.** Count = max(1, round(sectors / 3)) (design: a country of a few sectors at the start). Capitals: the most populous sectors, spaced like rule 3 in sector-centroid distance; every sector joins the capital it reaches cheapest over sector adjacency (centroid distance, sea links × 4). A country's sectors are therefore connected by land or a declared sea link.
5. **Income groups.** Countries are ranked by a development score, their mean suitability times a keyed draw in [0.5, 1.5] (luck and history the slice does not model), and filled from the top until each group (high, upper-middle, lower-middle, low) holds its WPP 2023 share of the world's people (15.6 / 34.7 / 40.2 / 9.1 %, from the global tables). Each country's base country is its group's.
6. **Towns.** A country's urban people are its people times its group's urban share (spec 46). Towns of at least 10,000 people follow rank-size with ζ = 1.11 (Soo 2005, mean over 73 countries): `size_k = size_1 · k^(−1/ζ)`, with K towns and `size_1 = 10,000 · K^(1/ζ)`, K the largest count whose sizes sum to no more than the urban people. Town k stands in the country's land cell with the most people not yet given to a town; the capital is the largest.
7. **The block's town.** The one world (`zs::gen::make_world`) puts the block in a town of 10,000 to 200,000 people in a temperate cell whose annual mean is nearest 12 °C (as spec 42, now among towns); its country's group picks the one world's base country instead of the fixed upper-middle default of spec 43. `Place` gains the town's population, the sector, the country index and its income group.
8. **Determinism and record.** Every step is a pure function of (grid, fields, seed) with keyed draws; each town and sector records why it is where it is (its seed cell, its rank).

## Checks (tests)

- People add up to the scaled total within one person per cell; sea and ice cells have none.
- Every land cell is in exactly one sector; sector count within one of the target formula; each sector is connected through its own cells or a sea step.
- Income groups' shares of people within 10 points of WPP's (whole countries: with 16 of them seed 1 lands within 4).
- Each country's urban people within 10 % of its group's share (rank-size in whole towns); fitted rank-size slope within 0.1 of ζ for countries with 10 or more towns.
- The same seed gives the same world; `make_world` puts the block in a town of its rules.

## Results (seed 1, frequency 24)

49 sectors, 16 countries, 74,587 towns of 10,000 or more, 8.04 billion people. Income groups: high 19.6 % of people (WPP 15.6 %), upper-middle 31.1 % (34.9 %), lower-middle 37.0 % (40.4 %), low 12.3 % (9.1 %). The block stands in a temperate-forest town of 11,149 people in an upper-middle-income country.

## Not in this spec

Capacity K, deposits and routes (steps 2–4, 6); borders shaped by circumscription and reach (step 8 in full); zones inside towns; country names and cultures; real Earth.
