# Absence, no-shows and missing persons: how the world notices someone is gone

**Date:** 2026-10-10. **Status:** research note, not a decision. **Feeds:** [cases and living lore](../design/cases-and-living-lore.md) (§14 strange cases, §15 absence), [inhabitants](../design/inhabitants.md), [spec 24](../tier0/specs/24-pop-ai.md), [spec 39](../tier0/specs/39-orders-and-belief.md), [spec 33](../tier0/specs/33-establishments-and-chains.md), [0069](../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md). **Evidence:** one research pass on 2026-10-10 (11 web calls; two failed). The UK National Crime Agency missing-persons report 2022/23 was read in full from its PDF text; the BLS absence table and other pages were read directly; the rest came from search summaries, marked `reported`. College of Policing risk definitions, return interviews and UK unauthorised-absence law are listed as known but not read. **IDs:** AB.

The user (2026-10-10): if a player possesses a police officer, takes him off his schedule and leaves him in another city, "there must be consequences: people asking and looking for him". The proposed mechanism is that every commitment in a schedule creates expectations in others, and a broken expectation is an event. This note gives that chain real numbers and procedures.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| AB-1 | US, 2025 annual averages (11 months, October excluded): 3.2 % of full-time wage and salary workers had an absence in the reference week (2.2 % illness or injury, 1.0 % other reasons); lost worktime 1.7 % of usual hours (1.1 illness, 0.6 other). | sourced | BLS CPS Table 47, https://www.bls.gov/cps/cpsaat47.htm |
| AB-2 | US 2025, protective service occupations: absence rate 2.9 % (2.0 illness, 1.0 other), lost worktime 1.8 %. Police are not a separate row. | sourced | BLS CPS Table 47 (as AB-1) |
| AB-3 | US 2025 absence rate by occupation group: management 2.5 %, production 3.4 %, transportation and material moving 3.4 %, office and administrative support 3.9 %, healthcare support 4.1 %. | sourced | BLS CPS Table 47 (as AB-1) |
| AB-4 | BLS definition: an absence is a usually full-time worker working under 35 h in the reference week for illness, injury, child care, family or personal obligations, civic or military duty, or parental leave; vacation, holidays and labour disputes are excluded. | sourced | BLS CPS Table 47 footnotes (as AB-1) |
| AB-5 | SHRM sample job-abandonment policy: missing 3 consecutive business days without notifying the company counts as voluntary resignation; the employer considers extreme circumstances (medical emergency, natural disaster) and the explanation's timing. | reported | SHRM sample policy, https://shrm.org/topics-tools/tools/policies/job-abandonment-policy |
| AB-6 | US: no federal law sets the number of no-call/no-show days; 3–5 full business days is a common measure; state case law or unemployment agencies may set what is reasonable. | reported | SHRM HR Answers, https://shrm.org/topics-tools/tools/hr-answers/considered-job-abandonment |
| AB-7 | SHRM cautions that not every no-call/no-show is abandonment: employees may be unable to call (medical crisis, incarceration); medically related absence may be protected (FMLA, ADA); the employer should use those days to find out what is happening. An attorney quoted: employers tend to be forgiving if the employee does call. | reported | SHRM HR Magazine 2001, https://www.shrm.org/hr-today/news/hr-magazine/pages/0401frase-blunt.aspx ; SHRM, https://shrm.org/in/topics-tools/employment-law-compliance/to-no-call-no-show-employees-reappear |
| AB-8 | Neither California nor US federal law defines "job abandonment"; the number of days is whatever the employer's written policy says. | reported | CalChamber HR Library, https://p-hrcalifornia.calchamber.com/hr-library/qa/california-employee-call-show-work-days-row-meet-legal-definition-job-abandonment |
| AB-9 | England and Wales 2022/23: 312,901 missing-person-related calls to police (35 forces); +10 % on like-for-like forces (278,105 → 306,013). Scotland 17,599 calls (−43 %). | sourced | NCA UK Missing Persons Unit, Missing Persons Data Report 2022/23, https://tnaqa.mirrorweb.com/ukgwa/20260803081315mp_/https://www.nationalcrimeagency.gov.uk/images/mpu/downloads/UKMPU%20Annual%20Data%20Report%202022-23.pdf |
| AB-10 | England and Wales 2022/23: 319,745 missing incidents involving 156,454 individuals (about 2.0 incidents per individual). | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-11 | England and Wales 2022/23: children were 63 % of incidents (200,704) but 45 % of individuals (69,927); adults 55 % of individuals (86,189). Derived: ~2.9 incidents per missing child, ~1.4 per missing adult. | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-12 | Scotland 2022/23: 17,409 incidents (58 % children), 10,454 individuals (41 % children). Northern Ireland: 9,581 incidents (54 % children), 4,284 individuals (27 % children). | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-13 | England and Wales 2022/23 incidents: ages 12–17 are 60 %; 85 % are under 40. Male 54.3 %, female 43.6 %, trans 0.4 %, unknown 1.7 %; the 40–59 bracket is 65.1 % male. | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-14 | Risk level of UK incidents 2022/23: medium 75 %; high about 19 %, low about 6 % (pie-chart labels). By nation (chart labels): England and Wales 74 / 20 / 6 % (medium / high / low); Northern Ireland high 7.7 %; Scotland high 12.1 %. Forces differ widely: Staffordshire 59 %, Cheshire 73.8 % high. | sourced | NCA UKMPU 2022/23 (as AB-9) (chart values read from labels) |
| AB-15 | Adults are graded high risk more often than children: 31 % against 14 % of incidents; medium is 58 % for adults and 83 % for children (so low plus very low is about 11 % for adults and 3 % for children, derived). | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-16 | The College of Policing renamed the risk category "no apparent risk" to "very low risk" (stated in the NCA report). | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-17 | Police Scotland uses a risk-based definition: missing is "anyone whose whereabouts is unknown" and the circumstances are out of character, or the context suggests a crime, or the person is at risk of harm to self or others. | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-18 | UK police weigh a person's Article 8 right to choose to go missing against the duty to safeguard; a national debate is open on how much enquiry is needed before closing a case where an adult chose to leave and can keep safe. | sourced | NCA UKMPU 2022/23, foreword (as AB-9) |
| AB-19 | 2022/23: 54,288 missing incidents carried a mental-health flag (England, Wales, Scotland), 63 % of them adults; 14,593 carried a child-sexual-exploitation flag (England, Wales, Northern Ireland), 84 % female. | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-20 | 2022/23: 0.30 % of missing incidents ended in a fatal outcome (971 deaths, 1 in 330); 96 % were adults (934), 4 % children (36); no cause-of-death data. Derived: about 0.7–0.8 % of adult incidents, under 0.02 % of child incidents. | sourced | NCA UKMPU 2022/23 (as AB-9) (derived shares are ours) |
| AB-21 | 88.3 % of missing people were found within 48 hours in England and Wales, 95.3 % in Scotland (2022/23). Derived for adults in England and Wales: about 85 % within 48 h (assumes the 88.3 % and 90 % child figures share one base; rough). | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-22 | Children, England and Wales 2022/23: 78 % of incidents resolved within 24 h, 90 % within 48 h. Scotland: 88 % and 97 %. Duration bands in the report: 0–8 h, 8–16 h, 16–24 h, 24–48 h, 2–7 d, 7–27 d, 28+ d (adult per-band values only in the chart, not read). | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-23 | Still missing after 28 days: England and Wales 0.5 % of children and 1.2 % of adults; Scotland 0.1 % and 0.3 %. | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-24 | Who finds missing people, England and Wales (30 forces): returned of their own free will 39 %, found by police 33 %; the other 28 % split among family, carer or local authority, friend and other (chart labels 13, 10, 4, 1 %; which is which not readable). Scotland: police 61 %, own free will 24 %. | sourced | NCA UKMPU 2022/23 (as AB-9) (partial) |
| AB-25 | Children returned of their own free will in 43 % of incidents in England and Wales, 27 % in Scotland. | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-26 | Reasons (selected England and Wales forces, share of incidents with a stated reason): the top four (mental health, relationship, drugs or alcohol, depression or anxiety) make 56 %. Mental health is the top reason for adults (15.6 %); relationship for children (9.3 %). | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-27 | Long-term missing individuals at 31 March 2023: England and Wales 4,102 (2,622 adults, 1,477 children, 3 unknown); Scotland 60 (48 adults, 12 children). England and Wales individuals reported in 2022/23 still missing at year end: 1,402; excluding those reported in the last 28 days: 928. | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-28 | Children in care are 20 % of missing children in England and Wales (12,616 individuals) but 36 % of child incidents (65,231): repeat missing concentrates in a few. | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-29 | The UKMPU holds 1,033 unidentified cases (cumulative since the 1950s): 814 bodies, 159 body parts, 60 people found alive; people found alive but unidentified are often injured or have amnesia or dementia. | sourced | NCA UKMPU 2022/23 (as AB-9) |
| AB-30 | US 2024: 533,936 missing-person records entered into NCIC; 537,446 purged in the same year (subject located, returned home, or record invalid). | reported | FBI, 2024 NCIC Missing and Unidentified Person Statistics, https://www.fbi.gov/file-repository/cjis/2024-ncic-missing-and-unidentified-person-statistics.pdf |
| AB-31 | US NCIC active missing-person records on 31 Dec 2024: 93,447; under 18: 25,493 (27 %); under 21: 34,256 (37 %). On 31 Dec 2023: 96,955 active, 29,451 under 18 (30 %). | reported | FBI 2024 and 2023 NCIC statistics (as AB-30; https://le.fbi.gov/file-repository/2023-ncic-missing-person-and-unidentified-person-statistics.pdf) |
| AB-32 | US 2024: the Missing Person Circumstances field was filled in 48 % of NCIC entries (258,849); 95 % of those (245,818) were coded runaway. | reported | FBI 2024 NCIC statistics (as AB-30) |
| AB-33 | US law requires agencies to enter missing people under 21 into NCIC (34 U.S.C. § 41307(a)); NCIC entry categories include Disability, Endangered, Involuntary, Catastrophe Victim and Juvenile. | reported | FBI 2024 NCIC statistics (as AB-30) |
| AB-34 | Canada 2024: 67,611 missing-person occurrences entered on CPIC (−4 % from 70,168 in 2023; 8 % below 2019); tables give 35,109 adults and 32,680 children (the page's totals disagree by under 1 %). | sourced | RCMP NCMPUR, Canada's Missing 2024 Fast Fact Sheet, https://canadasmissing.ca/pubs/2024/index-eng.htm |
| AB-35 | Canada 2024: missing adults are 55.7 % male; missing children 58.2 % female. | sourced | Canada's Missing 2024 (as AB-34) |
| AB-36 | Canada 2024, adults by probable cause: unknown 62.9 %, runaway 16.0 %, other 11.9 %, wandered off or lost 8.6 % (63 % male); accident 57, stranger abduction 41, presumed dead 31, human trafficking 31 (counts). | sourced | Canada's Missing 2024 (as AB-34) |
| AB-37 | Canada 2024, children by probable cause: runaway 71.1 % (57 % female), unknown 22.5 %, other 4.6 %; wandered off 391, parental abduction 116, stranger abduction 24 (counts). | sourced | Canada's Missing 2024 (as AB-34) |
| AB-38 | Canada 2024, time to removal from CPIC: adults 32 % within 24 h, 72 % within a week; children 55 % within 24 h, 90 % within a week. 1 % (509) of 2024 occurrences were still open on 10 Feb 2025. | sourced | Canada's Missing 2024 (as AB-34) |
| AB-39 | Canada, earlier years (adults removed within 24 h / within a week): 2019 62 % / 90 %; 2021 60 % / 89 %; 2022 34 % / 73 %. Children 2021: 67 % / 92 %. The drop from 2021 to 2022 is not explained on the pages. | reported | Canada's Missing fast fact sheets 2019, 2021, 2022, https://canadasmissing.ca/pubs/2022/index-eng.htm |
| AB-40 | Canada: research cited by the RCMP suggests 25–75 % of missing-person reports are never entered on CPIC, mostly people found within hours; the share varies by province. Removal is a database event and can lag the real return. | sourced | Canada's Missing 2024 (as AB-34) (secondary claim on the page) |
| AB-41 | Canada, open occurrences across all years on 31 Dec 2024: 8,346 (oldest 1919); by cause: unknown 2,996, accident 1,884, runaway 912, other 785, presumed dead 750, wandered off 658, stranger abduction 140. | sourced | Canada's Missing 2024 (as AB-34) |
| AB-42 | No law requires a 24-hour (or 48-hour) wait before reporting an adult or a child missing (US state and Toronto police guidance); waiting is said to delay the investigation. The belief persists mainly for adults, and adult intake procedures vary by agency. | reported | Toronto Police information sheet, https://www.tps.ca/files/download/1762449637/45642/ ; azfamily 2024, https://www.azfamily.com/2024/03/15/common-misconception-with-filing-missing-persons-report-waiting-24-48-hours-file-it ; Brainerd Dispatch, https://www.brainerddispatch.com/news/the-vault/the-myth-of-the-24-hour-rule-how-the-false-belief-damages-missing-persons-investigations |
| AB-43 | "Geographies of Missing People" (Parr, Fyfe, Stevenson, Woolnough): interviews with 45 adults aged 18–79 reported missing in 2009–2011 (Grampian and London); its findings fed the College of Policing APP on missing persons (2016) and Police Scotland's procedures (2015). Their 2016 work covers families "living with ambiguous absence". | reported | University of Glasgow news 2013, https://www.gla.ac.uk/news/archiveofnews/2013/june/headline_280827_en.html ; REF 2021 impact case, https://results2021.ref.ac.uk/impact/44ea9bd5-971a-48e4-8b0d-59aa8ef619b7/pdf |
| AB-44 | Dissociative fugue: DSM-5-TR classes it as a type of dissociative amnesia; uncommon; sudden, purposeful travel away from home or bewildered wandering; lasts "from hours to months, occasionally longer". | sourced | MSD Manual Professional, Dissociative Amnesia, https://www.msdmanuals.com/professional/psychiatric-disorders/dissociative-disorders/dissociative-amnesia |
| AB-45 | Brief fugues may look like missed work or coming home late; long ones can mean travelling far, taking a new name and a new job without awareness; during a fugue the person may look normal or mildly confused. | sourced | MSD Manual (as AB-44) |
| AB-46 | A fugue ends when the person suddenly finds themselves in a new place with no memory of how they got there; shame, discomfort, grief, depression or fear follow; the diagnosis is usually made afterwards from the trail. | sourced | MSD Manual (as AB-44) |
| AB-47 | Dissociative amnesia: prevalence estimates range 0.2–7.3 % (population and period not stated); likely underdetected; most patients recover the memories; suicidal risk may rise when amnesia resolves suddenly. | sourced | MSD Manual (as AB-44) |
| AB-48 | Fugue triggers: trauma, severe stress (including financial trouble), guilt, unresolvable interpersonal conflict, criminal behaviour; fugue as an escape from distress or embarrassment, often family conflict. | sourced | MSD Manual (as AB-44) |

**Rates per head (derived, denominators not fetched):** England and Wales 2.6 missing individuals and 5.4 incidents per 1,000 people per year (2021 Census population 59.6 million). Canada about 1.6 occurrences per 1,000 (population about 41 million). US about 1.6 NCIC entries per 1,000 (about 340 million). The UK counts every incident, including repeats; the US and Canada count database entries.

### Known but not read in this session (verify before use; no number above depends on them)

- College of Policing APP, Missing persons: "anyone whose whereabouts cannot be established will be considered as missing until located and their well-being or otherwise established"; risk tiers high (serious harm very likely, immediate deployment), medium (harm likely but not serious), low (harm possible but minimal), very low (formerly "no apparent risk", formerly "absent"). The fetch returned 403.
- UK statutory guidance on children who run away or go missing from home or care (DfE 2014): an independent return interview within 72 hours of return. Police do a "prevention interview" or "safe and well check" on return.
- UK (ACAS): no automatic resignation in law; the employer should try to contact the employee, investigate and follow a fair disciplinary process before dismissing for unauthorised absence.
- Australia (AFP National Missing Persons Coordination Centre): about 38,000 reports a year, most found quickly (page returned 404).
- Ambiguous loss (Pauline Boss); ICRC Restoring Family Links and the Central Tracing Agency.

## The reaction chain, with timings

Scenario: a police officer, married, lives with his household, works a weekday day shift. A player possesses him at home on Monday before his shift, drives him to another city and releases him on Thursday. Timings marked **(D)** are defaults taken by Claude where no source was found; the rest cite findings.

| When (from the first broken expectation) | Who | What they notice and believe | What they do | Basis |
| --- | --- | --- | --- | --- |
| Before the shift | The pop (if the player phones in) | — | A call ("sick") turns the absence into a normal one: about 3 % of full-time workers are absent in any week, 2 % of them for illness. No chain starts. | AB-1, AB-2, AB-4 |
| Shift start, 0–30 min | Supervisor (employer's expectation) | "Absent, no call." A no-call is the anomaly, not the absence. | Phones the officer **(D)**. | AB-1, AB-7 |
| +1–3 h | Supervisor | Phone unanswered; for a police officer, a welfare risk (means, stress). | Phones the emergency contact (spouse); sends a unit to the home **(D)**. | AB-7; welfare-check practice not found (gap) |
| +1–3 h | Spouse (warned by the employer) | "He left for work." Out-of-character absence. | Phones him, his friends and colleagues; checks hospitals; posts on social media **(D)**; no source quantified these steps. | gap |
| Same day, within hours | Spouse or employer | Out of character + failed contacts. | Reports him missing. No waiting period applies. | AB-42, AB-17 |
| At the report | Police intake (home force) | Risk tier: adults are 31 % high, 58 % medium, ~11 % low or very low. Out-of-character, mental-health signs, means of harm push to high. | High: immediate search, supervisory oversight **(D from APP, to verify)**; entry on the national database, so any police check in another city hits the record. | AB-14, AB-15, AB-16, AB-30, AB-33, AB-40 |
| +24 h | — | Base rate if nobody drove him: about a third to two thirds of adults are back or found (Canada 32 % in 2024, 60–62 % in 2019–2021, by database removal). Children: 78 % in England and Wales. | — | AB-38, AB-39, AB-22 |
| +48 h | — | About 85 % of adults and 90 % of children are found (England and Wales). | Search continues; the case is reviewed. | AB-21, AB-22 |
| 3 business days without contact | Employer | In a private US workplace the job-abandonment threshold (3, commonly 3–5 days). | Suspended: a missing-person report and the family's contact are the "unable to call" exception. In the UK, no automatic resignation. | AB-5, AB-6, AB-7, AB-8 |
| Thursday: release in another city | The pop | Comes to: a strange city, days gone (cases §14 dissonance decides the account). Like the end of a fugue: confusion, shame, fear. | Phones home or police, or travels home; 39 % of missing people return of their own free will, 33 % are found by police (England and Wales). A local check on him hits the national record. | AB-24, AB-44, AB-45, AB-46 |
| +7 days (if not released) | — | Adults 72–90 % resolved (Canada); children 90–92 %. | — | AB-38, AB-39 |
| +28 days | Police | 1.2 % of adults still missing become long-term missing in England and Wales. | Case stays open; records cross-matched against unidentified found people. | AB-23, AB-27, AB-29 |
| On return, first days | Police | Case closed. | Return or safe-and-well interview **(to verify)**: why he left, harm suffered, risk of repeat. | known, not read |
| On return | Employer | Absence explained or not. | Return-to-work meeting; discipline if no good reason, forgiveness if the cause is credible. For police, a conduct process for absence without leave **(D)**. | AB-7 |
| On return | Household and others | They seek a reason from their priors: mental health (top adult reason, 15.6 %), relationship, drink or drugs, depression. In this world, infection also. | Watch him; strained bond; post-return suicide risk if memories return suddenly (fugue). | AB-26, AB-47 |
| Outcome odds without a player | — | Fatal outcome in about 0.7–0.8 % of adult incidents. | — | AB-20 |

## What this means for us (Proposal)
1. **Expectations are records made from the schedule** (spec 40, spec 24, decision 0069 rule 5). Each commitment in a pop's schedule (shift start, arrival home, a meeting) names its holders and a tolerance. Only a broken one becomes an event; kept ones cost nothing. Data shape, `data/life/expectations.toml`:
   ```toml
   [[expectation]]
   commitment = "shift_start"
   holder = "employer.supervisor"
   tolerance_min = 30          # (D)
   first_act = "phone"
   escalate_after_min = 120    # (D) to emergency contact; police employers add a home visit
   [[expectation]]
   commitment = "arrive_home"
   holder = "household"
   tolerance_min = 60          # (D) shrinks when the pop is out of character
   first_act = "phone"
   ```
2. **A call turns a no-show into an ordinary absence** (spec 39 beliefs, spec 33 shifts). Holders' prior for "absent" comes from absence rates by occupation (AB-1 to AB-3): a call makes "sick" believed and nothing escalates. A no-call is the anomaly. Careful role-play (phone in) leaves no trace, as cases §14 asks. Data: `data/life/absence_rates.toml` with weekly absence share and lost-hour share per occupation group and country.
3. **The escalation ladder is the same for every holder** (spec 24): phone → ask around (friends, workplace, hospitals) → report → search. Anyone may report at any time; there is no waiting period (AB-42). What varies is the holder's tolerance and the pop's out-of-character score.
4. **A missing-person report is a police case with a risk tier** (decision 0069 rule 3, institutions as queues). Tier shares by adult and child (AB-14, AB-15) set staff time and search radius. The home force owns the case; a national record (NCIC or CPIC style) makes any force's check on that pop a hit (pop-lore §7 jurisdiction). Data: `data/institutions/police/missing.toml` with tier shares per country and the modifiers (out of character, mental-health flag, means of harm, age, dementia).
5. **Real missing people exist without players** (decision 0069, spec 24, spec 46/47), so possession hides among them. Base rates per country:
   ```toml
   [missing.gb_ew]                  # NCA UKMPU 2022/23
   individuals_per_1000_year = 2.6
   incidents_per_individual = { adult = 1.4, child = 2.9 }
   child_share_individuals = 0.45
   risk_tiers = { adult = { high = 0.31, medium = 0.58, low_or_very_low = 0.11 }, child = { high = 0.14, medium = 0.83, low_or_very_low = 0.03 } }
   resolved_cdf.child = [[24, 0.78], [48, 0.90], [672, 0.995]]   # hours, share
   resolved_cdf.adult = [[48, 0.85], [672, 0.988]]               # 48 h derived, rough
   fatal_share = { adult = 0.0075, child = 0.0002 }              # derived from AB-20
   found_by = { own_will = 0.39, police = 0.33, other = 0.28 }
   [missing.ca]                     # Canada's Missing 2024
   adult_cause = { unknown = 0.629, runaway = 0.160, other = 0.119, wandered = 0.086 }
   resolved_cdf.adult = [[24, 0.32], [168, 0.72]]                # database removal, lags
   resolved_cdf.child = [[24, 0.55], [168, 0.90]]
   ```
   Reasons for adults draw from AB-26 and AB-36; children are mostly runaways (AB-32, AB-37) and repeat missing concentrates in children in care (AB-28).
6. **Job abandonment is an employer rule, with a country default** (spec 46/47, spec 33). After N consecutive scheduled days missed without any contact, the employer records a separation. US default N = 3 business days (range 3–5, AB-5, AB-6); UK: no automatic resignation, a disciplinary process instead (to verify). The rule pauses while a missing-person report or a family contact explains the absence (AB-7). Police employers open a conduct case after return instead. Data: `data/employment/absence_policy.toml` per country and employer kind.
7. **On release the pop tries to get home** (spec 24): orient, phone household or police, travel home. A police check in the other city hits the record and the local force hands him back (AB-24 sets how often return is self-made against police-found when no player is involved).
8. **The return has three meetings** (spec 39, cases §14): police (safe-and-well interview, to verify), employer (return to work; outcome from the explanation's credibility) and household (bond strain). The pop's account follows cases §14 dissonance. High-dissonance "missing time" maps onto fugue's real shape (AB-44 to AB-46): it comes to in another place, feels shame or fear, and carries a short-lived risk flag (AB-47). Others explain it from their priors, led by mental health and relationship trouble (AB-26); "infection" joins them in this world.
9. **Disasters multiply the same chain** (spec 39, spec 24): many broken expectations at once, phone networks down, institutions overloaded. Tracing and ambiguous loss were not researched here (gap); until then, the same ladder runs with longer tolerances and longer queues.

## Gaps

- **How long families wait before reporting, and what they do first** (calling hospitals, social media, search parties): not found with numbers. Defaults taken above.
- **Police officers who fail to report for duty:** no source found on welfare checks or departmental absence-without-leave procedures (US or UK). Defaults taken.
- **College of Policing APP text** (risk definitions, response by tier, return interviews): fetch refused (403); only the NCA report's mention was read.
- **UK employment law on unauthorised absence (ACAS)** and other countries: not fetched.
- **Adult duration bands for England and Wales** (0–8 h to 28+ d): in a chart whose values were not readable; only child figures and the overall 48-hour figure are in the text.
- **Australia** (AFP statistics page 404) and the AIC research on missing persons in Australia: not read.
- **NCIC 2024 PDF:** numbers come from a search summary, not the PDF itself.
- **Fugue prevalence and typical duration in numbers:** the MSD Manual gives only "hours to months"; DSM-5-TR figures not read. Dissociative amnesia prevalence 0.2–7.3 % has no stated population.
- **Return home interviews, stigma, effects on employment and family after return:** not found with numbers.
- **Disasters:** ICRC Restoring Family Links caseloads, reunification times and ambiguous-loss duration not researched (budget).
