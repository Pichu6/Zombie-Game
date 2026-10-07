# 29 · Embodied pops: the jump from the registry to the body simulation

**Status:** Proposal. **Stage:** not Tier 0; first jump of the [layer jumps](../../design/layer-jumps.md) audit (row 2). **Needs:** [03](03-stock-transfer.md), [11](11-body-composition.md), [12](12-body-wounds.md), [27](27-detail-levels.md), [28](28-pop-levels.md). Research: [layer jumps, multi-resolution](../../research/layer-jumps-multiresolution.md). Decision: [0039](../../decisions/0039-fill-the-layer-jumps.md).

## Purpose

A pop in the registry is a lazy pop or a record ([spec 28](28-pop-levels.md)). The body simulation (`Bodies`) runs real bodies: metabolism, blood, wounds, infection. Nothing connects them. This spec defines the third level, **embodied**: a registry pop gets a body in the body simulation and gives it back, so the same person is a count in Macro, a record in City and a body in Individual, and nothing about them is created, lost or contradicted on the way.

## What the code forces (found by reading it)

- **The entity set is fixed after setup.** A world is loaded into a candidate built by the same setup ("bodies are created by setup; the save holds what changed since"). Ledger owners and bodies cannot be created mid-run without breaking that. So embodiment uses a **pool of body slots reserved at setup**.
- **The ledger only mints at setup.** A body's tissue (fat, muscle, structure, glycogen, water, blood) is ledger mass. A lazy pop's tissue is not in the ledger at all. So embodying has to bring mass into the ledger by an explicit, audited step, and demoting has to take it out.
- **Two `Sex` enums** existed (pop and body, opposite value orders); the pop one is now `PopSex` ([28](28-pop-levels.md)), with a conversion at this boundary.

## State

| Field | Notes |
| --- | --- |
| Body pool | `Bodies::reserve_slots(world, n)` at setup creates `n` vacant bodies with fixed ids and one ledger owner each, holding nothing. A vacant slot runs no rule and is skipped by every reader (`occupied()`). `n` is the cap on the embodied set (the hot-set budget). |
| Slot occupant | a spec, the body's normal values and state, and the pop it belongs to; saved with the body |
| `BodyState` | everything the body model holds about one body: spec, normals, scalars (posture, activity, sleep pressure, exertion, fear, morale, active minutes, carries), the wound list, and its ledger amounts per body resource |
| Record fields (registry) | `embodied` (the slot's body id, 0 if none), `body_state` (the last `BodyState` saved at demotion), `embodied_since` (tick, for dwell) |
| Declared flows | `Ledger::declare_inflow` and `declare_outflow`: a caused entry that changes a resource's expected total, allowed after setup |

## Rules

1. **Embody** (registry → bodies). A pop can be embodied if it is alive at the current year, is not already embodied, and a slot is free. Otherwise the request is refused with a logged reason (`PopCommandRefused`). The pop becomes a record if it is not one.
2. **Where the body comes from.** If the record holds a saved `BodyState`, restore it exactly. Otherwise generate a `BodySpec` from the pop: a **stateless keyed function** of (world seed, pop id, age, sex) using the same distributions as the Tier 0 census (NHANES-like height and BMI, Deurenberg body fat, VO2max by age and sex), so a pop embodied twice from the same record gets the same body in any order (A1). Age is the pop's age at the current year.
3. **Mass accounting.** The ledger conserves what is *in play*. Embodying declares an **inflow** of the body's tissue amounts to the slot's owner, with the command's event as cause; demoting declares an **outflow** of whatever the body then holds. People are conserved as counts (a pop is lazy, a record or embodied, never two). Tissue mass of lazy pops is outside the ledger by design, so the flows are visible, caused entries and never silent mints.
4. **Demote** (bodies → registry). Allowed only if the body is alive and not on the map as a walker, and has been embodied for at least the **dwell** time (hysteresis: no flicker at the edge of the hot set). The body's state is saved as `BodyState` in the pop's record, its amounts leave the ledger by outflow, and the slot becomes vacant. If the body died while embodied, the pop is dead: the death is a registry exception (death year) and the corpse stays in the body simulation as a thing, not a slot occupant.
5. **Round trip.** Embody, run no time, demote, embody again: the same body in a possibly different slot, the same state, the same ledger totals (A1).
6. **Time while demoted.** *Known gap, stated plainly:* a demoted pop's body state is restored **as it was left**; the elapsed time is not yet applied by body jump forms ([27](27-detail-levels.md) rule 2). The pop's own life (age, death, births) continues lazily and correctly; the body's hunger, healing and wound progress are paused. This disagrees with stepping and is to be closed by a body `advance(from, to)`; until then a test records it as expected.
7. **Single source of truth.** The registry's `embodied` flag and the body pool's occupancy are checked against each other by the audit every tick when checks are on; a disagreement throws.
8. **Budget.** Embodying moves one body; the pool size is the cap, a request beyond it is refused (never queued silently). Waking a district does not embody it: embodiment is requested by the layer or rule that needs a body (the player's attention, a fight), so one house cannot wake the street ([research](../../research/layer-jumps-multiresolution.md) failure mode 3).
9. **Commands.** `PopEmbody` and `PopDisembody` (subject = pop) go through the world's command path like every other pop change, so they are replayable, carry a reason, and are saved with the world.
10. **Items and action.** Out of this slice: a body here has no actor or inventory. Embodying a pop that should act (walk, use, fight) needs an actor slot too; demotion of a body holding items is refused until items are handled. Both are listed under Not in this spec.

## Invariants

- A pop is in exactly one of lazy, record, embodied.
- `occupied slots = records with embodied flag`, and each pairs to one pop.
- Ledger totals are equal before and after any embody or demote apart from the declared inflow or outflow, which equals the body's tissue amounts exactly.
- No resource goes negative; vacant slots hold nothing.

## Tests (agreement levels, mapped to the failure modes in the research note)

- **Same pop, same body, any order (A1; mode 1, 11):** embody pops A and B in either order: each gets the identical spec and state.
- **Round trip (A1; mode 2):** embody, demote, embody, demote across 20 cycles with no time: world state hash after each demotion equals the first; ledger totals equal.
- **Conservation (A1; mode 8):** after each step, tissue mass of all slots equals the net declared flows; ledger check passes every tick.
- **Two truths (A1; mode 9):** random sequences of embody and demote: registry flags equal pool occupancy at every step.
- **Idempotence (A1; mode 10):** embodying an embodied pop, or demoting a lazy one, is refused and changes nothing.
- **Budget (A1; mode 3):** with a pool of 8, the ninth request is refused; the embodied set never exceeds 8 over a fuzzed run.
- **Thrash (A1; mode 4):** requests to demote before the dwell time are refused; a pop pushed back and forth is demoted at most once per dwell.
- **Death (A1):** a body that bleeds out while embodied makes the pop dead in the registry; its corpse remains; the slot is freed.
- **Save, load, replay (A1):** a world with embodied pops saves, loads and replays to the same hash; the v4 and earlier registry fixtures still load.
- **Time gap recorded (A3):** re-embodied state equals the state left (a pause), with a comment that stepping would differ.

## Calibration

| Value | Source |
| --- | --- |
| Body distributions | Same as the Tier 0 census: NHANES-like heights and BMI, Deurenberg body fat, VO2max by age ([22](22-census.md)); placeholders until fitted to the nation's profile |
| Pool size, dwell time | None: settings, tuned by the hot-set benchmark |

## Not in this spec

Actors and inventories for embodied pops (action, AI), items carried, body jump forms for time spent demoted, embodiment of walkers from the census, the player's possession switch, the policy that decides *who* to embody (attention, proximity, fights), and the City-level chunk wake ([27](27-detail-levels.md)).

## Open

1. **Mass of lazy people.** Recommendation (adopted here): outside the ledger, with declared flows at the jump. The alternative, minting every lazy pop's mass at world creation so embodying is a pure transfer, is exact but costs a sum over every pop at setup and still leaves births unexplained.
2. **Pool size** per world: a setup setting; recommendation 256 and measure.
3. **Body jump forms** for time spent demoted (hunger, healing, wounds): next unit after this one.
