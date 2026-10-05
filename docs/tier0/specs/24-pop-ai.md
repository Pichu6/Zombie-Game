# 24 · Pop AI

**Status:** Proposal. **Stage:** 8. **Needs:** [11](11-body-composition.md), [14](14-action-executor.md), [15](15-items-recipes.md), [17](17-power-network.md)–[19](19-light-field.md), [21](21-infection.md)–[23](23-walker-brain.md).

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
