# 0072 · The calendar: real dates from a start date, one civil time, day kinds by role

- **Date:** 2026-10-11
- **Status:** Default taken by Claude ([0052](0052-one-executable-one-simulation.md)), from research [calendars-school-and-clock-hours](../research/calendars-school-and-clock-hours.md) (CK). A later decision can change any default here.
- **Why:**
  - The world has ticks and a day count, but no dates. Schedules ([0069](0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md) rule 5, [0071](0071-days-from-time-use-data-dispositions-nudge.md)) need weekdays, public holidays and school days.
  - Countries rest on different days: 153 rest on Saturday and Sunday, 15 on Friday and Saturday, and 15 on other patterns ([CK-247](../research/calendars-school-and-clock-hours.md)).
  - 44 % of public holiday days move from year to year ([CK-278](../research/calendars-school-and-clock-hours.md)).
  - The weather's daily cycle runs 8 hours off the clock: `fields::time_of_year` ignores the clock's 08:00 start.
- **Builds on:** [spec 05](../tier0/specs/05-clock.md) (tick 0 at 08:00), [spec 36](../tier0/specs/36-fields-and-weather.md) (365-day weather years), [spec 49](../tier0/specs/49-real-country-profiles.md) (real-country profiles), [0062](0062-the-game-loop-attention-possession-and-late-starts.md) (scenarios set a start date).

## Decision

1. **Real dates.** The world runs on the Gregorian calendar, leap years included. Tick 0 is 08:00 on the world's start date.
   - The default start is **Monday 11 January 2027, 08:00**. It is a Monday morning outside the year-end holidays of most countries. The Tier 0 run was tuned in the same season: its weather started on day 0 of the weather year, 1 January.
   - A scenario may set its own start date ([0062](0062-the-game-loop-attention-possession-and-late-starts.md)).
2. **One civil time.** Every system reads the date and the hour from one function, the weather included. Weather years stay 365 days long: in a leap year, 29 February repeats 28 February's weather.
3. **A calendar per country.** It holds:
   - the official weekly rest days and the main rest day;
   - official short days;
   - a five- or six-day private working week;
   - public holidays: dated for 2021–2031 from public holiday feeds, and beyond that from fixed-date and Easter-linked rules;
   - the school year.
4. **Days are classed by role, not by name.**
   - For a sector (official, or private): workday, short day, rest day or holiday.
   - For the time-use tables, three day types:
     - `weekday`;
     - `saturday`: the other rest day;
     - `sunday_holiday`: the main rest day, or a holiday.

   So Friday is the `sunday_holiday` day in Saudi Arabia, and Saturday is in Israel.
5. **Base countries borrow a real calendar:**
   - high income from the United States;
   - upper-middle from Serbia (the same country as its time-use table, CK rule 4);
   - lower-middle from India;
   - low from Ethiopia, the most populous low-income country with a full calendar row.

## Consequences

- [Spec 53](../tier0/specs/53-calendar.md) builds this. Schedules ([spec 54](../tier0/specs/README.md), to write) read its day kinds.
- Fixing the weather's hour changes outdoor temperatures through the day. The stage goldens are re-pinned once on MSVC.
- The window shows the date. Headless text keeps "day N HH:MM", because logs and tests read it.
