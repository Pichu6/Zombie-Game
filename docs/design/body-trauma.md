# Body: trauma, surgery, infection and decay

Part of [body.md](body.md). Wounds live in the part tree as a list per part, with affected layers. Bleeding, pain and infection come from each wound and lower capabilities. Field surgery solves what bandaging cannot, with noise and risk. Corpses do not disappear: they rot in phases, attract flies, spread smell and end as a visible skeleton. Almost everything is a Proposal; only decay down to the visible skeleton is Decided.

Provenance: **Decided** = the user confirmed it. **Proposal** = unconfirmed. **Open** = unanswered question.

## Trauma

- List of wounds per part, each with type, trajectory, depth and layers. The prototype stored a single active wound per region. **Proposal**.
- Minimum types: scratch, laceration, bite, bullet, fracture, burn, amputation. **Proposal**.
- Each wound accumulates bleeding, pain and infection. **Proposal**.
- Full chain to close first: impact → injury → effect → action limit → presentation → save. **Proposal**.
- Damage readable at a glance, as in Project Zomboid. **Proposal**.

## Field surgery

All **Proposal**.

| Procedure | Requirement | Risk |
| --- | --- | --- |
| Suturing a deep laceration | Thread and curved needle sterilized (fire or alcohol) | Without anesthesia: 100 % pain and 75–80 dB screams that attract zombies |
| Muffling screams | Bite a belt or thick cloth | Lowers noise to a safe level |
| Removing a bullet or shrapnel | Disinfected tweezers | If the bullet was plugging an artery, sudden massive bleeding |
| Sudden hemorrhage | Immediate tourniquet or compression | Death if not controlled |

- Slow bullets left lodged: necrosis and lead poisoning. **Proposal**.
- Free-form surgery with arbitrary cuts is a different product. If it matters, only in a separate medical mode. **Proposal**.

## Drugs

- Insulin, reconstituted antibiotics and antidotes lose effect after 24 h above 20 °C. **Proposal**.
- Cold chain with a 12 V fridge in a base or vehicle. **Proposal**.
- Dry pills lose 10 % efficacy per year in a humid environment. **Proposal**.

## Infection

- Bacterial infection and sepsis progress from open wounds, as conditions of the same system. **Proposal**.
- Swelling from infection moves morphs; clothing follows it through wrap. **Proposal** (see clothing-wrap).
- Zombie infection mechanics (transmission, timing, conversion) are not defined in the sources. **Open**.

## Death and decay

Corpses, human or zombie, do not disappear. All **Proposal**.

| Phase | Time | Effect |
| --- | --- | --- |
| Fresh | 0–24 h | Rigor: joints temporarily locked |
| Bloat from gases | Days 2–5 | Swollen abdomen; a shot or cut to the torso releases gas and infectious splatter |
| Liquefaction | Day 6+ | Soft tissues dissolve and expose the skeleton |

- Flies attracted from over 100 m. Their buzzing reveals hidden bodies. **Proposal**.
- Maggots ruin uncovered food in the same room within a few hours. **Proposal**.
- Smell (miasma) in the shelter: nausea, vomiting, less appetite, worse mood. It travels through the room graph. **Proposal**.
- Mass grave with a shovel: up to 10 bodies, costs stamina and time, removes the smell. **Proposal**.
- Pyre with firewood and fuel: destroys pathogens; black smoke visible for kilometers. **Proposal**.
- Decay advances layer by layer down to the visible anatomical skeleton. **Decided**.

## Prototype sources

cirugia-trauma-descomposicion.md, evaluacion-humano-costo-realismo.md, decisiones-generador-anatomia.md, conocimiento-produccion-personajes-superficie-evidencia.md. Also: engine research and research/clothing-wrap.md.
