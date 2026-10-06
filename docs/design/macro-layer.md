# Macro layer (Layer 3)

How the world map plays: what you see, what you can decide, how dated events reach you, how countries and cultures are set up before the run, and how a decision here shows up in a hospital two layers down. The simulation underneath (population, outbreak, economy, edicts) is defined in [world simulation](world-simulation.md); this doc only adds the play on top of it.

Labels: **Decided** (the user confirmed it, see [0022](../decisions/0022-macro-layer-intent.md)), **Proposal** (not confirmed). Research and sources: section 10.

## 1. The idea in one paragraph

Layer 3 mixes three games. From **Plague Inc** it takes the world map where you watch the outbreak spread, with a headline feed and world totals. From **Hearts of Iron IV** it takes a real-time calendar that keeps running, events that pop up with a few options and resolve themselves if you ignore them, and decisions aimed at a place on the map. From **Stellaris** it takes the setup screen where you build the world, its peoples and its governments (and save them as presets), the country panel with its government and cabinet, and an assembly of nations that meets and votes. You don't play one nation. You are a **director**: you watch, and when you want you take a decision for whoever holds that decision in the world. Every country's decisions, yours or not, run on the same rules (Decided as intent).

## 2. Who you are in Layer 3

| Rule | Status |
| --- | --- |
| You are not a nation. You watch the simulation and step in where you want. | Decided |
| A decision is always taken **as** the actor that owns it in the world (a government, a ministry, a council member). The order goes out through that actor's offices, with its money, staff and legitimacy, and people obey or not by their own rules. This is the Director role from [world simulation](world-simulation.md) §1 and research `loop/16d` §4. | Proposal |
| There is no abstract decision currency (no political power, influence or DNA points). The cost of a decision is what it does in the world: money, staff pulled from elsewhere, lost trust. This matches "sliders, no points" ([0018](../decisions/0018-sandbox-answers.md)). | Proposal |
| Playing micro → macro, Layer 3 shows only what your character's role allows and what they believe (fog, [0012](../decisions/0012-world-simulation-layers.md), [0018](../decisions/0018-sandbox-answers.md) §1). Every other play style may take any chair and sees the truth. | Decided (0018) |
| Sandbox adds "world sliders" (virus numbers, weather) labelled as settings, outside any actor. | Proposal (research `16d`) |

## 3. What you see

### 3.1 The map

- **Default start:** Layer 3, world map, outbreak running (Decided).
- **Hover** a country: its border lights up and a small tooltip shows name, population and outbreak state. **Click**: the country is selected, the camera centres on it and the country panel opens (Decided).
- **Map modes** (one key each, Proposal): political (countries and borders), outbreak (infected, dead and risen per region; known vs true when fog applies), response (each region's authority state: normal → advisory → travel ban → curfew → cordon → martial law → collapse), collapse pressure ([0021](../decisions/0021-recommended-defaults.md) §3), supply (food and fuel days left), power grid, trust, flows (travel, refugees, aid shipments as moving lines, Plague Inc planes and ships), relations (standings with the selected country).
- **Orders on the map:** every active decision shows as an icon on its target with a progress ring, and highlights its area while it runs (HOI4 state-targeted decisions, research `22c`).
- **World bar** (Plague Inc bottom bar, Proposal): world totals of healthy, infected, risen and dead; the leading cure project's stage; today's date and speed controls.
- **Headline feed:** every headline is generated from a real event record; clicking it flies the camera there. No filler headlines (research `16d` §2.3, `22c`).

### 3.2 The country panel (Stellaris-style, Proposal)

Tabs, all read from the simulation; nothing here is a separate stat.

| Tab | Shows |
| --- | --- |
| **Overview** | Population; healthy, infected, risen, dead; trend graph; authority state; the three numbers that most limit the response right now (for example "hospital staff 61%", "fuel 4 days", "trust 38%"). |
| **Government** | Government type, head of government, cabinet seats (interior, health, defence, foreign affairs, economy), next election or succession, legitimacy. Each office holder is a real record in the census, so you can open them, follow them and possess them like any pop. |
| **Laws and edicts** | Active laws and edicts with their numbers, start date, issuer and upkeep; queued ones; a forecast vs actual plot for each (research `16d` §2.2). |
| **Economy and supply** | Output by sector and what limits it (workers, inputs, power); stocks and days left; imports and exports; aid given and received. |
| **Health** | Hospital beds and staff vs need, labs and their cure projects, medicine stock. |
| **Relations** | Standing with every other country (a number and its reasons), treaties, open borders or closed, aid flows, votes in the assembly. |
| **Chronicle** | Significant events in this country, filtered and linked to their records. |

### 3.3 Layer by zoom (Decided as intent; mechanism Proposal)

The game knows which layer you are in and switches the interface, the information and the camera to match (Decided).

- **The layer is picked by camera height**, with a gap between the "enter" and "leave" heights so the interface doesn't flicker at the border. Globe and countries are Layer 3; zooming onto a region shows Layer 2 for that place; taking a body is Layer 1.
- **Each layer has its own interface set:** Layer 3 shows the country panel, agenda and decisions; Layer 2 shows building and pop inspectors and city verbs ([world simulation](world-simulation.md) §13); Layer 1 shows the body, objectives and actions.
- **Entering Layer 2 somewhere new** (Africa, where infection is high) makes that place the detailed zone. Turning a region's counts into individual people and buildings is work, so it happens behind the switch transition with time frozen, and it never changes outcomes ([world simulation](world-simulation.md) §8, research `16d` §3). Only one place is detailed at a time; when you leave, its people go back to records with every count kept (Proposal: the compute budget for more than one zone is unmeasured).
- **Fog:** playing micro → macro, zooming out shows what your character believes, not the truth.

## 4. Agenda and events

Pichu's example: a pop-up "UN Council meeting on 21 July" prompts you to be aware and maybe take a decision; if you don't act, the simulation decides on its own and never waits for you (Decided).

### 4.1 Two kinds of events (Proposal)

| Kind | What it is | Examples |
| --- | --- | --- |
| **Agenda** (scheduled) | A dated item known ahead of time: it sits on a calendar strip at the top of the map and gives a reminder a few days before. | An assembly session, an election, a budget vote, a summit, a public holiday or mass gathering, a health emergency committee meeting. |
| **Decision event** | A situation the simulation has produced that needs an actor to choose. | "Country X asks for medical aid", "Hospital in Y overwhelmed: requisition the stadium?", "Truckers refuse to enter the cordon", "Neighbour closes its border: retaliate?". |

**Events are detected, never injected.** The game watches the simulation's state and raises an event when a real condition holds (the aid request exists because that country's hospitals are short). No outside storyteller adds disasters to make things interesting (research `16d` §2.3: our storyteller detects and narrates only). Scheduled items come from the world's own calendar (election cycles, assembly sessions) or from actors that call meetings because of the state (an emergency session called because a threshold was crossed).

### 4.2 How an event plays (Proposal)

1. **It appears** as a toast or a pop-up card with a short text, the actor it belongs to, a target on the map, and a deadline in game time.
2. **Each option is an existing order:** an edict, a project, a vote, a transfer. No option does something the simulation can't do some other way. Each shows a forecast, labelled as a forecast.
3. **One option is marked "the government will choose this"**: what that actor's AI would pick by its own priorities, the same choice it makes when you are not watching.
4. **If you don't answer before the deadline, that choice is applied.** Hearts of Iron IV picks the first option after 13 days by default and Stellaris picks the first option when a pop-up is closed; we pick the owner's own choice instead, so ignoring an event is the same as the world deciding without you. Time never stops for the card (Decided). Opening the card to decide pauses the game, like any order card ([0014](../decisions/0014-open-questions-answered.md) §6); a card you only glance at does not.
5. **Notification settings per event type** (Paradox message settings): ignore, log only, toast, pop-up, or pop-up and pause. Default: everything is a toast or pop-up and nothing auto-pauses; the player can turn on auto-pause for the types they care about.
6. **Afterwards** the event's record shows what was chosen, by whom (you or the AI) and the actual result next to the forecast.

### 4.3 Situations (Proposal)

A long problem in one place (a hospital system collapsing, a cordon leaking, a government losing control) shows as a **situation**: a bar with stages, computed from the simulation's own numbers, tracked in an outliner list (Stellaris situations). Each stage can raise a decision event. Unlike Stellaris, the "approach" is not a separate choice: it is whichever edicts are in force there, so changing it means changing the orders.

### 4.4 The assembly of nations (Proposal)

A world body that meets on a calendar and in emergency sessions (Stellaris Galactic Community; the real UN Security Council, which in September 2014 called the Ebola outbreak a threat to peace and security in resolution 2177).

- **Members** are countries; each votes by its own AI priorities (health, economy, its standing with the proposer, fear of the outbreak at home).
- **Resolutions** are edict templates applied to every member that ratifies them: an aid pool (a share of medical stock or staff sent abroad), a travel recommendation, shared cure research, a border protocol. Compliance follows each country's own rules: a country under pressure may ratify and then not deliver.
- **Your part:** as director you may take a member's chair to propose, lobby (spend that country's standing) or vote. Ignoring the session lets every member vote by itself.
- **Names:** real Earth uses real countries; international bodies get invented names ("the World Assembly"), and leaders and office holders are always generated people, never real politicians (Proposal).

## 5. From a law to a hospital: the causal trace

Pichu's example: a Layer 3 law sends a percentage of medical supplies to global aid; in Layer 2 a hospital in one zone is understaffed and short of supplies **because of** that law.

- This already works in the simulation: an aid edict moves real medicine and real staff out of the region's stock ([world simulation](world-simulation.md) §2 rule 2, §5). The hospital's output is limited by the scarcest of staff, inputs and power.
- **What we add (Proposal): every transfer carries the reason it happened**, the id of the edict, project or event that caused it. The ledger already records who moved what; the reason is one more field.
- **The building inspector's Status tab names its limit and its cause** ([world simulation](world-simulation.md) §13): "Output 54%: limited by staff (9 of 20). 6 nurses reassigned to the national aid quota, edict 'Medical aid 30%', signed day 12 by the Health Ministry under World Assembly resolution 4." Each name is a link: click it and the camera goes to the edict in Layer 3.
- **The other way round:** an edict's panel lists where it bit hardest ("top 5 places that lost staff to this order").
- This is the reason the three layers exist: you see the decision above and its price below.

## 6. Setup: building the world (Decided as intent; model Proposal)

Before the run you set the world, its countries and cultures, the outbreak and the cure, then start. Setup already has four screens (World, Outbreak, Scenario, Character; [world simulation](world-simulation.md) §1, §12). This adds **Countries and cultures** to the World screen.

### 6.1 Every field has three ways to fill it

Following Stellaris empire creation and Pichu's "auto fill, customize one by one, or pick a preset":

1. **Auto:** the generator fills everything from the world fields (geography, settlement suitability, research `22b`).
2. **Preset:** pick a saved design (a culture, a government, a whole country, a whole world).
3. **Custom:** edit one by one. Any edit can be saved as a preset and shared as a code.

### 6.2 World-level settings

| Setting | Notes |
| --- | --- |
| World | Real Earth (default) or generated. Size and land share for generated worlds. |
| Number of countries | For generated worlds, for example 37. Real Earth uses real countries. |
| Population | Total, and how clustered it is: share living in cities (urbanisation) and how dominant the biggest city is in each country (city size spread). Real cities follow a rank-size rule; the slider bends it. |
| Cities | Number of cities per country, or let the suitability function place them. |
| Cultures | How many, and how mixed (one per country, or spread across borders). Each culture is a data package: names, building style, family size, trust in authority, how people react to rules ([world simulation](world-simulation.md) §12 culture kits). |
| Institutions | Whether a world assembly exists, how often it meets, how binding its resolutions are. |

### 6.3 Country settings

Each country is a record of numbers; the validator gives each a realistic, sandbox and rejected band ([world simulation](world-simulation.md) §2 rule 13).

| Field | Drives |
| --- | --- |
| Name, flag, colour | Display only. |
| Government type (democracy, single party, military, monarchy, collapsed) | Which edicts need which approval, election cycle, how fast orders pass, use-of-force limits. |
| Wealth and economy mix | Budget, stocks, what is produced at home vs imported. |
| Health capacity (beds and doctors per 1,000) | Hospital capacity and cure research. |
| State capacity (how well orders are carried out) and public trust | Response lag and compliance. |
| Infrastructure (roads, grid, phones) | Travel, backup clocks, how far orders reach. |
| Borders and transport (airports, ports, roads to neighbours) | Spread routes and what a border closure can stop. |
| Cultures, languages, religions, ancestry and values with their shares | The people the census generates, and what children start with (§6.5). |
| AI priorities | What this government tends to choose: protect the economy, protect lives, protect the regime, help neighbours. |

On real Earth, country numbers start from public data (population, health capacity, infrastructure) and editing them is a sandbox change, tagged as such ([0014](../decisions/0014-open-questions-answered.md) §9).

### 6.5 Creating a country, and how pops inherit (Intent Decided, [0024](../decisions/0024-country-creation-inheritance.md); model Proposal)

Research: `research/global-sim/loop/28-country-creation-inheritance.md`.

- **Screen, in Stellaris order:** People → Values → Government → Economy → Services → Security → Origin → Name and flag. Every field fills from real Earth data, can be edited, randomised or saved as a preset. No point budgets and no trait bonuses.
- **People:** population, age structure, urbanisation; cultures and languages with shares; religions with shares and devotion; ancestry mix (appearance only); education and health.
- **Values:** four opposed axes as Stellaris ethics, but as a spread across the population, not one stance, drifting with events (war pulls towards militarism); plus trust and individualism vs collectivism.
- **Down to pops:** country shares split as whole numbers into cells per region, and each region's census slots map to those cells, so the lazy census stays exact.
- **Inheritance:** a child's body mixes both parents' genes (looks, height potential, some health risks). Culture, language, religion and political leaning start as the family's beliefs (a mixed family gives a mix), then change at school, between about 15 and 27, and after life events (moving, war, the outbreak). Calibration: in the US, teens match their parents' religion and party roughly 8 times in 10; the party link weakens into the late twenties; immigrant languages mostly fade by the third generation; minorities keep their culture longer when parents work at passing it on (Bisin–Verdier).
- **New cultures:** a mixed community that lasts for generations can become a new culture record (Crusader Kings 3 hybrid cultures), and the world view shows these shifts as map modes.
- **No bonuses from ancestry:** differences between pops come only from upbringing, circumstances and their own body.
- **Where it plugs in:** the culture packs and pop generator in [content tools](content-tools.md) (owned by the content tools thread), the lifepath in [world simulation](world-simulation.md) §3, the pop inspector and the country panel (§3.2).

### 6.4 Outbreak and cure

The virus editor and cure settings already exist: sliders on real rates, trait cards for the feel, no points, and a preview from many headless runs ([world simulation](world-simulation.md) §12, [0018](../decisions/0018-sandbox-answers.md)). Different origins, countries and cultures give very different runs (Decided).

## 7. What the simulation needs underneath (Proposal)

Most of it exists in the world-sim design. New pieces:

| Piece | What it is |
| --- | --- |
| **Country** | An actor that owns regions: government type, offices (each held by a census record), budget, legitimacy, laws and edicts, AI priorities, relations. Built from the existing Actor and Edict records. |
| **Standing** | A number between two countries with its reasons (aid given, borders closed on them, votes), decaying over time. |
| **Assembly** | Members, a session calendar, resolutions as edict templates, votes. |
| **Calendar** | Scheduled items (sessions, elections, holidays) owned by actors. |
| **Event detector** | Rules "when this condition holds for this actor, raise this decision with these options". Reads state only. |
| **Event record** | Options (existing orders), the AI's choice, deadline, who chose, forecast and actual. |
| **Transfer reason** | One field on every ledger transfer: the order that caused it. |

Cost: countries are a few hundred actors that decide about once a game day from a fixed menu, which is cheap next to people (research `global-sim.md`: the planet is cheap, people are expensive).

## 8. Copy and avoid

| From | Copy | Avoid |
| --- | --- | --- |
| Plague Inc | World map and spread lines, world totals bar, headlines tied to events, clicking a country for its numbers, customising the disease. | DNA points; a single world cure bar with no lab behind it; headlines that are just flavour. |
| Hearts of Iron IV | Real-time calendar, events with options that resolve themselves by deadline, decisions aimed at places on the map, map modes, message settings. | Political power as a currency; scripted events that fire on a date regardless of the world. |
| Plague Inc: Cure Mode | Per-country non-compliance that rises with harsh measures and falls with economic support, and that reopens borders when it peaks; a response that fails if trust collapses. We already model this as compliance per pop with thresholds ([world simulation](world-simulation.md) §5); the country panel shows it. | One global Authority meter as a lose condition: our failure is only what happens in the world. |
| Plague Inc: government actions | A ladder of responses that each government climbs as deaths and awareness rise, and government collapse that stops cure work and reopens borders. Our authority states (§3.1) are that ladder with real lags. | Fixed death-percentage triggers; ours come from each government's beliefs and priorities. |
| Stellaris | Galaxy and empire creation with presets and random fill; the country panel with government, cabinet and next election; an assembly that meets and votes; the outliner. | Pop lag (our people are lazy until touched); abstract influence. |

## 9. Order of work (dependencies only)

This waits for the world-sim order ([world simulation](world-simulation.md) §14): census and records, region graph with authorities, economy, then: country actors and the transfer reason → event detector and records → calendar and assembly → country panel, map modes and agenda → setup screen for countries and cultures with presets. Nothing here is needed by Tier 0.

## 10. Research and sources

- **Hearts of Iron IV.** Events time out after 13 days by default (`EVENT_TIMEOUT_DEFAULT`, per-event `timeout_days`) and the first option is applied; AI picks by `ai_chance` weights; news events reach every country and their pop-up can be turned off ([Event modding](https://hoi4.paradoxwikis.com/Event_modding)). Decisions need conditions and usually cost political power; missions are decisions with a time limit ([Decisions](https://hoi4.paradoxwikis.com/Decisions)). Political power: +2 per day base, spent on focuses, laws, advisors, diplomacy and decisions; stability scales its gain ([Political power](https://hoi4.paradoxwikis.com/Political_power)). The government view shows leader, political system, ideology support, focus tree, laws and advisors ([User interface](https://hoi4.paradoxwikis.com/User_interface)). Per-message pop-up and pause settings: general Paradox knowledge, not checked.
- **Stellaris.** The Galactic Community alternates a 720-day recess and a 1440-day session; one proposal per empire in the queue; votes weighted by diplomatic weight; Council members can call an emergency measure once every 20 years ([Galactic Community](https://stellaris.paradoxwikis.com/Galactic_Community)). Situations: a staged progress bar with approaches that change its rate, tracked in the outliner ([Situations](https://stellaris.paradoxwikis.com/Situations)). Closing an event pop-up picks the first option ([Events](https://stellaris.paradoxwikis.com/Events)). Galaxy setup: size 200–1000 stars, 10 shapes, number of AI empires, fallen empires, habitable worlds, tech and tradition cost, crisis strength, mid- and end-game years ([Galaxy](https://stellaris.paradoxwikis.com/Galaxy)). Empire creation: 3 ethic points, 2 trait points and 5 picks, saved designs and random fill ([Ethics](https://stellaris.paradoxwikis.com/Ethics), [Species traits](https://stellaris.paradoxwikis.com/Species_traits), [Empire](https://stellaris.paradoxwikis.com/Empire)).
- **Plague Inc.** Countries differ in population, density, climate, wealth and a hidden cure budget ([Countries](https://plagueinc.fandom.com/wiki/Countries)). Government actions escalate with discovery, death thresholds and awareness; closing airports, ports or borders cuts infectivity; collapse stops cure research and reopens borders ([List of government actions](https://plagueinc.wiki.gg/wiki/List_of_government_actions)). The Scenario Creator edits the disease, countries (population, climate, health budget, borders), government responses and scripted events ([Scenario Creator](https://plagueinc.fandom.com/wiki/Scenario_Creator)). Cure Mode (2020–2021): the player leads a health organisation, with an Authority meter and per-country non-compliance that reopens borders at 100% ([Cure Mode](https://plagueinc.wiki.gg/wiki/Cure_Mode), [Non-Compliance](https://plagueinc.wiki.gg/wiki/Non-Compliance)).
- **Director games.** WorldBox: a god sandbox with no nation to play; civilisations grow, ally and fight on their own ([Wikipedia](https://en.wikipedia.org/wiki/WorldBox)).
- **Real anchors.** The UN Security Council called the Ebola outbreak a threat to international peace and security in resolution 2177 (18 September 2014). The WHO declared COVID-19 a public health emergency of international concern on 30 January 2020. General knowledge, not re-checked in this pass.
- **Earlier project research:** `research/global-sim/loop/16d` (edicts, Director role, chronicle, auto-pause classes, transitions), `22a` (Plague Inc and the virus editor), `22b` (world generation and cultures), `22c` (map-targeted orders, inspectors, possession); summary in [research/global-sim.md](../research/global-sim.md).
