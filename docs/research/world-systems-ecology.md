# World systems: flora, fauna, soils and fisheries in numbers

**Date:** 2026-10-06
**Status:** research note, not a decision. Nothing here is Decided, Implemented or Validated. It feeds [0031](../decisions/0031-realistic-world-numbers-and-the-pop-nation-loop.md) and [0032](../decisions/0032-worlds-start-with-nations-and-everything-follows-logic.md) (decision 5: the simulation must cover flora, fauna and resources) and sits beside [world numbers](../design/world-numbers.md) and [world survival](../design/world-survival.md) (agriculture, wildlife).

**Method and limits.** WebFetch was blocked by the network proxy (daac.ornl.gov returned EGRESS_BLOCKED; other hosts were not tried after that). Every number tagged "search" comes from WebSearch result summaries, not from reading the source page. "derived" means our own arithmetic on cited inputs. "memory" means background knowledge not confirmed in this session; the URL is only a place to check, and the confidence is L. Confidence: H = summary quotes the number and it matches well-known values; M = second-hand or range-valued; L = memory or derived from weak inputs.

## 1. Numbers

### Biomass, productivity, forests

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Global forest area | 4.06 billion ha, 31% of land (FRA 2020) | ha | https://www.fao.org/newsroom/detail/World's-most-comprehensive-analysis-of-forest-resources-launched-today-in-an-innovative-format/en (search) | H |
| Share of forest by domain | tropical 45%, then boreal, temperate, subtropical | share of forest area | https://www.atibt.org/en/fao-global-forest-resources-assessment-2020-fra-2020 (search) | M |
| Forest carbon density (live+dead+soil as FRA reports it) | 159 (1990) to 163 (2020) | t C/ha | https://openknowledge.fao.org/3/ca9825en/ca9825en.pdf (search) | M |
| Forest carbon stock | 668 Gt (1990) to 662 Gt (2020) | Gt C | same FRA 2020 report (search) | M |
| Global NPP, ocean | 48.5 | Pg C/yr | https://bionumbers.hms.harvard.edu/bionumber.aspx?id=102936 (search; Field et al. 1998) | H |
| Global NPP, land and total | not confirmed in this session; Field 1998 is the place to read it | Pg C/yr | https://www.fsl.orst.edu/rna/Documents/publications/Primary production of the biosphere Integrating terrestrial and oceanic components.pdf | not found (read) |
| NPP, 53 sites (34 grassland, 14 tropical forest, 5 boreal) | above-ground 35 to 2,320; below-ground 20 to 1,832; total 182 to 3,538 | g/m2/yr (dry matter) | https://daac.ornl.gov/NPP/guides/NPP_MULTIBIOME.html (search) | M |
| Biome NPP means (rainforest about 2,200, temperate deciduous about 1,200, boreal about 800, savanna about 900, temperate grassland about 600) | classic textbook values, not confirmed by any fetched page | g/m2/yr (dry matter) | https://link.springer.com/book/9783642809156 (Lieth and Whittaker 1975) | L (memory) |
| Global vegetation carbon today vs without land use | about 450 vs about 916 | Pg C | https://ideas.repec.org/a/nat/nature/v553y2018i7686d10.1038_nature25138.html (search; Erb et al. 2018) | M |
| Share of that loss from forest management plus grazing (no land-cover change) | 42 to 47% (two-thirds forest management, one-third grazing) | % of vegetation carbon loss | same Erb 2018 URL (search) | M |
| Mean annual increment, tropical and temperate plantations | 1 to 2 (poor) up to 25 to 30 | m3/ha/yr | https://www.fao.org/4/ac121e/ac121e02.htm (search) | M |
| MAI of Eucalyptus grandis and hybrids; rotation | 15 to 40 m3/ha/yr; 5 to 15 yr | m3/ha/yr; yr | https://www.fao.org/4/ac121e/ac121e03.htm (search) | M |
| MAI temperate eucalypts; rotation | 5 to 18 m3/ha/yr; 10 to 15 yr | m3/ha/yr; yr | same (search) | M |
| MAI of pine plantations | 3 (Scandinavia) to 24 (New Zealand) | m3/ha/yr | https://www.fao.org/4/ac133e/ac133e05.htm (search) | M |
| Rotation extremes | shortest 5 yr (coppiced eucalypt); over 100 yr for high-value temperate | yr | https://www.fao.org/4/ac121e/ac121e02.htm (search) | M |
| Growing stock per ha, by domain | not found in summaries (FRA 2020 tables hold it) | m3/ha | https://openknowledge.fao.org/3/ca9825en/ca9825en.pdf | not found |
| Above-ground biomass in a Neotropical secondary forest after 20 yr | 122 mean (20 to 225 across sites) | Mg/ha | https://repository.lsu.edu/biosci_pubs/4715 (search; Poorter et al. 2016) | H |
| Time to 90% of old-growth above-ground biomass | median 66 | yr | same Poorter 2016 (search) | H |
| Secondary-forest carbon uptake | 3.05, 11 times old-growth | Mg C/ha/yr | same Poorter 2016 (search) | H |
| Grassland and crop residue yields per ha | not found | t/ha | not found | not found |

### Animals

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Damuth's law: herbivore density vs body mass W | density proportional to W^-0.75; pooled communities fit about -0.70 | individuals/km2 | https://pdodds.w3.uvm.edu/files/papers/others/1981/damuth1981a.pdf (search) | H |
| Consequence | energy used by a local population is independent of body size (density x metabolic rate about constant) | qualitative | same | H |
| Carnivore density rule | 10,000 kg of prey supports about 90 kg of carnivore, any body mass (about 111 kg prey per kg carnivore; 25 species) | kg/kg | https://synapsesocial.com/papers/W2014666871 (search; Carbone and Gittleman 2002) | H |
| Trophic transfer efficiency | mean 10% (Pauly and Christensen 1995); observed 2 to 24% | % | https://fishbase.se/Ecopath/PPRnatur.htm (search) | H |
| Large-herbivore biomass rises with rainfall and soil nutrients (African savanna) | relation exists; coefficients not extracted | kg/km2 vs mm | https://link.springer.com/article/10.1007/BF00317566 (search; East 1984, Coe et al. 1976) | M (shape), not found (numbers) |
| Serengeti wet-season large-mammal density | about 100 animals/km2, about 0.014 kg/m2 (14,000 kg/km2) | animals/km2; kg/m2 | https://arxiv.org/pdf/1412.5773 (search) | L |
| Wild mammal biomass vs humans vs livestock | 0.007 vs 0.06 vs 0.11 | Gt C | https://www.greenpeace.org/international/story/17788/how-much-of-earths-biomass-is-affected-by-humans/ (search; Bar-On et al. 2018) | H |
| Total biomass of life; plants; animals | 550; 450; about 2 | Gt C | https://wis-wander.weizmann.ac.il/node/9421 (search) | H |
| Sustainable hunting yield: Schaefer MSY | H_MSY = rK/4 at B = K/2 | per yr | https://en.wikipedia.org/wiki/Maximum_sustainable_yield (search) | H |
| Sustainable hunting yield: refined models | MSY at about 30% of unexploited size | fraction of K | same (search) | M |

### Livestock

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Feed conversion, 2023 | chicken 1.7; pork 2.75; beef 6.0 (1950s: 4.75, 4.5, 7.5) | kg feed per kg gain | https://www.spglobal.com/energy/en/news-research/special-reports/agriculture/atlas-of-food-fertilizer-rice-grain-sugar-protein/protein/meat-production (search) | M |
| Animal unit | 1 AU = 1,000 lb cow; 1 AUM = 780 lb dry forage; 1 AUY = 9,360 lb | lb | https://projects.sare.org/media/pdf/P/u/b/Pub-31-1005-Carrying-Capacity-and-Stocking-Rate.pdf (search) | M |
| Stocking rate, unirrigated annual rangeland, Sierra Foothills | 15 to 18 acres (6 to 7.3 ha) per 1,200 lb cow-year | ha/cow-yr | https://ucanr.edu/sites/SoCo/files/27848.pdf (search) | M |
| Irrigated pasture | about 1 acre (0.4 ha) per cow for a 6-month season | ha/cow | same (search) | M |
| Livestock share of farmland / calories / protein | 77% / 18% / 37% | share | https://ourworldindata.org/global-land-for-agriculture (search) | M |
| Stocking rate by biome, milk and draught output, livestock per person by era | not found | | not found | not found |

### Fisheries and marine

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Per-capita fish consumption | 9.1 (1961) to 20.7 (2022) | kg/person/yr | https://www.fao.org/newsroom/detail/fao-report-global-fisheries-and-aquaculture-production-reaches-a-new-record-high/en (search) | H |
| Fisheries plus aquaculture, 2022 | 223.2 Mt total (185.4 animals, 37.8 algae); aquaculture 130.9 Mt, 51% of animals | Mt | same (search) | H |
| Stocks fished sustainably (2021) | 62.3% (37.7% over-fished) | % of monitored stocks | same (search) | H |
| Worm et al. 2006 "collapse" definition | catch under 10% of the series maximum; heavily criticised | | https://www.forest-trends.org/wp-content/uploads/imported/murawski_etal_2007_letters_how-bad-ocean-biodiv-loss-pdf.pdf (search) | M |
| Rebuilding target | B_MSY about 40% of unfished biomass (used in management documents) | fraction of B0 | https://www.afma.gov.au/sites/default/files/2023-02/Redfish-rebuilding-strategy-2016.pdf (search; one agency, not universal) | L |
| Fish yield per km2 of sea, regional MSY values | not found | | not found | not found |

### Soils, land use, recovery

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Tolerable erosion (equal to soil formation) | about 1 | t/ha/yr | https://pmc.ncbi.nlm.nih.gov/articles/PMC1948917 (search; Montgomery 2007) | M |
| US soil-loss tolerance (T) | 5 to 12 t/ha/yr, about 0.4 to 1 mm/yr | t/ha/yr | https://soil.copernicus.org/articles/5/253/2019/ (search) | M |
| Erosion under ploughing vs natural rates | 1 to 2 orders of magnitude above soil production; no-till close to production | factor | https://pmc.ncbi.nlm.nih.gov/articles/PMC1948917 (search) | M |
| Soil formation range, global compilation | 0 to 28.8 mm/yr (overlaps erosion 0 to 52.9) | mm/yr | https://soil.copernicus.org/articles/5/253/2019/ (search) | M |
| Shifting-cultivation fallow | 7 to 10 yr usual; up to about 20 yr to restore nutrients to forest level | yr | https://en.wikipedia.org/wiki/Shifting_cultivation (search) | M |
| Share of population fed by synthetic N | 44% (2000), 48% (2008) | % | https://ourworldindata.org/how-many-people-does-synthetic-fertilizer-feed (search; Erisman et al. 2008) | H |
| Yield gain from fertiliser | 30 to 50% reasonable; often 60% or more vs unfertilised | % | https://repository.rothamsted.ac.uk/id/eprint/32789 (search; Stewart et al. 2005) | M |
| Net forest loss | 7.8 (1990-2000), 5.2 (2000-2010), 4.7 (2010-2020) | Mha/yr | https://www.drishtiias.com/daily-updates/daily-news-analysis/deforestation-rate-declined/print_manually (search; FRA 2020) | H |
| Deforestation (gross) | 12 (2010-2015), 10 (2015-2020); tropics 10.1 (2000-2010) to 7 (2010-2018) | Mha/yr | https://www.downtoearth.org.in/forests/global-forest-resource-assessment-tropical-forests-the-size-of-europe-lost-in-first-20-years-of-this-century-82679 (search) | M |
| Wheat yield history (already in world-numbers) | 0.5 to 1 t/ha pre-1600 to 8 t/ha (2018) | t/ha | see [world numbers](../design/world-numbers.md) | M |

### Disease and pests

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Yield loss to pests and pathogens | wheat 21.5 (10.1 to 28.1); rice 30.0 (24.6 to 40.9); maize 22.5 (19.5 to 41.1); potato 17.2 (8.1 to 21.0); soybean 21.4 (11.0 to 32.4) | % of yield | https://hal.inrae.fr/hal-02628565 (search; Savary et al. 2019) | H |
| Highest losses | in food-deficit regions with fast-growing populations | qualitative | same (search) | M |
| Zoonotic share of human pathogens | 868 of 1,415 species (61%); 75% of emerging ones | % | https://pmc.ncbi.nlm.nih.gov/articles/PMC1088493 (search; Taylor et al. 2001) | H |
| Zoonotic share of emerging infectious disease events | 60.3%, of which 71.8% from wildlife (335 events, 1940-2004) | % | https://pmc.ncbi.nlm.nih.gov/articles/PMC5960580 (search; Jones et al. 2008) | H |

### Human use

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Fuelwood per person, developing countries | mean 0.40 (tropical 0.51, non-tropical 0.23); Africa 0.67, Asia 0.29, Latin America 0.56, Oceania 0.88; 19 developing countries above 1 | m3/person/yr | https://www.fao.org/4/q4960e/q4960e02.htm (search) | M |
| Pre-industrial firewood, Spain 1860 | up to 1.8 | kg/person/day | https://pfcyl.es/sites/default/files/biblioteca/documentos/forests_fuelwood_pulpwood_and_lumber_in_spain_1860-2000._a_non_declensionist_story.pdf (search) | M |
| Wood per modern house | about 7 (timber frame, one estimate) to 14.58 (Australia 2017-18); 8.0 (France) to 20.5-28.6 (Sweden) per dwelling | m3/dwelling | https://fwpa.com.au/news/how-much-wood-in-an-average-house-14-58-m3-not-42/ (search) | M |
| Forager returns: Ache hunting; palm starch | 830; 850 to 1,200 | kcal/hour | https://www.unm.edu/%7Erobwal/achehunt.pdf (search) | M |
| Forager returns, Kelly tables by resource | tables exist (Kelly 2013, table 3.4); values not extracted | kcal/hour | https://ia801202.us.archive.org/23/items/robert-l.-kelly-the-lifeways-of-hunter-gatherers-the-foraging-spectrum-cambridge/Robert%20L.%20Kelly%20-%20The%20Lifeways%20of%20Hunter-Gatherers_%20The%20Foraging%20Spectrum-Cambridge%20University%20Press%20%282013%29.pdf | not found (numbers) |
| Timber for pre-industrial buildings, per-building tables | not found | | not found | not found |

## 2. Equations we could implement

All are per province (or cell) and per day, with a monthly or yearly aggregation for slow stocks. Symbols: `B` stock, `K` capacity, `r` regrowth rate, `H` harvest. Bands: "real" is what the sources above support; "sandbox" is a wide band for tuning. Bands marked (design) are our choice, not a source figure.

1. **Plant stock, logistic with harvest.** `dB/dt = r B (1 - B/K) - H`. Applies to forest wood, wild plants, grass. `K = biome_K × soil × water`. Real: r about 1/MAI-derived (a stand with MAI 3 to 25 m3/ha/yr on a stock of 100 to 300 m3/ha gives r of roughly 0.01 to 0.1 per yr, derived). Sandbox: 0.005 to 0.3. Sustainable wood yield per ha = MAI (3 to 25 m3/ha/yr plantation; natural forest lower, not found).
2. **Wood demand vs supply.** `demand = pop × (fuel 0.4 to 1.0 m3/yr [FRA/FAO] + building and tools term)`; `supply_sustainable = forest_ha × MAI`. Stress `S = demand / supply`; if `S > 1` the stock `B` falls by `(S - 1) × supply`. Sandbox fuel: 0.2 to 2 m3/person/yr.
3. **Forest recovery after clearing.** `AGB(t) = AGB_old × (1 - exp(-t / tau))`, with `tau` set so 90% is reached at 66 yr median (tau about 29 yr, derived from Poorter 2016); site range 20 to 225 Mg/ha at 20 yr gives wide `tau`. Sandbox tau: 10 to 100 yr.
4. **Soil fertility, depletion and fallow.** `dF/dt = regen × (1 - F) - uptake × crop_intensity + fertiliser`; `F` in [0,1] multiplies yield. Regen tuned so fallow of 7 to 10 yr restores most of F and about 20 yr restores forest-level F. Fertiliser raises yield 30 to 60% at cost of an input flow; its absence caps the population (about 48% of people today depend on synthetic N). Real band regen: 0.05 to 0.2 per yr (design, from the fallow numbers).
5. **Soil loss.** `soil_depth -= erosion × (1 - cover)`; real: about 1 t/ha/yr tolerable; ploughed 10 to 100 times natural rates; no-till near natural. Sandbox: 0.1 to 50 t/ha/yr.
6. **Animal density from body mass (Damuth).** `D = c × W^-0.75`, with `c` scaled so total herbivore energy use equals `eta × NPP_edible × area` (eta about 0.1 per trophic step, range 0.02 to 0.24). Carnivore stock `B_carn = B_prey × 90/10000 = 0.009 × B_prey`. Sandbox: carnivore ratio 0.003 to 0.03.
7. **Wildlife with hunting.** Same logistic as (1) with `r` falling with body mass; hunting `H = effort × q × B`. Sustainable cap: `H_msy = r K / 4`; warning below `B < 0.3 K`. Real r: small game higher, large game low (values not found; sandbox 0.05 to 1.0 per yr).
8. **Livestock carrying capacity.** `N_AU = pasture_ha × forage_t_per_ha / 4.2 t per AU-yr` (9,360 lb = 4.25 t dry forage per AU-year, derived). Rangeland 6 to 7 ha per cow in dry annual range; irrigated 0.4 ha per cow for half a year. Output via feed conversion (1.7 to 6 kg feed per kg gain). Livestock compete with crops for land (77% of farmland).
9. **Fishery (Schaefer).** `dB/dt = r B (1 - B/K) - q E B`; `MSY = r K / 4` at `B = K/2`. Collapse flag when catch < 10% of peak (Worm definition) or `B < 0.2 K` (design). Aquaculture adds a flow decoupled from `B` but needing feed (FCR) and coast area. Per-capita fish demand 9 to 21 kg/yr.
10. **Crop yield.** `yield = potential × F × water × (1 - pest_loss)`; `pest_loss` real band 0.10 to 0.41 (Savary), a stochastic multiplier with outbreak years; sandbox 0 to 0.7.
11. **Zoonosis hazard.** `hazard = base × wild_contact × livestock_density`, tying to the dead and disease systems; share of new pathogens from animals about 60%.
12. **Hunting and gathering time budget.** `food_per_hour = kcal_per_hour(resource, density)`; return falls as local density falls (diet-breadth logic). Real: about 830 kcal/h hunting, 850 to 1,200 kcal/h starchy plants (Ache). Sandbox 200 to 3,000.

## 3. How this closes the loop with pops and nations

- **Food cap on pops:** cropland × F × yield, minus feed and loss, divided by need, sets province carrying capacity (extends `K_ag` in world-numbers); soil depletion lowers it over generations.
- **Fuel and materials cap:** wood supply (MAI × forest area) against fuel and building demand makes cities that outgrow their forest import wood or shrink; the stress `S` is a visible nation stat.
- **Pop choices feed back to the land:** more pops, more clearing (net forest loss about 5 Mha/yr today), erosion, overfishing; the stock then lowers the next yield.
- **Nations decide land use:** policies (fallow rules, hunting limits, fishing quotas, fertiliser) change `r`, `H` and `F`; subject provinces can be drained.
- **Wildlife and fish as buffers:** hunting and fishing give cheap food at first, then decline past 0.3 K; they are the early-game and famine fallback.
- **Pests and blight shock output:** a 10 to 40% yield loss band gives famine years that stress stability and migration.
- **Zoonosis ties fauna to health:** livestock and wildlife contact raise epidemic hazard, linking this note to disease and the dead.
- **Recovery is slow:** forest recovery takes decades (66 yr to 90%), so mistakes outlive the generation that made them, which suits the generational run.

## 4. Unverified / not found

- WebFetch blocked: all numbers are from search summaries unless marked derived or memory.
- Biome NPP and standing biomass table (Whittaker and Likens): values above are memory (L); need the Lieth and Whittaker 1975 table or the ORNL NPP files.
- Global terrestrial NPP total (Field 1998) and ocean primary production by region: not confirmed.
- FRA 2020 growing stock (m3/ha) and biomass (t/ha) by domain: not found in summaries.
- Natural-forest sustainable harvest per ha, grassland and crop residue t/ha: not found.
- Herbivore and carnivore biomass by biome (kg/km2): only qualitative (rainfall-driven) and one Serengeti figure (L).
- Livestock: stocking rates by biome, milk, meat and draught output, livestock per person by era: not found.
- Fisheries: collapse thresholds beyond Worm's catch-based rule and one agency's 40% B0; yield per km2; typical r values: not found.
- Timber per pre-industrial building, per-hour gathering returns by resource: not found.
- Soil fertility regeneration rate per year: only fallow durations found; our `regen` band is a design choice.
- Costello et al. 2012 and Robinson and Redford production model were searched and not found.

## 5. Data licences

We use only public-domain or CC BY data. Checked from search summaries (not read on the source page):

| Source | Licence found | Use here | URL |
| --- | --- | --- | --- |
| FAO corporate statistical databases incl. FAOSTAT | CC BY 4.0 (current policy; the 2020 policy used CC BY-NC-SA 3.0 IGO, now superseded) | OK with attribution; re-check the page date before shipping | https://www.fao.org/contact-us/terms/db-terms-of-use |
| FAO FRA 2020 report text and data | not confirmed in this session; FRA tables are in the FAO corporate databases | check before use | https://openknowledge.fao.org/3/ca9825en/ca9825en.pdf |
| Sea Around Us catch reconstruction | CC BY-NC-SA 4.0 per search summary: NonCommercial and ShareAlike | not allowed under our rule (NC); use FAO capture data instead | https://vliz.be/imisdocs/publications/ocrd/295390.pdf |
| GBIF | per-dataset: CC0, CC BY or CC BY-NC (83% of publishers CC BY, 5% CC0) | OK if we filter to CC0 and CC BY datasets and cite each | https://www.gbif.org/terms |
| TRY plant traits | CC BY 4.0 since 2019, cite TRY and contributing datasets | OK with attribution | https://www.bgc-jena.mpg.de/5451834/1554881196 |
| ORNL DAAC NPP Multi-Biome | licence not checked (US government data host; terms not confirmed) | check before use | https://daac.ornl.gov/NPP/guides/NPP_MULTIBIOME.html |
| Published papers (Damuth, Carbone and Gittleman, Poorter, Savary, Jones, Taylor, Erb, Bar-On, Field) | copyrighted; we use the reported numbers as facts with citation, not the figures or tables | cite only | see Numbers |
