# World systems: how a real economy works in numbers

**Date:** 2026-10-06. **Status:** research note, not a decision. **Feeds:** [world systems](../design/world-systems.md), [world numbers](../design/world-numbers.md), [sandbox parameters](../design/sandbox-parameters.md), [0035](../decisions/0035-realistic-world-numbers-and-the-pop-nation-loop.md), [0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md), [spec 32](../tier0/specs/32-stocks-goods-and-money.md). **Evidence:** WebFetch was blocked by the network egress proxy (ourworldindata.org, rug.nl, bls.gov all returned EGRESS_BLOCKED), so every number comes from WebSearch result summaries, not from reading the source page; no source page was opened. Confidence is therefore medium at best in the section 1 table; "low" there marks numbers from trade or vendor sites, or numbers a summary gave without its table. Where a source URL is a data portal, the number is the one the summary quoted. Licences were also checked only by search summaries. **IDs:** WEC.

Question: how does a real economy work in numbers, at a level we can simulate per province and nation?

## Findings

All numbers here come from search summaries (see Evidence); the section 1 table keeps the note's finer medium/low marks.

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| WEC-1 | Agriculture is on average 27.25 % of GDP in 23 low-income countries (2023), range 16.3 to 35.8 %. | reported | https://www.theglobaleconomy.com/rankings/share_of_agriculture/WB-low/ |
| WEC-2 | Employment in low-income countries (2023): agriculture 59 %, industry 10 %, services 31 %; in rich countries services about 75 % and agriculture about 3 %. | reported | https://ourworldindata.org/data-insights/in-low-income-countries-most-people-work-in-farming-in-richer-countries-they-work-in-services |
| WEC-3 | OECD manufacturing value added is about 34 % of gross output (about 66 % intermediate, 2022); US whole-economy intermediate consumption is about 45 % of gross output. | reported | https://oecdstatistics.blog/2024/03/14/insights-on-engagement-in-global-value-chains-in-2023/ |
| WEC-4 | Steel energy by route (2022): BF-BOF 23.98, scrap-EAF 10.20, DRI-EAF 22.37 GJ per t crude steel; global average 21.27 GJ/t (2023, vendor site); route shares 72 / 21 / 7 % (low). | reported | https://worldsteel.org/media/press-releases/2022/sustainability-indicators-2022/ |
| WEC-5 | Cement clinker needs 3.1 to 4.0 GJ thermal per t (CSI mean 3.53); cement electricity 90 to 120 kWh/t (global 2012: 104); clinker-to-cement ratio world 0.76, US 0.88 (low). | reported | https://www.rite.or.jp/system/en/latestanalysis/pdf/E-Comparison_EnergyIntensity2010cement.pdf |
| WEC-6 | Oil refineries use 3 to 4 % (simple) or 7 to 8 % (complex) of crude energy content; flour milling uses 60 to 100 kWh per t wheat; textiles 8 to 20 kWh per kg fabric (low). | reported | https://digitalrefining.com/article/1001127/optimisation-of-energy-consumption |
| WEC-7 | Embodied energy (MJ/kg): cement 5.6, in-situ concrete 1.9, clay brick 2.5, glass 12.7, aluminium 170, galvanised steel 38. | reported | https://en.wikipedia.org/wiki/Embodied_energy |
| WEC-8 | World gross fixed capital formation was 25.9 % of GDP (2023); capital-output ratio in developed economies is stable at about 3 to 4; ICOR about 3 (low). | reported | https://data.worldbank.org/indicator/NE.GDI.FTOT.ZS?locations=1W |
| WEC-9 | Depreciation (BEA/BLS): dwellings 1.52 %, other construction 2.28 %, machinery and equipment 11.0 %, transport equipment 11.0 % per year; many buildings 2 to 3 %; PWT studies use a blanket 6 % (low). | reported | https://apps.bea.gov/national/pdf/BEA_depreciation_rates.pdf |
| WEC-10 | One study gives a physical-capital share of 0.46 to 0.56 (109 countries, 1960-2014), which conflicts with the usual one-third (low). | reported | https://link.springer.com/10.1007/s00181-019-01647-y |
| WEC-11 | Household budget shares, EU 2024: food 13.2 %, housing/water/energy 23.6 %; food ranges 9.4 % (Luxembourg) to 27.6 % (Romania). | reported | https://ec.europa.eu/eurostat/statistics-explained/index.php?title=Household_consumption_by_purpose |
| WEC-12 | Food is above 40 % of consumer spending in low-income African and South Asian countries (above 50 % in Nigeria, Kenya, Bangladesh, 2022) and below 10 % in the US and UK. | reported | https://www.ers.usda.gov/data-products/charts-of-note/chart-detail?chartId=107494 |
| WEC-13 | Trade openness (X+M over GDP): world about 90 % (low); USA 25.0, China 37.9, Brazil 35.6 % (2024); Finland 83.4, Luxembourg 412, Hong Kong 385, Singapore 332 %. | reported | https://www.theglobaleconomy.com/USA/trade_openness/ |
| WEC-14 | Services are 24.7 % of world trade (goods 75.3 %, 2023). | reported | https://www.kmu.admin.ch/en/wto-sees-slowdown-in-global-trade |
| WEC-15 | Gravity model: trade has a GDP elasticity of about +1 and a distance elasticity of about -0.7 to -1. | reported | https://sites.bc.edu/james-anderson/wp-content/uploads/sites/114/2020/02/GravitySlides.pdf |
| WEC-16 | Domestic credit to the private sector was 147.99 % of world GDP (2024), on a series whose definition varies (low). | reported | https://data.worldbank.org/indicator/FS.AST.PRVT.GD.ZS |
| WEC-17 | US inflation peaked at 13.5 % (1980), double digits 1979-1981; fed funds near 20 % and 30-year mortgages 13.74 % in 1980 (low). | reported | https://marketplace.org/2014/09/18/once-upon-time-inflation-really-was-high |
| WEC-18 | Residential floor area per person: Finland 40.5 m² (2017), Lithuania 35.5 m² (2018), China urban 38.6 m² (2020) and rural 48.9 m² (2019). | reported | https://stat.fi/til/asas/2017/01/asas_2017_01_2018-10-10_tau_002_en.html |
| WEC-19 | Residential buildings last 60 to 100 years typically; China urban average about 34 years (low). | reported | https://www.repository.cam.ac.uk/handle/1810/295360 |
| WEC-20 | A US single-family house took 9.1 months in 2024 (1.4 permit + 7.6 build), 6 to 7 months since the 1970s; labour is about 35 % of residential construction cost (low). | reported | https://eyeonhousing.org/2024/08/single-family-build-time-continues-to-trend-upward-for-2023/ |
| WEC-21 | World Bank 2000 infrastructure: power capacity low/middle/high income 116 / 406 / 2,031 (labelled kW per capita, likely W, a unit slip) and roads 5.8 / 1.10 / 10.54 km per 1000 people (middle below low looks like an artefact); use as relative ratios only (low). | reported | https://documents1.worldbank.org/curated/en/502041468739488372/pdf/multi0page.pdf |
| WEC-22 | Proposal E1: province production `Y = A·K^a·L^(1-a)·min(1, E_avail/E_need)`, capital share realistic 0.30 to 0.45 (design), sandbox 0.1 to 0.7; A derived from the province's capabilities. | estimate | this note |
| WEC-23 | Proposal E2-E3: sector shares as functions of log income per worker anchored on WEC-1 and WEC-2; Leontief closure over 6 sectors (agriculture, mining and energy, manufacturing, construction, services, government) with column sums from WEC-3. | estimate | this note |
| WEC-24 | Proposal E4: capital by perpetual inventory per asset class, `K[t+1] = (1-d)K[t] + sY`, d from WEC-9, s realistic 0.15 to 0.35, sandbox 0 to 0.6; build time = max(labour hours / crew hours per month, material wait). | estimate | this note |
| WEC-25 | Steady-state K/Y = s / (g + d) = 3.7 with s = 0.26, g = 0.02, d = 0.05, inside the observed 3 to 4 (from WEC-8, WEC-9). | derived | this note |
| WEC-26 | Proposal E5: food budget share falls with income from about 0.45-0.55 (poor) to about 0.10 (rich), eta 0.4 to 0.6 (design); housing roughly flat at 0.24 to 0.27 (from WEC-11, WEC-12). | estimate | this note |
| WEC-27 | Proposal E6: per good per province, a damped price step `p·(1 + k·clamp((D-S)/S, -c, c))` with stock; shelves empty when stock + supply < demand; k 0.05 to 0.3 per month for staples is design, not sourced. | estimate | this note |
| WEC-28 | Proposal E7: gravity trade rescaled to an openness target falling with size (anchors WEC-13), distance exponent 0.7 to 1.0, sandbox 0.3 to 2; goods tradable, services mostly not (design). | estimate | this note |
| WEC-29 | Proposal E8-E9: recipes carry energy per tonne from WEC-4 to WEC-6, with labour derived from the labour share rather than invented hours; inflation `pi = pi_target + g_m - g_y`, credit ceiling c·Y with c 0.2 to 1.5 (design). | estimate | this note |
| WEC-30 | Loop with pops: pops are L (drafted or official pops cost output), pop wealth sets demand and sector mix, infrastructure is capital that decays, shortages feed hunger, unrest, migration and legitimacy loss, and nation decisions change s, k and trade parameters. | estimate | this note |
| WEC-31 | Usable data licences (by search summaries): World Bank WDI, Penn World Table, WIOD, EU KLEMS are CC BY 4.0; OECD is CC BY 4.0 from 1 July 2024; Our World in Data is CC BY for its own work; aggregators (theglobaleconomy, civils.ai, oxmaint) must not be shipped. | reported | https://datacatalog.worldbank.org/public-licenses |
| WEC-32 | Not found: IO coefficients for sectors other than manufacturing, labour-hours per tonne for smelting, refining and milling, construction labour-hours per m² (in BLS Bulletins 1755, 1892, 2102, not read), tradable share by sector, and price-adjustment speed k. | reported | this note |

## 1. Numbers

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Agriculture share of GDP, low-income countries (23 countries, 2023) | mean 27.25 (range 16.3 to 35.8) | % of GDP | https://www.theglobaleconomy.com/rankings/share_of_agriculture/WB-low/ | medium |
| Employment by sector, low-income countries, 2023 | agriculture 59, industry 10, services 31 | % of workforce | https://ourworldindata.org/data-insights/in-low-income-countries-most-people-work-in-farming-in-richer-countries-they-work-in-services | medium |
| Employment in rich countries | services about 75, agriculture about 3 | % of workforce | https://ourworldindata.org/data-insights/in-low-income-countries-most-people-work-in-farming-in-richer-countries-they-work-in-services | medium |
| Services share of GDP, mean of 156 countries, 2025 | 55.7 (max 81.5 Luxembourg) | % of GDP | https://www.theglobaleconomy.com/rankings/share_of_services/ | low |
| Value-added ratio, OECD manufacturing, 2022 | about 34 of gross output (so about 66 intermediate) | % of gross output | https://oecdstatistics.blog/2024/03/14/insights-on-engagement-in-global-value-chains-in-2023/ | medium |
| Intermediate consumption, US whole economy | about 45 of gross output | % of gross output | https://en.wikipedia.org/wiki/Intermediate_consumption | low |
| Services share of US intermediate inputs | about 30 (1980s) to more than 40 (today) | % of intermediate use | https://en.wikipedia.org/wiki/Intermediate_consumption | low |
| IO table coverage (OECD ICIO 2025) | 80 economies, 50 activities, 1995-2022 | tables | https://www.oecd.org/content/dam/oecd/en/topics/policy-sub-issues/trade-in-value-added/tiva-indicators-guide-2023.pdf | medium |
| Steel, global average energy, 2023 | 21.27 | GJ per t crude steel | https://oxmaint.com/industries/steel-plant/energy-benchmarking-for-steel-industry | low (vendor site quoting worldsteel) |
| Steel by route, 2022 | BF-BOF 23.98; scrap-EAF 10.20; DRI-EAF 22.37 | GJ per t crude steel | https://worldsteel.org/media/press-releases/2022/sustainability-indicators-2022/ (via summary) | medium |
| Steel route shares | BF-BOF 72, scrap-EAF 21, DRI-EAF 7 | % of output | https://oxmaint.com/industries/steel-plant/energy-benchmarking-for-steel-industry | low |
| Cement clinker, thermal energy | 3.1 to 4.0; CSI mean 3.53 | GJ per t clinker | https://oxmaint.com/industries/cement-plant/cement-plant-energy-kpis-kwh-tonne-gj-tonne-cmms ; https://www.rite.or.jp/system/en/latestanalysis/pdf/E-Comparison_EnergyIntensity2010cement.pdf | medium |
| Cement electricity | 90 to 120 (global 2012: 104) | kWh per t cement | same two URLs | medium |
| Clinker-to-cement ratio | world 0.76; US 0.88 (2022) | t clinker per t cement | https://gccassociation.org/list-of-data-available-on-the-gnr/ | low |
| Oil refinery own energy use | simple 3 to 4; complex 7 to 8 | % of crude energy content | https://digitalrefining.com/article/1001127/optimisation-of-energy-consumption | medium |
| Flour milling electricity | 60 to 100 (FAO range 65 to 100) | kWh per t wheat | https://www.world-grain.com/articles/10186-electrical-energy-savings-in-flour-milling | medium |
| Textile specific energy (BREF) | 8 to 20; ring spinning 4.2 to 6.8 | kWh per kg fabric / yarn | https://ifactoryapp.com/industries/textile-manufacturing/energy-benchmarking-kwh-per-kg-spinning-weaving-dyeing | low |
| Embodied energy, cement | 5.6 | MJ per kg | https://en.wikipedia.org/wiki/Embodied_energy (University of Bath inventory) | medium |
| Embodied energy, in-situ concrete / clay brick / glass / aluminium / galvanised steel | 1.9 / 2.5 / 12.7 / 170 / 38 | MJ per kg | https://en.wikipedia.org/wiki/Embodied_energy | medium |
| Gross fixed capital formation, world, 2023 | 25.9 | % of GDP | https://data.worldbank.org/indicator/NE.GDI.FTOT.ZS?locations=1W | medium |
| Capital-output ratio, developed economies | stable at about 3 to 4 over decades | K/Y | https://growthecon.com/StudyGuide/facts/capital.html | medium |
| Incremental capital-output ratio (ICOR), most countries | about 3 | dK/dY | https://en.wikipedia.org/wiki/Incremental_capital-output_ratio | low |
| Depreciation, BEA/BLS style | dwellings 1.52; other construction 2.28; machinery and equipment 11.0; transport equipment 11.0 | % per year | https://apps.bea.gov/national/pdf/BEA_depreciation_rates.pdf ; https://www.bls.gov/opub/mlr/2022/article/alternative-capital-asset-depreciation-rates-for-us-capital-and-total-factor-productivity-measures.htm | medium |
| Depreciation, many buildings (BEA, BLS) | 2 to 3 | % per year | same | medium |
| Depreciation, blanket value in PWT-based studies | 6 | % per year | https://pages.stern.nyu.edu/~dbackus/BCH/inputs/pwt_ky_ratios.pdf | low |
| Capital share of output, Cobb-Douglas, 109 countries 1960-2014 | 0.46 to 0.56 (physical capital) | share | https://link.springer.com/10.1007/s00181-019-01647-y | low |
| Piketty identity | capital share = return on capital x capital/income ratio | identity | https://en.wikipedia.org/wiki/Labor_share | medium |
| Food share of household spending, EU, 2024 | 13.2 | % of consumption | https://ec.europa.eu/eurostat/statistics-explained/index.php?title=Household_consumption_by_purpose | medium |
| Housing, water, energy share, EU, 2024 | 23.6 | % of consumption | same | medium |
| Food share spread inside EU | 9.4 (Luxembourg) to 27.6 (Romania) | % of consumption | same | medium |
| Food share, EU lowest vs highest income quintile | more than 22 vs 13 | % of budget | https://en.wikipedia.com/wiki/Engel_curve | low |
| Food share, low-income African and South Asian countries, 2022 | above 40; Nigeria, Kenya, Bangladesh above 50 | % of consumer spending | https://www.ers.usda.gov/data-products/charts-of-note/chart-detail?chartId=107494 | medium |
| Food share, US and UK | below 10 | % of consumer spending | same | medium |
| US housing share of budget, 1901 vs now | 25 vs 27 | % of budget | https://en.wikipedia.com/wiki/Engel_curve | low |
| Trade openness (X+M over GDP), world average | about 90 | % of GDP | https://lumistats.com/rankings/trade/ | low |
| Trade openness, large economies, 2024 | USA 25.0; China 37.9; Brazil 35.6 | % of GDP | https://www.theglobaleconomy.com/USA/trade_openness/ ; /China/ ; /Brazil/ | medium |
| Trade openness, small economies | Finland 83.4 (2025); Luxembourg 412; Hong Kong 385; Singapore 332 | % of GDP | https://www.theglobaleconomy.com/Finland/trade_openness/ ; https://lumistats.com/rankings/trade/ | medium |
| Services share of world trade, 2023 | 24.7 (goods 75.3) | % of trade | https://www.kmu.admin.ch/en/wto-sees-slowdown-in-global-trade | medium |
| Gravity model: GDP elasticity / distance elasticity | about +1 / about -0.7 to -1 | exponent | https://metricgate.com/docs/gravity-model-trade ; https://sites.bc.edu/james-anderson/wp-content/uploads/sites/114/2020/02/GravitySlides.pdf | medium |
| Domestic credit to private sector, world, 2024 | 147.99 | % of GDP | https://data.worldbank.org/indicator/FS.AST.PRVT.GD.ZS | low (series definition varies) |
| US inflation peak | 13.5 (1980); double digits 1979-1981 | % per year | https://marketplace.org/2014/09/18/once-upon-time-inflation-really-was-high | medium |
| US rates 1980 | fed funds near 20; 30-year mortgage average 13.74 | % | https://joingerald.com/learn/money-basics/interest-rates-1980 | low |
| Residential floor area per person | Finland 40.5 (2017); Lithuania 35.5 (2018); China urban 38.6 (2020), rural 48.9 (2019) | m2 per person | https://stat.fi/til/asas/2017/01/asas_2017_01_2018-10-10_tau_002_en.html ; https://osp.stat.gov.lt/en/lietuvos-statistikos-metrastis/lsm-2019/gyventojai-ir-socialine-statistika/bustas ; https://www.ceicdata.com/en/china/residential-area-per-capita | medium |
| Residential building lifetime | 60 to 75 (modern), 70 to 100 (typical); China urban average about 34 | years | https://www.swisslife.com/en/home/hub/what-is-the-lifespan-of-a-house.html ; https://www.repository.cam.ac.uk/handle/1810/295360 | low |
| Single-family build time, US 2024 | 9.1 total (1.4 permit + 7.6 build); 6 to 7 since 1970s | months | https://eyeonhousing.org/2024/08/single-family-build-time-continues-to-trend-upward-for-2023/ | medium |
| Labour share of residential construction cost | about 35 | % of cost | https://coconstruct.com/blog/the-latest-residential-construction-employment-and-labor-trends | low |
| Construction cost, new build, China 2026-indexed | Beijing 2,224; Shanghai 2,118; Chengdu 1,887 | US$ per m2 | https://civils.ai/construction-cost-data/china | low |
| Electric capacity per capita, 2000 | low-income 116; middle 406; high 2,031 | W per capita (source says kW; likely a unit slip, see section 3) | https://documents1.worldbank.org/curated/en/502041468739488372/pdf/multi0page.pdf | low |
| Roads per capita, 2000 | low 5.8; middle 1.10; high 10.54 | km per 1000 people | same | low (middle below low looks odd) |
| Infrastructure mix | low income roads about 50% of stock; middle electricity about 50%; high roads and electricity each 40 to 45% | % of stock | same | low |

## 2. Equations we could implement

All parameters are **Proposal**. "Realistic" band comes from section 1; "sandbox" band is wider and flagged by the validator ([sandbox parameters](../design/sandbox-parameters.md)). Parameters with no source are marked *design*.

**E1. Province production (Cobb-Douglas with energy and a materials gate).**
`Y = A * K^a * L^(1-a) * min(1, E_avail/E_need)`
- a (capital share): realistic 0.30 to 0.45 (source rows above, 0.46 to 0.56 for physical capital in one study; the lower band is the usual textbook range and is *design* here); sandbox 0.1 to 0.7.
- A (productivity): derived from capabilities the province has (era is a readout, world numbers section 1 rule 3). Calibrate so a province at the K/Y band below reproduces its GDP per worker.

**E2. Sector structure by income (Engel and Clark shifts).** Target shares as a function of log income per worker `y`: `s_ag(y)` falls, `s_ind(y)` hump, `s_srv(y)` rises. Anchors: low income ag employment about 59%, GDP about 27%; rich services about 75% of jobs. Interpolate on log y (linear in log y between the two anchors is *design*). Sandbox: any shares summing to 1.

**E3. Input-output closure (Leontief).** `x = (I - A_io)^-1 * f`, where x is gross output by sector, f is final demand and `A_io[i][j]` is input from i per unit output of j. Use 6 sectors: agriculture, mining and energy, manufacturing, construction, services, government. Column sum of `A_io` = intermediate share of gross output: manufacturing about 0.66, whole economy about 0.45 (section 1); agriculture, mining and services columns are **not found** from a fetched table and must be taken from the WIOD or OECD ICIO tables (CC BY) in a later step.

**E4. Capital stock (perpetual inventory).** `K[t+1] = (1 - d) * K[t] + I[t]`, per asset class.
- d: dwellings 0.015, other construction 0.023, machinery 0.11, transport 0.11 (BEA). Realistic band: buildings 0.015 to 0.03, machinery 0.08 to 0.15 (upper end *design*). Sandbox: 0.005 to 0.30.
- Investment `I = s * Y`, s realistic 0.15 to 0.35 (world 0.259), sandbox 0 to 0.6.
- Steady state: `K/Y = s / (g + d)`. With s = 0.26, g = 0.02, mixed d = 0.05 this gives 3.7, inside the observed 3 to 4.
- Build queue: each project needs materials and labour-hours (E8); the build time is `max(labour_hours / crew_hours_per_month, material wait)`.

**E5. Household demand (budget shares by income).** `w_food(y) = w_min + (w_max - w_min) * (y / y_ref)^(-eta)`, with w_food about 0.10 at rich anchor (EU 0.13, US and UK below 0.10), about 0.45 to 0.55 at poor anchor (ERS 2022), eta about 0.4 to 0.6 *design*, fitted to those anchors. Housing share is roughly flat, 0.24 to 0.27 (EU 2024 0.236, US 0.27). Remainder goes to goods and services by E2.

**E6. Market clearing price (damped tatonnement with stock).** Per good per province:
`p[t+1] = p[t] * (1 + k * clamp((D - S) / max(S, eps), -c, c))`
Stock: `Stock[t+1] = Stock[t] + S - min(D_effective, Stock + S)`. A shop is empty when `Stock + S < D` at the price in force. Causes of empty shelves: price below clearing (cap or slow k), supply cut (energy, transport, lost plant), or demand spike. k realistic 0.05 to 0.3 per month for staples (*design*), sandbox 0.01 to 1. The search found only a textbook statement that a price ceiling locks in a shortage (https://legalclarity.org/what-is-excess-demand-and-how-does-it-create-shortages/); the equation itself is *design*, not sourced.

**E7. Trade (gravity with an availability limit).** `T_ij = G * (Y_i * Y_j) / D_ij^delta`, delta realistic 0.7 to 1.0, GDP exponent 1; then rescale each province so exports plus imports over GDP matches an openness target `phi = a - b * log(size)`. Anchors: world about 0.9; USA 0.25, China 0.38, small economies 0.8 and above. Sandbox: delta 0.3 to 2. Shares: services are about 25% of trade, goods 75%. Tradable share of output by sector is **not found**; use goods sectors tradable and services mostly not (*design*).

**E8. Material and energy per tonne (recipe table).** Each process is `inputs -> output` with energy in GJ and labour in hours: steel scrap-EAF 10.2 GJ/t, BF-BOF 24.0 GJ/t; clinker 3.5 GJ/t plus 0.1 MWh/t cement; refinery own use 3 to 8% of crude; milling 60 to 100 kWh/t. Labour per tonne for these processes is **not found**; derive from labour share of value added (E1) instead of inventing hours.

**E9. Money, inflation and rates (simple).** `pi = pi_target + g_m - g_y` (quantity-theory form, *design*); real rate `r = i - pi`. Realistic eras: 0 to 4% per year in stable regimes, up to 13.5% (US 1980) in a bad one; hyperinflation exists (Reinhart-Rogoff, https://scholar.harvard.edu/sites/scholar.harvard.edu/files/rogoff/files/the_i_word.pdf, no figure fetched). Credit ceiling `Credit <= c * Y`, c realistic 0.2 to 1.5 (world 1.48 on one series, wide spread), sandbox 0 to 3.

**E10. Outputs the province returns.** Output Y by sector from E1 and E3; jobs by sector = output / output-per-worker; trade flows from E7; shortages from E6 stock shortfalls; energy deficit from E1's gate. Each is a head count or a stock, never a free stat (world numbers rule 1).

## 3. How this closes the loop with pops and nations

- Pops are L in E1: workers leave the pool when drafted or taxed, so army and officials cost output.
- Pop wealth sets income per worker, which sets E5 demand and the E2 sector mix, so a richer province shifts jobs from farms to services.
- Nation revenue is a share of Y collected by officials who are pops; investment `s*Y` is what the state or citizens divert from consumption.
- Infrastructure is capital in E4: roads, grid and water have d and build time, so neglect lowers A and raises transport cost in E7.
- Shortages (E6) feed back to pops as hunger, unrest and migration, and to the nation as legitimacy loss.
- Energy and materials gate output (E1, E8); a lost plant lowers yield, a restored one raises it, matching the era-as-readout rule.
- Trade (E7) lets a nation cover missing inputs at a price and exposes it to blockade or a subject-state tribute.
- Nation decisions (tax, investment share, price caps, tariffs) change `s`, `k`, `delta` and reach pops next tick.

## 4. Unverified / not found

- Not found: IO coefficients by sector (agriculture, mining, energy, construction, services) from a table; only manufacturing 34% value added and US whole-economy 45% intermediate share.
- Not found: value-added share at each stage of extraction, processing, manufacturing, distribution chains.
- Not found: labour-hours per tonne for smelting, refining, milling.
- Not found: construction labour-hours per m2 by building type (BLS Bulletins 1755, 1892 and 2102 on fraser.stlouisfed.org hold them, not read); construction cost per m2 beyond one China set; building lifetimes by type with solid sources.
- Not found: tradable share of output by sector; sector shares of GDP for high-income countries in a fetched table; Engel curves by income decile for housing in poor countries; price-adjustment speed k; typical interest-rate ranges by era beyond the US 1980 peak.
- Doubtful: the World Bank infrastructure table (2000) gives power capacity in kW per capita, which would be implausibly high; read as W per capita it is also low. Treat as relative ratios only. Middle-income road density below low-income looks like a series artefact.
- Doubtful: credit to GDP 148% (world) mixes definitions; US 198% is on a different series.
- The capital-share range 0.46 to 0.56 is a single paper's estimate and conflicts with the usual one-third; the realistic band in E1 is *design*.
- Several figures came from vendor or aggregator sites (oxmaint, theglobaleconomy, civils.ai); verify against worldsteel, IEA, WDI before pinning goldens.
- No source page was opened (WebFetch blocked).

## 5. Data licences

We use only public-domain or CC BY data. Checked by search summaries only; confirm on each portal before shipping data.

| Source | Licence found | URL | Status |
| --- | --- | --- | --- |
| World Bank Open Data (WDI) | CC BY 4.0 (attribution) | https://datacatalog.worldbank.org/public-licenses | OK, medium |
| Penn World Table 10.0 (and 10.01, 11.0 listed on the same catalogue) | CC BY 4.0 | https://rug.nl/ggdc/productivity/pwt/pwt-releases/pwt100 | OK, medium |
| World Input-Output Database (WIOD) | CC BY 4.0 | https://www.rug.nl/ggdc/valuechain/wiod/ | OK, medium |
| OECD data and publications | CC BY 4.0 for content from 1 July 2024; earlier content under OECD terms "on terms similar to CC BY"; exceptions possible | https://www.oecd.org/en/about/news/press-releases/2024/07/oecd-data-publications-and-analysis-become-freely-accessible.html | OK for new data; check each dataset, medium |
| EU KLEMS | CC BY 4.0 (DataverseNL) | https://www.rug.nl/ggdc/productivity/eu-klems/ | OK, low (one summary) |
| Our World in Data | CC BY for its own work; third-party data keeps its own terms | https://ourworldindata.org/employment-in-agriculture | OK with care |
| Eurostat | not checked | https://ec.europa.eu/eurostat/statistics-explained/index.php?title=Household_consumption_by_purpose | not found: check reuse policy |
| BEA, BLS, US Census | US federal data, not checked here (normally public domain) | https://apps.bea.gov/national/pdf/BEA_depreciation_rates.pdf | not found: confirm |
| WTO, FAO, USDA ERS, worldsteel, IEA, GCCA | not checked; IEA and worldsteel often restrict reuse | https://worldsteel.org/media/press-releases/2022/sustainability-indicators-2022/ | not found: do not ship their tables without a check |
| theglobaleconomy.com, civils.ai, oxmaint.com, tradingeconomics | commercial aggregators; not CC BY | various | do not ship; use only to locate the primary series |
