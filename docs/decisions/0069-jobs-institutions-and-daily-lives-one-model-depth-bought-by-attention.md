# 0069 · Jobs, institutions and daily lives: one model for all, depth bought by attention

- **Date:** 2026-10-10
- **Status:** Decided by the user as intent (2026-10-10). The six rules and the first list of hands-on jobs are defaults taken by Claude ([0052](0052-one-executable-one-simulation.md)): a later decision can change them. The user's words (lightly cleaned):

  > "On jobs, their functions and professions: I don't know how far we should take it, computationally. But we need to cover many types of jobs and the gameplay that comes from them, activities and so on, simulating a Second Life, if you wish to play at that level. I would like things to make sense: how police, and everything a city has, is dealt with in real life. Not everything has everything, and those are things that bleed from the simulation above. Things are slow and take time. Pops have lives: rest time, free time, work time. Some are workaholics, some are more family-minded, some lonely. Pop behaviour and the lore from it must make sense with the world around them. I don't know how far we can go in behaviour simulation computationally; the project is already too complex."
- **Builds on:**
  - [0019](0019-activities-and-delegation.md) and [0020](0020-activity-answers.md): activities, where playing by hand is optional and the pop's own draw is the default;
  - [0058](0058-offices-open-layers-single-player-is-multiplayer-with-ai.md): stations open layers;
  - [0060](0060-one-fight-three-views-behaviour-from-jobs-and-skills.md): behaviour from jobs and skills;
  - [0061](0061-the-world-runs-without-the-player.md);
  - specs [33](../tier0/specs/33-establishments-and-chains.md) (establishments; hours and shifts named as next), [40](../tier0/specs/40-time.md) (wake ticks, jump forms), [46](../tier0/specs/46-education-labour-occupations-settlement.md) and [47](../tier0/specs/47-country-towns.md) (occupations);
  - research [pop-behaviour](../research/pop-behaviour.md) (PB), [PKC-157](../research/playing-knowing-commanding-moving.md) (schedules from time-use surveys), [SIMS-10](../research/the-sims.md) (no rabbit holes), [EF](../research/coarse-layers-from-fine-rules.md) (coarse from fine);
  - the [compute budget](../design/compute-budget.md): cost is attention × speed, not world size.

## Decision

1. **Jobs are data, never code.** An occupation record holds:
   - the workplace kind;
   - a weekly schedule pattern (shifts, nights, on call);
   - its duties (the actions and activity families it may use);
   - the establishment process its labour feeds (spec 33);
   - its skills;
   - its station view (verbs and map layers).

   Adding a profession adds a data entry, not a system.
2. **Three depths of the same job:**
   - **Lived** (every pop, always). The pop is where its schedule puts it, at a real workplace, and its labour enters that establishment's real process. There are no rabbit holes (SIMS-10). Cost is close to nothing: position is a function of time, and the work is part of the establishment's jump form (specs 33, 40).
   - **Played by time** (any job, when possessed). The shift runs at the real workplace, compressed, stopping for the decisions that the establishment's real state raises: a customer, a shortage, a late delivery. If the player does nothing, the result equals the pop's own draw (0020).
   - **Played by hand** (a short list). Hands-on activities and a station view. The first list follows what this game needs: police (patrol officer and detective), medic and nurse, firefighter, soldier, farmer, mechanic, shopkeeper and driver. More join through data where the eight interaction types of [activities](../design/activities.md) already cover them.
3. **Institutions are queues with staff.** Police, hospitals, fire services, courts, schools and councils share one mechanic:
   - cases come in (calls, patients, fires, crimes);
   - they wait in a queue;
   - staff with skills serve them, with service times from real data;
   - each case ends in an outcome.

   "Things are slow and take time" comes from this: an investigation takes weeks because the queue and the staff make it so. At Macro a queue is counts and rates; up close each case is a record. The coarse queue is derived from the fine one (EF).
4. **Not every place has everything.** Which services a settlement has follows its size and its country: thresholds and catchments from real data. Smaller places are served by larger ones ([pop lore](../design/pop-lore-and-custom-pops.md) §7, jurisdiction). Staffing and money come from above:
   - a country's budget and policies (spec 32, spec 39 staffing edicts) size each institution;
   - the city runs it;
   - pops work in it.

   A shortage above shows as long queues below.
5. **A pop's day comes from time-use surveys, shaped by its dispositions.** Sleep, work, commuting, household, care, leisure and social time follow real time-use distributions by age, sex, employment and country (PKC-157). Dispositions (PB §1) shift them:
   - high conscientiousness and weak family ties give overtime (a workaholic);
   - strong family bonds give more care and household time;
   - low sociability and few bonds give solitary leisure (lonely).

   The schedule is a function of (pop, time); only events change it, and only those changes are stored.
6. **Behaviour depth is bought by attention, never by world size.**
   - **Hot pops** (bodies near what a player watches: a few dozen, up to about 1,000) run the full decision step (PB: dual utility every 5 s).
   - **Warm pops** follow their schedules and wake only on events (spec 40 wake ticks).
   - **Cold pops** are cohort rates derived from the warm ones (EF).

   On the user's PC, reading one lazy pop costs about 1.5 µs whether the world holds 1 million or 10 million (compute budget). "Second Life" depth where you look is affordable; depth everywhere at once is neither needed nor attempted.

## Consequences

- **Data, not systems:**
  - occupation records gain a schedule, duties and a station;
  - a services table gives thresholds by settlement size and country;
  - time-use tables per country;
  - service times per institution.
- **Research to do:**
  - time-use surveys: ATUS (United States), HETUS (Europe), and others where they exist;
  - service thresholds and catchments: central place theory; police, doctors and fire stations per population by country;
  - institution service times: case clearance, emergency waits.
- **Build order:**
  1. The calendar and the schedule function, from time-use data (the fourth demo flow's slice 1).
  2. Institutions as queues, police first.
  3. Played-by-time shifts.
  4. Hands-on jobs, one at a time.
- **A guard on complexity.** A new job or institution that needs code beyond data must say which of these six rules it cannot fit. That case goes to the user before anything is built.
