# 53 · The calendar: dates, day kinds, holidays and school days

**Status:** Implemented and tested on MSVC (2026-10-11), written before its code. **Stage:** after Tier 0. It is the first half of the fourth demo flow's slice 1 ([0069](../../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md) build order 1); daily schedules (spec 54) are the second half. **Needs:** [05](05-clock.md), [36](36-fields-and-weather.md), [40](40-time.md), [42](42-place.md), [49](49-real-country-profiles.md). **Decision:** [0072](../../decisions/0072-the-calendar-real-dates-day-kinds-by-role.md). **Research:** [calendars-school-and-clock-hours](../../research/calendars-school-and-clock-hours.md) (CK), [daily-lives-and-time-use](../../research/daily-lives-and-time-use.md) (TU).

## Purpose

The world has ticks and a day count, but no dates. People's lives run on dates: a Monday, a public holiday, the first day of school. This spec gives the one world a real calendar:

- **one function from tick to civil date and time**, which every system reads. That includes the weather, which today runs 8 hours off the clock;
- **for each country**, which days are workdays, short days, rest days, holidays and school days, from the CK data.

Daily schedules (spec 54) read the day kinds. Nothing else changes behaviour in this slice.

## What the code forces

- `Clock::hour()` puts tick 0 at 08:00 (`kStartTimeOfDayTicks`, [05](05-clock.md)); `fields::time_of_year(tick)` does not. So the weather's warmest hour, 15:00 in its own frame, shows at 23:00 on the clock, and its day changes at 08:00. Noise, light and the pop AI read `Clock::hour()` and are right.
- Weather years are 365 days, and their keyed anomalies sum to zero ([36](36-fields-and-weather.md)). The exactness test depends on it.
- Systems save their own state. Adding a system changes the save bytes and the stage hashes; [42](42-place.md) set the precedent: re-pin once, no format bump.
- The one world's country is a real profile (ISO3, [49](49-real-country-profiles.md)) or a base country (`base_<income>`).

## State

| Field | Notes |
| --- | --- |
| `CivilDate` | year; month 1–12; day 1–31 |
| `CivilTime` | day number (days since 1970-01-01); date; weekday (0 Monday … 6 Sunday); day of year (0–365); second of day; hour |
| `CountryCalendar` | id; weekend (7-bit mask, Monday first); main rest day; official short days (mask); private week days (5 or 6); holidays (fixed month-days, Western and Orthodox Easter offsets, dated holidays per year 2021–2031 with full or half day and a name); school (first day as month and day, long break weeks, instruction days, school week days 5 or 6); source and confidence per part |
| `CalendarSystem` | the start day number (saved) and the country calendar (loaded from data at setup; the saved id must match it) |
| `data/setup/calendar/<id>.toml` | one per real country (183) and per base country (4), written by `tools/data/calendars.py` from `data_raw/calendar/` and the CK tables; never edited by hand |

## Rules

1. **Civil dates.** Proleptic Gregorian calendar, with leap years by the 4/100/400 rule. Day numbers count from 1970-01-01 (day 0, a Thursday). Conversion both ways is integer arithmetic (days from civil, civil from days), exact for years 1600–2400.
2. **Civil time.** Tick 0 is 08:00 on the start date. `civil_time(start_day, tick)` takes the seconds since the start date's midnight (tick × 0.5 s + 8 h); the day number and the second of the day follow by floor division. `Clock::hour()` and `Clock::calendar_day()` keep their meaning and agree with it.
3. **The start date** is world state, saved by the calendar system. The default is Monday 11 January 2027 ([0072](../../decisions/0072-the-calendar-real-dates-day-kinds-by-role.md)). A scenario may set another before setup closes ([0062](../../decisions/0062-the-game-loop-attention-possession-and-late-starts.md)).
4. **The weather reads civil time.**
   - Year: the civil year.
   - Weather day: the day of the year. In a leap year, 29 February repeats 28 February (weather day 58) and later days shift back by one. A weather year is therefore always 365 days, and spec 36's exactness holds for it.
   - Hour: the civil hour.

   The daily cycle now peaks at 15:00 on the clock, and the day's mean and rain change at midnight.
5. **Day kinds for a sector.** For a date, the official sector (government, offices, schools) and the private sector each get one kind:
   - A full public holiday is a **holiday** for both sectors in this slice. Essential services are a job pattern of spec 54.
   - An official weekly rest day is a **rest day** for the official sector. For the private sector it depends on the working week:
     - with a five-day week, it is a rest day;
     - with a six-day week, the main rest day stays a rest day and any other weekend day is a **short day**.
   - An official short day on a non-weekend day, or a half-day holiday, is a **short day**.
   - Every other day is a **workday**.
6. **Survey day types**, which say which time-use table applies (CK, TU):
   - a holiday, or the main rest day, is `sunday_holiday`;
   - any other weekly rest day is `saturday`, and so is the private six-day week's short weekend day;
   - every other day is `weekday`.

   The main rest day is set per country by the data tool:
   - Friday where the weekend includes Friday;
   - Saturday in Israel and Nepal;
   - Sunday everywhere else.
7. **Holidays by year.**
   - For 2021–2031 the dated table is the truth. It includes substitute days and lunar holidays.
   - Outside it, only the fixed month-days and the Easter offsets apply. Western Easter comes from the anonymous Gregorian computus; Orthodox Easter from the Julian computus, converted to the Gregorian date.
   - Lunar and lunisolar holidays and substitute days are missing outside the table (Open 1).
8. **School days.** The school year that starts in year Y runs from its first day S(Y) to the next one, S(Y+1).
   1. **First day.** The year begins on the first school-week day on or after the data's first day in year Y. School-week days are the official workdays for a five-day school week, or every day but the main rest day for a six-day one.
   2. **Long break.** The data's long break weeks sit at the end of the span, just before S(Y+1).
   3. **Candidates.** The candidates are the school-week days in the span, outside the long break, that are not full public holidays.
   4. **Short breaks.** If there are more candidates than the data's instruction days, the excess becomes two short breaks of consecutive candidates. The larger half is centred at one third of the teaching span, the rest at two thirds.
   5. **Too few days.** If there are fewer candidates than instruction days, every candidate is a school day and the year is marked short.

   A date is a school day if it remains a candidate in the school year that contains it: the one that started in Y, or in Y−1.
9. **Base countries borrow a real country's calendar** ([0072](../../decisions/0072-the-calendar-real-dates-day-kinds-by-role.md)): high income from the United States, upper-middle from Serbia, lower-middle from India, low from Ethiopia. The file names the country it copies.
10. **The one world.**
    - Setup adds the calendar system with its country's calendar: `Place::profile` if set, otherwise `base_<income>`.
    - The window shows the date ("Mon 11 Jan 2027 08:00"), and the day's kind and name when it is not a workday ("Holiday: Christmas Day"; "Rest day"). It also says whether schools are open.
    - Headless text keeps "day N HH:MM", because logs and tests read it.
11. **Pure and cheap.**
    - Every answer is a pure function of the start day, the country calendar and the tick or date.
    - Holiday lookups search a sorted array per year.
    - School years are computed on demand and cached; the cache is derived and not saved.
    - The calendar system never wakes (`kNever`, [40](40-time.md)).

## Data (`tools/data/calendars.py`)

The tool reads its inputs from `data_raw/` (outside git, CK Evidence) and writes `data/setup/calendar/`. Every value keeps its source and its confidence.

- **Weekend, short days, private week, main rest day** come from the CK weekend table ([CK-247](../../research/calendars-school-and-clock-hours.md) to [CK-264](../../research/calendars-school-and-clock-hours.md); `ck-C-calendar-by-country.tsv`).
  - Mostly `reported`. For 58 countries Saturday and Sunday are assumed.
  - One current record per country: the dated changes of 2026 are not applied (`estimate`, [CK-257](../../research/calendars-school-and-clock-hours.md) to [CK-259](../../research/calendars-school-and-clock-hours.md)).
- **Holidays** come from the public holiday calendar feeds (`data_raw/calendar/gcal/<ISO3>.ics`, [CK-274](../../research/calendars-school-and-clock-hours.md), `reported`).
  - Taken: events described as "Public holiday", tentative ones included. Left out: observances and regional holidays.
  - A day whose name says "half" is a half day.
  - Fixed month-days are the ones present in all 11 years. Easter offsets are the ones at the same distance from Western or Orthodox Easter in all 11 years.
- **School** comes from the CK school table (`ck-B-school-by-country.tsv`):
  - first month from UIS ([CK-205](../../research/calendars-school-and-clock-hours.md)), with the exact first day from Eurydice for 37 European systems ([CK-210](../../research/calendars-school-and-clock-hours.md));
  - instruction days, long break weeks and school week days.
  - The US first day is set to 25 August (`estimate`): UIS codes the US year as starting in July ([CK-208](../../research/calendars-school-and-clock-hours.md)).
  - Other first days are the 1st of the month.

## Invariants

- Converting a day number to a date and back gives the same number. Day 0 is a Thursday.
- `civil_time(start, 0)` is 08:00 on the start date. One day of ticks later it is the next date at the same time.
- Every year visits each of the weather's 365 days once, and one of them twice in a leap year.
- No school day is a full public holiday or falls outside the school-week days. A school year never has more school days than its instruction days.
- All 187 calendar files load. Every country has at least one holiday in each year of the table.

## Tests (`engine/sim/tests/test_calendar.cpp`)

- **Dates (A1):** every day from 1900 to 2100 converts both ways; 2027-01-11 is a Monday; 2024-02-29 exists and 2100-02-29 does not; 1970-01-01 is day 0.
- **Civil time (A1):** tick 0 is 2027-01-11 08:00; 16 hours later it is 2027-01-12 00:00; `Clock::calendar_day()` agrees.
- **Weather (A1):** in one day the warmest hour is 15:00 and the coldest 03:00 on the clock; the day's mean changes at civil midnight; in 2028, 29 February has 28 February's mean and 1 March is weather day 59; spec 36's exactness test still holds.
- **Easter (A1):** Western Easter is 2027-03-28 and 2028-04-16; Orthodox Easter is 2027-05-02 and 2028-04-16. Serbia's Orthodox Good Friday in the table, 2027-04-30, is Orthodox Easter minus 2.
- **Day kinds (A1):**
  - Saudi Arabia: Friday is a rest day and the main one, Saturday is a rest day, Sunday a workday.
  - United States: Monday 2027-07-05, the substitute for Independence Day, is a holiday.
  - Serbia: 2027-01-07 is a holiday.
  - India: Saturday is an official short day.
  - United Arab Emirates: Friday is an official short day.
- **Survey day types (A1):**
  - United States: a Saturday is `saturday`; a Sunday and Thanksgiving 2027-11-25 are `sunday_holiday`.
  - Saudi Arabia: Friday is `sunday_holiday` and Saturday `saturday`.
  - Israel: Saturday is `sunday_holiday` and Friday `saturday`.
- **School (A1):**
  - The United States' 2027 school year has 180 school days, none on a weekend or a holiday, none in the long break.
  - Japan's year starts in April; Argentina's in February or March.
  - India's six-day school week has Saturday school days.
  - No country's year has more school days than its instruction days.
- **Data (A1):**
  - All 187 files load.
  - Every country has at least one holiday in each year 2021–2031.
  - The median number of holiday days in 2025 is within 2 of [CK-275](../../research/calendars-school-and-clock-hours.md)'s 14.
- **The one world (A1):** `make_run` adds the calendar; save and load keep the start day; the goldens are re-pinned once (the weather's hour and the calendar's save).

## As built

- `engine/sim/include/zs/calendar.h`, `src/calendar.cpp`:
  - dates: `CivilDate`, `days_from_civil` and `civil_from_days` (Howard Hinnant's algorithms, `constexpr`), `weekday_of`, `is_leap_year`, `day_of_year`, `easter_western`, `easter_orthodox`;
  - time: `kDefaultStartDay`, `CivilTime`, `civil_time`, `format_civil`, `civil_now`, `start_day_of`;
  - countries: `CountryCalendar` (`holidays_in`, `holiday_on`, `day_kind`, `survey_day`, `is_school_week_day`, `school_year`, `is_school_day`), `load_country_calendar`, `calendar_path`, `calendar_id`;
  - `CalendarSystem`, which saves the start day and the country id.
- The weather: `fields::weather_time(year, day_of_year, leap_year, hour)` gives the weather's day and hour, and `fields::temperature_c` takes that instead of a tick. The tick-based `time_of_year` is gone, so nothing can read the weather 8 hours off again. `rain_between_mm` keeps its own timeline, documented as not the world's clock; only spec 36's partition test uses it.
- `PlaceSystem` reads `civil_now` and caches the day's weather by civil day number.
- `make_run` adds the calendar system after the place system; `Tier0::calendar` points to it.
- The window's "Now:" line shows the date, the day's kind and holiday name, and whether schools are open, followed by the old "day N HH:MM".
- `tools/data/calendars.py` writes the 187 files (about 900 KB).
  - Holidays are stored as name indexes: "MM-DD:i" per year, with "h" for a half day.
  - Five federal countries take one large region: New South Wales (Australia), the Federation (Bosnia and Herzegovina), Ontario (Canada), Zurich (Switzerland), England (United Kingdom). Their 2025 holiday days come out at 12, 5, 12, 9 and 8, as in CK.
  - Nine feeds stop in 2026 or 2027, because those governments announce holidays year by year: United Arab Emirates, Azerbaijan, Bangladesh, Bhutan, Indonesia, Sri Lanka, Myanmar, Nepal and Pakistan. Their later years use the rules.
  - Israel and Oman have only moving holidays, so no rule survives beyond the table (Open 1).
- `engine/sim/tests/test_calendar.cpp`: 8 cases, as listed above.
  - The day-by-day date check covers 1900–2100, and the Easter windows 1900–2099.
  - The one-world place test now averages over the civil year 2027, a whole weather year.
- `test_fields.cpp` calls the new weather signature.
- The stage 9 golden is re-pinned on MSVC: the weather's hour and the calendar's save. Stages 6–8 have no place system and keep their goldens.
- Measured:
  - The median country has 14 full holiday days in 2025 (from 5 to 33).
  - The United States' 2027 school year starts on Wednesday 25 August 2027 and has exactly its 180 instruction days.
  - No country's 2027 school year exceeds its instruction days.

## Not in this spec

- Where pops are: spec 54.
- Activity around holidays: bridge days, Ramadan hours, the Lunar New Year (CK rule 8).
- Annual leave per worker: spec 54.
- Regional holidays and school regions.
- Time zones and daylight saving time: the clock is local time.
- Daylight by latitude and season.
- Dated changes of weekends.
- Holiday dates beyond 2031 for moving holidays.

## Open

1. **Holidays beyond 2031.** Lunar and lunisolar holidays and substitute days are missing outside the table. Israel and Oman have only moving holidays, so they have none at all there. The tabular Islamic calendar could fill the Islamic ones, within a day or two. Recommendation: when a scenario starts after 2030.
2. **Where the breaks fall.**
   - About 25 countries take the long break in the middle of the school year (Japan, India and others, [CK-214](../../research/calendars-school-and-clock-hours.md)); here it sits at the end.
   - Short breaks are placed by rule. Eurydice holds the real blocks for 37 European systems.

   The totals are right either way. Recommendation: fix the placement when a school matters on screen.
3. **Weekends.** 58 countries are assumed to rest on Saturday and Sunday, and Afghanistan and Iran have conflicting sources ([CK-250](../../research/calendars-school-and-clock-hours.md), [CK-256](../../research/calendars-school-and-clock-hours.md)). A primary-source pass would settle them.
