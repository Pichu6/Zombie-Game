# Body simulation

Every human and every zombie has a simulated body. It is a tree of parts with tissue layers, masses in kg and a slow metabolism. Wounds, infection and decay are states of that same body. The game reads derived capabilities (move, manipulate, see, breathe), not individual organs. The view draws the state with morphs, texture masks and inner layers revealed by damage. The state is the authority; the mesh never stores physiology. Player and NPC use exactly the same simulation.

Provenance: **Decided** = the user confirmed it in the prototype. **Proposal** = unconfirmed technical idea. **Open** = unanswered question.

## Principles

| Rule | Status |
| --- | --- |
| Real human anatomy is the main reference for bones, muscles and proportions. | Decided |
| Filmed or photographed real people are the primary reference; other games are secondary. | Decided |
| Realistic variety: thin, fat and muscular bodies, different breasts, buttocks and faces. | Decided |
| Body diversity is gameplay: the body changes with exercise and diet during the game. | Decided |
| The creator sets the initial body state. Strength, endurance and the rest emerge from that state. | Decided |
| Labels like "skinny" or "fit" describe the current state. They are not fixed bonuses. | Decided |
| Muscles deform, grow and atrophy. Fat grows, shrinks and deforms passively. | Decided |
| Fat also affects the face and double chin, without scaling the head or erasing features. | Decided |
| Layers: animatable skeleton, outer skin with morphs, light interior revealed by wounds or rot. | Decided |
| Do not simulate muscles or organs per vertex in real time. | Decided |
| The anatomical skeleton is rig structure and also the final visible result of decay. | Decided |
| Soft, slightly translucent tissues. Visible heart and lungs beat and breathe according to state. | Decided |
| Distinguish soft from stocky bodies, redness, vascularity and fat distribution. | Decided (requested) |
| Woman and man each have their own anatomy; never a scaled man with added breasts. | Decided |
| Eyes belong to the organ layer, not the skeleton. | Decided |
| Finite population; player and NPC have the same simulation and capabilities. | Decided |
| Reuse existing solutions when they work (libraries, open bases). | Decided |

## Architecture

Single flow: persistent state → capabilities → allowed actions → pose and presentation.

- The state lives in the simulation core, with no dependency on the renderer. **Proposal** (engine research).
- Changing camera, unloading the view or moving away does not alter metabolism, identity or injuries. **Decided** (parity and finite population).
- The game view and the medical view read the same body. They never create independent bodies. **Proposal**.
- Updates by event (hit, food) or by slow tick; never every frame. **Proposal** (research).
- Detail by distance: near, everything; loaded district, slow tick; rest, statistical while keeping masses and identity. **Proposal**.
- Conflict: the prototype built the playable body from an anatomical atlas, inside out. The research limits that to the medical view. **Proposal**: part tree for play, atlas only for inspection.

## Part tree

- Part tree with stable IDs shared by rig, wounds, clothing, organs and save data. **Proposal**.
- The prototype used 17 logical regions. It works as a starting point. **Proposal**.
- Each region defines: anatomical side, joint, deformation control, surface, tissues and anchors. **Proposal**.
- Separate visible bone, biomechanical joint and deformation control. They are not the same. **Proposal**.
- Left/right side is validated against anatomical reference, never by name suffix. **Proposal**.
- Design IDs, units, joints and anchors for the whole body from the start. Extend geometry by region. **Proposal**.
- Reserve cavities and anchors for organs, wounds and deterioration from the beginning. **Proposal**.

## Tissue layers

Layered model in the style of Dwarf Fortress: each part has skin, fat, muscle, bone and organs where relevant. **Proposal** (research).

| Layer | Data per region | Effect |
| --- | --- | --- |
| Skin | Integrity, dirt, blood, perfusion (color) | Barrier, infection, appearance |
| Fat | kg and rest thickness per deposit | Protects, weighs, insulates, slows |
| Muscle | kg, rest volume, training, fatigue | Strength, endurance |
| Fascia | Displacement limits | Prevents fat behaving as free liquid |
| Bone | Integrity, fracture | Support, movement |
| Organs | Individual functional state | Vital capabilities |

- Hits are resolved layer by layer: trajectory, depth and affected tissues. **Proposal**.
- Fat also fills internal spaces; it is not a uniform shell. **Proposal**.

## Masses and composition

Three mutually exclusive compartments in wet kg: fat, muscle and structural (all other tissues). **Proposal** (implemented and tested in the prototype).

- Total weight and percentages are derived. They are never stored or converted back. **Proposal**.
- Water is already included. Do not add hydration again. **Proposal**.
- Atomic edit: fat ≥ 0, muscle ≥ 0, rest > 0, sum ≤ 1000 kg (technical limit, not clinical). **Proposal**.
- Height: data domain 0.1–4 m. Changing height keeps the kg. **Proposal**.
- Height scales skin, rig and interior together. Scaling only the skin breaks joints. **Proposal**.
- Body mass and moving mass are different. Carried load never becomes tissue. **Proposal**.
- Basal metabolism and thermal surface use body mass. Active work uses moving mass with gear. **Proposal**.
- When redistributing by region, keep total mass. Do not count kg twice. **Proposal**.
- Bleeding, dehydration and amputation must subtract mass. The prototype did not do this. **Proposal**.
- Digestive contents get explicit mass if they count in the total. Never convert kcal into liters. **Proposal**.

## Muscle and fat: shape versus amount

| Phenomenon | Muscle | Fat |
| --- | --- | --- |
| Fast shape change | By length, contact and activation; keeps approximate volume | Squashes, shears and recovers; passive |
| Slow amount change | Regional hypertrophy or atrophy from load, rest and nutrition | Regional storage or mobilization from energy balance |
| Consequence | Strength separate from activation, fatigue and training | Mass, inertia and protection; never an actuator |

- Bending an elbow does not change mass. Compressing fat does not remove it. **Proposal**.
- Separate transient fatigue from persistent atrophy. **Proposal**.
- Separate conditioning and neuromuscular adaptation from muscle size. **Proposal**.
- Do not derive strength only from visible volume, nor dexterity from large muscles. **Proposal**.
- Fat distribution (android, gynoid) is a bias. It never takes a region to zero. **Proposal**.

## Metabolism

Calories, macronutrients, hydration and digestion. Fat and muscle change over days, in the style of SCUM. **Proposal** (research).

- Losses and gains are applied directly in kg. **Proposal**.
- All prototype coefficients were authored, not calibrated. Passing tests does not calibrate them. **Open**.
- Heart rate, ventilation and recovery are state numbers. Organ animation reads them. **Proposal**.
- Do not use pulse as a stand-in for breathing rate and depth. **Proposal**.
- Circulation as reserve, perfusion and main vascular routes. The prototype used a global 5000 ml. **Proposal**.
- Nerves as connections from regions to centers: motor capacity and sensation. **Proposal**.
- Stomach: volume, filling and emptying slightly change the belly, without creating fat. **Proposal** (user idea).
- The belly also depends on intestines, gas, diaphragm, wall and fat. Do not scale only the stomach. **Proposal**.

## Capabilities

Capabilities in the style of RimWorld: move, manipulate, consciousness, see, breathe, pump blood. They are computed from the parts. AI and UI read capabilities. **Proposal** (research).

- Fulfills the decision that strength and endurance emerge from the body. **Decided** (principle) / **Proposal** (method).
- Formulas and calibration are still pending. **Open**.

## Trauma, surgery, infection and decay

See [body-trauma.md](body-trauma.md).

## From state to image

| State | Representation |
| --- | --- |
| Height | Common scale for skin, rig and interior |
| Fat and muscle per region | Morphs as sparse deltas; correctives per combination |
| Swelling, gas, infection | Regional morphs |
| Dirt, blood, grease, sweat, wetness | Masks in the body's common UV |
| Surface wounds | Mask and material |
| Deep wounds, amputation | Prepared wound pieces and regional cuts |
| Interior | Light layer with the same IDs, visible only through damage or rot |
| Vital rhythms | Organ animation phases driven by state |

- All bodies share one topology and one UV; they vary by morphs. **Proposal** (clothing-wrap).
- The kg → morph mapping is experimental. Record its domain and warn when it saturates. **Proposal**.
- The view limits only its representation; it never clips the kg. **Proposal**.
- Visual correctives stay outside the mass state. **Proposal**.
- Translucency through subsurface scattering, not uniform transparency. Different for skin, cloth, bone and organ. **Proposal**.
- Hidden layers do not update deformation. Hiding does not free memory: load on demand. **Proposal**.
- Soft tissue dynamics (physics in SI units with gravity) only near the camera. **Decided** (direction) / **Proposal** (near level only).
- The prototype's tissue coefficients were not calibrated. **Open**.

## Shoulder and movement

- The scapula and back move with the shoulder and arm. **Decided**.
- Three separate scapular families: protraction/retraction, elevation/depression, rotation when raising the arm. **Proposal**.
- Movement through animation and IK compatible with joints and capabilities. **Proposal**.
- Muscles that generate movement through forces: out of scope. **Proposal**.

## Women and variety

- Shared logic; own data per base: pelvis, thorax, lengths, rest poses, insertions, deposits. **Proposal**.
- Vertex correspondence within each morph family, not between man and woman. **Decided**.
- No universal strength multipliers by sex. Continuous individual variation, not a boolean. **Proposal**.
- Breast tissue and sex-specific organs as later components. **Proposal**.

## Open

- Is the game's hero system the body or the infrastructure?
- How much interior anatomy is visible in play versus the medical view?
- Formulas and calibration for metabolism, capabilities and tissue.
- Zombie infection rules, and whether the zombie keeps the inhabitant's identity.

## Prototype sources

bases-simulador-corporal.md, bases-simulador-corporal-arquitectura-regional.md, bases-simulador-corporal-base-femenina.md, bases-simulador-corporal-contratos-masas.md, bases-simulador-corporal-tejidos-esqueleto.md, decisiones-tejidos-esqueletos.md, decisiones-generador-anatomia.md, evaluacion-humano-costo-realismo.md, conocimiento-produccion-personajes-reposo-movimiento.md, conocimiento-produccion-personajes-superficie-evidencia.md, movimiento-capas-rendimiento.md. Also: engine research and research/clothing-wrap.md.
