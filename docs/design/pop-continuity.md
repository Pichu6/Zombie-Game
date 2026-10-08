# One pop at every layer

A pop is the same person whether you see a count in Macro, a marker in City or a body in Individual. This doc says how, and how loss and gain travel between layers. It extends [world simulation](world-simulation.md) §3 and §8, [27 · detail levels](../tier0/specs/27-detail-levels.md) (places) and [inhabitants](inhabitants.md). Decided intent: [0029](../decisions/0029-leader-records-foxhole-tech-readout.md), [0030](../decisions/0030-pop-continuity-and-capability-loss.md). The model is a **Proposal**; none of it is built. Needed before any nation or office code ([sandbox parameters](sandbox-parameters.md) §5, §7).

Spec: [28 · Pop identity and levels](../tier0/specs/28-pop-levels.md) (the technique, rules, tests and the benchmark plan).

## 1. The rule

**Identity is fixed; detail varies.** A pop has one id for life. What changes between layers is how much of them is stored and simulated, never who they are, what they have done, or what is true about them.

## 2. Three levels, one pop

| Level | What is stored | Who | How state is known |
| --- | --- | --- | --- |
| **Lazy** | Nothing per pop | Most of the census | Computed on demand from the seed, the region's daily series and the events recorded for their household or workplace |
| **Record** | Compact state: id, age, job, household, ties, beliefs, wounds, items, office, plans | Anyone touched, and always: **officeholders, followed pops, pops the player has met or possessed** (Decided) | Updated by life-event jumps; fully exact |
| **Embodied** | Full body, perception, actions | People near a player-controlled body | Ticked |

- A lazy pop is **not** a different or "fake" pop: the same pure function that makes a record makes them ([world simulation](world-simulation.md) §3), so promoting gives exactly the person the lazy view implied.
- Looking is a pure read. Promotion to record happens by a stated rule (office, follow, meeting, employment by something the player runs, a story tie), never by the camera.
- Nobody is demoted below record while they hold an office or are followed. Others may go back to lazy only if their record equals what the seed and the region series would give, plus a small saved list of exceptions (so nothing the pop did is lost).

## 3. Making the aggregate agree

Macro counts are never a separate stored number. A region's counts (by age, job, wealth, health, loyalty) are **sums over its pops**, where lazy pops contribute through the region's series and records contribute their own state. Three consequences:

1. **Conservation:** people, jobs, money and stock move through the ledger; promotion, demotion and jumps leave every total unchanged (contract rule 5 of spec 27).
2. **Deterministic choice of who:** when an aggregate event hits a group (a shop closes and 40 of its 120 workers are laid off), the affected pops are picked by a **fixed ranking from their seed and the rule** (seniority, skill, an edict's order), so later promoting any of the 120 gives an answer consistent with the count. No one is chosen "when someone looks".
3. **Jump agreement:** a life-event jump (age, births, marriages, deaths, job moves) from tick A to B gives the same distribution as stepping, using the same rates; tested against many seeded step runs, and exactly equal for rules that are deterministic.

## 4. Births, deaths and ids

- **Id:** invertible and deterministic: the founder's slot plus the path of child numbers (7 generations, 15 children each), so a lazy birth can be found again from its id alone, promoted later, and always be the same child ([spec 28](../tier0/specs/28-pop-levels.md)). Limit: about 150 years of generations until the format is widened.
- **Birth** is a conserved event ([sandbox parameters](sandbox-parameters.md) §7.4); **death** removes the pop from offices, households and jobs and writes one record (cause, date) kept for the chronicle. The dead are never regenerated.
- **Templates and imports** ([sandbox parameters](sandbox-parameters.md) §7.3) create a pop once, with a new id, at world creation.

## 5. Loss and gain travel both ways (Decided as intent)

| Down (decision above, effect below) | Up (change below, effect above) |
| --- | --- |
| A nation can't sustain a service or tech: its establishments lose staff, parts or power, and close | A shop closes: its workers become unemployed pops, with real households losing income |
| A law or budget change: jobs and stocks move | A hospital lacks workers and shuts: the region's health capacity falls, the country panel shows it |
| A lost capability: an item (some ammunition) stops being made, and is not found in the area | A factory is disabled: the nation's tech readout loses that node |

Every change carries its reason, as already planned for the ledger ([macro layer](macro-layer.md) §5), so the inspector can answer "why".

**Lost capabilities.** A capability is lost when the establishments, parts or people that make it are gone, never by a timer. Knowledge held by living people can be lost with them (and with their notes), not "forgotten" by drift (Decided, [0030](../decisions/0030-pop-continuity-and-capability-loss.md)). A lost capability **can** be restored but only with great effort (Decided, [0031](../decisions/0031-restoring-capabilities-real-time-playable-age.md)): the AI never chooses it on its own; it is a human goal or late-game play that needs trade, cooperation and real time. The cost is made of resources, pops and time, for real reasons ([0033](../decisions/0033-restoring-cost-and-two-way-projects.md)); the model is in §5.1.

### 5.1 Restoring a capability: a project, from either end (Decided as intent, model Proposal)

A restoration is a **Project** ([world simulation](world-simulation.md) §7), the same record whoever starts it. It needs, as real reasons for a real project:

| Cost | What it means |
| --- | --- |
| **Resources** | Parts, materials, tools, machinery, energy and money that exist in the ledger and move to the site; if a part is no longer made anywhere, making it is itself a project (the production ladder, [0013](../decisions/0013-production-ladder.md)) or it must be imported through trade |
| **Pops** | Specific people with the know-how (found, hired, trained, or brought by partners), for the hours the work needs; people who hold the knowledge may be few, old or dead (Decided: knowledge is lost with people and notes) |
| **Time** | Real durations ([0031](../decisions/0031-restoring-capabilities-real-time-playable-age.md)): months to years for a plant, a school year for a cohort of trainees; it cannot be skipped by money alone |
| **Partners** | Other nations, companies or groups for parts, trade, training and security; the more that is missing, the more partners it takes |
| **Upkeep** | The restored thing needs staff, power and inputs to stay restored, or the capability is lost again |

**Two ways to drive it** (the same Project, seen from either layer):

- **Micro → Macro (bottom up):** a possessed pop or a group finds the parts, repairs the machines, trains the staff and brings the plant back. The work is real actions at Layer 1 and jobs at Layer 2; when it is done the nation's tech readout (Layer 3) rises by itself, because the establishment and people now exist ([sandbox parameters](sandbox-parameters.md) §11.1).
- **Macro → Micro (top down):** an actor at Layer 3 (a government, a ministry, a company board) opens the programme with a budget and a goal. It becomes a site and job postings at Layer 2; pops and companies take them by their own rules ([world simulation](world-simulation.md) §7, §13); a possessed pop can take one of those jobs. If nobody can or will do it, the programme stalls and says why.
- **Mixed:** both can run on one Project: a nation funds it, a player's group does the hard part.
- **AI:** pops and AI governments do not start the hard restorations on their own (Decided); they do ordinary repairs and maintenance.

### 5.2 Groups that change (Proposal, built in spec 28)

Generated cohorts never change (their aggregate events use the permutation rank). Anything whose members come and go is a **roster**, an explicit list of ids that is the ledger's owner of the relationship ("works here", "lives here", "holds this office"). An aggregate event on a roster picks its members once and stores them, so Macro's count and every pop's own answer are the same list. See [spec 28](../tier0/specs/28-pop-levels.md) rule 11.

## 6. Invariants and tests (to write when specified)

- Total people, jobs, money and stock are equal before and after promotion, demotion and jump (agreement A1).
- A pop's id, history and office are unchanged across any level change.
- Aggregate event vs per-pop result: the count affected equals the number of promoted pops that report being affected, for any chosen subset.
- A save with a region lazy and the same save with it embodied replay to the same hash once both are embodied.
- An officeholder or followed pop is never lazy.
- Benchmark: records and embodied counts per frame budget; the number of records is bounded by the rule in §2, so the benchmark sets how many followed pops a player can have.

## 7. Open

1. **Budget:** how many records the sim can hold and update on a normal PC, and how a Macro run at "years per second" drives lazy jumps. Needs a benchmark. Other games' evidence and a first estimated budget (50–300 embodied, 10,000–100,000 records, lazy unbounded) are in [research/sim-scale-in-other-games.md](../research/sim-scale-in-other-games.md); the figures are estimates and must be measured with `zs --bench`.
2. **Selection rules for layoffs and similar aggregate events** per system (seniority, skill, random draw): each system's spec states its ranking.
3. **Numbers for the restoring cost** (§5.1): per capability, which parts, which skills, how many person-hours, which partners. Each comes from its own spec.
4. **Age gate:** a pop younger than the playable age is a record the player can follow but not possess ([0031](../decisions/0031-restoring-capabilities-real-time-playable-age.md)).

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- When does a record collapse back to lazy (seed only), and what is kept after death? ([sim-scale-in-other-games](../research/sim-scale-in-other-games.md))
- Measure the record tier: first guesses are 50-300 embodied, 10,000-100,000 records, a few hundred bytes to 1 KB each. ([sim-scale-in-other-games](../research/sim-scale-in-other-games.md))
- How often do record-level pops update: on events, or a few times per game day? ([sim-scale-in-other-games](../research/sim-scale-in-other-games.md))

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [detail-levels](../research/detail-levels.md): Detail levels: one simulation, read and run in chunks
- [layer-jumps-multiresolution](../research/layer-jumps-multiresolution.md): Layer jumps: what multi-resolution simulation research and games say
- [sim-scale-in-other-games](../research/sim-scale-in-other-games.md): Simulation scale in other games
