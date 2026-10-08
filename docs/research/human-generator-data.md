# The human generator: data, licences and the generation pipeline

**Date:** 2026-10-07. **Status:** research note, not a decision. **Feeds:** [world-simulation](../design/world-simulation.md), [world-numbers](../design/world-numbers.md), [spec 22](../tier0/specs/22-census.md), [spec 02](../tier0/specs/02-draw.md), [0045](../decisions/0045-open-questions-answered-2.md), [0046](../decisions/0046-realistic-numbers-from-free-data.md). **Evidence:** licence quotes are from the licence pages fetched on 2026-10-07; "verify" marks a page that could not be read directly; sources marked (S) are secondary and (W) weak; no lawyer has reviewed any of it. **IDs:** HG.

Follows [pop-simulation-and-persistence.md](pop-simulation-and-persistence.md) §4, which found the method research epidemic models use to build synthetic populations (iterative proportional fitting over census marginals, then sampling whole households from microdata) but not the data side. This note answers: which datasets can a commercial game legally ship, how to turn them into a pure function (seed, cell, slot) → household that hits the cell's integer totals exactly, and how lifepaths get real rates. It extends the licence list in [world-simulation §3](../design/world-simulation.md#3-population-proposal).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| HG-1 | IPUMS International forbids commercial use and redistribution to third parties; fitting our tables from it would itself be commercial use. | sourced | https://international.ipums.org/international/terms.shtml |
| HG-2 | US ACS PUMS household and person microdata is US federal public domain (attribution requested): shippable raw, areas of about 100k people. | sourced | https://www.census.gov/programs-surveys/acs/microdata.html |
| HG-3 | Statistics Canada's Open Licence allows selling the information and names public use microdata files: the 2021 PUMF is shippable raw. | sourced | https://www.statcan.gc.ca/en/reference/licence |
| HG-4 | Mexico's INEGI allows commercial exploitation and redistribution with credit "Fuente: INEGI": Censo 2020 microdata is shippable raw. | sourced | https://www.inegi.org.mx/inegi/terminos.html |
| HG-5 | The UK ONS 1 % teaching sample is under the Open Government Licence (shippable); the safeguarded 5 % and 1 % files are not-for-profit only. | sourced | https://www.ons.gov.uk/census/aboutcensus/censusproducts/microdatasamples |
| HG-6 | Argentina INDEC is CC BY-SA 4.0 (derived tables only, ShareAlike may attach); Brazil IBGE Censo 2022 microdata has no explicit licence. | sourced | https://www.indec.gob.ar/indec/web/Institucional-Indec-Normativa |
| HG-7 | WorldPop age and sex rasters, GHSL settlement layers, ILOSTAT, O*NET 31.0 and UN migrant stock 2024 are CC BY (shippable raw); OECD data is CC BY 4.0 from July 2024. | sourced | https://hub.worldpop.org/project/categories?id=8 |
| HG-8 | UN DESA household size and marriage data: reports say CC BY 3.0 IGO but the data pages link generic non-commercial UN terms; needs confirmation. | sourced | https://www.un.org/development/desa/pd/data/household-size-and-composition |
| HG-9 | Prem et al. 2021 contact matrices (177 countries): paper CC BY 4.0, repository GPL-3.0, so numbers must be re-derived from the paper's data; Mistry et al. 2021 (35 countries, per setting) is Apache 2.0. | sourced | https://pmc.ncbi.nlm.nih.gov/articles/PMC8354454/ |
| HG-10 | Health is the gap: IHME Global Burden of Disease and WHO Global Health Observatory are non-commercial; NHANES (no restrictions) is the only clean source of condition rates by age and sex. | sourced | https://www.cdc.gov/nchs/data/nhanes/nhanes_release_policy.pdf |
| HG-11 | DHS surveys, Global Data Lab, RTI SynthPop and FRED populations are non-commercial or non-redistributable: not shippable. | sourced | https://dhsprogram.com/data/terms-of-use.cfm |
| HG-12 | Name sources: SSA baby names (CC0) and Wikidata (CC0, biased to notable people) are clean; INSEE is expected open (verify); names-dataset on PyPI has leaked Facebook provenance and is excluded. | sourced | https://www.wikidata.org/wiki/Wikidata:Licensing |
| HG-13 | MakeHuman/MPFB core assets are CC0 (usable); its code is GPL/AGPL and must never be linked. | sourced | https://static.makehumancommunity.org/about/license.html |
| HG-14 | SynthPops and Covasim are MIT: their household-building algorithms can be ported freely; eqasim and MATSim are GPL and must be reimplemented. | sourced | https://github.com/InstituteforDiseaseModeling/synthpops |
| HG-15 | Where no microdata ships, the "hybrid" route builds households from shippable marginals: size from a UN distribution, reference person's age given size, other members from the home contact-matrix row, relationships labelled by age gaps. | sourced | https://docs.idmod.org/projects/synthpops/en/v1.5.2/households.html |
| HG-16 | Iterative proportional fitting matches household or person marginals, not both; iterative proportional updating (Ye, Konduri, Pendyala 2009, PopGen) matches both by reweighting household records. | sourced | https://scag.ca.gov/sites/main/files/file-attachments/populationsynthesizerpaper_trb.pdf |
| HG-17 | Integerise fitted weights per cell with the largest-remainder (Hamilton) rule, not Lovelace and Ballas's random tail, so the build reproduces. | estimate | this note |
| HG-18 | Ship per cell the household and person totals and the integer count per archetype; slot k maps to its archetype by binary search on cumulative counts, so totals are exact without generating anyone. | estimate | this note |
| HG-19 | Per-slot attributes are keyed Philox draws (key = hash(seed, cell), counter = (slot, member, attribute)), so any slot can be generated in any order on any thread. | sourced | https://users.cs.utah.edu/~hari/teaching/bigdata/random123sc11.pdf |
| HG-20 | A census cell is a group of WorldPop/GHSL grid cells of roughly 500 to 5,000 people nested in a GeoNames administrative unit, rescaled to UN national totals. | estimate | this note |
| HG-21 | Face genes as a fixed-length vector of MakeHuman target weights, each child gene taken from either parent plus a small mutation (the Crusader Kings 3 pattern), makes families resemble each other. | reported | https://primagames.com/?p=313090 |
| HG-22 | Lifepaths: each domain is a time-inhomogeneous Markov chain in age (as in MicSim), sampled backwards from the known present; a Traveller-style term is a four-year step. | estimate | https://rdrr.io/cran/MicSim/man/MicSim-package.html |
| HG-23 | Lifepath constraints: no job before school-leaving age, attainment consistent with cohort, union age near the singulate mean age at marriage, a service term only where the CIA Factbook says conscription applies. | estimate | this note |
| HG-24 | Health rates must be labelled as calibrated on US data (NHANES), scaled by each country's mortality. | estimate | this note |
| HG-25 | No shippable microdata exists for most of Africa, Asia, the Middle East, Eastern Europe and most of Latin America: lower fidelity on multi-family households and relationships there. | estimate | this note |
| HG-26 | Name tables have large gaps for China, India, the Arab world and Africa. | estimate | this note |

## 1. Answer in one page

1. **The worldwide census microdata store is off limits.** IPUMS International's terms: "Commercial use is strictly prohibited"; "redistribution of the data to third parties is prohibited". Fitting our tables from it would itself be commercial use. Either never touch it, or get written permission from IPUMS and each national agency for fitted joint distributions. Legal review, not a forum answer.
2. **Raw household microdata is shippable for a handful of countries:** United States (ACS PUMS, federal public domain), Canada (Statistics Canada Open Licence, which names "public use microdata files" and allows "sell"), Mexico (INEGI "Puede explotar comercialmente la información"), the United Kingdom's 1 % teaching sample (Open Government Licence), and some Eurostat public-use files (seven countries, licence per file to verify). Argentina is CC BY-SA (derived tables only, with care), Brazil has no explicit licence.
3. **Everywhere else, the population is built from shippable marginals plus synthetic household archetypes.** Age and sex per grid cell from WorldPop (CC BY 4.0) scaled to UN World Population Prospects; household size from the UN household database; home-setting age mixing from Mistry et al. (Apache 2.0) or Prem et al. (numbers from a CC BY paper, not the GPL repository); jobs from ILOSTAT (CC BY 4.0) and O*NET (CC BY 4.0); schooling from UNESCO (CC BY-SA, so derived tables); marriage and migration from UN DESA. This is the SynthPops and Covasim "hybrid" route, lower fidelity on relationships but legally clean.
4. **Health data is the gap.** The two global morbidity sources are non-commercial (IHME Global Burden of Disease, WHO Global Health Observatory). NHANES (United States, no restrictions) is the only clean source for condition rates by age and sex; the game must say its health rates are calibrated on US data, scaled by each country's mortality.
5. **Fit offline with iterative proportional updating, not only fitting.** Iterative proportional fitting matches household marginals or person marginals, not both. Iterative proportional updating (Ye, Konduri, Pendyala 2009, the PopGen algorithm) puts the weight on the household record and constrains the people inside, so a cell can hold exactly 412 households containing exactly 1,243 people with the right age pyramid.
6. **Conservation by construction, not by sampling.** Integerise the fitted weights per cell with the largest-remainder rule, ship the integer count per archetype, and map slot k to its archetype by binary search on the cumulative counts. The sum over slots reproduces the cell's totals exactly without generating anyone. Every attribute below the archetype is a keyed draw (Philox, key = hash(seed, cell), counter = (slot, member, attribute)), so any slot can be generated in any order on any thread.
7. **Lifepaths are a Markov chain in age sampled backwards from the present**, with transition rates from the same open sources (enrolment and attainment, employment by age and occupation, job tenure, marriage age, fertility, conscription from the CIA World Factbook, migration from UN stock data). Constraints: no job before the country's school-leaving age, children's ages consistent with the household, service terms only where conscription applies to that sex and cohort.

## 2. Licence table

"Ship" means the data, raw or as tables derived from it, inside a commercial game.

| Dataset | Licence (quoted or cited) | Ship? |
| --- | --- | --- |
| IPUMS International (census microdata, 100+ countries) | "Commercial use is strictly prohibited"; "redistribution of the data to third parties is prohibited"; staff on the forum allow sharing "descriptive statistics, aggregate data, or analytical results" for research users | **No.** Derived tables need written permission (legal review) |
| US ACS PUMS (Census Bureau) | US federal work, public domain; attribution requested | **Yes, raw** (household and person records with weights, areas of about 100k people) |
| Statistics Canada PUMF (2021) | Open Licence: "use, reproduce, publish, freely distribute, or sell the Information", which "includes public use microdata files"; attribution required | **Yes, raw** |
| Mexico INEGI (Censo 2020 extended questionnaire microdata) | "Puede explotar comercialmente la información"; "hacer y distribuir copias"; credit "Fuente: INEGI" and note transformations | **Yes, raw** |
| UK ONS Census 2021 | 1 % teaching sample under the Open Government Licence; safeguarded 5 % and 1 % files under the UK Data Service licence, "only for not-for-profit research or teaching" | Teaching sample **yes, raw**; safeguarded files **no** |
| Argentina INDEC | Creative Commons Attribution-ShareAlike 4.0 | Derived tables only; ShareAlike may attach (legal review) |
| Brazil IBGE (Censo 2022 sample microdata, released August 2026) | Public FTP, no explicit licence found | Derived tables; confirm with IBGE |
| Eurostat microdata | Scientific-use files only for recognised research entities; public-use files for seven countries without registration | Scientific-use **no**; public-use files: verify each; aggregate tables **yes** |
| Australia ABS | TableBuilder output "licensed under Creative Commons"; microdata "must not be shared" | Cross-tabs **yes**; microdata **no** |
| UN World Population Prospects 2024 | CC BY 3.0 IGO (stated on publications; verify on the download page) | **Yes, raw** |
| UN DESA household size and composition; World Marriage Data | Methodology reports say CC BY 3.0 IGO; the data pages link the generic UN terms ("personal, non-commercial use") | Likely yes; get written confirmation from the Population Division |
| UN DESA International Migrant Stock 2024 | "Creative Commons license (CC BY 3.0 IGO)" | **Yes, raw** |
| Prem et al. 2021 contact matrices (177 countries) | Paper CC BY 4.0; repository GPL-3.0 | Numbers **yes**, re-derived from the paper's data, not copied from the repository |
| Mistry et al. 2021 mixing patterns (35 countries, per setting) | Apache 2.0 | **Yes, raw** |
| DHS surveys | "will not be re-distributed, either directly or within any tool/dashboard" | **No**; published summary tables only (verify) |
| WorldPop age and sex structures | CC BY 4.0 | **Yes, raw** |
| GHSL settlement layers | CC BY 4.0 | **Yes, raw** |
| Global Data Lab (subnational indicators) | "noncommercial purposes" | **No** |
| ILOSTAT (employment by occupation, sector, age, sex) | "© ILO under CC BY 4.0" | **Yes, raw** |
| O*NET 31.0 | CC BY 4.0 with a required attribution text and trademark notice | **Yes, raw** |
| OECD (job tenure, education) | CC BY 4.0 for data and content from July 2024 | **Yes, raw** (post-July-2024 vintages) |
| UNESCO UIS (enrolment, attainment) | CC BY-SA 4.0 | Derived tables, kept separable, or the World Bank EdStats mirror (CC BY 4.0, verify) |
| WHO Global Health Observatory | CC BY-NC-SA 3.0 IGO; "shall not be used for or in conjunction with the promotion of a commercial enterprise" | **No** |
| IHME Global Burden of Disease | Non-commercial user agreement; commercial use is paid | **No** |
| NHANES (US health and body measurements) | "The Public Use File has no restrictions on access" | **Yes, raw** |
| CIA World Factbook (military service age and obligation) | Public domain | **Yes, raw** |
| SSA baby names (US) | CC0 on data.gov | **Yes, raw** |
| INSEE first names 1900–2025 (France) | INSEE open data (Licence Ouverte expected; verify) | Yes, verify |
| IBGE names API (Brazil), INE names (Spain) | No explicit licence found | Verify |
| Wikidata given and family names | CC0 | **Yes, raw** (biased toward notable people) |
| names-dataset (PyPI) | MIT code, data "extracted from the Facebook massive dump" | **No** (leaked provenance) |
| MakeHuman / MPFB base mesh and targets | Core assets CC0; code GPL and AGPL | Assets **yes**; never link the code |
| SynthPops, Covasim (code) | MIT | Port the algorithms freely |
| RTI SynthPop, FRED populations | "free for non-commercial use" | **No**; regenerate from PUMS ourselves |
| eqasim and MATSim synthesis pipelines | GPL | Do not link; reimplement |

## 3. The pipeline

### 3.1 Offline, on the build machine

1. **Cells.** A census cell is a group of WorldPop or GHSL grid cells of roughly 500 to 5,000 people, nested in a GeoNames administrative unit. Cell population from WorldPop, age and sex from the WorldPop rasters, rescaled to the UN national totals.
2. **Marginals per cell** (all shippable): age × sex; household size distribution and composition; marital status by age and sex; employment by ISCO-08 major group and ISIC sector by age and sex; enrolment by age; attainment by cohort; migrant stock by origin; military obligation.
3. **Seed households (archetypes).** Where raw microdata is shippable, real household records. Elsewhere, synthetic archetypes the SynthPops way: draw the household size from the UN distribution; draw the reference person's age conditional on size ("if no data, the age of the reference person is sampled from the age distribution of adults"); fill the other members from the home-setting row of the contact matrix for the reference person's age; label relationships (partner, child, parent) by age gaps. OpenABM-Covid19 does the same by rejection sampling "to match the aggregate statistics for the age demographics and household size".
4. **Fit weights by iterative proportional updating.** Fitting (Beckman 1996) matches one level; updating (Ye, Konduri, Pendyala 2009; PopGen; the pyipu port; Müller and Axhausen's hierarchical fitting at ETH) "iteratively adjusts and reallocates weights among households of a certain type until both household and person-level attributes are matched". The R package synthpop is a different tool (classification trees for disclosure control); it could make a shippable synthetic seed from a restricted file only where that file's licence allows it (legal review).
5. **Integerise per cell** with the largest-remainder rule (Hamilton): floor every weight, give the remaining households to the largest fractional parts. Lovelace and Ballas's "truncate, replicate, sample" is the same with a random tail; use the deterministic rule so the build reproduces. Fix any residual in the person total by swapping archetypes greedily.
6. **Ship per cell:** the household and person totals, the integer count per archetype as a run-length list, and alias tables for occupation given age and sex, workplace size class, school given age, condition rates given age and sex (from NHANES), plus country-level name tables and body regressions (NHANES, ANSUR II). The archetype table itself is global or per country: a few thousand household templates with ages, sexes and relationships.

### 3.2 At run time: (seed, cell, slot) → household

- **Slot to archetype** by binary search on the shipped cumulative counts. Totals are exact by construction; nothing is sampled to know how many people a cell has. This is systematic sampling with a seeded start, where the start is the deterministic apportionment.
- **Per-slot randomness** from Philox ([spec 02](../tier0/specs/02-draw.md)): key = hash(world seed, cell id), counter = (slot, member, attribute). Any slot in any order on any thread; "a unique (counter, key) tuple will always produce the same result". The Vivarium microsimulation framework and Unity's primer on repeatable random numbers use the same discipline.
- **Attributes** by Walker–Vose alias tables (constant-time draws): occupation given age and sex; a workplace by size class with a stable id = hash(cell, size class, bucket); a school by enrolment rate given age then nearest school; body from NHANES regressions by age and sex; face genes as a fixed-length vector of MakeHuman target weights, children taking each gene from either parent plus a small mutation (the Crusader Kings 3 pattern), so families resemble each other because they share the household's parent vectors.
- **Secondary totals.** Only households and people need to be exact. Workers in a given factory either use the same cumulative-count trick on a per-cell workplace capacity table or are allowed to deviate like a Poisson count.

### 3.3 Lifepaths

Each domain (education, work, union, fertility, service, migration, health) is a time-inhomogeneous Markov chain in age, as in demographic microsimulation (the MicSim package). A Traveller or Twilight: 2000 term is a four-year step of that chain. The generator knows the present (age, occupation group, education, marital status, household from the archetype) and samples the past backwards: at each earlier term, the previous state with probability proportional to its prior at that age times the transition into the current state (standard backward sampling; no game-specific paper exists). Constraints: no job before the school-leaving age; attainment consistent with the cohort; union age consistent with the country's singulate mean age at marriage; children's ages consistent with the household; a service term only where the Factbook says conscription applies; a migration year from the stock data; current job tenure from OECD or ILO. Health transitions from NHANES only, scaled by national mortality, and labelled as US-calibrated.

## 4. Gaps and legal review

**Decided 2026-10-07 ([0045](../decisions/0045-open-questions-answered-2.md) item 11):** only data that is free and needs no permission. The ambiguous datasets below are excluded rather than queried. In practice: household size distributions come from sources with an explicit licence (WorldPop and GHSL for people per cell, the Mistry and Prem home-setting matrices for who lives with whom, national statistics offices with explicit open licences such as INEGI, Statistics Canada, the US Census Bureau, INSEE), and where none exists the synthetic route uses the age pyramid and the home contact matrix alone. No emails. Realism stays the target: a number we cannot take from a restricted dataset is derived from open sources, published facts or a calibrated model, with its method named ([0046](../decisions/0046-realistic-numbers-from-free-data.md)).

1. **No shippable microdata** for most of Africa, Asia, the Middle East, Eastern Europe and most of Latin America: the hybrid route above, lower fidelity on multi-family households and relationships.
2. **IPUMS International:** do not use, or obtain written permission for fitted distributions. Do not rely on forum answers.
3. **ShareAlike sources** (UNESCO, INDEC): ship their derived tables as separable CC BY-SA files, or substitute CC BY mirrors. Legal review.
4. **UN DESA data pages** link non-commercial generic terms while the reports say CC BY 3.0 IGO: get a written confirmation.
5. **GPL repositories holding CC BY numbers** (Prem): re-derive from the paper's supplementary data.
6. **Health:** NHANES only; look at national open surveys (UK Health Survey for England teaching data under OGL, verify) and Eurostat health interview aggregate tables (CC BY 4.0).
7. **Names:** clean sources exist for the US, France and Wikidata; large gaps for China, India, the Arab world and Africa. Wikidata is biased to notable people.
8. **Eurostat public-use files and IBGE 2022:** confirm each licence.

This list joins the "one legal review before the first real-data build" already required in [world-simulation §3](../design/world-simulation.md#3-population-proposal).

## 5. Where it lands

| Doc | Change (Proposal) |
| --- | --- |
| [world-simulation §3](../design/world-simulation.md#3-population-proposal) | The human generator's first stage is the offline fit and integerisation above; the shipped unit is the per-cell archetype count list; IPUMS International, Global Data Lab, WHO and IHME are added to the excluded list, Statistics Canada, INEGI and the ONS teaching sample to the allowed one |
| [spec 22](../tier0/specs/22-census.md) (Tier 1 census of about 300) | The 300-person census can be the first use of the pipeline on one real cell of the home region: PUMS or the hybrid route, integerised, slots keyed by Philox |
| [glossary](../glossary.md) | Archetype, cell, slot |

## 6. Sources

Primary unless marked (S = secondary, W = weak). Licence pages:

- IPUMS International terms: https://international.ipums.org/international/terms.shtml ; staff forum answers (S): https://forum.ipums.org/t/reproducibility-and-data-redistribution/3975 , https://forum.ipums.org/t/redistribution-of-aggregate-statistics/1506 ; IPUMS USA: https://usa.ipums.org/usa/terms.shtml
- US ACS PUMS: https://www.census.gov/programs-surveys/acs/microdata.html ; citation policy https://www.census.gov/about/policies/citation.html
- Statistics Canada Open Licence: https://www.statcan.gc.ca/en/reference/licence ; 2021 PUMF https://www150.statcan.gc.ca/n1/en/catalogue/98M0001X2021001
- INEGI terms: https://www.inegi.org.mx/inegi/terminos.html ; Censo 2020 microdata https://www.inegi.org.mx/rnm/index.php/catalog/632
- INDEC licence statement: https://www.indec.gob.ar/indec/web/Institucional-Indec-Normativa
- IBGE Censo 2022 microdata: https://ftp.ibge.gov.br/Censos/Censo_Demografico_2022/Microdados_e_Areas_de_Ponderacao/
- Eurostat public-use files: https://cros.ec.europa.eu/book-page/public-use-files-eurostat-microdata
- ONS microdata tiers: https://www.ons.gov.uk/census/aboutcensus/censusproducts/microdatasamples ; UK Data Service licence https://ukdataservice.ac.uk/cd137-enduserlicence
- ABS: https://www.abs.gov.au/statistics/microdata-tablebuilder/frequently-asked-questions
- UN World Population Prospects: https://population.un.org/wpp/ ; household database https://www.un.org/development/desa/pd/data/household-size-and-composition and https://population.un.org/Household/index.html ; World Marriage Data https://www.un.org/development/desa/pd/node/3593 ; migrant stock https://www.un.org/development/desa/pd/node/3315 ; generic UN terms https://www.un.org/en/about-us/terms-of-use
- Prem et al. 2021: paper https://pmc.ncbi.nlm.nih.gov/articles/PMC8354454/ , repository https://github.com/kieshaprem/synthetic-contact-matrices ; Mistry et al. 2021 https://github.com/mobs-lab/mixing-patterns
- DHS terms: https://dhsprogram.com/data/terms-of-use.cfm
- WorldPop: https://hub.worldpop.org/project/categories?id=8 ; GHSL https://human-settlement.emergency.copernicus.eu/GHSLhowToCite.php ; Global Data Lab https://globaldatalab.org/termsofuse/
- ILOSTAT: https://ilostat.ilo.org/about/dissemination-and-analysis ; O*NET https://www.onetcenter.org/license_db.html ; OECD https://www.oecd.org/en/about/terms-conditions.html ; UNESCO UIS https://databrowser.uis.unesco.org/terms-and-conditions
- WHO: https://www.who.int/about/policies/publishing/copyright and https://www.who.int/about/policies/publishing/data-policy/terms-and-conditions ; IHME https://www.healthdata.org/sites/default/files/files/free-of-charge_non-commercial_user_agreement.pdf ; Our World in Data on GBD redistribution (S) https://ourworldindata.org/faqs
- NHANES release policy: https://www.cdc.gov/nchs/data/nhanes/nhanes_release_policy.pdf ; CIA World Factbook https://www.cia.gov/the-world-factbook/about/faqs/
- Names: SSA https://catalog.data.gov/dataset/baby-names-from-social-security-card-applications-national-data ; INSEE https://www.insee.fr/fr/statistiques/8595113 ; IBGE https://censo2010.ibge.gov.br/nomes ; Wikidata https://www.wikidata.org/wiki/Wikidata:Licensing ; names-dataset provenance https://github.com/philipperemy/name-dataset
- MakeHuman licence: https://static.makehumancommunity.org/about/license.html

Methods:

- SynthPops (MIT): https://github.com/InstituteforDiseaseModeling/synthpops ; household algorithm docs https://docs.idmod.org/projects/synthpops/en/v1.5.2/households.html ; Covasim hybrid population https://github.com/InstituteforDiseaseModeling/covasim/blob/master/covasim/population.py and household sizes https://github.com/InstituteforDiseaseModeling/covasim/blob/master/covasim/data/household_size_data.py
- OpenABM-Covid19 rejection sampling: https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1009146
- RTI SynthPop and FRED (non-commercial): https://www.rti.org/focus-area/rti-synthpoptm , https://fred.publichealth.pitt.edu/syn_pops
- Iterative proportional updating (Ye, Konduri, Pendyala): https://scag.ca.gov/sites/main/files/file-attachments/populationsynthesizerpaper_trb.pdf ; pyipu (S) https://pypi.org/project/pyipu ; hierarchical IPF (Müller and Axhausen) https://www.strc.ch/2012/Mueller_Axhausen.pdf ; synthpop (R) https://cran.r-project.org/web/packages/synthpop/ ; sample-free synthesis for Melbourne https://arxiv.org/pdf/2008.11660 ; eqasim (GPL) https://ile-de-france.readthedocs.io/en/latest/
- Integerisation: Lovelace and Ballas https://arxiv.org/pdf/1303.5228 ; largest remainder (S) https://www.ams.org/samplings/feature-column/fcarc-apportionii1
- Keyed randomness: Random123 https://users.cs.utah.edu/~hari/teaching/bigdata/random123sc11.pdf ; alias method (S) https://en.wikipedia.org/wiki/Alias_method ; Vivarium randomness (S) https://vivarium.readthedocs.io/en/latest/api_reference/framework/randomness/index.html ; Unity primer on repeatable random numbers (S) https://unity.com/blog/engine-platform/primer-on-repeatable-random-numbers
- Life-course microsimulation (S): https://rdrr.io/cran/MicSim/man/MicSim-package.html ; lifepath systems in games (W) https://www.dampfkraft.com/games/lifepaths.html ; CK3 DNA and mutation (W) https://primagames.com/?p=313090
