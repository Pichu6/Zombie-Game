# Sandbox parameters

The catalogue of everything the player can set before (and, where allowed, during) a run: the session, the world, nations, cultures, pops, households and groups, businesses, corporations, government and politics, the economy, infrastructure, and the outbreak. It also says how the game stays a full game **without an outbreak**: a politics, grand-strategy and Sims-like life simulation on the same world ([0023](../decisions/0023-no-outbreak-game-and-parameter-catalogue.md)).

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

Data, not code, like the content packs ([content tools](content-tools.md) §1 rule 3): TOML presets under `data/setup/` (`world/`, `nation/`, `culture/`, `government/`, `corporation/`, `scenario/`), each with an `id`, `version`, a `[meta]` block (name, author, realism tag) and the fields below. The setup screens, `zs_headless`, the sandbox window and tests read the same files. A run records the final resolved settings and the seed in its save, so a preset update never changes an old save.

## 2. The outbreak is a module

With **no outbreak** (zero first cases, Decided in [0018](../decisions/0018-sandbox-answers.md)), the zombie and infection rules never fire and the census never moves to infected or risen. Nothing else changes: the same bodies, the same economy, the same rules. That is only a game if the rest of the world carries it, so these must hold when the outbreak is off (all Proposals, none built yet):

| Needed for a no-outbreak game | Why | Today |
| --- | --- | --- |
| **Ordinary life and death:** ageing, ordinary illness, accidents, crime, births, marriage, migration | A world with no outbreak must not be static, nor empty out | Body, illness and wounds exist; births and ageing are not modelled (Open, §15 question 1) |
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
| Time scale | 1×, 3×, 10×, rest-until-event, and **long steps** (day, week, month, year per second) | 1 game day per real hour | Long steps only at Macro/City. A sandbox setting (Decided in [vision](../vision.md)); the long steps are a Proposal |
| Auto-pause classes | per event type: ignore, log, toast, pop-up, pop-up and pause | toast/pop-up, no auto-pause | [macro layer](macro-layer.md) §4.2 |
| Fog | on (what the character believes) or off (truth) | on in micro → macro | Decided |
| Instant build / god tools | off, sandbox only | off | Decided ([0014](../decisions/0014-open-questions-answered.md)); tagged |
| Permadeath of the played pop | on, or switch to a relative / next in office | on | Control passes on death, as in the run ([spec 26](../tier0/specs/26-run.md)) |
| Realism tag | realistic, sandbox (set automatically from the bands) | realistic | Shown on the save and on shared presets |

## 4. World parameters

Intent Decided ([0016](../decisions/0016-sandbox-setup-and-layer-flow.md)); model [world simulation](world-simulation.md) §12.

| Group | Parameters | Notes |
| --- | --- | --- |
| **Source** | Real Earth (default) or generated; size and land share (generated) | Real Earth only accepts sliders that don't contradict the data (population, resources, wildlife) |
| **Physical causes** | Temperature, seasonality, moisture, relief, fertility, resources, vegetation, wildlife | Sliders; biome is painted, never a slider |
| **Population** | Total; urbanisation (share in cities); city size spread (rank-size bend); cities per nation or auto | [macro layer](macro-layer.md) §6.2 |
| **Nations** | Number (generated worlds); size spread; how borders follow geography (rivers, ranges) | Real Earth uses real nations, with invented leaders ([macro layer](macro-layer.md) §4.4) |
| **Cultures and religions** | How many; one per nation or spread across borders; how mixed | Data packages, §6 |
| **Languages** | Count and mutual intelligibility | Proposal; affects trade, trust and rumour spread. *Open whether needed for v1* |
| **Resources and trade geography** | Deposit richness per resource; chokepoints; sea routes vs land routes | Feeds §11 |
| **Disasters** | Frequency and severity of floods, droughts, quakes, storms, fires, plus *crop failure* | Detected-not-injected rule: they happen from the world's own state (weather, tectonics), at the set rate. Default realistic; "none" is allowed |
| **Institutions** | A world assembly exists or not; how often it meets; how binding its resolutions | [macro layer](macro-layer.md) §4.4 |
| **Pace of change** | Technology and innovation rate; urban growth rate | Proposal. Technology tree vs none is *Open* (§15 question 4) |

## 5. Nation parameters

A nation is an Actor that owns regions, a budget, offices (each held by a real census record), laws and relations ([macro layer](macro-layer.md) §7). Each field has realistic, sandbox and rejected bands.

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

War is **not** in the catalogue yet: it breaks "nothing easy, everything real" if abstracted, and needs its own decision (§15 question 2).

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

## 12. Outbreak parameters (when enabled)

The virus editor, cure and origins live in [world simulation](world-simulation.md) §4 and §12 and [threats](threats.md); nothing here repeats them. What this catalogue adds is only the **switch and its neighbours**:

| Parameter | Values | Default |
| --- | --- | --- |
| Outbreak | off, on | on in the survival presets, **off** in the no-outbreak presets |
| Origin and patient zero | real or chosen place; origin type; start day | per preset |
| Virus group | sliders, trait cards, validator bands | realistic default ("necrobiotic syndrome") |
| Cure | per [0018](../decisions/0018-sandbox-answers.md) and [0021](../decisions/0021-recommended-defaults.md) | per default |
| Zombie speed, persistence | always slow, body-driven, fast; tiring off | body-driven, no tiring (Decided) |
| **Late trigger** | Outbreak off at the start, but allowed to begin later at a chosen day or on a chosen condition (a lab accident rate) | off. *Open* (§15 question 3) |

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

## 14. Presets (Proposal)

Whole-world presets, each a set of nation, culture, government and corporation presets.

| Preset | Outbreak | Feel |
| --- | --- | --- |
| **Seven days (default)** | on | The warning week, then the outbreak ([vision](../vision.md)) |
| **Quiet world** | off | Real Earth, watch it run, no goals |
| **Small town sim** | off | One town, Sims life, local politics and shops |
| **Nation builder** | off | One generated nation, a government chair, Cities: Skylines meets Stellaris |
| **Corporate ladder** | off | A company, a market, regulators |
| **Cold peace** | off | Several generated nations, tense diplomacy, an assembly |
| **Slow burn** | off, late trigger | Years of normal life, then a lab accident |
| **Hard mode** | on | Fast, hidden-bite, collapsing governments |

## 15. Open questions

1. **Births and ageing** in a "finite world, no respawn" game ([vision](../vision.md), [inhabitants](inhabitants.md) Open item): a no-outbreak world over decades needs births. Recommendation: a birth is a conserved event, a new record drawn from two parents' census slots, counted in the ledger; it is not a respawn.
2. **War and violence between nations:** in scope for the politics game or not? Recommendation: not in the first version; keep unrest, coups, blockades and the army as guards and orders only.
3. **Late outbreak trigger:** allowed (the "Slow burn" preset) or is the outbreak chosen only at setup? Recommendation: allowed, as a drawn event from a lab accident rate, so it is detected, not injected.
4. **Technology and progress:** a tech tree, or only skills and Q? Recommendation: none; progress is know-how and production quality through the production ladder ([0013](../decisions/0013-production-ladder.md)).
5. **Scope of the first slice:** which of nations, businesses, corporations and groups is built first? Recommendation: pops and households, then businesses, then a nation actor with a budget and three edicts, then corporations. Nothing here is needed by Tier 0 ([macro layer](macro-layer.md) §9).
6. **Player in a corporation:** may the player hold a CEO chair in micro → macro? Recommendation: yes, only through a character who actually holds the role.

## 16. Order of work (dependencies only)

Nothing here blocks Tier 0. After the world-sim order ([world simulation](world-simulation.md) §14): the validator and the setup file format (small, needed by everything) → household, group and ideology record → business on the ledger (jobs, stock, wages) → nation actor with budget and edicts → life events and aspirations → corporations and ownership → long time steps → scenario objectives and presets → setup screens. Each unit gets a spec first ([CLAUDE.md](../../CLAUDE.md) rules).
