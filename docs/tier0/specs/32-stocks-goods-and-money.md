# 32 · Stocks, goods and money across levels

**Status:** Implemented and tested, headless (the library, and the world system with commands, hourly spoilage, saves and replay). **Stage:** not Tier 0; the [layer jumps](../../design/layer-jumps.md) audit rows 5 (stocks and goods) and 6 (money). **Needs:** [03](03-stock-transfer.md), [15](15-items-recipes.md) rule 4, [27](27-detail-levels.md) rule 2. **Decisions:** [0039](../../decisions/0039-fill-the-layer-jumps.md), [0044](../../decisions/0044-information-scope-per-layer.md), [0051](../../decisions/0051-money-and-resources-shown-follow-the-selection.md).

## Purpose

A province's stock of food, a shop's shelves and the sack in a pop's hands are the same grain. A nation's budget, a company's account and a wallet are the same money. This spec makes them one thing on the ledger with many owners, so Macro, City and Individual read the same amounts and nothing is created, lost or counted twice between them. It also fixes whose account each layer's panel shows ([0051](../../decisions/0051-money-and-resources-shown-follow-the-selection.md)).

## What the code forces

- **The ledger already conserves integers held by one owner** ([03](03-stock-transfer.md)); owners and resources are fixed at setup; conversions are declared pairs with integer batches. Everything here is built on those three facts, with no new primitive.
- **Instances cannot be created mid-run** ([29](29-embodied-pops.md), "the entity set is fixed after setup"). So a visit does not create item instances; it makes **units** (a stock of sacks or cans with no identity, [15](15-items-recipes.md) rule 4) from bulk by a declared conversion, and leaving turns them back. The same mass, two shapes.
- **Rates must jump exactly** ([27](27-detail-levels.md) rule 2). Spoilage is written as a closed form of (start amount, elapsed time), so stepping and a jump move the same grams.

## State

| Field | Notes |
| --- | --- |
| `Owners` | the owner tree: each ledger owner that holds goods or money has a `Scope` (Sink, Country, Region, Building, Pop) and a parent. A pop's parent is the building it lives or works in; a building's is its region; a region's is its country. Sinks and countries are roots. |
| `GoodDef` | one good: a bulk resource (g, mL, a count), a unit resource (sack, can, crate), `bulk_per_unit`, the two declared conversions (pack, unpack), and a half-life in days (0 = does not spoil) |
| `Spoilage::Lot` | an owner's bulk of one good as it stood at `start_tick`, and the tick it is caught up to |
| `Money` | the resource `money_cents` plus tallies of what each owner received and paid, by `FlowKind` (Sale, Wage, Tax, Grant, Other) |
| `Context` | for the panel rule: the played pop, the region the player is at, the country, and the City selection if any |
| `EconomySystem` | the world system: owns the tree, the goods, the money and the lots; applies the commands below; saves lots and tallies (definitions are rebuilt by setup, as the pop pool is) |

## Rules

1. **One resource, many owners.** Every level is an owner on the same ledger. There is no "province stock" apart from what the province's owners hold. Moving goods or money between scopes is a transfer with a cause; nothing else moves them ([0044](../../decisions/0044-information-scope-per-layer.md)).
2. **Enumeration.** An owner's **aggregate** of a resource is its own account plus every descendant's. A region's grain is what its buildings and people hold plus what lies loose in the region's own account; a country's is its regions'. `by_children` gives the same total split one level down (one row per direct child, one for the owner's own account). This is how Macro reads City and City reads Individual; it is a sum, never a stored number.
3. **Goods: bulk at a distance, units up close.** `materialise(owner, good, max_units)` packs whole units from the owner's bulk (clamped to what it holds and to the cap); the remainder stays bulk. `dematerialise` returns every unit to bulk. Both are the ledger's declared conversion, so mass is conserved by construction and the round trip is the identity.
4. **Spoilage with an exact jump.** `remaining(start, half_life, elapsed) = ceil(start · 2^(−elapsed / half_life))`, with the deterministic `detmath::exp`. `advance(to_tick)` moves `remaining(caught_up) − remaining(to_tick)` of each lot to the decay sink and marks the lot caught up. Because every step reads the same curve from the same origin, stepping every tick, every hour or once gives identical balances (telescoping). A transfer of the bulk resets the lot (the caller's duty; the rule is written once, here).
5. **Money is a resource.** Minted at setup like any other, held by owners at every scope. `purchase(buyer, seller, good, price, want)` moves `n = min(want, seller's units, buyer's money ÷ price)` units one way and `n · price` cents the other, in one tick with one cause: all or nothing per unit, no debt, no negative wallet. `pay(from, to, cents, kind)` is a clamped transfer for wages, taxes and grants.
6. **Aggregates are sums of transfers.** Revenue, wages paid and tax taken are the tallies by `FlowKind`; GDP-like figures are sums of Sale flows. For every owner, `received − paid` equals its change of balance. No figure is stored that a sum of entries does not reproduce.
7. **Whose account a panel shows** ([0051](../../decisions/0051-money-and-resources-shown-follow-the-selection.md)). `shown_owner(layer, ctx)`: Individual → the played pop; City → the selected pop or building, else the region; Macro → the country. A selected region or country is not a City selection. `panel()` reads that owner's **own account** at Individual and City, and the country's **aggregate** at Macro with `money_by_part` split by region. A building's panel therefore never includes the region's loose stock, and the region's never includes the country's.
8. **Views only read.** A selection or a layer change never moves anything ([27](27-detail-levels.md) rule 7).
9. **Commands** (`EconCommand`, 0x2001–0x2005), through the world's command path like every pop change, so they are replayable and saved: `EconVisit` (owner, good, max units), `EconLeave` (owner, good), `EconPurchase` (buyer; seller; good, want and price packed), `EconPay` (payer; receiver; kind and cents), `EconShip` (from; to; good and bulk amount: the one way goods cross scopes, [0044](../../decisions/0044-information-scope-per-layer.md)). A command aimed at an owner outside the tree, at a sink, at an unknown good, or that would move nothing, is refused with an `EconRefused` event carrying an `EconRefusal` code and changes nothing.
10. **Spoilage in the world.** Lots start at the end of setup (`start_lots`). The system advances every lot once a game hour and emits `EconSpoiled` with the grams moved; a visit, a leave or a shipment of a spoiling good catches its lots up first, then resets them to what the owners hold. Units on a shelf do not spoil in this slice (a bottle's own half-life is a later rule).

## Invariants

- Per resource, the sum over all owners and sinks is constant, up to recorded conversions (the ledger's check, every tick in tests).
- `aggregate(country)` is constant under any transfers among the country's owners.
- A visit and a leave return the owner to the same bulk, with zero units.
- Spoilage balances after any partition of an interval equal the single jump's; the stock never rises.
- Money total is constant: no hidden mint. Tallies reconcile with balances for every owner.

## Tests (`engine/sim/tests/test_economy.cpp`, agreement levels)

- **Enumeration (A1):** aggregate equals own plus descendants on a fixed fixture; `by_children` sums to the aggregate; property: random transfers among the country's owners keep the country's aggregate.
- **Visit round trip (A1):** 1,234,567 g makes 24 sacks and leaves 34,567 g; leaving restores the gram; property over any amount and cap.
- **Spoilage (A1):** 5 days of hourly steps equal one 5-day jump to the gram, and the result sits in the expected band (2^(−5/3) of 20 L); property: random partitions agree with the single jump and the stock is monotone; closed-form edge cases (no half-life, exactly one half-life, zero, the last unit).
- **Purchase (A1):** goods and money cross in one step, capped by stock and by money; a shop with nothing sells nothing; a buyer with too little buys nothing.
- **No hidden mint (A1):** random purchases, wages and taxes keep the money total and the unit total; every owner's tallies reconcile with its balance.
- **Determinism (A1):** the same sequence twice gives identical balances.
- **In the world (`test_economy_system.cpp`, A1):** a four-day script (shipments, shelving, a wage, purchases, taxes up two scopes, packing away, one refused purchase) ends with the expected balances, tallies and event counts, grain and money totals unchanged and spoiled milk only in the sink; five refusals name their reason and leave balances untouched; save, load, save gives identical bytes and the hash covers lots and tallies; replaying the input log reproduces the hash; loading mid-run and continuing matches never having stopped; the 0051 panels read from the world.
- **0051 (A1):** `shown_owner` for every layer with no selection, a building, a pop, a region and the country; a building's panel shows 100,000 g and its own money, the region's shows its 7 loose grams, the pop's its wallet, the Macro panel the country's aggregate with rows that sum to it.

## As built

- `engine/sim/include/zs/economy.h`, `engine/sim/src/economy.cpp`: `Owners`, `Goods`, `Spoilage`, `Money`, `shown_owner`, `panel`. Built on `Ledger` alone, with no `World`, so a test runs in microseconds.
- `engine/sim/include/zs/economy_system.h`, `economy_system.cpp`: `EconomySystem`, the commands and their argument packers (`econ_purchase_arg`, `econ_pay_arg`, `econ_ship_arg`), events `EconTraded` (46), `EconPaid` (47), `EconSpoiled` (48), `EconRefused` (49). The system's save section holds the lots and the money tallies only; worlds without the system save byte for byte as before.
- The owner tree is kept beside the ledger (a map by `OwnerId`), so body-slot owners and other owners that hold no goods may stay out of it.
- Half-lives and `bulk_per_unit` are passed in by the setup that adds the good; the data file for goods ([15](15-items-recipes.md) materials and items) is the next place for them.

## Calibration

| Value | Source |
| --- | --- |
| Units per good (50 kg sack, 1 L can) | Convention; to come from the items data |
| Half-lives | Placeholders; fresh food spoilage by temperature is Tier 0's fridge rule and should be the same curve |
| Prices | A market state per sector, damped adjustment ([world systems](../../design/world-systems.md) §10.1); here a price is an argument |

## Not in this spec

Goods data in `items.toml` (units and half-lives are still setup arguments); spoilage of units on a shelf; establishments and recipes (the next jump, rows 7–9); prices that move; credit and debt; the central bank's mint as a declared inflow; the display itself (the panels are data for a view).

## Open

1. **Where lazy owners' goods live.** A pop that is a count in Macro holds nothing on the ledger; its household's account does. Recommendation: households and establishments are the leaf owners for cold pops; a pop gets its own account when it becomes a record (as its body gets a slot when embodied, [29](29-embodied-pops.md)). The pool pattern applies: wallets reserved at setup.
2. **Debt.** Rule 5 forbids a negative wallet. Credit is a separate resource (a claim) held by the lender, not negative money. Later.
