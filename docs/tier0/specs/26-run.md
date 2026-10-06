# 26 · The run: "The generator problem"

**Status:** Proposal. **Stage:** 9. **Needs:** every other spec.

## Purpose

The Tier 0 scenario that ties the units into one 14-day run with a pressure arc, and the telemetry that answers the kill/keep questions ([status](../../status.md), [concept v2 §5](../../research/concept-v2.md#5-the-tiers)).

## Start (defaults picked here)

- **Day one of the outbreak in the block:** the run starts the morning the first walkers appear on the street. The warning week is not in Tier 0.
- **One survivor:** the player picks a preset civilian or adjusts height, age, fat and muscle on one screen. Start in the main building.
- **Grid** on at start; fails on a drawn day between 2 and 5 ([17](17-power-network.md)).
- **Fuel:** generator tank part-full plus cans in the houses; enough for roughly the first 9–10 evenings of moderate use, so fuel runs short around day 10.

## Found survivors (3)

| Who | Where | Twist |
| --- | --- | --- |
| A neighbour | House A, barricaded, low on water | Knows first aid; heavy body, slow, strong hauler |
| A stranger | Arrives at the yard gate around day 4 | Carries a hidden bite with probability 0.5 (drawn per seed); secretive or honest |
| A technician | Hiding in a windowless storeroom in House B, in the dark | Knows electronics: the only one who can repair the radio |

## Goal and pressure arc

- **Goal:** be alive on day 14. **Better ending:** repair the radio, power it, and hear the evacuation broadcast that names the pickup.
- **Pressure:** fuel crunch near day 10; waves that grow with noise and light ([22](22-census.md)); hidden infection inside the group; the radio needs power, and power means noise.
- **Control after death** (Proposal, not yet in any decision): when the controlled pop dies, control passes to another group member; the run ends when the goal is met or no group member is alive.

## Telemetry (for the kill/keep questions)

| Question | Logged |
| --- | --- |
| 1. Comfort vs noticed every 5–10 min? | Each decision moment: generator on/off, lights at night, loud work at night, real minutes between them |
| 2. Direct control used, when? | Possession time per pop, what triggered it (refusal, combat, fine work) |
| 3. Infected friend lands? | Player actions when symptoms appear; recognition of named walkers (asked after the run) |
| 4. Sped-up time feels like play? | Time spent at each speed; how often events dropped to 1× |
| 5. Can the player tell why? | Chronicle viewed; post-run question |
| 6. Different bodies, different decisions? | Job assignments by body type |
| 7. Death feels fair and taught? | Danger tag of each death; post-run question |

The view writes these to a separate telemetry file with wall-clock stamps (the simulation's input log keeps ticks only). A short post-run form in the app asks the questions that logs can't answer.

## Tests

- A scripted "careless" run dies of a tagged cause before day 14; a scripted "careful" run reaches day 14. (A3)
- 50 AI-only runs: in most of them the fuel crunch lands between day 8 and day 12. (A3)

## Rules for the headless run (stage 9, Proposal 2026-10-06)

Picked here so stage 9 can be built before the window (stage 5); the view adds choice, not new rules.

1. **Setup** comes from `data/tier0/run.toml`: the three found survivors (census survivor slots), where each waits, their trades and twist. The map holds the fuel: 4 L in the tank, a 15 L can in the workshop, a 5 L can in each house (29 L in all).
2. **Neighbour:** House A, the small back room, door shut, starts 30 % short of water. Trade: first aid. **Technician:** House B, the windowless storeroom, door shut, no light. Trade: electronics. **Stranger:** arrives outside the yard gate at a drawn time on day 3-5 (08:00-18:00); with p 0.5 carries an established bite and is secretive about it (given by a walker still out there when he arrives; if none is left he comes clean).
3. **Joining:** a found survivor joins when a group pop does `invite` (a raised voice, 20 s) next to them. The AI only asks people the group knows about: found in a search, met in the same room, or the stranger calling at the gate. Until then they are bodies with no AI: they stay where they are and only their body runs.
4. **Search job (new, 7th):** by day, with no walker in sight, an AI pop walks to a house it has not searched, invites anyone found, and brings back the fuel can there. One pop per house.
5. **Radio:** broken at the start. `repair_radio` needs the electronics trade (only the technician knows it), a screwdriver and 30 min. `listen_radio` needs it repaired and powered (switched on, mains or generator), takes 10 min; from a drawn day 9-12 at 12:00 the broadcast names the pickup and `RadioHeard` is logged (the better ending). Before that only static.
6. **Control after death:** the run has one controlled pop (the player's body at the start). When it dies, control passes to the living group member who joined first; `ControlPassed` is logged, and if the dead one was possessed, possession moves with control.
7. **End:** day 14 08:00 with someone alive (survived; rescued if the broadcast was heard), or the moment nobody in the group is alive. `RunEnded` carries the outcome.
8. **AI generator policy for the run:** run it 10:00-16:00 when the grid is down and fuel lasts (the radio is listened to inside those hours); never at night.

## Not in Tier 0

Setup screens, other scenarios, the warning week, Macro and City layers.

## Model as implemented (stage 9, headless; numbers are Proposal)

Code: `engine/sim/src/run.cpp` (`Run`, a saved System updated last), `tier0::make_run`, data `data/tier0/run.toml`, map objects tagged `scenario = "run"` in `block.toml`; chronicle in `tier0.cpp` (`danger_tags`, `missed_counters`, `day_summary`); `zs_headless --run [--seed N] [--seeds N] [--careless] [--trace F]`.

- **Fuel:** 4 L in the tank, 15 L in the workshop, 5 L in each house: 29 L, about nine evenings of 4 h at the fridge-and-lamps load.
- **The stranger** is a body marked `away` (spec 27: cold, nothing runs or senses it) until the drawn time; a hidden bite is given on arrival by one of the walkers still off the map, so its clock starts at the gate.
- **Scenario effects** go through one `story` effect op (`invite`, `repair_radio`, `listen_radio`) with new refusal reasons `broken` and `no power`.
- **Death screen** (headless): the sentence, danger tags and missed counters for each of the group; day summaries by calendar day.

**AI-only results (A3, 48 seeds, 14 days):** someone reaches day 14 in 23 of 48 runs, and 8 of those heard the broadcast; 131 of 144 found survivors join; deaths are mostly bite infections (113), then bleeding (24). The fuel runs out in 15 runs, 11 of them between day 8 and 12, so the crunch and the broadcast compete for the same fuel: saving it for the radio is the player's call, and stage 10 tests whether that choice is fun. With `--careless` (generator any hour, no barricades or guards) only 8 of 48 reach day 14, none hears the broadcast, and bleeding deaths double (55). Careless play is clearly punished.

**Known gaps:** no setup screen (one preset body); no telemetry file (the view writes it, stage 5); the technician's dark storeroom is lit enough by day for the AI to walk in (rule 4 refusal rarely fires there); food is a fixed pantry and fridge that a group of four empties around day 6, with no food to find in the houses; the AI never shares a bandage or isolates the bitten.

