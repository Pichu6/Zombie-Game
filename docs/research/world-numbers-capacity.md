# World numbers: how many people can a world hold?

**Date:** 2026-10-06
**Status:** research note, not a decision. Nothing here is Decided, Implemented or Validated. It feeds [0035](../decisions/0035-realistic-world-numbers-and-the-pop-nation-loop.md) and [world-simulation section 12](../design/world-simulation.md).

**Method and limits.** WebFetch was blocked by the network proxy (ourworldindata.org, wikipedia.org, usgs.gov all returned EGRESS_BLOCKED). Every number tagged "search" below comes from WebSearch result summaries, not from reading the source page. Numbers tagged "derived" are our own arithmetic on cited inputs. Numbers tagged "memory" are from the author's background knowledge, were not confirmed in this session, and carry a URL only as a place to check; treat them as low confidence until someone opens the source. Confidence: H = summary quotes the number and it matches well-known values; M = quoted in a summary but second-hand or range-valued; L = memory or derived from weak inputs.

## 1. Numbers

### Planet and land

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Earth land area | 148.94 (29.1% of surface) | million km2 | https://www.arhfoundation.org/total-land-area-earth-148-94-million-sq-km (search) | M |
| Earth total surface (4 pi R^2, R = 6371 km) | 510.1 | million km2 | derived (geometry) | H |
| Habitable land (not ice, not barren) | 104, i.e. 71% of land | million km2 | https://ourworldindata.org/global-land-for-agriculture (search) | M |
| Agricultural land | about 48, i.e. 44% of habitable (OWID also says "roughly half") | million km2 | https://ourworldindata.org/global-land-for-agriculture (search) | M |
| Cropland vs grazing land | cropland 1/3, grazing 2/3 of farmland | share | https://ourworldindata.org/global-land-for-agriculture (search) | M |
| Cropland (arable) share of land | about 11% | share of land | https://wesr.unep.org/media/docs/geo/final_versions/part_a/chapter8_land/figure8_6_global_area_allocation_for_food_production.pdf (search; the summary was ambiguous about "land" vs "habitable land") | L |
| Livestock share of farmland / of calories / of protein | 77% / 18% / 37% | share | https://ourworldindata.org/global-land-for-agriculture (search) | M |
| Radius effect on climate | larger radius lowers albedo (warmer); higher gravity raises outgoing longwave (cooler); both secondary to stellar flux, atmosphere, rotation | qualitative | https://arxiv.org/abs/1904.12267 (search) | M |
| Land fraction, hypsometry vs radius | not found as an equation; Earth-only reference. Treat land fraction as a free parameter | not found | not found | n/a |

What generalises: area scales as R^2 (exact). Land fraction, ice share, desert share and arable share do NOT follow from radius; they follow from water inventory, relief and climate, so they should be generator outputs or sliders. Earth's shares are the reference point, not a law.

### Primary production and carrying capacity

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| NPP tropical forest | 2000-3000 | g (dry matter) m-2 yr-1 | https://see.leeds.ac.uk/enviroweb/biosphere/1_organic_matter/organic9.php (search) | M |
| NPP temperate forest | 600-1500 | g m-2 yr-1 | same | M |
| NPP temperate grassland | 400-1000 | g m-2 yr-1 | same | M |
| NPP savanna | 400-600 | g m-2 yr-1 | same | M |
| NPP Mediterranean | 300-600 | g m-2 yr-1 | same | M |
| NPP taiga / boreal | 200-600 | g m-2 yr-1 | same | M |
| NPP tundra | 100-200 | g m-2 yr-1 | same | M |
| NPP desert | 0-120 | g m-2 yr-1 | same | M |
| Second table, different units basis | tropical 2500, temperate forest 1550, grassland 750, desert 250, boreal 380, tundra 180 | g m-2 yr-1 | https://pmc.ncbi.nlm.nih.gov/articles/PMC4241063/table/MCG041TB1 (search) | L (desert 250 conflicts with 0-120; sources differ, so use bands) |
| Global terrestrial NPP | 56.8 | Pg C yr-1 | https://encyclopedia.pub/entry/13323 (search) | M |
| Human appropriation of NPP (HANPP) | 13% (1910) to 25% (2005); 6.9 to 14.8 | % of potential NPP; Gt C yr-1 | https://wad.jrc.ec.europa.eu/humanappropriation (search) | M |
| Hunter-gatherer density tracks NPP but with about 1000-fold unexplained variation per unit NPP; temperate and subtropical forest biomes give the highest density | qualitative | n/a | https://dx.doi.org/10.1038/s41467-022-32750-x (search) | M |
| Mesolithic France density | about 0.1 | people km-2 | https://archeologie.culture.gouv.fr/megalithes/en/population-changes (search) | M |
| Middle Neolithic France | 1-2 | people km-2 | same | M |
| Early farmers, Schleswig-Holstein 3700-3200 BC | 2.5-7.5 | people km-2 | https://hraf.yale.edu/ehc/documents/1289 (search, citing Hinz and Muller 2015) | M |
| Iron Age France (end) | 10-20 | people km-2 | https://archeologie.culture.gouv.fr/megalithes/en/population-changes (search) | M |
| Pacific Islands neolithic density (Childe) | 30 or more | people per square mile (about 12 per km2) | https://faculty.washington.edu/plape/citiesaut11/readings/Childe-urban%20revolution%201950.pdf (search; conversion is derived) | L |
| World population about 10,000 BC | about 4 million | people | https://en.wikipedia.org/wiki/Estimates_of_historical_world_population (search) | M |
| Hunter-gatherer density by biome (Binford/Kelly medians) | not found as numbers; the HG papers confirm the dataset exists | not found | https://dx.doi.org/10.1038/s41467-022-32750-x | n/a |
| Pre-industrial agrarian density, typical fertile river valley or China/India/Europe 1800 | not found in search; memory suggests tens to about 100+ per km2 | not found | https://ourworldindata.org/grapher/population-density (OWID has HYDE series; check it) | n/a |

### Yield by technology era

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| English wheat yield, 16th century | just over 1 | t ha-1 | https://ourworldindata.org/grapher/wheat-yields (search; OWID series starts 1275) | M |
| English wheat yield, about 1800 | 1.5 | t ha-1 | same | M |
| English wheat yield, 1965 | nearly 4 | t ha-1 | same | M |
| British wheat yield, 2018 | about 8 | t ha-1 | same | M |
| Medieval yield before 16th century | "nearly doubled" to reach 1 t, so about 0.5-0.6 | t ha-1 | derived from same | L |
| Population fed by Haber-Bosch nitrogen | 44% (2000), 48% (2008); Smil 40% (2000) | share of world population | https://www.iai.int/admin/site/sites/default/files/uploads/2008.Erisman-et-al_NatureGeo.pdf (search) | M |
| Wheat food energy | about 3.3-3.4 | kcal per gram | memory; check https://www.fao.org/ (FAO food composition tables) | L |

### People: calories, water

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| FAO minimum dietary energy requirement (MDER) | defined per country by age and sex mix, low activity; numeric average not obtained (memory: about 1700-1900 kcal) | kcal person-1 day-1 | https://ourworldindata.org/grapher/minimum-requirement-calories | numeric: not found |
| Typical working-adult food energy | about 2000-2500 (memory); 2000 kcal/day = 3.05 GJ/yr (derived) | kcal day-1 | memory | L |
| Water, survival minimum for drinking and food prep | 7.5 | L person-1 day-1 | https://www.ferrovial.com/blog/en/2020/03/how-many-litres-of-water-does-a-person-need-per-day/ (search, quoting WHO) | M |
| Water, basic access | at least 20 | L person-1 day-1 | same (WHO tiers) | M |
| Water, intermediate access | 50 | L person-1 day-1 | same | M |
| Water, optimal access | 100 | L person-1 day-1 | same | M |
| Agriculture share of freshwater withdrawals, 2022 | 71% (industry 15%, municipal 13%) | share | https://ourworldindata.org/grapher/agricultural-water-as-a-share-of-total-water-withdrawals (search) | M |
| Irrigation share of world food | about 40% | share | https://www.fao.org/nr/water/aquastat/water_use (search) | M |
| Falkenmark water stress / scarcity / absolute scarcity | 1700 / 1000 / 500 | m3 renewable water person-1 yr-1 | https://pmc.ncbi.nlm.nih.gov/articles/PMC5547033/ (search) | M |

### Energy and materials

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| World primary energy per capita 2024 | 73 | GJ person-1 yr-1 | https://dieselnet.com/news/2025/06energyreview.php (search, quoting Energy Institute) | M |
| North America 2024 | 217 | GJ person-1 yr-1 | same | M |
| United States 2024 | 265.9 | GJ person-1 yr-1 | same | M |
| Iceland / Qatar 2024 (extremes) | 788 / 769 | GJ person-1 yr-1 | same | M |
| US per capita, alternative figure | 78,754 | kWh person-1 yr-1 (= 283 GJ, derived) | https://en.wikipedia.org/wiki/List_of_countries_by_energy_consumption_per_capita (search) | L |
| Quality of life vs energy use | rises about linearly early, diminishing returns above 50-70 | GJ person-1 yr-1 | https://vaclavsmil.com/?p=5639 (search, Smil) | M |
| Pre-industrial and hunter-gatherer per-capita energy | not found in search (Smil's books hold it); a bottom-up bound is food alone, about 3 GJ/yr (derived), plus fuel wood | GJ person-1 yr-1 | https://vaclavsmil.com/book/energy-in-world-history/ | not found |
| Copper world mine production 2023 | 22 | million t yr-1 | https://www.usgs.gov/publications/mineral-commodity-summaries-2025 (search) | M |
| Copper reserves | 880-980 | million t | same | M |
| Copper reserves-to-production | about 40-45 (derived from the two rows above) | years | derived | L |
| Share of metal mine value 2021: copper 35%, gold 31%, iron ore 13%, zinc 7% | shares | US-mined value | https://skillings.net/u-s-mines-produced-an-estimated-90-4-billion-in-nonfuel-mineral-commodities-during-2021/ (search; US only, not world) | L |
| Iron ore, bauxite, coal, oil, gas, lithium, phosphate reserves and production | not found (USGS PDF unreachable) | not found | https://pubs.usgs.gov/periodicals/mcs2025/mcs2025.pdf | not found |
| Per-capita metal consumption by income level | not found | not found | not found | n/a |

### Dynamics (qualitative, cited)

| Quantity | Value | Source URL | Confidence |
| --- | --- | --- | --- |
| Malthus: birth rate rises, death rate falls with living standards; pre-1750 Europe "high pressure" equilibrium, slow uneven growth | qualitative | https://faculty.econ.ucdavis.edu/faculty/gclark/Farewell%20to%20Alms/FTA-chapter2.pdf (search) | M |
| England had only moderately high birth rates, lower than the old Malthusian model predicted (preventive check) | qualitative | https://www.lrb.co.uk/v20/n05/roy-porter/why-mr-fax-got-it-wrong (search) | M |
| Boserup: food pressure induces intensification, so catastrophes often do not come | qualitative | https://acawiki.org/Population_and_Technology_in_Preindustrial_Europe (search) | M |
| N. Italy 1650-1881: weak homeostasis, population growth cut living standards | qualitative | https://www.springerprofessional.de/en/malthusian-dynamics-in-a-diverging-europe-northern-italy-1650-18/6097800 (search) | M |

## 2. Equations we could implement

Every parameter has a **realistic band** (earned from the rows above) and a **sandbox band** (wide, tagged as realism-breaking per 0014). Bands marked "tune" have no cited value yet and must be calibrated against HYDE (see section 5) before use.

**E1. Planet geometry.** `A_total = 4 pi R^2`; `A_land = f_land * A_total`; `A_hab = A_land * (1 - f_ice - f_barren)`.
Earth: R = 6371 km, f_land 0.292, A_hab/A_land 0.71. Realistic band for a habitable world: f_land 0.15-0.45, A_hab/A_land 0.5-0.85. Sandbox: f_land 0.02-0.95. R realistic 0.7-1.5 R_Earth (so area 0.5-2.3x). Gravity and radius shift the habitable-zone edge only a little (arxiv 1904.12267), so use them for movement and build cost, not for land shares.

**E2. Land by biome.** Generator gives area shares `s_b` summing to 1 over `A_hab`, from the existing biome map. Each biome has `NPP_b` (bands in the table: forest 600-3000, grass 400-1000, savanna 400-600, boreal 200-600, tundra 100-200, desert 0-120 g m-2 yr-1) and an arable fraction `phi_b(era)`.

**E3. Arable and pasture limit.** `A_crop = sum_b s_b * A_hab * phi_b`, with `phi_b <= phi_max(era)`. Earth reference: cropland about 1/3 of 48M km2 farmland, i.e. about 16M km2, about 15% of habitable land today. Realistic `phi_max` by era: hunter-gatherer 0; early agriculture 0.03-0.08 of fertile biome area (tune); pre-industrial 0.10-0.25 (tune); mechanised 0.15-0.35 (Earth today about 0.15 of habitable, tune ceiling). Sandbox: 0-0.9.

**E4. Food per hectare by era.** `kcal_ha(era) = Y(era) * 1000 * e_grain * (1 - seed(era)) * (1 - loss(era))` with `e_grain` about 3300 kcal/kg (memory, verify).
Y (t grain/ha): medieval 0.5-0.8, 16th century to 1800 1.0-1.5, 1965-era 3-4, modern 6-8 (OWID wheat series, England; other crops and climates differ; apply a biome fertility multiplier 0.3-1.3, tune). Seed share 0.15-0.30 pre-industrial, under 0.05 modern (memory, tune). Loss 0.1-0.3. Sandbox Y: 0.1-15.

**E5. Agrarian carrying capacity.**
`K_ag = [ A_crop * kcal_ha * (1 - f_feed) + A_pasture * kcal_pasture ] / (365 * c)`
with `c` = kcal per person per day (realistic 1900-2500, sandbox 1200-4000; 2000 kcal/day = 3.05 GJ/yr), `f_feed` the share of crops fed to animals (Earth today high; pre-industrial 0.1-0.3, tune). Pasture is low-yield: Earth livestock use 77% of farmland for 18% of calories, so `kcal_pasture` should be about 1/10 or less of cropland per ha (derived from 77%/18% against 1/3 vs 2/3 land split; tune).
Sanity check, Earth today: 16M km2 cropland * 1.5e6 ha/M km2... = 1.6e9 ha at a world mean of maybe 4 t/ha is far above need, which matches that livestock and waste absorb much; use it to calibrate `f_feed` and loss, not as validation.

**E6. Foraging capacity (hunter-gatherer era).** `K_hg = sum_b s_b * A_hab * D_b`, `D_b = k_hg * NPP_b * m_b`, with `m_b` a biome access multiplier (forests and coasts high, desert low). Calibrate to the cited points: Mesolithic temperate 0.1 people/km2; the literature shows 1000-fold unexplained spread per unit NPP, so the realistic band is `D` 0.01-1 person/km2 for most biomes (the 0.01 and 1 ends are memory; the 0.1 point is cited). Sandbox 0.001-5.

**E7. Era mixing and tech.** `K = max(K_hg, K_ag(era))` per region, and an era index `t` in {forager, early agri, pre-industrial, mechanised} scales Y, phi_max, seed, loss and `f_feed`. Boserup: intensification is a response, so `t` advances when `P / K_current > 0.8` for N generations (a threshold we choose; the cited sources say pressure triggers intensification but give no number). Realistic effect size: pre-industrial to mechanised yield ratio 4-6x (England 1.5 to 8 t/ha, OWID).

**E8. Malthusian pop dynamics (pops loop).** Logistic with a lagged, noisy limit: `dP/dt = r * P * (1 - P/K_eff)`, `K_eff = K * (1 + trade) * (1 - shock)`. Preindustrial `r` about 0.0005-0.005 per year long-run (memory; derive from HYDE, tune); sandbox 0-0.05. Better, vital rates instead of a single `r`: `birth = b0 * g(w)`, `death = d0 / g2(w)`, where `w = food/need`, per the Malthus rows (birth up and death down with welfare), plus a preventive check (England: lower fertility than the pure model predicted).

**E9. Water.** Household demand `W_h = 20-100 L/person/day` (WHO tiers); total per capita `W = W_h + W_irrigation + W_industry`. Supply is renewable flow `Q` per region; stress index `S = Q / P` (m3/person/yr) with Falkenmark bands 1700 stressed, 1000 scarce, 500 absolute scarcity. Capacity cap from water: `K_water = Q / S_min` where `S_min` realistic 500-1700 (modern, trade-supported) or lower in low-tech eras when irrigation use is small (tune). Agriculture share of withdrawal 0.7 today (tune by era).

**E10. Energy and materials flows.** Demand `E = P * e(era, income)`, with `e` anchors: food floor about 3 GJ/yr; world mean 73 GJ/yr (2024); North America 217; US 266; diminishing welfare returns above 50-70 GJ/yr. Pre-industrial `e` is not found in our sources; set provisional 10-30 GJ/yr (memory, low) and mark tune. Resource depletion: `stock(t+1) = stock(t) - extraction(t)`, `extraction = min(capacity, P * q_mat(era))`, with reserves-to-production `RP = reserves / production`. Copper: RP about 40-45 years today (derived), but reserves are an economic category that grows with price and technology, so model `reserves` as `resources * f_econ(tech, price)` and not as a fixed stock.

## 3. How this closes the loop with pops and nations

- A region's `K` (E3-E7) is the ceiling its pop count grows toward; pops split by job from the food, water and energy that region produces.
- Surplus above `c` per pop (E5) is what pays for non-farmers: soldiers, scholars, officials. Farmer share needed rises as yield falls, so a nation's tax and army base is an output of its biomes and era, not a free slider.
- Nation capability feeds back into `K`: roads and trade raise `trade` in E8, irrigation raises `phi_max` and `Y`, security keeps `shock` low, research advances the era index `t` (Boserup push).
- Pressure `P/K` drives migration, unrest, wars for land and the push to intensify; a famine is `w < 1` for a season, with the death-rate response of E8.
- Water stress (E9) and fuel/ore stocks (E10) cap city size independently of food, so a district can be food-rich and water-poor.
- Trade and tech let `K_eff` exceed local `K`, so nations with ports and rivers outgrow equal-area inland ones; blockade or fallen government removes that term (matches section 12's "no guards, no blockade" style of effect-only-through-actors).
- Every number is a slider with a realistic band and a flagged sandbox band, so a generated world stays a laboratory and the Zombie-Game layers (district population, household ledgers) read the same fields.
- A new virus or war acts as `shock` on `K_eff` and on `P`, so the same equations show recovery time after an outbreak.

## 4. Unverified / not found

- Fetching was blocked, so no source page was read; all "search" numbers are second-hand summaries.
- Per-biome hunter-gatherer density medians (Binford, Kelly): not found. Only the 0.1/km2 Mesolithic France point and the "1000-fold unexplained variation" statement were found.
- Pre-industrial agrarian density by biome and region (people/km2): not found beyond the 10-20 Iron Age France and 2.5-7.5 early-farmer points. HYDE via OWID has the series; not opened.
- Early-agriculture and medieval yields outside England; yields for rice, maize, potato: not found.
- Per-capita energy for foragers, early farmers and pre-industrial societies (Smil): not found.
- FAO MDER numeric world average: not found.
- Reserves and production for iron ore, bauxite, nickel, coal, oil, gas, lithium, phosphate and others: not found. Per-capita metal consumption by income level: not found.
- Quantitative Boserup or Malthus parameters (elasticities, growth rate r, famine mortality): not found.
- Land fraction, ice and arable share as a function of planet radius or climate: not found; no general law, Earth reference only.
- Cropland share of land (11%) was ambiguous in the summary (of all land vs habitable).
- Values marked "memory" (grain kcal/g, seed share, preindustrial r, pasture ratio, preindustrial GJ per capita) were not confirmed this session.

## 5. Data licences

Only public-domain or CC BY data may be used (CLAUDE.md data rule). Licence statements below come from search summaries or memory unless stated; each must be re-read on the publisher's page before we ship any data.

| Dataset | Use | Licence | Source URL | Status |
| --- | --- | --- | --- | --- |
| HYDE 3.2 (population and land-use history, 10,000 BCE to 2023) | Calibrate E3-E8 density and cropland shares per era | CC BY 4.0 | https://datahub.io/climate-and-environment/hyde-history-database-of-the-global-environment (search) | OK, cite PBL/Utrecht, Klein Goldewijk et al. |
| USGS Mineral Commodity Summaries | World reserves and production per mineral | US federal government work, public domain (memory, not confirmed in search) | https://www.usgs.gov/publications/mineral-commodity-summaries-2025 | likely OK; verify |
| FAOSTAT and AQUASTAT | Yields, calories, water use | FAO open data is CC BY 4.0 (memory, not confirmed in search) | https://www.fao.org/nr/water/aquastat/water_use | likely OK; verify |
| Our World in Data charts | Convenience access to FAO, HYDE series | CC BY 4.0 for OWID-produced content; underlying third-party series keep their own licence | https://ourworldindata.org/grapher/wheat-yields | OK only when the upstream licence also qualifies |
| Broadberry et al. 2015 and Brassley 2000 (English wheat series inside OWID) | Medieval yield | licence not found | https://ourworldindata.org/grapher/wheat-yields | do not copy; cite only |
| Energy Institute Statistical Review | Per-capita energy | not found; likely restrictive | https://dieselnet.com/news/2025/06energyreview.php | do not use as data; use OWID/FAO equivalents if licensed |
| NPP biome tables (Leeds page, PMC table) | NPP bands | page licence not found; facts are bands from the literature | https://see.leeds.ac.uk/enviroweb/biosphere/1_organic_matter/organic9.php | cite the original papers, do not copy tables |
| McEvedy and Jones Atlas; Smil books | Population, energy history | copyrighted books | https://vaclavsmil.com/book/energy-in-world-history/ | read only; do not embed data |
| Natural Earth | Land/ocean shapes | public domain (memory, not confirmed) | https://www.naturalearthdata.com/about/terms-of-use/ | likely OK; verify |
