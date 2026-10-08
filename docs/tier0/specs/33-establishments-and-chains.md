# 33 · Establishments and chains

**Status:** Implemented and tested, headless (the library, and the wiring in the economy world system). **Stage:** not Tier 0; the [layer jumps](../../design/layer-jumps.md) audit rows 7 (jobs and establishments) and 8 (processing chains). **Needs:** [03](03-stock-transfer.md), [27](27-detail-levels.md) rule 2, [28](28-pop-levels.md) (rosters), [32](32-stocks-goods-and-money.md). Design: [world systems](../../design/world-systems.md) §6 and §10.1. Decision: [0039](../../decisions/0039-fill-the-layer-jumps.md).

## Purpose

A sector's output at Macro, a plant with its staff and inputs at City, and a worker running a process at Individual are one production. This spec gives the middle term, the **establishment**: one owner on the ledger ([32](32-stocks-goods-and-money.md)) running one **recipe** with the staff its workplace roster has ([28](28-pop-levels.md)), so output above is a sum over establishments, a layoff above becomes less output below, and a process the player runs by hand is the same recipe a plant runs.

## What the code forces

- **Conversions are one-to-one** on the ledger ([03](03-stock-transfer.md) rule 4). A recipe therefore has one **primary input** converted into one **output** by a declared conversion, and **secondary inputs** that are transfers to the `world.consumed` sink. Mass is not conserved through processing in the world either (water is lost, husks are waste); what the ledger conserves is that every gram either converts by a declared rate or goes to a named sink.
- **Rates must jump exactly** ([27](27-detail-levels.md) rule 2). Production is written as labour accrued over an interval and spent in whole batches, with one rule for lost labour, so any partition of time gives the same balances.
- **Rosters are the ledger of "works here"** ([28](28-pop-levels.md)). Staff is therefore read from the roster, never stored apart from it.

## State

| Field | Notes |
| --- | --- |
| `RecipeDef` | name; primary input good and amount per batch; output good and amount per batch; secondary inputs (good, amount per batch); `labour_per_batch` in worker-ticks; `power_w` needed while running (0 = none); the declared conversion id |
| `Establishment` | `owner` (a Building-scope owner: its inputs, output and money are its account); `recipe`; `roster` (the workplace roster whose present members are its staff, 0 = staff set by command); `staff`; `power_w` available; `labour_bank` (worker-ticks accrued and not yet spent); `caught_up`; `produced` (batches, ever) |
| Sinks | `world.consumed` (secondary inputs), beside `world.decayed` of [32](32-stocks-goods-and-money.md) |

## Rules

1. **A recipe is data, declared once.** `Recipes::add` declares the primary conversion on the ledger. `affordable(owner)` is the whole batches the owner's own stock pays for (the minimum over the primary and every secondary input). `run(owner, batches)` runs up to that many on the owner's account: secondary inputs to the sink, the primary converted, all in one tick with one cause. Labour is the caller's: an establishment's bank, or an action's duration for a pair of hands.
2. **By hand is the same recipe.** `EconRunByHand` (subject = a pop's owner, recipe, max batches) calls the same `run`. The action executor's job (later) is to supply the labour time and the tools; the amounts are never written twice.
3. **Staff is the roster.** An establishment with a roster has `staff = present − laid off − ill` at the current year, read from the pop system at the end of setup and at every hourly update. `EstabSetStaff` is refused for it (`StaffFromRoster`): a layoff is a roster event ([28](28-pop-levels.md)), as the design wants ([world systems](../../design/world-systems.md) §6, "an edict changes establishments' staffing"). An establishment without a roster takes its staff from `EstabSetStaff`.
4. **Production jump.** `advance(to_tick)` for each establishment with `dt = to_tick − caught_up > 0`: if powered (`power_w` available ≥ the recipe's, or the recipe needs none) and `staff > 0`, `labour_bank += staff × dt`; `n = min(labour_bank ÷ labour_per_batch, affordable)`; run `n` batches, `labour_bank −= n × labour_per_batch`, `produced += n`; then, if `labour_bank ≥ labour_per_batch` (labour that found no inputs, power or staff), `labour_bank %= labour_per_batch`: **idle hours are not banked**. Because the bank is integer and the loss rule depends only on the running total, stepping every tick and one jump give the same balances and the same bank (A1), including when inputs run out midway.
5. **Power is a gate** in this slice: below the recipe's watts the establishment stands idle. The fractional law `min(1, energy available ÷ needed)` of [world systems](../../design/world-systems.md) §10.1 is for the cold sector form and is not written here, because a fraction applied per step would not jump exactly. `EstabSetPower` sets the watts available; the link to the power network ([17](17-power-network.md)) is later.
6. **Chains are shipments.** A chain is recipes whose output good is another's input, fed by `EconShip` ([32](32-stocks-goods-and-money.md) rule 9). Throughput is each stage's production given what has reached it; nothing is computed for the chain as a whole.
7. **Macro is a sum.** `produced_under(root, recipe?)` is the batches produced by the establishments under an owner in the tree (a region's milling, a country's everything). It is computed from the establishments, never stored.
8. **In the world.** The economy system catches every establishment up (rule 4) at the start of any economy command, so a shipment, a staff change or a power change never applies to time already elapsed, and once a game hour in `update` (after spoilage, after re-reading staff). Each production emits `EconProduced` (subject = the owner, a = recipe, b = batches). Staff is read hourly: a roster event lands at the next hour.
9. **Save.** The establishments' state (owner, recipe, roster, staff, power, bank, caught-up tick, produced) is saved after the lots and tallies; recipes are setup. Worlds with no establishments save an empty list.

## Invariants

- The ledger check passes every tick; what leaves an account leaves by a declared conversion or to a named sink.
- `produced × output_per_batch` equals the output ever converted for that establishment.
- `labour_bank < labour_per_batch` after every advance, unless inputs allowed every batch the labour paid for (then it is the remainder either way).
- Balances and banks after any partition of an interval equal the single jump's.

## Tests (`test_establishment.cpp`, `test_establishment_system.cpp`)

- **One account (A1):** 3 batches of milling move 300 kg grain to 285 kg flour and 3 kWh to the sink; by hand the same recipe is capped by the scarcer input.
- **Jump equals stepping (A1):** 6 hours stepped every tick against one jump, with grain for 4 of the 9 batches the labour could pay: identical balances, bank and count. Property: random partitions and any staff agree with one jump.
- **Gates (A1):** under the recipe's watts, or with no staff, nothing is produced and nothing banked; once able, a day's work with grain for 10 batches makes 10 and keeps no stored hours.
- **Macro and chain (A1):** two mills' output sums under the region; a bakery fed hourly by shipments bakes what has reached it, never more; `produced_under` a leaf with no establishment is 0.
- **In the world (A1):** six millers on a workplace roster run the mill at 3 batches an hour; a `RosterApply` layoff of three lands at the next hour and halves it (142 batches over three days, bank empty); a bakery waits for power, then bakes its 40 shipped batches; `EstabSetStaff` on the mill is refused as `StaffFromRoster`; a pop bakes by hand; the Macro sums match; save, load, save are identical bytes; replaying the input log and resuming from a mid-run save reproduce the hash.

## As built

- `engine/sim/include/zs/establishment.h`, `src/establishment.cpp`: `RecipeDef`, `Recipes`, `Establishment`, `Establishments`. `Owners::under` was added to [32](32-stocks-goods-and-money.md)'s tree for the Macro sum.
- `EconomySystem` ([32](32-stocks-goods-and-money.md)): `recipes()`, `establishments()`, `consumed_sink()`, `attach_pops()`, `start_establishments()`; commands `EstabSetStaff` (0x2006), `EstabSetPower` (0x2007), `EconRunByHand` (0x2008); refusals `NoEstablishment`, `UnknownRecipe`, `StaffFromRoster`; event `EconProduced` (50).

## Calibration

| Value | Source |
| --- | --- |
| Recipes (inputs, energy and labour per tonne) | [world systems](../../design/world-systems.md) §10.1 anchors (steel 10–24 GJ/t, milling 60–100 kWh/t); labour per tonne to be derived from the labour share of value added. The tests use round placeholders |
| Capital and the production function | Not here: `Y = A K^a L^(1−a)` is the cold sector form, to be fitted to sums of establishments |

## Not in this spec

Construction and projects (row 9: a project is an establishment whose output is a finished building, with posted jobs; next), hours and shifts (staff counts for the whole interval), skill and quality of output ([15](15-items-recipes.md) Q), machines as assemblies with condition, the power network link, wages paid by the establishment on a schedule (a `pay` the scheduler will issue), prices and sales (a shop is an establishment with a `purchase` counter), and the fractional energy law.

## Open

1. **Stored labour across input gaps.** Rule 4 loses idle hours. The alternative, banking them so a late delivery is caught up at once, is wrong for workers but right for machines that queue work. Recommendation: keep the loss for staff; a machine queue is a later state of its own.
2. **Staff read hourly.** A roster event lands at the next hour, not at its tick. Exact alternative: the pop system notifies the economy on roster changes so the establishment catches up at that tick. Recommendation: do it when the cost of the hour's inexactness shows in a test or in play.
