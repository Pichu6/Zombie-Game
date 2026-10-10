# How real people spend their days: time use, working time, commuting and sleep

**Date:** 2026-10-10. **Status:** research note, not a decision. **Feeds:** [0069](../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md) rule 5, [spec 40](../tier0/specs/40-time.md) (calendar and schedules), [spec 33](../tier0/specs/33-establishments-and-chains.md) (hours and shifts), [spec 46](../tier0/specs/46-education-labour-occupations-settlement.md), [spec 47](../tier0/specs/47-country-towns.md), [inhabitants](../design/inhabitants.md), [pop lore](../design/pop-lore-and-custom-pops.md) §7, [demo flow](../design/demo-flow.md) (fourth flow). **Evidence:** one research pass on 2026-10-10 (40 web calls). `sourced` rows were read in the primary table or text (BLS ATUS 2025 tables, ILOSTAT, Eurofound, national time-use reports, peer-reviewed papers), some as PDF text extracted locally; `reported` rows come from summaries or abstracts. HETUS 2020 country tables and US start-time tables were not read (see Gaps). **IDs:** TU.

Decision 0069 says a pop's day comes from time-use surveys, shaped by its dispositions. This note gathers those surveys and what they mean for the calendar and schedule function, the first slice of the fourth demo flow.

## Findings

### United States: ATUS 2025 (civilian population aged 15+; annual averages; released 25 June 2026)

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TU-1 | Sleeping: 9.03 h/day (men 8.93, women 9.13); 99.9 % do it on any day. Weekdays 8.76 h, weekends and holidays 9.68 h (+0.92 h). | sourced | BLS ATUS 2025, Tables 1–2, https://www.bls.gov/news.release/atus.t01.htm, https://www.bls.gov/news.release/atus.t02.htm |
| TU-2 | "Working" (whole population): the population average is 3.03 h/day, but only 39.6 % work on a given day, and those who do work 7.65 h. Population means hide the shape. | sourced | ATUS 2025 Table 1 |
| TU-3 | Employed people work on 65.5 % of days, 7.66 h on days worked. Weekdays: 80.5 % work, 8.02 h. Saturdays, Sundays and holidays: 30.3 %, 5.41 h. | sourced | ATUS 2025 Table 4, https://www.bls.gov/news.release/atus.t04.htm |
| TU-4 | Full-time workers: weekdays 87.3 % work, 8.45 h; weekends and holidays 29.6 %, 5.49 h. Part-time: weekdays 54.3 %, 5.42 h; weekends and holidays 33.0 %, 5.11 h. Multiple jobholders work on 77.4 % of days. | sourced | ATUS 2025 Table 4 |
| TU-5 | Weekend work by education: on weekend days worked, a high-school graduate works 6.40 h and a bachelor's holder 4.00 h. Bachelor's or higher work on 72 % of days against 63 % for high school only. | sourced | ATUS 2025 Table 4 and news release, https://www.bls.gov/news.release/atus.nr0.htm |
| TU-6 | On days worked, 35 % of employed people did some or all of the work at home and 70 % worked at the workplace. Bachelor's or higher: 51 % worked at home; high school only: 19 %. | sourced | ATUS 2025 news release |
| TU-7 | Household activities: 80.9 % on a given day (men 74.9 %, women 86.7 %), 2.46 h when done (men 2.11, women 2.75). Weekday average 1.88 h, weekend 2.26 h. | sourced | ATUS 2025 Tables 1–2 |
| TU-8 | Purchasing goods and services: 39.7 % on a given day, 1.74 h when done. Caring for household members: 22.0 %, 2.23 h when done (weekdays 23.0 %, weekends 19.8 %). Caring for people outside the household: 7.6 %, 1.97 h. | sourced | ATUS 2025 Tables 1–2 |
| TU-9 | Leisure and sports: 95.1 % on a given day, 5.16 h (men 5.58, women 4.76); weekdays 4.60 h, weekends 6.48 h. TV: 74.3 % watch, 3.51 h when they do. | sourced | ATUS 2025 Tables 1–2 |
| TU-10 | Socialising and communicating: 30.4 % of days, 1.92 h when done. Weekdays 28.4 % for 1.55 h; weekends 35.2 % for 2.62 h. Participation was 38 % in 2015. | sourced | ATUS 2025 Tables 1–2 and news release |
| TU-11 | Organisational, civic and religious activities: 11.8 % on weekdays, 17.9 % on weekends and holidays (2.49 h when done). | sourced | ATUS 2025 Table 2 |
| TU-12 | Age profile, h/day. Work and work-related: 0.71 (15–19), 3.85 (20–24), 4.58 (25–34), 4.83 (35–44), 4.70 (45–54), 3.83 (55–64), 1.14 (65–74), 0.26 (75+). Leisure: 5.66, 4.67, 4.50, 3.89, 4.30, 5.21, 6.90, 7.39. Caring for household members peaks at 35–44 (1.23 h). Education at 15–19: 3.45 h. | sourced | ATUS 2025 Table 3, https://www.bls.gov/news.release/atus.t03.htm |
| TU-13 | Married with spouse present vs. other: caring for household members 0.73 vs. 0.25 h; household activities 2.23 vs. 1.76 h; leisure 4.86 vs. 5.45 h; work 3.57 vs. 3.06 h. | sourced | ATUS 2025 Table 3 |
| TU-14 | Children: primary childcare is 2.3 h/day when the youngest child is under 6 (women 2.8, men 1.7) and 47 min when the youngest is 6–17. Employed adults: 4.5 h leisure with no children under 18, 3.2 h with a child under 6. Parents of children under 18: household activities 3.1 h if not employed, 1.7 h if employed. | sourced | ATUS 2025 news release |
| TU-15 | US work schedules, 2024–25 (wage and salary workers, main job): 85 % regular daytime; 15 % non-daytime (evenings 5 %, nights 3 %, the rest rotating, split or irregular). Non-daytime shares by industry: leisure and hospitality 40 %, transportation and utilities 25 %, trade 20 %. | sourced | BLS, Job Flexibilities and Work Schedules 2024–2025 (ATUS module), https://www.bls.gov/news.release/flex2.nr0.htm |
| TU-16 | 68 % usually work Monday to Friday; 8 % usually work Saturday and Sunday. 57 % have flexible start and end times. 59 % know their schedule 4+ weeks ahead and 9 % one day or less ahead (construction and extraction: 28 %). 34 % sometimes work at home. | sourced | Same release |

### Europe: HETUS and EWCS

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TU-17 | HETUS 2010 wave (18 countries, 2008–2015), per day, all people: sleep from 8 h to almost 9 h; leisure and social life 4 h 30 min to 6 h; household and family care 2 h 30 min to 3 h 30 min; employment 2–3 h. | sourced | Eurostat Statistics Explained, "How do women and men use their time" (data April 2019), PDF https://ec.europa.eu/eurostat/statistics-explained/SEPDF/cache/78191.pdf |
| TU-18 | HETUS 2010: 29–43 % of people do paid work on a given day, for 6 h 30 min to almost 8 h when they do. Study: 6–11 % participate, 4 h to just over 6 h. | sourced | Same |
| TU-19 | HETUS 2010, daily participation in household and family care: women 94–98 % in every country. Men vary: Turkey 53 %, Italy 70 %, Greece 72 %, Spain 77 %, Norway 92 %, Finland 93 %. Gender gap in time among participants: Turkey 3 h 16 min, Italy 2 h 47 min, Greece 2 h 21 min, France 1 h 11 min, Finland 1 h 09 min, Netherlands 1 h 02 min, Norway 47 min. | sourced | Same |
| TU-20 | HETUS 2010, daily food management: men 20–70 % (Norway 70 %, Finland 61 %), women 77–91 % (Poland 91 %). | sourced | Same |
| TU-21 | HETUS 2020 round: 20 Eurobase tables; Eurostat says it will publish further indicators during 2026. Per-country 2020 values were not read. | reported | Eurostat metadata tus_20, https://ec.europa.eu/eurostat/cache/metadata/en/tus_20_esms.htm |
| TU-22 | EWCS 2015 (EU28 + Norway): 52 % work at least one Saturday a month and 23 % at least three. 30 % work at least one Sunday a month (27.5 % in 2005, 28 % in 2010); more than 10 % work at least three. 54 % work at least one weekend day a month. | sourced | Eurofound, "What's happening with Sunday work in Europe?" (copy) https://www.bollettinoadapt.it/wp-content/uploads/2018/10/EUROFOND-What’s-happening-with-Sunday-work-in-Europe.pdf |
| TU-23 | EWCS: about 21 % of workers do shift work and 19 % do night work (survey wave not shown in the snippet; probably 2015). | reported | Eurofound blog "Working time clashes with work–life balance" (HTTP 429, not read), https://www.eurofound.europa.eu/en/podcast-and-blog/all/working-time-clashes-work-life-balance-many-workers |

### Non-Western surveys

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TU-24 | India TUS 2024: Jan–Dec 2024; 139,487 households; 454,192 persons aged 6+. Diary of 48 half-hour slots, 04:00 to 04:00. | sourced | MoSPI press note, 28 Mar 2025, https://www.mospi.gov.in/sites/default/files/press_release/Press_Note_28.03.2025-1.pdf |
| TU-25 | India 2024, ages 15–59, employment and related activities: participation men 75.0 %, women 25.0 %; per participant men 480 min/day, women 344 min. Per person over everyone: men 360, women 86 min. Paid activities: 74.5 % / 24.9 %, 421 / 305 min. | sourced | Same, Tables 1–3 |
| TU-26 | India 2024, ages 15–59, unpaid domestic services: participation men 30.4 %, women 92.9 %; per participant men 86 min, women 305 min. Unpaid caregiving: men 21.4 %, women 41.0 %. | sourced | Same |
| TU-27 | India 2024, domestic services for the household: on normal days men 26.4 %, women 83.1 %; on "other days" (weekly off-days, holidays) men 32.2 %, women 67.5 %. | sourced | Same |
| TU-28 | India 2024, ages 15–59: self-care and maintenance, sleep included, is 690 min (men) and 686 min (women) per participant; at 60+ it is 764 and 771 min. Socialising, community and religious practice: about 93 % take part, 135 min. Culture, leisure and mass media: men 94.6 % for 156 min, women 89.2 % for 145 min. | sourced | Same |
| TU-29 | India 2024: commuters aged 15–59 who travel for work spend 77 min/day commuting (men) and 67 min (women). | sourced | Same |
| TU-30 | Mexico ENUT 2019 (ages 12+; a stylised weekly questionnaire, not a diary): 61.3 % do market work (men 76.1 %, women 48.0 %), averaging 43.7 h/week among participants. Total work, paid plus unpaid: women 59.5 h/week, men 53.3 h. | sourced | INEGI, ENUT 2019 nota técnica, https://www.inegi.org.mx/contenidos/programas/enut/2019/doc/enut_2019_nota_tecnica.pdf |
| TU-31 | Mexico 2019, unpaid domestic work for one's own home: women 30.8 h/week (98.6 % participate), men 11.6 h (95.7 %). Among people with 40+ h of market work: women 25.7 h, men 11.0 h. Direct care: women 12.3 h, men 5.4 h; with passive care, 28.8 vs. 12.9 h. | sourced | Same |
| TU-32 | Japan STULA 2021 (ages 10+, weekly average per day): sleep 7 h 54 min (men 7 h 58, women 7 h 49), up 14 min on 2016. People who work (15+) work 6 h 27 min/day (men) and 4 h 42 min (women), as weekly averages. | sourced | Statistics Bureau of Japan, 2021 STULA Summary (Questionnaire A), https://www.stat.go.jp/english/data/shakai/2021/pdf/timeuse-a2021.pdf |
| TU-33 | Japan 2021, housework and related work: men 51 min, women 3 h 24 min. Couples with a child under 6: husbands 1 h 54 min, wives 7 h 28 min. | sourced | Same |
| TU-34 | Japan 2021: 6.7 % of people working on a weekday teleworked that day. Teleworkers commuted 1 h 03 min less and slept 18 min more. | sourced | Same |
| TU-35 | Japan 2021, waking time spent alone: highest at ages 70–74 (6 h 51 min/day), then 65–69 (6 h 49 min). Time alone rose from 2016 in every age group, most at 25–29 (+1 h 21 min). | sourced | Same |

### Working time across countries, and long hours

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TU-36 | ILOSTAT 2023, mean weekly hours actually worked per employed person (main job), total / men / women: World 40.98 / 43.49 / 37.22. By income group: low 36.45, lower-middle 43.91, upper-middle 42.42, high 34.80. | sourced | ILOSTAT HOW_TEMP_SEX_NB_A, https://rplumber.ilo.org/data/indicator/?id=HOW_TEMP_SEX_NB_A&timefrom=2023&timeto=2023&type=label&format=.csv |
| TU-37 | ILOSTAT 2023, by country (total / men / women): India 45.82 / 50.22 / 36.24; Bangladesh 46.02 / 50.27 / 31.25; Türkiye 43.78 / 45.35 / 40.52; South Africa 42.56 / 44.46 / 40.16; Mexico 42.49 / 45.46 / 38.12; Poland 39.05 / 40.27 / 37.54; Korea 39.09 / 41.45 / 36.06; Brazil 38.79 / 40.53 / 36.44; US 37.80 / 39.64 / 35.68; UK 34.98 / 38.02 / 31.54; Germany 33.47 / 36.84 / 29.51; Netherlands 30.37 / 33.91 / 26.24. Highest: Jordan 48.79. Lowest: Kiribati 27.71. Japan, China and Nigeria have no 2023 value. | sourced | Same |
| TU-38 | ILO (2022): over a third of the world's workers regularly work more than 48 h/week, and about a fifth work fewer than 35 h. | reported | ILO, Working Time and Work-Life Balance Around the World (2022), https://www.ilo.org/publications/working-time-and-work-life-balance-around-world |
| TU-39 | OECD How's Life: employees working 50+ h/week, OECD average 13 %; Turkey 39 % (another report says 43 %); Netherlands, Sweden and Denmark about 1–2 %. The figures are dated and secondary. | reported | Hürriyet Daily News citing OECD, https://hurriyetdailynews.com/turks-work-longest-hours-among-oecd-members-57552 |

### Commuting and start times

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TU-40 | US 2019 (ACS; people who work at home excluded): mean one-way commute 27.6 min (25.0 in 2006). 11.9 % commute under 10 min; 9.8 % commute 60+ min (7.9 % in 2006). Bus commuters average 46.6 min. | sourced | US Census Bureau release CB21-TPS.29, https://www.census.gov/newsroom/press-releases/2021/one-way-travel-time-to-work-rises.html |
| TU-41 | US 2019: about 57 % of workers leave for work between 06:00 and 08:29. Those leaving at 06:00–06:29 have the longest average commute, 32.8 min. | sourced | Same |
| TU-42 | US 2009: the busiest half-hour for leaving home is 07:00–07:29, with 14.1 % of workers. | reported | Census, Commuting in the United States: 2009 (search snippet), https://libraryarchives.metro.net:443/DPGTL/harvested/2011-Commuting-in-the-united-states-2009.pdf |
| TU-43 | ACS table B08302 ("Time leaving home to go to work") has 14 bins: 00:00–04:59, then half-hours from 05:00 to 08:59, then hours 09–11, 12:00–15:59 and 16:00–23:59. The labels were read; the values were not (the API needs a key). | sourced | https://api.census.gov/data/2024/acs/acs1/groups/B08302.json (labels only) |

### Sleep timing and how much one person's days vary

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TU-44 | Smartphone sleep data from about 100 countries: a country's average bedtime predicts its sleep duration, but its average wake time does not ("society governs bedtime, the internal clock governs wake time"). Women sleep about 30 min more. Age is the strongest factor in sleep timing. | reported | Walch, Cochran & Forger 2016, Science Advances 2:e1501705 (abstract and press coverage), https://www.sciencedaily.com/releases/2016/05/160506160105.htm |
| TU-45 | Within one person, sleep duration varies from night to night by an SD of 85.8 min (diary), 77.4 min (actigraphy) or 67.0 min (EEG); the coefficient of variation is about 19 %. Younger adults vary more. Pooled from eight datasets of healthy sleepers; the paper itself was not identified. | reported | Search summary (paper not identified) |
| TU-46 | CARDIA actigraphy (N = 669): within-person SD of sleep duration is 1.26 h from day to day but only 0.39 h from year to year. | reported | Knutson et al. 2007, Sleep (via search summary) |
| TU-47 | Mobidrive 1999 (Karlsruhe and Halle; 362 persons in 162 households; six weeks; 14,360 person-days; 52,273 trips): travel is broadly routine, with considerable background variability on top. | reported | Axhausen et al. / Schönfelder & Axhausen; https://www.fhwa.dot.gov/ohim/trb/mobidrive.pdf |
| TU-48 | On weekdays, the spread of activity places is stable from day to day for workers and students. For non-workers on weekdays, and for everyone at weekends, random factors dominate. Much of the variation is stable difference between people. | reported | Susilo & Kitamura 2005 (search summary) |
| TU-49 | Ghent seven-day diary (2008): large variability within each person for trips, time use and activity sequences. Systematic day-of-week effects explain only a very small share of it. | reported | Search summary (WCTR/Belgium) |

### Personality and time use

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TU-50 | German SOEP diaries (5,456 days from 1,364 people): personality is linked to time use, but weakly (r ≈ .10–.25). Conscientiousness goes with more time working and studying, whether or not employed full time, and less time on TV. Extraversion predicts whether someone meets friends at all, not how long. | reported | Rohrer & Lucas 2018, "Only so many hours", Collabra: Psychology 4(1):1, https://doi.org/10.1525/collabra.112 (full text 403) |
| TU-51 | Extraversion and emotional stability are negatively related to time spent alone; emotional stability also to chores and TV. | reported | Wrzus et al. 2016, cited in a review (secondary) |
| TU-52 | Workaholism (Bergen Work Addiction Scale, 4 of 7 items rated "often" or "always") among Norwegian employees in 2012 (N = 1,124): 8.3 % (95 % CI 6.7–9.9). | sourced | Andreassen et al. 2014, PLoS ONE, https://pmc.ncbi.nlm.nih.gov/articles/PMC4131865/ |
| TU-53 | Workaholism's predictors, adjusted odds ratios: neuroticism 1.21 (1.13–1.31), intellect 1.09, agreeableness 1.12 (borderline), age 18–31 2.11 and 32–45 2.01 vs. 46–58. Conscientiousness is not significant (0.93, 0.85–1.01) and is negative in the crude model (0.88). Sex, marital status and children are not significant once adjusted. | sourced | Same |

### How simulations build schedules

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TU-54 | CEMDAP outputs a complete daily activity-travel pattern for each person in each household, and serves as MATSim's initial plans. Berlin reused Los Angeles parameters and calibrated to traffic counts. Other activity-based systems output hourly OD matrices or tours, which are harder to integrate. | reported | Ziemke, Nagel & Bhat 2015, https://www.caee.utexas.edu/prof/bhat/ABSTRACTS/ZiemkeNagelBhat2015.pdf |
| TU-55 | Agent-based transport simulations need a synthetic population plus an activity plan for every agent. Each plan holds activity type, start and end time, duration, sequence, location and mode. Travel diaries cover only a sample, so plans must be generated. | reported | Swedish synthetic population dataset, https://research.chalmers.se/en/publication/535909 ; Swiss study (search summary) |
| TU-56 | ALBATROSS (Eindhoven) is a learning-based "computational process" scheduling model. CEMDAP is econometric. MATSim is an agent-based simulator that re-plans (PKC-129: about 10 % of agents per iteration, departure shifts up to 1,800 s). | reported | IntechOpen review chapter, https://www.intechopen.com/chapters/73240 ; PKC-129 |
| TU-57 | Other generators: an ETH hidden Markov model made 1 million agents' 24-hour days, checked against Singapore survey statistics. The Melbourne model calibrates to observed activity types, start and end times and durations. Another approach draws start times from census hour distributions, given the previous activity's end and the travel time. | reported | Search summaries; https://arxiv.org/html/2111.10061v2 |

## What this means for us (Proposal)
**1. A day is a participation draw, then a duration draw. Never use population means.** *Spec 40, decision 0069 rule 5.*
For each pop and day, first draw whether it does an activity at all: p(activity | country, sex, age band, employment, household role, day type). Then draw how long, from the hours-if-done distribution.
- Example (TU-2, TU-3): an employed American works on 80.5 % of weekdays (8.02 h) and 30.3 % of weekend days (5.41 h). It is never 3.03 h every day.
- Data shape, one file per base nation and real country, e.g. `data/timeuse/us.toml`:
```toml
source = "BLS ATUS 2025, Tables 1-4"   # method = "diary" | "stylised" (ENUT is stylised)
population = "civilian 15+"
[[row]]
activity = "work"          # ICATUS-like ids: sleep, selfcare, work, commute, household, care, shop, leisure, social, civic, study
who = { employment = "full_time" }
day = "weekday"            # weekday | saturday | sunday_holiday
participation = 0.873
hours_if_done = 8.45
```
- The four base nations (`base_low` … `base_high`) borrow from the closest real survey: India for low and lower-middle, Mexico for upper-middle, the US, HETUS and Japan for high. ILOSTAT hours by income group set the targets (TU-36).

**2. Weekly hours by country and sex are the calibration target.** *Spec 47, pop-lore §7.*
- Store ILOSTAT mean actual weekly hours by sex (TU-36, TU-37) per real country and per income group.
- A generated country's average across its pops must land on its row. Examples: World men 43.5 h, women 37.2 h; India men 50.2 h; Netherlands women 26.2 h.
- The gap between the sexes is itself data and differs by country: Bangladesh 19 h, Philippines 0.1 h.

**3. Occupations carry a schedule pattern, drawn once per pop at hire.** *Spec 33, specs 46/47, decision 0069 rule 1.*
- Pattern weights in `jobs.toml`: `schedule = { day = 0.85, evening = 0.05, night = 0.03, rotating = 0.07 }` as the default (TU-15). Leisure and hospitality get 40 % non-daytime, transport 25 %, trade 20 %.
- Days: 68 % usually Monday to Friday, 8 % usually weekends (TU-16). In Europe, Saturday work at least once a month is 52 % and Sunday 30 % (TU-22). These set how many weekend shifts an establishment needs (spec 33).
- Advance notice: 9 % learn their schedule a day or less ahead, 28 % in construction (TU-16). This gives the share of pops whose next week is drawn late, which is a source of events.

**4. Replace pop-lore §7's "office 08:00–17:00" default with distributions.** *Pop-lore §7, spec 40.*
- Keep 08:00–17:00 as the mode, not the rule.
- Start times: draw a departure half-hour per pop, once; about 57 % leave between 06:00 and 08:29, with the peak at 07:00–07:29 (TU-41, TU-42). Use the B08302 bins as the data shape (TU-43); the values still have to be fetched.
- Commute: draw per pop from home and work places. Check the outcome against TU-40 (mean 27.6 min, 9.8 % at 60+ min, 11.9 % under 10 min) and TU-29 (India 77/67 min).
- Hours on days worked follow TU-3 and TU-4, so a detective's 8–5 is one draw among several.

**5. Every pop has a fixed skeleton plus daily noise.** *Spec 40, PKC-157, decision 0069 rule 5.*
- **Drawn once per pop** (keyed by seed and pop): job pattern, departure anchor, sleep anchor (bedtime and wake time), household chore share.
- **Drawn per day** (keyed by seed, pop and day): which discretionary activities happen (TU-7 to TU-11), and noise on durations.
- Sleep duration varies within a person by an SD of about 70 min from night to night (TU-45, TU-46). Most variation is day to day, not year to year.
- Workers' weekdays are stable; non-workers and weekends are loose (TU-48). Day-of-week effects are small apart from the weekday/weekend split (TU-49).
- This replaces PKC-157's flat ±30 min jitter with jitter per activity: small for anchored work starts (bounded by MATSim's 1,800 s), large for sleep and leisure.

**6. Dispositions nudge the draws, inside the survey spread.** *Spec 24, decision 0069 rule 5, PB.*
Personality effects are small (r ≈ .10–.25, TU-50). Structure matters far more:
- employment and day type (TU-3);
- children: leisure 4.5 h → 3.2 h with a child under 6 (TU-14);
- marriage: care 0.73 vs. 0.25 h (TU-13);
- age (TU-12).

A disposition therefore shifts a participation probability or a duration by up to about one survey SD, and never overrides structure. Changes to 0069 rule 5:
- **Workaholic splits in two:**
  - `work_drive`: conscientiousness gives more work and study time and less TV (TU-50).
  - `work_compulsion`: about 8 % of employees, higher with neuroticism and youth, not with conscientiousness (TU-52, TU-53). This is the affair-and-stress hook for the detective.
- **Long hours mostly come from country and occupation:** 50+ h is 13 % across the OECD, 1–2 % in the Netherlands and 39 % in Turkey (TU-39); more than 48 h is over a third worldwide (TU-38). Draw long-hours jobs from the country table first, then let `work_drive` pick who takes them.
- **Lonely** means a lower probability of socialising on a given day (US base 30 %: 28 % on weekdays, 35 % at weekends) and more time alone. Extraversion moves whether someone socialises, not how long (TU-10, TU-50, TU-51). Old age raises time alone (Japan, ages 70–74: 6 h 51 min; TU-35).

**7. Chores are split within the household, not drawn per pop.** *Spec 24, households (spec 45), pop-lore §7.*
- A household has a chore load: for the US, 80.9 % of people do some on a given day, 2.46 h when they do (TU-7). It is divided between adults by a country's gender split:
  - India: men take part 30 %, women 93 % (TU-26);
  - Mexico: 11.6 vs. 30.8 h/week (TU-31);
  - Japan: 51 min vs. 3 h 24 min (TU-33);
  - HETUS: men's daily participation from 53 % in Turkey to 93 % in Finland (TU-19).
- Off-days shift chores: in India men's participation rises on off-days (26 → 32 %) and women's falls (83 → 68 %) (TU-27).
- The detective "doing his last chores on Sunday evening" is then a draw from his household's split, consistent with his wife's.

**8. Three day types and one holiday flag.** *Spec 40 calendar.*
- Day types: `weekday`, `saturday` and `sunday_holiday`. ATUS and India's "other days" merge holidays into the weekend; Japan keeps Saturday and Sunday apart.
- US weekend shifts relative to weekdays (TU-1, TU-9 to TU-11): sleep +0.92 h, leisure +1.88 h, socialising participation 28 → 35 %, civic and religious 12 → 18 %, work participation for employed people 80 → 30 %.
- Lights at night (demo flow beat 7) follow from these tables plus the shift shares in rule 3.

**9. Generation method: template chains now, Markov sequences later.** *Spec 40, PKC-157, EF.*
- The output must have the shape the ABMs use: an ordered plan per (pop, day) of activity, start, end, place and mode (TU-54, TU-55). That is exactly "position = f(pop, time)".
- **Slice 1:** a template chain per day type: sleep → self-care → [commute → work → commute] → household/care → leisure/social → sleep. Participation and duration come from rule 1; anchors from rule 5.
- **Later:** an activity-transition (Markov or sequence) model fitted to diary microdata (ATUS or HETUS), as the ETH HMM and the Melbourne model do (TU-57). The fit is done offline; the game ships only the transition tables.
- Cold pops (decision 0069 rule 6) use the same tables as cohort shares by clock hour.

**10. Survey categories must be harmonised before mixing countries.** *Decision 0069, `data/`.*
- ATUS "sleeping" (9.03 h) and India's "self-care and maintenance" (690 min) are different categories. Japan reports weekly averages.
- Mexico's ENUT is a weekly questionnaire, not a diary. Its hours are not per-day diary minutes and must not be divided by 7 as if they were.
- Every table carries `method` and `category_map` to an ICATUS-like id list. Only like-for-like numbers become calibration targets.

## Gaps

- **HETUS 2020-round country tables** (Eurobase `tus_20*`) were not read. HETUS 2010 is available only as cross-country ranges plus a few named countries; the charts are images.
- **Shift and night shares in the EWCS** (21 % and 19 %) are from a snippet; Eurofound returned HTTP 429. EWCS 2024 results were not found.
- **ILO hour bands** (share of workers above 48 h by country, `EMP_TEMP_SEX_HOW_DT_A`) returned HTTP 400. For long hours there are only the ILO and OECD headline figures, both reported.
- **ACS 2024 departure-time and travel-time bins** were not fetched: the Census API now needs a key. A free key would fill TU-43 in one call.
- **Start-time data exists only for the US.** No European or non-Western start-time distribution was found.
- **Japan:** the commuting row in Table 1-1 came out garbled in extraction and is not cited. Weekday vs. Sunday figures are in the statistical tables, not the summary.
- **South Africa and Brazil time-use surveys** were not covered, for lack of budget. South Africa appears only through ILOSTAT hours (TU-37).
- **Sleep timing (clock times):** Walch 2016's full text was not read, and no ATUS sleep-timing table was read. The pooled sleep-variability study (TU-45) was not identified by name.
- **Personality:** the full text of Rohrer & Lucas 2018 returned HTTP 403, so there are no per-trait minutes. Wrzus 2016 is secondary.
- **Day-to-day variability:** for Mobidrive, Susilo & Kitamura and Ghent, only summaries were seen. No within-person SD of work start time was found.
- **ATUS Tables 8A–8C and 11A–11B** (by children, by employment status, leisure by characteristics) were not read; only the news-release text was used.
- **ALBATROSS:** inputs and outputs were not read in a primary source.
