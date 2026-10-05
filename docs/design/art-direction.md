# Art direction and visual production

The visual goal is realism with fluid animation and readable physiology. Silhouette, posture and function read first; detail comes after. Everything visible matches a real simulation state: a texture does not invent a system. Characters, clothing, items, animations and analysis views share proportions, materials, skeleton and lighting rules. Production advances through review gates: an attractive concept is not an animatable mesh.

Labels: **Decided** (the user confirmed it in the prototype), **Proposal** (not confirmed), **Open** (pending question).

## Direction decisions

| Rule | Status |
| --- | --- |
| Realism and fluid animation. Replaces the earlier "simplified realism" proposal. | Decided |
| Readable physiology, visible body consistency and evolution. | Decided |
| Consistency across guide images, characters, animations, items and analysis view. | Decided |
| Clear, integrated body analysis: transparency, targeted inspection, signals of the simulated state. | Decided |
| Items with an icon, an on-body representation and references to real models and materials. | Decided |
| Do not switch to semi-realism to justify a prototype. | Decided |
| Era (present day), region (real Earth), tone: realistic, not gore ([0015](../decisions/0015-tone-realistic-damage.md)). Cultural clothing still open. | Decided |
| Visible population density and graphic scope. | Open |
| Do not infer a military setting from references or a single item. | Proposal |

## Visual principles

Proposal, unless stated otherwise.

1. Silhouette, posture and function before surface detail.
2. Plausible dimensions, materials and mechanisms. Wear has a visible cause.
3. Normal view and analysis view are different. Skin and clothing do not glow outside the scanner.
4. Show only states that exist in the simulation.
5. Reuse proportions, materials, skeleton and lighting within each asset family.

Avoid: seam noise, uniform dirt, everything looking plastic, heroic proportions by default, functionless tech ornaments, generated text inside images.

## Characters

| Rule | Status |
| --- | --- |
| All bodies share topology, UVs and skeleton; they vary through morphs. | Proposal |
| Height, bone structure, muscle and fat are separate axes with valid combinations and correctives. | Proposal |
| Do not extrapolate morphs without limits (the prototype deformed shoulders and chest this way). | Proposal |
| Musculature through volume and movement, without exaggerated grooves. | Proposal |
| Body, clothing, hair and gear are separate; clothing is never a painted region of the body. | Proposal |
| Compare characters under the same review camera and lighting. | Proposal |
| Review anatomy with tight, neutral underwear. | Proposal |
| Test base body: 1.78 m adult; do not set kilograms from an image. | Proposal |
| Fat, dirt, sweat and blood in texture masks, not in geometry. | Proposal |

Composition matrix to review with the same identity and height: thin, average, muscular, adipose, muscular with fat. Check abdomen, chest, pelvis, neck and limbs. Proposal.

### Clothing that follows the body

From the engine research; I prefer this to the prototype's per-garment adjustments. Proposal.

| Option | Method | Use |
| --- | --- | --- |
| A. Bake morphs | Copy each body morph to the garment as a shape key | First step; memory cost per morph |
| B. Real-time wrap | Each garment vertex binds to the body surface and is repositioned on the GPU | Scales to any morph; layers with minimum thickness |
| C. Contour bones | Belly, chest, hip and thigh bones scaled by fat and muscle | First pass and distant LODs |

| Item | Method |
| --- | --- |
| T-shirts, trousers, underwear | A or B, fitted |
| Coats, skirts, loose shirts | A or B; simulated cloth only near the camera |
| Backpacks, belts, holsters | Rigid, bound to a bone; straps with A or B |
| Plates and helmets | Rigid, sizes S, M, L |

- Garments have sizes: if the character loses weight, size L trousers get loose. Proposal.
- Clothing layers from the GDD: skin, underwear, base garments, outerwear/armor, load-bearing gear, accessories. Proposal.
- Closed clothing hides the skin it covers, except for wounds or anatomical view. Open.

### Reference sheet

Proposal. Front, side and back of the same design in A-pose, full body, hands visible, feet aligned, constant scale. Neutral gray background, diffuse light, minimal perspective. A separate three-quarter view.

- Pockets, straps, scars and fasteners on the same side in every view.
- If two views contradict each other, resolve it before modeling. A generated image is a reference, not a measurement.
- Each candidate records ID, prompt or source, references, date and status (proposed/approved).

## Materials

Proposal.

| Family | Read | Rule |
| --- | --- | --- |
| Skin | Soft variation, natural volume | No plastic sheen or excessive contrast |
| Fabric | Matte, folds from tension and gravity | Seams and wear to scale |
| Leather and rubber | Different response from fabric | Differentiate roughness and edges, not just color |
| Metal | Localized highlights, readable construction | Metalness only on metal parts |
| Plastic | Its own volume and finish | Not the same as painted metal |

- Visual dimensions in meters. Mass and protection come from the simulation, never from appearance. Proposal.
- Crafting quality can show in seams, fit and finish. Proposal.
- Do not encode quality or rarity by color. Proposal.

## Items and icons

- Each item: stable ID, visual reference, icon, model (or pending), real materials, attachment points, dimensions, moving or replaceable parts. Proposal.
- Icon readable at 32 and 64 px; uniform framing and margin; transparent background; light from the top left. Proposal.
- Three-quarter by default; side view if it communicates function better. Test on light and dark backgrounds. Proposal.
- No small text in the icon. Proposal.
- Icon, equipped model and world model come from the same design. Proposal.

## Animation

- Priority: weight, support and readable transitions. Proposal.
- No foot sliding at reference speed; consistent knees and elbows; hands do not clip through gear. Proposal.
- Subtle breathing at idle; no camera sway to communicate every state. Proposal.
- First family: idle, walk, run, sit down, stand up, lie down. Proposal.
- Eating and drinking require real hand-object contact. Proposal.
- Each clip documents start and end pose, duration, loop, speed and source of displacement (root or game, never mixed). Proposal.
- Locomotion separate from additive layers (breathing). Responses to injury and exhaustion tied to real data. Proposal.
- States that will need animation: limp per side, torso wound, staggering from blood loss, fainting, bandaging, tourniquet, panting. Proposal.
- Contextual animations at walls, corners and doors in combat stance. Decided.
- IK and motion matching only for concrete contact or transition problems. Proposal.

## Body analysis view

| Rule | Status |
| --- | --- |
| Hover with transparency into the interior; overlays by hotkeys; damage lights up the zone locally. | Decided |
| Separate medical diagnosis from item configuration. | Decided |
| Avoid square regions and isolated columns in the analysis UI. | Decided |
| Four areas: general state, inspected zone, gear, actions. | Proposal |
| Layers: base, muscle, bone, vascular, nerves, all. | Proposal |
| Bones, muscles and vessels share position and scale; no double limbs. | Proposal |
| Pain, infection and fracture with distinct symbols; never color alone. | Proposal |
| Do not show per-muscle fatigue or exact injury coordinates if the simulation lacks them. | Proposal |
| Real anatomy in the medical view; the playable body is simulated as a part tree. | Proposal |

Starting palette taken from the prototype UI. Proposal; check contrast.

| Use | Color | Complement |
| --- | --- | --- |
| Background | #0B1016 | Hierarchy through spacing |
| Main text | #D1EFE8 | Readable size |
| Secondary | #AEC3C7 | Do not hide data through opacity |
| Inspection / selection | #66C7CC | Dot, outline or label |
| Local attention | #F2B080 | Name of the finding |
| Bleeding / alarm | #E75A6A | Symbol and text |

## World and system readability

- In third person, systems show in the world: dark windows, hissing pipes, puddles under doors. Proposal.
- In high views, overlaid layers of power, water and pressure show dead sectors and flooded rooms. Proposal.
- Room water as a rising plane, with splashes at stairs and doors. Proposal.
- Fog: visible in color, remembered in gray, unknown in black. Light and information are separate concepts. Proposal.
- References to establish: one exterior and one interior, day and night, with a person and an object for scale. Proposal.

## Workflow with review gates

Proposal.

| Gate | What gets approved |
| --- | --- |
| A | Visual direction with sheets; tell realism apart from level of detail |
| B | Gray base body: clavicle, thorax, pelvis, elbow, knee, hands, feet |
| C | Body states on shared topology, combinations and correctives |
| D | Deformation: arms up, elbow, torso twist, squat, seated |
| E | Short walk with start, turn and stop; then across the whole body matrix |
| F | Skin, hair and separate clothing under neutral, exterior and interior light |
| G | Small gameplay and analysis scene with the same asset; measure performance |

Acceptance criteria (Proposal):

- Comparison panel with identical camera and lighting, front and side.
- Intermediate and extreme body combinations, not just isolated sliders.
- 10–15 s video with foot contacts, turn and bending; no volume pops or intersections.
- A clothing texture does not count as a garment; hair without a helmet effect at gameplay distance.
- Technical test and art review are recorded separately; if either fails, it is still a prototype.
- Do not set polygon or texture budgets without a representative camera and scene.

First visual sequence (Proposal): same underground facility, same inhabitant, same lighting in four shots. Floor plan with the roof cut, selected inhabitant with a route, follow shot next to a door, third person in the corridor. A fifth view opens the body analysis.

## Prototype sources

- wiki/guia-de-arte.md
- wiki/bases-produccion-visual.md
- wiki/laboratorio.md
- wiki/decisiones-experiencia-control.md
- GDD_Survival_3D.md (sections 4, 15, 19)
- art-direction/multi-team-blockout/README.md
- engine-research.txt
