# Stations: who sets what, at every scope

The map of inputs of the simulation: at each scope, from a pop to a country, the stations (jobs and offices) that set a parameter in play, the pops who hold them, the tasks that holding means, the cost and delay of a change, and which spec produces the input today. Built so the count of stations is the honest count of what a government, a town, a company or a group needs, and so an AI holding a station changes the simulation's own numbers in real time ([0075](../decisions/0075-parameters-change-through-stations-setup-locked.md)).

Labels: **Decided**, **Proposal**, **Implemented**, **Validated** (see CLAUDE.md). The rules of §1 are Decided ([0075](../decisions/0075-parameters-change-through-stations-setup-locked.md), [0069](../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md)); every table row is a Proposal unless its "Today" cell names a built spec. Requested by the user on 2026-10-11 ("when we know how a government works, we know how many inputs we need; every input means a station and a pop using it"). Builds on [sandbox parameters](sandbox-parameters.md) §1.2 and §1.6, [macro layer](macro-layer.md) §6.3, [spec 39](../tier0/specs/39-orders-and-belief.md), the research [institutions-as-queues](../research/institutions-as-queues.md) (IQ), [services-by-settlement-size](../research/services-by-settlement-size.md) (SV), [worldviews-and-reputation](../research/worldviews-and-reputation.md) (WV) and [daily-lives-and-time-use](../research/daily-lives-and-time-use.md) (TU). It does not open a new line of work: it is the station column the catalogue asked for, written once so the schedules (spec 54), the police queue (IQ) and the services by size (SV) that are next in `docs/status.md` know which stations they are building.

## 1. Rules

1. **A station is a job with levers.** It is an occupation record ([0069](../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md) rule 1: jobs are data) whose tasks include setting one or more parameters of its scope. Most jobs have no levers; a cook cooks. A station is the exception, and the table below is the list.
2. **Levers are numbers rules already read.** A station never adds a verb pops do not have (sandbox parameters §1.1). Its change goes through the ordinary commands below it, with the signing as root cause, so it traces and it sums ([spec 39](../tier0/specs/39-orders-and-belief.md)).
3. **Every lever has a cost and a delay.** The cost is paid in the station's own currency: money, stock, trust, obedience, the holder's time. The delay is the chain of commands below (a levy takes a day per owner it reaches; a broadcast takes the hops of spec 39's retelling).
4. **The holder decides from their own parameters.** An AI holder reads the world through its beliefs (spec 39: source, age, hops, confidence), its interests and its worldview (WV), and chooses as the pop AI chooses anything else ([0071](../decisions/0071-days-from-time-use-data-dispositions-nudge.md): dispositions nudge). No station runs a script.
5. **Scope decides realism.** No station sets the world. The higher the scope, the fewer the holders and the longer the delay. A station exists where a real office exists; a parameter nobody sits at is derived.
6. **Holding is a job task in the day.** A station is held during work hours of the holder's schedule (spec 54, TU): a mayor who is asleep signs nothing. An empty station (the holder dead, absent, [0070](../decisions/0070-broken-commitments-are-noticed.md)) changes nothing until someone takes it, which is itself a parameter of the scope above (who appoints).
7. **Built from the queues outward.** Police, hospitals, fire, courts, schools and councils are queues with staff ([0069](../decisions/0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md) rule 3); their stations set the queue's parameters (staffing, priorities, hours). Which a settlement has follows its size (SV).

8. **A lever delivers only through the pops below it; collapse is derived** ([0076](../decisions/0076-collapse-is-derived-never-declared.md)). A station's change goes down a chain of commands carried out by pops at the stations and queues beneath it; where those pops are dead, absent, fled or unpaid, the chain stops and the lever reads as set while delivering nothing. Nothing switches a society off: the share of stations held, the queue lengths, the compliance of each chain and trust are readouts, and society fails or recovers as the sum of pops' own decisions to hold or leave their stations. Every row below therefore carries two more questions, answered as the row is built: who carries it out, and what makes them stop.

## 2. The stations, scope by scope

Columns: the station; who holds it (the occupation, and how many at a given size where research says); the levers (the parameters it sets, each a number a rule reads); cost and delay; **Today**: the spec that produces the lever, or the research it waits on, or "lacking".

### 2.1 Pop (self)

| Station | Holder | Levers | Cost, delay | Today |
| --- | --- | --- | --- | --- |
| The pop's own day | every pop | what to do next, with whom, where; what to believe (accept or doubt a retelling) | the body (needs), the ties, the schedule's anchors | spec 24 (pop AI), spec 39 (beliefs); the day from surveys is spec 54, next |
| Head of household | one adult per household | the household's rations, who works, who stays, where the household lives, what it buys | money (spec 32), the members' willingness | spec 51 gives the household and its food; the budget as a policy record is lacking |

### 2.2 Group (the survivors, the commander layer)

| Station | Holder | Levers | Cost, delay | Today |
| --- | --- | --- | --- | --- |
| Leader | one pop of the group (the player, or the AI pop the group follows) | rations, watches, who scavenges where, where the group goes, who may join, pause on alerts | morale, obedience; a broken order is noticed ([0070](../decisions/0070-broken-commitments-are-noticed.md)) | the run's group exists (spec 26) with the player's orders as commands; group rules as records are lacking; military orders (pinned question 3) are this station's levers |
| The radio | whoever holds the set | what to broadcast, when to listen | fuel or batteries (spec 17), the hops and distortion of retelling (spec 39) | spec 26 has the radio as an event; broadcast as a belief source is spec 39 rule 5 |
| The generator | whoever tends it | the fuel ration, what is powered | fuel (spec 17), cold rooms and dark (specs 18, 19) | spec 17 built; the ration as a parameter is lacking |

### 2.3 Building and establishment

| Station | Holder | Levers | Cost, delay | Today |
| --- | --- | --- | --- | --- |
| Owner or manager | the establishment's owner (spec 33: staff is the workplace roster) | staffing (hire, lay off), hours, prices, what recipe to run, who may enter | wages (spec 32), stock, the roster's willingness (rosters, spec 28) | spec 33 runs the recipe with the roster; **Staffing** is a built edict kind (spec 39); hours and prices as policies are lacking |
| Contractor | a building firm's manager | which stage to build next, whether to stall | the project's escrow and materials (spec 34) | spec 34 built: stalls say why; choosing is lacking |
| School head, clinic head, station chief | the institution's head (0069 rule 3: queues with staff) | hours, priorities, how many on duty | the staff's day (spec 54), the queue's length | IQ gives the queues' numbers (IQ-8 priorities, IQ-13 response, IQ-31 waits); the queue mechanic is the next research build after spec 54 |

### 2.4 Town and district

| Station | Holder | Levers | Cost, delay | Today |
| --- | --- | --- | --- | --- |
| Mayor's office | one pop; a council in larger towns | local rationing, a curfew, a road closed, a shelter opened, a market day, the water schedule | trust (macro §6.3), the town's money, a day for patrols to follow | lacking; the town exists as spec 47 and 48 records, the curfew would be a parameter the schedules (spec 54) read |
| Police chief | one per department; SV-6: half of US departments have under 10 officers, SV-9: 3 sworn per 2,500 people | patrol hours, how calls are prioritised, how many on each shift | officers' time (IQ-17: 40-45 % unaccounted), response by priority (IQ-13: P1 4 min, P4 78 min median) | the police queue is the next research build (IQ); the chief's levers are its parameters |
| Fire chief | one; SV-29: under 10,000 people all volunteer | who is on call, which calls go first | volunteers' days (SV-27: 62 % are volunteers) | lacking; IQ-39 gives the response standard |
| Clinic or hospital head | one; SV-22, SV-23: doctors and beds per 1,000 by income group | who is admitted, when to divert, hours | beds (SV-23), the staff's day | lacking; IQ-31 to IQ-37 give waits, stays, triage and diversion |
| School head | one; SV: thresholds by size | the school's days and hours | the calendar (spec 53: school days built) | spec 53 gives the school year; the head's levers are lacking |
| Utility operator | one per grid node | load shedding, repair order | the grid tree (spec 35) | spec 35 built: shedding above is unserved load below; choosing who to shed is lacking |

### 2.5 Province, region, sector

| Station | Holder | Levers | Cost, delay | Today |
| --- | --- | --- | --- | --- |
| Governor | one per region (spec 48: sectors cut inside countries) | the region's levy and staffing (the built edict kinds), where aid goes, which towns get the grid first | the region's money and stock (spec 32), trust | **Levy** and **Staffing** are built edict kinds on a region (spec 39); the rest is lacking |
| Regional services | the heads of the region's hospital, court, police | catchments: which towns a service covers | the staff's day; SV-33, SV-34: thresholds are regional | lacking; SV gives thresholds and catchments |

### 2.6 Country

| Station | Holder | Levers | Cost, delay | Today |
| --- | --- | --- | --- | --- |
| Head of state | one; the government type (macro §6.3) says who may hold it and how the holder changes | which offices exist and who holds them, the state of emergency, the AI priorities (protect the economy, lives, the regime, neighbours) | legitimacy, trust (macro §6.3), the delay of every office below | lacking; government type is a setup field (macro §6.3) |
| Treasury | one | tax rates, the levy on goods, the budget by office, money minted | money as a ledger resource (spec 32): purchases, wages and taxes exist | **Tax** and **Levy** are built edict kinds (spec 39); the budget is lacking |
| Interior | one | curfew, lockdown, borders closed, patrols' priorities, conscription | state capacity and compliance (macro §6.3: response lag and compliance), the police queue | lacking; the outbreak's warning as a belief the outbreak writes (TC, research) is the first lever here |
| Health | one | beds opened, triage rules, cure research spend, vaccination order | beds and doctors (SV-22, SV-23), money | spec 38 gives the outbreak's rates; the cure is macro §6.4, lacking |
| Communications | one | what is broadcast, how often, to whom | the communications the broadcast needs (spec 39 rule 5), the distortion of retelling | spec 39 built: broadcasts need communications and retelling distorts |
| Defence | one | mobilisation, where forces go, rules of engagement | the levy of people (conscription as a levy), supply (spec 32), morale | lacking; war as a Project is sandbox parameters §2 (Open) |
| Calendar authority | one | public holidays declared, the school year, the official rest days | the calendar (spec 53) | spec 53 built from data; declaring a holiday as a policy is lacking |

### 2.7 World

No station. The planet, the start date, the outbreak's rules and sizes are setup only ([0075](../decisions/0075-parameters-change-through-stations-setup-locked.md) rule 3). A world assembly (macro §6.2) is a station of the countries that join it, not of the world.

## 3. What the table says

- **Three levers are built**: levy, tax and staffing (spec 39), each already a station's lever (treasury, governor, owner). Every other row is a parameter whose record does not exist yet, or a queue whose numbers are in research and whose mechanic is next.
- **The count is small.** About twenty-five stations across six scopes cover what the research describes, and most of them are one pop each at a town's size. A country of a million people has a few hundred holders; a group has one leader. That is the honest size of the commander layer and of the AI's levers.
- **The order of building follows status, not this table.** Spec 54 (schedules) makes holding a job a thing that happens at hours; the police queue (IQ) is the first institution with a chief; the services by size (SV) say which towns have which stations. Each of those gives two or three rows above their records. The rows gain a station column in the sandbox catalogue as they are built.
- **The AI's levers are the same rows.** An AI head of state with "protect the regime" as a priority (macro §6.3) and a belief that the outbreak is small (spec 39) keeps the borders open; one with "protect lives" and a belief that it is large closes them. Both act through Interior's lever, both pay in trust, both trace. That is the closed loop the user described: a pop acting by its own parameters, and its act becoming a parameter others read.

## Open

- The record shape for a lever beyond the three edict kinds: one `Policy` record (scope, station, parameter, value, signed-by, valid-until) that the Atlas's cards can show and edit when a station is held, or a kind per lever as the edicts are today. Recommendation: one record, because the cards, the chronicle and the trace all read it the same way.
- How a holder is appointed and removed at each scope (elections, succession, a coup, a vacancy) is the head of state's lever set and is not drawn here; it waits for government type to be more than a setup field.
- Which stations the player may hold in play, and when (0066: stations gate deciding; 0062: possession), is the game-loop question of whether a player who possesses a mayor holds the mayor's office. Default: yes, while possessed, with the pop's own standing as the cost.
