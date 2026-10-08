# Body: trauma, surgery, infection and decay

Part of [body.md](body.md). Wounds live in the part tree as a list per part, with affected layers. Bleeding, pain and infection come from each wound and lower capabilities. Field surgery solves what bandaging cannot, with noise and risk. Corpses do not disappear: they rot in phases, attract flies, spread smell and end as a visible skeleton. Almost everything is a Proposal; decay down to the visible skeleton and the realistic, not gore tone are Decided.

Provenance: **Decided** = the user confirmed it. **Proposal** = unconfirmed. **Open** = unanswered question.

## Trauma

- List of wounds per part, each with type, trajectory, depth and layers. The prototype stored a single active wound per region. **Proposal**.
- Minimum types: scratch, laceration, bite, bullet, fracture, burn, amputation. **Proposal**.
- Each wound accumulates bleeding, pain and infection. **Proposal**.
- Full chain to close first: impact → injury → effect → action limit → presentation → save. **Proposal**.
- Damage readable at a glance, as in Project Zomboid. **Proposal**.

## One damage system

Tone Decided ([0015](../decisions/0015-tone-realistic-damage.md)): realistic, not gore. Shared model Proposal.

Bodies and buildings are both assemblies of layered parts, so they share one damage resolver:

- **Input:** a load with energy or force, contact area, sharpness and direction (a blade, a bullet, a bite, a fall, a crushing wall, a car).
- **Per layer:** each layer has material numbers (thickness, strength, toughness, how it tears or snaps), whether it is skin, fat, muscle, bone, organ, or plaster, stud, brick, glass. The load is spent layer by layer: it deforms, cuts, penetrates or breaks, then passes what is left inward.
- **Output:** wounds on the body part (type, depth, layers) or damage on the building part (cracks, holes, broken joint), plus sound, debris or blood. The same rules for player, pops and zombies.
- **Separation:** when a part's connection fails (a joint, a cut through bone, a wall stud), it becomes its own assembly: a severed limb is an object with its own decay; a fallen wall section is rubble that can pin and crush.
- **Crushing:** the building load graph (solved from the roof down) passes force into whoever is under a failing part, through the same resolver.

**Realistic, not gore**, in practice:

- Severe outcomes need real forces. A knife does not cut a body in half; a collapsing floor, a vehicle or machinery can. Melee amputation needs a heavy edge, skill and several blows.
- The view shows what the state says: a cut plane reveals the layers in cross-section, an open fracture shows bone, rot exposes tissue layer by layer. No extra blood or flying parts beyond what the physics produced.
- Wounds stay readable for diagnosis (where, how deep, which layers), which is gameplay: it tells you what killed you.
- A sandbox or accessibility setting can tone down the visuals without changing the simulation. Proposal.

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

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [clothing-wrap](../research/clothing-wrap.md): Clothing that follows the body, in our own engine
- [project-zomboid](../research/project-zomboid.md): Project Zomboid, as a reference for the Individual layer
- [prototype-sim-code](../research/prototype-sim-code.md): Prototype simulation code: what to keep, what to avoid

## Prototype sources

cirugia-trauma-descomposicion.md, evaluacion-humano-costo-realismo.md, decisiones-generador-anatomia.md, conocimiento-produccion-personajes-superficie-evidencia.md. Also: engine research and research/clothing-wrap.md.

## Open

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- Does the player see "sick" while a pop is incubating, given that spec 21 hides infection from the UI? (Project Zomboid shows "queasy", with the same symptoms as food poisoning.) ([project-zomboid](../research/project-zomboid.md))
