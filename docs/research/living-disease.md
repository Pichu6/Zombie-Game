# Disease and degradation in living stocks and soil

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [0059](../decisions/0059-countries-from-water-land-and-resources-everything-alive-can-be-sick.md), [spec 37](../tier0/specs/37-flora-and-fauna.md), [spec 38](../tier0/specs/38-outbreak.md), [world systems](../design/world-systems.md). **Evidence:** web-search summaries of papers, agency pages and press coverage (`reported`); no primary paper was read in full, so nothing here is `sourced`. Numbers to re-check before they go to `data/`: the GLASOD breakdown (BD-46), the Ug99 dates (BD-3), the FMD kernel distances (BD-26), the ASF front speeds (BD-31), the sea star figures (BD-37). Crop, forest and soil numbers already in [world systems ecology](world-systems-ecology.md) are cited by WSE ID, not repeated. **IDs:** BD.

Decision 0059 says everything alive can be sick, soil degrades, the links between them matter, and spread runs on local grid rules with real rates. This note collects those real rates (losses, speeds, R0, recovery times) for crops, forests, livestock, wildlife, fish and soil, and the spatial models that can carry them, then proposes one host-pathogen model for every living stock.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| BD-1 | Irish potato famine: late blight (*Phytophthora infestans*) destroyed about half the crop in 1845 and nearly all of it in 1846 and 1848; of about 8 million people, about 1 million died (mostly of disease, some estimates 1.5 million by 1850) and about 1 million more emigrated; excess mortality roughly doubled for 1846-50 | reported | encyclopedia.com "Famine, Irish"; galwaycommunityheritage.org |
| BD-2 | Bengal famine 1943: 2 to 3 million deaths; rice brown spot (*Cochliobolus miyabeanus*) cut some 1942 plots by up to 90 % (Padmanabhan's data) after a cyclone; whether the blight or war-time policy caused the famine is disputed (Tauger vs Sen); brown spot's usual loss in South Asian lowland rice is about 5 % | reported | environmentandsociety.org "Bengal Famine of 1943"; gktoday.in; phytopathdb.org |
| BD-3 | Wheat stem rust Ug99: found in Uganda 1998-99, Kenya 2001, Ethiopia 2003, Sudan, Yemen 2006 (across the Red Sea), Iran 2007-08, Egypt 2014, Iraq 2019; in more than 14 countries; stem rust can take 70 % or more of a field's yield | reported | gd.eppo.int/reporting/article-770; undark.org 2018; farmprogress.com |
| BD-4 | Ug99 moved about 4,000 km (Uganda to Iran) in about 8 years, about 500 km per year including sea crossings by wind | derived | BD-3, great-circle distance Kampala to western Iran |
| BD-5 | Southern corn leaf blight 1970: a new race (T) of *Bipolaris maydis* took about 16 % of the US corn crop (one source 25 %), because 85-90 % of the corn carried the same Texas male-sterile cytoplasm | reported | par.nsf.gov (Pataky abstract); agweb.com "When the Corn Died"; ntrs.nasa.gov 19720021767 |
| BD-6 | Banana Fusarium wilt TR4: first in Cavendish in Indonesia and Taiwan in the early 1990s, now in at least 21 countries; moves with plants, soil and water; fungicides and fumigants do not work; it stays in soil about 30 years or indefinitely | reported | abgc.org.au 2019; promusa.org TR4; bdspublishing.com 2024 chapter |
| BD-7 | Usual yield lost to pests and pathogens: wheat 21.5 %, rice 30 %, maize 22.5 %, potato 17.2 %, soybean 21.4 % | reported | [WSE-27](world-systems-ecology.md) |
| BD-8 | Rice variety mixtures in Yunnan (Zhu et al. 2000): blast 94 % less severe and yield 89 % higher in the susceptible variety than in monoculture; incidence 18 % in pure stands against 1 % in mixtures; farmers stopped spraying | reported | Zhu et al. 2000, Nature 406:718, ideas.repec.org; edepot.wur.nl/704959 |
| BD-9 | Crop pests and pathogens moved poleward about 2.7 km per year since 1960 (Northern Hemisphere 2.2 ± 0.8), across 612 species; fungi moved poleward, nematodes and viruses did not | reported | Bebber et al. 2013, Nature Climate Change 3:985; carbonbrief.org |
| BD-10 | Rust and mildew spores cross oceans by wind; transoceanic jumps are reported for coffee leaf rust, sugarcane rust and wheat stem rust | reported | Brown and Hovmøller 2002, Science 297:537, cited in pmc.ncbi.nlm.nih.gov/articles/PMC5222527 |
| BD-11 | Plant pathogens show two speeds: a front of tens to hundreds of km per year carried by wind or trade (Ug99 about 500, chestnut blight 30-80, ash dieback about 16) and a slow climate drift of about 2-3 km per year | derived | BD-4, BD-9, BD-13, BD-15 |
| BD-12 | The great crop losses share one condition: a single uniform host over a large area (the Irish lumper potato, Texas-cytoplasm corn, Cavendish banana), and mixing varieties cuts severity by an order of magnitude | estimate | BD-1, BD-5, BD-6, BD-8 |
| BD-13 | Chestnut blight: found in New York in 1904; killed 3 to 4 billion American chestnuts by about 1940-50 across about 9 million acres; spread 20-50 miles (30-80 km) per year with spot infections up to 150 miles (240 km) ahead of the front; the tree survives only as root sprouts that rarely pass 6 m before reinfection | reported | ebsco.com research starter; extension.uga.edu; npn.rngr.net 1978 USDA paper |
| BD-14 | Dutch elm disease: a first wave (*Ophiostoma ulmi*, from about 1920) killed 10-40 % of elms in several European countries and died down by the 1940s; the second (*O. novo-ulmi*, UK from about 1967 on Canadian logs) killed about 20 million UK elms in a decade, over 25 million by the 1990s and probably over 30 million since; about 75 % of North America's 77 million elms | reported | forestresearch.gov.uk Dutch elm disease page; en.wikipedia.org/wiki/Dutch_elm_disease; salisburygreenhouse.com |
| BD-15 | Ash dieback (*Hymenoscyphus fraxineus*): confirmed in the UK in 2012 (probably present from about 2004); 70-85 % mortality expected, up to 99 % feared; by November 2024 in 63 % of UK 10 km squares (England 79.9 %); wind-blown spores, about 16 km per year (local estimate), plus long jumps on nursery stock | reported | Forest Research Chalara situation report Nov 2024; collections.etoncollege.com; uwfs.org.uk |
| BD-16 | Ash dieback reached 63 % of UK squares in about 12 years from confirmation, about 5 % of the country's squares a year | derived | BD-15 |
| BD-17 | Mountain pine beetle, British Columbia from the early 1990s: over 18.1 million ha affected; 752 million m³ of merchantable pine lost by 2017 (58 % of it), half the commercial lodgepole pine volume | reported | natural-resources.canada.ca mountain pine beetle; opentextbc.ca geography ch. 7.6 |
| BD-18 | That single outbreak covered about 0.45 % of the world's forest (18 of 4,060 Mha) | derived | BD-17, [WSE-1](world-systems-ecology.md) |
| BD-19 | A host-specific killer that stays in the landscape stops recovery: the chestnut has not come back in 120 years, though forests in general recover with a time constant of about 29 years ([WSE-35](world-systems-ecology.md)); the stand is replaced by other species instead | estimate | BD-13, WSE-35 |
| BD-20 | Drossel-Schwabl forest-fire model (1992): on a grid an empty cell grows a tree with chance p, a tree is struck with chance f, a burning tree burns its neighbours and becomes empty; with f much smaller than p and fast burning, fire sizes follow a power law with no tuning (self-organised criticality) | reported | Drossel and Schwabl 1992, PRL 69:1629; Clar, Drossel, Schwabl arXiv cond-mat/9610201 |
| BD-21 | Rothermel (1972): steady surface-fire spread rate = reaction heat flux × (1 + wind factor + slope factor) ÷ (fuel bulk density × effective heating number × heat of ignition); inputs are fuel load, depth, moisture and its extinction value, particle surface to volume, wind and slope; it assumes a quasi-steady line fire in uniform fuel and underlies BEHAVE and FARSITE | reported | USDA Forest Service INT-115 (fs.usda.gov/rm/pubs_int/int_rp115.pdf); firebench.readthedocs.io |
| BD-22 | Rinderpest in Africa from 1887-89: killed 80-90 % of cattle in eastern and southern Africa plus buffalo, giraffe and wildebeest; famine is said to have killed a third of Ethiopians and two thirds of the Maasai; grass turned to thicket, tsetse spread and sleeping sickness followed | reported | rgs.org rinderpest case study; newsarchive.ilri.org; newspaper.animalpeopleforum.org |
| BD-23 | Rinderpest's last field case was Kenya 2001; OIE declared it eradicated on 25 May 2011, the first animal disease and second virus eradicated | reported | cfsph.iastate.edu rinderpest factsheet; ijvm.org.il; DFID impact case study 2010 |
| BD-24 | Foot-and-mouth, UK 2001: about 6 million animals killed, 2,026 infected premises, over by September; cost about £8 billion (£3 billion public, £5 billion private), with tourism losing about as much as farming | reported | wwwnc.cdc.gov/eid/content/9/1/22-t1.htm (NAO figures); en.wikipedia.org 2001 UK FMD outbreak; blog.uvm.edu/jmsmith |
| BD-25 | Ferguson et al. 2001: species mix, herd sizes and the number of separate land parcels per farm explained regional transmission; a full cull of infected and neighbouring farms from 1 April would have cut cases 16 % and animals culled 30 %; the neighbour cull was later disputed (Mansley 2011) | reported | sciencedaily.com 2001-10-04; agris.fao.org record; Tildesley et al. 2009 |
| BD-26 | FMD 2001 spread by a distance kernel between farms: about half of transmission within 3 km once movements were banned; another estimate gives a mean of 8.9 km (contact tracing 6.3 and 11.2 km); straight-line distance predicted risk as well as road distance | reported | Keeling et al. 2001 as described in edepot.wur.nl/553768 (Prev Vet Med 2021); Savill et al. 2006, PMC1395309 |
| BD-27 | Avian influenza H5N1 clade 2.3.4.4b: over 131 million poultry died or were culled in 2022 in 67 countries; about 485 bird species and 37 mammal species infected since 2021 (seals, sea lions, mink, foxes); WOAH warned of wildlife population collapse | reported | woah.org H5N1 2021-22 report; chinadailyhk.com 2023 (WOAH/FAO statement); rr-europe.woah.org 2023 slides |
| BD-28 | H5N1 risk in Thailand and Vietnam tracked duck numbers, people and rice-cropping intensity (two or three crops a year), not chicken numbers | reported | Gilbert et al. 2008, PNAS; modis.gsfc.nasa.gov abstract 01838 |
| BD-29 | African swine fever kills almost every pig it infects; China's herd fell 27.5 % in 2019 officially (38.7 % year on year by August; analysts about 50 %), and pork output fell 21.3 % | reported | thepigsite.com 2019; sg.news.yahoo.com (NBS figures); businessday.co.za (Rabobank) |
| BD-30 | ASF within-herd R0: median 10 (Vietnam, range 1.1-30), 4.4-17.3 across nine Russian herds | reported | aasv.org abstract "within-herd transmission rate for ASF in Vietnam"; PMC5887875 |
| BD-31 | ASF in wild boar moves 2.9-11.7 km per year on its own (EFSA); pooled spread across Europe runs about 200 km per year because people carry it in jumps | reported | EFSA Journal 2020 pub 5996; pigprogress.net; europarl question and answer |
| BD-32 | Livestock and wildlife disease spreads by two terms: a short kernel of a few km (contact, wind, local animals) and rare long jumps of hundreds of km (trade, trucks, people); movement bans cut the second and leave the first | derived | BD-26, BD-31 |
| BD-33 | Chytrid fungus (*Batrachochytrium*): declines in at least 501 amphibian species and 90 presumed extinctions over 50 years, peaking in the 1980s; 12 % show recovery, 39 % still declining; present in over 60 countries | reported | Scheele et al. 2019, Science 363:1459; discovery.ucl.ac.uk/10069937 |
| BD-34 | White-nose syndrome: found in New York about 2006, in at least 40 US states by 2025 (Oregon in September 2025); over 5.5 million bats dead by 2012; little brown bats down 93 % on average, some species over 98 % in the Northeast | reported | npr.org 2012-04-05; eenews.net 2025; biologicaldiversity.org 2013 |
| BD-35 | White-nose syndrome crossed about 4,000 km of continent in about 19 years, about 200 km per year on average, carried by hibernating bats and people | derived | BD-34 |
| BD-36 | Chronic wasting disease: in wild or captive deer in 36 US states (April 2025); wild prevalence 10-25 % in affected areas, up to 79 % in captive herds; prions stay in soil and water for years; in one Saskatchewan area with over 70 % prevalence in male mule deer, 75 % of soil samples were positive | reported | epi.ufl.edu 2025; cdc.gov/chronic-wasting; PMC11210205 |
| BD-37 | Sea star wasting since 2013: over 20 species from Mexico to Alaska; the sunflower star is gone from most of its range, including deep water; marine heat waves sped it up; with the star gone, urchins boomed and kelp fell | reported | Harvell et al. 2019, Science Advances; cals.cornell.edu 2019; news.mongabay.com 2019 |
| BD-38 | Salmon lice from farms: louse mortality of wild juvenile pink salmon was 16 % to over 97 %, commonly above 80 %; the authors projected a 99 % collapse in four generations if outbreaks continued; the projection was disputed | reported | Krkosek et al. 2007, Science 318:1772, PubMed 18079401; Riddell et al. 2008 comment |
| BD-39 | Saiga 2015: about 200,000 died in about three weeks, over 60 % of the species, from *Pasteurella multocida* B, a bacterium they normally carry, triggered by humid warm weather at calving; similar die-offs: 270,000 in 1998, 12,000 of a 26,000 herd in 2010 | reported | Kock et al. / Fereidouni et al., Emerging Infectious Diseases 2019, PMC6537709 |
| BD-40 | 727 mass mortality events (1940-2012) across 2,407 animal populations: disease caused 26 %; one event can kill over 90 % of a population; events grew in size for birds, fish and marine invertebrates | reported | Fey et al. 2015, PNAS 112:1083, PubMed 25583498 |
| BD-41 | 335 emerging disease events (1940-2004): 60.3 % zoonotic, 71.8 % of those from wildlife; hotspots match wildlife richness and human density and growth | reported | Jones et al. 2008, Nature 451:990; [WSE-29](world-systems-ecology.md) |
| BD-42 | Spillover passes a chain of ordered barriers (Plowright et al. 2017): reservoir density and infection share, pathogen release and survival outside the host, human exposure, then the human's own susceptibility; the chance is the product, so any one closed barrier stops it | reported | Plowright et al. 2017, Nat Rev Microbiol 15:502, PMC5791534 |
| BD-43 | In disturbed land (secondary, farm, urban) known zoonotic hosts make up 18-72 % more of local species and 21-144 % more of individuals than in undisturbed land nearby; strongest for rodents, bats and songbirds | reported | Gibb et al. 2020, Nature 584:398 |
| BD-44 | In a multi-host model, spillover risk peaks at intermediate habitat loss; the largest but rarest epidemics come at the extremes | reported | Faust et al. 2018, Ecology Letters 21:471, eprints.gla.ac.uk/153361 |
| BD-45 | Global water erosion was about 35.9 Pg of soil a year in 2012, rising mainly with cropland expansion, worst in Sub-Saharan Africa, South America and Southeast Asia; tolerable loss is about 1 t per ha per year and ploughing runs 10-100 times faster ([WSE-22](world-systems-ecology.md), [WSE-23](world-systems-ecology.md)) | reported | Borrelli et al. 2017, Nat Commun 8:2013, PMC5722879 |
| BD-46 | GLASOD (1990-91): about 15 % of ice-free land degraded by people; water erosion 56 % of that, wind 28 %; the commonly cited totals are 1,964 Mha, with chemical (salt, nutrient loss, pollution) 12 % and physical (compaction, waterlogging) 4 % (totals not confirmed in the summaries) | reported | Oldeman, Hakkeling, Sombroek 1991, ISRIC; soilerosion.net/extent_of_erosion.html |
| BD-47 | Salt-affected soils cover 1,381 Mha, 10.7 % of land (FAO 2024), with about 1 billion ha more at risk; about 10 % of irrigated and of rainfed cropland is affected; salt can cut rice or bean yields by up to 70 % in the worst countries | reported | fao.org newsroom "first major global assessment of salt-affected soils in 50 years" (2024) |
| BD-48 | Plant-parasitic nematodes cost about $78-100 billion a year worldwide, 10-15 % of yield; soybean cyst nematode alone over $1 billion a year in the US and up to 30 % loss with no visible symptom; it persists in soil for years and is slowly breaking crop resistance | reported | uaex.uada.edu 2026-06-24; intechopen.com/chapters/53722 |
| BD-49 | Caesium-137, the long-lived contaminant of nuclear accidents, has a half-life of about 30 years, so a contaminated field holds 10 % of its load after about 100 years | reported | standard nuclide data (NNDC), half-life 30.1 years |
| BD-50 | Soil-borne agents outlast a human generation: TR4 about 30 years, CWD prions years, nematode cysts years, Cs-137 a 30-year half-life; a field lost to them stays lost to that crop for decades | derived | BD-6, BD-36, BD-48, BD-49 |
| BD-51 | At 12 t per ha per year about 1 mm of soil goes ([WSE-23](world-systems-ecology.md)); ploughing at 10-100 t per ha per year ([WSE-22](world-systems-ecology.md)) removes about 1-8 mm a year, so a 30 cm topsoil lasts about 40-400 years | derived | WSE-22, WSE-23 |
| BD-52 | Measles across England and Wales (Xia, Bjørnstad, Grenfell 2004) fits a gravity coupling: flow between towns ∝ N_i^τ1 × N_j^τ2 ÷ d^ρ with ρ = 1, τ1 = 1, τ2 = 1.5; a distance-only kernel fitted poorly | reported | Xia et al. 2004, Am Nat 164:267; ento.psu.edu gravity.pdf |
| BD-53 | On a lattice, the mean-field (well-mixed) SIR ignores that an infected cell's neighbours are already infected, so it spreads too fast; the pair approximation (Keeling 1999) tracks lattice runs much better, and triplet closures better still | reported | Joo and Lebowitz 2004, arXiv q-bio/0404035; Mollison, Warwick talk |
| BD-54 | At our cell sizes a 3-9 km kernel (BD-26) lies inside one 70 km Macro cell and crosses about 15-45 cells of 200 m; so at Macro, local livestock spread is a within-cell rate and only long jumps cross cells, while at City it is a neighbour rule | derived | BD-26; [ZL-5](zoom-and-layer-two.md) |
| BD-55 | One host-pathogen model fits all living stocks: per cell and per host stock, compartments S, I, R and dead (E where latency matters), with R0, latent and infectious days, mortality, and persistence outside the host taken from a pathogen table | estimate | BD-20, BD-30, BD-53; [spec 38](../tier0/specs/38-outbreak.md) |
| BD-56 | Spread between cells needs three terms: a short neighbour kernel (contact, km), a wind kernel with a fat tail (tens to hundreds of km, for spores), and movement jumps along trade and travel flows by a gravity rule, which bans and culls can cut | estimate | BD-10, BD-11, BD-26, BD-31, BD-32, BD-52 |
| BD-57 | Soil health is a stock per cell, not a host: fertility, topsoil depth, salt, contamination and soil-borne pathogen load, each with its own inflow and slow decay, multiplying crop yield | estimate | BD-45 to BD-51; [WSE-37](world-systems-ecology.md) |
| BD-58 | The links of 0059 become multipliers with real anchors: cleared forest raises zoonotic host share and spillover (peak at intermediate loss), livestock density raises within-herd R, a failed harvest lowers people's resistance, an abandoned field becomes a pest source, losing grazers turns grass to bush | estimate | BD-22, BD-25, BD-28, BD-42, BD-43, BD-44 |
| BD-59 | Unwatched regions can run each host-pathogen pair as a front with a measured speed and a per-cell aggregate (pair-corrected SIR), and watched regions as the cell rule; the two must agree in distribution, as in spec 38 | estimate | BD-11, BD-53; [spec 38](../tier0/specs/38-outbreak.md) |
| BD-60 | Test targets from history: a chestnut-like blight front at 30-80 km per year with spot jumps, ASF-like boar spread at 3-12 km per year without people, an FMD-like kernel with half of spread inside 3 km, monoculture severity about 10 times a mixture's | estimate | BD-8, BD-13, BD-26, BD-31 |

## 1. Crops

Crop disease is the oldest famine maker. Late blight took the Irish potato (BD-1), brown spot is one of the causes argued for Bengal (BD-2), and a new rust race can take most of a field (BD-3). The usual, quiet loss is already a fifth to a third of the harvest every year (BD-7), so a sick year is a shift from that baseline, not from zero.

Two lessons carry into the model. First, speed: wind-borne rusts move hundreds of km a year and cross seas (BD-4, BD-10), soil-borne wilts move only with plants, soil and water but never leave (BD-6), and climate moves the whole range slowly (BD-9). Second, uniformity: every great loss here hit a crop grown as one genotype over a huge area (BD-5, BD-12), and mixing varieties cut severity by about 94 % in a real trial (BD-8). A region that plants one crop should be fragile in a measurable way.

## 2. Forests

Forest killers act over decades. Chestnut blight removed a dominant tree from a continent in about 40 years (BD-13); Dutch elm disease came in two waves 40 years apart (BD-14); ash dieback is crossing the UK at about 5 % of the country a year (BD-16); a beetle took over half the merchantable pine of one province (BD-17). Recovery is not the forest regrowth curve of WSE-35 when the pathogen stays: the host does not come back and other species take the ground (BD-19).

Fire is the other spatial killer. Drossel-Schwabl (BD-20) is the classic grid rule (grow, strike, burn neighbours) and gives realistic fire sizes without tuning; Rothermel (BD-21) gives the real speed from fuel, moisture, wind and slope. A cell rule can use the first for what burns and the second for how fast.

## 3. Livestock

Livestock plagues are fast and heavy. Rinderpest killed most of Africa's cattle and set off famine and an ecological chain (BD-22) before it was eradicated (BD-23). ASF kills nearly every pig it infects and halved China's herd in a year by some counts (BD-29); its within-herd R0 is around 10 (BD-30). H5N1 has cost hundreds of millions of poultry and jumped to wild birds and mammals (BD-27). FMD kills few animals but the response (culls, movement bans) cost the UK about £8 billion and tourism as much as farming (BD-24).

The 2001 FMD work gives the spatial model directly: a distance kernel between farms with most spread inside a few km (BD-26), risk raised by species mix, herd size and scattered land (BD-25). ASF shows the second term: its own front is slow, people make it fast (BD-31). So livestock spread is a short kernel plus long jumps, and policy acts on the jumps (BD-32).

## 4. Wildlife and fish

Wild hosts suffer the largest losses with nobody to cull or vaccinate. Chytrid is the largest disease-driven loss of species recorded (BD-33); white-nose syndrome cut bat colonies by over 90 % in a decade (BD-34, BD-35); CWD builds up in soil and keeps reinfecting (BD-36); sea star wasting removed a predator and the kelp went with it (BD-37); saiga lost over 60 % of the species in three weeks to a bacterium they normally carry, set off by weather (BD-39). Mass mortality events are common, a quarter of them disease (BD-40). Farms feed wild disease too: salmon lice from net pens (BD-38), H5N1 between poultry and wild birds (BD-27, BD-28).

Spillover to people needs a chain of barriers to open together (BD-42). Land-use change opens them: disturbed land holds more of the hosts that carry human pathogens (BD-43), and risk peaks when habitat is partly cleared, not fully (BD-44). The hotspots of BD-41 sit where wildlife is rich and people are many and growing.

## 5. Soil

Soil does not catch a disease; it wears out. Erosion is the largest term (BD-45, WSE-22, WSE-23): ploughed land can lose its topsoil in a few centuries or less (BD-51). Salt affects a tenth of land and of cropland (BD-47). GLASOD put human-made degradation at about 15 % of land, mostly erosion (BD-46). Soil also holds the agents that never leave: nematodes (BD-48), wilts (BD-6), prions (BD-36), radioactive contamination (BD-49). These set the slowest clock in the world (BD-50).

## 6. Spatial models

Three model families cover what we need.

- **Lattice and cellular-automaton SIR.** Each cell holds a state or compartment shares; infection passes to neighbours with a chance from the local rate. It is the natural form for fire (BD-20), blight fronts and boar ASF. Its well-mixed aggregate overstates speed; the pair approximation fixes most of that (BD-53).
- **Kernel models.** Each infected unit infects others with a rate that falls with distance (BD-26). A fat-tailed kernel reproduces spot infections far ahead of the front (BD-13, BD-15).
- **Metapopulations with gravity coupling.** Places are well-mixed pools linked by flows that grow with their sizes and fall with distance (BD-52). This is the form for trade and travel jumps (BD-31, BD-32) and fits our Macro cells.

Running coarse where unwatched follows the jump rule of spec 40: inside a 70 km Macro cell local spread is a rate, not a neighbour rule (BD-54); between Macro cells a front advances at a speed measured from the fine rule, and long jumps are drawn on the gravity flows.

## What this means for us (Proposal)

All of this is `estimate`, tied to the findings by ID.

**One host-pathogen model (BD-55).** A pathogen is a row in a data table: host stocks it infects, R0 (or a transmission rate per contact), latent and infectious days, mortality, recovery and immunity, persistence outside the host (days to decades, BD-50), and spread modes with their scales. For each cell and each host stock (a crop field, a forest stand, a herd, a wild population, a fish stock: the spec 37 populations) we keep compartment shares S, E, I, R and a dead flow. Deaths leave the spec 37 stock as a declared outflow with a cause, so the ledger stays exact. Up close one animal or tree draws its state from the shares, as a wound in spec 21 (0059 item 5). The aggregate day reuses spec 38's tau-leap with keyed binomial draws.

Starter rows, all tagged as analogues of real diseases: a wind rust (crops, front hundreds of km a year, BD-4), a soil wilt (crops, persists 30 years, BD-6), a forest blight (front 30-80 km a year with spot jumps, mortality near 1 in the host, BD-13), a swine fever (R0 about 10, mortality near 1, boar front 3-12 km a year, BD-29 to BD-31), a hoof disease (low mortality, 3 km kernel, BD-26), a bird flu (poultry and wild birds, risk from ducks and wet rice, BD-27, BD-28), an amphibian fungus and a bat fungus (wild, BD-33, BD-34), a prion (deer, soil reservoir, BD-36), a fish louse (farm to wild, BD-38).

**Spread between cells (BD-56).** Three terms: neighbour contact on the grid (cellular automaton, City cells); a wind kernel with a fat tail for spores and insects; movement jumps on trade, herd and people flows by gravity weights. A movement ban zeroes the third for the banned goods; a cull empties a cell's host stock (a transfer to waste, with its cost). The 0059 "abandoned farms let pests spread" is a field whose host stock is no longer managed, so its infected share is not cleared and it seeds its neighbours.

**Watched and unwatched (BD-59).** Watched cells step the grid rule daily. Unwatched Macro cells hold one pair-corrected SIR per host-pathogen pair and a front position; the front advances at the speed measured from the grid rule for that pathogen and terrain. A test (as in spec 38) shows the two give the same distribution of arrival times and losses.

**Soil health as a stock (BD-57).** Per cell: fertility 0-1 (WSE-37), topsoil depth in mm (erosion from slope, rain, cover and tillage, BD-51), salt 0-1 (irrigation without drainage raises it, BD-47), contamination per agent with its half-life (BD-49), soil-borne pathogen load per agent with its decay (BD-6, BD-48). Crop yield = potential × fertility × water × (1 − pest loss) as in WSE-37, with depth, salt and soil pathogen as extra factors. Recovery runs at decay rates, so a ruined field stays ruined for a generation.

**Links (BD-58).** As multipliers on rates, each with its anchor:
- forest share cleared → share of zoonotic hosts up 20-140 % (BD-43) and human exposure, peaking at intermediate clearing (BD-44);
- livestock density and species mix → within-herd and between-farm transmission (BD-25, BD-30);
- monoculture share of a crop in a region → severity, about 10 times a mixture's (BD-8, BD-12);
- harvest shortfall → people's nutrition → susceptibility in spec 21 (BD-1, BD-2: most famine deaths were disease);
- grazer collapse → grass to bush → new vector habitat (BD-22);
- heat → faster marine and wild die-offs (BD-37, BD-39).

**Zombie pathogen in animals (0059 item 6).** None by default: animal stocks never get a Z compartment. Carriers: scavenger stocks that feed on the dead gain a "contaminated" share that moves on the movement and neighbour terms and raises human exposure where they go, with no turning. Animals turn: chosen host stocks get spec 38's S, E, Z, R with rates measured from their own encounter rule; an animal's speed enters the encounter rule like a runner's.

**Tests.** (1) Single cell: final epidemic size matches the closed-form SIR relation for the given R0. (2) Grid front speed for a test pathogen within a tolerance of the speed used by the unwatched form; arrival-time distributions agree (A2, spec 38). (3) History replays as targets (BD-60): blight front 30-80 km a year with jumps, boar ASF 3-12 km a year with no people, FMD-like half of spread inside 3 km, mixtures cutting severity about tenfold. (4) Ledger: every death, cull and harvest loss is a transfer or a declared flow with a cause (spec 37). (5) Soil: a ploughed slope loses depth at 1-8 mm a year and a fallow field regains fertility at the WSE-37 rate; a field seeded with a 30-year agent is unfit for its host for that time. (6) Option: with the zombie option at "none", no animal stock ever holds Z.

## Open questions

1. Which pathogens ship at world creation, and are they named real diseases or fictional analogues with real rates?
2. How many host-pathogen pairs can run per Macro cell before the aggregate step costs too much? Do we cap at a few active outbreaks per region?
3. Does each host stock get its own front, or is there one front per pathogen across all hosts it infects?
4. How is the wind kernel tied to the weather fields of spec 36: a fixed fat tail, or driven by the day's wind?
5. Do people respond on their own (cull, ban movement, burn fields) by a rule, or only through faction decisions?
6. How does evolution enter: new races that break resistance (BD-3, BD-5, BD-48) as rare events, or not at all?
7. What does the player see: a sick share per cell overlay, a front line, alerts when a stock passes a loss threshold?
8. Do soil-borne agents travel with soil moved downhill and in rivers (0059 item 7), or only with plants and animals?
9. In the "carriers" option, which species scavenge, and how long does contaminated material stay infectious?

## Sources

- encyclopedia.com/history/modern-europe/british-and-irish-history/irish-famine
- heritage.galwaycommunityheritage.org (The Famine in Ireland)
- environmentandsociety.org/node/1908; gktoday.in/bengal-famine-of-1943; phytopathdb.org/content/bipolaris-oryzae
- gd.eppo.int/reporting/article-770; undark.org/2018/01/08/ug99-wheat-stem-rust-1; farmprogress.com (Ug99)
- par.nsf.gov (Pataky); agweb.com/news/crops/crop-production/when-corn-died; ntrs.nasa.gov/citations/19720021767
- abgc.org.au/2019/04/26/tr4-around-the-world-ongoing-spread-and-impact; promusa.org/Tropical+race+4+-+TR4; bdspublishing.com TR4 chapter
- Zhu Y. et al. 2000, Genetic diversity and disease control in rice, Nature 406:718-722; edepot.wur.nl/704959
- Bebber D. et al. 2013, Nature Climate Change 3:985-988; carbonbrief.org/pests-moving-polewards-threaten-global-food-security
- Brown J.K.M., Hovmøller M.S. 2002, Science 297:537-541; pmc.ncbi.nlm.nih.gov/articles/PMC5222527
- ebsco.com/research-starters/consumer-health/chestnut-blight; site.extension.uga.edu (American chestnut); npn.rngr.net (1978, devastation of American chestnut by blight)
- forestresearch.gov.uk (Dutch elm disease); en.wikipedia.org/wiki/Dutch_elm_disease; salisburygreenhouse.com
- cdn.forestresearch.gov.uk Chalara situation report Nov 2024; collections.etoncollege.com/ash-die-back; uwfs.org.uk
- natural-resources.canada.ca/forest-forestry/insects-disturbances/mountain-pine-beetle; opentextbc.ca/geography/chapter/7-6-case-study
- Drossel B., Schwabl F. 1992, PRL 69:1629; Clar S., Drossel B., Schwabl F., arXiv cond-mat/9610201
- Rothermel R.C. 1972, USDA Forest Service INT-115, fs.usda.gov/rm/pubs_int/int_rp115.pdf; firebench.readthedocs.io Rothermel page
- rgs.org rinderpest vaccine case study; newsarchive.ilri.org; cfsph.iastate.edu/Factsheets/pdfs/rinderpest.pdf; DFID rinderpest impact case study 2010
- wwwnc.cdc.gov/eid/content/9/1/22-t1.htm; en.wikipedia.org/wiki/2001_United_Kingdom_foot-and-mouth_outbreak; blog.uvm.edu/jmsmith/?p=625
- Ferguson N., Donnelly C., Anderson R. 2001, Nature 413:542; sciencedaily.com/releases/2001/10/011004065655.htm; Tildesley et al. 2009 Proc R Soc B
- Keeling M. et al. 2001, Science 294:813, as described in edepot.wur.nl/553768; Savill N. et al. 2006, BMC Vet Res, PMC1395309
- woah.org H5N1 report winter 2021-2022; rr-europe.woah.org 2023 HPAI slides; chinadailyhk.com/article/340595
- Gilbert M. et al. 2008, PNAS, Mapping H5N1 risk in Southeast Asia; modis.gsfc.nasa.gov abstract 01838
- thepigsite.com 2019 China herd reports; sg.news.yahoo.com (China 2019 pork output); businessday.co.za 2019-04-14
- aasv.org (ASF within-herd rate, Vietnam); PMC5887875; efsa.europa.eu EFSA Journal pub 5996; pigprogress.net
- Scheele B. et al. 2019, Science 363:1459; discovery.ucl.ac.uk/id/eprint/10069937
- npr.org 2012/04/05/150000682; eenews.net (Oregon 2025); biologicaldiversity.org 2013-02-28
- epi.ufl.edu 2025-10-07; cdc.gov/chronic-wasting/data-research; PMC11210205
- Harvell C.D. et al. 2019, Science Advances; cals.cornell.edu 2019; news.mongabay.com 2019/01
- Krkosek M. et al. 2007, Science 318:1772, pubmed 18079401; Riddell B. et al. 2008 comment
- Emerging Infectious Diseases 2019, Mass die-off of saiga antelopes, Kazakhstan, 2015, PMC6537709
- Fey S. et al. 2015, PNAS 112:1083, pubmed 25583498
- Jones K. et al. 2008, Nature 451:990; ideas.repec.org record
- Plowright R. et al. 2017, Nature Reviews Microbiology 15:502, PMC5791534
- Gibb R. et al. 2020, Nature 584:398, doi 10.1038/s41586-020-2562-8
- Faust C. et al. 2018, Ecology Letters 21:471, eprints.gla.ac.uk/153361
- Borrelli P. et al. 2017, Nature Communications 8:2013, PMC5722879
- Oldeman L.R., Hakkeling R., Sombroek W. 1991, World map of the status of human-induced soil degradation, ISRIC/UNEP; soilerosion.net/extent_of_erosion.html; fao.org/4/v4360e/V4360E05.htm
- fao.org newsroom, FAO launches first major global assessment of salt-affected soils in 50 years (2024)
- uaex.uada.edu 2026-06-24 (soybean cyst nematode); intechopen.com/chapters/53722
- NNDC nuclear data (Cs-137 half-life)
- Xia Y., Bjørnstad O., Grenfell B. 2004, American Naturalist 164:267; ento.psu.edu gravity.pdf
- Joo J., Lebowitz J. 2004, arXiv q-bio/0404035; Keeling M. 1999, Proc R Soc B (pair approximation)
- [World systems ecology](world-systems-ecology.md) (WSE-1, WSE-22, WSE-23, WSE-27, WSE-29, WSE-35, WSE-37); [zoom and layer two](zoom-and-layer-two.md) (ZL-5)
