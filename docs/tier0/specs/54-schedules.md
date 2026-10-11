# 54 · Daily schedules: where any pop is at any time

**Status:** Implemented and tested on MSVC (2026-10-11), written with its code; slice 1 of the fourth demo flow's second half ([0069](../../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md) build order 1). **Stage:** after Tier 0. **Needs:** [02](02-draw.md), [28](28-pop-levels.md), [40](40-time.md), [47](47-country-towns.md), [53](53-calendar.md). **Decisions:** [0069](../../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md) rules 5 and 6, [0070](../../decisions/0070-broken-commitments-are-noticed.md), [0071](../../decisions/0071-days-from-time-use-data-dispositions-nudge.md), [0072](../../decisions/0072-the-calendar-real-dates-day-kinds-by-role.md). **Research:** [daily-lives-and-time-use](../../research/daily-lives-and-time-use.md) (TU), [calendars-school-and-clock-hours](../../research/calendars-school-and-clock-hours.md) (CK).

## Purpose

A pop is somewhere at every minute. Today the block's pops are driven tick by tick by the pop AI, and everyone else is a count. This spec gives every pop, lazy or embodied, a **day as a function**: of the pop, the day number and its country's time-use table, nothing else. Reading where a pop is costs a few keyed draws and no stepping, so the town's thousands have days at the price of one lazy snapshot each ([compute budget](../../design/compute-budget.md)). The day is what the schedules, the queues (IQ), the commitments ([0070](../../decisions/0070-broken-commitments-are-noticed.md)) and the stations ([stations](../../design/stations.md) rule 6) read.

Slice 1 is the function and its data, a headless mode that shows a town's day, and the row on the Atlas's pop card. Wiring the function into the pop AI (a warm pop walking to its workplace) and the broken-commitment events are later slices.

## What the code forces

- The calendar ([53](53-calendar.md)) gives each civil day a survey type (weekday, saturday, sunday_holiday) per sector; the tables are by survey type, so a holiday is a Sunday to the schedule.
- The generator's pops ([47](47-country-towns.md)) carry an occupation string ("student", "out of work", "retired", a job), an age and a sex; the simulation's lazy pops ([28](28-pop-levels.md)) carry age and sex, and employment only as cohort shares. The schedule takes a small `SchedulePerson` that either can fill.
- Keyed draws must be integer-only and cross-compiler ([01](01-quantity.md)): the normal is a mean of uniforms, the lognormal commute uses `detmath` or a table, never `std::exp` on the hot path.

## State

| Field | Notes |
| --- | --- |
| `TimeUseTable` | id, source, method, the ILOSTAT weekly-hours target; rows of (activity, employment or any, sex or any, survey day or any, participation, hours when done); the sleep block (weekday and weekend hours, night SD, bedtime mean and SD); the work-pattern shares; the departure half-hour shares; the commute lognormal; `work_scale` (rule 2) |
| `SchedulePerson` | key (pop id mixed with the world seed), age, sex, employment (full time, part time, not employed, student, retired, child), has a workplace, married, a young child |
| `ScheduleSkeleton` | drawn once per pop: the work pattern, weekend worker, the departure minute, the bedtime minute, the one-way commute |
| `DayPlan` | the survey day; segments in order covering 00:00 to 24:00 exactly, each (start, end, activity, place kind, commitment flag); works, studies, minutes of work and sleep |
| `data/timeuse/<id>.toml` | one table per real country that has a diary survey (USA from ATUS 2025, IND from TUS 2024) and `index.toml`: which table each income group borrows and the ILOSTAT target per group |

Nothing is saved: a plan is derived whenever asked. What a pop actually did differs from its plan only through events (a possession, an order, an illness), and those are the events the world already records.

## Rules

1. **A day is a participation draw, then a duration draw** ([0071](../../decisions/0071-days-from-time-use-data-dispositions-nudge.md) rule 1). For each activity with a row for this pop and day type, the pop does it with the row's participation, and if so for the row's hours when done, times a noise factor in [0.75, 1.25] drawn per day (mean of uniforms, no tails). Population means are never used: an employed pop on a US weekday works on 87 % of days for 8.45 h, not 7.4 h every day (TU-4).
2. **A country reads one table.** A real profile reads its own file when one exists; otherwise, and for the base countries, its income group's table (`index.toml`: high and upper-middle from USA, lower-middle and low from IND), with the work rows' hours scaled by the group's ILOSTAT weekly target over the table's own (TU-36, TU-37), so a generated country's average lands on its group's row.
3. **The most specific row wins.** A row naming the pop's employment and sex beats one naming only one of them, which beats a row for everyone; a row for the day type beats a row for any day.
4. **A fixed skeleton, drawn once per pop from its key alone** ([0071](../../decisions/0071-days-from-time-use-data-dispositions-nudge.md) rule 2): the work pattern (TU-15 shares: day, evening, night, rotating), whether it is a weekend worker (TU-16), the departure half-hour from the departure shares (TU-41, TU-42), the bedtime (the table's mean and SD), the one-way commute (lognormal, TU-40, mean 27.6 min, about 10 % over an hour). A pop without a workplace (TU-6: 35 % of days worked at home, and pops whose workplace is not placed) works at home with no commute.
5. **The template chain** ([TU proposal rule 9](../../research/daily-lives-and-time-use.md)), per day, from the skeleton and the draws of rule 1:
   - **sleep** from the bedtime of the previous night to the wake time: the table's weekday or weekend hours plus the night's noise (SD 70 min, TU-45), so the wake minute moves and the bedtime holds (TU-44: society governs bedtime);
   - **self-care** 45 min after waking;
   - if the pop works today: **commute**, **work** for the drawn hours, **commute** back; a day worker starts at the departure minute plus the commute; an evening worker at 15:00; a night worker at 22:00 (its sleep then falls in the day); a rotating worker takes day, evening or night by the day number;
   - if a student on a school day ([53](53-calendar.md) rule 8): **study** at the school from 08:00 for the drawn hours;
   - then, in this order and only if drawn: **household** (at home), **shop** (out), **care** (at home), **civic** (out), **social** (out);
   - **leisure** fills whatever is left before bedtime, at home; a working day with no room left gives leisure its last 30 min and shortens the discretionary segments in reverse order.
   - A work segment's start and the return home are **commitments** ([0070](../../decisions/0070-broken-commitments-are-noticed.md) rule 1): the plan marks them; the holders (the workplace roster, the household) are the rosters of [28](28-pop-levels.md), read by the later slice that raises the events.
6. **Where the pop is at a tick** is the segment of the plan for the civil day the tick falls in, with the day's survey type from the country calendar's private sector ([53](53-calendar.md) rule 4) and the clock's minute of day ([05](05-clock.md): tick 0 is 08:00). The place kinds are home, workplace, school, out, travelling; which building they are is the pop's home and workplace records ([47](47-country-towns.md), [51](51-dwellings.md)) and is not this spec's business.
7. **Calibration is a test, not a knob.** Over many pop-days, participation and hours when done reproduce each row within tolerance, and an employed pop's weekly hours (five weekdays and two weekend days) land within 10 % of the table's ILOSTAT target. The USA table gives 35.6 h against 37.80 (TU-37); IND gives its own.
8. **Dispositions are not in this slice.** [0071](../../decisions/0071-days-from-time-use-data-dispositions-nudge.md) rule 4 (a disposition shifts a draw by at most one survey SD) waits for the pop's dispositions to exist as data (PB); the function has the hooks (the participation and the noise factor) and no caller moves them yet.

## Headless mode: `zs --day`

`zs --day [--seed N] [--country base_low|base_high|...] [--people N] [--date YYYY-MM-DD]` generates a town for the country ([47](47-country-towns.md)), gives every pop a `SchedulePerson` from its record, and prints: the day type; a table by hour of how many are asleep, at home, at work, at school, out or travelling, for that date and for the following Sunday; and three pops' days as lines (a full-time worker, a student, a retired pop) with their skeleton. The Atlas's pop card shows the same lines as its "today's schedule" row, and the Try panel runs the mode.

## Invariants

- Segments cover 0 to 1440 exactly, in order, none empty.
- The same key, day number and table give the same plan (A1); another seed gives another plan.
- A pop never works on a day its row's participation is 0; a child never works; a retired pop never studies.
- `work_scale` changes work hours only; participation and every other activity are untouched.

## Tests (`engine/sim/tests/test_schedule.cpp`)

- Coverage and order of segments for 1,000 pop-days across employments and day types.
- Determinism: the same inputs twice; a changed seed differs.
- Participation and hours when done over 20,000 pop-days per row, within 2 points and 5 %, for the USA table's full-time weekday and weekend work rows and the social rows.
- Weekly hours of full-time USA pops within 10 % of 37.80; of lower-middle pops (IND, scaled) within 10 % of 43.91.
- Departure: about 57 % of day workers leave between 06:00 and 08:29.
- Sleep: the within-pop night-to-night SD between 55 and 85 min.
- Students study only on school days of the calendar; nobody at a workplace on a day with work participation 0.
- `where_at` on the one world's calendar: a full-time pop is at the workplace at 10:00 on Monday 11 January 2027 when it works that day, and at home at 03:00.

## As built (2026-10-11, MSVC release, every test green)

- `zs --day --seed 1 --country base_high --people 300`: New Wington, USA table scaled 0.92 (high income's 34.80 h over the US 37.80). Monday 11 January 2027: 158 of 300 work (the retired, whose occupation reads "retired janitor", do not), 7.8 h on average when they do; at 10:00 about 90 at work, 34 at school, most of the rest at home; at 03:00, 295 asleep and the night shifts at work. Sunday 17 January: 48 work, 5.1 h; the afternoon sees about 85 out. A hairdresser on the evening pattern leaves at 12:18, beds at 00:06, commutes 66 min.
- `zs --day --seed 2 --country base_lower_middle --people 300`: IND table scaled to 43.91 h. Monday: 162 of 300 work, 7.1 h; 66 at school from 08:00; the evening sees 57-67 out (India's 93 % socialising, community and religion, TU-28).
- The tests: coverage and order over 1,080 pop-days; determinism; USA full-time weekday participation 0.873 and 8.45 h, weekend 0.296 and 5.49 h, part-time 0.543 and 5.42 h, social 28.4 and 35.2 % reproduced within tolerance; weekly hours per employed person within 10 % of 37.80 (USA) and 43.91 (lower-middle, IND scaled), weighting the sexes by the table's share of women among the employed; 57 % of day workers leaving between 06:00 and 08:29; a retired pop's sleep SD between 55 and 85 min; at 10:00 on the first Monday over 80 % of day workers who work that day are at the workplace and nearly all are home at 03:00; students study on the school day and not on the Sunday.
- The Atlas's pop card shows the day as its "today's schedule" row, and the Try panel runs `--day`.

## Not in this spec

- Walking: a warm pop's body following its plan (the pop AI, [24](24-pop-ai.md)), and the embodied block's pops, who are post-outbreak survivors and follow the run, not a schedule.
- The broken-commitment events and the ladder of [0070](../../decisions/0070-broken-commitments-are-noticed.md) (phone, ask, report, search): the plan marks the commitments; raising the events is the next slice.
- Chores split inside the household (TU proposal rule 7), children's care tied to a parent's presence, multiple jobs, annual leave, sick days.
- The activity-transition model fitted to diary microdata (TU proposal rule 9, later); cold cohorts as shares by clock hour (EF).

## Open

- The departure shares are an estimated shape around two sourced points (TU-41, TU-42); the ACS B08302 values are still to fetch (TU-43).
- IND's day-type split of work borrows the US ratio; India's survey reports no split. The six-day week (CK) makes Saturday a workday there, which the table says as an estimate.
- Mexico (upper-middle's natural source) has a stylised survey, not a diary (TU-30); upper-middle borrows USA scaled to 42.42 h. A HETUS or Japanese table would serve high-income countries that are not the US.
