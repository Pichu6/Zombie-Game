# Institutions as queues: how police, hospitals, courts and emergency services process their work

**Date:** 2026-10-10. **Status:** research note, not a decision. **Feeds:** [0069](../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md) rule 3, [spec 33](../tier0/specs/33-establishments-and-chains.md), [spec 39](../tier0/specs/39-orders-and-belief.md), [spec 40](../tier0/specs/40-time.md), [pop lore](../design/pop-lore-and-custom-pops.md) §7 (caseloads), [demo flow](../design/demo-flow.md) (fourth flow) (beat 16, the detective's desk). **Evidence:** one research pass on 2026-10-10 (16 web calls). Primary PDFs (NCVS 2024, FBI 2024, NHAMCS 2022, Vera 2020) were converted locally and their numbers copied from the tables; the UK crime-outcomes page was read through an extraction tool. The Larson and Erlang queueing assumptions are background knowledge, not read this session, and are marked `reported`. **IDs:** IQ.

Decision 0069 says institutions are queues with staff: cases arrive, wait, are served and end in outcomes, and "things are slow and take time" because of it. This note gives that mechanism real intake rates, priorities, service times and outcomes.

## Findings

### Police: intake (how many cases arrive, and of what kind)

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| IQ-1 | Tucson, AZ, 2016 (population 527,586): 283,245 calls for service (CFS) placed by the public through 911 (71.1% of computer-aided dispatch, CAD, events), 114,696 officer-initiated events (28.8%) and 714 walk-ins (0.2%). That is 398,663 CAD events in all, or 75,564 per 100,000. *Derived:* **537 public 911 CFS and 217 officer-initiated events per 1,000 residents per year.** | sourced | Vera, *Understanding Police Enforcement: A Multicity 911 Analysis* (2020), fig. 5.2.6A. [PDF](https://vera-institute.files.svdcdn.com/production/downloads/publications/understanding-police-enforcement-911-analysis.pdf) |
| IQ-2 | Tucson 911 CFS rose from 283,245 (2016) to 317,827 (2017), +12%, or 602 per 1,000 (derived). | sourced | Vera 2020, fig. 5.2.6A |
| IQ-3 | Camden County PD (Camden city, NJ, population about 76,005), 2016: 70,320 911 calls dispatched, of which 63,870 were phone calls. All CAD events numbered 280,202 (228,700 in 2017), most of them police-initiated (85,218) or motor-vehicle stops (20,720). *Derived:* **925 911 calls and 3,687 CAD events per 1,000 residents per year.** This is a small, high-crime city with very heavy proactive policing. | sourced | Vera 2020, figs. 5.2.1A, 5.2.2A and text p.136 |
| IQ-4 | **Mix of 911 calls, Tucson 2016 (% of 283k):** total crime 33.3%, made up of alarms 4.9, violent crimes 1.0, domestic violence 7.3, property 8.3 and other crimes 11.8 (disorderly conduct, animal cruelty and the like). Outside "total crime": sex offences 0.5, drugs 0.8, call-related issues (hang-ups, abandoned calls) 19.2, complaints and environmental conditions 10.2, officer status 9.1, assisting the public 9.1, accidents and traffic 6.1, suspicion 5.4, callback 2.1, behavioural health 1.5, missing persons 1.4, other (not crime) 1.2, warrants 0.2, liquor 0.1, medical 0.0, fire 0.0. | sourced | Vera 2020, fig. 5.2.4B |
| IQ-5 | **Mix of 911 calls, Camden 2016 (% of 70,320):** total crime 27.2%, made up of alarms 7.7, violent 1.6, domestic violence 6.7, property 3.3 and other crimes 7.9. Outside it: service assignments and statuses 30.1, complaints and environmental conditions 22.9, health 4.1, suspicion 3.7, traffic 3.7, behavioural health 2.2, reports 1.4, property check 1.1, proactive 1.0, missing persons 0.9, missing code 1.9. 2017 was nearly identical (crime 26.7%, violent 1.4%). | sourced | Vera 2020, fig. 5.2.1B |
| IQ-6 | A nine-city 911 study found that "no more than 7 percent of calls" involve violent crime, and that on average 19% of calls could be answered by unarmed crisis responders. | reported | Vera, *911 Analysis* (Apr 2022). [page](https://www.vera.org/publications/911-analysis) |
| IQ-7 | A 2002 review of US CAD data: "less than 20 percent of the citizen calls" are for serious crime incidents. Vera's own reading of the Camden and Tucson CAD data is that about 75% of calls are unrelated to crimes in progress. | reported | Cited in Vera 2020, p. 12 note 5 and literature review |
| IQ-8 | **Priority mix of 911 calls.** Each department defines its own levels, so the shares are not comparable across departments.<br>Tucson 2016: P1 0.7%, P2 15.9%, P3 29.4%, P4 28.2%, P7 6.4%, P8 1.4%, P9 17.6% (P9 is callback or alternative response with no evidence, witness or suspect).<br>Camden 2016: P1 18.8%, P2 29.8%, P3 8.7%, P4 28.2%, P5–9 14.2%. | sourced | Vera 2020, figs. 5.2.1A and 5.2.4A |
| IQ-9 | **Time of day, Camden 911 calls 2016 (% of the day's calls in each hour):** lowest at 04:00–06:00 (1.3–1.4%); rising from 07:00; highest at 15:00–19:00 (5.7–5.9% per hour); about 4.2% at 23:00. **Day of week:** almost flat, from 13.2% (Sunday) to 14.9% (Friday). | sourced | Vera 2020, figs. 5.2.1C–D (columns realigned by hand) |
| IQ-10 | **Recorded crime, US 2024 (FBI):** 1,221,345 violent offences, or 359.1 per 100,000 (2023: 379.5). 5,986,400 property offences, or 1,760.1 per 100,000 (2023: 1,934.1). By offence per 100,000: murder 5.0, rape 37.5, robbery 60.6, aggravated assault 256.1. Coverage: 16,675 agencies, 95.6% of the population. *Derived:* **21.2 recorded Part I offences per 1,000 residents per year.** | sourced | FBI, *UCR Summary of Reported Crimes in the Nation, 2024* (Aug 2025). [PDF](https://cde.ucr.cjis.gov/LATEST/resources/reports/UCR%20Summary%20of%20Reported%20Crimes%20in%20the%20Nation%202024.pdf) |

### Police: the share of crime that reaches the police (NCVS)

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| IQ-11 | **Share of victimisations reported to police, US 2024 (2023 in brackets):**<br>violent: total 47.9% (44.7); rape or sexual assault 23.6 (46.0); robbery 73.4 (42.4); aggravated assault 69.1 (57.1); simple assault 40.4 (40.9); domestic violence 64.5; with a weapon 70.1; with a firearm about 75.<br>property: total 30.5% (29.9); burglary 40.7 (42.2); motor-vehicle theft 75.0 (72.4); other theft 25.4 (24.8). | sourced | BJS, *Criminal Victimization, 2024* (Sept 2025), table 4. [PDF](https://bjs.ojp.gov/document/cv24.pdf) |
| IQ-12 | Violent victimisation in 2024 was 23.3 per 1,000 persons aged 12+, of which 11.2 per 1,000 were reported to police. There were about 6.7 million violent victimisations. Property victimisation was 97.6 per 1,000 households (13.1 million). Rural property victimisation was 48.3 per 1,000 households (56.5 in 2023). | sourced | BJS 2024, pp. 1 and 5; BJS landing page |

### Police: response times, and what changes them

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| IQ-13 | **Officer response (dispatch to arrival) in minutes, Tucson 2016, median (mean):** P1 4 (5.3), P2 10 (17.7), P3 17 (41.6), P4 78 (192.9).<br>**2017, after +12% calls:** P1 4 (6.5), P2 11 (26.0), P3 23 (74.2), **P4 117 (331.7)**. | sourced | Vera 2020, fig. 5.2.5B |
| IQ-14 | **Camden 2016, officer response in minutes, median (mean):** P1 4 (5.7), P2 4 (5.7), P3 4 (6.1), P4 5 (7.6), P5–9 7 (14.7). Its dispatcher delay (answer to dispatch), median (mean): P1 0 (1.0), P2 2 (6.3), P3 6 (14.2), P4 4 (12.6). | sourced | Vera 2020, figs. 5.2.2C–D |
| IQ-15 | *Derived:* the **lognormal spread (σ) implied by median and mean**:<br>Tucson P1 0.75, P2 1.07, P3 1.34, P4 1.35 (1.44 in 2017);<br>Camden P1 0.84, P4 0.92.<br>Low priorities are both slower and far more variable. | derived | IQ-13, IQ-14 |
| IQ-16 | Vallejo, CA: as the police force shrank, the median response to all calls went from 26 min (2019) to 105 min (first half of 2024), while the P1 median moved by less than a minute. | reported | [Open Vallejo, 2024](https://openvallejo.org/2024/09/23/as-vallejo-police-force-shrinks-911-response-times-soar/) (journalism) |
| IQ-17 | Across 10 police precincts (CEBCP, George Mason University), 40–45% of officer time was not accounted for in CAD: neither answering calls nor officer-initiated work. | reported | Cited in Vera 2020, convening chapter ("Capitalize on uncommitted police time") |
| IQ-18 | Response time and outcomes. In the Kansas City 1973 data (Pate et al. 1976), response time was "barely, if at all" related to clearance or property recovery. In Manchester, UK, a 10% faster response gave a 4.7% higher chance of clearance (Blanes i Vidal & Kirchmaier, *Review of Economic Studies*, 2018); the effect was larger for theft and smaller for violence. | reported | Cited in Vera 2020, literature review ("Response time") |

### Police: outcomes and time to outcome

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| IQ-19 | **US clearance 2024** (cleared by arrest or exceptional means): violent 43.8%, property 15.9%. Clearances involving only juveniles: 8.1% of violent and 7.5% of property clearances. | sourced | FBI RCN 2024, p. 13 |
| IQ-20 | **US clearance by offence 2022:** murder 52.3%, aggravated assault 41.4%, rape 26.1%, robbery 23.2%, burglary 13.0%, larceny-theft 12.4%, motor-vehicle theft 9.3%. The FBI 2023 per-offence table was not found (the NIBRS transition left gaps). | reported | FBI CIUS 2022 via [DrugPolicyFacts](https://www.drugpolicyfacts.org/node/2640) |
| IQ-21 | **England and Wales, year ending March 2025, victim-based crime outcomes:** charge or summons 6.3% (5.5% the year before); investigation complete with no suspect identified 42.1%; evidential difficulties 37.9%; not yet assigned an outcome 7.9%. Non-victim-based crime: charge 12.4%, no suspect 22.3%. | sourced | Home Office, *Crime outcomes in England and Wales 2024 to 2025*. [page](https://www.gov.uk/government/statistics/crime-outcomes-in-england-and-wales-2024-to-2025/crime-outcomes-in-england-and-wales-2024-to-2025) |
| IQ-22 | **E&W charge or summons rate by offence, YE Mar 2025:** possession of weapons 29.2%, drugs 18.6, shoplifting 18.5, non-residential burglary 8.9, public order 8.0, robbery 7.5, theft (all) 7.3, residential burglary 4.7, sexual offences 4.2, rape 2.8, vehicle offences 2.4.<br>**No suspect identified:** vehicle 83.9%, residential burglary 73.1, theft 70.8, non-residential burglary 69.6, shoplifting 55.3, robbery 52.2, public order 31.5, sexual 12.3, drugs 9.4.<br>**Not yet assigned:** sexual offences 32.3%. | sourced | Home Office 2025 |
| IQ-23 | **E&W median days from recording to outcome, any outcome:** theft 8, criminal damage and arson 8, shoplifting 10, public order 16, residential burglary 18, violence against the person 22, drugs 22, weapons 28, robbery 35, sexual offences 65.<br>**To charge or summons:** all victim-based 41 (non-victim 47), weapons 22, criminal damage 31, public order 36, residential burglary 56, drugs 62, robbery 67, **rape 434**. | sourced | Home Office 2025 |
| IQ-24 | Outcome rates are first-published figures and rise as open cases close. For example, the rape charge rate for YE Mar 2024 rose from 2.6% at first publication to 4.6% later. | sourced | Home Office 2025 |
| IQ-25 | *Derived, illustrative only* (NCVS violent includes simple assault; UCR violent does not): reported share × clearance gives about **21% of violent** and **5% of property** victimisations ending in a clearance (US 2024). | derived | IQ-11, IQ-19 |

### Courts

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| IQ-26 | E&W Crown Court: the median from charge to completion was **181 days** in Oct–Dec 2025, up from 172 a year earlier. Earlier readings: 179 in Jul–Sep 2025 and 178 for the year to Sep 2025. | reported | MoJ, *Criminal court statistics quarterly, Oct–Dec 2025*. [page](https://www.gov.uk/government/statistics/criminal-court-statistics-quarterly-october-to-december-2025/criminal-court-statistics-quarterly-october-to-december-2025); Commons Library [CBP-8372](https://researchbriefings.files.parliament.uk/documents/CBP-8372/CBP-8372.pdf) |
| IQ-27 | E&W magistrates' courts: the median from charge to completion was **56 days**, up from 40 a year earlier (+40%). | reported | MoJ Oct–Dec 2025 |
| IQ-28 | E&W Crown Court open caseload: **80,203**, a series peak, more than double end-2019. Over 21,000 cases have been open a year or more, and receipts exceeded disposals all year.<br>The outstanding caseload equals 67% of a year's receipts (25% in the magistrates' courts), and the magistrates' open caseload is 379,437.<br>*Derived:* by Little's law, a Crown backlog of 0.67 years of receipts means about **245 days** of queue. | reported | MoJ Oct–Dec 2025; CBP-8372 |
| IQ-29 | Offence to completion in the Crown Court: an *average* of 686 days nationally (Court Data UK, third party) and 602 days in London in 2025 (Law Society). These are means, not the MoJ medians. | reported | [courtdata.uk](https://courtdata.uk/); [Law Society](https://www.lawsociety.org.uk/contact-or-visit-us/press-office/press-releases/london-criminal-court-delays-worst-in-the-country) |

### Hospitals: emergency departments (US, NHAMCS 2022)

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| IQ-30 | 155.4 million ED visits, or **47.3 visits per 100 persons per year**. By age per 100 persons: under 1, 98.9; 75+, 76.0 (approximate, from table 1). *Derived:* about 130 visits per day per 100,000 residents. | sourced | CDC NCHS, *NHAMCS 2022 ED Summary Tables*, table 1. [PDF](https://cdc.gov/nchs/data/nhamcs/web_tables/2022-nhamcs-ed-web-tables.pdf) |
| IQ-31 | **Wait to see a physician, APRN or PA (% of visits):** under 15 min 40.6; 15–59 min 29.7; 1–2 h 9.0; 2–3 h 3.2; 3–4 h 1.6; 4–6 h 1.0; 6 h+ 0.6; not applicable 3.2; blank 11.0. **Median 16.0 min, mean 38.1 min.** | sourced | NHAMCS 2022, table 3 |
| IQ-32 | **Total time in the ED (% of visits):** under 1 h 8.0; 1–2 h 18.4; 2–4 h 33.1; 4–6 h 17.6; 6–10 h 11.4; 10–14 h 2.9; 14–24 h 2.6; 24 h+ 1.6; blank 4.1. | sourced | NHAMCS 2022, table 3 |
| IQ-33 | *Derived* least-squares lognormal fits to the binned data:<br>**ED wait:** median 16.5 min, σ 1.38, matching the bins within 1.5 points.<br>**ED length of stay:** median 3.15 h, σ 0.80. This fits to within 1.5 points up to 10 h; the real tail beyond 14 h is heavier (2.0% observed ≥24 h against 0.6% fitted). | derived | IQ-31, IQ-32 |
| IQ-34 | **Triage level (% of visits):** 1 immediate 1.0; 2 emergent 10.1; 3 urgent 33.1; 4 semi-urgent 19.4; 5 non-urgent 1.8; no triage 8.8; unknown 25.8. | sourced | NHAMCS 2022, table 6 |
| IQ-35 | **Arrivals:** 17.0% by ambulance (26.5 million). 56.3% after business hours (outside Monday–Friday 08:00–17:00). | sourced | NHAMCS 2022, tables 3 and 4 |
| IQ-36 | **Disposition (% of visits; more than one may apply):** admitted to this hospital 11.5 (critical care 2.0); admitted to observation 2.5; transferred to another hospital 1.4; transferred to a psychiatric hospital 1.0; to a nursing home 0.9; died in ED 0.3; left without being seen 1.9; left before treatment was complete 1.4; left against medical advice 1.2; follow-up referral 71.4. | sourced | NHAMCS 2022, table 23 |
| IQ-37 | 75.9% of EDs reported admitted patients boarded 2 h or more while waiting for a bed (94.4% of EDs with 50,000+ visits). 28.4% of EDs went on ambulance diversion in 2021. | sourced | NHAMCS 2022, table 26 |

### EMS and fire

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| IQ-38 | **EMS response time (911 call to arrival on scene), US 2015, NEMSIS, 1.75 million encounters,** median (IQR), 90th percentile:<br>overall 7 (4–10), 14 min;<br>urban 6 (4–9), 12;<br>suburban 6 (4–10), 14;<br>rural 13 (8–19), 26.<br>Mean rural 14.5. *Derived:* lognormal σ ≈ 0.54. | reported | Mell et al., *JAMA Surgery* 2017;152(10):983–984. [link](https://jamanetwork.com/journals/jamasurgery/fullarticle/2643992) |
| IQ-39 | **NFPA 1710 (career departments), each met at the 90th percentile:**<br>fire: turnout 80 s, travel 240 s for the first engine;<br>EMS: turnout 60 s, travel 240 s for the first responder (5 min total) and 480 s for an ALS unit (9 min).<br>The 2016+ alarm-processing figure was not read. | reported | DC FEMS, [EMS](https://fems.dc.gov/page/ems-reponse-time) and [fire](https://fems.dc.gov/page/fire-response-time) response-time pages, citing NFPA 1710 |
| IQ-40 | The odds of survival in medical emergencies fall when response exceeds about 4–5 min (Blackwell & Kaufman 2002; Pons et al. 2005). | reported | Cited in Vera 2020, literature review, note 101 |

### Queueing models (background, not read this session)

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| IQ-41 | **Larson's hypercube model:**<br>N response units, each free or busy, giving 2^N states.<br>The region is split into "atoms", each with Poisson call arrivals.<br>Each atom has a fixed dispatch preference list (the closest free unit).<br>Service times include travel; the exact model assumes them exponential with a common mean.<br>A call that finds all units busy queues FCFS or is lost to mutual aid.<br>Outputs: workload per unit, the share of dispatches crossing district lines, and mean travel time. | reported | Larson, *Computers & OR* 1(1), 1974 (background (primary not read) |
| IQ-42 | **Erlang models.** Erlang C (M/M/c, unlimited queue) is the standard for staffing call-takers and dispatchers. Erlang B (M/M/c/c, a loss system) suits ambulances and beds, where overflow is diverted (compare diversion in IQ-37). Both assume Poisson arrivals, exponential service and identical servers. IQ-15, IQ-33 and IQ-38 show real service and response times are lognormal, not exponential. | reported | textbook (background (primary not read) |

---

## What this means for us (Proposal)
**R1 · One institution record, chained queues** (decision 0069 rule 3; a new institutions spec, police first per the build order). Every institution is the same data shape:
- intake rates per 1,000 residents per year, by case kind, with hour-of-day and day-of-week weights;
- a queue with priority levels;
- staff servers drawn from occupations (spec 46);
- service stages as lognormal (median, σ);
- outcomes with probabilities and a `next` queue.

Patrol feeds investigations, investigations feed prosecution, and prosecution feeds court. At Macro, Little's law (open = arrival rate × mean time open) gives the counts. Up close, each case is a record. Proposed `data/institutions/<kind>.toml`:

```toml
[[institution]]
id = "police_patrol"
serves = "jurisdiction"              # pop-lore §7: settlement hierarchy
staff_role = "police"                # data/gen/jobs.toml
hours = "24/7"                       # spec 33 shifts
hourly_weight = "police_cfs_hourly"  # 24 weights, IQ-9
weekday_weight = [0.132,0.142,0.145,0.144,0.142,0.149,0.145]  # Sun..Sat, Camden 2016

[institution.queue]
discipline = "priority_nonpreemptive_fcfs"   # FCFS within a level
levels = ["p1","p2","p3","p4"]
overflow = "callback"                        # Tucson P9, "alternative response"

[[institution.intake]]   # per 1,000 residents per year; one row per case kind
case = "violent"      rate = 5.4   priority = {p1=0.5, p2=0.5}   # Tucson 1.0% of 537
case = "domestic"     rate = 39.2  priority = {p2=1.0}
case = "property"     rate = 44.6  priority = {p3=0.6, p4=0.4}   # split: default
# ... remaining rows from IQ-4

[[institution.stage]]    # minutes
name = "dispatch_delay"  by = "priority"  median = {p1=0, p2=2, p3=6, p4=4}
name = "travel"          by = "priority"  median = {p1=4, p2=10, p3=17, p4=78}  sigma = {p1=0.75, p2=1.07, p3=1.34, p4=1.35}
name = "on_scene"        median = "not found"   # gap

[[institution.outcome]]
case = "property"  p = {crime_report = "...", no_action = "..."}  next = "police_investigations"
```

A note on the travel stage: Tucson's "travel" includes time spent waiting for a free unit. At hot zoom, split it into a queue wait plus a map travel time.

**R2 · Load falls on the lowest priority** (0069 rule 4: "a shortage above shows as long queues below"; spec 39 staffing edicts).
- The queue must be a strict priority queue. When Tucson's calls rose 12%, the P1 median stayed at 4 min while the P4 median rose 78 → 117 min. Vallejo's all-call median quadrupled as staff fell, with P1 flat.
- Test: raise intake or cut staff, and P1 changes little while P4 balloons.
- Backlogs persist while receipts exceed disposals (Crown Court).
- Never model a shortage as "everything a bit slower".

**R3 · Durations are lognormal (median, σ); use medians, never means** (spec 40 jump forms; spec 33 service times). Means run 1.3–2.5× medians.

| Process | Median | σ |
| --- | --- | --- |
| ED wait to see a provider | 16.5 min | 1.38 |
| ED length of stay | 3.15 h | 0.80, with an extra 2% tail beyond 24 h |
| Police P1 response | 4 min | 0.75 |
| Police P4 response | 78 min | 1.35 |
| EMS response, urban | 6 min | 0.54 |
| EMS response, rural | 13 min | 0.54 |

Draw each case's duration keyed by (seed, case id) so replays match (pop-lore §7 tests).

**R4 · Most police work is not crime** (demo flow beat 16, the detective's desk; spec 24 pop AI; cases doc).
- Crime is 27–33% of 911 calls, and violent crime outside the home is 1.0–1.6% (domestic violence adds about 7%).
- The rest: hang-ups (19%), complaints and environmental conditions (10–23%), status and admin (9–30%), traffic (4–6%), assisting the public (9%), suspicion (4–5%) and alarms (5–8%).
- The desk's map pins must look like this. Pops must generate calls from non-crime events (noise, animals, welfare, alarms, hang-ups), at about **540 public 911 calls per 1,000 residents per year** (Tucson). Camden's 925 is a high-crime upper bound.

**R5 · The reporting funnel before the queue** (pop-lore §7 rule "each past crime that was reported opened a case"). The probability that a crime is reported comes from NCVS table 4, as `data/institutions/reporting_us.toml`:

| Crime | Share reported |
| --- | --- |
| Motor-vehicle theft | 0.75 |
| Robbery | 0.73 (2023: 0.42; average both years) |
| Aggravated assault | 0.69 |
| Burglary | 0.41 |
| Simple assault | 0.40 |
| Sexual assault | 0.24–0.46 |
| Other theft | 0.25 |

Derived histories (0062 late starts) must honour these rates: about half of violent and 70% of property victimisations never reach the police, so they never open a case. That unreported crime is a source of hidden lore.

**R6 · Investigation outcomes and times per offence** (pop-lore §7, replacing "20–40 open cases each"; demo flow slice 2). Each offence carries `p_charge`, `p_no_suspect` and `p_evidential`, plus medians in days to any outcome and to charge (IQ-22, IQ-23):
- residential burglary: 0.047, 0.731, about 0.2; 18 days to any outcome, 56 to charge;
- robbery: 0.075, 0.522; 35 and 67 days;
- rape: 0.028; 434 days to charge.

Most cases close quickly with "no suspect": theft and criminal damage at a median of 8 days. Charges take 1–2 months, and sexual offences more than a year.

An open caseload then comes from Little's law rather than from a default. Example: a city of 100,000 at US rates logs about 2,120 Part I crimes a year, about 5.8 a day. With a mean open time of T days, about 5.8·T cases are open at once, which are then divided among the detectives. The detective count is not found here; see Gaps.

US clearance (IQ-19, IQ-20) is the cross-check for US countries: violent 0.44, property 0.16. Ratios by offence come from 2022.

**R7 · Courts as a queue whose wait is backlog ÷ throughput** (0069 rule 4; spec 39 staffing edicts; spec 32 budget).

| Court (E&W, 2025) | Median, charge to completion |
| --- | --- |
| Magistrates | 56 days (40 a year earlier) |
| Crown | 181 days |

The Crown backlog equals 0.67 years of receipts, and more than 21,000 cases have waited a year or more. Judges and sitting days are the servers. Cutting sitting days must lengthen these medians exactly through the queue, not through a lookup.

**R8 · Intake shape by hour and weekday** (spec 40 calendar; spec 33 shifts).
- Police calls run lowest at 04:00–06:00 (about 1.4% of the day per hour) and highest at 15:00–19:00 (about 5.8%). Weekdays are nearly flat.
- 56% of ED arrivals come after business hours.
- Shift data should staff to these curves. A detective (an office occupation) works days, but patrol and ED do not.
- Store the 24 hourly weights per institution kind in `data/institutions/hourly.toml`.

**R9 · Hospitals and EMS** (0069 rule 4 services table; spec 46/47 towns; pop-lore §7 jurisdiction).

| Measure | Value |
| --- | --- |
| ED intake | 473 per 1,000 residents per year |
| Arrival by ambulance | 17% |
| Triage mix | L1 1%, L2 10%, L3 33%, L4 19%, L5 2%, unknown 35% |
| Admitted | 11.5% |
| Left without being seen | 1.9% |
| Died in ED | 0.3% |
| EMS response | median 6 min urban against 13 rural; 90th percentile 12 against 26 |

For a town served from a city, the response comes from the map distance to the serving station, not from a flat number. NFPA 1710 (240 s travel at the 90th percentile) is a target that career departments may miss.

**R10 · Capacity slack is real** (spec 33 staffing; 0069 rule 2, "played by time").
- 40–45% of patrol time is uncommitted in CAD (IQ-17), and 29% of patrol events are officer-initiated (Tucson).
- A patrol officer's shift is therefore part answering calls, part proactive stops, and part unlogged time.
- When played by time, the unlogged part is where the player's own initiative goes.

---

## Gaps

- **Detective caseloads and investigator time use:** not found. Neither the open cases per detective nor the hours per case were read. The source to fetch is Greenwood, Chaiken and Petersilia, *The Criminal Investigation Process* (RAND R-1776-DOJ, 1975), together with PERF and UK College of Policing workload studies. The pop-lore defaults (one detective per 7 officers, 20–40 open cases) remain unsourced.
- **Time on scene per call, and patrol time by activity (minutes):** not found in what I read. The Vera report has duration tables I did not extract.
- **Police staffing per 1,000 residents** (FBI police employee data) was not read here. Another agent may cover it under services thresholds.
- **Calls per 1,000 residents across many cities:** only Tucson and Camden. The nine-city Vera 2022 figures were not available on its landing page. Lum, Koper and Wu (2022, *Police Quarterly*) were not read.
- **Distribution of time to clearance:** only medians (UK). The US has no national time-to-clearance figure.
- **US state court time to disposition:** not found. Candidates are NCSC CourTools and BJS *Felony Defendants in Large Urban Counties*.
- **Not fetched:** NHS A&E 4-hour performance, OECD ED and inpatient length of stay, and the Crime Survey for England and Wales reporting rate.
- **Larson and Erlang:** stated from background knowledge. The primary papers were not read.
- **Clearance by offence:** 2022 only. 2023 and 2024 per-offence tables were not found.
- **Fire department measured medians:** not found. Only the NFPA 1710 targets were found.
