# Layer jumps

One simulation is seen at three layers: **Macro** (nations, sectors), **City** (zones, buildings, supply) and **Individual** (one person, real time). A **jump** is where one layer's state is turned into another's: summed up, split down, switched to a different level of detail, caught up in time, or sent as an order or a report. This doc audits every jump the simulation needs, says which exist, and proposes the rule for each gap. It is the registry the contract in [world simulation](world-simulation.md) §2 rules 8 and 12 asks for ("multi-layer units ship an aggregation and disaggregation pair, tested together").

Intent **Decided** ([0039](../decisions/0039-fill-the-layer-jumps.md)): the gaps between layers must be filled, each with a defined and tested jump. Everything below is a **Proposal** unless a row says Built. Related: [pop continuity](pop-continuity.md), [spec 27](../tier0/specs/27-detail-levels.md) (places), [spec 28](../tier0/specs/28-pop-levels.md) (pops), [world systems](world-systems.md) §6, [detail levels](../research/detail-levels.md). Research on known failure modes in multi-resolution simulation: `docs/research/layer-jumps-multiresolution.md` (being written).

## 1. Kinds of jump

| Kind | What happens | Example |
| --- | --- | --- |
| **Up** (aggregate) | Micro entities add up to a macro number | A sector's output is the sum of its establishments' output |
| **Down** (disaggregate) | A macro number or order becomes micro events or entities | "Lay off 3%" becomes hits on workplace rosters; a sector's stock becomes goods on shelves |
| **Level switch** | The same entity runs at another detail level | A lazy pop becomes a record, then a body |
| **Time** | Elapsed time is applied in one step | A fridge off for 18 hours spoils 18 hours of food on wake |
| **Field** | A field is coarsened or refined | A district's background noise becomes the noise a body hears |
| **Order and report** | A decision goes down, a result comes up, with its reason | An edict reaches a roster; its effect returns as revenue |
| **Belief** | What a layer *knows* differs from what is true | The country panel shows what your character believes |

## 2. What every jump must declare

The contract each domain fills in before code ([world simulation](world-simulation.md) §2 rules 8, 11, 12):

1. **Truth:** which level owns the fact. Others read or derive; none stores a second copy.
2. **Up function** and **down function**, as a pair.
3. **Invariant:** what stays equal (people, kg, litres, money, ids), and the **agreement level**: A1 identical, A2 same distribution, A3 same direction or inside a band.
4. **Keyed draws:** every random choice keyed by entity and purpose, so waking early or late, or in any order, gives the same result ([spec 28](../tier0/specs/28-pop-levels.md) technique).
5. **Hysteresis:** the margin that stops a thing flipping between levels every tick.
6. **Cost and budget:** what the jump costs and how it is spread over ticks.
7. **Cause:** the reason recorded on every change, so an inspector can say why.
8. **Tests:** round trip, aggregate against enumeration, jump against stepping, order independence, and save and load.

## 3. The audit

Status: **Built** (code and tests), **Designed** (a doc says how), **Gap** (no rule yet).

| # | Domain | Macro | City | Individual | Jump | Status |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | **Pops: identity** | Counts by age, work, wealth, loyalty | Rosters (workplace, household) | Body, mind, ties | Same pop at every layer; counts are sums; lazy, record ([spec 28](../tier0/specs/28-pop-levels.md)) | **Built** for lazy and record, region events and rosters |
| 2 | **Pops: embodied** | (same) | (same) | The body in the body simulation | Promote a registry pop into `Bodies` and demote it back, keeping wounds, items and state as exceptions | **Built** ([spec 29](../tier0/specs/29-embodied-pops.md): pool, commands, declared ledger flows; body time while demoted is a recorded gap) |
| 3 | **Zombies and the dead** | Counts by cell | Off-map pool | Walkers with real bodies | Off-map walkers promote on arrival, demote when destroyed (Tier 0 census) | **Built** for Tier 0's 60; no rule for thousands |
| 4 | **Places and things** | Region numbers, stocks | Building ledger, room graph | Cells, items, fields | Chunks with cold, warm, hot ([spec 27](../tier0/specs/27-detail-levels.md)); placement generated on approach and saved | **Designed**, one hot chunk built |
| 5 | **Stocks and goods** | Province stock of food, fuel, goods | Establishment inventory, warehouse | Item instances on shelves and in hands | Stock of kg or litres becomes items on a visit; items on leaving become stock; mass conserved | **Gap** (the ledger and item model exist at Tier 0 scale only) |
| 6 | **Money** | Budgets, revenue, debt | Company accounts, wages | Wallet, price paid | One money ledger across levels; a price is a market state, a purchase is a transfer | **Gap** (no money resource) |
| 7 | **Jobs and establishments** | Jobs by sector, output | Establishment with staff roster and inputs | A job, a shift, an action at a machine | Output is the sum of establishments' output; a pop's job is a roster slot | **Built** ([spec 33](../tier0/specs/33-establishments-and-chains.md): staff from the workplace roster, output summed by the owner tree) |
| 8 | **Processing chains** | Chain throughput | Plant: inputs, workers, power | A worker running a process | Throughput is the sum of plants' output given inputs; a process the player runs by hand is the same recipe | **Built** ([spec 33](../tier0/specs/33-establishments-and-chains.md): recipes as declared conversions, `EconRunByHand`, chains by shipment) |
| 9 | **Construction and projects** | A programme and budget | A project: site, materials, jobs | Hauling, building, a task | Posted jobs and material demand answered by pops; stock of buildings is the sum of finished ones | **Built** ([spec 34](../tier0/specs/34-projects.md): a plan on a site, escrow, stages as a recipe, a contractor paid per stage, the building as one unit; job postings to pops are the roster link of spec 33) |
| 10 | **Networks** (power, water, roads, telecom) | National capacity, load, outages | District grid, plant, mains | Building circuit, tap, wire | A fault below shows as an outage above; a load-shedding order above becomes breaker events below | **Built** at building scale ([spec 17](../tier0/specs/17-power-network.md)) and above it ([spec 35](../tier0/specs/35-networks.md): a tree of grids with the same node records at every level; the Tier 0 building network is not yet a leaf of it) |
| 11 | **Fields** (noise, light, smoke, scent) | none | District ambient (the grid dying drops background noise) | Cells and sources through walls | Coarsen a sector's sources to an ambient; refine ambient plus nearby sources to what a body hears | **Built** at block scale ([specs 18, 19](../tier0/specs/18-noise-field.md)); **Gap** coarse level |
| 12 | **Weather and climate** | Climate fields by month | Local weather per district | Temperature on a body, rain on a roof | Downscale a field to a place and time with keyed variation; local conditions never contradict the field's mean | **Gap** |
| 13 | **Flora and fauna** | Stocks, growth, harvest | Forests, fields, herds by cell | A tree, an animal, a hunt | A stock becomes individual animals and trees when approached; kills and cuts lower the stock | **Gap** |
| 14 | **Outbreak** | Compartments by region | Infected pops in rosters | Wounds, infection, a turned body | Aggregate rates equal the entity-level rates (reproduction number built from what the sim has); infection of a record changes the counts | **Designed** ([threats](threats.md)); census-scale built |
| 15 | **Orders and laws** | Edict, programme | Policy at an establishment or sector | Compliance by threshold | An edict becomes events with a reason; compliance returns as effect | **Partial** (commands, region events, reasons built; edicts not) |
| 16 | **News and belief** | Headlines from events | Local rumours | What a pop believes (source, age) | Messages become beliefs; the panel shows beliefs, not truth, while playing a person | **Designed**, not built |
| 17 | **Time** | Years per second | Days | Seconds, real time | Long steps use jump forms; embodied bodies force a speed their rules allow; wake-up catch-up | **Partial** (life jumps built; no scheduler across layers) |
| 18 | **Travel** | Flows between sectors at path speed | Routes, ports | Walking, driving at body and vehicle speed | A person crossing the world changes level on the way; a convoy is a group | **Gap** (rule only: speed belongs to the path) |
| 19 | **Ownership and keys** | Who owns what | Building and company ownership | A key in a pocket, a deed | Ownership is a record on the ledger; a key opens its real door | **Gap** |
| 20 | **Culture, belief, politics of pops** | Shares by sector | Neighbourhood mix | A pop's culture, faith, leaning | Shares are sums over pops; a child inherits from the family ([0026](../decisions/0026-country-creation-inheritance.md)) | **Designed**, births have a mother only |
| 21 | **Technology and capability** | Readout of what exists | Establishments and schools | Know-how a person holds | Nation's tree is a readout; a capability is lost with its plants and people ([0030](../decisions/0030-pop-continuity-and-capability-loss.md)) | **Designed**, not built |
| 22 | **War and forces** | Forces, supply, resolution | A garrison, a front | A soldier in a fight | An aggregate result carries to bodies and back | **Designed**, secondary |

## 4. The proposed rules, grouped

**Truth is always the finest level that exists; coarser levels are sums or readouts of it** (rule 4 of [world systems](world-systems.md) §1). A coarse level never holds an independent number that could disagree.

1. **Pops: embodied (row 2).** Add the third level to the registry. A registry pop is promoted by generating its body from its id (body seed, age, sex, past, from the human generator) and handing it to `Bodies`; wounds, items, needs and ties from the old level are applied as the generated state plus saved exceptions. Demotion writes only what the seed cannot say as exceptions ([spec 28](../tier0/specs/28-pop-levels.md) rule 3) and removes the body. Hysteresis: promotion starts before the body is visible and demotes only after it has been out of range for a set time. The test is the round trip: promote, run, demote, promote again, and the pop is the same pop with the same history (A1), with ledger totals equal.
2. **Stocks and goods (row 5).** One resource, many owners: a province, an establishment, a room and an inventory are owners on the same ledger. "A stock becomes items" is a **conversion declared in data**: kilograms of grain to sacks, litres of fuel to cans, with a rule for what a visit may generate (only what the ledger holds). Spoilage and burn are rates and jump exactly ([spec 27](../tier0/specs/27-detail-levels.md) rule 2). Test: conservation across a visit and a round trip (A1).
3. **Money (row 6).** A single `money` resource on the ledger, owners at every level (state, company, household, wallet). Prices live in market state per sector and are read by shops; a purchase is a transfer; payroll is a transfer from establishment to pop. Aggregates (revenue, GDP) are sums of transfers. Test: money is conserved across levels; no hidden mint (A1).
4. **Establishments, chains, construction (rows 7–9).** An establishment is a first-class entity at City level with a staff roster, an input stock, a power draw and a recipe. Macro throughput is the sum over establishments; an order changes staffing or inputs through the same commands as pops. A process the player performs by hand runs the same recipe as an establishment's, so a hand-built tool and a factory's are the same item.
5. **Networks (row 10).** Hierarchical graphs with the same component records at every level: a national grid is a graph of district grids, a district grid a graph of building circuits. A fault is a component state; the upper level reads "capacity of sound components"; a load-shedding order is a command that opens switches at the level it names. Test: a fault in one building appears as lost load in its district and nation, and clearing it restores exactly the same (A1).
6. **Fields (rows 11, 12).** Two operators per field: **coarsen** (sum or max of sources into an ambient per cell) and **refine** (ambient plus local sources, with keyed variation). Refinement must reproduce the coarse mean over a region: the average of refined values equals the coarse value (A3, band). Noise is the first: when the grid is down, ambient drops, so voices carry farther, as already decided ([world simulation](world-simulation.md) §9). Weather downscales by the same rule.
7. **Flora and fauna (row 13).** A stock per cell with a *generator of individuals on approach*: trees and animals are drawn from the stock and the cell's seed, saved when seen, and returned to the stock when out of range; kills and cuts subtract from the stock. The same pattern as zombies in unloaded cells (a count per cell, bodies near the player).
8. **Outbreak (row 14).** The reproduction number of the aggregate compartments is *measured* from entity-level runs (bite chance from speed and warning, kill chance, conversion), not set separately; the preview and the world run the same numbers. Test: aggregate and entity-level runs of the same scenario agree in distribution (A2).
9. **Orders and reports (rows 15, 16).** Every order is a command with a reason that carries down to events; every report carries up with the same reason ([macro layer](macro-layer.md) §5). Belief is a view: while playing a person, upper layers show that person's beliefs, each with source and age.
10. **Time (row 17).** One scheduler decides, per region, the speed at which its things run: embodied bodies need a speed their rules allow (1× in a fight), everything else may jump. Long steps use only jump forms. Waking a chunk applies elapsed time with keyed draws so early and late wakes agree (A2). Tests: a long step against many short ones for each rule (A2).
11. **Travel (row 18).** Speed belongs to the path ([world simulation](world-simulation.md) §2 rule 7). A journey is a path through sectors; the traveller's level on each leg follows what is near them and the player's attention: a lazy pop between towns, a record in a convoy, a body in a fight. Position is a point on the path at any time, so promotion places them exactly.
12. **Ownership and keys (row 19).** Ownership is a ledger relation (owner of a building, a company, an item); a key is an item that names a door; the lock is the door's assembly. Macro "ownership" is the sum of what is owned.

## 5. Order of work

Dependencies only; the first two block the most.

1. **Pops: embodied** (row 2) completes the pop jump and joins the new pop work to the body simulation.
2. **A generic jump test harness** (round trip, aggregate against enumeration, jump against stepping, order independence, determinism) so each later jump is tested the same way; first applied to the pop jumps already built. *Done 2026-10-07:* `engine/sim/tests/jumps.h` (Anderson–Darling, energy distance, proportions, bands; [catch-up-jump-forms.md](../research/catch-up-jump-forms.md) §5), applied to the demography jump and to the new body jump that closes spec 29 rule 6 (time while demoted).
3. **Stocks and goods**, then **money**: the two conserved resources every other jump uses. *Headless slice done 2026-10-07:* [spec 32](../tier0/specs/32-stocks-goods-and-money.md) (`engine/sim/economy.h`): owner tree with scopes, goods as bulk and units by declared conversion, spoilage with an exact jump form, money as a resource with purchases and payments tallied by kind, and the panel rule of [0051](../decisions/0051-money-and-resources-shown-follow-the-selection.md); `EconomySystem` wires it to the world's commands, hourly spoilage, saves and replay.
4. **Establishments**, then **chains** and **construction**. *Establishments and chains done 2026-10-07:* [spec 33](../tier0/specs/33-establishments-and-chains.md) (`engine/sim/establishment.h`, wired in `EconomySystem`): a recipe is a declared conversion plus secondary inputs, labour and power; an establishment accrues labour as staff × ticks while powered and runs whole batches as far as inputs allow, with an exact jump (partition-independent); staff comes from the workplace roster; Macro output is the sum under an owner. *Projects done 2026-10-08:* [spec 34](../tier0/specs/34-projects.md) (`engine/sim/project.h`): an order escrows the budget on the site, the site's establishment builds stages by the plan's recipe as materials arrive, a contractor is paid per stage, stalls name their reason (staff, power, money, materials), the finished building is one unit of its good (the Macro building stock is a ledger sum), and the site then runs the plan's own recipe.
5. **Networks above building scale**, then **fields** and **weather**. *Networks done 2026-10-08:* [spec 35](../tier0/specs/35-networks.md) (`engine/sim/grid.h`): one node record (source, switch, load, hub) at every level, a child grid as a load of its parent fed through one node, demand summed bottom-up over closed and sound parts, supply allotted top-down in a fixed order; a fault below is unserved load above and a repair restores the exact state; an order above (open a switch, cut a plant) is unserved load below.
6. **Flora and fauna**, **outbreak**, **orders** and **belief**, **time scheduler**, **travel**, **ownership**.

## 6. Open questions

1. **Priority.** The order above follows dependency. Is there a jump you feel most as a gap (for example, going from a nation's order to one person's day)? That would move it up.
2. **How exact must a jump be for the player?** Recommendation: exact for anything conserved or countable (A1), same distribution for noisy processes (A2), within a band for fields (A3), as in the contract.
3. **Where the switch is visible.** The GTA-style switch hides promotions ([world simulation](world-simulation.md) §8). Do plain zooms between Macro and City also hide loading, or may they stream? Recommendation: zooms stream art only and never change what is true.
