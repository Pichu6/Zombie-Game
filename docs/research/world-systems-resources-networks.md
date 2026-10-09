# World systems: where resources are, and how grids and networks are laid out

**Date:** 2026-10-06. **Status:** research note, not a decision; nothing here is Decided, Implemented or Validated. **Feeds:** [world systems](../design/world-systems.md), [infrastructure](../design/infrastructure.md), [world numbers](../design/world-numbers.md), [0035](../decisions/0035-realistic-world-numbers-and-the-pop-nation-loop.md), [0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md). **Evidence:** WebFetch was blocked by the network egress proxy (tested on pubs.usgs.gov: `EGRESS_BLOCKED`), so every number comes from WebSearch result summaries, not from reading the source page; confidence is medium at best. In section 1, "high" means a well-known figure several summaries agree on, "medium" one summary of a credible source, "low" a secondary or blog source or a vendor range; rows marked "design" are not sourced and exist to be calibrated. **IDs:** WSR.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| WSR-1 | Banded iron formations (nearly all Precambrian) hold over 60% of world iron reserves; main regions Australia, Brazil, Canada, India, Russia, South Africa, Ukraine, USA | reported | https://www.encyclopedia.com/earth-and-environment/minerals-mining-and-metallurgy/metallurgy-and-mining-terms-and-concepts-42 |
| WSR-2 | About 70% of land-based nickel resources are laterite (tropical weathered ultramafics), about 30% sulfide; production over the last decade was 60% laterite, 40% sulfide | reported | https://en.wikipedia.org/wiki/Lateritic_nickel_ore_deposits |
| WSR-3 | Komatiite-hosted nickel sulfide on Archean cratons gives about 10% of annual nickel production and about 18% of sulfide nickel reserves | reported | https://dmp.wa.gov.au/msa/komatiite-ni-489.aspx |
| WSR-4 | Fewer than 40 supergiant oil fields (at least 5 billion bbl recoverable) exist; two thirds are in the Arabian-Iranian basin | reported | https://www.pmfias.com/petroleum-and-mineral-oil-world-distribution-supergiants-oilfields-across-world/ |
| WSR-5 | In 1998, 50% of remaining US giant-field oil reserves were in 6 fields and 80% in 42 fields | reported | https://pubs.usgs.gov/of/1999/0131/report.pdf |
| WSR-6 | Copper deposit tonnage and grade are both roughly lognormal, with no significant tonnage-grade correlation for porphyry copper | reported | https://pubs.usgs.gov/publication/pp907A |
| WSR-7 | USGS 2008 porphyry copper model: 488 Phanerozoic plus 24 Precambrian deposits, median 250 Mt ore at median 0.39% Cu | reported | https://www.usgs.gov/publications/porphyry-copper-deposits-world-database-and-grade-and-tonnage-models-2008 |
| WSR-8 | Lasky relation: cumulative tonnage vs grade follows g = C1 - C2 log t (form only; constants not found) | reported | https://pubs.usgs.gov/publication/70011787 |
| WSR-9 | Tonnes of ore per tonne of metal = 100 / grade%; copper below about 0.20-0.25% Cu is judged unlikely to be mined for common uses | reported | https://pubs.usgs.gov/publication/70232648 |
| WSR-10 | Average copper ore grade fell about 25% in ten years while energy use rose 46% against production up 30% | reported | https://zaguan.unizar.es/record/60643?ln=fr |
| WSR-11 | Copper concentrators use 0.45-0.6 m3 of water per t ore (up to 3); gold mines about 0.35 m3 per t ore | reported | https://www.mining-technology.com/sponsored/how-less-water-can-help-fill-the-copper-supply-gap/ |
| WSR-12 | Blast furnace plus BOF steel takes 24.2 GJ per t steel (2.3 t CO2 per t) | reported | https://www.rite.or.jp/system/en/global-warming-ouyou/download-data/2019_BF-BOF_energy%20intensity%20by%20RITE.pdf |
| WSR-13 | Cement clinker needs 3.53 GJ thermal per t clinker; cement about 104 kWh electricity per t | reported | https://www.rite.or.jp/system/en/latestanalysis/pdf/E-Comparison_EnergyIntensity2010cement.pdf |
| WSR-14 | Primary aluminium smelting uses 13-15 MWh per t Al (global average 13,990 kWh) | reported | https://www.alcircle.com/news/powering-aluminium-part-1-inside-the-global-smelting-energy-map-2023-24-116194 |
| WSR-15 | Alumina refining takes about 10 GJ per t (range 7-21); 2-2.5 t bauxite per t alumina; 1.9-1.95 t alumina per t aluminium | reported | https://en.wikipedia.org/wiki/Bayer_process |
| WSR-16 | High-income countries use about 67 times more electricity per person than low-income ones; every country above $30,000 GDP per capita uses at least 4,000 kWh per person per year | reported | https://energyforgrowth.org/article/watch-countries-climb-the-energy-ladder/ |
| WSR-17 | Capacity factors: nuclear 74%, coal 50-70%, gas CC 40-60%, large hydro 30-50%, wind 30-60% (35% typical), solar PV 12-25% (25% typical) | reported | https://visualizingenergy.org/what-are-capacity-factors-and-why-are-they-important |
| WSR-18 | Mean plant sizes: coal 380 MW, gas CC power block 500-820 MW, nuclear about 900 MW per reactor, wind 84 MW, solar about 11 MW | reported | https://www.eia.gov/todayinenergy/detail.php?id=38312 |
| WSR-19 | World transmission and distribution losses average 8.3% of output; Singapore 2%, USA 5.9%, India 19.4%, Togo 72.5%; 54% of countries lose 10% or more | reported | https://data.worldbank.org/indicator/EG.ELC.LOSS.ZS |
| WSR-20 | Voltage hierarchy: transmission 132/220/400/765 kV, sub-transmission 33/66 kV, primary distribution 11/6.6/3.3 kV, last mile 415/230 V | reported | https://electrical-engineering-portal.com/single-line-diagram-of-the-ac-transmission-and-distribution-system |
| WSR-21 | In 2024, 74% of people had safely managed drinking water (urban 83%, rural 60%) and 58% safely managed sanitation (urban 66%, rural 49%) | reported | https://washdata.org/report/jmp-2024-annual-report |
| WSR-22 | Non-revenue water is typically 30-35% of supply, 50-60% in some areas; leaks total about 32 billion m3 per year, half in developed countries | reported | https://en.wikipedia.org/wiki/Non-revenue_water |
| WSR-23 | Household water use ranges 70-300 L per person per day (USA about 300, Europe about 130) | reported | https://link.springer.com/10.1007/s11269-016-1314-x |
| WSR-24 | Paved road per million people: 170 km (low income), 1,660 km (middle), 10,110 km (high) | reported | https://ssatp.org/sites/default/files/publication/part3.pdf |
| WSR-25 | Highest road density in Europe is Belgium at 4.67 km per km2; rail density is about 0.07 km per km2 in the EU and 0.02 in the USA | reported | https://en.wikipedia.org/wiki/Kilometre_per_square_kilometre |
| WSR-26 | Airports per million people: USA 42.7, Canada 42.4, Bolivia 81.7 (2013) | reported | https://citypopulation.de/en/world/bymap/airports/ |
| WSR-27 | Slime-mould (Physarum) networks matched the Tokyo rail network in efficiency, fault tolerance and cost | reported | https://pubmed.ncbi.nlm.nih.gov/20093467/ |
| WSR-28 | Mobile cell radius: 1-2 km in cities, 5-10 km in busy macro areas, over 10 km (up to 32 km) in sparse areas; 47% of people lacked 4G access in 2024 | reported | https://radio-waves.orange.com/en/how-does-a-mobile-network-work |
| WSR-29 | Generator rule: tag each cell with a geological setting (craton, arc, basin, laterite lowland, river and coast, aquifer) that sets its deposit odds, and store a reason string per placement | estimate | this note |
| WSR-30 | Draw deposit tonnage and grade as independent lognormals (copper median 250 Mt, 0.39% Cu) and oil fields from a Pareto tail with exponent near 1 (from WSR-5, WSR-6, WSR-7) | derived | this note |
| WSR-31 | Reserves are the deposits whose cost per tonne of metal (rising with 1/grade) is below the price, so price and technology widen the resource base (from WSR-8, WSR-9) | derived | this note |
| WSR-32 | Siting rule: aluminium smelters beside the cheapest firm power, cement at limestone, steel at coal plus iron plus a port, concentrators near the mine | estimate | this note |
| WSR-33 | Installed capacity per source = annual energy / (8760 h x capacity factor), with plants opened at the table sizes until the target is met (from WSR-17, WSR-18) | derived | this note |
| WSR-34 | Lay grids, roads and rail as a minimum spanning tree on terrain cost, then add loops where cheap relative to the detour saved; transmission meshed, distribution radial from each substation | estimate | this note |
| WSR-35 | Grid losses and non-revenue water are outcomes of maintenance spending, felt by pops as blackouts and dry taps | estimate | this note |
| WSR-36 | Data licences: USGS and Natural Earth are public domain, the WRI power plant database and some IRENA datasets are CC BY 4.0; OpenStreetMap (ODbL share-alike) and the Energy Institute review need a decision before use | reported | https://old-datasets.wri.org/dataset/globalpowerplantdatabase |
| WSR-37 | World proved oil reserves at the end of 2020: 1,732 billion barrels (BP) or 1,549 (OPEC); estimates for 2020 ranged between the two. | reported | https://www.bp.com/content/dam/bp/business-sites/en/global/corporate/pdfs/energy-economics/statistical-review/bp-stats-review-2021-oil.pdf ; https://www.sciencedirect.com/science/article/pii/S2666049022000524 |
| WSR-38 | World proven natural gas reserves at the end of 2020: 206.7 trillion m³ (OPEC). | reported | https://spglobal.com/commodity-insights/en/news-research/latest-news/crude-oil/093021-world-proven-crude-reserves-rose-02-in-2020-gas-reserves-fell-04-opec-report |
| WSR-39 | World coal reserves about 1,074 billion tonnes (Energy Institute, 2020 values). | reported | https://ourworldindata.org/grapher/coal-proved-reserves?tab=table |
| WSR-40 | World iron ore reserves 180 billion t of crude ore holding 85 billion t of iron (USGS 2022); resources over 900 billion t of ore holding over 260 billion t of iron. | reported | https://pubs.usgs.gov/periodicals/mcs2022/mcs2022-iron-ore.pdf |
| WSR-41 | World bauxite reserves about 30 billion t (USGS, older summaries; a 2025 compilation gives 23.2); resources 55-75 billion t. | reported | https://pubs.usgs.gov/periodicals/mcs2020/mcs2020-bauxite-alumina.pdf |
| WSR-42 | Coal forms in foreland basins in front of rising mountain belts (the Variscan foreland of the Ruhr and Belgium, the Appalachian basin) and in intracratonic basins and rifts (the Karoo); the share of world coal by basin type was not found. | reported | https://ncs-old.naturalsciences.be/node/1671 ; https://gsa.confex.com/gsa/2001AM/webprogram/Paper27876.html |

## 1. Numbers

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| **Geology to deposits** | | | | |
| Banded iron formations (Precambrian) share of world iron reserves | > 60%; nearly all Precambrian; main regions Australia (Pilbara, Hamersley), Brazil (Minas Gerais), Canada, India, Russia, South Africa, Ukraine, USA | % of reserves | https://www.encyclopedia.com/earth-and-environment/minerals-mining-and-metallurgy/metallurgy-and-mining-terms-and-concepts-42 | medium |
| Nickel: laterite share of land-based resources | ~70% (laterite, tropical weathered ultramafics); sulfide ~30% | % of resources | https://en.wikipedia.org/wiki/Lateritic_nickel_ore_deposits | medium |
| Nickel production split, last decade | laterite 60%, sulfide 40% | % of production | https://en.wikipedia.org/wiki/Lateritic_nickel_ore_deposits | medium |
| Komatiite-hosted Ni sulfide (Archean cratons, e.g. Yilgarn) | ~10% of annual Ni production, ~18% of sulfide Ni reserves | % | https://dmp.wa.gov.au/msa/komatiite-ni-489.aspx | medium |
| Australia nickel: laterite 69% of resources, but sulfide 82% of production | 69 / 82 | % | https://www.ga.gov.au/ausgeonews/ausgeonews200609/nickel.jsp | medium |
| Supergiant oil fields (>= 5 billion bbl recoverable) | fewer than 40 worldwide; two thirds in the Arabian-Iranian basin | count, share | https://www.pmfias.com/petroleum-and-mineral-oil-world-distribution-supergiants-oilfields-across-world/ | low (secondary site) |
| Concentration in US giant oil fields (1998) | 50% of remaining reserves in 6 fields; 80% in 42 fields | share | https://pubs.usgs.gov/of/1999/0131/report.pdf | medium |
| **Deposit statistics** | | | | |
| Copper deposit tonnage and grade | both approximately lognormal, not exactly; no significant tonnage-grade correlation for porphyry copper | shape | https://pubs.usgs.gov/publication/pp907A | medium |
| Porphyry copper global model (USGS 2008 update) | 488 Phanerozoic + 24 Precambrian deposits; median 250 Mt ore, median 0.39% Cu | Mt, % | https://www.usgs.gov/publications/porphyry-copper-deposits-world-database-and-grade-and-tonnage-models-2008 (figures from search summary of the OFR 2005-1060 family) | medium |
| Lasky relation (cumulative tonnage vs grade) | g = C1 - C2 log t: each fixed drop in grade multiplies the available tonnage by a roughly constant factor | form | https://pubs.usgs.gov/publication/70011787 | medium (form only; C1, C2 not found) |
| Skinner thesis: possible bimodal distribution for scarce metals (< 0.1% crustal abundance) | qualitative | - | https://zaguan.unizar.es/record/60643?ln=fr | low |
| Ore needed per unit metal | hyperbolic in grade (tonnes ore per tonne metal = 100 / grade%) and energy rises with it; copper below ~0.20-0.25% Cu judged unlikely to be mined for common uses | % Cu | https://pubs.usgs.gov/publication/70232648 | medium |
| Copper ore grade trend | average grade fell ~25% in ten years while energy use rose 46% against production up 30% | % | https://zaguan.unizar.es/record/60643?ln=fr | low |
| **Extraction and processing** | | | | |
| Copper concentrator water | 0.45-0.6 (conventional) up to 3 | m3 per t ore | https://www.mining-technology.com/sponsored/how-less-water-can-help-fill-the-copper-supply-gap/ | low |
| Copper water at 0.3% ore | 287.6 (reducible to 64.2) | m3 per t Cu | same page as above (search summary) | low |
| Gold mine average water withdrawal | ~0.35 | m3 per t ore | https://www.azgs.arizona.edu/ask-a-geologist/how-much-water-required-mining-gold | low |
| Blast furnace + BOF steel energy | 24.2 (2.3 tCO2 per t) | GJ per t steel | https://www.rite.or.jp/system/en/global-warming-ouyou/download-data/2019_BF-BOF_energy%20intensity%20by%20RITE.pdf | medium |
| Cement clinker thermal energy (CSI members) | 3.53 | GJ per t clinker | https://www.rite.or.jp/system/en/latestanalysis/pdf/E-Comparison_EnergyIntensity2010cement.pdf | medium |
| Cement electricity | 104 (global average) | kWh per t cement | same as above | medium |
| Primary aluminium smelting electricity | 13-15; global average 13,990 | MWh per t Al | https://www.alcircle.com/news/powering-aluminium-part-1-inside-the-global-smelting-energy-map-2023-24-116194 | medium |
| Alumina refining energy | ~10 average (range 7-21); 2021 industry total ~1,300 PJ | GJ per t alumina | https://icsoba.org/assets/files/publications/2024/Papers/AA01%20-%20Decarbonisation%20of%20the%20Bayer%20Process%20-%20A%20Technical%20Assessment%20of%20Enabling%20Technologies%20and%20Process%20Economics.pdf | medium |
| Bauxite per t alumina | 2-2.5 (range 1.7-3.3 in one source) | t/t | https://en.wikipedia.org/wiki/Bayer_process | medium |
| Alumina per t aluminium | 1.9-1.95 | t/t | https://www.impol.rs/en/aluminium/aluminium-production | low |
| **Energy systems** | | | | |
| Electricity use per person, high-income vs low-income | high-income uses ~67 times more; all countries above $30,000 GDP per capita use at least 4,000 kWh; R2 = 0.81 for the trend | ratio, kWh | https://energyforgrowth.org/article/watch-countries-climb-the-energy-ladder/ | medium |
| Electricity per person, OECD vs world (one listing) | OECD 7,995; world 3,126 | kWh per person per year | https://www.globalpetrolprices.com/electricity_consumption.php | low |
| Sub-Saharan Africa electricity production | 481 (2012) | kWh per person per year | https://en.wikipedia.org/wiki/List_of_countries_by_energy_consumption_per_capita | low |
| Capacity factor: nuclear / coal / gas CC / large hydro / wind / solar PV | 74% / 50-70% / 40-60% / 30-50% / 30-60% (35% typical) / 12-25% (25% typical) | % | https://visualizingenergy.org/what-are-capacity-factors-and-why-are-they-important | low |
| Capacity factor 2023: offshore wind, utility solar PV | 41%, ~16% | % | https://statista.com/statistics/183680 (secondary; IRENA is the original) | low |
| Mean plant size: coal | 380 (at ~50% utilisation) | MW | https://thundersaidenergy.com/downloads/power-plants-average-capacities/ | low |
| Mean plant size: gas CC power block | ~500 (2002-2014); 820 (2017) | MW | https://www.eia.gov/todayinenergy/detail.php?id=38312 | medium |
| Mean plant size: nuclear | ~900 (398 GWe / 440 reactors, end 2024) | MW per reactor | https://world-nuclear.org/our-association/publications/world-nuclear-performance-report/preface-to-world-nuclear-performance-report-2025 | medium |
| Mean plant size: wind, gas (one dataset) | wind 84, gas 92; interquartile 5-144 and 8-160 | MW | https://thundersaidenergy.com/downloads/power-plants-average-capacities/ | low |
| Mean plant size: solar | ~11 | MW | same as above | low |
| Transmission and distribution losses, world average | 8.3 | % of output | https://data.worldbank.org/indicator/EG.ELC.LOSS.ZS | medium |
| T&D losses by country | Singapore 2; China 4; USA 5.9; EU 6.4; Russia 10; Brazil 15.8; India 19.4; Togo 72.5; 54% of countries >= 10% | % | https://data.worldbank.org/indicator/EG.ELC.LOSS.ZS?start=1960 | medium |
| Load factor (average / peak) | definition; examples 0.6-0.8 for single consumers. National-grid typical value: not found | ratio | https://electricrates.org/blog/load-factor-electrical/ | low |
| Voltage hierarchy | primary transmission 132 / 220 / 400 / 765 kV; secondary 33 / 66 kV; primary distribution 11 / 6.6 / 3.3 kV; secondary 415 / 230 V | kV | https://electrical-engineering-portal.com/single-line-diagram-of-the-ac-transmission-and-distribution-system | low |
| **Water** | | | | |
| Safely managed drinking water, global 2024 | 74% (urban 83%, rural 60%); 2.1 billion lack it | % | https://washdata.org/report/jmp-2024-annual-report | medium |
| Safely managed sanitation, global 2024 | 58% (urban 66%, rural 49%) | % | https://washdata.org/report/jmp-2024-annual-report | medium |
| Sewer vs on-site sanitation (JMP 2022) | sewer 42%, on-site 47% of people | % | https://washdata.org/report/jmp-2022-annual-report | low |
| Non-revenue water, global | 30-35% typical, 50-60% in some areas; World Bank: 32 billion m3 per year leaks, half in developed countries | % | https://en.wikipedia.org/wiki/Non-revenue_water | medium |
| Household water use | 70-300 (range); USA ~300; Europe ~130; urban developing-world households 241 / 272 / 290 for low / mid / high income | L per person per day | https://link.springer.com/10.1007/s11269-016-1314-x | low |
| **Transport** | | | | |
| Paved road length per million people | 170 (low income), 1,660 (middle), 10,110 (high); good-condition paved 40 / 470 / 8,550 | km per million people | https://ssatp.org/sites/default/files/publication/part3.pdf | medium |
| Road density, highest in Europe | Belgium 4.67 | km per km2 | https://en.wikipedia.org/wiki/Kilometre_per_square_kilometre | medium |
| Rail density | EU ~0.07, USA ~0.02 | km per km2 | https://bluestocking.nfshost.com/article/172/rail-coverage-in-eu-and-us-regions/ | low |
| Airports per million people | USA 42.7, Canada 42.4, Bolivia 81.7 (2013) | per million | https://citypopulation.de/en/world/bymap/airports/ | low |
| Air trade income elasticity | 1.1 (country level) | elasticity | https://research.usq.edu.au/item/zqq36/gravity-model-of | low |
| Tokyo rail vs slime-mould network | Physarum networks matched real rail in efficiency, fault tolerance and cost (36 nodes) | qualitative | https://pubmed.ncbi.nlm.nih.gov/20093467/ | medium |
| **Telecom** | | | | |
| Cell radius, dense areas vs rural | 1-2 km micro cells in cities; 5-10 km high-population macro; > 10 km (up to 32 km) low population | km | https://radio-waves.orange.com/en/how-does-a-mobile-network-work | low |
| Population without 4G access (ITU, 2024) | 47% (mostly rural) | % | https://statista.com/statistics/1228855/mobile-network-coverage-worldwide-by-region-urban-rural | low |
| **Not found** | see section 4 | | | |

## 2. Equations and generator rules we could implement

All parameters below are **Proposals**. Bands marked "real" are taken or bracketed from the table above; "sandbox" bands are deliberately wider and must be flagged as outside realism (validator rule in [sandbox parameters](../design/sandbox-parameters.md)). Every placement stores a `reason` string built from the rule that fired.

### 2.1 Geology layer (cheap tectonic and sedimentary tags)

Assign each terrain cell a **setting** from the world's plate and age fields, then let the setting set the deposit probabilities. Rule table (a generator rule, not a sourced law; the geological associations are textbook-level and partially supported by the iron and nickel rows above):

| Setting (cell tag) | Deposits it can hold | Reason string |
| --- | --- | --- |
| Ancient shield / craton (Archean-Proterozoic) | iron (banded formation), nickel sulfide (komatiite), gold, chromium | "iron on a Precambrian shield" |
| Orogen and volcanic arc (subduction margin) | porphyry copper, gold, molybdenum, silver, zinc-lead | "copper in a subduction arc" |
| Sedimentary basin (subsiding, thick) | coal (swamp margins), oil and gas (source + seal), salt (evaporite in arid basin), phosphate (shelf upwelling margin) | "oil in a basin with source and seal" |
| Tropical lowland over ultramafic rock | nickel laterite, bauxite | "laterite where warm and wet over ultramafic rock" |
| River and coast | placer gold, tin, heavy sands, sand and gravel | "placer downstream of an eroding orogen" |
| Permeable sediment with recharge | aquifer (water is a resource of the cell) | "aquifer under rain-fed gravel" |

Only the bold associations carried by the rows above (iron on shields, nickel laterite in warm wet rock, komatiite Ni on cratons, Arabian-style basin concentration of oil) are sourced; the rest are marked **Proposal, textbook, not sourced in this note**.

### 2.2 Deposit count and size per setting

1. **Tonnage**: draw `ln T ~ Normal(mu_T, sigma_T)` per deposit type; copper anchor: median 250 Mt, so `mu_T = ln(250e6)`. `sigma_T` is **not found**; real band proposal 1.0-1.8 (design), sandbox 0.5-2.5.
2. **Grade**: independent draw `ln g ~ Normal(mu_g, sigma_g)` for types where the source says no tonnage-grade correlation (porphyry copper: median 0.39% Cu; `sigma_g` not found, design 0.3-0.6).
3. **Concentration for fluids**: for oil and gas use a Pareto tail, `N(>S) = N0 (S/S0)^(-a)`, with `a` near 1 (design, from the 6-fields-50% and 42-fields-80% rows: giant fields dominate). Place few supergiants in one basin (two thirds in one basin is a real world anchor, so allow a single basin to take most of a world's supergiants).
4. **Count** per setting: `N = round(area_setting * density_type * U(0.5, 1.5))`; `density_type` is **not found** and must be calibrated against world totals chosen by the user.
5. **Lasky grade ladder** for reserves vs price: lower the cutoff grade `g_c` and the tonnage rises by a constant factor per step of `log` grade (`g = C1 - C2 log t`). Implement as `T(g_c) = T_ref * exp((g_ref - g_c) / C2')` with `C2'` per commodity (design).
6. **Reserves = f(price, tech)**: `reserve = sum of deposits with cost_per_tonne_metal(g) <= price`, where `cost` rises with `1/g` (ore per tonne of metal = 100/g%). Technology lowers energy and water per tonne (design: 1-3% per year; the only sourced trend is the rising energy at falling grade).
7. **Hard floor**: copper cutoff 0.20-0.25% Cu for the real band (sourced); sandbox 0.05-0.5%.

### 2.3 Extraction and processing (flows a plant needs)

For each processing node, input per tonne of product is a recipe:

`E_total = tonnes_ore * e_mine + tonnes_conc * e_mill + tonnes_metal * e_smelt`

where `tonnes_ore = tonnes_metal * 100 / grade%`. Use the table values as recipe constants:

- Steel BF-BOF 24.2 GJ per t. Aluminium 13-15 MWh per t (electricity), alumina 10 GJ per t, 2-2.5 t bauxite per t alumina, 1.9-1.95 t alumina per t Al. Cement 3.53 GJ per t clinker and ~104 kWh per t cement. Water per tonne ore 0.35 (gold) to 0.45-0.6 (copper concentrator), up to 3.
- **Siting rule**: electricity-hungry smelting (aluminium) goes to the cheapest firm power (hydro, or fuel plus a port); cement to limestone; steel to coal plus iron and a port; concentrators near the mine. Reason strings: "aluminium smelter beside the cheapest firm power".
- Yields and losses (mill recovery, smelter recovery) are **not found**; use design bands 80-95% real, 50-99% sandbox.

### 2.4 Energy: demand, mix, sites, grid

1. **Demand**: `E_cap = a * (GDPpc)^b` with the two anchors "high income uses 67 times low income" and "at least 4,000 kWh above $30,000 per person". Fit `a, b` by regression on the Our World in Data series (not done here). Add `x (1 + L)` for losses, `L` in 2-20% real (world mean 8.3%; Singapore 2, India 19.4), sandbox 1-70%.
2. **Peak**: `P_peak = E_year / (8760 * LF)` with load factor `LF`; national value **not found**; design band 0.55-0.75 real.
3. **Capacity to install**: `C_i = E_i / (8760 * CF_i)`, with CF from the table (solar 12-25%, wind 30-60%, hydro 30-50%, coal 50-70%, gas CC 40-60%, nuclear ~74%). Reserve margin on top of peak: design.
4. **Mix** (rule by what exists): hydro first where `head * flow` is large; coal and gas where basin fuel is inside the region or a pipeline or port is near; wind by wind-resource cell; solar by latitude and cloud cover; nuclear only if the nation has capital and tech (era read from capabilities, per [world numbers](../design/world-numbers.md) §1).
5. **Plant count and size**: draw plant size from the mean and spread in the table (coal 380 MW, gas CC 500-820 MW, nuclear ~900 MW per reactor, wind 5-160 MW IQR, solar ~11 MW), and open plants until `sum C_i` meets the target.
6. **Grid layout**: plants and load centres are nodes; connect them with an MST (Kruskal on terrain-weighted distance), then add loops: add an edge if `cost(edge) < k * saved_path_cost` (a Physarum-style fault tolerance rule, supported qualitatively by the Tokyo-rail study). Edge voltage from the transferred power: 765 / 400 / 220 / 132 kV for trunks, 33-66 kV regional, 11 kV distribution, 415/230 V last mile. Nodes per population (number of substations per 100,000 people): **not found**.
7. **Radial vs mesh**: transmission as mesh (MST + loops), distribution as radial trees from each substation (design; consistent with [infrastructure](../design/infrastructure.md) topology).

### 2.5 Water

1. **Demand**: `Q = pop * q_pc * (1 + NRW)`; `q_pc` 70-300 L per day real band (Europe ~130, USA ~300; low-income urban households far lower in practice, value not found), sandbox 20-600.
2. **Non-revenue water** `NRW`: 0.30-0.35 typical, 0.10-0.60 real band (World Bank), sandbox 0.05-0.80; tie to infrastructure age and budget.
3. **Coverage**: urban 83% and rural 60% for safely managed water (2024 global means), by income band; sanitation urban 66% and rural 49%; sewer share about 42% of people globally (2022).
4. **Layout**: intake (river, aquifer, reservoir) to treatment plant to elevated storage to a tree of mains in each city, with loops only in the dense core. Reservoir and pipe sizing numbers (storage days, peaking factor, velocity): **not found**.

### 2.6 Transport, telecom, pipelines

1. **Road length**: per capita by income from the SSATP row: paved km per million people 170 / 1,660 / 10,110 (low / middle / high); cap by area so density stays below the Belgium maximum 4.67 km per km2 (real), sandbox up to 8.
2. **Rail**: density 0.02-0.07 km per km2 (USA to EU), only where freight or population density justify it (mining line from deposit to port is the first rail edge).
3. **Layout**: connect settlements by MST on terrain cost, add loops until the detour ratio falls below a target (design 1.3-1.5), then hub-and-spoke for rail and air around the largest city. Cities rank by Zipf size; the top ones get ports (if coastal) and airports.
4. **Ports and airports**: air traffic scales with GDP and population (gravity model, income elasticity ~1.1); airports per million people 42 (USA, Canada) is an upper realistic anchor, not a global mean (design band 3-45 per million).
5. **Telecom**: tower spacing from density: radius 1-2 km (urban), 5-10 km (suburban and dense rural), 10-32 km (sparse), so `n_towers = area / (pi r^2 * overlap)`; coverage targets 4G about 53% of world population, higher in cities.
6. **Pipelines**: not sourced. Rule (design): a pipeline connects a gas or oil field to the nearest refinery, port or load centre along minimum-cost route; capacity is the field's plateau rate; length cost is linear.

## 3. How this closes the loop with pops and nations (max 8 bullets)

- A nation's mineral income is the sum of its deposits' output; the labour is drawn from its pops (miners, smelter workers), so power costs output ([world numbers](../design/world-numbers.md) rule 1).
- Pop count and income set electricity demand (`E_cap`), which sets how many plants and how long a grid the nation must build and maintain.
- Grid and water losses are a measured outcome of spending: more maintenance lowers `L` and `NRW`; neglect raises them, which pops feel as blackouts and dry taps.
- Water and sanitation coverage feed pop health and mortality ([demography note](world-numbers-demography.md)).
- Roads, rail and ports cut transport cost, which raises the share of land that can feed and supply cities (carrying capacity, [capacity note](world-numbers-capacity.md)).
- Raising the price or lowering the cutoff grade turns resources into reserves (Lasky), so wealth and technology widen a nation's resource base over time.
- Border placement can cut a grid, a river or a pipeline; a deposit or smelter on a border is a reason for conflict (factions, [0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md)).
- Every placed thing keeps its `reason` string, so a player can ask "why is the smelter here" and get the rule that fired.

## 4. Unverified / not found

- WebFetch blocked; no primary page was read. All figures are from search summaries.
- Not found: sigma values of the lognormal tonnage and grade fits per deposit type; Lasky constants; deposit density per km2 by setting; price elasticity of reserves; mill and smelter recovery yields; extraction cost vs grade curves; national-grid load factors; substations per capita; low-income per-capita water use; reservoir and pipe sizing rules; leakage in L per connection per day; pipeline logic and cost; airports and ports vs trade as equations; coverage vs density curves; road density by income per km2 (only per capita found) and by terrain; electricity generation mix by income band with numbers.
- The capacity factor row is a range from a secondary site; IRENA and IEA original values were not read.
- The Lasky and Skinner readings are summary-level; do not implement the Skinner bimodal tail without the paper.
- Several sources are secondary (Wikipedia, vendor blogs, citypopulation, statista); replace before any number becomes Decided.
- Specific figures the search did not confirm (for instance a "346 million m3 per day" leakage number) were not used.

## 5. Data licences

We only use public-domain or CC BY data. Findings from search summaries; check each licence page before ingest.

| Data | Licence found | Use here | Source URL |
| --- | --- | --- | --- |
| USGS publications and data | Public domain except restricted items | OK (grade-tonnage models, Mineral Commodity Summaries) | https://www.usgs.gov/faqs/what-usgs-policy-release-scientific-data-are-any-usgs-products-restricted |
| Natural Earth | Public domain, no attribution needed | OK (coastlines, borders, rivers) | https://atlas.co/data-sources/natural-earth/ |
| Global Power Plant Database (WRI) | CC BY 4.0; v1.3.0; no longer maintained (as of early 2022); ~35,000 plants in 167 countries | OK with attribution | https://old-datasets.wri.org/dataset/globalpowerplantdatabase |
| IRENA | A dataset (world electricity generation by country) is CC BY 4.0; publications need attribution and a copyright notice; per-dataset check needed | OK per dataset, with attribution | https://energydata.info/dataset/world-electricity-generation-by-country/resource/73da71f6-5307-43ad-a9c8-23722bd7e200 |
| World Bank WDI (losses, roads, water) | Not read in this note; World Bank open data is usually CC BY 4.0 (not confirmed here) | Check before ingest | https://data.worldbank.org/indicator/EG.ELC.LOSS.ZS |
| JMP (WHO/UNICEF) water and sanitation | Not read; licence not found | Check before ingest | https://washdata.org |
| Our World in Data | Not read; charts are usually CC BY (not confirmed here) | Check before ingest | https://ourworldindata.org |
| Energy Institute Statistical Review | Free to access; formal licence designation not found | **Flag: do not ingest until licence confirmed** | https://energyinst.org/exploring-energy/statistical-review |
| IEA | Usually restricted (stated in the task); not checked | **Flag: use only published numbers cited with source, no dataset ingest** | not checked |
| OpenStreetMap | ODbL, share-alike, attribution required | **Flag: share-alike may bind the derived database; do not use without a decision** | https://openstreetmap.us/our-work/public-domain-map/license |
