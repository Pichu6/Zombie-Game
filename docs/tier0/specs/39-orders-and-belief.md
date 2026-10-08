# 39 · Orders and belief across levels

**Status:** Implemented and tested, headless (edicts, the cause trace, beliefs, the orders world system). **Stage:** not Tier 0; the [layer jumps](../../design/layer-jumps.md) audit rows 15 (orders and laws) and 16 (news and belief). **Needs:** [06](06-save-replay.md), [28](28-pop-levels.md) (region events on rosters), [32](32-stocks-goods-and-money.md), [33](33-establishments-and-chains.md). Design: [macro layer](../../design/macro-layer.md) §3.3 (fog) and §5 (the causal trace from a law to a hospital), [world simulation](../../design/world-simulation.md) §7 (edicts) and §9 (messages become beliefs with source and age). Decisions: [0014](../../decisions/0014-open-questions-answered.md) items 2, 6 and 8, [0044](../../decisions/0044-information-scope-per-layer.md), [0039](../../decisions/0039-fill-the-layer-jumps.md).

## Purpose

An edict at Macro, its bite on an establishment at City, compliance at Individual: one order. A headline at Macro, a rumour at City, what one person believes at Individual: one piece of news. This spec makes an order a **record that acts only through the ordinary commands of the systems below it**, each application carrying the order as its cause, so the inspector can say *why* a transfer happened and the order's panel can say *where* it bit. And it makes news a **belief store per person**, filled by broadcasts that need working communications and by retellings that distort, read by a lens that never shows the truth while playing a person.

## What the code forces

- **Every change has a cause** ([03](03-stock-transfer.md): a ledger entry carries its cause event; [06](06-save-replay.md): events carry theirs). An edict needs no new field: its signing is an event, each application is an event caused by the signing, and every transfer of an application is caused by that. `behind(event)` walks the chain.
- **Commands are the only way to change a system after setup** ([28](28-pop-levels.md) rule, `PopSystem`, `EconomySystem`). An edict therefore does not touch stocks or rosters: it issues the same commands a player would (`EconShip`, `EconPay`, `RegionApply`), through the systems' own `apply_command`, with its application event as cause, so replays and refusals work unchanged.
- **The log is never read to decide anything** (`EventLog`): the trace is an inspector query; the edict's own record keeps the sums it needs.

## State

| Field | Notes |
| --- | --- |
| `EdictDef` | name; kind (`Levy`: a share of a good's bulk each day from every Building-scope owner under a scope to a destination; `Tax`: the same for money, as a `Tax` flow; `Staffing`: on signing, `delta` members of the region's workplace rosters laid off, negative recalls); scope, destination, good, per mille, delta, the registry's region |
| `Edict` | def, signer, signed tick, repealed tick, `signed_event` (the root cause), applications, `moved` (bulk, cents or members, ever) |
| `Belief` | about (an entity: an edict is `(1 << 40) | index`), claim (a number), source (`Seen`, `Told`, `Broadcast`), from (the teller), learned tick, hops, confidence |
| `OrdersSystem` | the edicts, the beliefs, the audience of a broadcast (setup), `comms_up`, the distortion per hop |

## Rules

1. **Sign.** `EdictSign` (subject = signer, def): refused if the def is unknown or the system it needs is not attached. The signing event is emitted first, so its id is the root cause of everything the edict does; a `Staffing` edict is applied at once (a `RegionApply` of laid-off members on the region's workplace rosters, cause = the signing); then the edict is broadcast (rule 5).
2. **Apply, daily.** Once a game day the standing `Levy` and `Tax` edicts are applied, not on the day they were signed: an `EdictApplied` event (cause = the signing), then for each Building-scope owner under the scope, other than the destination, a shipment or a payment of `balance × per mille ÷ 1000`, through `EconomySystem::apply_command` with the application as cause. Nothing is minted; a levy compounds down (10 % a day leaves 0.9ⁿ). `moved` sums what the destination received.
3. **Repeal.** `EdictRepeal` ends it; applications stop the next day; refused if not in force. What moved stays moved.
4. **The trace.** `Edicts::trace(log, event)` is the chain of causes, nearest first; `behind(log, event)` finds the `EdictSigned` event in it and the edict that owns it. So the Status tab of a building can name the edict behind the shipment that emptied it ([macro layer](../../design/macro-layer.md) §5), and an edict's panel is a sum over the entries caused by its applications. Compliance returns as effect: `moved`, `applications`, and the tallies of [32](32-stocks-goods-and-money.md) by kind.
5. **Broadcast.** When an edict is signed and `comms_up`, every pop in the audience receives a `Broadcast` belief about the edict entity with the claim (the per mille, or the delta) and the tick; a `Broadcast` event records how many heard. With communications down nobody hears ([0014](../../decisions/0014-open-questions-answered.md) item 8); the edict still applies, because stocks move by state, not by news.
6. **Tell.** `Tell` (subject = teller, listener, the thing): the listener receives the teller's belief with source `Told`, `from` the teller, one more hop, confidence × 0.7, and the claim drifted by a keyed normal share of its size (`distortion_per_hop`, default 15 %), keyed on (teller, listener, thing, hop): the same retelling always makes the same story. Refused if the teller believes nothing about it.
7. **The lens.** `Beliefs::view(pop, thing, now)`: known or not, the claim, source, age, hops, confidence. It reads the store only, never the edict: while playing a person the panel shows this ([macro layer](../../design/macro-layer.md) §3.3 fog, [0044](../../decisions/0044-information-scope-per-layer.md)).
8. **Save.** Edicts, beliefs and the communications flag are saved after the economy; defs and the audience are setup.

## Invariants

- Every ledger entry under an edict traces to its signing event; the edict's `moved` equals what its destination received under its applications.
- Money and goods totals are unchanged by any edict (transfers only).
- A belief's hops equal the retellings since its source; confidence falls with each; a story told twice is one story.
- The lens never returns a value that is not in the store.

## Tests (`test_orders.cpp`)

- **Edicts (A1):** a 10 % grain levy signed on day 0 and repealed on day 4 applies four times: the farms keep 0.9⁴ of their grain, the national store holds the rest, `moved` equals it; a 5 % business tax applies four times and its `moved` equals the country's `Tax` tally, with the money total unchanged; ledger entries under each trace to the right edict and the chain reads application then signing; a staffing edict lays off three on the mill's roster at once; three refusals (unknown def, telling what one does not know, repealing twice).
- **Beliefs (A1):** all ten hear the levy and the tax, nobody hears the layoffs signed after the phones died; the broadcast claim and age are right; pop 7 told by pop 0 has one hop and 0.7 confidence, pop 9 told by pop 7 has two and 0.49; at least one retold claim differs from the truth; the same retelling again gives the same claim.
- **Persistence (A1):** save, load, save identical, the communications flag included; replay and a mid-run resume reproduce the hash.

## As built

- `engine/sim/include/zs/edicts.h`, `src/edicts.cpp`: `EdictDef`, `Edict`, `Edicts` (`add_def`, `sign`, `repeal`, `trace`, `behind`, save and load).
- `engine/sim/include/zs/beliefs.h`, `src/beliefs.cpp`: `Belief`, `Beliefs` (`receive`, `about`, `of`, `retell`, `view`, save and load). Keyed draws through `fields::keyed_normal`, field 301.
- `engine/sim/include/zs/orders_system.h`, `src/orders_system.cpp`: `OrdersSystem` (commands `EdictSign` 0x4001, `EdictRepeal` 0x4002, `Tell` 0x4003, `SetComms` 0x4004; refusals `OrderRefusal`; events `EdictSigned` 59, `EdictApplied` 60, `EdictRepealed` 61, `OrderRefused` 62, `Broadcast` 63, `Told` 64). `Owners::descendants` was added to [32](32-stocks-goods-and-money.md)'s tree.

## Calibration

| Value | Source |
| --- | --- |
| Distortion 15 % per hop, confidence × 0.7 | Placeholders; the serial-reproduction literature (Bartlett) gives the shape, not these numbers |
| Edict shares and deltas | Scenario data; none here |

## Not in this spec

Edict kinds beyond levy, tax and staffing (price rules, rationing, conscription, conditions "in zone during hours", use-of-force thresholds, [world simulation](../../design/world-simulation.md) §7); upkeep and legitimacy; the audience from the registry (today a setup list); beliefs from seeing (`Seen` is defined, not produced: perception will write it); news as a feed of headlines from events; beliefs about places and people (the entity id allows it; nothing writes them); the compliance threshold of individuals (row 15's "compliance by threshold": whether a pop obeys a belief it holds); the `warning` of [38](38-outbreak.md) as a lens over beliefs.

## Open

1. **An edict's refusals.** When an owner under the scope has nothing to give, the economy emits `EconRefused` with the application as cause: honest, but noisy for a levy over a thousand farms. Recommendation: the edict skips owners with nothing (it does: `amount ≤ 0` is skipped), and a refusal reaching the log is a real anomaly worth seeing.
2. **Beliefs that age out.** Nothing forgets. Recommendation: a belief's confidence decays with age in the lens, not in the store, until a memory model exists.
