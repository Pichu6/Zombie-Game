# 0071 · A pop's days from time-use data; dispositions only nudge (refines 0069 rule 5)

- **Date:** 2026-10-10
- **Status:** Default taken by Claude ([0052](0052-one-executable-one-simulation.md)) from research [daily-lives-and-time-use](../research/daily-lives-and-time-use.md) (TU). It **refines rule 5 of [0069](0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md)**: that rule said "high conscientiousness and weak family ties give overtime (a workaholic)", which the research does not support.
- **Why:** personality is only weakly linked to time use, r ≈ .10–.25 ([TU-50](../research/daily-lives-and-time-use.md)). Compulsive workaholism goes with neuroticism and youth, not with conscientiousness ([TU-53](../research/daily-lives-and-time-use.md)). Employment, children, marriage and age matter far more ([TU-13](../research/daily-lives-and-time-use.md)).

## Decision

1. **Participation, then duration.** Each day a pop first draws *whether* it does an activity (by country, sex, age, employment and day type), then *how long*. Population means are never used: in the US, 39.6 % work on a given day for 7.65 hours, not everyone for 3 hours ([TU-2](../research/daily-lives-and-time-use.md)).
2. **A fixed skeleton plus daily noise.**
   - **Drawn once:** the job's schedule pattern at hire (US: 85 % regular daytime, 5 % evening, 3 % night, the rest rotating, split or irregular, by industry, [TU-15](../research/daily-lives-and-time-use.md)), the work start time ([TU-41](../research/daily-lives-and-time-use.md)), sleep timing and the household's chore split.
   - **Drawn per day:** optional activities and duration noise. Sleep varies by about 70–85 minutes (SD) from night to night ([TU-45](../research/daily-lives-and-time-use.md)).
3. **Three day types:** weekday, Saturday, and Sunday or holiday.
4. **Dispositions shift time use by at most about one standard deviation.** "Workaholic" splits in two:
   - **work drive** (conscientiousness): more work and study time;
   - **work compulsion** (about 8 % of employees, tied to neuroticism and youth): long hours with stress.

   "Lonely" lowers the daily chance of socialising, which is 30 % on a given day in the US ([TU-10](../research/daily-lives-and-time-use.md)).
5. **Country totals are the calibration target:** weekly hours worked by sex per country (ILOSTAT, [TU-37](../research/daily-lives-and-time-use.md)).
6. **The output is the shape activity-based travel models use** (activity, start, end, place, mode), which is exactly where a pop is at any time.

## Consequences

- Data: `data/timeuse/<country>.toml` (activity, group, day type, participation, hours when done, survey method), with survey categories harmonised before countries are mixed (TU §2).
- The calendar and schedule spec (the fourth demo flow's slice 1) is written from this decision and 0070.
