# 11 · Body: composition and needs

**Status:** Implemented (stage 3; numbers are Proposal, tuned against the bands below). **Stage:** 3. **Needs:** [01](01-quantity.md)–[05](05-clock.md).

## Purpose

The one body model for the player, every pop and every zombie ([0007](../../decisions/0007-body.md), [0010](../../decisions/0010-concept-v2.md)): masses in kg, energy, water, sleep and exertion. Needs are readouts of this state, never standalone bars. It is the source of "you starved", "you were dehydrated", "you collapsed from exhaustion".

## State

| Field | Unit | Conserved | Notes |
| --- | --- | --- | --- |
| fat, muscle, structure | g | Through the ledger | Three exclusive compartments ([body](../../design/body.md#masses-and-composition)); total and % derived |
| height, sex, age | m, enum, years | — | Set at creation |
| glycogen | g | Yes | Short-term energy store, ~400–500 g full |
| stomach | g and kJ of contents | Yes | Food eaten, emptied over hours |
| body water | mL | Yes | Its own compartment: total mass = fat + muscle + structure (dry) + water, so no gram is owned twice; deficit = normal − current, derived |
| posture | stand, crouch, sit, lie | — | Used by push force ([10](10-fortification.md)), noise and rest |
| sleep pressure S | 0–1 | — | Two-process model |
| exertion reserve | 0–1 | — | Short-term stamina; recovers at rest |
| fitness | VO₂max, mL/kg/min | — | Drifts weekly with activity |
| training stimulus | minutes of heavy work today | — | One value for the whole body in Tier 0 (muscle is one compartment); per group later |
| core temperature | °C | — | Tier 0: only fever moves it |
| fear, morale | 0–1 each | — | Mind state of the person, not of the AI: persists through possession ([0005](../../decisions/0005-finite-world-parity.md)). Fear rises with threats seen and heard, darkness and pain; morale falls with hunger, sleep loss and losses, rises with light, hot food and company. Walkers have none |

## Rules

1. Energy out per body step = basal rate (Mifflin-St Jeor from mass, height, age, sex) + activity (MET of the current action × moving mass, where moving mass = body + clothes + load) + load carriage (Pandolf) + fever cost.
2. Energy in = stomach emptying into glycogen; overflow stored as fat. Each step is a declared ledger conversion ([03](03-stock-transfer.md) rule 4).
3. Deficit draws glycogen first, then fat and muscle by the Forbes partition (more muscle is lost when body fat is low), again as declared conversions into `world.metabolised`.
4. Water: losses = baseline + sweat (rises with exertion) + blood loss share; drinking refills. Effects start at ~2 % body-mass deficit.
5. Sleep pressure rises while awake and falls while asleep (Borbély two-process model; the circadian term comes after Tier 0). High S lowers consciousness and raises the chance of involuntary sleep.
6. Weekly drift: muscle groups gain with training stimulus plus protein and energy surplus, lose with disuse or deficit; VO₂max follows recent activity. Changes are small per day and visible over a week.
7. "The wall": the exertion reserve's ceiling falls as glycogen runs low, so a hungry body tires sooner.
8. Needs shown to the player and read by the AI (hunger, thirst, tiredness, exhaustion) are derived from glycogen and stomach, water deficit, S and exertion reserve.
9. Zombies use the same state with a pathogen-driven profile ([21](21-infection.md)): no sleep, no hunger drive from glycogen, water and tissue still consumed.

## Invariants

- fat ≥ 0, muscle ≥ 0, structure > 0, total ≤ 1000 kg.
- Mass in = mass out + change in compartments (food and water in; metabolised, sweat and blood out through sinks), with every conversion in the ledger.

## Tests

- Total fast: a 75 kg adult at rest without food (with water) loses mass inside the Cahill / Hall band over 14 days. (A3)
- Semi-starvation: ~1600 kcal/day with light work follows the Minnesota experiment's early weight curve. (A3)
- No water at 20 °C and light work: critical state between day 2 and day 4. (A3)
- Low glycogen lowers the exertion ceiling: the same run ends sooner after a day without food. (A3)
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
| Lethal dehydration band (% body mass) | Adolph, Physiology of Man in the Desert (1947); to check |
| Glycogen and endurance ("the wall") | Bergström et al. (1967) |
| Prototype numbers not to reuse | Muscle loss at 4000 kcal/kg and death at ~3 % water loss were wrong; see [prototype sim code](../../research/prototype-sim-code.md) |
| Sleep pressure | Borbély (1982) two-process model; Van Dongen et al. (2003) |
| Muscle gain and disuse loss | Resistance training and bed-rest studies (to collect; tune) |

## Model as implemented (stage 3)

Code: `engine/sim/src/body.cpp`; scenarios: `engine/sim/src/body_lab.cpp` (`zs_headless --body-lab`).

- **Units.** The ledger holds tissues in mg, water and blood in µL and energy in J, and every continuous flow keeps a saved remainder ([01](01-quantity.md) rule 2).
- **Composition at creation.** Fat = fat share × mass. Muscle = 0.5 (male) or 0.45 (female) of fat-free mass. Mobile water = 0.2 × fat-free mass. Blood from Nadler. Glycogen store = 100 g + 12 g per kg of muscle. Structure is the rest and must be positive.
- **Energy.** Power = Mifflin-St Jeor BMR × MET at rest (0.95 asleep), Pandolf when walking, ~4 J/kg/m when running, + 13 % of BMR per °C of fever. The stomach empties first-order (τ 90 min) into glycogen up to the store, the rest into fat. Spending draws glycogen first, then fat and muscle by the Forbes partition (39.5 kJ/g fat, 7.6 kJ/g lean).
- **Water.** Losses = 1.6 L/day baseline + sweat 0.10 L/h per MET above 1.5. Each gram of glycogen holds 3 g of water, so an empty store lowers normal water and the kidneys excrete the excess (τ 45 min). "Critical" = 8 % deficit; death at 15 %.
- **Sleep.** S rises with τ 18.2 h awake and falls with τ 4.2 h asleep. Consciousness from sleep is 1 up to S = 0.6, then falls to 0.4 at S = 1. Involuntary sleep at S ≥ 0.98; natural waking at S ≤ 0.1.
- **Exertion.** Drains above half of VO₂max (0.1 per minute at VO₂max), recovers 0.05 per minute below it. The ceiling is glycogen / 45 % of the store, clamped to 0.3–1 ("the wall").
- **Daily drift.** VO₂max +0.05 with 60 active minutes, +0.05 more with 30 heavy minutes, −0.03 under 20 active minutes. 30 heavy minutes add 0.1 % muscle (from glycogen); a day under 20 active minutes and no heavy work loses 0.1 %.
- **Death.** Starvation when muscle falls below 60 % of its starting mass.
- **Mind.** Fear follows 0.6 × pain (τ 30 min); morale follows 0.75 − 0.25 hunger − 0.25 tiredness − 0.2 pain (τ 8 h). Threats, light and company join in stages 7–8.

**Lab results (seed 1, GCC and Clang):** 14-day fast −6.7 kg; 1600 kcal/day with light work −3.6 kg in 14 days; no water reaches 8 % deficit on day 2.8; 40 h awake gives consciousness 0.55; the 110 kg body burns 1.6× the 60 kg body on the same haul and tires sooner; running to exhaustion lasts 0.27× as long after 30 h without food.

## Not in Tier 0

Temperature and clothing insulation (weather comes after), detailed digestion and macronutrients, organ models, alcohol and drugs.
