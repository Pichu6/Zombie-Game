# Pop states and actions

The catalogue of what a pop (player, survivor or zombie) can **be** and can **do**. One source of truth for the vocabulary; the rules live in the specs it points to. Written 2026-10-07 from the user's draft list, reviewed against [spec 11](../tier0/specs/11-body-composition.md) (body state and needs), [spec 13](../tier0/specs/13-capabilities.md) (capabilities), [spec 14](../tier0/specs/14-action-executor.md) (primitives and processes), [spec 24](../tier0/specs/24-pop-ai.md) (AI), [interaction](interaction.md), [activities](activities.md) and [inhabitants](inhabitants.md).

Labels: **Implemented** (in code or data today), **Proposal**, **Later** (scheduled for a tier). Everything not marked Implemented is a Proposal.

## 1. The review in short

The draft mixed six kinds of thing, and the project already has a rule for each:

| Kind | Rule | Where |
| --- | --- | --- |
| **Posture** | Stored on the body; a modifier of everything else (noise, push force, rest, reach) | spec 11 `posture` |
| **Gait** (walk, run, sprint, sneak, crawl, climb) | Modes of the one `move` primitive; speed comes from the body, never from the mode alone | spec 13 readouts, spec 14 primitives |
| **Primitive** (one body verb) | In code, with a MET cost, a capability it needs and a noise | spec 14, 17 primitives today |
| **Process** (a verb applied to a target) | In data, matching tools and inputs by property; thousands of items, few primitives | `data/tier0/processes.toml`, 31 today |
| **Body readout** (hungry, tired, exhausted) | Never stored; derived from stocks (glycogen, water, sleep pressure, exertion reserve) | spec 11 rule 8, spec 13 |
| **Mind state** (calm, afraid; happy, low) | Fear and morale are stored on the person and persist through possession; the words are bands on them | spec 11 `fear, morale`, spec 24 |

Three things the draft had as states are not states: **idle** is the AI having no intent; **in love of** is a bond edge between two people ([inhabitants](inhabitants.md#bonds)), shown as a label when its numbers cross a threshold; **rested, tired, sleepiness, exhausted** are four readings of two numbers (sleep pressure and exertion reserve).

What the draft lacked entirely: involuntary states (unconscious, bleeding, fallen), the lifecycle (alive, dying, dead, walker), combat verbs and distances, communication, perception, control and AI states, and the action phases every process already runs through.

## 2. Posture (stored, one at a time)

| Posture | Status | Notes |
| --- | --- | --- |
| standing | Implemented | Default; full push force and reach |
| crouching | Implemented | Lower noise and profile; slower; the base of sneaking |
| kneeling | Proposal | For first aid, fine work on low targets, aimed shots later; between crouch and sit |
| sitting | Implemented | Rest and eat; reach limited to arm's length |
| lying | Implemented | Sleep, crawl, being carried; prone (face down, crawl, hide) and supine (face up, treated, unconscious) as a sub-flag |
| fallen | Proposal (involuntary) | Set by unconsciousness, a shove or a collapse; the pop must get up (a timed action) |
| grappled | Proposal (involuntary) | Held by a walker or a person ([interaction](interaction.md#posture-environment-and-combat): contact and grappling are distinct distances) |
| hanging / climbing | Proposal | Only during the climb primitive; falls if manipulation drops |

A posture change is itself a short timed action with a noise (standing up from prone is slow and loud with a heavy body).

## 3. Movement: gaits and modes

All are the `move` primitive. The body sets the speed ([spec 13](../tier0/specs/13-capabilities.md): leg length, fitness, moving capability, load, stairs); the mode sets the target speed, the MET and the noise.

| Gait or mode | Status | Notes |
| --- | --- | --- |
| walk | Implemented | Comfortable speed, about 1.2–1.4 m/s by age |
| jog / run | Implemented | Sustained; drains the exertion reserve slowly |
| sprint | Proposal | Maximum speed for seconds; drains the reserve fast; ends in a forced slow-down ("the wall" when glycogen is low) |
| sneak | Proposal | Crouched walk, lowest footstep noise, slowest; the only way past a walker in the dark |
| crawl | Implemented | Lying; the only gait with a broken leg, and the walker gait for a crawler |
| limp | Derived, not chosen | A low moving capability makes any gait a limp; it shows, and it is louder |
| climb | Implemented (primitive) | Ladders, windows, fences; needs both hands free |
| vault | Proposal | A short climb over waist height, one hand free; a jump down the other way (fall risk by height and load) |
| jump | Proposal | Over a gap or down a drop; a fall damage draw by height and moving mass |
| carry while moving | Implemented (load factor) | Not a gait: load slows and tires every gait; above the carry limit, speed drops fast |
| drag | Proposal | A body or a heavy object by one end, half speed, loud; needed for the wounded and the dead |
| swim / wade | Later (Tier 2, water) | |
| ride a bicycle | Later (Tier 3, vehicles) | Also an exercise with a MET |
| drive | Later (Tier 3) | |

## 4. Body maintenance (processes on self)

| Process | Status | Notes |
| --- | --- | --- |
| eat | Implemented | From hand or fridge; stomach contents in g and kJ |
| drink | Implemented | Tap, bottle; water safety states (safe, unknown, contaminated) |
| sleep | Implemented | Lying; lowers sleep pressure; interrupted by noise and alarms |
| rest / nap | Implemented | Sitting or lying; recovers the exertion reserve; a nap lowers sleep pressure a little |
| wait | Implemented (primitive) | Standing still; the AI's idle body, the player's "hold" |
| urinate / defecate | Proposal (Tier 1 or 2) | Bladder and bowel as timers fed by intake; a need with urgency; where it is done matters: hygiene, scent (a field walkers can follow), contamination of water. The draft's "pee, poop" |
| wash | Proposal (Tier 2, water) | Hygiene state per region; lowers wound infection odds |
| dress / undress | Implemented in part (dress a wound), Proposal for clothing | Clothing layers inner, middle, outer ([clothing research](../research/clothing-wrap.md)); each change is a timed action by layer |
| equip / unequip | Implemented in part (take, hands) | Hands and carried slots; what is in hand decides which processes qualify |
| treat a wound | Implemented | Clean, disinfect, dress, splint, tourniquet; on self or another body |
| take medicine | Proposal | Painkillers, antibiotics, with medicine clocks |
| warm up / cool down | Later (Tier 1, temperature) | At a hearth, in shade, by dressing |

## 5. Objects and the world (processes on a target)

| Process | Status | Notes |
| --- | --- | --- |
| open / close | Implemented (doors opened on the way, closed behind) | Doors, windows, containers, curtains; a locked one greys out with the reason |
| lock / unlock | Proposal | Needs the key or a pry; a zombie's house key opens its real house |
| take / drop / place | Implemented | Through the ledger in phases; never duplicated or lost |
| carry / haul | Implemented (job) | Move items to stockpiles |
| push / pull | Implemented (primitive) | Move furniture, shove a door, drag a body; push force from the body |
| pry | Implemented | By lever property; crowbar or screwdriver |
| cut | Implemented (primitive) | Saw, knife; by edge property |
| strike | Implemented | Hammer a board, break a chair; also the combat strike |
| fine work | Implemented (primitive) | Wiring, stitching, dressing; dark slows it |
| board / barricade | Implemented | Windows and doors, part by part |
| salvage | Implemented | Breaks an assembly into parts, conserving mass |
| repair / build / craft | Implemented in part (repair job, barricade); crafting by recipe Proposal | Quality from parts, capped by skill and tool ([items and crafting](items-crafting.md)) |
| switch on / off, start / stop, refuel, reset breakers, switch mains / generator | Implemented | The generator problem |
| hang a curtain | Implemented | Light leak |
| inspect | Proposal (the draft's "investigate / inspection") | Look closely at a thing or a body: gives an **assessment** (what this pop believes about its condition, with date and confidence, [inhabitants](inhabitants.md#learning-and-specialisation)); knowledge decides what is seen. The "Inspect" interaction type in [activities](activities.md) |
| search | Implemented (primitive) | A room or container; time by size; noise if hurried |
| study | Proposal (the draft's "research / study") | A manual, a mentor or an instance of a thing; gives knowledge, never practice ([inhabitants](inhabitants.md#learning-and-specialisation)); takes hours |
| read / write a note | Proposal (Tier 2) | Leaves a world fact others can find |
| shout | Implemented | Loud, far, draws walkers; calls others |

## 6. Exercise

One primitive (`exercise`, Implemented) with a MET and a training stimulus; the kinds are data. Tier 0 counts one stimulus for the whole body; later each kind trains its muscle groups ([body](body.md#muscle-and-fat-shape-versus-amount)).

| Kind | Needs | Trains (later) |
| --- | --- | --- |
| squat, push-up, sit-up, plank, burpee | Nothing | Legs, chest and arms, trunk |
| pull-up | A bar or beam | Back and arms |
| row, curl, press | Weights or improvised weights (a jerrycan) | Back, arms, shoulders |
| jump rope, running on the spot, stairs | Nothing | Fitness (VO₂max) |
| stationary bicycle | A bicycle on a stand | Fitness, legs |

Climbing, vaulting, carrying and jumping are movement, not exercise, but they count as stimulus the same way (minutes of heavy work).

## 7. Combat

From [interaction](interaction.md#posture-environment-and-combat): a held combat stance, four distances (out of reach, weapon reach, contact, grappling), aiming that never guarantees a hit.

| Verb | Status | Notes |
| --- | --- | --- |
| combat stance | Proposal | Slower, torso toward the threat; combat forces 1× time |
| strike | Implemented | By body strength and the tool's properties; every hit is a sound |
| shove | Proposal | Push a walker or a person back a cell; from push force |
| grapple / break free | Proposal | Contact distance; walkers grab, people break free by strength and a draw |
| block / brace | Proposal | Hold a door or a shield item against push |
| throw | Proposal (Tier 1) | Anything in hand; noise where it lands (a lure) |
| aim / shoot / reload | Later (Tier 1 stretch: the loud comfort item) | |
| put down | Implemented | Destroy the brain of a corpse before it rises |
| flee | Implemented (AI) | Panic runs to the nearest safe room; the player just runs |

## 8. Communication and social (verbs toward people)

From [world simulation §9](world-simulation.md#9-communication-decided-as-intent-model-proposal) and [inhabitants](inhabitants.md#bonds). Player and AI use the same verbs.

| Verb | Status | Notes |
| --- | --- | --- |
| talk, whisper, shout | Shout Implemented | Voice carries by the one signal-to-noise rule; whisper is safe, shout is a lure |
| ask / tell / report | Implemented in part (honest or secretive about bites) | Messages become beliefs with source and age |
| promise, lie, threaten, help, give | Proposal (Tier 2) | Change bonds (trust, debt, fear, resentment), never affection directly |
| invite / recruit | Implemented | Found survivors join |
| order / refuse | Implemented | Squad orders; refusal with a stated reason; fear decides |
| trade | Later (Tier 2) | The "Exchange" interaction type |
| radio: call, listen, tune | Later (Tier 1, the radio to repair) | |
| signal: light, noise, marks | Proposal | Anything that leaks is a signal, on purpose or not |

## 9. Body readouts (derived, never stored)

Each word is a band on a number the body already keeps ([spec 11](../tier0/specs/11-body-composition.md) rule 8, [spec 13](../tier0/specs/13-capabilities.md)). The UI shows the word; the AI reads the number.

| Readout | Source number | Bands (Proposal) |
| --- | --- | --- |
| hunger | glycogen and stomach | fed, peckish, hungry, starving, wasting |
| thirst | water deficit (% body mass) | fine, thirsty, dehydrated (2 %), critical |
| sleepiness | sleep pressure S | rested, drowsy, tired, falling asleep (microsleep risk) |
| exhaustion | exertion reserve | fresh, winded, exhausted, collapsing |
| load | load / carry limit | light, loaded, overloaded |
| pain | wound list | none, sore, in pain, agony |
| bleeding | wound list | none, bleeding, bleeding badly |
| fever | core temperature | none, feverish, burning |
| consciousness | capability | alert, drowsy, stunned, unconscious |
| mobility | moving capability | fine, limping, crawling only, immobile |
| hands | manipulation | fine, one-handed, no hands |
| worn down (the draft's "burnt out") | days of sleep debt plus heavy work (weekly drift) | Proposal: a slow state that lowers morale recovery and raises error odds; recovers with rest days |
| infection stage | pathogen clocks | **hidden**: shown only as tells (sweating, pallor, a dressed wound); the debug inspector shows the truth |

## 10. Mind states

Stored: **fear** and **morale**, 0–1, on the person ([spec 11](../tier0/specs/11-body-composition.md)); they persist when the player possesses or releases the body. Walkers have none.

| State | Source | Bands (Proposal) |
| --- | --- | --- |
| fear | threats seen and heard, darkness, pain | calm, alert, afraid, panicking (drops the action and runs) |
| morale | hunger, sleep loss, losses; light, hot food, company | happy, content, low, despairing (refuses more, works slower) |
| grieving | a loss event with a tie | Proposal (Tier 2): a timed morale drag, stronger with the bond |
| lonely | hours without company | Proposal (Tier 2) |
| bonds to a person | edges: familiarity, affinity, trust, respect, fear, resentment, dependence, optional attraction ([inhabitants](inhabitants.md#bonds)) | Labels on thresholds: friend, trusted, feared, resented, **in love with** (affinity and attraction high, the draft's "in love of"); always asymmetric |
| disposition traits | from the census, fixed | brave or timid, honest or secretive (Implemented) |

## 11. Control and AI states (not the body)

| State | Status | Notes |
| --- | --- | --- |
| idle | Implemented | No intent; the body waits or rests |
| intent queued | Implemented | Ordered intents per actor |
| acting: approach → prepare → work → finish | Implemented (approach and work) | Every process runs these phases; effects land only in the phase that causes them |
| interrupted | Implemented | Death, unconsciousness, lost reach, target changed, capability below need, a slow-down event |
| on a job / under an order / following | Implemented | Jobs by priority; orders override jobs |
| refusing | Implemented | With a reason: dark room, open ground with walkers in sight, live circuit |
| panicking / fleeing | Implemented | Fear above threshold |
| possessed | Implemented | The player holds the body; the AI resumes from the same state on release |
| hiding a bite | Implemented | The one false belief in Tier 0 |

## 12. Lifecycle

alive → (unconscious) → dying (infection stage, hidden) → dead → walker → destroyed. Stages only move forward ([spec 21](../tier0/specs/21-infection.md)); a corpse whose brain is destroyed before rising stays dead; the census conserves every person through all of it ([spec 22](../tier0/specs/22-census.md)).

## 13. Perception states (derived each tick)

| State | Notes |
| --- | --- |
| sees / hears a threat | From the fields and sight; drives fear and the AI |
| in the dark | Fine work slows; fear rises; refusal case |
| noticed by a walker | The walker's attention; sneaking and darkness lower it |
| in cover / behind a wall | Later (Tier 2, 3rd person peeking) |

## 14. Open

- Bands and thresholds for every readout (section 9 and 10) are starting values to tune in playtests, each with a source where one exists (sleep: Van Dongen 2003; dehydration: Sawka 2007).
- Whether "worn down" is a real state or just morale over days.

Answered in [0045](../decisions/0045-open-questions-answered-2.md): bladder and bowel are automatic for pops and orderable by the player in Individual (Tier 2 with water as a Proposal); fear also widens aim spread and raises work-error odds (nerves, no new number); temperature is suffered by the body in Individual from Tier 1.
