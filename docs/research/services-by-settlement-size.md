# Which services a place has: settlement size, country and who covers the rest

**Date:** 2026-10-10. **Status:** research note, not a decision. **Feeds:** [0069](../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md) rule 4, [pop lore](../design/pop-lore-and-custom-pops.md) §7 (jurisdiction), [spec 33](../tier0/specs/33-establishments-and-chains.md), [spec 39](../tier0/specs/39-orders-and-belief.md), [spec 46](../tier0/specs/46-education-labour-occupations-settlement.md), [spec 47](../tier0/specs/47-country-towns.md), [world numbers](../design/world-numbers.md). **Evidence:** one research pass on 2026-10-10 (11 web calls). Read in full: BJS *Local Police Departments 2016: Personnel*, World Bank physician and hospital-bed series, the NVFC fact sheet; other rows from search summaries, marked `reported`. **IDs:** SV.

"Not everything has everything" (decision 0069). Today the town generator gives every town one police station, one hospital and one fire station (`data/gen/businesses.toml`). This note gathers how services really scale with settlement size and country, and who covers the places that lack them.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| SV-1 | United States, 30 June 2016: 15,322 general-purpose state and local law enforcement agencies: 12,261 local police departments, 3,012 sheriffs' offices (excluding jail- and court-only offices) and 49 primary state agencies (Hawaii has none). | sourced | BJS, Local Police Departments 2016: Personnel, table 2, https://bjs.ojp.gov/content/pub/pdf/lpd16p.pdf |
| SV-2 | Full-time sworn officers in those agencies, 2016: 701,273 (local police 468,274 = 67%; sheriffs 173,354; state 59,645); part-time sworn 39,334; local police also had 131,274 full-time civilians (22% of their full-time staff). | sourced | BJS lpd16p, tables 1–2 |
| SV-3 | General-purpose full-time sworn officers per 1,000 US residents: 2.42 in 1997, 2.17 in 2016 (down 11%). | sourced | BJS lpd16p, p. 1 |
| SV-4 | Local police departments by population served, 2016 (departments; full-time sworn): 1M+ 17 (103,739); 500,000–999,999 33 (50,092); 250,000–499,999 57 (37,395); 100,000–249,999 213 (54,020); 50,000–99,999 432 (49,988); 25,000–49,999 905 (54,163); 10,000–24,999 1,931 (59,268); 2,500–9,999 3,952 (44,384); 2,499 or less 4,722 (15,224). 71% of departments serve under 10,000 residents; under 3% serve 100,000+ and employ 52% of officers. | sourced | BJS lpd16p, table 4 |
| SV-5 | Mean full-time-equivalent sworn local police officers per 1,000 residents by population served, 2016 (U-shaped): 1M+ 2.3; 500,000–999,999 2.2; 250,000–499,999 1.9; 100,000–249,999 1.7; 50,000–99,999 1.6; 25,000–49,999 1.8; 10,000–24,999 1.9; 1,000–9,999 2.3 (standard errors 0.01–0.09; places under 1,000 excluded). | sourced | BJS lpd16p, figure 2 and appendix table 6 |
| SV-6 | Local departments by size of agency (FTE sworn), 2016: 1 officer 6.3%; 2–4 18.0%; 5–9 23.4%; 10–24 27.4%; 25–49 12.9%; 50–99 6.9%; 100–249 3.5%; 250+ 1.6%. About half (48%) have under 10 FTE sworn; the 75% with under 25 employ 14% of officers; 624 departments with 100+ employ 62%. | sourced | BJS lpd16p, table 3 |
| SV-7 | Median full-time sworn officers: 8 in departments serving under 100,000 residents (11,948 departments, 97%); 304 in departments serving 100,000+ (313 departments; the text says 319). | sourced | BJS lpd16p, p. 6 and table 13 notes |
| SV-8 | Share of local departments with a full-time specialized unit, 2016, serving 100,000+ vs under 100,000: drug enforcement 88.7% vs 14.5%; child abuse 71.6% vs 6.0%; gangs 69.9% vs 3.4%; domestic violence 68.8% vs 5.7%; financial crimes 62.1% vs 4.4%; cybercrime 45.4% vs 2.8%; crime analysis 89.7% vs 5.9%; SWAT 57.1% vs 5.3%. Most small departments with such work have only "designated" personnel (for example child abuse 28.5%). | sourced | BJS lpd16p, tables 13–14 |
| SV-9 | Mean full-time sworn per local department by population served, 2016: 2,499 or less 3.2; 2,500–9,999 11.2; 10,000–24,999 30.7; 25,000–49,999 59.8; 50,000–99,999 115.7; 100,000–249,999 253.6; 250,000–499,999 656; 500,000–999,999 1,518; 1M+ 6,102. | derived | from SV-4 |
| SV-10 | About 15% of US state and local sworn personnel had criminal investigation as primary duty (censuses of 1996 and 2000); 64% (423,000) were uniformed officers responding to calls (1996); about 8% jail and 3% court duties (1996). No newer duty split was found. | reported | BJS CSLLEA 1996 and 2000 text, https://bjs.ojp.gov/content/pub/ascii/csllea00.txt, https://bjs.ojp.gov/content/pub/ascii/CSLLEA96.TXT |
| SV-11 | At 15% investigators, a department needs about 7 sworn officers to hold one investigator on average (1 / 0.15 = 6.7), which at 2.3 officers per 1,000 is a place of about 3,000 residents; about half of US local departments (SV-6: 47.7% under 10 FTE) are near or below that size. The 15% is all agencies; the share in small departments was not found. | derived | from SV-5, SV-6, SV-10 |
| SV-12 | Share of full-time sworn local officers who are female, by population served, 2016: 1M+ 17.6%; 500,000–999,999 14.6%; 250,000–499,999 15.2%; 100,000–249,999 12.0%; 50,000–99,999 10.5%; 25,000–49,999 9.6%; 10,000–24,999 9.0%; 9,999 or less 7.3%; all 12.3%. | sourced | BJS lpd16p, table 5 |
| SV-13 | 2.5% of full-time local police officers (11,870) were school resource officers in 2016. | sourced | BJS lpd16p, p. 1 |
| SV-14 | France: the Police nationale is generally responsible for communes over 20,000 inhabitants and is established in department capitals; it may be established where population counting seasonal flows exceeds 20,000 and crime resembles urban areas; the Gendarmerie nationale covers the rest. Decrees of September 1996 (under the January 1995 security law) set the gendarmerie for communes under 20,000. | reported | French parliamentary questions, https://questions.assemblee-nationale.fr/dyn/14/questions/QANR5L14QE84548.pdf and https://questions.assemblee-nationale.fr/dyn/10/questions/QANR5L10QG1916.pdf |
| SV-15 | France, exceptions: the Cour des comptes notes that more than one police district in ten covers under 20,000 people, and two-thirds of communes integrated into metropolises lie in gendarmerie zones; many communes over 20,000 on La Réunion remain gendarmerie zones. | reported | Senate question 2025, https://www.senat.fr/questions/base/2025/qSEQ250203122.html |
| SV-16 | France: the gendarmerie covers about 95% of the territory and about 50% of the population; the police about 5% of the territory and about 50% of the population (undated, non-official). | reported | https://infos.emploipublic.fr/article/devenir-policier-ou-gendarme-ou-travaillerez-vous-eea-7730 |
| SV-17 | England and Wales: 43 territorial police forces; 147,746 FTE officers at 31 March 2024 (highest since 2003), 146,868 at 30 September 2024. | reported | Home Office, Police workforce, https://www.gov.uk/government/statistics/police-workforce-england-and-wales-31-march-2024 |
| SV-18 | The Home Office withdrew its 2024 officers-by-function data (investigations, local policing) as out of date pending validation, so no current UK detective share was found. | reported | Home Office, same release |
| SV-19 | A UN analysis (2006) found a median of about 300 police per 100,000 inhabitants, highest (about 400) in West Asia, Eastern and Southern Europe, stable 2002–2006. | reported | relayed by https://en.wikipedia.org/wiki/List_of_countries_and_dependencies_by_number_of_police_officers |
| SV-20 | UNODC data, countries over 5 million people (year not stated): Russia 564.6, Turkey 474.8, Italy 467.2 police per 100,000. UNODC counts public agencies whose main functions are prevention, detection and investigation of crime and apprehension of offenders. | reported | https://www.statista.com/chart/amp/1540/russia-is-the-worlds-most-heavily-policed-country |
| SV-21 | US general-purpose sworn officers are about 217 per 100,000 (2016), below the UN 2006 median of about 300; the US figure excludes federal and special-purpose agencies, so definitions differ. | derived | from SV-3, SV-19 |
| SV-22 | Physicians per 1,000 people, 2022 (World Bank aggregates; earlier years empty in the API): high income 3.78; upper-middle 2.41; lower-middle 0.71; low income 0.34; world 1.86. | sourced | World Bank WDI SH.MED.PHYS.ZS, https://api.worldbank.org/v2/country/HIC;UMC;LMC;LIC;WLD/indicator/SH.MED.PHYS.ZS?format=json&date=2015:2022 |
| SV-23 | Hospital beds per 1,000 people: high income 5.24 (2022); upper-middle 4.18 (2022); lower-middle 1.52 (2021); world 3.29 (2021); low income has no value. | sourced | World Bank WDI SH.MED.BEDS.ZS, https://api.worldbank.org/v2/country/HIC;UMC;LMC;LIC;WLD;FRA;USA;GBR;MEX;BRA;IND;NGA/indicator/SH.MED.BEDS.ZS?format=json&date=2010:2022 |
| SV-24 | Hospital beds per 1,000, latest: France 5.65 (2022), United States 2.68 (2022), United Kingdom 2.42 (2022), Brazil 2.52 (2021), India 1.59 (2021), Mexico 1.01 (2022); Nigeria has no value. | sourced | World Bank WDI SH.MED.BEDS.ZS, same query |
| SV-25 | Beds move with time: high income fell from 5.81 (2010) to 5.24 (2022); upper-middle rose from 2.33 to 4.18; lower-middle stayed about 1.4–1.5. | sourced | World Bank WDI SH.MED.BEDS.ZS, same query |
| SV-26 | US fire departments, 2020: 29,452, of which 18,873 all volunteer (64.1%), 5,335 mostly volunteer (18.1%), 2,459 mostly career (8.3%) and 2,785 all career (9.5%). | reported | NVFC fact sheet relaying NFPA U.S. Fire Department Profile 2020, https://www.nvfc.org/wp-content/uploads/2026/02/volunteer-fire-service-fact-sheet.pdf |
| SV-27 | US firefighters: 1,018,100, of whom 635,100 (62%) are volunteers. | reported | NVFC fact sheet relaying NFPA, same URL |
| SV-28 | All or mostly volunteer departments (82%) protect about one third of the US population; all or mostly career departments (18%) protect more than two thirds. | reported | NFPA Profile 2020 as summarised, https://internationalfireandsafetyjournal.com/?p=77143 |
| SV-29 | Communities under 10,000 people are typically protected by all-volunteer departments (some hire a few paid firefighters); 10,000–100,000 by combination departments; over 100,000 mostly by combination departments with mainly paid staff; all-paid departments are found mainly in very urban areas. | reported | NVFC fact sheet, item 8 (underlying NFPA supporting tables not read) |
| SV-30 | Nearly half of US volunteer firefighters serve communities of under 2,500 people. | reported | USFA FA-240, https://www.usfa.fema.gov/downloads/pdf/publications/fa-240.pdf |
| SV-31 | US fire department call volume more than tripled over 40 years, mostly from emergency medical calls, while volunteer numbers reached a low in 2023. | reported | NVFC fact sheet, item 5 |
| SV-32 | 34.6 firefighters per US fire department on average; with the 2020 US population of about 331 million (census figure from memory, not fetched), about one department per 11,300 residents and 3.1 firefighters per 1,000. | derived | from SV-26, SV-27 |
| SV-33 | Berry and Garrison estimated threshold populations for 52 central functions in Snohomish County, Washington, with an exponential model; the per-function values were not retrieved. | reported | Blome 1968, https://journals.indianapolis.iu.edu/index.php/ias/article/download/6886/6881/12996 |
| SV-34 | Thresholds are regional: central places in Cache County, Utah needed larger populations than places in Snohomish County to support the same function (1968 thesis). | reported | https://digitalcommons.usu.edu/etd/4331 |

## What this means for us (Proposal)
1. **A services table, by size class and country (decision 0069 rule 4; specs 46/47).** Replace `count = 1` for `hospital`, `police` and `fire_station` in `data/gen/businesses.toml` (today every town gets one of each) with a table that says, per service, who hosts it at each settlement size and how many staff it has. Use the BJS size classes everywhere (under 2,500; 2,500–9,999; 10,000–24,999; 25,000–49,999; 50,000–99,999; 100,000–249,999; 250,000–499,999; 500,000–999,999; 1 million+), because they are the only classes with sourced staffing. Shape, in `data/services/`:

   ```toml
   # data/services/services.toml: one record per service kind
   [[service]]
   id = "police_patrol"            # an institution: a queue with staff (0069 rule 3)
   occupation = "police"           # staff come from jobs.toml
   host = "own_agency"             # own_agency | district | region | nation (see rule 3)
   ratio_kind = "per_1000"         # staff per 1,000 residents of the area served
   curve = "police_us_2016"        # a size curve in curves.toml
   country_scale = "police_per_100k"   # the country field that scales the curve
   sources = ["SV-5", "SV-21"]

   # data/services/curves.toml: a curve is measured in one reference country
   [[curve]]
   id = "police_us_2016"
   country = "us"; year = 2016; reference_per_100k = 217
   # [min_pop, max_pop (0 = open), mean per 1,000]
   classes = [[1000, 9999, 2.3], [10000, 24999, 1.9], [25000, 49999, 1.8], [50000, 99999, 1.6],
              [100000, 249999, 1.7], [250000, 499999, 1.9], [500000, 999999, 2.2], [1000000, 0, 2.3]]
   ```

   And one table for **services by country income group** (`data/services/income_groups.toml`), filled where a source exists and `not_found` elsewhere:

   | Service | Measure | High | Upper-middle | Lower-middle | Low | Size curve | Source |
   | --- | --- | --- | --- | --- | --- | --- | --- |
   | Physicians | per 1,000, 2022 | 3.78 | 2.41 | 0.71 | 0.34 | not found | SV-22 |
   | Hospital beds | per 1,000 | 5.24 | 4.18 | 1.52 | not found | not found | SV-23 |
   | Police | per 100,000 | US 217; median ~300 (all countries, 2006) | not found | not found | not found | `police_us_2016` | SV-5, SV-19, SV-21 |
   | Firefighters | per 1,000 | US 3.1 (62% volunteer) | not found | not found | not found | volunteer under 10,000 (rule 5) | SV-27, SV-29, SV-32 |

   A country profile in `data/setup/nations/` holds its own value when one is sourced (France 5.65 beds, United States 2.68) and falls back to its income group otherwise.

2. **Police staff by size: the curve times the country (spec 47; spec 39 staffing edicts).** Sworn officers of an agency = residents served × the curve's value for its class × (country police per 100,000 ÷ 217). The US curve is U-shaped (2.3 per 1,000 at both ends, 1.6 at 50,000–99,999), so small towns are not under-policed per head; they have few officers in total (mean 3.2 under 2,500, 11.2 at 2,500–9,999). Draw department size around the class mean using SV-6 as the spread. Using the US shape for other countries is an assumption (estimate) until a second country's curve is found. A staffing edict (spec 39) multiplies the country factor; less money means fewer staff and a longer queue, the "shortage above shows as long queues below" of 0069.

3. **The jurisdiction rule, replacing pop-lore §7's default (settlements under 20,000 use the nearest city's detectives).** Real systems do not share one threshold; the 20,000 line is real only in France, and there it decides which *force* (police or gendarmerie) serves a commune, not whether it has investigators (SV-14). Proposed rule, a function of the settlement map and the agency table, so it never depends on click order:
   - Each country names a **policing model** in its profile: `municipal` (United States: local departments, county sheriffs, state agencies; SV-1), `dual_national` (France: Police nationale in communes of 20,000+ and department capitals, gendarmerie elsewhere; SV-14), `regional_force` (England and Wales: 43 forces, each covering every place in its area; SV-17) or `national` (one national police with local posts; the default where nothing is sourced).
   - Each case kind has a **minimum level**: patrol and minor crime at the place's own post; general investigation (a robbery) at an agency that holds at least one investigator; specialised investigation (homicide, child abuse, financial, cyber) at an agency with a full-time unit, which in the US means a department serving 100,000+ (57–89% have one) or the state level (SV-8).
   - **The office for a case** = the nearest office, among agencies covering the place at or above the case's level, that has staff with the needed skill; ties broken by agency id.
   - What this gives: in a `municipal` country, a town under about 3,000 people (under about 7 officers, SV-11) has no investigator, so its robbery goes to the county's investigators at the county seat; a town of 30,000 has about 9 investigators of its own (SV-9 × SV-10). In `dual_national`, a commune under 20,000 is investigated by the gendarmerie, never by the city police. In `regional_force`, every place uses the force's investigators at their nearest base. The fourth demo flow's "detective from the nearest city" holds under all three models for a small town.

4. **Detective share (pop lore §7; spec 46 occupations).** Keep "one detective per 7 police officers" but source it: about 15% of sworn officers have investigation as their primary duty (SV-10), so investigators = round(0.15 × sworn), with a department under about 7 sworn holding none. Specialized units appear only in agencies serving 100,000+ (SV-8). The 20–40 open cases per detective has no source here (Gaps).

5. **Fire services: volunteers in small places (specs 33, 40, 24; 0069 rule 1).** Under 10,000 people a fire department is all volunteer; 10,000–100,000 combination; over 100,000 mainly career (SV-29). A volunteer firefighter is a pop with another main job and a second occupation record with schedule pattern `on_call`: they are at home or work and leave when paged (spec 24 decides whether they go; spec 40 wakes them). Career firefighters get shift patterns (shift length not researched here). One department per about 11,300 residents in the US (SV-32) is a check, not a rule: departments follow places, so one per settlement over a threshold (not found) and the county covers the rest.

6. **Health: beds and physicians by income group, hospitals by catchment (0069 rule 4; spec 47).** Physicians per 1,000 differ elevenfold between high and low income (3.78 against 0.34; SV-22). A town of 10,000 averages about 38 physicians in a high-income country and 3 in a low-income one, before any urban concentration (no source found for that). Beds sit in hospitals: a hospital is hosted in a place whose catchment × beds per 1,000 reaches a minimum hospital size. That size and the catchment radius were not found; they are estimates to replace. Smaller places get clinics or nothing and send patients up the hierarchy, like police cases.

7. **Thresholds are data per country, not constants (spec 47).** The same function needs a larger population in one region than in another (SV-34), and income moves every ratio above. Each threshold row carries its country or income group and its source ID; a missing value falls back to the income group, then to the US curve, and is marked as an estimate in the town dossier.

8. **Sex of officers by size class (specs 46/47).** The police occupation draws female officers at 7.3% in places under 10,000 rising to 17.6% in cities of 1 million+ (US 2016, SV-12) instead of one national share. Other countries: not found.

## Gaps

- **Central place thresholds:** Berry and Garrison's per-function values (and any modern study of service thresholds in towns) were not retrieved; only the method and the regional caveat (SV-33, SV-34).
- **Detectives:** no current duty split (BJS censuses after 2000 not found; Home Office function data withdrawn); no investigator share by department size; no caseload per detective (the 20–40 open cases default stays unsourced).
- **Who polices small US places:** that sheriffs cover unincorporated areas and towns without departments, and that state bureaus assist small agencies on major cases, is general knowledge, not read here. The share of US places with their own department was not found.
- **France:** the gendarmerie's investigative structure (brigades de recherches at the compagnie seat) is from memory, not read. Official shares of population and territory per force were not found; SV-16 is from a non-official page.
- **Other national systems:** no data was read on Latin American national police structures, or on Spain, Italy or Germany. No current UNODC per-country table was read; the 300 median dates from 2006.
- **Funding:** police spending per capita by country (Eurostat COFOG, OECD) and municipal budgets were not fetched.
- **Fire:** the NFPA supporting tables (departments and population protected by community size) are images in the NVFC sheet and were not read. NFPA 1710 and 1720 response-time standards are known only from memory, not read. Career shift patterns were not researched.
- **Health:** no hospital catchment sizes, rural travel times, clinic-against-hospital thresholds, or physicians by settlement size; no low-income bed value in WDI.
- **Schools and other services by settlement size:** not searched (budget).
