# World numbers

How big a world is, how many people it can hold, where they live, how nations arise, how pops are divided inside them, and how pops and nations shape each other. This is the quantitative core of generated worlds and of the Macro and City layers: the loop you feel in upkeeping a city in Cities: Skylines or running an empire in Stellaris, written down as laws with realistic numbers.

Intent **Decided** ([0035](../decisions/0035-realistic-world-numbers-and-the-pop-nation-loop.md)): realistic numbers matter more than Earth 1:1; the pops-and-nation loop is the point; numbers come from real sources, never invented and presented as real. Everything below is a **Proposal** until tuned and confirmed. Source of every number and equation: the four research notes, [capacity](../research/world-numbers-capacity.md), [settlement](../research/world-numbers-settlement.md), [nations](../research/world-numbers-nations.md), [demography](../research/world-numbers-demography.md). **Evidence caveat:** the research could not open the source sites, so every cited number comes from search summaries (confidence medium at best) and several are marked low; parameters marked *design* have no source and exist to be calibrated. Nothing here is Implemented or Validated.

The map of every system, the order a starting world is built in, and how factions hold territory: [world systems](world-systems.md) ([0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md)).

Related design: [world simulation](world-simulation.md) §3–5 and §12 (generated worlds, settlements), [macro layer](macro-layer.md), [sandbox parameters](sandbox-parameters.md) §4–5, [pop continuity](pop-continuity.md), [spec 28](../tier0/specs/28-pop-levels.md).

## 1. The loop

```
 land, climate, water, resources  ──►  carrying capacity K (per region, per era)
              ▲                                   │
              │                                   ▼
   infrastructure, tech, trade         pops: how many, where, age, schooling, work, wealth
   (what the nation builds)                       │
              ▲                                   ▼
              │                         surplus above subsistence
   policy and spending  ◄──  nation capability: revenue, army, officials, order, reach
```

Rules that keep it honest (the simulation contract, [world simulation](world-simulation.md) §2):

1. **A nation never has a stat that is not a head count of someone.** Revenue, soldiers and officials are drawn from pops (workers, taxpayers, literate recruits) and leave the labour pool when taken, so power costs output.
2. **Everything is a rate with a realistic band and a flagged sandbox band** (validator, [sandbox parameters](sandbox-parameters.md) §1.4). A generated world draws its exponents once, shows them, and saves them with the world.
3. **Era is not a clock; it is a readout of what exists.** Where the research uses an "era" (yield, tax ceiling, transport cost), the simulation reads it from the capabilities the nation actually has in Layers 2 and 1 (fertiliser plants, roads, schools, a tax office), per [0029](../decisions/0029-leader-records-foxhole-tech-readout.md) and [0030](../decisions/0030-pop-continuity-and-capability-loss.md). A lost plant lowers yield; a restored one raises it.
4. **The world is complete at the start** ([0018](../decisions/0018-sandbox-answers.md)): the laws below *generate* a plausible present (nations, cities, populations) and then keep running live (towns grow or fall, nations split or collapse). No history is simulated before play.

## 2. From land to population

### 2.1 The world

- **Area scales as R²; nothing else follows from radius** (no general law was found for land fraction, ice, desert or arable share). So land fraction, biome shares and relief are generator outputs or sliders; Earth is the reference: land 148.9 million km² (29.1% of the surface), 71% of land habitable (104 million km²), 44% of that farmed. Realistic band for a habitable world: radius 0.7–1.5 R⊕, land fraction 0.15–0.45, habitable share of land 0.5–0.85; sandbox wider.
- Biome shares and the 0034 world fields (elevation, rain, rivers) come from the generator ([world simulation](world-simulation.md) §12); this doc reads them.

### 2.2 Carrying capacity K (capacity note, E1–E10)

| Step | Law (all Proposals) | Anchors (search summaries) |
| --- | --- | --- |
| Food from foraging | `K_hg = Σ biome area × density`, density tied to net primary production | Mesolithic France 0.1 per km²; about 1000-fold unexplained spread per unit production, so band 0.01–1 per km² |
| Food from farming | `K_ag = (cropland × yield × kcal − feed − loss) / need`, `need` about 2000 kcal a day | Wheat yield 0.5–1 t/ha pre-1600 to 1.5 (1800), 4 (1965), 8 (2018); cropland 1/3 of farmland; livestock use 77% of farmland for 18% of calories |
| Density by stage | early farmers 2.5–7.5 per km², Iron Age 10–20; later periods not found | HYDE would give per-era density (CC BY 4.0; not opened) |
| Water | `K_water = renewable flow / minimum per person` | WHO 20 / 50 / 100 L a day; Falkenmark 1700 stress, 1000 scarcity, 500 absolute scarcity m³ a year; farming 71% of withdrawals |
| Energy and materials | demand `P × e(era, income)`; stocks deplete by extraction | World 73 GJ per person a year (2024), US 266; welfare gains diminish above 50–70; copper reserves about 40–45 years at today's rate (reserves grow with price and technology) |
| Combine | `K = min(food, water)` per region, raised by trade and technology, cut by shocks; growth toward K by vital rates that respond to food per need (Malthus, with the preventive check) | Haber-Bosch nitrogen feeds about 48% of people |

A region's capacity is therefore an **output of its land, its water and what its nation has built**, not a slider. A district can be food-rich and water-poor; ports and rivers let `K_eff` exceed the local figure; a blockade or a lost plant removes that term.

**Worked check (derived, not a source claim).** Earth's world population at about 10,000 BC was about 4 million against 104 million km² of habitable land: 0.04 per km² on average, inside the forager band. A world with R = 1.2 R⊕ and the same fractions has 1.44 times the land, so the same band holds 1.44 times the people, and the farming terms scale with the cropland its biomes allow.

### 2.3 Where people live (settlement note, E1–E10)

| Question | Law | Anchors |
| --- | --- | --- |
| Where | Cell suitability, log-linear: soil, water distance, climate, slope, elevation, coast and river proximity; density = `K × growth state` | About 50% of people on 1% of land (low confidence); about 40% within 100 km of a coast; 27% on the 9% of land both under 100 km from shore and under 100 m high |
| City sizes | Pareto (rank-size) with exponent ζ; draw ζ once per world | Soo mean 1.11 over 73 countries, Zipf rejected in 53; flatter for early or fragmented worlds (0.7–1.0); a world of 30 towns cannot tell ζ 1.0 from 1.25, and the generator says so |
| Growth | Gibrat noise plus a pull toward each town's K | σ not found: design |
| Urbanisation | Logistic in log income | Europe under 10% urban (towns over 10,000) 1300s–1800, China about 7% in 1800, world 55% in 2018 |
| Primacy | The capital is inflated, more under centralised rule | +50% in the cited data |
| Size effects | Output about N^1.15, infrastructure about N^0.85, needs about N^1.0, crime and disease about N^1.1–1.25; draw one exponent per world, in a band, because published values move with the boundary | Bettencourt et al.; per-quantity values partly from memory, low |
| Reach and trade | Cost-distance by mode and terrain; gravity flows `S_i S_j / cost^δ` with δ 0.6–1.0; towns spaced by Christaller classes (5–11 km village to market town) | Roman wheat cost per kg-km: sea 1, river down 5, river up 10, wagon about 52; Tobler walking speed |
| Roads | Spanning tree, then loops where flow is high, upgraded by traffic; total length ∝ N^0.85 | German road and cable data |

The result is the "pops gather where conditions are best, towns and roads follow" of [world simulation](world-simulation.md) §12, with sourced exponents, and the **same gravity flows** carry trade, migration and, with the virus layer, outbreaks.

## 3. How the people of a nation are divided

From the demography note. Pops follow the nation's income `y` and level of development; this is what [0034](../decisions/0034-nation-demographics.md) profiles hold, now with laws for how a profile moves over time.

| Division | Law | Anchors |
| --- | --- | --- |
| Life expectancy | Preston form: concave in log income, the curve shifting up with time | About 30 in 1800 (no region above 40) to 72.6 in 2019 |
| Fertility | Logistic fall in log income, plus a speed-up with era | Global TFR 4.84 (1950) to 2.23 (2021); early transitions about 1 child per decade, fast ones 6 to under 3 in 10–11 years (low confidence) |
| Age structure | Cohort projection from births and survival | Median age Africa 20.2, world 31.6, Europe 43.5; children per 100 working-age 68.3 and 23.8 (checks) |
| Sector and occupation | Agriculture share falls with log income, industry humps, services rise; occupations assigned by education and sector | DR Congo 69% against US 1% in agriculture; group shares by income not found |
| Education | Attainment by birth cohort, logistic in income with a cohort shift | Barro-Lee shows steady cohort gains (data not confirmed licensed) |
| Wages and skill | Mincer: `ln w = Σ r_j s_j` | World mean return 9% a year of schooling; Penn World Table 13.4 / 10.1 / 6.8% by schooling band |
| Labour force | Participation by age, sex and income; unemployment around 5% | Men 72%, women 47% (2023 world) |
| Production | `Y = A K^α (h L)^(1−α)`, α about 1/3 (sandbox 0.2–0.6), `h = exp(Σ r_j s_j)` | Labour share has fallen since the 1980s; TFP by era not found (world GDP per head ×13.7 over 1820–2020, about 1.3% a year) |
| Distribution | Lognormal incomes from the Gini, Pareto tail, parent-to-child elasticity | Average Gini 40.8 over 126 countries; wealth Gini about twice income Gini; US father-son elasticity 0.4–0.6 |
| Cities and migration | Urban premium 0.04–0.08 earnings elasticity to city size, offset by costs; gravity and Harris-Todaro migration | Distance elasticity −2 to −1 |

The Malthusian regime for early eras (output per head flat, population absorbs gains; takeoff needs TFP growth and a fertility fall) comes out of these laws with the capacity limit; it should not be scripted.

## 4. How nations arise and what they can do

From the nations note.

### 4.1 Arising, size and count

- **Conditions** (rules, from Carneiro, Scott, Tilly, Turchin, Diamond): a region becomes a state when it holds at least about 50,000 people (Diamond; low confidence), a storable, taxable surplus crop (Scott), is **circumscribed** so losers cannot flee (Carneiro), and faces competition or war pressure for a number of years. Turchin et al. (2013) explain 65% of the variance in where large old-world states arose, against 16% without the spread of military technology.
- **Size:** country population and area follow power laws with exponent about 1; population grows as area^0.82. State counts by era are a calibration target: 27 (1800), about 50 (1900), 51 (1945), 193 (2026). Largest empires at peak 14.7–35.5 million km².
- **Reach:** `r = message speed × the delay the centre tolerates` (Roman couriers about 46 miles a day). Territory beyond `r` needs a delegated tier and costs legitimacy; this is what limits size before technology and caps it per level of administration.
- **Collapse and split:** when fiscal stress, elite surplus and falling wages stay high and legitimacy low, a state falls one tier (empire, kingdom, chiefdom), keeping its pops. Secular cycles of 100–300 years should appear from the laws, not be scripted; if they do not, that is a calibration finding.

### 4.2 Capability from pops

| Capability | Law (design where unsourced) | Anchors |
| --- | --- | --- |
| Tax ceiling | `τ_cap(y)` rising with income | Preindustrial about 5% of GDP (Rome, medieval, China); Britain 3.4% (1665) to 12.9% (1800); modern low 14, lower-middle 18, upper-middle 22, high 33 |
| Revenue | `R = τ_cap × state capacity × f(legitimacy) × Y` | Weak-state leakage (Ottoman tax farming) qualitative only |
| Growth gate | Above a tax-to-GDP tipping point (about 12.75, rule of thumb 15) GDP per head is about 7.5% higher after 10 years, if capacity allows | IMF, 139 countries |
| Army | `min(m × P, army budget ÷ (pay × wage))`; soldiers leave the labour pool | Sustained 0.5–1% of population preindustrial; peaks 5–10% (Rome, WWII); about 10% industrial limit (mostly low-confidence sources); Roman army 50–80% of the budget |
| Officials | `B = b(y) × P`, limited by literate recruits | Rome about 8 per 10,000 (low); OECD government employment 18.4% of jobs |
| Levels | `ceil(log(P ÷ P_local) ÷ log(span))`, span 5–10 | Graicunas 5–6, typical 3–15 |
| Capacity `c` | Weighted mix of officials per need, literacy, infrastructure reach, minus corruption | Weights: design |
| Stability | Political stress = mass immiseration × elite overproduction × fiscal distress; legitimacy follows services and security minus stress | Structural-demographic theory; formula is ours |

## 5. The feedbacks, in one table

| From → to | Effect |
| --- | --- |
| Land, water, tech → K | Ceiling for pops; surplus pays non-farmers |
| K and settlement laws → pops | Where people live, city sizes, urban share |
| Pops → output | `Y = A K^α (hL)^(1−α)`; schooling raises `h` with a one-generation lag |
| Output → nation | Revenue, army, officials, within pop limits |
| Nation → pops | Services lift life expectancy and schooling; order lowers stress; mobilisation removes workers |
| Nation → land | Roads cut transport cost and raise reach; irrigation and fertiliser raise yield; security lowers shocks |
| Pops → nation stability | Falling wages, elite surplus, fiscal distress multiply into stress; stress lowers legitimacy; lower legitimacy lowers revenue |
| Shock (war, outbreak, famine, blackout) → K_eff and P | The same equations show recovery time |

Cities: Skylines is the superlinear city (output N^1.15 against upkeep and disease), Stellaris is the empire (revenue, army, reach, stability, secular cycles), both with sourced sizes.

## 6. Parameters, bands and where they go

Every parameter is a slider with a **realistic band** (inside the cited ranges) and a **sandbox band** (wide, tagged realism-breaking), validated by the one validator ([sandbox parameters](sandbox-parameters.md) §1.4). They join the nation and world presets ([sandbox parameters](sandbox-parameters.md) §4, §5.2) and are copied into each world's save like the other presets. The research notes list every band; the ones that decide most:

| Group | Parameters |
| --- | --- |
| World | radius, land fraction, habitable share, biome shares |
| Capacity | yield by stage, cropland share limit, kcal need, feed share, loss, water stress threshold, energy per person, trade bonus |
| Settlement | suitability weights, coast and river scales, ζ (city exponent), primacy, urban-share curve, scaling exponents, transport cost by mode, gravity δ, town spacing |
| Demography | life-expectancy and fertility curves, sector curves, education curve, Mincer returns, α, Gini |
| Nation | tax ceiling curve, state capacity, army share (sustained and peak), officials per head, span, tolerated delay, stress weights, tipping point |

## 7. Update order and time steps

Slow, aggregate and deterministic ([world simulation](world-simulation.md) §2): a **yearly step per region** (K, growth, urbanisation, structure, fiscal year) on top of the long steps already designed ([spec 28](../tier0/specs/28-pop-levels.md)); city sizes and flows move on the same yearly step; the nation actor decides about once a game day from a fixed menu ([macro layer](macro-layer.md) §7). Randomness is keyed per entity and purpose; the generator's draws (exponents, ζ, seeds) are saved with the world so a generator update never changes a save ([world simulation](world-simulation.md) §12).

## 8. What already exists, what is new

| Exists | New |
| --- | --- |
| Pop identity, lazy / record / embodied levels, births and the demography jump, rosters, region-wide events ([spec 28](../tier0/specs/28-pop-levels.md)) | World fields and K per region |
| `NationDemographics` profile and loader ([0034](../decisions/0034-nation-demographics.md)) | The settlement generator (suitability, ζ, roads) |
| Name and town generators, bridged to the registry ([content tools](content-tools.md) §8) | The nation actor: revenue, army, officials, stability, reach |
| Placeholder mortality and fertility as data in the save | Laws that move a profile over time (transition, structure, education) |
| Region events, commands with reasons | Generation of nations and their borders; split and collapse rules |

## 9. Calibration and validation

The model earns trust by hitting real targets, not by feel. A **realism report** ([sandbox parameters](sandbox-parameters.md) §13.1) generates many worlds (headless, fixed seeds) and prints each target as a band: share of people on the top 1% of land and within 100 km of coast; city-size exponent against the drawn ζ; urban share against income; age structure, median age and dependency ratios against the cited checks; tax-to-GDP and army shares against their ceilings; number of states against area and population power laws; and whether secular cycles appear. A failing band is a finding about the model, never a number to be fudged.

## 10. Data: what we can use

- **Confirmed open (CC BY):** UN World Population Prospects (CC BY 3.0 IGO), Maddison Project 2023 (CC BY 4.0), ILOSTAT (CC BY 4.0, via a mirror; confirm), HYDE 3.3 (CC BY 4.0), WorldPop (CC BY 4.0).
- **Not confirmed, do not ship data from them yet:** Penn World Table, Barro-Lee, World Bank, Our World in Data (per-chart licences), World Inequality Database, GHSL, UN urban prospects, USGS, FAO, Natural Earth, OECD tables, IMF datasets.
- **Not usable:** Fragile States Index and Seshat (CC BY-NC-SA), UBS wealth tables (proprietary), IHME fertility estimates (usually non-commercial).
- **Facts are not copyrightable; datasets and book tables are.** We cite published numbers and ship only our own derived parameter tables.
- **A practical gap:** the research agents could not open the data sites. Real tables will need a data tool (`tools/data/`) run where the network allows, that fetches a licensed dataset, converts it to a nation or world preset with its source and year on every table, and fails if the licence is not CC BY or public domain. Until then the only profile is a placeholder.

## 11. Open questions

1. **Start era and the default world.** The setting is present day (decided), but the laws are era-dependent. Recommendation: a world starts at the present, and era is the readout of what exists (§1 rule 3); a preset can start a world in an earlier era by starting it with fewer capabilities, which a tech tree makes natural.
2. **How deep the generated past goes.** [0018](../decisions/0018-sandbox-answers.md) says no history before play; [world simulation](world-simulation.md) §12 proposes a short four-era pass for district styles. Recommendation: generate nations and cities from the laws directly, with the short era pass only for build styles, and show the drawn exponents.
3. **Real tables.** The sourced numbers above are few and second-hand. Do you want the data tool (§10) and a first real country (or the global tables) once the network allows, run on your machine or in CI?
4. **Fathers, marriage and households** are not yet in the pop model; household formation interacts with fertility and wealth transmission. Recommendation: after the nation actor.
5. **War between nations** and Foxhole-style logistics stay secondary for now ([0031](../decisions/0031-restoring-capabilities-real-time-playable-age.md) and the war research note), but the army law above already prices it in pops, wages and revenue.

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- Golden test: a town that fills a new district all at once must not show a death wave in its age pyramid later. ([cities-skylines](../research/cities-skylines.md))
- Persons per km² by biome and era (k_biome) for carrying capacity: not found; what to use? ([world-numbers-settlement](../research/world-numbers-settlement.md))
- Numeric weights for the suitability factors and site catchment radii: not found; set by calibration only? ([world-numbers-settlement](../research/world-numbers-settlement.md))
- Urbanisation-vs-income slope and midpoint (E5): not found; fit to the 1800 and 2018 anchors? ([world-numbers-settlement](../research/world-numbers-settlement.md))
- Sigma of Gibrat growth (E4, 0.1-0.3 per year) is an unsourced design value; tune or source it. ([world-numbers-settlement](../research/world-numbers-settlement.md))
- Rail, steam and cart cost per tonne-km in one unit for post-Roman eras: not found. ([world-numbers-settlement](../research/world-numbers-settlement.md))
- Licences for GHSL, UN WUP 2018, CIESIN/SEDAC and ORBIS are unconfirmed; confirm before shipping data from them. ([world-numbers-settlement](../research/world-numbers-settlement.md))
- No hunter-gatherer density medians per biome, and few pre-industrial agrarian densities. Open HYDE to calibrate K per era? ([world-numbers-capacity](../research/world-numbers-capacity.md))
- No quantitative Malthus or Boserup parameters (r, elasticities, famine mortality); the 0.8 pressure threshold that advances the era is our own choice. ([world-numbers-capacity](../research/world-numbers-capacity.md))
- Unconfirmed values from memory: grain kcal per gram, seed share, pre-industrial r, pasture ratio, pre-industrial GJ per person. ([world-numbers-capacity](../research/world-numbers-capacity.md))
- Reserves and production for iron, bauxite, coal, oil, gas, lithium and phosphate are missing (the USGS PDF was unreachable). ([world-numbers-capacity](../research/world-numbers-capacity.md))
- Sector employment shares as a function of income per head (only two country points and the qualitative law were found). ([world-numbers-demography](../research/world-numbers-demography.md))
- Gini by income level and era, and intergenerational income elasticities outside the US. ([world-numbers-demography](../research/world-numbers-demography.md))
- TFP growth by era and the Penn World Table capital share; PWT's licence is unconfirmed, so not to be used yet. ([world-numbers-demography](../research/world-numbers-demography.md))
- The female labour-force U-shape in income, and the health-spending to life-expectancy coefficient. ([world-numbers-demography](../research/world-numbers-demography.md))
- Licences for Eurostat, US federal data (BEA, BLS, Census), worldsteel, IEA and GCCA are unchecked; confirm before shipping that data. ([world-systems-economy](../research/world-systems-economy.md))
- How many levels of government does a state need for its population size? No source; P_local and span are uncalibrated. ([world-numbers-nations](../research/world-numbers-nations.md))
- Bureaucrat-to-population ratios beyond Rome (Han and Qing are missing). ([world-numbers-nations](../research/world-numbers-nations.md))
- Turchin's published Political Stress Index formula; E7 (stress = M×E×F) is our stand-in. ([world-numbers-nations](../research/world-numbers-nations.md))
- An area-per-state rule, and mean and median country population and area, for seeding world generation. ([world-numbers-nations](../research/world-numbers-nations.md))
- How long a delay can a centre tolerate before reach fails? 15-60 days is a design guess. ([world-numbers-nations](../research/world-numbers-nations.md))

## 12. Order of work (dependencies only)

World fields and K per region → settlement generator and the road and trade graph → the yearly region step (demography and structure laws on `NationDemographics`) → the nation actor (revenue, army, officials, stability, reach) → nation formation, split and collapse → the realism report on generated worlds → the data tool and real tables. Each unit gets a spec first ([CLAUDE.md](../../CLAUDE.md)).

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [services-by-settlement-size](../research/services-by-settlement-size.md): Which services a place has: settlement size, country and who covers the rest
- [human-generator-data](../research/human-generator-data.md): The human generator: data, licences and the generation pipeline
- [war-in-other-games](../research/war-in-other-games.md): War in other games, as a reference
- [world-numbers-capacity](../research/world-numbers-capacity.md): World numbers: how many people can a world hold?
- [world-numbers-demography](../research/world-numbers-demography.md): World numbers: demography, structure, education, labour, inequality, production
- [world-numbers-nations](../research/world-numbers-nations.md): World numbers for nations: how they arise, how big they are, what they can do
- [world-numbers-settlement](../research/world-numbers-settlement.md): World numbers: how people spread over land, and how towns and cities arise
- [world-systems-ecology](../research/world-systems-ecology.md): World systems: flora, fauna, soils and fisheries in numbers
- [world-systems-economy](../research/world-systems-economy.md): World systems: how a real economy works in numbers
- [world-systems-factions](../research/world-systems-factions.md): World systems for factions: territory, subjects, expansion, conquest, collapse
- [world-systems-resources-networks](../research/world-systems-resources-networks.md): World systems: where resources are, and how grids and networks are laid out
