# World numbers for nations: how they arise, how big they are, what they can do

**Date:** 2026-10-06
**Status:** research note, not a decision. Feeds the "world numbers" design doc that [0035](../decisions/0035-realistic-world-numbers-and-the-pop-nation-loop.md) asks for. Nothing here is Decided, Implemented or Validated.

**Method and limits.** WebFetch was blocked by the network proxy for every domain tried (en.wikipedia.org, pnas.org, ourworldindata.org, arxiv.org), so no source page was read in full. **Every number below comes from WebSearch result summaries**, each tied to the URL the search returned. Where a summary quoted a secondary source (a blog, a forum, an aggregator), confidence is marked low and the primary source is named so it can be checked. Equations in section 2 are **our own proposals** built on these anchors; none is copied from a paper unless said so.

## 1. Numbers

Confidence: H = from a primary or institutional source in the summary; M = secondary summary of a primary study; L = aggregator, blog or forum, check before use.

### 1.1 State formation

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Carneiro circumscription: states arise where agricultural land is bounded (valley, coast, desert) so the losers of war cannot flee; the more circumscribed, the sooner the unification. Peru coast valleys yes, Amazon no. Qualitative only, no formula | rule, no number | n/a | https://en.wikipedia.org/wiki/Circumscription_theory (search summary; original: Science 169:733, 1970) | M |
| Diamond: band under about 100 people; tribe hundreds; chiefdom thousands; state over about 50,000 with bureaucracy and written law | 100 / hundreds / thousands / 50,000 | people | https://litcharts.com/lit/guns-germs-and-steel/chapter-14-from-egalitarianism-to-kleptocracy | L (study guide; check the book) |
| Scott: only cereal grains are visible, divisible, storable and assessable, so they are the tax base of early states | rule, no number | n/a | https://en.wikipedia.org/wiki/Against_the_Grain:_A_Deep_History_of_the_Earliest_States | M |
| Tilly: war made states; expensive war (gunpowder, mass armies) favoured states with capital and large populations, forcing taxation and conscription | rule, no number | n/a | https://en.wikipedia.org/wiki/Coercion,_Capital,_and_European_States,_AD_990%E2%80%931992 | M |
| Turchin et al. 2013: a spatial cultural-evolution model (war, diffusing cavalry and chariot technology, rugged terrain) explains where large states arose 1500 BCE to 1500 CE | 65% of variance explained (16% without military technology diffusion) | share of variance | https://peterturchin.com/publications/war-space-and-the-evolution-of-old-world-complex-societies/ and https://pmc.ncbi.nlm.nih.gov/articles/PMC3799307 | M |
| Seshat: one principal component captures about three quarters of the variation in social complexity across 414 societies, 30 regions, 51 variables | 75% | share of variance | https://csh.ac.at/publication/quantitative-historical-analysis-uncovers-a-single-dimension-of-complexity-that-structures-global-variation-in-human-social-organization/ | M |
| Secular cycle length (integrative expansion, stagflation, crisis, depression) | 100 to 300 | years | https://wiki.p2pfoundation.net/Secular_Cycles | M |
| Number of demographic cycles detected in Eurasia and North Africa | more than 40 | cycles | https://wiki.p2pfoundation.net/Secular_Cycles | L |
| Turchin and Korotayev 2006: population and internal warfare oscillate with the same period; warfare peaks follow population peaks | phase lag, no number found | n/a | https://wiki.p2pfoundation.net/Secular_Cycles | L |

### 1.2 Size and count of states

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Country area and population distributions follow a power law; population exponent about 0.97 (cities 1.94); area exponent near 1 | 0.97; about 1 | exponent | https://arxiv.org/pdf/1006.5269 | M |
| Mean country population grows with mean area as N ~ A^0.82, so density falls with area as A^-0.18 | 0.82; -0.18 | exponent | https://arxiv.org/pdf/1006.5269 | M |
| Upper tail looks both rank-size and lognormal depending on the full dataset | qualitative | n/a | https://arxiv.org/pdf/1006.5269 | M |
| Taagepera: after 600 BCE some empire always held at least 2.5 million km2; after 1600 CE at least 10 million km2; empire durations did not grow in 5000 years | 2.5; 10 | million km2 | https://cambridge.org/core/books/more-people-fewer-states/growthdecline-patterns-and-durations-of-empires/BE64D7F6A2C0D1CE600879E35A1E280C and https://lrb.co.uk/the-paper/v45/n09/tom-stevenson/empires-in-disguise | M (Taagepera's lists are criticised: https://www.bookandsword.com/2020/01/25/be-careful-with-rein-taageperas-lists-of-largest-empires/) |
| Taagepera: size breakthroughs at about 2800 BCE (first Mesopotamian cities), 600 BCE (Persian satrapies plus relay stations, "power delegation"), and the European overseas expansion | dates | year | https://lrb.co.uk/the-paper/v45/n09/tom-stevenson/empires-in-disguise | M |
| Largest empires at peak: British 35.5 (1920), Mongol 24.0 (c. 1270-1309), Russian 22.8 (1895), Qing 14.7 (1790) | 35.5 / 24.0 / 22.8 / 14.7 | million km2 | https://en.wikipedia.org/wiki/List_of_largest_empires (via search summary; also https://statista.com/chart/amp/20342/peak-land-area-of-the-largest-empires) | M |
| Independent states in 1800 | 27 | states | https://bbb-old.lwn.net/abcd/3612236 (search summary, aggregator) | L |
| States in 1900 | about 50 | states | same | L |
| UN founding members 1945 | 51 | states | https://en.wikipedia.org/wiki/Enlargement_of_the_United_Nations | H |
| UN members, 2026 | 193 | states | same | H |
| Roman courier speed on the road system | about 50 Roman miles per day (about 46 modern miles); urgent relays about 170 miles per day | miles/day | https://www.historyskills.com/classroom/ancient-history/roman-mail-system/ | L |
| Roman post-road network | about 47,000 | miles | https://www.historyskills.com/classroom/ancient-history/roman-mail-system/ | L |

### 1.3 Fiscal capacity, army, wages

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Tax revenue by income group (average): low 14, lower-middle 18, upper-middle 22, high 33 | 14 / 18 / 22 / 33 | % of GDP | https://www.oecd.org/en/publications/tax-policy-reforms-2026_43d18a55-en/full-report/tax-revenue-context_6e7b3dab.html (search summary; also https://opendocs.ids.ac.uk/opendocs/handle/20.500.12413/18266) | M |
| Same, one year (2023): low-income 13, high-income 31 | 13 / 31 | % of GDP | search summary on https://www.oecd.org/en/publications/tax-policy-reforms-2026_43d18a55-en/full-report/tax-revenue-context_6e7b3dab.html | M |
| Tax-to-GDP tipping point for growth: about 12.75; rule of thumb 15. Above it, GDP per capita about 7.5% higher after 10 years (139 countries, 1965-2011; 30 advanced economies, 1800-1980) | 12.75; 15; 7.5 | % of GDP; % of GDP per capita | https://www.imf.org/en/Publications/WP/Issues/2016/12/31/Tax-Capacity-and-Growth-Is-there-a-Tipping-Point-44436 and https://blogs.worldbank.org/governance/getting-15-percent-addressing-largest-tax-gaps | H |
| Roman Empire taxation | about 5 | % of gross product | https://knowablemagazine.org/article/society/2017/taxes-through-ages | M |
| Medieval states | no more than about 5 | % of GDP | https://knowablemagazine.org/article/society/2017/taxes-through-ages | M |
| 18th century imperial taxes: China 5, Russia 12-15, France 9-13, Britain 16-24 (the last two as shares of commodity production, not GDP) | see cell | % | https://academic.oup.com/book/11024/chapter/159355022 (search summary, exact chapter not confirmed) | L |
| Britain total tax: 1665 3.4, 1800 12.9. France: early 1700s 9.4, 1788 6.8 | see cell | % of GDP | https://mpra.ub.uni-muenchen.de/40403/1/MPRA_paper_40403.pdf (search summary, not confirmed) | L |
| Ottoman revenue lagged other states in the 17th-18th centuries because of high shares kept by intermediaries (tax farming); figure not retrieved | not found | n/a | https://ideas.repec.org/a/cup/jechis/v70y2010i03p593-629_00.html | M |
| Preindustrial rule of thumb: standing army about 0.5 | 0.5 | % of population | https://mythicscribes.com/community/threads/troop-to-population-ratio.14993/latest (forum; check Scheidel or Turchin) | L |
| Roman army under Augustus: about 350,000 men in a population of about 50 million | 0.7 | % of population | search summary, same forum thread family; primary not found | L |
| Rome in the 2nd Punic War about 10; early 2nd century BCE about 5 | 10; 5 | % of (citizen) population | same | L |
| Peacetime armies c. 1900: France about 2, Germany about 1, Russia 0.8, Austria-Hungary 0.7 | 2 / 1 / 0.8 / 0.7 | % of population | https://www.Strategypage.Com/cic/docs/cic440b.asp (search summary) | L |
| WWII armed forces at peak: USSR 7, USA 9, Japan about 8, UK 10 | 7 / 9 / 8 / 10 | % of population | https://warwick.ac.uk/fac/soc/economics/staff/mharrison/war_and_economy/08_ww2_mobilization.pdf (search summary; exact figures per country not confirmed) | M |
| Industrial economy can divert about 10 of total population to the military before production suffers; WWI peak about 15 (France 21) | 10; 15; 21 | % of population | https://www.alternatehistory.com/forum/threads/max-us-mobilization.321219/page-3 (forum) | L |
| Roman legionary pay under Augustus to Domitian | 225 | denarii/year | https://en.wikipedia.org/wiki/Pay_(Roman_army) | M |
| Labourer wage, late Republic to early Empire (disputed) | about 1 | denarius/day | https://imperiumromanum.pl/en/roman-society/salary-in-ancient-rome/ ; dispute: https://interpreterfoundation.org/journal/was-the-denarius-a-daily-wage-a-note-on-the-parable-of-the-two-debtors-in-luke-740-43 | L |
| Roman army share of imperial budget | 50 to 75 (some sources up to 80) | % of budget | https://en.wikipedia.org/wiki/Economics_of_the_Roman_army | L |

Derived from the two Roman rows (our arithmetic, not a source claim): 225 denarii per year against about 300 working days at 1 denarius is a soldier's pay of roughly 0.75 of a labourer's annual earnings, before food, kit and housing.

### 1.4 Administration

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Roman central salaried officials | 35,000 or more, for a population of about 45 million (about 8 per 10,000, our arithmetic) | officials | https://www.johndcook.com/blog/2012/08/09/roman-bureaucracy/ | L |
| Han empire administration | about 130,000 trained professionals (population not retrieved, so no ratio) | officials | https://backstory.london/products/9781350445123 (search summary; weak) | L |
| Qing: about 13,000 officials in formal appointments at any time, 1830-1911 (population not retrieved) | 13,000 | officials | https://www.cqh.hku.hk/research/organizational-demography-of-the-qing-civil-service-1830-1911/ | M |
| General government employment, OECD average 2023 | 18.4 | % of total employment | https://www.oecd.org/en/publications/government-at-a-glance-2025_0efd0bcd-en/full-report/employment-in-general-government_dafcfac5 | H |
| Highest: Norway 30.1, Sweden 28.2, Denmark 27.3, Finland 25.2. Lowest: Chile, Japan, Korea below 10 | see cell | % of total employment | same | H |
| Span of control: Graicunas 1933 and Urwick suggested 5-6 direct reports; typical range 3-15; supervision load rises geometrically with span | 5-6; 3-15 | subordinates | https://en.wikipedia.org/wiki/Span_of_control | M |
| Turchin-style hierarchy model: maximum hierarchy height grows logarithmically with the number of agents; average level and the share of ground-level agents are independent of size | log N | levels | https://arxiv.org/pdf/1707.01744 (search summary of the abstract) | M |
| Levels of government by population size | not found | n/a | n/a | n/a |

### 1.5 Strength and legitimacy

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Fragile States Index: 12 indicators, each 0-10, total 0-120, 120 most unstable. Indicators: security apparatus, factionalized elites, group grievance, economic decline, uneven development, human flight, state legitimacy, public services, human rights and rule of law, demographic pressures, refugees and IDPs, external intervention | 0-120 | index | https://en.wikipedia.org/wiki/Fragile_States_Index | H |
| Worldwide Governance Indicators: government effectiveness, rule of law, corruption control, voice and accountability, regulatory quality correlate with GDP per capita; political stability did not in the cited study (one small study, not general) | correlation, coefficient not retrieved | n/a | https://ortus0m.rtu.lv/science/lv/publications/14237/fulltext | L |
| Link between state size and fragility, and trust surveys by income and size | not found | n/a | n/a | n/a |

## 2. Equations we could implement

All are **Proposals**. Anchors come from section 1; parameters marked "design" have no source and must be calibrated by headless runs. Bands: **real** keeps within the cited ranges, **sandbox** is wide enough to play with.

Notation: P population, W working-age share, y income in multiples of subsistence, Y = y·s·P output, c state capacity in [0,1], L legitimacy in [0,1].

**E1. Tax ceiling by development (fiscal ceiling).**
`tau_cap(y) = tau0 + (tau1 - tau0) * y / (y + y_half)`
- Anchors: preindustrial 3-5% (Rome 5, China 5, medieval at most 5); late 18th century fiscal-military states 9-24; modern low 13-14, lower-middle 18, upper-middle 22, high 31-33, Nordic higher.
- Real band: tau0 0.03-0.05, tau1 0.30-0.40, y_half calibrated so y at subsistence gives about 5% and y at the low-income line gives about 13%.
- Sandbox band: tau0 0.01-0.10, tau1 0.20-0.55.

**E2. Revenue.**
`R = tau_cap(y) * c * f(L) * Y`, with `f(L) = 0.5 + 0.5 L` (design).
`c` is the share of the ceiling the state actually reaches. Weak states sit at c of 0.3-0.6 (Ottoman tax-farming leakage is the qualitative anchor; no number found). Real band c 0.3-1.0; sandbox 0.05-1.0.

**E3. Army size and cost.**
`A_max_pop = m * P` and `A_max_fiscal = (f_army * R) / (k_pay * w)`; `A = min(A_max_pop, A_max_fiscal)`.
- `m` (sustained) preindustrial 0.005-0.01 (rule of thumb 0.5%, Rome 0.7%, peacetime 1900 powers 0.7-2%). `m` (peak, years) 0.05-0.10 (Rome 5%, WWII 7-10%, WWI to 15%, France 21% peak).
- `k_pay` about 0.6-0.75 of a labourer's annual wage plus upkeep (our arithmetic on Rome; low confidence). `w` is the local wage.
- `f_army` of R: Rome 0.5-0.8; real band 0.1-0.8; sandbox 0.02-0.95.
- Soldiers leave the labour pool: `Y` falls by `A * (productivity per worker)`, which is why industrial mobilisation above about 10% hurts output.

**E4. Bureaucracy.**
`B = b(y) * P`, with `b` about 0.0008 (8 per 10,000) for Rome-like empires and up to 0.18 of employment (OECD average) for rich states, rising with y (Wagner-style, design curve through those two points). Real band for preindustrial b 0.0003-0.001; sandbox 0.0001-0.05.
Literate officials are limited by pop supply: `B <= literacy_share * P * recruit_share`.

**E5. Levels and reach.**
`levels = ceil( log(P / P_local) / log(s) )`, with span `s` 5-10 (Graicunas 5-6; typical 3-15) and `P_local` the size one chief can run (design, 50-500).
Reach radius `r = v * T`, with `v` the message speed (Rome about 46 miles/day; urgent relay 170) and `T` the longest delay the centre tolerates (design, 15-60 days). Territory beyond `r` needs a delegated tier (Taagepera's satraps) and costs one more level and a legitimacy-loss risk (E7).

**E6. Capacity from the pops.**
`c = clamp( a1 * B/B_need + a2 * literacy_of_officials + a3 * infra_reach - a4 * corruption )`, with `B_need = P / (s_eff * levels_factor)`; all `a` are design weights (sum to 1). This is the loop's core: pops supply B, literacy and the tax base; policy spending feeds back into infra, literacy and B.

**E7. Stability, structural-demographic form.**
Turchin's structural-demographic theory has three drivers: mass immiseration, elite overproduction and state fiscal distress, combined in a Political Stress Index (https://www.babypips.com/forexpedia/structural-demographic-theory, summary; exact formula not retrieved). Our version:
`stress = M * E * F` with
- `M = real wage now / real wage at start` inverted (falling wage raises M);
- `E = elite aspirants / elite positions`;
- `F = max(0, (spending - revenue) / revenue)` or debt service share.
`dL/dt = r_L * (services_and_security_score - stress) ` clamped to [0,1] (design).

**E8. Population and growth.**
Population follows a Malthusian pull on y (Turchin-Korotayev: population and internal war oscillate with a lag). `dP/dt = g(y) * P`, with `g` falling to zero near subsistence. Economic growth bonus when `tau_eff` exceeds 12.75-15% of GDP: +7.5% GDP per capita over 10 years (about 0.7% per year), gated by `c` (IMF).

**E9. When a region becomes a state, splits or collapses (rules, not formulas).**
- Becomes a state when all hold: P over about 50,000 (Diamond), a storable surplus crop above a threshold (Scott), at least one neighbour in competition and the region circumscribed (Carneiro: no free exit for losers), and a war or trade pressure above zero for N years (Tilly, Turchin).
- Territory is capped at the reach radius `r` per level of delegation (E5).
- Splits when `L < L_split` for a province or its distance exceeds `r` without a delegated tier.
- Collapses when `F` and `E` stay high and `L < L_min` for a period near a secular half-cycle (100-300 years, Turchin and Nefedov); afterwards it falls to the tier below (empire, kingdom, chiefdom) rather than to zero.
- Start-up of a generated world: seed states from circumscribed, high-surplus regions; count of states follows the area and population power laws (section 1.2) and the timeline of state counts (27 in 1800 to 193 in 2026).

## 3. How this closes the loop with pops and nations

- Pops are the only source of revenue, army and officials: `Y` from workers, `R = tau * c * Y`, `A <= m*P`, `B <= literate share * P`, so a nation never has a stat that is not a head count of someone.
- Soldiers and officials leave the labour pool, so a bigger army lowers `Y` and then `R`: the cost of power is visible in the economy, as in the E3 note on production above about 10% mobilised.
- Policy spending (services, schools, roads, order) feeds back into pops: health and education raise `y` and literacy, roads raise reach `r`, security lowers `M` and raises `L`.
- Fiscal distress `F`, elite surplus `E` and falling wages `M` multiply into stress, which drives legitimacy `L` and therefore `c` (the state collects less when trusted less).
- Growth beyond the tax tipping point (about 13-15% of GDP) is only reachable if capacity `c` is high enough, which ties a nation's development path to its bureaucracy.
- Reach (message speed times tolerated delay) caps size per administrative tier, so a larger nation needs more levels, more officials and more tax, and breaks at the edges first.
- Regions become states only when pops, surplus and pressure cross thresholds; collapse returns them to a lower tier with the same pops, so history is a loop and not a script.
- Secular cycles (100-300 years) should appear from E3-E8 without being scripted; if they do not, that is a calibration test, not a feature to fake.

## 4. Unverified / not found

- Primary texts were not read (WebFetch blocked). Every number is a search-summary number; check primary sources before they go into a design doc.
- Not found: levels of government by population size; bureaucrat-to-population ratios beyond Rome (and its population figure is itself a secondary estimate); Han and Qing ratios (populations not retrieved); trust-in-government by income or size; fragility versus state size; area-per-state rule (number of states a given land area holds); mean and median country population and area; a quantitative formula for Carneiro; Turchin's published political stress index formula; per-country WWII percentages confirmed from a primary table; cost per soldier relative to wages for any era except Rome (disputed wage).
- Low-confidence: the 27-state (1800) and about-50-state (1900) counts from an aggregator; the preindustrial 0.5% army rule from a forum; Roman army budget share (50-80%, sources disagree); 18th-century tax shares for France and Britain are given on different bases (GDP versus commodity production).
- Empire sizes are contested (Taagepera's lists criticised at https://www.bookandsword.com/2020/01/25/be-careful-with-rein-taageperas-lists-of-largest-empires/).
- All parameters marked "design" in section 2 are ours and have no source.

## 5. Data licences

We use only public-domain or CC BY data.

| Dataset | Licence | Usable? | Source URL |
| --- | --- | --- | --- |
| Our World in Data (data it produces) | CC BY; most series come from third parties under their own terms, check each | Yes for OWID-produced series; check the original per series | https://ourworldindata.org/faq |
| Maddison Project Database 2023 (historical GDP per capita) | CC BY 4.0 | Yes | https://www.rug.nl/ggdc/historicaldevelopment/maddison/releases/maddison-project-database-2023 |
| UN World Population Prospects | CC BY 3.0 IGO | Yes | https://www.un.org/development/desa/pd/node/4245 |
| OECD Government at a Glance | OECD terms not retrieved | Not confirmed; use only the cited figures with attribution until checked | https://www.oecd.org/en/publications/government-at-a-glance-2025_0efd0bcd-en/full-report/employment-in-general-government_dafcfac5 |
| World Bank open data (tax revenue, WDI) | licence not retrieved in search; commonly CC BY 4.0 (unverified) | Not confirmed | not found |
| IMF Government Revenue Dataset / WoRLD | licence not retrieved | Not confirmed | https://data.imf.org/en/Datasets/WORLD/WoRLD-2026-Update |
| Correlates of War state system membership | "Correlates of War terms", no standard licence named | Not confirmed (ask or avoid redistributing) | https://correlatesofwar.org/data-sets/ |
| Gleditsch-Ward state list | licence not retrieved | Not confirmed | http://ksgleditsch.com/data-4.html |
| Fragile States Index | CC BY-NC-SA 4.0 | **No** (non-commercial, share-alike) | https://fragilestatesindex.org/?p=81 |
| Seshat Global History Databank | CC BY-NC-SA | **No** (non-commercial) | https://datahub.io/@rufuspollock/seshat |

Recommendation: use the published numbers in section 1 as cited facts (facts are not copyrightable, but cite them) and ship only our own derived parameter tables; do not ingest Seshat or the Fragile States Index. If a dataset is wanted later, Maddison and UN WPP are the confirmed-open options.
