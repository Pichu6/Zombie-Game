# Calendars, school and clock hours: when and where people are

**Date:** 2026-10-11. **Status:** research note, not a decision. **Feeds:** [0069](../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md) rule 5, [0070](../decisions/0070-broken-commitments-are-noticed.md), [0071](../decisions/0071-days-from-time-use-data-dispositions-nudge.md), [spec 40](../tier0/specs/40-time.md) (the calendar), [spec 33](../tier0/specs/33-establishments-and-chains.md) (hours and shifts), [spec 46](../tier0/specs/46-education-labour-occupations-settlement.md) (schooling), [demo flow](../design/demo-flow.md) (fourth flow, beats 4, 7, 11 and 14), [pop lore](../design/pop-lore-and-custom-pops.md) §7; it continues [daily lives and time use](daily-lives-and-time-use.md) (TU). **Evidence:** four research passes on 2026-10-11, about 240 web calls in all. The US clock-hour curves, anchors and outings were computed from the BLS ATUS 2024 and 2025 public microdata (13,815 diary days), so they are `derived` from primary data. European and Turkish curves are Eurostat HETUS tables (`sourced`). School data comes from UIS, OECD, Eurydice and TIMSS; weekends and holidays from Wikipedia, ILO reports and public holiday calendar feeds (mostly `reported`). The raw downloads, the derived tables and the scripts are outside git in `data_raw/` (`atus`, `census_acs`, `eurostat`, `timeuse_um`, `school`, `calendar` and `derived-2026-10-11`); they move into `data/` and `tools/data/` with the spec that uses them. What was not read is under Gaps. **IDs:** CK.

[TU](daily-lives-and-time-use.md) gave daily totals: who does what on a given day and for how long. Four gaps would have left holes in the calendar and schedule spec: **when** in the day people are where; a real 24-hour diary for the **upper-middle-income** default country (its only source was a weekly questionnaire); **children and school**; and **weekends and holidays** country by country. This note fills them.

## Findings

**How the US clock-hour curves were computed.** ATUS diaries run from 04:00 to 04:00. Every share is weighted (`TUFINLWGT`, each year's weights halved, the BLS convention for pooled years); "at 10:45" means the share of time in the 15-minute slot 10:45–11:00. Every minute is in exactly one of five places, which sum to 100 %: **home** (TEWHERE 1; working at home counts as home), **workplace** (TEWHERE 2), **someone else's home** (TEWHERE 3), **travel** (any travel code or transport mode) and **other** (every other place: shops, restaurants, school, church, outdoors). **Sleep** (codes 0101xx) is a separate flag. ATUS does not ask where people slept, groomed or did private activities: 40.7 % of all minutes. Each such run takes its place from its neighbours (the same place on both sides gives that place; after travel, the next place; before travel, the previous place; otherwise the previous place; with no known neighbour, home); 96.0 % of sleep minutes end up at home. Day types: **weekday** (Monday to Friday, not a holiday), **saturday**, **sunday_holiday** (Sunday, or any day with TRHOLIDAY = 1). **Employed** is TELFS 1–2; a **workday** has at least 60 minutes of work codes 050101/050102. The script is `data_raw/derived-2026-10-11/scripts/ck-D-atus_tempogram.py` (`python -I <script> <data_dir> <out_dir>`, about 5 s).

### Where people are by clock hour: US ATUS 2024-2025 microdata

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-1 | On weekdays at 03:00, 95.5 % of US people aged 15+ are asleep and 95.7 % are at home. The sleep peak is 95.8 % at 02:45. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_tempogram.py |
| CK-2 | At 04:00 (diary start) on weekdays, 90.9 % are asleep and 6.3 % are not at home. Of weekday workdays, 3.8 % are already in progress at 04:00 (night shifts). | derived | same |
| CK-3 | Weekday share asleep: 82 % at 05:00, 61 % at 06:00, 36 % at 07:00, 20 % at 08:00, 12 % at 09:00. | derived | same |
| CK-4 | Weekday share asleep in the evening: 8 % at 20:00, 22 % at 21:00, 49 % at 22:00, 74 % at 23:00, 88 % at 00:00, 93 % at 01:00. | derived | same |
| CK-5 | The daytime low of sleep (naps included) is about 3 %. Weekdays reach 2.9 % at 18:00; it stays between 3.5 % and 4.5 % from 11:00 to 16:00. | derived | same |
| CK-6 | Mornings start later at the weekend. At 07:00, 58.0 % are asleep on Saturday and 60.7 % on Sunday/holiday, against 35.8 % on weekdays. At 08:00 the shares are 34.6 %, 37.6 % and 19.9 %. | derived | same |
| CK-7 | Sleep per diary day from the curves is 8.78 h on weekdays, 9.36 h on Saturday, 9.89 h on Sunday/holiday and 9.03 h on an average day. Check these against ATUS Table 1 "sleeping". | derived | same |
| CK-8 | The share at home is lowest at midday. On weekdays it falls to 46.8 % at 12:00 (all), 35.3 % (employed), 32.1 % (full-time) and 65.0 % at 11:00 (not employed). | derived | same |
| CK-9 | At weekends the home low is higher: 58.2 % at 13:00 on Saturday and 58.4 % at 12:00 on Sunday/holiday. | derived | same |
| CK-10 | People aged 65+ never fall below 65.2 % at home on weekdays (low at 11:00). | derived | same |
| CK-11 | At-home time per day is 17.4 h on weekdays (all), 15.7 h for the employed, 20.0 h for the not employed and 20.1 h for people aged 65+. | derived | same |
| CK-12 | On weekdays at 10:45, 30.1 % of all people aged 15+ are at their workplace. This is the daily peak; the night low is 1.2 % at 03:00. | derived | same |
| CK-13 | On weekdays at 10:45, 48.9 % of employed people are at their workplace (2024: 49.4 %; 2025: 48.3 %). For full-time workers the figure is 56.4 %. | derived | same |
| CK-14 | Employed weekday workplace share by hour: 10 % at 06:00, 21 % at 07:00, 36 % at 08:00, 45 % at 09:00, 47–49 % from 10:00 to 14:00, 42 % at 15:00, 32 % at 16:00, 20 % at 17:00, 12 % at 18:00, 9 % at 19:00. | derived | same |
| CK-15 | Working anywhere (codes 0501xx) peaks at 65.3 % of employed people on weekdays at 10:45. It dips to 47.4 % at 12:00 (lunch), while the workplace share only moves from 48.7 % to 46.9 %. Lunch happens mostly at the workplace. | derived | same |
| CK-16 | Working at home peaks at 15.0 % of employed people on weekdays (14:15). It adds 0.90 h per weekday per person aged 15+. | derived | same |
| CK-17 | The workplace peak is 8.2 % of all people aged 15+ on Saturday (11:45) and 6.3 % on Sunday/holiday (13:45). | derived | same |
| CK-18 | Weekday travel peaks twice. The morning peak is 10.0 % at 07:30 (12.4 % of the employed). The evening peak is 13.1 % at 17:00 (16.9 % of the employed). | derived | same |
| CK-19 | Weekday travel is 5.8–7.5 % through the middle of the day (09:00–14:00) and below 1 % from 00:00 to 04:00. | derived | same |
| CK-20 | Weekend travel has no rush hours. It holds a 10–11 % plateau from 10:00 to 17:00 on Saturday and peaks at 11.9 % at 12:00 on Sunday/holiday. | derived | same |
| CK-21 | Travel totals 1.14 h per day on an average day. | derived | same |
| CK-22 | Being at someone else's home peaks at 3.7 % on weekdays (18:45), 8.4 % on Saturday (17:45) and 8.2 % on Sunday/holiday (15:30). | derived | same |
| CK-23 | "Other places" peak at 15.2 % on weekdays (12:30), 20.1 % on Saturday (12:45) and 21.9 % on Sunday/holiday (11:15, the church hour). | derived | same |
| CK-24 | Not-employed people on weekdays reach 24.0 % at other places at 10:45. | derived | same |
| CK-25 | Shops and restaurants (TEWHERE 4, 6, 7) peak at 4.6 % on weekdays (12:30) and 8.7 % on Saturday (12:15). | derived | same |
| CK-26 | Aged 15–24 on weekdays, school peaks at 24.3 % at 10:45. This is averaged over all 12 months, including summer. | derived | same |
| CK-27 | Being at home and awake peaks in the evening: 72.9 % at 19:45 on weekdays. | derived | same |
| CK-28 | 15.0 % of people never leave home on a weekday. On Saturday it is 19.8 %, and on Sunday/holiday 23.4 %. | derived | same |
| CK-29 | On weekdays, 79.4 % of employed people work at least 60 min. On Saturday it is 28.6 %, and on Sunday/holiday 24.6 %. | derived | same |
| CK-30 | On weekday workdays, 74.5 % of workers spend some time at their workplace. | derived | same |

### Daily anchors from the same diaries: work start and end, leaving home, waking and bedtime

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-31 | First work start on weekday workdays (employed, n = 3,051): p10 05:40, p25 06:50, median 07:48, p75 08:50, p90 11:00. For full-time workers the median is 07:40 and p90 is 09:50. | derived | same |
| CK-32 | Part-time first work start on weekdays: median 09:00, p75 12:27, p90 16:00 (n = 417). | derived | same |
| CK-33 | Arrival at the workplace on weekdays: median 07:49 (p10 06:00, p90 12:00, n = 2,081). | derived | same |
| CK-34 | Leaving home on weekday workplace days: the first departure has median 07:00 (p10 05:22, p25 06:07, p75 08:00, p90 09:45, n = 1,997). The last departure before reaching work has median 07:11. | derived | same |
| CK-35 | Return home on weekday workplace days: median 18:25 (p10 15:45, p25 17:00, p75 20:20, p90 22:05). | derived | same |
| CK-36 | Last work end on weekdays: median 17:00 (p10 14:30, p25 15:45, p75 18:25, p90 21:30). | derived | same |
| CK-37 | Work time on weekday workdays: median 480 min (p10 301, p90 647). For full-time workers the median is 495 min. | derived | same |
| CK-38 | The travel leg that ends at the workplace has a median of 20 min (p10 5, p25 10, p75 30, p90 51). 5.0 % of weekday arrivals have no travel leg right before them. | derived | same |
| CK-39 | Weekday wake time, for people asleep at 04:00 (n = 6,167): p10 05:00, p25 06:00, median 07:00, p75 08:00, p90 09:30. | derived | same |
| CK-40 | Employed people wake earlier on weekdays: median 06:30, p10 05:00. Aged 15–24 the median is 07:30. | derived | same |
| CK-41 | Wake time is later at the weekend: median 08:00 on both Saturday and Sunday/holiday. Aged 15–24 on Sunday the median is 09:08 and p90 is 12:00. | derived | same |
| CK-42 | Bedtime on Sunday–Thursday evenings (the night before a weekday, for people asleep at 03:59, n = 8,559): p10 20:30, p25 21:25, median 22:05, p75 23:00, p90 00:05. | derived | same |
| CK-43 | Bedtime on Friday–Saturday evenings: median 22:30, p90 00:40. Aged 15–24 the median is 23:00 and p90 is 01:05. | derived | same |
| CK-44 | At 04:00 at the end of the diary, 4.6 % of people are not asleep: night owls and night-shift workers. If they are counted with bedtime ≥ 04:00, the Sunday–Thursday p90 bedtime moves from 00:05 to 01:00. | derived | same |
| CK-45 | Clock times in ATUS are heavily heaped on :00 and :30, so many percentiles land on exact hours. Treat ±15 min as reporting noise. | derived | same |
| CK-46 | Year-to-year stability (2024 vs 2025, each alone): key slot shares differ by ≤ 2 pp. At 10:45 the employed workplace share is 49.4 % vs 48.3 %; at 22:00 the share asleep is 49.0 % vs 48.6 %; travel at 17:00 is 13.8 % vs 12.4 %. | derived | same |
| CK-47 | Kish effective sample sizes for one 15-min slot: weekday all 4,069; weekday employed 2,446; Saturday all 2,022; Sunday/holiday all 2,029; weekday 15–24 only 321. | derived | same |
| CK-48 | The 95 % sampling half-width at p = 50 % is about ±1.5 pp (weekday all), ±2.0 pp (weekday employed), ±2.2 pp (weekend all) and ±5.5 pp (weekday 15–24). | derived | same (binomial approximation on n_eff) |
| CK-49 | The 2025 ATUS file has no diaries dated October 2025. The 2024 file covers all 12 months. | derived | atusresp-2025 TUDIARYDATE |

### Outings: stops away from home and the workplace

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-50 | Outing definition (used in all the outing rows that follow): a maximal run of consecutive non-travel episodes at one TEWHERE place that is neither home (1) nor the workplace (2). The travel legs before and after belong to the outing. Missing places are imputed as in the tempogram. A change of place code without travel starts a new outing. An outing at someone else's home is social_other_home; any other outing takes the ATUS tier-1 code with the most minutes. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-51 | The pooled sample yields 24,544 outings. 4.6 % are cut by the 04:00 diary edges; they are counted, but left out of the duration and start-time distributions. 7.7 % have no travel leg on either side, for example a store followed by a restaurant with no travel recorded between them. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-52 | On weekdays, 68.6 % of person-days include at least one outing (any class). People who go out make 2.49 outings on average. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-53 | On Saturday, 73.5 % of person-days include an outing, and those who go out make 2.71 on average. On Sunday/holiday the figures are 70.2 % and 2.36. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-54 | Outings per weekday person-day (all classes except work_offsite): 0 for 32.2 %, 1 for 25.1 %, 2 for 18.2 %, 3 for 10.2 % and 4+ for 14.3 %. The mean is 1.67. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-55 | On Saturday, 27.1 % of person-days have no outing and 19.0 % have 4+ (mean 1.97). On Sunday/holiday, 30.0 % have none and 13.3 % have 4+ (mean 1.65). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-56 | Employed people on a weekday workday make no outing on 38.6 % of days (mean 1.29). Going to work is not an outing. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-57 | Employed people on a weekday off make no outing on only 20.3 % of days (mean 2.51). This is the busiest group. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-58 | Not-employed people make no outing on 28.0 % of weekdays (mean 1.89). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-59 | People aged 65+ make no outing on 36.2 % of Sundays/holidays (mean 1.26). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-60 | On weekdays, 35.0 % of person-days include a shop/errand outing. Those with one make 1.54 on average. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-61 | On weekdays, 19.9 % of person-days include a leisure/sport outing. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-62 | On weekdays, 15.0 % of person-days include a care outing (1.76 per day among those). 15.5 % include an 'other' outing. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-63 | On weekdays, 13.3 % of person-days include a visit to someone else's home and 11.9 % include eating out. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-64 | On weekdays, 5.8 % of person-days include a study outing, 4.0 % a religious/civic one, and 2.7 % work done away from the workplace. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-65 | On Saturday, the share of person-days with each class is: shop/errand 42.2 %, leisure/sport 25.3 %, visit 22.6 %, eat out 19.3 %, care 9.1 %, religious/civic 6.2 %. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-66 | On Sunday/holiday, the shares are: shop/errand 33.0 %, visit 23.4 %, leisure/sport 18.1 %, religious/civic 16.6 %, eat out 16.0 %, care 6.2 %. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-67 | On weekdays, people spend a median of 30 min at a shop/errand stop (p10 5, p90 90). A meal out lasts a median of 60 min (p10 29, p90 120). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-68 | On weekdays, a leisure/sport stop lasts a median of 65 min (p10 20, p90 190). A visit to someone else's home lasts a median of 60 min (p10 5, p90 240). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-69 | On weekdays, a religious/civic stop lasts a median of 110 min (p10 30, p90 254). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-70 | On weekdays, a care stop lasts a median of only 10 min (p10 2, p90 65), so most are drop-offs and pick-ups. A study stop lasts a median of 244 min (p10 75, p90 525). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-71 | Weekend visits last longer: a median of 90 min on Saturday and 120 min on Sunday/holiday, with p90 about 330 min. Saturday leisure/sport has a median of 120 min (p90 300). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-72 | A travel leg to or from an outing has a median of 15 min on every day type. Its p90 is 35 min on weekdays and 45 min at weekends. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-73 | Trip chaining on weekdays: 51.8 % of outings start from home, 9.0 % from the workplace and 37.0 % from another outing. 55.3 % of outings end with going home. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-74 | On Saturday, 46.7 % of outings start from another outing. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-75 | On weekdays, 45.4 % of shop/errand and 53.1 % of eat-out outings are reached from another stop. 65.9 % of leisure/sport outings start from home. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-76 | On Sunday/holiday, 84.8 % of religious outings start from home. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-77 | Shop/errand outings on weekdays start fairly evenly through the day: 6–9 % per clock hour from 08:00 to 17:59. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-78 | Eat-out outings on weekdays have two start peaks: the 12:00 hour (14 %) and 17:00–18:59 (12–13 % per hour). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-79 | Care outings on weekdays have two start peaks: the 07:00 hour (15 %) and the 15:00 hour (13 %). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-80 | 28 % of weekday study outings start in the 07:00 hour. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-81 | On weekdays, visits and leisure/sport outings mostly start between 16:00 and 18:59 (10–12 % per hour). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-82 | Weekday religious/civic outings peak in the 18:00 hour (19 %). On Sunday/holiday, 48 % start between 09:00 and 10:59. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |

### Workdays: outings around work, work at home, lunch and split shifts

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-83 | On weekday workdays, 22.0 % of outings (excluding work_offsite) happen before work, 23.0 % during a work break and 55.1 % after work. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-84 | Per weekday workday, 20.9 % of days have an outing before work, 18.3 % during work and 42.4 % after work. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-85 | On weekday workdays, 35.5 % of eat-out outings happen during a work break. 35.9 % of care outings happen before work, which fits school drop-offs. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-86 | Aged 15–24 on weekdays, 31.7 % of person-days include a study outing (median 245 min at the place). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-87 | Aged 65+ on weekdays, 43.4 % of person-days include a shop/errand outing. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-88 | 88.6 % of eat-out outings (weighted) are at a restaurant or bar (TEWHERE 4). The 'other' class is mostly household tasks away from home (ATUS tier 02) and personal care away from home (tier 01, for example hotel nights). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-89 | On weekday workdays (employed, n = 3,051), 73.0 % of work time is at the workplace, 22.7 % at home and 4.3 % elsewhere. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-90 | 31.0 % of weekday workdays include at least 15 min of work at home. 21.4 % have at least 90 % of their work at home. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-91 | On weekday workdays where the person is at the workplace (n = 2,217), 20.5 % have two or more separate stays there. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-92 | True split shifts (a gap of more than 120 min between workplace stays) occur on 5.2 % of weekday workplace days. On 4.3 % the person goes home during the gap. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-93 | 57.8 % of weekday workplace days record an eating episode at the workplace (full-time 61.1 %, part-time 36.6 %). | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-94 | On 9.3 % of weekday workplace days, the person leaves the workplace between 11:00 and 13:59 and is back within 2 h. In 2.5 % they go home and in 2.6 % to a restaurant. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-95 | 33.3 % of weekday workdays record no eating between the first and last work minute (part-time 57.4 %). ATUS records only the main activity, so eating while working counts as work. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-96 | On Saturday workdays, 18.1 % of work time is at home, and 67.4 % of those days include time at the workplace. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |
| CK-97 | On Sunday/holiday workdays, 21.3 % of work time is at home, and 59.1 % of those days include time at the workplace. | derived | ATUS 2024+2025 atusact/atussum, ck-D-atus_outings.py |

### Departure and travel time to work: US ACS 2024

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-98 | ACS 2024 1-year, US, workers 16+ who did not work from home (143.3 M): 5.5 % leave home for work between 00:00 and 04:59. | sourced | Census ACS table-based summary file acsdt1y2024-b08302.dat (US row), labels from ACS20241YR_Table_Shells.txt |
| CK-99 | ACS 2024 departure bins: 4.0 % at 05:00–05:29, 4.8 % at 05:30–05:59, 8.6 % at 06:00–06:29, 9.4 % at 06:30–06:59, 14.3 % at 07:00–07:29 (the largest bin), 11.9 % at 07:30–07:59, 11.6 % at 08:00–08:29, 5.7 % at 08:30–08:59, 7.0 % at 09:00–09:59. | sourced | same |
| CK-100 | ACS 2024 late departure bins: 3.1 % at 10:00–10:59, 1.4 % at 11:00–11:59, 6.7 % at 12:00–15:59, 6.0 % at 16:00–23:59. ACS 2023 is within 0.2 pp in every bin. | sourced | same + acsdt1y2023-b08302.dat |
| CK-101 | ACS 2024 median departure is about 07:38, interpolated linearly inside the 30-min bins. p25 is about 06:36 and p90 about 13:35. | derived | from CK-98..CK-100 |
| CK-102 | ATUS leaves home earlier than ACS (all days). Departures before 07:00 are 39.3 % in ATUS vs 32.3 % in ACS; before 08:00, 64.8 % vs 58.5 %. Departures at 16:00 or later are 2.4 % vs 6.0 %. The ATUS measure is the last departure from home before arriving at the workplace. | derived | ATUS script + ACS B08302 |
| CK-103 | ACS 2024 travel time to work (one way): 2.8 % under 5 min, 9.4 % 5–9, 13.1 % 10–14, 15.0 % 15–19, 14.1 % 20–24, 7.0 % 25–29, 13.7 % 30–34, 3.4 % 35–39, 4.1 % 40–44, 8.3 % 45–59, 6.3 % 60–89 and 3.0 % 90 min or more. | sourced | acsdt1y2024-b08303.dat (US row) |
| CK-104 | The ACS 2024 travel-time median is about 23.5 min, interpolated, with p90 about 59 min. The ATUS final travel leg is shorter (median 20 min, p90 51 min). | derived | B08303 + ATUS script |

### Sleep timing across countries

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-105 | Walch, Cochran & Forger 2016 is based on 5,450 ENTRAIN app users (8,070 submitted; 45 % in the US). Users self-reported their "normal" wake time and bedtime, rounded to the nearest hour. The most common answers were 07:00 wake, 23:00 bedtime and 8 h of sleep. | sourced | Walch et al. 2016, Sci. Adv. 2:e1501705 (PMC4928979 full text) |
| CK-106 | Walch 2016: age is the main driver of sleep timing. Younger users have later and wider distributions of wake time and bedtime, and the spread narrows with age. | sourced | same |
| CK-107 | Walch 2016: women schedule more sleep than men, through both earlier bedtimes and later wake times. Midsleep does not differ by sex. | sourced | same |
| CK-108 | Walch 2016: a country's average bedtime predicts its average sleep duration; average wake time does not. Countries differ more in bedtime than in wake time. | sourced | same |

### Europe by clock hour: HETUS 2020

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-109 | Eurostat publishes European tempograms. tus_20startime (HETUS 2020 wave, 11 countries: AT, BG, DE, EE, FI, HR, HU, NL, NO, PL, RS) and tus_00startime (2000/2010 waves) give the participation rate by 10-min slot for 9 activity groups. They cover activities only, with no location, and are not split by weekday. | sourced | Eurostat tus_20startime TSV (downloaded), tus_00/tus_20 ESMS metadata, Eurostat catalogue TOC |
| CK-110 | HETUS 2020 "personal care except eating" (mostly sleep) at 07:00: 64.6 % in DE, 78.7 % in NL, 59.6 % in PL, 49.6 % in HU. The ATUS equivalent (tier 01, all days) is 51.6 %. | derived | tus_20startime; ATUS script (confidence note: sourced (HETUS) / derived (ATUS)) |
| CK-111 | The same HETUS category at 22:00: 34.4 % in DE, 29.5 % in NL, 56.2 % in PL. ATUS: 53.8 %. US people get up earlier than people in NW Europe, and go to bed at times close to Poland and Hungary. | derived | same (confidence note: sourced (HETUS) / derived (ATUS)) |
| CK-112 | HETUS 2020 "employment and study" peaks at 30–40 % of the population between 09:40 and 11:50 in all 11 countries (DE 33.3 % at 10:50). The ATUS work+study equivalent (tiers 05+06, all days) peaks at 35.5 % at 10:45. | derived | same (confidence note: sourced (HETUS) / derived (ATUS)) |
| CK-113 | HETUS 2020 travel to/from work or study peaks at 4.5–12.4 % between 06:50 and 08:10, depending on the country. | sourced | tus_20startime |
| CK-114 | Bulgaria shows 22.3 % in personal care at 15:00, against 3–7 % elsewhere. This looks like a siesta or a data artifact; do not use BG as a reference. | sourced | tus_20startime |

### Upper-middle-income diaries: methods and what exists

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-115 | Türkiye's survey ran in 2014-2015 as part of HETUS wave 2010. Wave-2010 diaries use 144 slots of 10 minutes, with one weekday and one weekend diary per person. | sourced | https://ec.europa.eu/eurostat/cache/metadata/en/tus_00_esms.htm (sections 3.4, 3.7, 5) |
| CK-116 | In Eurostat's 2000/2010 tables, `tus_00age` covers ages 15+. All other `tus_00` tables cover ages 20-74 only. | sourced | https://ec.europa.eu/eurostat/cache/metadata/en/tus_00_esms.htm (section 3.6) |
| CK-117 | Türkiye (TR) appears only in `tus_00age` (sex × age) and `tus_00startime` (time of day). It is absent from `tus_00week` (day of week) and `tus_00selfstat` (labour status). | sourced | Eurostat TSV files listed above (geo lists parsed locally) |
| CK-118 | Serbia's survey ran in 2021-2022 as part of HETUS wave 2020. Each respondent should fill one weekday and one weekend diary. Slots are 10 minutes. | sourced | https://ec.europa.eu/eurostat/cache/metadata/en/tus_20_esms.htm (sections 3.4, 3.8, 5) |
| CK-119 | Serbia is published by sex × Mon-Fri, Mon-Thu, Friday, Saturday, Sunday; × 9 labour statuses; × 10-year age bands; and × 10-minute time of day. | sourced | tus_20week, tus_20selfstat, tus_20age, tus_20startime (geo=RS) |
| CK-120 | Serbia's 10-14 age band has no values, so its totals are best read as ages 15+. | derived | tus_20age, geo=RS (confidence note: derived (empty Y10-14 rows in tus_20age)) |
| CK-121 | Eurostat blanks values based on fewer than 20 observations (":") and flags values based on fewer than 50 with "u". Several Serbian Saturday/Sunday and part-time cells are flagged. | sourced | tus_20_esms.htm (accuracy/reliability section) |
| CK-122 | China 2018 used a 24-hour diary with 15-minute slots. It covered one weekday and one rest day in May 2018, ages 15+, and 20,226 households / 48,580 persons in 11 provinces. The bulletin excludes Shanghai. | sourced | https://www.stats.gov.cn/xxgk/sjfb/zxfb2020/201901/t20190125_1768278.html (附注一) |
| CK-123 | China 2018's daily figures weight weekday by 5/7 and rest day by 2/7. | sourced | same page (附注三) |
| CK-124 | China 2024 used a 15-minute diary from 11 to 31 May 2024. It covered ages 6+ in all 31 provinces: 38,500 households and 107,000 persons, mostly self-filled in a phone mini-app. | sourced | https://www.stats.gov.cn/zt_18555/zthd/lhfw/2025/2025_qgsjlydc/202501/t20250127_1958536.html |
| CK-125 | China 2024 rules: activities under 7 minutes are not recorded, and activities of 7-15 minutes count as 15 minutes. | sourced | https://www.stats.gov.cn/sj/zxfb/202410/t20241031_1957216.html (note 1) |
| CK-126 | China 2024 publishes 13 activity categories for both sexes together, plus 6 domains by sex, urban/rural and age (6-17, 18-59, 60+). No weekday/rest-day split is published. More detail is promised in a "China Time Use Survey Yearbook". | sourced | t20241031_1957216.html; https://www.stats.gov.cn/sj/zxfb/202410/t20241031_1957215.html; t20250127_1958537.html |
| CK-127 | Colombia's ENUT 2020-21 is not a slot diary. It asks people aged 10+ about a list of activities done on the previous day. Fieldwork ran September 2020 to August 2021, with 49,519 households. | sourced | https://www.dane.gov.co/files/investigaciones/boletines/ENUT/Bol_ENUT_2020_2021.pdf (pp. 2-3) |
| CK-128 | Colombia's minutes per participant overlap. For men, personal activities (16:25) plus paid work (8:57) already exceed 24 hours. | derived | Bol_ENUT_2020_2021.pdf p.7, Gráfico 3 (confidence note: derived (CK-133 values)) |
| CK-129 | Thailand's TUS 2015 used a 10-minute diary covering ages 6+, July 2014 to June 2015, with 83,880 individuals, coded to ICATUS 2005. | reported | https://unstats.un.org/unsd/demographic-social/meetings/2019/newyork-egm-tus-nov/Thailand.pdf (via search summary) |
| CK-130 | South Africa's TUS 2010 was a stand-alone household survey of about 32,000 dwelling units (3,080 PSUs) covering people aged 10+. | sourced | https://unstats.un.org/unsd/gender/Events/28-30_June_2016/Session_2_SouthAfrica.pdf (Stats SA slides, pp. 2-3) |
| CK-131 | ILOSTAT's SDMX catalogue has 1,215 dataflows and none on time use. ILOSTAT cannot fill other income groups. | sourced | https://sdmx.ilo.org/rest/dataflow/ILO/all/latest?detail=allstubs (confidence note: sourced (negative check)) |
| CK-132 | UNSD's SDG 5.4.1 series gives only the share of a 24-hour day spent on unpaid domestic and care work, by sex. It has no participation rates or minutes per participant. | reported | https://databank.worldbank.org/metadataglossary/world-development-indicators/series/SG.TIM.UWRK.MA (via search summary) |
| CK-133 | Colombia 2020-21 (ages 10+, previous-day activity list). Paid work: men 53.3%, women 29.9% take part; PTP men 8:57, women 7:37. Unpaid work: men 63.0%, women 90.3%; PTP men 3:06, women 7:44. | sourced | https://www.dane.gov.co/files/investigaciones/boletines/ENUT/Bol_ENUT_2020_2021.pdf pp. 5-7, Gráficos 2-3 |

### Türkiye 2014-15 (ages 15+, all days)

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-134 | Sleep: 100% take part. Average 8:42 (522 min); men 8:37, women 8:47. | sourced | tus_00age TR, AC01 |
| CK-135 | Eating: 99.7% take part, 1:58 PTP. | sourced | tus_00age TR, AC02 |
| CK-136 | Main/second job: 35.8% take part on a given day (men 53.3%, women 18.8%). PTP 7:47 (men 8:10, women 6:42). | sourced | tus_00age TR, AC1A |
| CK-137 | Commuting (travel to/from work): 36.4% take part (men 53.6%, women 19.8%), 1:06 PTP (men 1:08, women 1:01). | sourced | tus_00age TR, AC913 |
| CK-138 | Study: 9.2% take part, 4:46 PTP. In the youngest band (Eurostat code Y15-20), 33.5% take part with 5:31 PTP. | sourced | tus_00age TR, AC2 |
| CK-139 | Household and family care: men 51.6% at 1:43 PTP; women 92.3% at 4:59 PTP. | sourced | tus_00age TR, AC3 |
| CK-140 | Food management: men 18.3% at 0:37 PTP; women 83.6% at 1:39 PTP. Cleaning the dwelling: men 7.4%, women 67.9% (1:23 PTP). | sourced | tus_00age TR, AC31A, AC321 |
| CK-141 | Shopping and services: 21.0% take part (men 21.3%, women 20.7%), 1:05 PTP. | sourced | tus_00age TR, AC36 |
| CK-142 | Childcare (excluding teaching/reading/talking): men 5.3% at 0:44 PTP; women 27.7% at 1:31 PTP. | sourced | tus_00age TR, AC38A |
| CK-143 | Participatory activities (meetings and religious): 33.6% take part, 2:05 PTP (men 28.0%, women 39.2%). | sourced | tus_00age TR, AC43 |
| CK-144 | Participatory activities rise with age: 15.7% (youngest band), 25.4% (25-44), 46.4% (45-64), 67.5% (65+). At 65+ the PTP is 2:47. | sourced | tus_00age TR, AC43 by age |
| CK-145 | The high AC43 share in Türkiye is very likely daily prayer, which HETUS codes as a religious activity under "participatory activities". | estimate | interpretation of CK-143 and CK-144 |
| CK-146 | Organisational volunteer work is rare: 0.3% take part. | sourced | tus_00age TR, AC41 |
| CK-147 | Visiting and feasts: 23.2% at 1:18 PTP. Other social life: 52.9% at 1:38 PTP. | sourced | tus_00age TR, AC51A, AC51B |
| CK-148 | TV and video: 82.8% take part, 2:35 PTP. At 65+: 85.5% at 3:24 PTP. | sourced | tus_00age TR, AC82 |
| CK-149 | Travel other than job-related: 77.4% take part, 1:22 PTP (men 89.4%, women 65.7%). | sourced | tus_00age TR, AC9A |
| CK-150 | Ages 25-44: men take part in a job 71.9% of days (8:25 PTP), women 25.9% (7:00 PTP). | sourced | tus_00age TR, AC1A, Y25-44 |
| CK-151 | TurkStat's own release (ages 10+): sleep averages 8:48, with 8:35 on weekdays and 9:20 at weekends. | reported | https://www.alomaliye.com/2015/12/04/zaman-kullanim-arastirmasi-2014-2015/ (via search summary; TurkStat portal needs JavaScript) |

### Serbia 2021-22 (ages 15+; weekday, Saturday, Sunday)

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-152 | Sleep, both sexes (100% take part): 8:11 Mon-Fri, 8:36 Saturday, 8:59 Sunday. | sourced | tus_20week RS, AC01 |
| CK-153 | Employment participation, both sexes: 47.50% Mon-Fri, 27.51% Saturday, 16.13% Sunday. | sourced | tus_20week RS, AC1 |
| CK-154 | Employment PTP, both sexes: 7:09 Mon-Fri, 6:13 Saturday, 5:43 Sunday. | sourced | tus_20week RS, AC1 |
| CK-155 | Employment on weekdays: men 56.42% at 7:31 PTP; women 39.19% at 6:39 PTP. | sourced | tus_20week RS, AC1, D1-5 |
| CK-156 | Employment participation ratio to weekdays: Saturday 0.58, Sunday 0.34. | derived | — (confidence note: derived (CK-153)) |
| CK-157 | Commuting participation: 40.92% Mon-Fri (1:01 PTP), 19.92% Saturday, 10.13% Sunday. | sourced | tus_20week RS, AC910 |
| CK-158 | Household and family care, all days: women 93.21% at 4:14 PTP; men 76.10% at 2:22 PTP. | sourced | tus_20week RS, AC3_713, D1-7 |
| CK-159 | Food management: women 80.30% at 1:47 PTP; men 31.34% at 0:47 PTP. Cleaning the dwelling: women 50.19%, men 12.17%. | sourced | tus_20week RS, AC31_X_312, AC321 |
| CK-160 | Shopping and services: 39.21% Mon-Fri (0:38 PTP), 37.60% Saturday (0:50 PTP), 32.99% Sunday (0:43 PTP). | sourced | tus_20week RS, AC36 |
| CK-161 | Socialising with others: 32.60% Mon-Fri (1:29 PTP), 44.25% Saturday (1:52 PTP), 49.19% Sunday (2:02 PTP). | sourced | tus_20week RS, AC512_513_519 |
| CK-162 | TV: 85.39% take part on all days, 2:53 PTP. PTP is 2:48 Mon-Fri and 3:05 on Saturday and Sunday. | sourced | tus_20week RS, AC821 |
| CK-163 | Voluntary work and meetings (including religious): 6.35% take part, 2:24 PTP. Sunday is 8.54%. | sourced | tus_20week RS, AC4 |
| CK-164 | Study on weekdays: 8.68% at 5:27 PTP. School/university excluding homework: 4.87% at 4:33 PTP. | sourced | tus_20week RS, AC2, AC21_X_212 |
| CK-165 | Sport except walking is 6.91% (1:53 PTP). Walking and hiking is 16.34% (1:26 PTP). Both are higher on Sunday (8.39% and 21.38%). | sourced | tus_20week RS, AC6_X_611, AC611 |
| CK-166 | Full-time employed, all days: 77.72% do paid work on a given day at 7:27 PTP. 71.42% commute (1:01 PTP). Sleep is 7:44. | sourced | tus_20selfstat RS, EMP_FT |
| CK-167 | Full-time employed work about 5.4 days per week. | derived | — (confidence note: derived (0.7772 × 7, CK-166)) |
| CK-168 | Unemployed: 25.27% log some employment activity (5:49 PTP). Household care is 87.63% at 4:03 PTP. | sourced | tus_20selfstat RS, UNE |
| CK-169 | Retired: TV 95.77% at 3:49 PTP; household care 89.42% at 4:15 PTP; sleep 8:55. | sourced | tus_20selfstat RS, RET |
| CK-170 | Students: study 74.59% at 5:23 PTP (all days). Homemakers (women): household and family care 99.17% at 6:32 PTP. | sourced | tus_20selfstat RS, EDUC, HOME |
| CK-171 | Employment participation by age (all days): 18.51% at 15-24, 64.86% at 35-44 (7:11 PTP), 10.28% at 65-74, 3.14% at 75+. | sourced | tus_20age RS, AC1 |

### China 2018 (ages 15+, 11 provinces)

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-172 | Sleep and rest (including naps): 100% take part, 559 min (men 556, women 562). It is 553 min on weekdays and 574 on rest days. | sourced | t20190125_1768278.html, 附表1-3 and section 2 |
| CK-173 | Employment ("就业工作"): 38.4% take part (men 46.0%, women 31.3%). PTP is 461 min (men 472, women 444). | sourced | same, 附表2-3 |
| CK-174 | Employment by day type: 44.0% take part on weekdays and 24.5% on rest days. PTP is 7:40 and 7:42. | sourced | same, section 3 |
| CK-175 | Household business or farming: 23.1% take part (rural 41.1%), 374 min PTP. On weekdays it is 23.3%, on rest days 22.7%. | sourced | same, 附表2-3, section 3 |
| CK-176 | Housework: 58.5% take part (men 40.4%, women 75.6%). PTP is 147 min (men 110, women 166). | sourced | China 2018, 附表2-3 |
| CK-177 | Caring for children's daily life: 18.9% (men 12.3%, women 25.1%), with 188 min PTP (men 137, women 211). Escorting or tutoring children for study: 10.2%, 92 min PTP. | sourced | China 2018, 附表2-3 |
| CK-178 | Buying goods or services: 21.1% (men 14.7%, women 27.1%), 80 min PTP. Per-capita time is 14 min on weekdays and 25 on rest days. | sourced | China 2018, 附表2-3, section 4 |
| CK-179 | TV: 66.5% at 151 min PTP. Leisure and entertainment (online video, games, outings): 40.7% at 159 min PTP. | sourced | China 2018, 附表2-3 |
| CK-180 | Social contact (which includes religious activities, weddings and funerals): 17.6% at 136 min PTP. | sourced | China 2018, 附表2-3, 附注二 |
| CK-181 | Exercise: 30.9% at 101 min PTP; urban 38.7%, rural 18.7%. | sourced | China 2018, 附表2-3 |
| CK-182 | Travel: 50.8% (men 56.3%, women 45.6%) at 75 min PTP. Travel for paid employment: 30.3% at 66 min PTP. | sourced | China 2018, 附表2-3, section 7 |
| CK-183 | Learning and training (ages 15+): 7.2% at 372 min PTP. Ages 15-19 average 8:02 per person. | sourced | China 2018, 附表2-3, section 6 |

### China 2024 (ages 6+, national)

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-184 | Sleep and rest: 586 min (9:46), 100% take part. | sourced | t20241031_1957216.html 附表2 |
| CK-185 | Labour and employment: 54.4% take part, 383 min PTP. In the paid-work domain, men take part 60.2% (6:44 PTP) and women 47.7% (5:55 PTP). | sourced | 1957216 附表2; 1957215 section 2 |
| CK-186 | Paid-work domain by age: 18-59 take part 75.6% at 6:32 PTP; 60+ take part 36.3% at 5:30 PTP. | sourced | 1957215 section 2 |
| CK-187 | Housework: 64.9% at 119 min PTP. Caring for family: 28.4% at 106 min PTP. | sourced | 1957216 附表2 |
| CK-188 | Buying goods or services: 34.1% at 43 min PTP. | sourced | 1957216 附表2 |
| CK-189 | Sport and fitness: 49.6% at 70 min PTP. Culture and leisure: 95.3% at 160 min PTP. Social contact: 26.3% at 69 min PTP. | sourced | 1957216 附表2 |
| CK-190 | Learning: 25.1% take part overall. Ages 6-17 take part 98.4% at 8:12 PTP (all days, weighted). | sourced | 1957215 section 5 |
| CK-191 | Travel: 80.5% at 62 min PTP. Ages 6-17 take part 84.9% at 50 min PTP. | sourced | 1957216 附表2; 1957215 section 6 |
| CK-192 | Unpaid-work domain: men take part 67.5% (1:52 PTP), women 83.9% (3:29 PTP). | sourced | 1957215 section 3 |

### Upper-middle-income countries by clock hour

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-193 | Türkiye time-of-day curves cover ages 20-74 (all `tus_00` tables except `tus_00age`). | sourced | tus_00_esms.htm section 3.6 |
| CK-194 | Türkiye: 95.5% are in "personal care except eating" (mostly sleep) at 03:00. The share drops below 50% in the 08:00 slot (48.0%) and passes 50% again at 23:00 (58.3%). | derived | tus_00startime TR, AC0A (confidence note: sourced values; crossing derived) |
| CK-195 | Serbia: 99.1% are in personal care except eating at 03:00. The share drops below 50% at 07:30 (45.8%) and passes 50% at 22:40 (50.6%). | derived | tus_20startime RS, AC0_X_021 (confidence note: sourced values; crossing derived) |
| CK-196 | Türkiye: work and study peak at 33.1% at 11:10 and fall to 17.6% at 12:30, while 21.9% are eating. | sourced | tus_00startime TR, AC1_2_TR, AC02 |
| CK-197 | Serbia: work and study peak at 35.2% at 11:50. They are still 30.3% at 13:00 and fall to 13.85% at 16:00. | sourced | tus_20startime RS, AC1_2 |
| CK-198 | TV peaks at 51.1% at 21:30 in Serbia and at 34.0% at 21:40 in Türkiye. | sourced | tus_20startime RS AC821; tus_00startime TR AC82 |
| CK-199 | Türkiye: 2.0% are eating at 02:30, and leisure other than TV rises to 6.4% at 05:30 (0.8% at 02:30). These are probably Ramadan pre-dawn meals and dawn prayer. | sourced | tus_00startime TR, AC02, AC4-8A (confidence note: sourced values; reason is estimate) |

### Harmonising survey categories

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-200 | HETUS mapping to ICATUS 2016: AC0 sleep/eat/personal care → 9 self-care. AC1 employment → 1. AC2 study → 6. AC31-AC37 housework and shopping → 3. AC38-AC39 care of household members → 4. AC41-AC42 volunteering and informal help → 5. AC43 meetings and religious → 7. AC51 social life → 7. AC52-AC8 culture, sport, hobbies, computing and media → 8. | estimate | mapping by code labels (tus_00age JSON-stat labels) |
| CK-201 | HETUS, China and ATUS all report travel as its own block. ICATUS 2016 assigns travel to each purpose's division. Harmonising needs a purpose split: HETUS gives one (AC913/AC910 work, AC936 shopping, AC938 childcare, AC9D/AC940-980 leisure); China 2018 gives only work and study travel. | estimate | tus_00age / tus_20week labels; China 2018 section 7 |
| CK-202 | China mapping: 就业工作 → 1. 家庭生产经营 → split across 1 (for sale) and 2 (own use); the bulletin defines it as "for income or own use". 家务劳动 and 购买商品或服务 → 3. 陪伴照料 → 4. 公益 → 5. 社会交往 → 7. 学习培训 → 6. Fitness, TV, reading and leisure → 8. Sleep, hygiene, eating and 看病就医 → 9. | estimate | China 2018 附注二 definitions |
| CK-203 | Religious practice sits in different places: HETUS ACL43 "participatory activities" (with meetings), China's "social contact", and ICATUS division 7. It needs its own small activity in the game to carry the Türkiye signal. | estimate | CK-143, CK-163, CK-180 |
| CK-204 | HETUS "resting/time out" (AC53, AC531) is 49.9% in Türkiye and 56.22% in Serbia. Its ICATUS division (8 leisure or 9 self-care) should be checked against the ICATUS 2016 text before mapping. | estimate | tus_00age TR AC53; tus_20week RS AC531 (confidence note: sourced values; mapping is estimate) |

### The school year: start, end and the long break

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-205 | UIS publishes the start month of the school year (pre-primary to post-secondary) for 182 of our 183 countries (all except NCL). The latest year is 2025 for all of them. | sourced | UIS API, indicator 403 and 404: https://api.uis.unesco.org/api/public/data/indicators?indicator=403&indicator=404&indicator=10403&start=2015&end=2025&geoUnitType=NATIONAL (file `uis_403_404.json`) |
| CK-206 | Start months across the 183 countries (UIS, with Eurydice dates for 37 European countries): September 94, January 27, October 21, February 12, August 10, March 8, April 5, July 3, June 2, May 1. | derived | Count over CK-205 and CK-210 in `ck-B-school-by-country.tsv` |
| CK-207 | 39 countries start in January or February (38 from UIS, plus NCL as an estimate). These are the southern hemisphere and some tropics, e.g. AUS, NZL, ZAF, KEN, TZA, UGA, ZMB, ZWE, ARG, COL, MYS, SGP, BGD, LKA). April starts: IND, JPN, PAK, ECU, PER. March: BRA, CHL, KOR, URY, AFG, BTN, PAN, LSO. | derived | Filter over CK-205 (UIS 403) |
| CK-208 | UIS end months follow a statistical convention, not the last teaching day. The USA is coded July start and May end (a reporting year). ESP, HRV and SVN are coded with an August end, while Eurydice shows June. | derived | Comparison of UIS 403/404 with Eurydice calendar dates (CK-210) |
| CK-209 | UIS also gives, for all 183 countries, the official primary entry age (6 in 135 countries, 7 in 31, 5 in 17) and the length of primary school (6 years in 112 countries, 5 in 29, 4 in 23, 7 in 17). Lower secondary is 3 years in 97 countries and 4 years in 59. | sourced | UIS API indicators 299905, 299932, 999976, 999978: https://api.uis.unesco.org/api/public/data/indicators?indicator=299905&indicator=299932&indicator=999975&indicator=999976&indicator=999977&indicator=999978&indicator=400&start=2024&end=2025&geoUnitType=NATIONAL (file `uis_ages_durations.json`) |
| CK-210 | Eurydice publishes exact 2023/24 school calendars for 37 European systems: first and last day, and every holiday block by level and region. | sourced | https://eurydice.eacea.ec.europa.eu/sites/default/files/2023-09/Calendars_Country_Sheets_2023_2024_0.xlsx (sheet All_data) |
| CK-211 | In Europe the school year starts on 1 September in 11 systems. Malta starts last (end of September). Finland can start as early as 9 August. | sourced | Eurydice, The organisation of school time in Europe 2023/2024, p.5: https://eurydice.eacea.ec.europa.eu/sites/default/files/2023-09/Organisation_of_school_time_2023_2024.pdf |
| CK-212 | Most European systems give 8 to 12 weeks of summer holiday. It is 8 weeks or less in the French Community of Belgium, Denmark, some German Länder, France, the Netherlands, some Swiss cantons, Liechtenstein and Norway. It is over 12 weeks in Greece, Latvia, Malta, Albania, most Italian regions, and some levels in Ireland, Portugal and Iceland. | sourced | Same PDF, p.5 |
| CK-213 | From the Eurydice dates, the summer break runs from 6.7 weeks (DNK) to 13.9 weeks (ITA, Veneto). The median of 33 countries is 10.9 weeks. DEU, CHE and NLD are set to 6 weeks (`estimate`) because their dates vary by region. | derived | `eury_cal.py` on CK-210: last day to the same start date one year later, or the listed summer block |
| CK-214 | Countries where the coded end month and next start month are adjacent (25 countries: AUS, BGD, BLZ, BRN, BWA, FJI, GBR, GHA, IDN, IND, JPN, KEN, LKA, MAC, NAM, NZL, PAK, PNG, PRK, SGP, SWZ, TZA, ZAF, ZMB, ZWE) have their long break in the middle of the coded school year, or about 6 weeks at the year boundary. Their long-break length is set to 6 weeks. | estimate | Rule in `build.py`; months alone cannot place the break |

### Instruction time: days and hours a year

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-215 | OECD (EAG 2025 data, reference 2024/25) gives compulsory instruction days per year for 40 of our countries. The OECD average is 185.7 days in primary and 184.5 in lower secondary. The EU-25 average is 180.7 days. | sourced | OECD Data Explorer, dataflow DSD_EAG_IT@DF_EAG_IT_ORG: https://sdmx.oecd.org/public/rest/data/OECD.EDU.IMEP,DSD_EAG_IT@DF_EAG_IT_ORG,1.1/all?dimensionAtObservation=AllDimensions&format=csvfilewithlabels (unit D_Y) |
| CK-216 | Primary instruction days range from 162.5 (BGR) to 214 (ISR). JPN has 201.8, AUS, BRA, COL, CRI, DNK and ITA 200, KOR, MEX, NOR and SVN 190, England and Scotland 190, DEU 188, USA and FRA 180, ESP 175. | sourced | Same dataflow (England = UKB, Scotland = UKM) |
| CK-217 | OECD average compulsory instruction time is 804 hours a year in primary and 909 hours in lower secondary. | sourced | OECD dataflow DSD_EAG_IT@DF_EAG_IT_ISCED: https://sdmx.oecd.org/public/rest/data/OECD.EDU.IMEP,DSD_EAG_IT@DF_EAG_IT_ISCED,1.1/all?dimensionAtObservation=AllDimensions&format=csvfilewithlabels (unit H_Y, COMPUL) |
| CK-218 | Primary hours a year range from 472 (HRV) to 1,147 (CRI). Lower secondary ranges from 663 (HRV) to 1,200 (COL, DNK). USA: 974 and 1,023. JPN: 768 and 884. DEU: 725 and 886. FRA: 864 and 973. | sourced | Same dataflow |
| CK-219 | Across 39 OECD-data countries, compulsory instruction per school day (hours a year divided by days a year) has a median of 4.0 h in primary (range 2.7 to 5.7). Lower secondary has a median of 4.7 h (range 3.8 to 6.0). This is instruction only, without breaks and lunch. | derived | CK-216 to CK-218 |
| CK-220 | Eurydice 2024/25: across Europe, compulsory instruction averages 725 h a year in primary (under 500 h in Croatia, 1,000 h in Denmark) and 857 h a year in lower secondary (465 h in Ireland, 1,000 h or more in Denmark, Spain and the Netherlands). | sourced | Eurydice, Recommended annual instruction time in full-time compulsory education in Europe 2024/2025, Part I pp.14-15 (FRSE mirror of the Eurydice PDF): https://www.frse.org.pl/brepo/panel_repo_files/2025/07/08/r7nxi1/recommended-annual-instruction-time-in-full-time-c.pdf ; official page https://eurydice.eacea.ec.europa.eu/publications/recommended-annual-instruction-time-full-time-compulsory-education-europe-20242025 |
| CK-221 | Eurydice Figure 1 gives total hours per level for European countries outside the OECD: ALB 2,927 (primary) and 3,098 (lower secondary); BIH 2,700 and 3,008; MNE 2,682 and 2,698; SRB 2,457 and 3,407; CYP 4,760 and 2,522; MLT 4,701 and 2,330. | sourced | Same PDF, Figure 1 (p.12) |
| CK-222 | Per year, those totals give: ALB 585 and 774 h; BIH 540 and 752; MNE 536 and 674; SRB 614 and 852; CYP 793 and 841; MLT 784 and 777. | derived | CK-221 divided by grade counts (ALB, BIH, MNE 5+4; SRB 4+4; CYP, MLT 6+3). The grade counts are mine; for BIH, SRB and MNE they match the report's text on which countries have a gap of 200 h or more |
| CK-223 | Instruction days from the 2023/24 calendars for European countries outside the OECD: ALB 178, BIH 186, CYP 177, MKD 180, MLT 168, MNE 183, SRB 170. NLD has 190 (OECD has no NLD day count). | derived | Weekdays from first to last day minus listed holidays (`eury_cal.py` on CK-210). Overlapping regional holiday rows can bias this by a few days |
| CK-224 | TIMSS 2019 measures the total instruction hours a pupil gets in a year (principal-reported school days times hours per day). The international average is 895 h at grade 4 and 1,023 h at grade 8. | sourced | TIMSS 2019 International Results, Exhibits 12.2 and 12.3, pp.450-451: https://timssandpirls.bc.edu/timss2019/international-results/wp-content/themes/timssandpirls/download-center/classroom/T19_Ch12-mathematics-curriculum-instruction.pdf |
| CK-225 | TIMSS 2019 grade 4, total hours a year: PHL 1,225; PAK 1,218; ZAF 1,205 (grade 5); CHL 1,186; MAR 1,081; SAU 1,056; ARE 1,034; HKG 1,022; SGP 1,009; KAZ 732; RUS 663; IRN 627 (lowest). Grade 8: MAR 1,341 (highest); MYS 1,165; EGY 1,110; JOR 1,020; IRN 768 (lowest). | sourced | Same PDF (raw text extraction; the layout extraction misaligns rows) |
| CK-226 | Where both exist, TIMSS grade-4 hours are a median of 1.14 times the OECD compulsory primary hours (27 countries, range 0.95 to 1.82). Grade 8 is 1.12 times lower secondary (17 countries). Actual school instruction time is higher than the compulsory minimum. | derived | CK-218 vs CK-225 |
| CK-227 | Intended and actual hours for 13-year-olds (1996/97) differ widely. PHL: 1,467 intended, 960 actual (-53%). RUS: 893 and 1,467 (+64%). IDN: 1,120 and 999. THA: 1,167 and 1,167. ARG: 913 and 888. PRY: 1,080 and 1,189. BRA: 667 and 800. | sourced | World Bank (2001), Double-shift secondary schools, Table 1, p.4: https://documents1.worldbank.org/curated/en/819151468740107174/pdf/multi0page.pdf |
| CK-228 | India's RTE Act schedule sets 200 working days and 800 instruction hours for classes 1-5. It sets 220 days and 1,000 hours for classes 6-8. | reported | Lok Sabha answer and Meghalaya notification, via search: https://eparlib.nic.in/bitstream/123456789/952318/1/AU2483.pdf ; https://www.megeducation.gov.in/edu_dept/notices_and_circulars/Old/No-EDN-RTE-226-2011-40.pdf |
| CK-229 | China's compulsory curriculum (2022 edition, as implemented by the provinces): 39 school weeks a year, of which 35 are teaching weeks. Pupils have 26 lessons a week in grades 1-2, 30 in grades 3-6 and 34 in grades 7-9. Lessons are 40 minutes in primary and 45 in lower secondary. | reported | Guangdong and Beijing implementation documents, via search: https://edu.gd.gov.cn/attachment/0/560/560274/4495766.pdf |
| CK-230 | This gives China about 669 instruction hours a year in primary and about 876 in lower secondary, over about 195 school days. | derived | CK-229: lessons x weeks x minutes; 39 weeks x 5 days |

### The school day: start and end times

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-231 | In the USA (2020-21), the average start time of public K-12 schools is 8:13 a.m. Elementary schools start at 8:16, middle at 8:11 and secondary/high at 8:07. 9% of public high schools start before 7:30. | sourced | NCES, Characteristics of 2020-21 public and private K-12 schools (NTPS First Look), p.3 and Table A-5: https://nces.ed.gov/pubs2022/2022111.pdf |
| CK-232 | In 2017-18 the average start time of US public high schools was 8:00 a.m. | reported | Search summaries of NCES NTPS 2017-18 (e.g. https://kappanonline.org/start-time-us-high-schools-nces/) |
| CK-233 | England expects a minimum school week of 32.5 hours from September 2024. This counts registration to home time, including breaks and lunch. It equals 8:45 to 15:15 on five days. It is not statutory. | reported | Search summary of the DfE note and school pages: https://dera.ioe.ac.uk/39119/1/Minimum_expectation_for_length_of_school_week_-__information_note.pdf ; https://nasuwt.org.uk/static/909eb3a1-3235-4e28-b02ee66ad3886143/Directed-Time-and-the-32-5-hour-Week-Briefing-England.pdf |
| CK-234 | Typical school days quoted by blogs: Japan 8:45 to 15:15; France 8:00 or 8:30 to 16:00 or 16:30, with Wednesday off or a half day; China 7:30 to 17:00 with a lunch break of about 2 hours, often eaten at home; Kenya 8:00 to 15:30 plus 75 minutes of games and clubs. | reported | https://www.teachstarter.com/au/blog/school-hours-around-the-world/ ; https://babyyumyum.com/?p=8792 |
| CK-235 | In Latin America the traditional school day is 4 to 5 hours, in a morning shift or an afternoon shift. | reported | World Bank blog: https://blogs.worldbank.org/es/latinamerica/am-rica-latina-es-muy-corta-la-jornada-escolar |
| CK-236 | Mexican primary schools (2025-26): the morning shift runs 8:00 to 13:00 and the afternoon shift 13:30 to 18:00. | reported | https://www.adn40.mx/mexico/2025-09-02/por-que-cambia-el-horario-clases-en-primaria-y-secundaria |

### Shift schooling

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-237 | Brazil's law sets a minimum of 800 hours over at least 200 school days. In 2024, 19.1% of ensino fundamental enrolments were full-time (7 hours or more), up from 14.4% in 2022. | reported | OECD dataflows above; https://diariodonordeste.verdesmares.com.br/ceara/censo-escolar-com-ceara-na-lideranca-veja-os-estados-do-brasil-com-maiores-taxas-de-matriculas-de-tempo-integral-1.3639115 (confidence note: sourced (800 h / 200 days, OECD CK-216/CK-218); reported (19.1%)) |
| CK-238 | Example double-shift times: first shift 7:40 to 12:40, second shift 13:00 to 18:00. A Zambian triple shift runs 7:00 to 10:45, 11:00 to 14:45 and 15:00 to 18:45. Administrators fill the morning shift first and put the overflow in the afternoon. | reported | Orkodashvili (2009), quoting Bray (2000), pp.3-4: https://files.eric.ed.gov/fulltext/ED510593.pdf |
| CK-239 | A school day of much more than 5 hours is hard to fit in a double-shift system (for example 7:30 to 12:30 and 13:00 to 18:00). Turkey and Romania have significant numbers of double-shift schools. | sourced | World Bank (2001), pp.1 and 4 (same PDF as CK-227) |
| CK-240 | 80% of secondary classes in Malaysia run in double shifts. In Brazil, about 60% of upper secondary students attend night school. | reported | World Bank (2001) quoting World Bank (1986) and World Bank/IADB (2000), p.1 footnote 3 |
| CK-241 | Bangladesh: 79.6% of government primary schools (52,048) ran double shifts in 2024. In 2023, 19% ran a single shift. Double-shift pupils get up to 34% fewer learning hours. | reported | APSS 2024 via The Daily Star: https://www.thedailystar.net/opinion/views/news/bangladesh-needs-better-education-infrastructure-prevent-primary-dropouts-3977821 ; https://www.dhakatribune.com/bangladesh/367342/12-515-more-schools-to-end-double-shifts-in |
| CK-242 | Tanzania (about 2005): two thirds of classrooms and up to a quarter of teachers were assigned to double-shift teaching. These are shares of classrooms, not pupils. | reported | EFA GMR 2008 p.110, quoted in Orkodashvili (2009), p.1 |
| CK-243 | Senegal: 14% of primary pupils were in double-shift schools by 1988. | reported | Search summary only (World Bank-linked case study; document not opened) |

### Pupils' and students' own time

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-244 | India Time Use Survey 2024: 89.3% of children aged 6-14 took part in learning activities, and those who did spent about 413 minutes (6.9 h) a day on them. | sourced | MoSPI press release, 25 Feb 2025, p.1: https://mospi.gov.in/sites/default/files/press_release/PR_TUS_25022025.pdf |
| CK-245 | ATUS 2007 (USA): people aged 15-19 spent 3.1 hours a day on educational activities, averaged over all days of the year. People who attended class on a weekday spent 5.2 hours in class. People who did homework on a weekday spent 2.4 hours on it. | sourced | BLS ATUS 2007 news release, p.2: https://www.bls.gov/news.release/archives/atus_06252008.pdf |
| CK-246 | European university students spend about 48 hours a week on study and paid work: 16 h of taught studies (classes on campus), 18 h of personal study and 14 h of paid work (EUROSTUDENT VIII). | reported | Search summary of https://www.eurostudent.eu/download_files/documents/Eurostudent8_2024_240618_Chapter_B5_bf.pdf |

### Weekends and the working week

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-247 | Official (public-sector) weekend across the 183 game countries: 153 Sat+Sun, 15 Fri+Sat, 10 Sunday only, 4 Friday only, 1 Fri+Sun. 58 of the 153 Sat+Sun rows are assumed, because W1 does not list those countries. | derived | TSV column `weekend_days`; https://en.wikipedia.org/wiki/Workweek_and_weekend |
| CK-248 | Fri+Sat weekend: Bahrain, Bangladesh, Algeria, Egypt, Iraq, Israel, Jordan, Kuwait, Libya, Maldives, Oman, Qatar, Saudi Arabia, Syria, Yemen. | reported | https://en.wikipedia.org/wiki/Workweek_and_weekend |
| CK-249 | Friday-only weekend (Saturday to Thursday week): Iran, Djibouti, Somalia; Afghanistan probably too. Iran and Afghanistan treat Thursday as a half or free day in many offices. | reported | https://en.wikipedia.org/wiki/Workweek_and_weekend ; https://www.hireborderless.com/post/weekend-rest-days-around-the-world |
| CK-250 | Afghanistan sources conflict. W1's table gives Sunday–Thursday (Fri+Sat), W1's list says Fri+Sat since 2015, an aggregator says Thu+Fri, and 2021 news calls Saturday "the first day of the Afghan working week". | reported | https://www.newarab.com/news/taliban-turn-back-afghan-govt-employees-returning-work |
| CK-251 | Sunday is the only legal weekly rest day, with a 6-day legal norm, in India, Pakistan, Mexico, Philippines, Colombia, Bolivia, Honduras, Equatorial Guinea, North Korea and Uganda. Government offices in several of these work Monday to Friday. | reported | https://en.wikipedia.org/wiki/Workweek_and_weekend |
| CK-252 | Brunei has the only split working week: Monday–Thursday plus Saturday. Friday and Sunday are rest days. The Indonesian province of Aceh does the same. | reported | https://en.wikipedia.org/wiki/Workweek_and_weekend |
| CK-253 | UAE federal government moved to a Saturday–Sunday weekend on 1 Jan 2022. Monday–Thursday is 8 hours (07:30–15:30). Friday is 4.5 hours. Sharjah's government took Friday to Sunday off (4-day week). | reported | https://shrm.org/topics-tools/news/uae-workweek-changes-public-sector ; https://mondaq.com/employee-rights-labour-relations/1148748/a-new-working-week-in-the-uae-from-1-january-2022 |
| CK-254 | UAE private sector: normal hours are 8 hours a day or 48 hours a week (Art. 17, Decree-Law 33/2021). Overtime is at most 2 hours a day. During Ramadan, hours are 2 hours shorter every day. | sourced | https://u.ae/en/information-and-services/jobs/Sector-of-employment/employment-in-the-private-sector/working-hours |
| CK-255 | Saudi Arabia switched from Thu+Fri to Fri+Sat by royal order on 23 Jun 2013, effective 29 Jun 2013. Oman had switched one month earlier. For the private sector, Friday stayed the only legal rest day. | reported | https://www.crowell.com/en/insights/client-alerts/saudi-arabia-changes-weekend-to-friday-saturday-from-29-june-2013 ; https://www.arabnews.com/news/455923 |
| CK-256 | Iran: in May 2024 parliament passed a Fri+Sat weekend with a 40-hour week. The Guardian Council vetoed it in Aug 2024. In Mar 2025 parliament voted a Thu+Fri weekend and 40 hours for government offices. No source confirms that this law is in force. | reported | https://iran-times.com/majlis-makes-weekend-thursday-friday/ ; https://www.rferl.org/a/iran-economy-workers-saturday-off/32949834.html |
| CK-257 | Nepal's cabinet gave public offices and schools both Saturday and Sunday off (reported 5 Apr 2026), citing disrupted petroleum supply. Before this, Saturday was the only weekly holiday. Nothing says the private sector changed. | reported | https://www.sharesansar.com/newsdetail/government-decides-two-day-weekly-holiday-for-public-offices-and-educational-institutions-2026-04-05 |
| CK-258 | Pakistan's federal government notified a 4-day week (Monday–Thursday) for its offices on 11 Mar 2026, "effective immediately". Health and education staff are excluded. | sourced | https://www.radio.gov.pk/11-03-2026/govt-notifies-four-day-workweek-for-offices-effective-immediately |
| CK-259 | The Philippines put some executive offices on a temporary 4-day week from 9 Mar 2026 (Memorandum Circular 114). Agencies had to cut power and fuel use by 10–20%. Several cities copied it. | reported | https://www.philstar.com/headlines/2026/03/31/2518043/marcos-orders-government-offices-reduce-power-fuel-use/amp/ |
| CK-260 | Malaysia: Johor returned to a Sat+Sun weekend on 1 Jan 2025. Kedah, Kelantan and Terengganu keep Fri+Sat. | reported | https://en.wikipedia.org/wiki/Workweek_and_weekend |
| CK-261 | Israel: the standard week is 42 hours (43 before Apr 2018). The usual 5-day week runs Sunday–Thursday. 6-day workers get a short Friday. Weekly rest is 36 hours in a row and includes the Sabbath for Jewish workers. | reported | https://www.cwsisrael.com/israel-hours-of-work-and-rest-law/ |
| CK-262 | The ILO describes the classic standard week as Monday–Friday (Sunday–Thursday in the Arab States) or Monday–Saturday for a 6-day week. The usual day runs from 8 or 9 am to 5 or 6 pm. | sourced | ILO22, PDF p. 67 |
| CK-263 | No source found gives the share of workers who work 6 days a week in low- and middle-income countries. India's 2023–24 PLFS microdata show hours on day 6 for about 31% of records, but this figure is unweighted and not a population estimate. | reported | https://microdata.gov.in/nada/index.php/catalog/213/variable/V252 |
| CK-264 | The TSV marks 47 of 183 countries as having a 6-day private-sector norm (`workweek_days`=6). For 10 this rests on W1 ("Monday–Saturday" or "Saturday–Thursday"). The rest is judgement based on a 44–48 hour limit plus notes on Saturday work. | estimate | TSV column `workweek_days` |

### Working hours: legal limits and long hours

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-265 | In 2012, 36% of countries set the normal weekly limit at 40 hours, and 52% set it above 40 hours (31% at 48 hours). 3% set it below 40 hours and 1% above 49 hours. | sourced | ILO12, PDF p. 10 |
| CK-266 | In the Arab States, 48 hours is the normal limit in 90% of countries (2012). Syria was the only one with 40 hours. | sourced | ILO12, PDF p. 11 |
| CK-267 | Weekly hours limit in the TSV: 161 of 183 countries have a value (140 from the ILO 2012 annex, 21 from news or W1). 10 of these have no universal limit (e.g. Germany, Ireland, Netherlands, Nigeria, Sri Lanka). Of the 150 numeric values, 64 are 40 h, 47 are 48 h, 16 are 44 h and 14 are 45 h. The median is 44 h. | derived | TSV column `weekly_hours_limit` |
| CK-268 | Chile cut its week from 45 to 44 hours on 26 Apr 2024 and to 42 hours in Apr 2026. It reaches 40 hours in 2028. A 4-day/3-day split is allowed under the 40-hour regime. | sourced | https://www.mintrab.gob.cl/ley-40-horas-conoce-las-principales-medidas-que-comienzan-a-regir-el-26-de-abril/ |
| CK-269 | Colombia's limit fell from 48 to 42 hours in steps: 47 (2023), 46 (2024), 44 (2025), then 42 from 15 Jul 2026 (Law 2101 of 2021). The premium for Sunday or holiday work rose to 90% in Jul 2026. | reported | https://www.bloomberglinea.com/latinoamerica/colombia/a-quienes-si-y-a-quienes-no-les-aplica-la-reduccion-de-la-jornada-laboral-desde-hoy-en-colombia/ |
| CK-270 | Mexico's constitutional reform was published on 3 Mar 2026. The limit stays at 48 hours in 2026, then falls 2 hours a year to 40 hours in 2030. The rule of 1 paid rest day per 6 days worked stays. | reported | https://www.cuatrecasas.com/en/global/labor-and-employment/art/mexico-reduced-working-hours ; https://www.poresto.com/mexico/2026/3/4/reforma-laboral-ya-es-oficial-publican-en-el-dof-reduccion-de-jornada-a-40-horas-en-mexico.amp.html |
| CK-271 | In 2019, 35.4% of the world's workers worked more than 48 hours a week. The share was 57.1% in Southern Asia, 47.7% in Eastern Asia, 40.0% in Northern Africa, 25.1% in the Arab States, 11.0% in Europe and Central Asia, and 4.5% in Eastern Europe. | sourced | ILO22, PDF p. 30 |
| CK-272 | Before COVID-19, the world average was about 43.9 hours a week. One fifth of workers worked under 35 hours. | sourced | ILO22, PDF p. 64 |
| CK-273 | More than 41% of informal workers work over 48 hours a week, against 28% of formal workers. By sector, 48.8% of workers in wholesale and retail trade work over 48 hours, against 20.3% in public administration. | sourced | ILO22, PDF pp. 34, 62 |

### Public holidays

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-274 | Holiday data coverage: Google feeds exist for all 183 countries, with years 2021–2031 for most. Nager.Date covers 151 of 183 countries. It is missing India, Iran, Saudi Arabia, UAE, Thailand, Malaysia, Pakistan, Israel, Nepal, Sri Lanka and 22 others. | derived | GC; NG |
| CK-275 | National full-day public-holiday days in 2025 (GC, without substitute days; AUS, CAN, CHE, GBR and BIH use one large region): the minimum is 5 (Bosnia, state level only), Q1 12, median 14, mean 14.7, Q3 16, and the maximum is 33 (Maldives). | derived | TSV column `public_holidays` |
| CK-276 | Countries with the most holiday days in 2025: Maldives 33, Nepal 31, Myanmar 31, China 28 (bridged "golden week" days included), Philippines 27, Macau 27, Indonesia 27, Bangladesh 27, Sri Lanka 26 (12 full-moon Poya days). | derived | GC |
| CK-277 | Countries with the fewest holiday days in 2025: Bosnia 5 (the entities add more), England 8, Laos 8, Mauritania 8, Zurich 9, Germany 9 national (+0 to 4 by state), Spain 9 national, Israel 9. | derived | GC |
| CK-278 | 44% of national holiday days in 2025 do not have a fixed Gregorian date. By rule: Islamic lunar 416 days, other lunisolar or astronomical 331 (Chinese, Hindu, Buddhist, Jewish, equinoxes), Western Easter 316, nth-weekday rules 79, Orthodox Easter 41. 75 countries have Islamic-date holidays, 78 have other lunisolar ones and 126 have Easter-linked ones. Only 6 have none. | derived | GC, rules classified across 2021–2031 |
| CK-279 | Islamic dates are not final in advance. For 2027, GC flags 485 holiday events in 80 countries as "tentative". The 2025 dates are confirmed. | derived | GC |
| CK-280 | Nager and Google agree within 2 days for 87 of 151 countries. Google is 3 or more days higher for 57 countries. Nager leaves out Eid and other Islamic holidays for many countries: Algeria 5 vs 14, Bahrain 5 vs 19, Djibouti 3 vs 12, Libya 5 vs 18, Yemen 5 vs 16. It also counts multi-day blocks once: China 6 vs 28, Vietnam 4 vs 16. | derived | NG; GC |
| CK-281 | Substitute or transfer days ("observed", "day off for") appear in the 2025 feeds of 49 countries. Examples: Japan, Korea, Russia, Kazakhstan, South Africa, Kenya, Thailand, Indonesia, Argentina, Uzbekistan. | derived | GC |
| CK-282 | Half-day public holidays appear in the feeds of 8 countries: Bolivia, China, Germany, Guatemala, Honduras, Iceland, Luxembourg, Turkey. Examples: 24 and 31 Dec in Germany, the eve of each Eid in Turkey, and 8 Mar (women) and 4 May (youth) in China. | reported | GC |
| CK-283 | About 20% of national holiday days in 2025 (545 of 2,660) fell on the country's own weekend. The median country lost 20%. Without substitute rules, these days add no time off. | derived | GC + TSV weekend |
| CK-284 | Holiday days across all countries in 2025 bunch in April (411), January (340), May (339), June (331), December (308) and March (280). February has the fewest (78). Easter and Eid al-Fitr both fell in Mar–Apr 2025. | derived | GC |
| CK-285 | 1 January is a national holiday in 165 of 183 countries. 25 December is one in 135. | derived | GC |
| CK-286 | The longest runs of non-working days in 2025 (weekend plus holidays): Maldives 13 days from 20 Mar (Eid al-Fitr), Azerbaijan 12 (Novruz), Qatar and Indonesia 11 (Eid), Myanmar 11 (Thingyan), Kyrgyzstan 11 (May), Vietnam 9 (Tet). The median country's longest run is 4 days. | derived | GC + TSV weekend |
| CK-287 | Working days in 2025 (non-weekend days minus national holidays): minimum 236 (Maldives), median 250, maximum 303 (Mexico, counted with its legal 6-day week). | derived | GC + TSV weekend |
| CK-288 | Real calendars include one-off closures. The 2025 feeds show a day of mourning (USA, 9 Jan), extra Christmas closures (USA, 24 and 26 Dec), weather days (Sri Lanka, Maldives) and an election day (Azerbaijan). | reported | GC |
| CK-289 | China raised its statutory holiday days from 11 to 13 on 1 Jan 2025 by adding Spring Festival Eve and 2 May. Swaps of weekend days continue, with usually no more than 6 workdays in a row. 26 Jan and 8 Feb 2025 were make-up workdays. | reported | https://www.hsfkramer.com/notes/employment/2024-posts/prc-public-holiday-entitlement-increased-to-13-days ; https://www.linklaters.com/knowledge/publications/alerts-newsletters-and-guides/2024/november/27/china-expands-public-holidays-embracing-a-more-employee-friendly-holiday-arrangement |
| CK-290 | W2's count of *paid* public holidays: median 12 across 148 game countries, range 0–27. A 0 means the law does not require holiday pay (USA, UK, Japan). It does not mean there are no holidays. | derived | https://en.wikipedia.org/wiki/List_of_minimum_annual_leave_by_country |

### Annual leave

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-291 | In 2012, 96% of countries legislated a minimum of paid annual leave. 43% gave 20 or more working days, and 4% had no universal minimum. ILO Convention 132 sets at least 3 working weeks. | sourced | ILO12, PDF p. 21 |
| CK-292 | Minimum annual leave (working days, 5-day-week basis) for 174 of 183 countries (129 from ILO 2012, 45 from W2): the minimum is 5 (China, Nigeria, Philippines), the median 20, the mean 17.3 and the maximum 30 (UAE, Bahrain, Kuwait, Yemen). The USA has no statutory minimum. | derived | TSV column `annual_leave_min_days` |
| CK-293 | ILO 2012 leave figures are old. Mexico raised its minimum in 2023, and W2 now shows 12 days against ILO's 6. Many W2 figures are calendar days rather than working days (Russia 28 vs ILO 20). ILO and W2 agree within 2 days for 138 of 174 countries. | derived | ILO12; W2 |

### What holidays do to activity

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CK-294 | Calendar effects on output: in Germany, 1% more working days raises output by 0.3% on average. One extra working day raises monthly manufacturing output by 3.4% in Jan–Nov but only 2.6% in December. Industrial output on a bridge day is about one third lower than on a normal working day. | sourced | https://bundesbank.de/resource/blob/707332/37db302ea1ea94fa8fd6b71916c17991/mL/2012-12-calendar-data.pdf |
| CK-295 | One extra working day changes quarterly GDP by 0.1% to 0.4% across euro-area countries. In Q1 2006, euro-area GDP grew 0.6% after adjustment but fell 2.5% unadjusted. | sourced | https://www.ecb.europa.eu/pub/pdf/other/mb200608_focus04.en.pdf |
| CK-296 | China's statistics bureau says Lunar New Year cuts industrial output for about 4 days before the festival and 15–20 days after. Jan–Feb 2019 industrial growth was 5.3% as reported and 6.1% without the holiday effect. Analysts disputed this adjustment. | reported | https://www.bloomberg.com/news/articles/2019-03-14/china-blames-lunar-new-year-for-underwhelming-economic-data |
| CK-297 | A 2011 study estimated that cutting Ramadan workdays by 2 hours costs about 7.7% of one month's GDP: UAE about USD 1.4 bn, Saudi Arabia about USD 2.4 bn, GCC USD 5.8 bn. This is a scaling model, not measured output. | reported | https://www.thenationalnews.com/uae/what-is-the-economic-impact-of-ramadan-1.867969 |
| CK-298 | W1 lists 20 countries with special Ramadan hours. Examples: Saudi Arabia 30–36 h/week, Qatar 25 h/week, Kuwait 25 h/week, Oman 30 h/week, and Bahrain 6 h/day for Muslim staff. | reported | https://en.wikipedia.org/wiki/Workweek_and_weekend |

## What this means for us (Proposal)

**1. The schedule's test is where people are, hour by hour.** *Spec 54 (daily schedules, to write); demo flow beats 4 and 7.*
- Targets: the five place shares and the sleep share per hour, for weekday, Saturday and Sunday-or-holiday × all, employed and not employed (CK-1 to CK-30; the full curves are in `ck-D-tempogram.tsv`). Every one of these cells has an effective sample of 786 or more (CK-47).
- Compare hourly means, not 15-minute slots: diary times heap on :00 and :30 (CK-45).
- Tolerances: ±3 pp per hour on weekdays for all and employed (root-mean-square error over the day ≤ 1.5 pp); ±4 pp at weekends; the two weekday travel peaks within ±25 % in height and ±30 min in time. That is about twice the sampling half-width and above the year-to-year drift of ≤ 2 pp (CK-46, CK-48).
- Shape checks: on weekdays sleep crosses 50 % near 06:30 and 22:00 (CK-3, CK-4); the home share bottoms out at 45–50 % near noon on weekdays and stays near 58 % at weekends (CK-8, CK-9); employed people hold a 46–49 % workplace plateau from 10:00 to 14:00 (CK-14); the evening travel peak is higher than the morning one, and weekends have no rush hour (CK-18, CK-20).
- **The test must stay a test.** The schedule draws its inputs from separate tables (anchors, rule 2; outings, rule 3), never from these curves, so passing it means something.

**2. Anchors are drawn once per pop, from measured distributions.** *Spec 54; refines TU rule 5.*
- Waking on weekdays: median 07:00 (p10 05:00, p90 09:30); employed people 06:30; at weekends 08:00 (CK-39 to CK-41).
- Bedtime before a weekday: median 22:05 (p10 20:30, p90 00:05); before a weekend day 22:30 (CK-42, CK-43).
- First work start on weekday workdays: median 07:48 (full time 07:40; part time 09:00 with p90 16:00) (CK-31, CK-32). Work time: median 480 min, full time 495 (CK-37). Last work end: median 17:00 (CK-36).
- The commute leg: median 20 min, p90 51 (CK-38); ACS one way: median about 23.5 min, p90 about 59 (CK-103, CK-104).
- The tails are real and must exist: 5.5 % of workers leave home before 05:00 and 12.7 % after noon (CK-98 to CK-100); 3.8 % of weekday workdays are already under way at 04:00 (CK-2); about 4 % of people are away from home at night (CK-2, CK-44).
- Lunch is eaten at the workplace: working anywhere dips from 65 % to 47 % of employed people at noon while the workplace share barely moves (CK-15).
- The test uses the ATUS departures; ACS stays a cross-check (each cumulative bin within ±8 pp), because the two differ by about 20 minutes at the median (CK-102).

**3. Outings come from their own table.** *Spec 54.*
- An outing is a stop away from home and the workplace, with its travel legs (CK-50). A day is a chain: home, then work if it is a workday, then stops, then home. 37 % of weekday stops follow another stop (CK-73), so pops chain errands instead of going home between them.
- How many: on weekdays 32.2 % of people make no outing, 25.1 % make one and 14.3 % make four or more (CK-54). Employed people make none on 38.6 % of workdays but on only 20.3 % of their weekdays off (CK-56, CK-57).
- Which kind: shopping or an errand on 35.0 % of weekdays, leisure or sport on 19.9 %, care (school runs, appointments) on 15.0 %, a visit to another home on 13.3 %, eating out on 11.9 % and religious or civic activity on 4.0 % (CK-60 to CK-64). On Sunday religious or civic outings rise to 16.6 % (CK-66).
- When: a start-hour table per kind (CK-77 to CK-82). Errands spread from 08:00 to 18:00, meals cluster at noon and 17:00–19:00, care at 07:00 and 15:00, visits and sport at 16:00–19:00, and Sunday worship at 09:00–11:00. On workdays 22 % of outings come before work, 23 % in a break and 55 % after (CK-83).
- How long: a stay table per kind. Errands last a median of 30 min, a meal out 60, a weekday visit 60 (120 on Sunday) and worship 110 (CK-67 to CK-71). Each leg takes a median of 15 min (CK-72).
- Work itself: 22.7 % of weekday work time is at home, and 21.4 % of workdays are spent almost entirely at home (CK-89, CK-90). 58 % of workplace days include a meal there, 9 % leave at midday (CK-93, CK-94), and 5 % are split shifts (CK-92).
- Days at home emerge from these draws instead of being set: nobody leaves home on 15.0 % of weekdays, 19.8 % of Saturdays and 23.4 % of Sundays (CK-28), and the test checks it.
- Other countries take the share of people doing each kind of outing from their own diary where one exists (Serbia's shopping, socialising, worship and sport: CK-160, CK-161, CK-163, CK-165). Stay lengths and start hours keep the US shape, shifted by the country's timing (rule 6).

**4. The upper-middle default runs on Serbia 2021–22.** *`data/timeuse/`; decision 0071.*
- Serbia is the only upper-middle 24-hour diary published by sex × weekday/Saturday/Sunday × labour status × age, with 10-minute time-of-day curves (CK-118, CK-119). Serbia and Türkiye are both upper-middle in the game's own country data (the World Bank list in `data_raw/ilostat/wb_countries.json`).
- Türkiye 2014–15 is the contrasting profile: women do paid work on 18.8 % of days against 53.3 % for men (CK-136); meetings and religious activity reach 33.6 % of people a day and 67.5 % at 65 and over (CK-143, CK-144). It has no day-type split, so it borrows Serbia's (CK-156; `estimate`).
- China (2018, 2024) adds own-use production and household farming (23.1 % of people a day, 41.1 % in rural areas, CK-175) and children aged 6–17 (CK-190).
- Mexico's ENUT ([TU-30](daily-lives-and-time-use.md)) and Colombia's (CK-127, CK-128) are activity lists, not diaries: use them only for the gap between the sexes, never for minutes.
- The base countries borrow: `base_high` from the United States (ATUS), `base_upper_middle` from Serbia, `base_lower_middle` and `base_low` from India ([TU-24](daily-lives-and-time-use.md) to [TU-29](daily-lives-and-time-use.md)). A real country with its own diary uses it (United States, Serbia, Türkiye; China in part).

**5. The day type acts through participation more than duration.** *Spec 54.*
- Serbia: employment participation is 47.5 % on weekdays, 27.5 % on Saturday (× 0.58) and 16.1 % on Sunday (× 0.34); minutes per participant 7:09, 6:13 and 5:43 (CK-153, CK-154, CK-156). China: 44.0 % on weekdays and 24.5 % on rest days, 7:40 and 7:42 (CK-174). United States: employed people work on 79.4 %, 28.6 % and 24.6 % of weekdays, Saturdays and Sundays (CK-29).
- Serbian full-time workers work on 77.7 % of days for 7:27, about 5.4 days a week (CK-166, CK-167). Employed pops are calibrated on this, not on population-wide rates divided by the employment rate.

**6. Countries shift the timing; the US gives the shape of where.** *Spec 54.*
- Sleep crosses 50 % at 06:30 and 22:00 in the United States (CK-3, CK-4), at 07:30 and 22:40 in Serbia (CK-195) and at 08:00 and 23:00 in Türkiye (CK-194). The work and study peak is at 10:45 in the United States (CK-12), 11:50 in Serbia (CK-197) and 11:10 in Türkiye, with a lunch dip (CK-196).
- A country's anchors shift by its offset from the United States. The US place curves give the shape, and each diary country's activity curves (HETUS `tus_20startime`, `tus_00startime`) are its own test (CK-109 to CK-114). No other country publishes where people are, only what they do.
- Bedtime is social and wake time biological: countries differ more in bedtime (CK-108, [TU-44](daily-lives-and-time-use.md)).

**7. A school calendar per country.** *Spec 53 (the calendar, to write) for school days; spec 54 for a child's day; spec 46.*
- The school year's start month is known for 182 of 183 countries: 94 start in September and 39 in January or February; Japan, India and Pakistan start in April with the long break in mid-year (CK-205 to CK-207, CK-214). Entry age and years per level are known for all 183 (CK-209).
- School days a year: OECD average 186 (CK-215); 52 countries have real values, the rest take their income group's default (`estimate`, `ck-B-school-by-country.tsv`).
- A day at school is instruction plus breaks. Instruction is a median 4.0 h a day in primary and 4.7 h in lower secondary (CK-219); presence is about 6 h in primary (US public schools start at 08:13 on average, CK-231; England expects a 32.5-hour week, CK-233). Where hours a year divided by days exceed 6 h, lunch is at school.
- Shift schooling: a morning shift of about 07:30–12:30 and an afternoon shift of 13:00–18:00 (CK-236, CK-238, CK-239); 80 % of Bangladesh's government primary schools run two shifts (CK-241). Afternoon-shift children are at home or out in the morning, so the streets look different.
- Children aged 6–17 in China learn on 98.4 % of days for 8:12 (all days, CK-190); in India 89.3 % of children aged 6–14 learn for 6.9 h (CK-244). ATUS has no one under 15; at 10:45 on weekdays 24 % of 15–24-year-olds are at school, averaged over all months (CK-26).
- The school week follows the national weekend (rule 8). Six-day school weeks are an `estimate` everywhere (Gaps).

**8. Weekends and holidays per country, with dates computed from rules.** *Spec 53.*
- Weekends: 153 countries rest on Saturday and Sunday, 15 on Friday and Saturday, 10 on Sunday only, 4 on Friday only and 1 on Friday and Sunday (CK-247). Changes carry a date (the UAE in 2022, Saudi Arabia in 2013: CK-253, CK-255), and some 2026 changes are temporary (Nepal, Pakistan, the Philippines: CK-257 to CK-259).
- Two worker populations: offices follow the official weekend; about 47 countries have a six-day private week (CK-264, `estimate`), India and Mexico among them (CK-251).
- Day types go by role, not by name: the main rest day (Sunday, or Friday where the weekend is Friday and Saturday) and holidays use the `sunday_holiday` tables; the other weekend day uses `saturday`; a six-day week's Saturday is a short workday (`estimate`: the surveys measure only Saturday and Sunday).
- Holidays: a median of 14 days off a year, from 5 to 33 (CK-275 to CK-277). 44 % of them move from year to year (CK-278), so they are stored as rules (fixed date, Easter, nth weekday) plus a table of lunar and lunisolar dates for 2021–2031 from the downloaded feeds (CK-274, CK-279). 49 countries move a holiday that falls on a weekend (CK-281); elsewhere about 20 % of holidays are lost to weekends (CK-283).
- Activity around holidays comes in a later slice: bridge days produce about a third less (CK-294), the Lunar New Year slowdown (CK-296), Ramadan hours about 2 h a day shorter in the Gulf (CK-254, CK-298).
- Annual leave: a median of 20 working days, from 5 to 30, none by law in the United States (CK-292). Leave is drawn per worker, not per country.

**9. One activity list across surveys.** *`data/timeuse/`.*
- The HETUS major groups with a crosswalk to ICATUS 2016 (CK-200, CK-202); travel kept as its own activity (CK-201); a small religious and civic activity weighted by country, from 6 % of people a day in Serbia to 34 % in Türkiye (CK-163, CK-143, CK-203); own-use production for rural places (CK-175).

## Gaps

- **Where people are is measured only in the United States.** HETUS, China and India publish activities, not places. Elsewhere the place curves are a model (rule 6) tested against activity curves.
- **The ATUS location imputation** (40.7 % of minutes) is our modelling choice; the 4 % of sleep away from home depends on it and cannot be checked from ATUS. Night shifts and true bedtimes are cut by the 04:00 diary boundary (CK-2, CK-44). The 2025 file has no October diaries (CK-49).
- **No one under 15** in ATUS, and no diary anywhere gives children's places. Children follow the school calendar (rule 7), checked only against participation (CK-190, CK-244).
- **Upper-middle diaries:** Serbia's sample size was not found; Türkiye has no day-type or labour-status table (CK-117); China 2024 has no split by sex or day type (CK-126). South Africa 2010 (the report would not download) and Thailand 2015 (CK-129) were not read.
- **School:** start and end times are sourced for the United States only (CK-231); the share of pupils on shifts is known for four countries; the school week pattern is an `estimate` everywhere; travel time to school was not found.
- **Weekends:** 58 countries are assumed to rest on Saturday and Sunday because no source lists them; Afghanistan and Iran are in conflict (CK-250, CK-256); the 2026 changes may be temporary; no source gives the share of six-day workers (CK-263); the ILO law tables are from 2012.
- **Holidays** come from public calendar feeds, an aggregator (`reported`); Islamic dates after 2026 are tentative (CK-279); regional holidays are handled only for a few federal countries.
- **Carried from TU:** HETUS 2020 daily tables for countries other than Serbia were not used; per-trait personality minutes are still missing.
