# 11 · Body: composition and needs

**Status:** Proposal. **Stage:** 3. **Needs:** [01](01-quantity.md)–[05](05-clock.md).

## Purpose

The one body model for the player, every pop and every zombie ([0007](../../decisions/0007-body.md), [0010](../../decisions/0010-concept-v2.md)): masses in kg, energy, water, sleep and exertion. Needs are readouts of this state, never standalone bars. It is the source of "you starved", "you were dehydrated", "you collapsed from exhaustion".

## State

| Field | Unit | Conserved | Notes |
| --- | --- | --- | --- |
| fat, muscle, structure | g | Through the ledger | Three exclusive compartments ([body](../../design/body.md#masses-and-composition)); total and % derived |
| height, sex, age | m, enum, years | — | Set at creation |
| glycogen | g | Yes | Short-term energy store, ~400–500 g full |
| stomach | g and kJ of contents | Yes | Food eaten, emptied over hours |
| body water | mL | Yes | A stock, part of structure mass (1 mL = 1 g); deficit = normal − current, derived |
| sleep pressure S | 0–1 | — | Two-process model |
| exertion reserve | 0–1 | — | Short-term stamina; recovers at rest |
| fitness | VO₂max, mL/kg/min | — | Drifts weekly with activity |
| training stimulus | per muscle group (arms, legs, trunk) | — | Accumulates from heavy work |
| core temperature | °C | — | Tier 0: only fever moves it |
| fear, morale | 0–1 each | — | Mind state of the person, not of the AI: persists through possession ([0005](../../decisions/0005-finite-world-parity.md)). Fear rises with threats seen and heard, darkness and pain; morale falls with hunger, sleep loss and losses, rises with light, hot food and company. Walkers have none |

## Rules

1. Energy out per body step = basal rate (Mifflin-St Jeor from mass, height, age, sex) + activity (MET of the current action × moving mass, where moving mass = body + clothes + load) + load carriage (Pandolf) + fever cost.
2. Energy in = stomach emptying into glycogen; overflow stored as fat. Each step is a declared ledger conversion ([03](03-stock-transfer.md) rule 4).
3. Deficit draws glycogen first, then fat and muscle by the Forbes partition (more muscle is lost when body fat is low), again as declared conversions into `world.metabolised`.
4. Water: losses = baseline + sweat (rises with exertion) + blood loss share; drinking refills. Effects start at ~2 % body-mass deficit.
5. Sleep pressure rises while awake and falls while asleep (Borbély two-process model with a circadian term). High S lowers consciousness and raises the chance of involuntary sleep.
6. Weekly drift: muscle groups gain with training stimulus plus protein and energy surplus, lose with disuse or deficit; VO₂max follows recent activity. Changes are small per day and visible over a week.
7. Needs shown to the player and read by the AI (hunger, thirst, tiredness, exhaustion) are derived from glycogen and stomach, water deficit, S and exertion reserve.
8. Zombies use the same state with a pathogen-driven profile ([21](21-infection.md)): no sleep, no hunger drive from glycogen, water and tissue still consumed.

## Invariants

- fat ≥ 0, muscle ≥ 0, structure > 0, total ≤ 1000 kg.
- Mass in = mass out + change in compartments (food and water in; metabolised, sweat and blood out through sinks), with every conversion in the ledger.

## Tests

- Total fast: a 75 kg adult at rest without food (with water) loses mass inside the Cahill / Hall band over 14 days. (A3)
- Semi-starvation: ~1600 kcal/day with light work follows the Minnesota experiment's early weight curve. (A3)
- No water at 20 °C and light work: critical state between day 2 and day 4. (A3)
- 40 hours awake: consciousness readout ≤ 0.6 (the starting threshold for slowed work and microsleep risk, tuned against Van Dongen et al. 2003 and Dawson and Reid 1997). (A3)
- Two bodies with the same muscle mass and VO₂max, at 60 kg and 110 kg total, doing the same haul: the heavier burns more and tires sooner on stairs. (A3)

## Calibration

| Value | Source |
| --- | --- |
| Basal rate | Mifflin-St Jeor equation (1990) |
| Activity cost | Compendium of Physical Activities (Ainsworth et al. 2011) |
| Load carriage cost | Pandolf, Givoni and Goldman (1977) |
| Energy density of fat and lean loss, Forbes partition | Hall (2008), Hall et al. (2011), Forbes (2000) |
| Total fasting | Cahill (1970), Hall et al. (2011) |
| Semi-starvation | Keys et al., Minnesota Starvation Experiment (1950) |
| Dehydration effects | ACSM position stand on fluid replacement (Sawka et al. 2007) |
| Sleep pressure | Borbély (1982) two-process model; Van Dongen et al. (2003) |
| Muscle gain and disuse loss | Resistance training and bed-rest studies (to collect; tune) |

## Not in Tier 0

Temperature and clothing insulation (weather comes after), detailed digestion and macronutrients, organ models, alcohol and drugs.
