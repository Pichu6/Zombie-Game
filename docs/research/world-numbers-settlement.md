# World numbers: how people spread over land, and how towns and cities arise

**research note, not a decision**

- **Date:** 2026-10-06
- **Serves:** [0031](../decisions/0031-realistic-world-numbers-and-the-pop-nation-loop.md) (realistic numbers, pop/nation loop) and [world-simulation section 12](../design/world-simulation.md) (generator writes world fields: elevation, rivers, climate, soil, population).
- **Status of everything below:** Proposal / research. Nothing is Decided, Implemented or Validated.
- **Method and limits:** WebFetch was blocked by the network egress proxy (tested on acris.aalto.fi and en.wikipedia.org). **Every number below comes from WebSearch result summaries, not from reading the papers.** Where a figure is quoted from memory of the literature and the search did not return it, it is marked "memory, not confirmed" and its confidence is Low. Sandbox bands and "design" values in the equations are our own proposals, not sourced.

## Numbers

Confidence: High = several consistent summaries or a primary statistical agency; Medium = one summary of a primary paper; Low = popular article, memory, or an estimate with wide historical uncertainty.

### 1. How concentrated is population on land

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Share of world population on the densest land | about 50% of people on about 1% of land (cells of 9 sq mi, over 8,000 people each; about 900 per sq mi, about 350 per km2) | share of people / share of land | https://99percentinvisible.org/article/99-darkness-see-half-worlds-population-lives-1-land/ | Low (popular map from NASA gridded data; cell-size dependent) |
| Population within 100 km of a sea coast | about 38% (1990), about 40% (commonly quoted) | share of world population | https://www.causeartist.com/content/files/esa/sustdev/natlinfo/indicators/methodology_sheets/oceans_seas_coasts/pop_coastal_areas.pdf | Medium |
| Population on the near-coast lowland (under 100 km from shore and under 100 m elevation) | 27% of people on 9% of land (2010); about 25% in 1990 | share of population / share of land | https://www.ldeo.columbia.edu/~small/PDF/ISDE_SmallCohen.pdf (Small and Cohen, CIESIN) | Medium |
| Implied density ratio of that coastal lowland to the rest of land | about 3.7x average (27/9 = 3.0 relative to the world mean; rest of land about 0.8x) | ratio, derived here | derived from the row above | Medium (derived) |
| Rivers and lakes as a settlement factor | Kummu et al. quantify population share near rivers and lakes (figures not retrieved: fetch blocked) | not found | https://acris.aalto.fi/ws/portalfiles/portal/11440226/kummu_et_al_erl.pdf | not found |
| Urban agglomerations | about half of urban dwellers live in settlements under 500,000; about 1 in 8 in 33 megacities of over 10 million | share of urban population | https://population.un.org/wup/publications/Files/WUP2018-KeyFacts.pdf | High |

### 2. City size distribution

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Zipf / Pareto exponent under Gibrat's law (steady state) | 1 (rank-size: size of rank r is about S1 / r) | exponent | https://scholar.harvard.edu/sites/scholar.harvard.edu/files/xgabaix/files/zipfs_law.pdf (Gabaix 1999) | High |
| Cross-country mean Pareto exponent, 73 countries (Soo 2005, OLS) | 1.11; Zipf (exponent 1) rejected in 53 of 73 countries by OLS, 30 of 73 by Hill estimator | exponent | https://researchonline.lse.ac.uk/id/eprint/19947 | Medium (the 1.11 reached me via a secondary summary) |
| Later study of the same countries | mean exponent about 1.02 | exponent | https://vskp.vse.cz/english/38885 (as summarised) | Low |
| Rosen and Resnick 1980 | 32 of 44 countries had exponent significantly above 1; 4 significantly below 1 | count | https://mpra.ub.uni-muenchen.de/11504/1/MPRA_paper_11504.pdf (secondary mention) | Low |
| Standard error of a Pareto exponent estimate | ζ times sqrt(2/n), n = number of cities in the tail (not the OLS standard error) | formula | https://nber.org/papers/t0342 (Gabaix and Ibragimov, Rank-1/2) | Medium |
| Gibrat's law | growth rate distribution (mean, variance) independent of city size; full city distribution (not just the upper tail) is lognormal in US 2000 data | qualitative | https://ideas.repec.org/h/eee/regchp/4-53.html and https://mpra.ub.uni-muenchen.de/10411/1/MPRA_paper_10411.pdf | Medium |
| Pre-industrial Europe | a power law is rejected 1300 to 1600; plausible only in 1700 and 1800 (lognormal also plausible) | qualitative | https://mpra.ub.uni-muenchen.de/83096/1/MPRA_paper_83096.pdf (as summarised) | Low |
| Islamic world vs Europe, 800 to 1800 | large empires: exponent above 1 stated as theory (capitals enlarged by drawing from other cities); competing small European states: exponent below 1 | qualitative | https://mpra.ub.uni-muenchen.de/80475/1/MPRA_paper_80475.pdf (Bosker et al. line of work, as summarised) | Low |
| Netherlands rank-size exponent | 0.55 (1600), 1.03 (1900), 0.72 (1990) | exponent | https://arxiv.org/pdf/1309.4862 (as summarised) | Low |
| Primacy: optimal degree of primacy rises with income to about USD 5,000 (1987 PPP), then declines modestly; falls with country size; 0.1 off optimum costs about 0.6% growth a year | see left | index / USD / % | https://www.nber.org/papers/w7503.pdf (Henderson 2003) | Medium |
| Primacy and politics | dictatorships: central city about 50% larger than in democracies; high tariffs and high internal trade costs raise primacy; urban concentration peaks around USD 2,400 income in their data | % / USD | https://www.nber.org/papers/w4715.pdf (Ades and Glaeser) | Medium |

### 3. Urbanisation vs income and era; largest cities by era

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Urban share of world population today | 55% (2018), projected 68% (2050) | share | https://population.un.org/wup/publications/Files/WUP2018-KeyFacts.pdf | High |
| Largest city today | Tokyo about 37 million; Delhi 29; Shanghai 26; Mexico City and Sao Paulo about 22 (2018) | people | https://population.un.org/wup/publications/Files/WUP2018-KeyFacts.pdf | High |
| Europe 14th to 19th century urban share | under 10% in towns over 10,000 people; under 13% in towns over 5,000 (Bairoch) | share | https://www.statista.com/statistics/1304890/urbanization-europe-1800 | Medium |
| China 1800 | about 7% urban (down from 11 to 12% earlier in the Qing) | share | https://ideas.repec.org/p/ucg/wpaper/0063.html | Medium |
| Largest city in Western Europe, 1500 | Paris about 225,000, almost double the second (Naples) | people | https://statista.com/statistics/1021988/thirty-largest-cities-western-europe-1500 | Medium |
| First cities of about 1 million | Rome, Jiankang or Chang'an (between 1st and 8th century); Baghdad in the 9th century; all highly uncertain | people | https://www.visualcapitalist.com/cp/animated-map-largest-cities-throughout-history (relays Wikipedia list) | Low |
| 1200 CE | Baghdad, Cairo, Constantinople about 250,000 each (Bairoch 1988 as cited) | people | https://www.imsc.res.in/~indu/JM/2015/SepOct/Cities/cities.txt | Low |
| Urbanisation vs income | urban share rises with the log of GDP per capita (cross-section); a time series of speed vs growth shows no global correlation | relation | https://pmc.ncbi.nlm.nih.gov/articles/PMC4123908 and https://paulromer.net/urbanization-passes-the-pritchett-test/ | Medium (slope not retrieved) |
| Long-run urban share series (10,000 BCE to 2023, HYDE 3.3) | exists as a CC BY series; values not retrieved | share | https://ourworldindata.org/grapher/long-term-urban-population-region | not found (values) |

### 4. Settlement suitability factors

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Rank of factors in archaeological GIS suitability studies | soil fertility most significant; then slope, elevation, distance to streams; also proximity to water and to communication routes | ordinal | https://gsma.lu.ac.ir/article_738005_35b7445bfe1fd8bfd088df17dc90337b.pdf and https://orbilu.uni.lu/bitstream/10993/58733/1/Sikk_etal_2022_JArcheoReports.pdf | Medium |
| Early Neolithic spread models | slope important; rivers acted as transport vectors | qualitative | https://keep-24.lib.asu.edu/node/155075 (as summarised) | Low |
| Carrying capacity of a pre-industrial settlement | set mostly by soil fertility and area; density dependence mediated by soil fertility | qualitative | https://www.cts.cuni.cz/~storch/publications/Fanta_et_al_2018_PRSB_Equilibrium_dynamics_of_human_populations.pdf | Medium |
| Density in 1 to 1500 CE | higher with higher land productivity and earlier agriculture (Ashraf and Galor) | qualitative | https://economics.brown.edu/sites/default/files/papers/2008-6_paper.pdf | Medium |
| Numeric weights of each factor, site catchment radius, persons per km2 by biome | not found in retrieved text | | | not found |

### 5. Urban scaling laws

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Socio-economic outputs (GDP, wages, patents, crime, communicable disease) | beta about 1.15 (superlinear); "about 1.2" for wealth and innovation | exponent, Y = Y0 N^beta | https://arxiv.org/pdf/1401.3956 and https://arxiv.org/abs/1301.1674 | Medium |
| Infrastructure (cable length, gas stations, road area) | beta about 0.85 (sublinear) | exponent | https://arxiv.org/abs/1301.1674 | Medium |
| Road area, Germany 2002 | 0.83 | exponent | https://arxiv.org/pdf/1401.3956 | Medium |
| Individual needs (water, electricity use, jobs, housing) | beta about 1 | exponent | https://arxiv.org/abs/1301.1674 | Medium |
| Per-quantity values (patents 1.27, GDP 1.15, wages 1.12, crime 1.16, AIDS 1.23, gas stations 0.77, roads 0.83, cable 0.87) | not returned by search; these are my recollection of Bettencourt 2007 (PNAS 104:7301) | exponent | https://www.citedrive.com/en/discovery/growth-innovation-scaling-and-the-pace-of-life-in-cities | Low (memory, not confirmed) |
| Uncertainty | exponents depend on how the city is delimited (commuting-based definitions, thousands of re-draws); most per-capita indicators in England and Wales scale about linearly, and where nonlinear the exponent fluctuates considerably | qualitative | https://discovery-pp.ucl.ac.uk/1455713/4/20140745.full.pdf (Arcaute et al. 2015) | Medium |
| Statistical caveat | the scaling exponent depends on model assumptions and on the heavy-tailed city-size distribution (Leitao et al.) | qualitative | https://arxiv.org/pdf/1905.06166 | Medium |

### 6. Trade, spatial interaction and transport cost

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Roman sea transport | 0.00067 | denarii per kg wheat per km | https://www.ancientportsantiques.com/wp-content/uploads/Documents/ETUDESarchivees/MedNavigationRoutes/ORBISprices-Scheidel2013.pdf | Medium |
| Roman river, downstream / upstream | 0.0034 / 0.0068 | denarii per kg per km | same | Medium |
| Roman wagon on road | 0.035 | denarii per kg per km | same | Medium |
| Derived ratios (Roman) | wagon about 52x sea, about 10x river down, about 5x river up; Roma to Tarraco route example: 46x dearer by road than sea | ratio | same, and https://legacy.geog.ucsb.edu/interactive-map-shows-how-roman-empire-was-shaped-by-economic-constraints/ | Medium (derived) |
| Walking speed on flat terrain (Tobler) | 5 km/h; max 6 km/h at about -2.86 degrees; off-path multiply by 3/5; horseback by 5/4 | km/h | https://en.wikipedia.org/wiki/Tobler%27s_hiking_function | High |
| Tobler formula (as written in a result) | W = 6 exp(-3.5 abs(S + 0.05)), S = slope as rise over run | km/h | https://arnac.eie.gr/research/algorithms | Medium |
| Pack animals, late Middle Ages | pack horse about 120 kg, up to 25 km/day; donkey 75 kg; mule 100 kg; camel 175 kg | kg, km/day | https://transportgeography.org/contents/chapter1/emergence-of-mechanized-transportation-systems/pre-industrial-travel-modes/ | Medium |
| Mule in rough country | 16 to 24 km/day with 250 lb; 32 to 40 km/day under ordinary conditions | km/day | https://mail.mapaspects.org/tripcevich-phd-diss/ch3-archaeology-region-/32-long-distance-trade/325-caravan-travel-distances-and-/index.html | Low |
| River barge, Rhine downstream | 50 to 60 km/day; draught boats 15 to 17 km/day (medieval and early modern, upstream) | km/day | https://intarch.ac.uk/journal/issue36/5/5-4.html | Low |
| Rail and 19th century ocean freight | ocean freight per ton-mile fell from 0.23 cent (1887) to 0.08 cent (1898); canal cost per mile 25,000 to 80,000 USD vs turnpike 5,000 to 10,000 USD (1820s) | cents, USD | https://kar.kent.ac.uk/113785/1/heag001.pdf and https://1632.org/1632tech/transportation-cost-faq/ | Low |
| Per-tonne-km cost for rail and cart by era in consistent units | not found | | | not found |
| Gravity model, distance elasticity of trade | recent estimates cluster between -1.0 and -0.6, no downward trend; meta-analysis of 1,467 estimates in 103 papers, later extended to about 2,700 estimates in 197 studies | elasticity | https://blogs.ubc.ca/khead/files/2016/07/meta.pdf and https://papers.ssrn.com/abstract=4980883 | Medium |
| Distance effect rose around mid-20th century and stayed high | qualitative (Disdier and Head 2008) | | https://blogs.ubc.ca/khead/files/2016/07/meta.pdf | Medium |
| Christaller hierarchies | k = 3 (marketing), 4 (transport), 7 (administrative): each higher-order centre serves k centres of the order below (counting itself); town spacing multiplies by sqrt(k) each level | factor | https://www.geog.leeds.ac.uk/people/a.turner/projects/medalus3/cpt.htm | Medium |
| Medieval English market spacing | Bracton (13th century): no market within 6.66 miles (about 10.7 km) of another; derived from a 20-mile day split in thirds; real maps show the rule widely ignored | miles / km | https://www.mikegravel.org/?p=146135 and https://historyrise.com/article/market-towns-and-fair-days-centers-of-medieval-commerce/ | Low |
| Late medieval English small towns | half of trading partners within about 10 km | km | https://www.medievalists.net/?p=3833 | Low |
| Skinner, rural China standard market | one standard market town and 18 villages (6 + 12) in a hexagon; community 6,000 to 12,000 people; farthest village at most 8 km | people, km | https://escholarship.org/uc/item/51x4g3qh | Medium |
| Huizhou example | market centres form a triangle with sides 33 km; hexagon sides 5.5 km (1/6) | km | https://re.public.polimi.it/retrieve/e0c31c10-4ba7-4599-e053-1705fe0aef77/LIN%2005%20Paper.pdf | Low |

## Equations we could implement

All parameter bands are **Proposals**. "Realistic band" is anchored to the table above; "sandbox band" is wide, for worlds that break realism on purpose (tagged in Sandbox per 0014). Inputs are the world fields from section 12: elevation, slope, river flow, distance to coast, climate class, soil fertility, and the carrying capacity K computed per cell.

**E1. Cell suitability** (log-linear, as in the archaeological suitability studies)

`S(c) = exp( w_soil*soil + w_water*f_water + w_clim*f_clim - w_slope*slope - w_elev*max(0, elev-elev0)/1000 + w_coast*f_coast + w_river*f_river )`

with `f_water = exp(-d_water / 2 km)`, `f_coast = exp(-d_coast / 100 km)`, `f_river = exp(-d_river / 10 km)`. The 100 km coast scale matches the coastal-share figures; the others are tunable. Weight order from the sources: soil > slope, elevation, water distance. Realistic band: w_soil 1.0 to 2.0, w_water 0.5 to 1.5, w_slope 0.5 to 2 per 10 degrees, w_coast 0.3 to 1.0. Sandbox: 0 to 4.

**E2. Carrying capacity and density**

`K(c) = A_cell * k_biome * soil * tech(era)`; `density(c) = K(c) * g(t)` where g(t) is the pop-growth state of 0030. Population is the integral of density over the cell. Calibrate so that the global share on the top 1% of land is about 50% (target, from the table) and the share within 100 km of coast about 40%. Realistic band: calibrate to those two targets. Sandbox: coast share 5 to 90%. Per-biome k_biome values: not found (see Unverified).

**E3. Settlement sizes (rank-size with a regional exponent)**

Let the total urban population be `U = u(era, income) * P`. Draw city sizes from a Pareto law with exponent `zeta`, then place them by sampling cells by E1 weight. Size of rank r: `S_r = S_1 * r^(-1/zeta)`, where S_1 is fixed by sum of S_r = U. Realistic band: zeta 0.9 to 1.2 (Soo mean 1.11, Zipf 1, later 1.02). Pre-industrial and fragmented states: zeta 0.7 to 1.0 (flatter, rank-size rejected before 1700). Large empires with a strong capital: zeta 1.1 to 1.4 (theory only, low confidence). Standard error for a fit with n cities is zeta * sqrt(2/n), so a world with 30 towns cannot distinguish zeta 1.0 from 1.25 by fit alone; the generator should say so. Sandbox: 0.5 to 2.0.

**E4. Growth with Gibrat noise plus a suitability drift**

`d ln S_i / dt = mu(era) + a * (ln K_i - ln S_i)/tau + sigma * N(0,1)`, with the last term independent of size (Gibrat). With a = 0 it converges to a lognormal / Zipf-like shape; with a > 0 and tau about 50 to 200 years, towns stay tied to their carrying capacity. sigma: not found; start from sigma 0.1 to 0.3 per year of simulation for towns and tune against E3. This is a design choice, not a sourced value.

**E5. Primacy and urbanisation**

`u = u_min + (u_max - u_min) / (1 + exp(-(ln(y) - ln(y0)) / s))` (logistic in log income). Targets: u about 0.07 to 0.10 before industrialisation (Europe and China 1800), about 0.55 in a modern economy (UN 2018). Slope s and midpoint y0: not found; set to hit those two anchors. Primacy: raise S_1 by `1 + p`, p = 0 to 0.5 (dictatorship about +50% as in Ades and Glaeser), with an optimum that rises with income up to about USD 5,000 (1987 PPP) and then falls (Henderson). Realistic band for p: 0 to 0.5. Sandbox: 0 to 3.

**E6. Output and cost scaling by city size N (for pop and nation loop)**

`output_i = y0 * N^beta_y`, `infra_i = c0 * N^beta_i`, `crime_i, disease_i = k * N^beta_s`. Realistic band: beta_y 1.10 to 1.20, beta_i 0.80 to 0.90, beta_needs 0.95 to 1.05, beta_s 1.10 to 1.25 (the last from the "social" group; individual values unconfirmed). Because published exponents move with the city boundary, treat them as defaults, not laws: use a band, draw one exponent per world, and display it. Sandbox: 0.5 to 1.5.

**E7. Transport cost and market reach**

Cost of an edge: `cost = d * c_mode(era) * terrain_factor`, with a time version `t = d / v(slope)`, `v = 6 exp(-3.5 |s + 0.05|)` km/h (Tobler, off-path times 0.6). Cost ratios by mode in the pre-industrial era (relative, from the Roman figures): sea 1, river down about 5, river up about 10, wagon about 52. Realistic band: wagon/sea 20 to 60 (Roman figure about 52; the 46x Roma-Tarraco example sits inside it). Sandbox: any mode ratio from 1 to 200. Rail and steam shipping cut these ratios; the retrieved numbers are not enough to calibrate them (not found).

**E8. Gravity flows between towns i and j (trade, migration, road demand)**

`T_ij = G * S_i^a * S_j^b / d_ij^delta`, with a, b near 1, delta 0.6 to 1.0 (modern gravity-model estimates) and 1.0 to 2.0 as a sandbox band for the pre-industrial world, where cost per distance is much higher (the cost, not the exponent, carries the era difference; with a cost-distance d_ij measured as E7 cost, delta can stay 0.6 to 1.0). Use E7 cost-distance, not Euclidean distance, as d_ij.

**E9. Town spacing and market areas (Christaller check)**

For a town of level L (L = 0 is a village), expected spacing to the next town of the same level: `D_L = D_0 * sqrt(k)^L`, with k = 3 (market), 4 (transport) or 7 (administrative). Anchors: village-to-market-town about 5 to 11 km (Skinner: farthest village 8 km; hexagon side 5.5 km in Huizhou; Bracton 10.7 km), so D_0 about 5 to 11 km; with k = 3 each higher level multiplies spacing by 1.73. Use this as a prior for placement: after E3 places towns by suitability, apply a repulsion term so that towns of the same size class sit at least about 0.5 D_L apart. Real maps often ignore the rule, so keep the repulsion weak in the realistic band (0.3 to 0.7) and let sandbox set 0 to 1.

**E10. Road and trade network**

Build a graph in three passes: (1) minimum spanning tree over towns on E7 cost-distance, so every town is connected; (2) add an edge i-j when `T_ij / cost_ij` exceeds a threshold that falls with era wealth, which adds loops (rings of roads) between large pairs; (3) upgrade edges by traffic: `road_class = floor(log_k(T_ij))`. Road length per capita follows the infrastructure exponent: total road length `L = L0 * N_total^0.85`, so the generator should check the built network against it (about 0.83 to 0.85 as sourced; the sample is German roads and cable networks, not pre-industrial). Realistic band for the exponent: 0.80 to 0.90. Sandbox: 0.5 to 1.2.

## How this closes the loop with pops and nations

- **Where pops live is a function of the land, not a slider.** E1 and E2 produce density from soil, water, coast and relief, so a nation's workers, soldiers and taxpayers are tied to its geography and a lost river valley is a lost population.
- **Pop counts feed the output of a city superlinearly (E6).** Merging or growing a city raises output per person by roughly 15% per doubling of size, but crime and disease per person also rise: this gives the Stellaris/Cities: Skylines trade-off with a sourced size.
- **Infrastructure cost is sublinear.** Per-capita road, cable and pipe length falls as a city grows (exponent about 0.85), so upkeep is a nation's lever for efficiency and a reason large cities are cheaper per head to serve.
- **Transport cost sets the nation's reach.** E7 makes rivers and coasts cheap and land roads dear in early eras, so early nations hug water, and road building (the nation's action) is what lets the interior grow.
- **Urbanisation follows income and era (E5).** A nation's economy changes the urban share of its pops (farmers become workers); the rural-to-urban move is a pop flow, so policy affects the shape of the city list.
- **Primacy is a political outcome.** Centralised or autocratic nations skew size to the capital (about +50% in the cited data); the loop is nation type -> city distribution -> where the nation is vulnerable (the capital) and where its tax base is.
- **Gravity (E8) turns town sizes into trade, migration and disease routes.** The same flows give the nation its trade income and also the paths of an outbreak, which links to the virus layer without a separate model.
- **A saved world stores its draws.** Per section 12, generated settlements are saved at creation; the exponents, zeta and the seed are saved with them, so a generator change never alters a save.

## Unverified / not found

- WebFetch was blocked (egress proxy), so no paper was read. All numbers are from search summaries; several were returned only by secondary pages.
- Per-quantity Bettencourt exponents (patents, GDP, wages, crime, AIDS, gas stations): only the group values (about 1.15, 1, 0.85) and German road area 0.83 were returned. The detailed table is from memory.
- Exponent uncertainty (confidence intervals) for urban scaling: not found numerically; only the qualitative result that exponents change with the city boundary.
- Kummu et al. river and lake population shares and the density gradient with distance from rivers: not found.
- Elevation and climate effects on density (population by elevation band and by climate class): only the coastal-lowland result was returned. Not found otherwise.
- Persons per km2 by biome and by era (carrying capacity values k_biome): not found.
- Numeric weights for suitability factors and site catchment radii: not found.
- Urban share series for 1800, 1900 and 2000 from HYDE/OWID: sources identified, values not retrieved. Only Europe (under 10 to 13%) and China (about 7%) for 1800.
- Urbanisation-vs-income slope and the 2.7% result: the "1% more urban share goes with 2.7% higher GDP per capita" line was in a summary without a clear source or direction; not used.
- Largest cities by era beyond the few listed (Wikipedia list was blocked): Rome / Baghdad / Chang'an dates are disputed.
- Rail, road truck and 19th century cart cost per tonne-km in one consistent unit: not found. Roman figures are for wheat in denarii and cannot be converted to modern money without a further source.
- Pre-industrial Zipf exponents by region: only the Netherlands numbers and a qualitative Europe result; both low confidence.
- Standard deviation of Gibrat growth (sigma in E4) and Christaller conformity for real maps: not found.
- Gravity exponents for city mass (a, b) in historical trade: not found; modern distance elasticity only.

## Data licences

We only use public-domain or CC BY data. Licences below come from search summaries; confirm on the dataset page before shipping a build.

| Dataset | Use | Licence found | Source URL | Status |
| --- | --- | --- | --- | --- |
| HYDE 3.3 (PBL, History Database of the Global Environment) | historical population, urban share, built-up area | CC BY 4.0 | https://datahub.io/climate-and-environment/hyde-history-database-of-the-global-environment | Usable with attribution |
| Our World in Data grapher series built on HYDE | long-run urban share | OWID republishes under CC BY; source is HYDE (CC BY 4.0) | https://ourworldindata.org/grapher/long-term-urban-population-region | Usable with attribution; check the grapher page |
| WorldPop gridded population | modern population grid | CC BY 4.0 | https://worldpop-public-data.soton.ac.uk/GIS/Population/Individual_countries/CAF/Central_African_Republic_100m_Population/licence.txt | Usable with attribution |
| GHSL (JRC Global Human Settlement Layer) | built-up and population grids | not confirmed by the search | https://human-settlement.emergency.copernicus.eu/ghs_pop.php | Not found: do not use until the licence is read |
| UN World Urbanization Prospects 2018 | modern urban shares and city sizes | not confirmed (UN data terms are commonly CC BY 3.0 IGO, from memory) | https://population.un.org/wup/publications/Files/WUP2018-KeyFacts.pdf | Not found: confirm before using the data (quoting a figure in a note is fine) |
| CIESIN / SEDAC gridded population (Small and Cohen figures) | coastal and elevation shares | not confirmed | https://www.ldeo.columbia.edu/~small/PDF/ISDE_SmallCohen.pdf | Not found: confirm |
| Stanford ORBIS | Roman transport costs | not confirmed | https://www.ancientportsantiques.com/wp-content/uploads/Documents/ETUDESarchivees/MedNavigationRoutes/ORBISprices-Scheidel2013.pdf | Not found: we use the published numbers as facts with citation, not the dataset |
| Papers cited (Bettencourt, Gabaix, Soo, Henderson, etc.) | numbers and equations | copyright of the publishers; facts and equations are not copied, only cited | URLs in the tables | Cite, do not redistribute text |
