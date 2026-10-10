# Focus and compression: show and compute what the player attends to, keep everything coherent

**Status:** Proposal (2026-10-07); the user's answers in section 7 are **Decided** in [0048](../decisions/0048-realistic-time-default-commit-on-observation-and-pins.md) and [0049](../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md). From the user's description: only show what the player is seeing, whatever the layer or zoom; the conscious focus decides what is detailed. Layer 3 takes political decisions that take realistic time; layer 2 shows the result (a hospital being built, pops coming and going); layer 1 lets you work in it. Intent is the user's; the mechanisms below are mine and **not decided**. Builds on [spec 27](../tier0/specs/27-detail-levels.md), [spec 28](../tier0/specs/28-pop-levels.md), [spec 29](../tier0/specs/29-embodied-pops.md), [0033](../decisions/0033-restoring-cost-and-two-way-projects.md), [layer jumps](layer-jumps.md), the [zoom continuum](zoom-continuum.md) and the [multi-resolution research](../research/layer-jumps-multiresolution.md) (19 failure modes).

## 1. The principle

> **Store the least that can reproduce everything on demand; compute detail where the player's attention is; once detail has been seen or acted on, it is true for good.**

Three kinds of attention, which replace "everything runs at full detail":

| Attention | Who gets it | Simulated as |
| --- | --- | --- |
| **Attended** (hot) | The player's body and the ring around it; a building being inspected | Full detail (bodies, cells, fields every tick), capped by a budget |
| **Watched** (warm) | Anything the player has an order or a pin on: the hospital project, a supply route, a clinic | A record with a process (progress, roster, stocks, event log), advanced by jump forms; never full detail |
| **Unattended** (cold) | The rest of the world | Counts, stocks and rates; caught up in one step when read |

An order from layer 3 makes its target at least **watched**, so a project can never be forgotten.

## 2. The compression toolbox (what "compress" means here)

Each technique is already in the repo or a small step from it; the new part is using them together.

| # | Technique | Example | Where it is already |
| --- | --- | --- | --- |
| C1 | **Counts, not entities** | 412 walkers as a number per cell until one is near | Spec 22, 28 |
| C2 | **Rates and jump forms** | Spoilage, growth, wages applied in one step over elapsed time | Spec 27 rule 2 |
| C3 | **Seeds, not stored detail** | A building's furniture is a function of its seed; generated on approach, saved on first visit | [0017](../decisions/0017-slow-endgame-and-generation-on-the-go.md), world simulation §12 |
| C4 | **Exceptions only** | A demoted pop saves only what its seed cannot say | Spec 28 rule 3 |
| C5 | **Schedules as pure functions** | Where a worker is at 08:40 is a function of (pop, time), not a simulated walk | New small unit (see [low-resource note](../research/low-resource-city-simulation.md) point 2) |
| C6 | **Sample to show, sum to tell** | Dots are a keyed sample of real pops; every number is an exact sum | [Zoom continuum](zoom-continuum.md) 3b |
| C7 | **Process, not movie** | A project is `progress`, a roster, stocks and an event log; what it looks like is derived (section 4) | This document |
| C8 | **Incremental aggregates** | A sector's totals are kept as sums updated by each change, so showing them is O(1) | Spec 28 §3 (counts are sums) |
| C9 | **Inspect on demand** | Clicking a building computes its panel from records then; nothing is maintained for display | Cities: Skylines style |
| C10 | **History into sums** | Old events are folded into totals and only notable ones kept in the chronicle | Spec 25 |

Research behind this section, with talks, papers and the storage methods compared: [pop storage and lenses](../research/pop-storage-and-lenses.md).

## 3. What keeps it coherent

1. **One owner per fact.** The finest level that exists holds it; coarser levels are sums or readouts, never a second copy (world systems §1 rule 4; failure mode 9).
2. **Observation commits.** Detail that is only derived (not stored) may be re-derived any number of times and will give the same answer (keyed, pure). The moment the player sees it or changes it, what is **observed** becomes a stored exception. You can never see a thing, look away and find it was something else (modes 1, 11).
3. **Monotone refinement.** Derived detail is a function of (plan, seed, one scalar), such that **the state at an earlier scalar is a prefix of the state at a later one**. This is what lets layer 1, 2 and 3 views taken at different times agree (section 4).
4. **Conservation across every jump:** people, kg, litres and money move between owners; a declared flow, never a mint (mode 8).
5. **Idempotent catch-up:** `advance(a, b)` then `advance(b, b)` is a no-op; save, load, wake gives the same hash (modes 10, 11).
6. **Reasons travel with orders and results** ([macro layer](macro-layer.md) §5), so the inspector can say *why* the hospital is late.
7. **A hot-only rule is never jumped** (a fight, a door push): it runs only when attended (mode 12).

## 4. Worked case: "build a hospital", through all three layers

**t0, layer 3.** The player orders a 400-bed hospital in a sector. The order is a command with a reason. It becomes a **Project** record (Decided path, [world simulation](world-simulation.md) §7; [0033](../decisions/0033-restoring-cost-and-two-way-projects.md)):

| Part | What it holds | Real because |
| --- | --- | --- |
| Plan | Site plot, floor plans, structure, a **bill of materials** (kg of concrete, steel, pipe, wire, equipment) and **labour-hours by skill**, from a hospital template and the quality system | Fixed **at order time**, so every later view matches it |
| Money | Budget escrowed from the state's ledger | Money is conserved |
| Demand | Job postings (rosters) and material requests to the supply chain | Pops and companies answer by the normal rules, [0033](../decisions/0033-restoring-cost-and-two-way-projects.md) |
| Progress | One scalar `p` in [0, 1]: work done weighted by the plan | The only thing that advances |
| Log | Events: steel delayed, blackout, strike, a worker hurt, with their causes | For the inspector |

**What advances `p`.** Work per hour = `min(labour on site, material on site, power, ...)`. Labour is the roster's pops times their skill; material is what the ledger has delivered to the site's owner; deliveries are real transfers over real routes at real speed. Cold, this is integrated in one step (`advance`); hot, the same work is done by pops doing tasks, and the two agree in distribution (A2, tested as jump against stepping).

**Layer 3 shows** `p`, the ETA, and the **bottleneck with its cause** ("waiting on 12 t of steel, route blocked at the bridge"). All are readouts.

**Layer 2 shows** a building at a **stage** derived from `p` (site clearing, foundation, frame, envelope, fit-out, commissioning), its massing and look from the plan and a seed. **Trucks** are the real deliveries in transit (each transfer is split into loads with a departure time and a path); **workers** are the roster, placed by their schedule function (home to site by shift). Nothing here is decoration.

**Layer 1 shows** the interior at stage `p`: because tasks complete in a **fixed order given by the plan** (a seeded order for crews), the rooms that are framed, wired or finished at `p` are a prefix of the plan, so walking in at any time agrees with the aerial view and with an earlier visit. If the player takes a job on the site, their labour-hours are real and raise `p` (two-way, 0033); if they sabotage or steal materials, the ledger shows it.

**Completion:** the finished facility posts its own jobs and its own supply demand. The building is then an establishment, summed into the sector's numbers.

**What is stored for the whole hospital while nobody looks:** the plan, `p`, the roster, the site's stocks and the event log: hundreds of bytes plus the plan, not a building.

**Disruption keeps it coherent:** a blackout or strike sets the work rate to zero over an interval in the log; `p` is the integral, so the views remain consistent; at layer 1 the crew is idle *now* because the cause is *now*. History is only for the inspector.

## 5. Where this can go wrong (from the research, applied)

| Risk | Rule |
| --- | --- |
| Layer 1 shows something layer 2 contradicts | One scalar `p` and a prefix-ordered plan feed both |
| The view at hour 3 differs from a view at hour 1 plus 2 hours | `advance` is pure and keyed; tests compare one jump against many steps |
| Waking the site wakes the whole district (mode 3) | A project is *watched* (record only); only an observer's ring becomes *attended* |
| Teleporting workers hide bugs (mode 19) | A worker's position is the schedule function; any override is logged with a reason |
| Seam exploits: leave, return, get more (mode 18) | Same ledger whether attended or not |
| Rare events lost in a long jump (mode 17) | Events (accident, blackout) are drawn by keyed hazards over the interval, not skipped |
| Realistic duration versus play speed | Durations are real ([0049](../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md)); a hospital's build time (years) is a number of the plan, played through the speed ladder, which runs cold with jump forms. The speed an area may run at is capped by what is attended there (section 9) |

## 6. What it needs built, in order

1. **Specs first** (no code yet): **Project** (the process above, [layer jumps](layer-jumps.md) row 9), **Schedule and position at time t** (C5), **Attention** (the three levels and who owns the budget; extends spec 27).
2. **Prerequisites already in the audit:** stocks and goods, then money, then establishments (rows 5, 6, 7).
3. **The generic jump test harness** (round trip, aggregate against enumeration, jump against stepping, order independence), applied first to the pop jumps already built.
4. A **vertical slice** to prove it: one Project (a small clinic), one sector, ten pops, one supply route, run cold, watch at layers 2 and 1, compare the hot and cold runs.

## 7. Answers (2026-10-07, Decided in [0048](../decisions/0048-realistic-time-default-commit-on-observation-and-pins.md)) and what is still open

1. **Commit on observation:** yes, where it is better for memory.
2. **Pins:** yes, but each layer must give its own information about the same thing (section 8).
3. **Durations:** real for everything; long waits are played as progress bars with a speed ladder (pause, slow, 1:1, 1 hour = 1 day, faster), [0049](../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md). A new game starts at 1 hour = 1 day and time stops while closed ([0050](../decisions/0050-start-speed-and-time-while-closed.md)).
4. **Still open:** the order of work. Recommendation: the **schedule and position at time t** unit first (small, and it serves the dots and the project's workers), then the **Project** spec.

## 8. Pins and lenses: one thing, a different view per layer

A pin is `(entity, watch level)`. What the player is shown is a **lens**: a pure function of (the entity's records, the layer, what the observer knows). The same hospital project:

| Layer | The lens shows | Source of truth it reads |
| --- | --- | --- |
| Macro | Progress, ETA, budget used, the bottleneck and its cause, effect on the sector's capacity | `p`, the ledgers, the log (sums and readouts) |
| City | The building at its stage, who is staffed, trucks and routes, what is missing at the site, supply chain links | The roster, the deliveries in transit, the site's stock |
| Individual | What a person standing there can see and has heard: crews at work, a "delayed" sign, rumours; a worker on site knows their own task and pay | The plan's prefix at `p`, what is in the person's senses and beliefs (not the ETA unless told) |

Rules: a lens never stores anything (C9, inspect on demand); a lens can only show what the layer's character could know ([macro layer](macro-layer.md) §3.3 fog; [layer jumps](layer-jumps.md) row 16); every lens of the same thing agrees, because they all read the same records. A pin also keeps the thing *watched* (section 1), so a pinned person stays a record and a pinned project keeps its process alive.

## 9. Long processes: progress bars, mini-games, and why they make fast-forward safe (Proposal, from [0049](../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md))

Every long thing is the same kind of record, a **Process**: a plan (inputs, labour, duration), a progress scalar `p`, a rate, an owner and a log. A hospital is a Process owned by a state; a crop, a cheese, a wound healing, a journey and a craft are Processes owned by a place, an item, a body. The Project of section 4 is the large case. Consequences:

1. **One display, three layers.** At Individual a Process is a **loop animation with a progress bar**; at City it is the building or field at its stage; at Macro a number with its cause (section 8).
2. **Mini-games are hooks on the rate or quality of a Process**, never a separate state: a mini-game result changes `rate` or the output's quality ([0013](../decisions/0013-production-ladder.md)); skip it and the Process runs at its plain rate. So play and fast-forward give the same kind of result (A3).
3. **Fast-forward is safe for a body inside a Process.** The rule from [spec 27](../tier0/specs/27-detail-levels.md) and [layer jumps](layer-jumps.md) row 17 is that an attended body forces the speed its rules allow. A body in a progress-bar activity runs only needs, metabolism and the Process, which all have jump forms, so its speed cap lifts; **any event** (hunger threshold, noise, a stranger, an injury) drops it to the speed the new situation needs and can pause. Combat, walking through a door and other hot-only rules keep their caps.
4. **Several Processes at once** (Haven and Hearth style): a body or a place can run many; the player queues and switches while bars fill.
5. **Open:** whether time passes while the game is closed (0049 open point); a Process model makes either answer cheap, because catch-up is the same `advance`.

## Open

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- Do Roaring-style compressed id sets beat plain sorted arrays at our roster sizes? Needs a measurement. ([pop-storage-and-lenses](../research/pop-storage-and-lenses.md))
- How do Songs of Syx and Dwarf Fortress actually lay out their data? Not readable yet. ([low-resource-city-simulation](../research/low-resource-city-simulation.md))
- Should a stored region-scale id set be over founder slots, record slots or 64-bit PopId (roaring64)? Is CRoaring's portable serialisation byte-identical for the same set whatever its history, as the save hash needs? ([ecs-memory-layout](../research/ecs-memory-layout.md))
- Saves (spec 06): which zstd release to vendor, and does adding it need a decision beside 0002? ([data-compression](../research/data-compression.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** vendor zstd (a pinned release); recorded in 0055, no separate decision.
- Should save format version 4 add the codec, the uncompressed size and spec 06 rule 4's fields (binary hash, data hash, seed, settings) all at once? ([data-compression](../research/data-compression.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** yes, all at once; the version 3 reader stays.
- How often to write a full save in the autosave and rewind history (one per game day proposed, deltas between)? ([data-compression](../research/data-compression.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** one per game day, deltas between.
- Pop records (spec 28): regenerate founder specs from the generator, or store them as compressed columns? Switch the death-year and effect-year doubles to fixed point? ([data-compression](../research/data-compression.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** regenerate; fixed point when the record format next changes.
- Chronicle (spec 25): after old events are folded into sums, keep the full log compressed in the save or drop it? ([data-compression](../research/data-compression.md))
- Is a trained dictionary per blob kind worth the upkeep for a 16% gain? ([data-compression](../research/data-compression.md)) **Default in [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md):** no, not for now.
- From [borrowed-from-other-fields](../research/borrowed-from-other-fields.md) §4: a minute level at 60 s between hot and hourly warm, or event coupling (XD-116); the hot set's regrid interval, which sets the buffer width (XD-118).

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [detail-levels](../research/detail-levels.md): Detail levels: one simulation, read and run in chunks
- [haven-and-hearth](../research/haven-and-hearth.md): Haven and Hearth: a reference game for long real-time play
- [layer-jumps-multiresolution](../research/layer-jumps-multiresolution.md): Layer jumps: what multi-resolution simulation research and games say
- [low-resource-city-simulation](../research/low-resource-city-simulation.md): Low-resource city simulation: "ASCII City" and the games that fake or tame big populations
- [pop-storage-and-lenses](../research/pop-storage-and-lenses.md): Pop simulation, saving data, and "lenses" across zoom
- [ecs-memory-layout](../research/ecs-memory-layout.md): Entity storage and memory layout: archetypes, sparse sets, id sets, profiling
- [data-compression](../research/data-compression.md): Data compression for saves, replays, traces and pop records
- [perception-and-run-ahead](../research/perception-and-run-ahead.md): Perception-driven detail and the simulation running ahead of the view
- [borrowed-from-other-fields](../research/borrowed-from-other-fields.md): Borrowed from other fields: nested models, telemetry, tracking and paging
