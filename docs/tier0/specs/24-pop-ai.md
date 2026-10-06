# 24 · Pop AI

**Status:** Implemented (stage 8, headless; numbers are Proposal). **Stage:** 8. **Needs:** [11](11-body-composition.md), [14](14-action-executor.md), [15](15-items-recipes.md), [17](17-power-network.md)–[19](19-light-field.md), [21](21-infection.md)–[23](23-walker-brain.md).

## Purpose

What a pop does when nobody controls it: look after its body, do its jobs, obey orders, and panic or refuse in a few readable situations. It picks intents only; the body and executor do the rest (parity, [0010](../../decisions/0010-concept-v2.md)). The AI is cautious and average on purpose, so possession matters.

## State

| Field | Notes |
| --- | --- |
| mood | reads fear and morale from the person ([11](11-body-composition.md)); the AI never owns them |
| job priorities | 6 jobs × priority, from the job board |
| order | current squad order: go, hold, follow, attack (overrides jobs) |
| disposition | a few fixed traits from the census: brave/timid, honest/secretive |
| knowledge | a few procedures: mechanic, electronics, first aid (from census job) |

## Rules

1. Utility choice each decision step (not every tick): scores urgent body needs (bleeding, thirst, hunger, sleep, wound care), then order, then jobs by priority, then idle. Ties broken by fixed order.
2. Jobs (Tier 0): haul (move items to stockpiles), refuel (keep the generator in fuel when it runs), repair (generator, radio, doors), cook, guard (watch a door or window, wake others), barricade.
3. Squad orders: go, hold, follow, attack, given from the command view; they persist until done or cancelled.
4. Panic and refusal, three situations: entering a dark room with no light; crossing open ground with walkers in sight; touching a live circuit. The AI refuses with a stated reason; an order can push it only if fear is below a threshold. A possessed pop can do it, but fear still widens aim spread and slows fine work.
5. Panic: fear above a threshold drops the current action and runs to the nearest safe room.
6. Found survivors join when the player uses "invite" and the pop is not hostile; Tier 0 has no trust model.
7. An infected pop with the secretive trait hides the wound (says "just a cut"); an honest one reports it. This is the one false belief in Tier 0; otherwise beliefs inside the building equal the truth.
8. Unpossessing never resets state: the AI takes over the same body, pain, fear, load and job.

## Invariants

- AI intents go through the executor only.

## Tests

- **AI-only run:** 4 pops with no player for 14 days, 50 seeds. They should die of the same ignored things a careless player does (log of causes), and survive often enough to show the rules are survivable. (A3, plus the logged causes)
- Each refusal situation triggers with its reason. (A1)
- Possess, act, release: AI continues the same job with the same state. (A1)

## Calibration

No real source for utility weights; tuned so the AI is average and cautious. Fear in darkness and panic rarity follow disaster behaviour research (panic is rare; Quarantelli) as a qualitative check.

## Not in Tier 0

Relationships, trust, complaints, learning, dialogue, bandits, belief stores.

## Model as implemented (stage 8)

Code: `engine/sim/src/pop_ai.cpp` (`PopAI`, a saved System updated after Bodies), `zs_headless --group [--seed N] [--days D] [--trace F] [--seeds N]`.

- **Decision step** every 5 s per pop not possessed. Wants in rank order: fight or flee (0); put down the group's dead, treat a wound (1); drink at thirst 0.4 (2); eat at hunger 0.5 when there is food it can start on (3); sleep at tiredness 0.75 by day, 0.35 at night, upstairs if there is one (4); the order (5); pick up a weapon lying about, or ask a found survivor the group knows about to join (6); jobs by priority 1-3 (7); idle (9). A more urgent want cancels the current plan. A want that cannot be planned waits 5 game minutes. Each want becomes a short plan of process steps sent through `World::queue_intent`.
- **Fight or flee:** fear ≥ 0.7 (brave 0.9) panics: drop everything and run to the safest reachable room (one `Panicked` per scare), unless armed and already in reach. A walker in reach: strike if armed (weapon score ≥ 0.3), else flee. Armed and brave, or with fear below 0.5: step up to a walker within 4 m indoors. Unarmed: back off from one within 8 m. Both only when the walker is in the same room (or both outdoors): one beyond a wall or window is watched, not fled (stage 9). An armed pop that is not panicking answers a struggle within 15 m on its level (a walker in reach of another pop).
- **Put down the dead:** an armed pop strikes the head of a dead group member until its brain is destroyed, so the infected do not rise inside the house. One pop per corpse.
- **Jobs:** refuel keeps the generator going from 10:00 to 16:00 (stage 9; was 08:00-18:00) (switch the panel, refuel below 0.3 of the tank when a fuel can is at hand, start it while there is fuel) and stops it at night, when its noise carries farthest; barricade boards the nearest unboarded window on its level (one pop per window), taking loose planks or breaking up a chair that still has some; guard shouts the sleepers awake when walkers are in sight; search (stage 9, 7th job) walks by day to a house not yet searched, learns who hides there and brings back its fuel can; repair (stage 9) is the radio: whoever knows electronics repairs it, and once a day between 12:00 and 14:00, with power, someone listens. One pop per job target. A careless switch (spec 26 test) runs the generator day and night and never boards up or keeps watch.
- **Refusals** (rule 4) are checked for orders and for job and order steps, never for fight, flee, treat, putting down the dead, needs or idle (so getting a weapon can be refused): open ground with walkers in sight when leaving the indoors; a dark room (< 1 lux) it is not already in. An insisted order goes through below fear 0.5. Refused orders log `PopRefused` with the reason.
- **Traits:** brave with p 0.3, honest with p 0.7, drawn per body. Honest pops log `BiteReported` once; secretive ones never do.
- **Possession:** the possessed pop gets no AI steps; on release the AI decides again with the same mind, body and job board.

**AI-only run (A3, 4 pops, 14 days, 50 seeds):** 57 of 200 alive at the end; someone alive in 27 of 50 runs. Causes: 121 infection from a bite, 22 bled out. Most bites happen in close fights at windows and doors; infection then kills over 1-2 days.

**Known gaps:** haul, cook and repair have no processes in Tier 0; the live-circuit refusal has no process that touches wiring; no command view until stage 5 (orders and the job board are API calls); walkers use their own mover, so pops do not body-block them; no first aid knowledge or skills; the AI does not isolate the bitten or show mercy; fights are always melee (no cover, no shove).
