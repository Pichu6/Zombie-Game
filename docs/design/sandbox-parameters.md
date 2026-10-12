# Sandbox parameters

The catalogue of everything the player can set before (and, where allowed, during) a run: the session, the world, nations, cultures, pops, households and groups, businesses, corporations, government and politics, the economy, infrastructure, and the outbreak. It also says how the game stays a full game **without an outbreak**: a politics, grand-strategy and Sims-like life simulation on the same world ([0027](../decisions/0027-no-outbreak-game-and-parameter-catalogue.md)).

This doc is the source of truth for **what is configurable and under which rules**. How each system works stays in its own doc: [world simulation](world-simulation.md), [macro layer](macro-layer.md), [inhabitants](inhabitants.md), [infrastructure](infrastructure.md), [threats](threats.md), [content tools](content-tools.md).

Labels: **Decided** (the user confirmed it), **Proposal** (not confirmed), **Open** (needs an answer). Every default, band and field name below is a **Proposal** unless marked; values marked *calibrate* are filled from public data when the system is specified, not invented here.

## 1. How parameters work

### 1.1 The rule that keeps the game honest

**Settings change numbers, never rules** (Decided, [vision](../vision.md)). A parameter is a rate, a size, a share, a limit or a starting state that a rule already reads. A parameter can never add a verb for the player that pops don't have, or switch off conservation (people, goods, money and fuel move between owners; nothing is created from nothing).

### 1.2 Four kinds of parameter (Proposal)

| Kind | When it is set | Can change during the run? | Example |
| --- | --- | --- | --- |
| **Setup** | World, Nations, Outbreak, Scenario and Character screens | No. Locked at the start, so nothing is created or destroyed mid-run | Number of nations, climate, starting population, which government a nation begins with |
| **Sandbox rule** | The same screens, plus a settings panel | Yes, tagged "sandbox change" in the save and the chronicle | Time scale, disaster frequency, AI aggression, zombie speed |
| **Policy** | In play, through an actor's offices | Yes, by the actor that owns it, with real cost | Tax rate, a trade ban, a wage floor, a company's hiring plan |
| **Derived** | Never set | Computed from the simulation | GDP, unemployment, trust, collapse pressure |

Policies are the politics game: the same record shape for a player, an AI government, a company board and a household budget. A setup parameter is only the **starting value** of something that later becomes a policy or a derived number (a nation's starting tax rate is setup; changing it in year 3 is a policy).

### 1.3 Three ways to fill every field (Decided, [0022](../decisions/0022-macro-layer-intent.md))

1. **Auto:** the generator fills it from the world fields and the other choices.
2. **Preset:** a saved design for one thing (a culture, a government, a company, a nation) or a whole world.
3. **Custom:** edit one by one. Any edit can be saved as a preset and shared as a code with its seed.

Fields cascade: a world preset gives nation presets, which give culture and company presets. Editing a parent re-fills only the fields you have not touched (a touched field shows a dot).

### 1.4 One validator (Decided as intent, [world simulation](world-simulation.md) §2 rule 13)

Every parameter has three bands: **realistic** (calibrated on real data), **sandbox** (allowed, tagged as realism-breaking), **rejected** (would break an invariant, such as shares over 100% or a negative population). The same validator checks a preset, a scenario and a slider. Cross-field rules apply too: a nation's industry shares sum to 1; a biome cannot contradict its climate; a company cannot own more shares than exist.

### 1.5 Where it lives (Proposal)

Data, not code, like the content packs ([content tools](content-tools.md) §1 rule 3): TOML presets under `data/setup/` (`world/`, `nation/`, `culture/`, `government/`, `corporation/`, `scenario/`), each with an `id`, `version`, a `[meta]` block (name, author, realism tag) and the fields below. The setup screens, `zs`, the sandbox window and tests read the same files. A run records the final resolved settings and the seed in its save, so a preset update never changes an old save. First built for pop lives: `data/setup/demography/default.toml`, copied into each world's save ([spec 28](../tier0/specs/28-pop-levels.md), "Built to be changed").

### 1.6 Who may change a parameter in play: stations (Decided, [0075](../decisions/0075-parameters-change-through-stations-setup-locked.md))

Setup is locked at creation; a change makes another world ([0074](../decisions/0074-worlds-create-save-edit-fork-delete.md)). In play, a parameter changes through a **station**: a job or office of the simulation that exposes a few parameters of its scope (a presidency, a ministry, a mayor's office, a board, a leader, a radio, a generator), set by whoever holds it, player or AI, at a real cost and with a real delay, through the same records ([spec 39](../tier0/specs/39-orders-and-belief.md)'s edicts and their successors). No station changes the world; a country's parameters change through its government's stations, a town's through its own, a building's through its owner, a group's through its leader, a pop's own through the pop. The sandbox panel stays as the free, immediate, tagged path. Each row of the catalogue below gains a **station** column as its system is built: which station sets it, its cost, its delay; rows with none are setup or sandbox only. The scope table is in the decision; the stations themselves, scope by scope with their holders, levers, costs and the spec that produces each, are in [stations](stations.md). **Collapse is derived, never declared** ([0076](../decisions/0076-collapse-is-derived-never-declared.md)): no option sets it; a lever delivers only through the pops below it; the share of stations held, the queue lengths, the compliance of each chain and trust are the derived readouts of a society failing or recovering, and the no-outbreak game (§2) runs the same loop under other pressures.

## 2. The outbreak is a module

With **no outbreak** (zero first cases, Decided in [0018](../decisions/0018-sandbox-answers.md)), the zombie and infection rules never fire and the census never moves to infected or risen. Nothing else changes: the same bodies, the same economy, the same rules. That is only a game if the rest of the world carries it, so these must hold when the outbreak is off (all Proposals, none built yet):

| Needed for a no-outbreak game | Why | Today |
| --- | --- | --- |
| **Ordinary life and death:** ageing, ordinary illness, accidents, crime, births, marriage, migration | A world with no outbreak must not be static, nor empty out | Body, illness and wounds exist; births and ageing are not modelled yet (Decided as part of the simulation, [0028](../decisions/0028-births-military-mystery-tech-leaders.md); model in §7.4) |
| **Long time scale:** months and years per real minute at Macro, with Individual dormant | A politics game cannot run at 1 real hour per game day | 1× and 3×/10× only ([vision](../vision.md)); the detail levels exist as a design |
| **Standing goals:** the world gives pops and actors things to want without a threat | Sims aspirations, company growth, political ambition | Needs and jobs exist; ambitions and org goals do not |
| **Pressure that is not zombies:** recession, scandal, disaster, shortage, unrest, rivalry | The brake that keeps a sandbox from going flat | Only the outbreak provides pressure today |
| **Optional goals:** scenario objectives instead of "survive 14 days" | A run needs a way to feel finished, or none at all | Run goals exist for Tier 0 only |

Everything in §4–§11 is written so it works with the outbreak off. The outbreak then adds its own group (§12) on top.

## 3. Session parameters

| Parameter | Values | Default | Notes |
| --- | --- | --- | --- |
| Seed | integer or text | random, shown | Same seed and settings give the same world (determinism, [spec 02](../tier0/specs/02-draw.md)) |
| Play style | micro → macro, macro → micro, one layer only, spectator | micro → macro | Decided ([0016](../decisions/0016-sandbox-setup-and-layer-flow.md)) |
| Starting role | civilian, a chair in an office (mayor, minister, CEO, board member), a household head, spectator | civilian | In micro → macro the role limits orders; other styles may take any chair (Decided, [0018](../decisions/0018-sandbox-answers.md)) |
| Starting country | any country on the generated map, chosen from its card (food, water, energy, minerals, coast, neighbours, difficulty), or a start type the generator finds a region for: river valley, mineral rich, arid and landlocked, island, between giants, rich and coveted, forest frontier | chosen on the map | The difficulty is the land itself, never a modifier ([0059](../decisions/0059-countries-from-water-land-and-resources-everything-alive-can-be-sick.md)) |
| Time scale | **Pause**, **slow down** (½×, ¼×), **1:1 real time (the reference)**, 1 hour = 1 day (the usual speed, 24×), 3×, 10×, rest-until-event, and **long steps** (day, week, month, year per second) | start at 1 hour = 1 day (Decided, [0050](../decisions/0050-start-speed-and-time-while-closed.md)) | Real time is the reference and real Earth data the default for every value; the usual play speed is 1 hour = 1 day ([0032](../decisions/0032-real-time-reference-and-generational-runs.md)). Pause, acceleration and slow-down are Decided ([0031](../decisions/0031-restoring-capabilities-real-time-playable-age.md)); the exact steps and long steps are Proposals. Long steps only at Macro/City, with Individual dormant. Combat forces the usual speed or slower, never faster, and allows pause. Events never wait for the player ([0022](../decisions/0022-macro-layer-intent.md)), so in a future multiplayer game pause needs its own rule |
| Real-time durations | Every action and process takes its real-life duration in game time (cooking, healing, building, travel, research, pregnancy, growing up) | on, always | Decided as the standard ([0031](../decisions/0031-restoring-capabilities-real-time-playable-age.md)). Only the clock speed changes, never the duration. A "faster processes" multiplier is a sandbox change, tagged |
| Playable age | Youngest age a pop can be possessed or chosen as a character | 16 (Proposal; sandbox 12–25) | Decided that a minimum exists. Younger pops age as records and can be followed, never played |
| Auto-pause classes | per event type: ignore, log, toast, pop-up, pop-up and pause | toast/pop-up, no auto-pause | [macro layer](macro-layer.md) §4.2 |
| Fog | on (what the character believes) or off (truth) | on in micro → macro | Decided |
| Instant build / god tools | off, sandbox only | off | Decided ([0014](../decisions/0014-open-questions-answered.md)); tagged |
| Permadeath of the played pop | on, or switch to a relative / next in office | on | Control passes on death, as in the run ([spec 26](../tier0/specs/26-run.md)) |
| Realism tag | realistic, sandbox (set automatically from the bands) | realistic | Shown on the save and on shared presets |

## 4. World parameters

The laws and realistic bands behind these world parameters (capacity, settlement, nations) are in [world numbers](world-numbers.md).

Intent Decided ([0016](../decisions/0016-sandbox-setup-and-layer-flow.md)); model [world simulation](world-simulation.md) §12.

| Group | Parameters | Notes |
| --- | --- | --- |
| **Source** | Real Earth (default) or generated; size and land share (generated) | Real Earth only accepts sliders that don't contradict the data (population, resources, wildlife) |
| **Physical causes** | Temperature, seasonality, moisture, relief, fertility, resources, vegetation, wildlife | Sliders; biome is painted, never a slider |
| **Population** | Total; urbanisation (share in cities); city size spread (rank-size bend); cities per nation or auto | [macro layer](macro-layer.md) §6.2 |
| **Nations** | Number (generated worlds); size spread; how borders follow geography (rivers, ranges) | Real Earth uses real nations, with invented leaders ([macro layer](macro-layer.md) §4.4); political fragmentation (a few empires to many small states) inside what the geography allows ([0059](../decisions/0059-countries-from-water-land-and-resources-everything-alive-can-be-sick.md)) |
| **Cultures and religions** | How many; one per nation or spread across borders; how mixed | Data packages, §6 |
| **Languages** | Count and mutual intelligibility | Proposal; affects trade, trust and rumour spread. *Open whether needed for v1* |
| **Resources and trade geography** | Deposit richness per resource; chokepoints; sea routes vs land routes | Feeds §11 |
| **Disasters** | Frequency and severity of floods, droughts, quakes, storms, fires, plus *crop failure* | Detected-not-injected rule: they happen from the world's own state (weather, tectonics), at the set rate. Default realistic; "none" is allowed |
| **Institutions** | A world assembly exists or not; how often it meets; how binding its resolutions | [macro layer](macro-layer.md) §4.4 |
| **Pace of change** | Technology and innovation rate; urban growth rate | Proposal; technology in §11.1 |

## 5. Nation parameters

A nation is an Actor that owns regions, a budget, offices (each held by a real census record), laws and relations ([macro layer](macro-layer.md) §7). Each field has realistic, sandbox and rejected bands. The **country creation screen** (people, values, government, economy, services, security, origin, in Stellaris order) and the government types it offers (including theocracy and corporate state) are in [macro layer](macro-layer.md) §6.3 and §6.5 ([0026](../decisions/0026-country-creation-inheritance.md)); this catalogue lists the parameters and their bands, and the profile data a nation carries ([§5.2](#52-demographic-profile-decided-as-intent-0034-model-proposal)).

| Group | Parameters (setup values; most become policy or derived later) | Drives |
| --- | --- | --- |
| **Identity** | Name, flag, colour, capital | Display |
| **Territory and people** | Regions and sectors, population, age pyramid, urbanisation, culture and religion shares, ethnic or regional minorities | The census the generator makes |
| **Government** | Type (democracy, parliamentary, presidential, single party, military junta, monarchy, theocracy, collapsed); head of state and of government; cabinet seats; term lengths and limits; electorate and franchise; voting system; separation of powers (courts, legislature, press freedom) | Which edicts need which approval, election cycle, how fast orders pass, use-of-force limits |
| **Parties and factions** | Number, size, positions on the ideology axes (§8), polarisation, how parties form and merge | Votes, coalitions, protest |
| **Capacity and trust** | State capacity (how well orders are carried out), legitimacy, corruption, public trust, rule of law, bureaucracy size | Response lag, compliance, leakage of money |
| **Economy** | Wealth (GDP per head), sector mix, inequality (Gini), unemployment, informal share, currency and its stability, debt, reserves | §11 |
| **Fiscal** | Tax rates by kind (income, corporate, sales, property, tariff), spending shares (health, education, security, welfare, infrastructure, debt service), budget rule | Policies the actor can change |
| **Services** | Health capacity (beds and doctors per 1,000), education, police and courts, welfare coverage, pensions | [world simulation](world-simulation.md) §5 |
| **Infrastructure** | Roads, rail, ports, airports, power grid, water, phones and internet | §11, [infrastructure](infrastructure.md) |
| **Security** | Police size, army size, intelligence, border control, crime rate | Orders that need guards exist only through guards |
| **Diplomacy** | Starting standing with each other nation, treaties (trade, open borders, defence), alliances, rivals | Standing with reasons ([macro layer](macro-layer.md) §7) |
| **AI priorities** | Weights: protect lives, protect the economy, protect the regime, help neighbours, expand, status quo | What the government picks when you are not watching; same menu as the player |
| **Politics play** | Elections, coups, revolutions, secession, reform: on or off, and how likely | Sandbox rules; never forced on a date |

### 5.2 Demographic profile (Decided as intent, [0034](../decisions/0034-nation-demographics.md); model Proposal)

Every nation has its own demographics, as real countries do. The profile is data, `data/setup/nations/<id>.toml`, and it is what the generators and the realism report read, so a country is not one US-shaped town copied.

| Table | What it holds |
| --- | --- |
| Age pyramid | Shares by five-year band (0-4 to 95+); founders drawn by rule take this age structure |
| Education | Attainment of adults (none, primary, secondary, tertiary) for ages 25-34, 35-64 and 65 and over |
| Labour | Participation by age group and sex, unemployment rate, retirement age |
| Occupations and sectors | Shares of the employed by ISCO-08 major group and by agriculture, industry, services |
| Households | Shares of households of 1, 2, 3, 4, 5 or more people |
| Settlement | Urban share |
| Mortality and fertility | The demography preset it names (`data/setup/demography/`) |

Each table carries its source and year. A profile is **placeholder** (a plausible shape, not data) or **sourced**; the loader refuses "sourced" without a citation on every table. The profile is copied into a world's save when the world is made, like the other presets, so updating it never changes an old save. Next: the town generator draws education, occupation and households from the nation's profile (today it uses one US-like job table), and the realism report ([§13.1](#131-realism-checks-for-long-runs-decided-as-a-use-0032-model-proposal)) compares a run with the same profile's real data.

### 5.1 Military power (Decided as intent, [0028](../decisions/0028-births-military-mystery-tech-leaders.md); model Proposal)

Military power counts in the simulation whether or not anyone plays combat. Combat gameplay is wanted in some form and at some later time; it will read and write the same records.

- **It is made of real things:** armed forces are groups of pops (§8) with ranks, training and loyalty, holding items (weapons, vehicles, ammunition, fuel) from the ledger. A nation's "power" is a **derived** number read from those records, never a setting. Nothing is created: a unit that loses its fuel stops, a unit that is not paid may desert.
- **Parameters** (setup values, then policies): size of regular forces, reserves and militia, share of budget, conscription rule, training level, equipment stock and quality (Q), logistics capacity, doctrine (defensive, expeditionary, internal security), command structure and who holds each office, loyalty to the government, readiness.
- **Uses without combat gameplay:** deterrence and standing between nations, border control, policing and cordons (an order exists only through the guards posted, [world simulation](world-simulation.md) §13), coups and civil unrest, disaster response, and war.
- **Reference to study:** Foxhole (persistent war where logistics is the war: supply chains, facilities, frontline, regiments). See [research/foxhole.md](../research/foxhole.md). Not a decision.
- **War between nations** is in scope as a simulated process: a campaign is a Project with forces, supply and objectives, resolved from the real numbers of both sides (strength, supply, terrain, morale, leadership) with the same draws and conservation as everything else; casualties are real pops, destroyed items leave the ledger. Resolved in aggregate when nobody is there; when a body is there (a possessed soldier, a combat scene), the same fight is played at body level and the result carries back. Which scale of war and what the detail looks like is *Open* (§15 open question 2).
- **Same rule as all else:** settings change numbers, never rules, so an AI army and the player's obey the same logistics.

## 6. Cultures and religions

A culture is a data package ([world simulation](world-simulation.md) §12 culture kits; `data/gen/cultures/` already holds six name and look packs). For the sandbox it gains the **behaviour** half:

| Field | Drives |
| --- | --- |
| Names, looks, food, dress, building style | Already in the generator packs |
| Household size and structure (nuclear, extended, single), age at marriage, family size | Pop generation, housing demand |
| Trust in authority, rule compliance threshold, risk tolerance, collectivism vs individualism | Compliance with edicts (persistent threshold per pop, [world simulation](world-simulation.md) §5), protest |
| Work norms (hours, retirement age, women's and youth work), entrepreneurship, saving rate, consumption tastes | Economy, business formation |
| Religion: presence, strictness, calendar (holidays), influence on law and marriage | Agenda items, voting blocs |
| Attitudes: to outsiders, to hierarchy, to corruption, to technology | Migration, politics, adoption |
| Language and mutual intelligibility | Trade and rumour spread (if kept) |

Invented religions and cultures only where a real one would be a joke or an insult (Proposal, from the DLC rule in [world simulation](world-simulation.md) §4).

## 7. Pop parameters

A pop is one person with a body, know-how, ties and a life ([inhabitants](inhabitants.md)). The Sims half of the game is this section: needs, wants, a life and a history. Two parts: how pops are **made**, and how they **live**.

### 7.1 Generation (distributions per nation, region and culture)

| Group | Parameters |
| --- | --- |
| Demography | Age pyramid, sex ratio, household structure, fertility, life expectancy, disability and chronic illness rates |
| Body | Real measurement data by age, sex, culture and wealth (existing body generator, [spec 11](../tier0/specs/11-body-composition.md)) |
| Education and skills | Education levels, skill mix by job, literacy; a custom character takes a matching census slot |
| Work | Employment shares by job ([content tools](content-tools.md) §4 and `data/gen/jobs.toml`), unemployment, self-employment, retirement age |
| Wealth | Income and savings distribution, debt, home ownership, inheritance |
| Personality and politics | Trait distribution; position on the ideology axes (§8); party attachment; trust; religion |
| Lifepath | The TTRPG-style past sampled backwards, with real rates ([world simulation](world-simulation.md) §3); a **term count and detail level** setting for how deep it goes |

### 7.2 Living (rates that act every day)

| Group | Parameters |
| --- | --- |
| Needs | Hunger, thirst, sleep, warmth, pain, from the body; plus social, hygiene, fun, purpose as needs with their own decay rates (Sims) |
| Wants and aspirations | A few life goals per pop (career, family, wealth, status, craft, faith, power), rate at which they form, change and complete |
| Mood | How needs, events and relationships add up; thresholds for refusal, quitting, protest, crime, breakdown |
| Life events | Marriage, divorce, children, illness, injury, job loss and hire, move, education, retirement, death: **rates by age and culture**, scaled by one "life event rate" multiplier |
| Relationships | Sparse bond graph ([inhabitants](inhabitants.md)); chance of new bonds, trust decay, how fast rumours mutate |
| Crime and conflict | Base crime rate by need and opportunity; how far pops tolerate unfairness before acting |
| Migration | Willingness to move, distance, push and pull weights (jobs, safety, family, services) |
| Learning | Practice and novelty rules ([inhabitants](inhabitants.md)); a "learning speed" multiplier (sandbox) |
| Detail | How many pops are embodied near the player vs records vs lazy census ([world simulation](world-simulation.md) §3) |

### 7.3 Pop presets and imports (Proposal)

Each pop exists once per simulation: no respawn, only birth and death. A saved pop is therefore a **template**, not a person.

- **Template (default):** loading "Maria, nurse, 34" into a new world makes her **once**, at world creation, in a matching census slot (age, region, job). The generator then builds a consistent household, past and ties around her (shared events are generated once so both sides remember them, [world simulation](world-simulation.md) §3). The pop in the old save is untouched.
- **Import (optional):** copies a pop from another run **as of a date**: new id, same body, skills and traits, no old ties, items only if the scenario allows. It is a copy in a different world, so nothing is duplicated within one simulation.
- **Same for leaders:** a saved president or general can be placed as a candidate for an office; the office's succession rule still decides.
- A world can never hold two copies of one template; the validator rejects it.

### 7.4 Births, ageing, leaders and researchers (Decided as intent, [0028](../decisions/0028-births-military-mystery-tech-leaders.md); model Proposal)

Births, ageing and death run in every game, outbreak or not. This is what gives the Stellaris kind of play: you appoint leaders and researchers, they have careers, they age, and they die.

- **Births:** a birth is a conserved event that creates a new record from two parents' census slots (or one), counted in the ledger as population in; it is not a respawn. **Inheritance** follows [0026](../decisions/0026-country-creation-inheritance.md): the body mixes both parents' genes (looks, height potential, some health risks); culture, language, religion and political leaning start as the family's and change later; ancestry gives no bonuses. The pop-level model built so far ([spec 28](../tier0/specs/28-pop-levels.md)) records the mother only; fathers, households and inherited culture are not modelled yet. The newborn gets a body from the human generator (body, genes, family resemblance, culture), a household, and then grows through real ages. Rates follow fertility by age and culture, health, wealth, housing, policy and war.
- **Ageing and death:** every body ages with real rates by age and sex (the body model already has weekly drift). Death by age, illness, accident, violence or outbreak is one event with a cause, and it removes the person from every office and group they held.
- **Offices are held by real people.** A president, minister, general, CEO, board member, party leader, lab director or chief researcher is a census record in a role slot ([macro layer](macro-layer.md) §3.2). A leader is an ordinary pop with a long record; they do not turn into a special object.
- **Succession:** each office has a rule (appointed, elected, hereditary, seniority, strongest, council) set per nation or group (§8). When the holder dies, resigns or is removed, the rule picks a successor from real candidates; no candidate means the office is empty and its work stalls. Succession crises are events the world detects, not scripts.
- **Careers:** pops move up through jobs by skill, experience, connections and age, so a future leader exists years before the post. Pops past retirement may keep advisory seats.
- **Researchers:** a researcher is a pop with know-how in a field, employed at a lab or school. Their output depends on their skill, age, tools and the lab's power, staff and funds (production-limited by the scarcest input, [world simulation](world-simulation.md) §5). Research is slow and can be lost with their deaths and notes ([inhabitants](inhabitants.md): "if the only virologist dies, no replacement appears; her notes remain").
- **Select and play (Decided as intent):** any officeholder is a selectable character in the higher layers (a president you like in Layer 3, a general, a CEO) and you can possess them to play them in Layer 1. Possession gives control, not power ([vision](../vision.md)): they still act through their office, with its limits, and the AI replans when you release them. Following a person across layers needs the same record at every zoom level, which is the aggregation problem in §15. Play style macro → micro (start in an office, drop into a body) is unchanged and stays Decided ([0016](../decisions/0016-sandbox-setup-and-layer-flow.md)).

## 8. Households and groups

One record shape for every group of people, with different defaults per kind. Operational group, faction and role are already defined in [inhabitants](inhabitants.md) (Proposal).

| Kind | Examples | Specific parameters |
| --- | --- | --- |
| **Household** | Family, flatmates, one person | Size, structure, shared budget, who decides, housing |
| **Informal** | Friend circle, neighbours, congregation | Cohesion, size cap, meeting rhythm, shared norms |
| **Workplace team** | Shift, crew, department | Lead, schedule, skills needed |
| **Organisation** | Union, party, NGO, guild, club, gang, cult, militia | Aim, membership rule, dues, discipline, secrecy, rivals |
| **Operational group** | Survivors who travel and work together | Shared orders, shared stores ([inhabitants](inhabitants.md)) |
| **Government body** | Cabinet, council, assembly, court | Seats, voting rule, quorum, term |

Common fields for every kind:

| Field | Values |
| --- | --- |
| Size and cap | Min, max, growth rule |
| Cohesion and loyalty | Starting level, decay, what raises it (shared work, shared danger, shared meals) |
| Leadership model | Appointed, elected, hereditary, strongest, consensus, none; succession rule (the Open item in [inhabitants](inhabitants.md)) |
| Membership | Open, invite, dues, oath; how to leave and what it costs |
| Resources | Money, goods and buildings the group owns (they live in the ledger like anyone's) |
| Goals and ideology | Position on the ideology axes; what it wants; how it chooses actions |
| Relations | Standing with other groups, with reasons |
| Authority | What it can order and of whom: nothing without legitimacy or a role |

**Ideology axes** (Proposal): a small fixed set shared by pops, parties, groups and governments so that positions can be compared: economic (state ↔ market), social (open ↔ traditional), authority (liberty ↔ order), outlook (local ↔ global), and religion (secular ↔ devout). Five numbers each; the set is a Proposal and can change before it is built.

## 9. Businesses

A business is a **site-level enterprise**: one shop, farm, workshop, clinic or plant that employs pops, holds stock and produces or sells. Business types already exist as data for town generation (`data/gen/businesses.toml`, `data/gen/jobs.toml`); the sandbox gives them an economy.

| Group | Parameters |
| --- | --- |
| **Type** | Sector and subtype; building and equipment it needs (real assemblies, so a stripped machine stops output) |
| **Size** | Jobs by role, floor area, capacity, number of sites |
| **Production** | Inputs per unit, output per worker-hour, quality (Q) range tied to machinery and skill ([0013](../decisions/0013-production-ladder.md)), minimum crew, power and water need |
| **Money** | Starting cash, debt, margin target, prices (posted and sticky, shops run out rather than raise prices, [world simulation](world-simulation.md) §5), wages (policy, subject to the nation's wage floor) |
| **Staff policy** | Hiring and firing thresholds, shifts, training, wage vs staffing priority, treatment of sick staff |
| **Owner** | A pop (family business), a corporation, a government, a cooperative |
| **Behaviour** | Expansion appetite, risk, how it reacts to shortage, strikes, unpaid wages, closure rule |
| **Rules it obeys** | Licences, safety, tax, opening hours (set by the nation's laws) |
| **Formation and failure** | Rate at which pops start businesses (culture and wealth), failure rate, how stock and building pass on when it closes (nothing vanishes: stock and parts return to the ledger) |

Business density per town and business-type counts stay in the town generator's data; the sandbox slider scales them (more shops per head, fewer chains).

## 10. Corporations

A corporation is an **owner and strategist** over many businesses: it holds shares in them, raises and spends money, and decides where to expand. It is an Actor like a nation, with the same edict and ledger rules, so it can be played from a chair (CEO, board member) or run by AI.

| Group | Parameters |
| --- | --- |
| **Identity** | Name, sector focus, home nation, founding story (a generated history; a family firm, a state spin-off, a startup, a conglomerate) |
| **Structure** | Subsidiaries and businesses owned, ownership of those (percent), headquarters, number of employees |
| **Ownership** | Shareholders (pops, other corporations, governments, a family), share count, voting rights, board seats and how the board is chosen |
| **Money** | Capital, debt and credit terms, dividend policy, reserves, how it raises money (loans, shares, retained profit) |
| **Strategy** | Weights: growth, profit, market share, stability, staff welfare, prestige, political influence; appetite for takeovers and mergers; R&D share |
| **Market** | Products and prices, market share, supply contracts, how it responds to a rival |
| **Labour** | Wage policy, union relations, outsourcing, layoff threshold, how it behaves in a crisis |
| **Ethics and compliance** | Compliance with the law, corruption propensity, safety culture, environmental record; each has consequences only through events the world can detect |
| **Political reach** | Donations, lobbying budget, ties to officials. It works only through real people: an official takes a gift or does not, by that pop's own rules |
| **Limits** | Antitrust (market-share cap, merger review) and ownership caps are laws, not hard-coded, set by the nation |

Corporations are **a few hundred actors, deciding about once a game day**, the same cost class as nations ([macro layer](macro-layer.md) §7). Their businesses and people are lazy until touched.

## 11. Economy and infrastructure parameters

Rules in [world simulation](world-simulation.md) §5 (stock and flow per region, production limited by the scarcest of workers, inputs and power) and [infrastructure](infrastructure.md).

| Group | Parameters |
| --- | --- |
| **Resources** | Deposits and their richness; renewal rate of renewables; depletion is permanent |
| **Production** | Productivity growth, input shares, machinery ceilings per method |
| **Trade** | Tariffs, transport cost per distance, border friction, sea vs land routes |
| **Money** | Currency, how stable, how banks work, inflation and interest rule, stages of failure (card, cash, barter) |
| **Labour market** | Wage level, minimum wage, unemployment support, mobility, union strength |
| **Housing and land** | Supply, rent level, ownership rate, zoning rules |
| **Networks** | Power, water, roads, rail, radio, phone: starting capacity, backup clocks, maintenance staffing, failure rates |
| **Environment** | Pollution and its effects, resource stress, climate drift (rate, off by default) |

### 11.1 Technology (Decided as intent, shape tbd, [0028](../decisions/0028-births-military-mystery-tech-leaders.md))

A Stellaris-style tech tree is wanted, and (Decided, 2026-10-06) **a nation's tree reflects the items, establishments, cities and buildings it has in the lower layers (2 and 1)**. It must fit the rest: knowledge is held by people, and quality comes from parts, skill and machinery ([0013](../decisions/0013-production-ladder.md)). Proposal for how:

- **The tree is a readout of what exists:** a node is a capability (recipes, the establishments that run it, the people who know it). A nation "has" a node when working establishments and trained staff exist; a plant destroyed or a school emptied lowers it. Layer 3 shows it as a tree; Layer 2 shows the buildings behind each node; Layer 1 shows the machines and the people.
- **It also pulls the other way:** a research project, a funded school or an imported plant adds real establishments and people, which raises the node. Nothing is a list of unlocks that ignores the world.

- **A technology unlocks, it doesn't hand out power:** it adds recipes, building parts, procedures and know-how to what researchers and workers can learn, and lifts ceilings (for example the quality a method can reach). It adds no verb the player has and pops don't.
- **Research is a Project** at labs and schools, done by researchers (§7.4), paid for in money, staff, power and samples, with a research rate and cost curve as parameters. A fallen government or a blackout stalls it.
- **Spreads by people and trade:** knowledge moves with books, teachers, trade and espionage; losing the people, the notes or the establishments loses it locally (Decided: lost, not forgotten by drift, [0030](../decisions/0030-pop-continuity-and-capability-loss.md)).
- **Parameters:** research speed, cost growth, number of fields, branch breadth, randomness of what is offered (Stellaris offers a few cards), whether the tree is fixed or generated per world, whether other nations' progress is visible. Tree contents, tiers and cadence are *tbd* (§15 open question 3).

## 12. Outbreak parameters (when enabled)

The virus editor, cure and origins live in [world simulation](world-simulation.md) §4 and §12 and [threats](threats.md); nothing here repeats them. What this catalogue adds is only the **switch and its neighbours**:

| Parameter | Values | Default |
| --- | --- | --- |
| Outbreak | off, on | on in the survival presets, **off** in the no-outbreak presets |
| Origin and patient zero | real or chosen place; origin type; start day | per preset |
| Virus group | sliders, trait cards, validator bands | realistic default ("necrobiotic syndrome") |
| Cure | per [0018](../decisions/0018-sandbox-answers.md) and [0021](../decisions/0021-recommended-defaults.md) | per default |
| Zombie speed, persistence | always slow, body-driven, fast; tiring off | body-driven, no tiring (Decided) |
| **Animals and the pathogen** | none: animals never carry or turn; carriers: scavengers spread contaminated material but do not turn; animals turn too | **none** (Decided, [0059](../decisions/0059-countries-from-water-land-and-resources-everything-alive-can-be-sick.md)); animals still get their own ordinary diseases |
| **Late trigger** | Outbreak off at the start, but allowed to begin later at a chosen day or from a chance of a lab accident, a spillover or a smuggled sample | off by default. Decided as an option ([0028](../decisions/0028-births-military-mystery-tech-leaders.md)) |
| **Mystery** | The outbreak is real, but nobody knows what it is: a normal epidemic or the dead rising | off. Decided as an option |
| **Outbreak odds** | How likely a late outbreak is over a chosen span: never (the off option), rare, likely, certain on a date | never |

### 12.1 "Epidemic or apocalypse?" (Decided as intent, [0028](../decisions/0028-births-military-mystery-tech-leaders.md); model Proposal)

With a late trigger and the mystery option, the world may or may not get an outbreak, and when it does, nobody knows which kind it is. It uses only existing rules:

- **The truth is fixed at the start of the run** (the cause is drawn once from the settings and the seed) and **hidden by fog**: the world fields and the character's beliefs ([world simulation](world-simulation.md) §1). The player can't read it from the setup preview while playing.
- **Early cases look like any epidemic:** fever, hospital load, news, absenteeism. What separates a normal epidemic from the dead rising is real and observable: whether the dead stay dead, the **time to turn** and how hidden it is. A first rise is what reveals it, as it would in life.
- **Pops and authorities act on beliefs:** health officers treat it as a flu-like epidemic, cordons and edicts follow the believed kind, and some pops hide bites. A wrong belief has real consequences (quarantine wards that turn into death traps).
- **Outcomes:** a normal epidemic can burn out like any epidemic; a late-triggered one can fizzle or go global by the same calibrated odds ([world simulation](world-simulation.md) §4). The player never sees a countdown or a script.
- **Spoilers (Decided: hidden by default):** with the mystery option, the setup preview must not show the cause. The game draws it from the seed and shows only a mix of outcomes; a sandbox "reveal" button exists for testing and tags the save. A creator who wants to know simply turns the mystery off.
- **Off option:** setting the outbreak to off, or the odds to never, gives no outbreak at all, with no hidden cases and no hidden cause; the world's ordinary illnesses still exist.

## 13. Scenario and goals

A scenario only sets numbers or starting state, schedules real events and adds objectives (the validator enforces it, [world simulation](world-simulation.md) §11). For no-outbreak play the objective library needs its own entries:

| Kind | Examples (each a measurable condition on derived numbers) |
| --- | --- |
| **Sandbox (none)** | No end, no objectives |
| **Life** | A pop reaches an aspiration; a family survives three generations |
| **Business** | A company reaches a market share, a size or an age |
| **Politics** | A nation holds N free elections; reaches a stability, trust or GDP level; a reform passes |
| **Survival** | Survive N days (the existing Tier 0 goal), keep a cordon, reach a cure |
| **Failure to avoid** | Collapse of state, famine, bankruptcy of the player's company |

### 13.1 Realism checks for long runs (Decided as a use, [0032](../decisions/0032-real-time-reference-and-generational-runs.md); model Proposal)

A no-outbreak run left to run for decades (a few generations) is a test of the whole sim: do the pops behave like real people? The headless runner produces a **realism report** compared with real data for the world's settings, never with a feel:

| Check | Compared with |
| --- | --- |
| Age pyramid, life expectancy, deaths by age and cause | Real demography for the nation and culture |
| Births per woman, age at first child, marriage and divorce rates | Real fertility and family data |
| Household size and structure over time | Real household statistics |
| Employment by job, unemployment, job changes, retirement | Real labour data ([content tools](content-tools.md) §4) |
| Wealth distribution (Gini), mobility between generations | Real inequality and mobility data |
| Business formation, survival and closure | Real firm-demography data |
| Migration, urbanisation | Real rates |
| Crime and unrest | Real rates, tuned by culture |

Failures are reported as a band, not a pass or fail: "fertility 1.1, real 1.6–1.9". Runs with the same seed and settings give the same report (determinism). A population that dies out, explodes, or freezes in its starting shape is a finding about the model, not about the player. The data sources follow the licence list in [world simulation](world-simulation.md) §3. A run needs the long steps and the lazy-pop jumps from [pop continuity](pop-continuity.md) to be fast enough: that is part of its benchmark.

## 14. Presets (Proposal)

Whole-world presets, each a set of nation, culture, government and corporation presets.

| Preset | Outbreak | Feel |
| --- | --- | --- |
| **Seven days (default)** | on | The warning week, then the outbreak ([vision](../vision.md)) |
| **Quiet world** | off | Real Earth, watch it run, no goals |
| **Generations** | off | Long steps over decades; a realism report at the end ([§13.1](#131-realism-checks-for-long-runs-decided-as-a-use-0032-model-proposal)) |
| **Small town sim** | off | One town, Sims life, local politics and shops |
| **Nation builder** | off | One generated nation, a government chair, Cities: Skylines meets Stellaris |
| **Corporate ladder** | off | A company, a market, regulators |
| **Cold peace** | off | Several generated nations, tense diplomacy, an assembly |
| **Slow burn** | off, late trigger | Years of normal life, then a lab accident |
| **Is it the flu?** | off, late trigger, mystery | A normal epidemic or the dead rising: nobody knows which |
| **Hard mode** | on | Fast, hidden-bite, collapsing governments |

## 15. Questions

### Answered (2026-10-06, [0028](../decisions/0028-births-military-mystery-tech-leaders.md))

1. **Births:** part of the simulation (§7.4), with leaders and researchers who age and die.
2. **Military power:** counts in the simulation regardless of combat gameplay; combat gameplay to exist in some form later (§5.1).
3. **Late outbreak and mystery:** yes, with an off option (§12, §12.1).
4. **Technology:** a Stellaris-style tree is wanted; shape tbd (§11.1).
5. **Everything makes sense in the simulation:** the whole catalogue obeys the one contract; the hard part is levels of detail and pop levels (below).
6. **Player in a corporation or any office:** yes; officeholders are selectable in higher layers and playable in Layer 1 (§7.4).

### Answered (2026-10-06, second round, [0029](../decisions/0029-leader-records-foxhole-tech-readout.md))

- **Officeholders always stay records:** yes.
- **War:** study Foxhole for its concept and gameplay ([research](../research/foxhole.md)).
- **Tech tree:** a nation's tree reflects the items, establishments, cities and buildings in Layers 2 and 1 (§11.1).
- **Mystery spoilers:** explained; default hidden, reveal button in sandbox (§12.1). Awaiting confirmation.
- **Pop presets:** a template made once per world, plus an optional import (§7.3). Awaiting confirmation.
- **First slice:** as recommended; macro → micro play style unchanged.

### Answered (2026-10-06, third round, [0030](../decisions/0030-pop-continuity-and-capability-loss.md))

- **Pop templates** (§7.3) and **hidden mystery with reveal** (§12.1): Decided.
- **A pop is the same at every layer:** Decided; model in [pop continuity](pop-continuity.md).
- **War:** secondary for now; waits for the Foxhole research.
- **Capabilities are lost, not forgotten;** loss travels down and up (§11.1, [pop continuity](pop-continuity.md) §5).

### Open

1. **Pop continuity spec and benchmark** ([pop continuity](pop-continuity.md) §7): record budget, long time steps, layoff selection rules.
2. **Restoring a lost capability:** Decided ([0031](../decisions/0031-restoring-capabilities-real-time-playable-age.md)): nearly impossible, never done by the AI on its own, a human goal or late-game play needing trade and cooperation. Open: the exact cost model (what "nearly" means in parts, people, time and partners).
3. **War scale and look:** after the Foxhole research.
4. **Tech tree contents:** which capabilities and how many tiers.
5. **Starting speed of a new game:** Decided: 1 hour = 1 day, 1:1 one key away ([0033](../decisions/0033-restoring-cost-and-two-way-projects.md)).

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- Does the played fight resolve a slice of the campaign (one front, one night) or the whole engagement, and what ends it? ([war-in-other-games](../research/war-in-other-games.md))
- How are casualties assigned when a fight is resolved in aggregate: draws over pops, weighted by role and armour? ([war-in-other-games](../research/war-in-other-games.md))
- May aggregate results be better or worse than expected play, or must their expected values match? ([war-in-other-games](../research/war-in-other-games.md))
- How much logistics does the player touch directly (hauling) versus set as policy? ([war-in-other-games](../research/war-in-other-games.md))
- What does a watching player see: a front on the map, a feed of events, a unit list, or all three? ([war-in-other-games](../research/war-in-other-games.md))
- What ends a war: exhaustion, objectives, collapse of loyalty or supply? ([war-in-other-games](../research/war-in-other-games.md))
- What does a commander see at each scale, and how do large numbers stay readable beyond a map of towns and regions? ([foxhole](../research/foxhole.md))
- What rule makes a front hold or move (Foxhole's capture rule was not found)? ([foxhole](../research/foxhole.md))

## 16. Order of work (dependencies only)

Nothing here blocks Tier 0. After the world-sim order ([world simulation](world-simulation.md) §14): the validator and the setup file format (small, needed by everything) → household, group and ideology record → business on the ledger (jobs, stock, wages) → nation actor with budget and edicts → life events and aspirations → corporations and ownership → long time steps → scenario objectives and presets → setup screens. Each unit gets a spec first ([CLAUDE.md](../../CLAUDE.md) rules).

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [foxhole](../research/foxhole.md): Foxhole, as a reference for war and logistics
- [project-zomboid](../research/project-zomboid.md): Project Zomboid, as a reference for the Individual layer
- [war-in-other-games](../research/war-in-other-games.md): War in other games, as a reference
- [world-systems-economy](../research/world-systems-economy.md): World systems: how a real economy works in numbers
- [lore-made-and-generated-worlds](../research/lore-made-and-generated-worlds.md): Lore in made and generated worlds: Project Zomboid, other games, and tools outside games
