# World simulation

One outbreak simulation of the whole world, played at three scales. This doc is the source of truth for the world layer, the population model, the outbreak, the economy, laws and edicts, communication and fortification. Body, items and infrastructure details stay in their own design docs.

Labels: **Decided** (the user confirmed it, see [0012](../decisions/0012-world-simulation-layers.md)), **Proposal** (not confirmed), **Open** (question for the user). Research behind every Proposal: [research/global-sim.md](../research/global-sim.md).

## 1. Structure

| Part | What it is | Status |
| --- | --- | --- |
| Setup | Four screens: World (real Earth or generated), Outbreak (origin, virus numbers), Scenario (start), Character (pick a local, profession start, or custom). They only set numbers and the starting state. | Decided (screens), Proposal (split into four) |
| Macro layer | Globe and regions. Watch the outbreak, use time controls, overlays and a chronicle; issue laws and edicts. | Decided |
| City layer | Isometric RTS over the local zone: pops, resources, buildings, projects, the local effect of decisions. | Decided |
| Individual layer | Play one pop, isometric or over the shoulder. | Decided |
| Play style | Chosen in setup: micro → macro, macro → micro, one layer only, or spectator. | Decided |
| Fog | Playing an individual, every layer above shows that character's beliefs (source, age, confidence), not the truth. After death, the truth and a replay. | Decided (fog), Proposal (after-death replay) |
| Verbs by role | What you can order depends on your character's role (civilian, group leader, mayor, regional authority), not on the camera. In macro → micro or spectator styles, the player takes "chairs" in governments freely. | Proposal |
| Responder start | Play the real local health officer or mayor from the census, with macro decisions during the warning week. | Proposal |

## 2. The simulation contract (Proposal)

Every system obeys these rules; they are what make "same rules for player and pops" true at every scale.

1. Every rule is written once as a rate; settings and edicts change its numbers, never the rule.
2. People, goods, money and fuel are integers that move between owners; they are never recomputed.
3. Competing outcomes are drawn in a fixed order, so nothing goes negative.
4. Randomness is keyed per entity and per purpose (Philox), with our own samplers (never `std::*_distribution`).
5. Every system reads the previous tick's state; one fixed update order.
6. Every entity has exactly one owner layer at a time; hand-offs are checked against the ledger.
7. Speed belongs to the path, never to the layer: inside the local zone everyone moves at body speed; between towns, real-world speeds in game time.
8. One fact, three renderings: views only read. A project's integer state carries over exactly between layers; rates agree in distribution.
9. Verbs come from role, not camera.
10. Spec before code: each unit gets a one-page spec (purpose, state, rules, invariants, tests, calibration source).
11. Every test states its agreement level: identical numbers, same distribution, or same relationship.
12. Multi-layer units ship an aggregation and disaggregation pair, tested together.
13. One validator for edicts, scenarios and sandbox sliders: fixed templates, with realistic / sandbox / rejected ranges.

**Minimal units** (DEVS-style, composed into bigger ones): Quantity, Draw, Stock, Transfer, Hazard, Event, Action, Component, Assembly, Body part, Body, Agent, Belief store, Cell, Project, Actor, Field (noise, light, smoke, scent), Network (power, water, roads, radio), World.

## 3. Population (Proposal)

| Level | Who | Cost per person |
| --- | --- | --- |
| Lazy census | Everyone on Earth | 0 bytes: regenerated from the world seed; their outbreak history is read from the region's daily series |
| Record | Anyone the simulation has touched (met, hired, bitten, story tie) | ~64 bytes, up to 0.5–2 KB once they have been near the player |
| Embodied | People near the player's body | Full body simulation; "hundreds" is safe, more needs a benchmark |

- Looking at someone is a pure read and never changes them. Detail follows the player's body, never the camera.
- **Human generator:** one pure function makes every person (player, pops, zombies) from a seed: census slot, body from real measurement data, health conditions with medicine clocks, job and skills, personality, name, face genes (families resemble each other), clothes, then traits as readouts.
- **Lifepath** (TTRPG style, Traveller / Twilight: 2000): each pop's past is sampled term by term backwards from who they are today, with real rates (school, training, jobs, service, marriage, children, injuries). Draws that shape the present always run; the prose biography is lazy. Shared events (a marriage, a workplace) are generated once so both people remember the same past.
- **Custom character:** takes a matching census slot before the game starts, so the world stays consistent. Profession starts: Civilian, Police, Military, Reservist, Nurse, Responder. Kit follows real custody rules (Decided).
- **Inspector:** tabs for Life (biography, timeline, schedule), Body, Mind (motives, beliefs), Ties, Things (possessions, keys, access), Know-how. Through a character's eyes it is a dossier where each fact shows its source and age and can be wrong (Decided).
- **Licences:** use public-domain or CC BY data only (Natural Earth, GeoNames, GPW/GHSL/WorldPop, OurAirports, WRI power plants, ESA land cover, UN population, ILO job data after May 2023, O*NET with credit). Body meshes from MakeHuman/MPFB core assets (CC0) or Anny (MakeHuman topology); not SMPL. OpenStreetMap only for the home region, as a separate share-alike database. One legal review before the first real-data build.

## 4. Outbreak (Proposal unless marked)

- **Model per layer:** stochastic SEIRZ compartments per region; household, work and school contact layers for records; wound-based infection for bodies. One transmission formula shared by all.
- **Calibration (Decided as a requirement):** each zombie's reproduction number is built from things the sim already has (who is vulnerable, bite chance from speed and warning, kill chance, conversion). Results:
  - A slow, visible shambler almost always fizzles once people know (each infects ~0.3 people when people fight back).
  - A fast runner goes regional and stalls at low density.
  - "Everyone reanimates" is global by definition.
  - A hidden bite (1–2 days to turn, people hide it) goes regional often, global sometimes. **Proposed realistic default.**
  - Airborne behaves like COVID.
  - Fizzle odds use the branching-process formula with superspreading (Lloyd-Smith).
- **Warning time:** arrival minus first public news. About 7 days by default (Decided), drawn narrowly around day 7 (Proposal); "most real" places patient zero anywhere on Earth by population. All 7 days are live (Decided).
- **Response:** authority states per region (normal → advisory → travel ban → curfew → cordon → martial law → collapse, plus local order), from real lags (Italy's red zone on day 2, Wuhan's lockdown on day 23, Ebola cordons). A held cordon is a valid run.
- **Origins:** viral, bacterial, "everyone reanimates", fungal, prion in the base game (numbers plus two general hooks: what makes a corpse rise, and pathogen held in water, soil or corpses). Rule-changing origins (parasites that control the living, controllers, occult) are DLC (Decided). If an occult DLC ships, it uses an invented occult, not a real religion.

## 5. Entanglement (Decided as a requirement; loop details Proposal)

Infection → sick and dead → missing workers → sector output → shelves. Fear → hoarding, flight, fewer contacts. Detection → authorities → cordons → backlash and hidden bites. Scarcity → forced contact. Staff loss → power, water and phones fail. Power use → noise and light → zombies. Every reinforcing loop has a brake, so collapse is never guaranteed.

- **Economy:** stock and flow per region, production limited by the scarcest of workers, inputs and power (Leontief-min), minimum-crew thresholds. Shops run out rather than raise prices (anti-gouging law); prices stay posted and sticky; money fails in stages (card, cash, barter).
- **Behaviour:** panic is rare; flight is the dangerous behaviour. Beliefs carry source, age and confidence. Compliance with edicts uses a persistent threshold per pop with hysteresis.
- **Infrastructure:** a dependency graph with backup clocks (cell towers hours, 911 and hospitals a day or two, water about a week, power about three weeks), then load shedding.

## 6. The living week (Proposal)

The 7 live warning days are driven by drift (the world changes every hour from its own state), uncertainty (arrival and origin not fully known), shared scarcity (you queue with pops under your rules) and obligation (job, family, neighbours). Real anchors: schools close in waves, about 16% of workers stop showing up, early movers withdraw cash and buy staples. News arrives by TV, radio, phone calls and coworkers.

## 7. Laws, edicts and build orders (Proposal)

- **Edict:** a record that may only change a rate, a capacity, an allocation, a condition ("in zone during hours"), a use-of-force threshold or a broadcast. It takes effect through real staff and materials, has upkeep, and severe ones need legitimacy. AI governments use the same edicts.
- **Build order:** order → Project (site, materials, labour, escrowed money) → job postings and material requests → pops and companies take them by the same rules as the player → goods and money move → the finished facility posts its own jobs. That is the Cities: Skylines zoning effect made of real jobs. Macro sets a programme and budget (a planner picks sites); City places the exact plot; Individual takes jobs, hauls, guards, steals or proposes (Decided: an individual can't order a plant).

## 8. One fact, every layer (Decided; renderings Proposal)

- **Power plant:** one assembly record. Macro: part of the region's power bar. City: a building with smoke, lit windows, component bars and staff. Individual: a walkable interior whose machines are the record's components (a stripped turbine is an empty slot).
- **Hand wound:** one wound on one person. Individual: movement and one-handed limits. City: slower work, a clinic queue. Macro: part of the counts.
- **Interiors:** key buildings (plant, hospital, water works, police, pharmacy, petrol station) use hand-made templates with slots; ordinary buildings get generated layouts. Layouts are saved on first visit so updates never break saves.
- **Transitions:** a plain zoom only streams art. Taking a different body runs a GTA V-style switch (pull up, pan, drop, land) that hides promoting people to bodies, building interiors and warming AI. Time is frozen and hidden during the switch, and it never changes outcomes.

## 9. Communication (Decided as intent; model Proposal)

- One rule for every channel: signal minus noise at the listener sets clarity. Voice, voice through walls and doors, radio, phone and zombie hearing all use it.
- When the grid dies, background noise drops: voices carry ~10–15× farther, engines ~30×. The generator, not the shout, brings the horde.
- Radios are assemblies (power stage, receiver, antenna, mast, cable, battery). Part quality can only fall short of rated spec; mast height matters more than quality. Phone networks jam, then die on backup batteries.
- Messages become beliefs with source and age; rumours distort as they are retold.
- Options: knowledge hides an action, skill sets the odds, missing items or access grey it out with the reason, quality changes the result. Same gates for pops.

## 10. Items, resources and fortifications

- **Every item and resource** has quality (how well made, fixed), condition (wear, repairs lower the maximum) and states (wet, frozen, rotten, contaminated, loaded, charged...). Bulk resources per batch: fuel goes stale, water is safe / unknown / contaminated, food spoils with temperature. Decided.
- **Fortifications** (Decided as intent; model Proposal): every door, window, wall, fence and barricade is an assembly of parts, breaking at its weakest part (doors usually at the strike plate, then hinges, then frame). Zombie push comes from each body's mass, posture and decay, summed; breach time falls smoothly with horde size. Failures happen in order (lock, hinges, panel, frame, glass). Every hit is a sound. Walls and floors carry load through a simple graph solved from the roof down. Calibrated on security-door classes (EN 1627), glass classes (EN 356) and mob-attack tests (ASTM F3038). Zombies don't tire by default (Decided).

## 11. Scenarios (Proposal)

A scenario can only set numbers or starting state, schedule real events, and add objectives (the validator enforces it). Presets: 7-day warning (default), Responder, Day one, Apocalypse, One week later, Six months later, Nurse at the hospital, Military base, Evacuee, Infected, Last stand, Opening hours, Nomad, Winter, Really bad day, Pacifist / Builder / Hardcore. Late starts are simulated from day 0. Shareable as a code with a fixed seed.

## 12. Order of work (Proposal, dependencies only)

Tier 0 stays as decided ([0010](../decisions/0010-concept-v2.md)): contract, body, assemblies (doors and windows included), sound core, room-graph power, ~60-person stub census, walkers and pop AI, 2D individual view, kill/keep runs. Then: census and records, region graph with authorities and behaviour, economy and infrastructure, social order, the living week, real-Earth data and world setup, macro and city views, scenarios, switching, scale.

## Open

See [vision](../vision.md#open-questions).
