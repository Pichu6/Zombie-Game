# 34 · Projects (construction)

**Status:** Implemented and tested, headless (the library, and the wiring in the economy world system). **Stage:** not Tier 0; the [layer jumps](../../design/layer-jumps.md) audit row 9 (construction and projects). **Needs:** [32](32-stocks-goods-and-money.md), [33](33-establishments-and-chains.md). Design: [world simulation](../../design/world-simulation.md) §7 (build orders), [focus and compression](../../design/focus-and-compression.md) §4 (the hospital through three layers) and §9 (a Process). Decisions: [0033](../../decisions/0033-restoring-cost-and-two-way-projects.md), [0039](../../decisions/0039-fill-the-layer-jumps.md), [0049](../../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md).

## Purpose

A programme and a budget at Macro, a site with materials and a crew at City, hauling and building at Individual: one project. This spec makes a project the **Process** of [focus and compression](../../design/focus-and-compression.md) §9 out of parts that already exist: a plan fixed at order time, money in escrow on the ledger, work by stages run by the site's establishment with the plan's recipe ([33](33-establishments-and-chains.md)), a contractor paid per stage, and one finished building that is a unit of a good ([32](32-stocks-goods-and-money.md)), so the Macro building stock is a ledger sum.

## What the code forces

- **Owners are fixed at setup** ([29](29-embodied-pops.md)). A project cannot create its site: it is ordered **on an existing site**, a Building-scope owner with an establishment (a plot reserved at setup, as body slots are). The finished building does not become a new owner either; the site is the building.
- **A recipe makes one output per batch at a declared rate** ([33](33-establishments-and-chains.md)). So the building good's **bulk is stages** and its **unit is the finished building**: the plan takes `bulk_per_unit` stages, each stage one batch of the plan's recipe (so much concrete and steel, so many worker-hours), and finishing is `materialise(site, building, 1)`: the stages become the building by the same declared conversion a visit uses.
- **Production jumps exactly** ([33](33-establishments-and-chains.md) rule 4). A project adds only a **batch cap** (stages left, money left) before each advance, which keeps the jump exact: a stage is never started that cannot be paid, and labour that waited for money or materials is lost as in any establishment.

## State

| Field | Notes |
| --- | --- |
| `PlanDef` | name; `stages` (the construction recipe); `building` (the good whose unit is the finished building); `budget_cents`; `cost_per_stage_cents` (the contractor's price); `contractor` (an owner); `then` (the recipe the finished building runs, optional) |
| `Project` | `site`; `plan`; `orderer`; `ordered_tick`; `finished_tick`; `produced_at_order` and `stages_paid` (for the contractor's pay); `produced_seen`, `caught_up_seen` (for stall detection); `state` (Active, Finished, Cancelled); `stall` (None, NoStaff, NoPower, NoMoney, NoMaterials) |
| `Establishment::batch_cap` | added to [33](33-establishments-and-chains.md): the most batches the next advance may run (−1 = no cap) |

## Rules

1. **Order.** `ProjectOrder` (subject = site, plan, orderer): the site must have an establishment and no active project, and the orderer must hold the whole budget; otherwise refused (`NoEstablishment`, `SiteBusy`, `NoBudget`). The budget moves from the orderer to the site's account as a `Grant` (escrow: money is conserved, the state's panel shows it gone, the site's shows it held, [0051](../../decisions/0051-money-and-resources-shown-follow-the-selection.md)). The site's establishment takes the plan's recipe with an empty labour bank. Fixed at order time: the plan, so every later view matches it.
2. **Materials arrive by shipment** (`EconShip`, [32](32-stocks-goods-and-money.md)) into the site's account, from anyone: a depot, a company, a pop. The project does not fetch them; the material request to the supply chain ([world simulation](../../design/world-simulation.md) §7) is the sender's business.
3. **Work is the establishment's.** Staff comes from the site's roster or `EstabSetStaff`, power from `EstabSetPower` ([33](33-establishments-and-chains.md) rules 3 and 5). Before each advance the project sets `batch_cap = min(stages left, money left ÷ cost per stage)`; the establishment then runs stages as its labour and the site's materials allow.
4. **Pay per stage.** After each advance the contractor is paid `cost_per_stage` for every stage built since the last payment, as a `Wage` flow from the site's escrow. The cap of rule 3 guarantees the money is there.
5. **Finish.** When the site holds `bulk_per_unit` stages, one unit of the building is materialised, the project is Finished at that tick, the establishment's bank is cleared and it takes the plan's `then` recipe (with no cap) or stands idle (cap 0). Escrow left stays on the site; materials to spare stay too. "The finished facility posts its own jobs" is the site's roster from then on ([33](33-establishments-and-chains.md) rule 3).
6. **Stalls are judged on the end state** of each advance, so a jump and stepping agree: the project is stalled if it could not start another stage now, in this order: no staff, not enough power, escrow below one stage's cost, no materials for one stage. A labour bank short of a stage is not a stall. A change of stall state emits `ProjectStalled` (with the reason) or `ProjectResumed`; the inspector reads the current reason from the project.
7. **Cancel.** `ProjectCancel` returns the escrow left to the orderer (`Grant`), marks the project Cancelled and caps the site at 0. Materials and stages built stay on the site: a half-built shell is real.
8. **View.** `Projects::view(site)` is a lens ([focus and compression](../../design/focus-and-compression.md) §8): state, stages done and total, `progress = done ÷ total`, the stall reason, money left. It stores nothing.
9. **Macro.** The building stock under an owner is `aggregate(owner, building unit)` ([32](32-stocks-goods-and-money.md) rule 2); the programme's spend is the sum of `Grant` flows out of the state; nothing is kept apart.
10. **Save.** Projects are saved after the establishments (plans are setup). `batch_cap` is saved with the establishment.

## Invariants

- Money total is constant through order, stages, finish and cancel; `escrow + paid + returned = budget` for every project.
- `stages built × materials per stage` equals the materials that left the site's stock by the recipe; the rest is still there.
- A Finished project's site holds exactly one more unit of the building and no stages.
- Balances, banks and the stall state after any partition of an interval equal the single jump's.

## Tests (`test_project.cpp`)

- **Order and cancel (A1):** no site, no budget, busy site refused; escrow moves in full; cancel returns the remainder and a second cancel is refused.
- **Stages, stalls, finish (A1):** 20-stage clinic, a stage per crew-day: stalled for staff, then for materials; materials for five stages give five days of work and a stall at the end of the jump with nothing new to report a day later; the rest arrives with spare; finished exactly at 20 stages on the jump's tick; the unit exists, no stages remain, the spare concrete stays, the contractor has 20 stages' pay and the escrow the rest; money total unchanged; the clinic then runs its own recipe.
- **Money runs out, jump equals stepping (A1):** escrow for 7 stages, materials for 30, labour for 20: both paths build 7, hold identical balances and banks, and end stalled for money.
- **In the world (A1):** `ProjectOrder` then a refused second order (`SiteBusy`); events in order: Ordered, Stalled (materials), Resumed, …, Finished; the builder paid, money conserved, the country's building stock is 1 by enumeration; save, load, save identical; replay and a mid-run resume reproduce the hash.

## As built

- `engine/sim/include/zs/project.h`, `src/project.cpp`: `PlanDef`, `Project`, `Projects` (`add_plan`, `order`, `cancel`, `before_advance`, `after_advance`, `view`, save and load).
- `EconomySystem`: `projects()`; commands `ProjectOrder` (0x2009), `ProjectCancel` (0x200a); refusals `UnknownPlan`, `SiteBusy`, `NoProject`, `NoBudget`; events `ProjectOrdered` (51), `ProjectFinished` (52), `ProjectStalled` (53), `ProjectResumed` (54), `ProjectCancelled` (55). The catch-up runs `before_advance`, the establishments, then `after_advance`.

## Calibration

| Value | Source |
| --- | --- |
| Bills of materials and labour per stage | Templates per building type ([world simulation](../../design/world-simulation.md) §8: plant, hospital, water works…), from the quality system; placeholders in the tests |
| Build durations | Real ([0049](../../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md)); years for a hospital, set by labour per stage and the crew |

## Not in this spec

Job postings that pops answer on their own (the roster is set by setup or by roster commands; a labour market is later), material requests placed by the project with the supply chain, hazards over the interval (accident, blackout, strike as keyed draws), a site that is also a walkable interior (its layout), the planner that picks sites from a Macro programme, and the two-way restoration of capabilities ([0033](../../decisions/0033-restoring-cost-and-two-way-projects.md): the tech readout rising when a plant is finished).

## Open

1. **Partial progress at Individual.** A stage is the unit of progress; inside a stage the crew is "working" with no finer state. Recommendation: fine for City and Macro; Individual can draw the stage's work as a loop ([focus and compression](../../design/focus-and-compression.md) §9) until the interior is built.
2. **Escrow left over.** It stays on the site. Recommendation: the finished facility's first operating budget; or return it on finish. Decide when facilities have running costs.
