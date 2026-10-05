# Character production lessons

The prototype spent most of its effort on characters: AI-generated meshes, inherited rigs, shoulders and armpits, clothing and hand-fitted anatomy. Almost all failures came from mixing responsibilities: rest shape, rig, weights, correctives, composition and garment. These lessons summarize symptom, cause and what to do in the new engine. The figures describe specific prototype assets; they are not universal tolerances. The underlying rule: one base body with one topology, one UV and one skeleton, varied by morphs; clothing bound to the body; and visual review from many views.

Provenance: the "What to do" column is a **Proposal** unless it says **Decided** (confirmed by the user).

## AI-generated meshes and sources

| Symptom | Cause | What to do |
| --- | --- | --- |
| Gaining weight caused cuts in the torso and shorts. | The AI mesh was torso, shorts, legs and face disconnected; the garment overlapped the skin. There was no body underneath. | Inventory components, open edges and overlaps before creating morphs. Do not use AI meshes as the base body. |
| Blending the normal and fat variants lost volume and broke the armpits. | Each AI generation had a different topology; they were paired by proximity. | One common topology; variants only through morphs. **Decided** (same topology and identity across variants). |
| AI T-shirt looked like burlap, with an irregular collar and stretched sleeves. | Bad geometry and texture; correct weights and morphs cannot make up for it. | Do not use AI garments as deformable garments. Own pattern with clean openings and matte material. |
| Regenerating a character for every weight or face. | Mutually incompatible topologies. | Never. One base body and morphs. |
| Anatomical export loaded fine but was missing a vertebra, cheekbone and teeth. | A filter by piece name skipped parts. | Classify by explicit ID, audit what was excluded and review region by region. |
| A foot from another source did not fit; scaling it left it at 17 cm. | Real pieces from different sources do not share joints. | Use coherent regional sets. Do not scale bones to fit. Measure the joint. |
| CT-scan skull with altered size. | Stacked slices without physical scale metadata. | Require physical spacing. If walls or teeth are missing, do not treat it as a healthy head. |
| Atlas forced inside the skin with non-uniform scales. | Someone tried to keep a bad silhouette by deforming bones. | Anatomy guides the skin; fix the skin. **Decided**. An atlas is one person, not a norm. |

## Rig, rest pose and retargeting

| Symptom | Cause | What to do |
| --- | --- | --- |
| Feet turned ~85° with soles facing forward. | Switching between rest matrices carried the technical roll over as twist. | Minimal rotation between segments, separate from roll. |
| The neck disappeared when moving a pivot. | Trapezius skin with over 90 % cervical weight; it rose 6–7 cm. | After changing a rest pose, invalidate weights, correctives and contacts tied to the old one. |
| Clavicle and scapula separated 25 mm when raising the arms. | The rig kept the bones but not the joint connection. | Common anchor computed at rest and translation correction after rotating. |
| Radius and ulna rotated together; elbow 12 mm off axis. | A single control for the forearm; inherited pivot placed wrong. | Radius independent with the hand. Separate orientation, real joint center and technical origin. |
| Rotating around an offset pivot moved the joint. | Only the quaternion was applied. | Rotating around a point requires compensating translation. |
| Anchor with no drift, but ulna inside the humerus when flexing. | Zero drift at one point does not prevent penetration. | Measure crossing depth during movement. |
| V-shaped shoulders and inverted legs when posing. | The engine expected a full local rotation, not a delta from the authoring tool. | Compose rest pose by delta. Test: a zero delta keeps the rest pose. Check global positions. |
| Arms crossed when playing clips. | A T-pose delta applied to an A-pose rig. | Retarget in a common frame; compare animated directions with the source. |
| Elbow and descendants misaligned between layers. | Rotations were copied between rigs with different rest poses. | Transfer the full delta (current times inverse rest), with pivot translation. |
| Pointed knee; calf moved backward when gaining fat. | Pivots in front of the volume; the fat morph moved section centers. | Check joint centers from both profiles. Composition does not move joint centers. |
| Floating squat. | Fixed angles with no support. | Measure support along the whole trajectory; compensate with pelvis or IK. |
| Artificially square shoulder. | The control's orientation was applied as rest tissue rotation. | Control orientation and tissue shape are different things. |
| Injuries possible on the wrong side. | In one asset ".R" was the anatomical left. | Validate sides against anatomical reference, never by suffix. |
| Neck muscles did not follow skull or jaw. | Rig with no jaw bone; anchors with 49 % foreign influence. | Jaw in the rig from the start. Anchor weights by muscle identity. |
| Foot tendons stretched when walking. | A bone was missing from the allowed list; vertices fell back to the pelvis. | Forbid silent fallback to another bone. Weight test by region and side. |

## Shoulder and armpit

| Symptom | Cause | What to do |
| --- | --- | --- |
| Defective shoulder when raising, with little or much fat. | There was no independent scapula; only clavicle and arm. | Scapula and back move with the arm. **Decided**. |
| A single scapular control is not enough. | There are three families: protraction/retraction, elevation/depression, rotation when raising the arm. | Test them separately before combining. Also check lowering. |
| Raising the clavicle angle from 16° to 32° created another scapula protrusion. | The goal was a number, not joint coherence. | Fix the joint trajectory and regional tissues, not more angle or minor bones. |
| An envelope to cover the scapula produced 51 mm "wings". | A corrective hid a joint failure by inflating tissue. | Fix the joint first; the surface support ended at 4 mm. |
| A filled-in armpit looked fake. | A real raised armpit keeps a hollow. | Do not fill the hollow. The pectoral fold and the dorsal fold are different transitions. |
| The shoulder looked like glued pieces. | Mix of geometry, normals and material with no relief or scattering. | Isolate geometry, normals and texture. A shadow does not prove a hole. |
| Muscle definition from a thin subject applied to the fat one. | Reference from a different build. | References of the same build; real people first. **Decided**. |

## Skinning, correctives and tissue

| Symptom | Cause | What to do |
| --- | --- | --- |
| Automatic correctives enlarged deltoids and armpits. | Mathematical volume preservation knows nothing of insertions or folds. | Compare the rig with and without the corrective in the same views. Remove it if any view gets worse. |
| Folded region when mixing dual quaternion and linear. | Spatial mix of two methods. | One method per region, or a patch with a fixed border and detail transport. |
| Spikes in the weights. | Gradients computed over small mesh islands. | Group by muscle identity before smoothing. |
| Navel folded when flexing. | Neighboring rings with different weights. | Local weight continuity. Compare case by case, not totals. |
| Shorts moved with the arms. | 10 of 280 vertices had arm influence; they moved 3.5 cm. | Move one isolated chain and check that everything else stays still. |
| Tissue spring pushed in the wrong direction (20 mm instead of the 10 requested). | Conversion in the bone's frame instead of the parent's. | Measure the effective world position, not the spring state. |
| Muscle "activation" that moved nothing. | It only stiffened skin springs. | Stiffness is not force. Inventory tissue by function, not by mesh count. |
| Subcutaneous layer detached from the muscle. | A sheet separated by air. | The subcutaneous layer fills its volume down to the support. |
| Closing a gap by projecting vertices flipped 65 normals. | Independent per-vertex projection. | Smooth patch transport; check normals and areas. |
| Thin head on a fat body. | The face was frozen to keep identity. | Regional facial deposits that protect eyes, nose, lips and skull. **Decided** (fat on face and double chin). |
| Extreme fat only on the abdomen or only on the hips. | Exclusive regional distribution took regions to zero. | Distribution is a bias; shared fat stays everywhere. |
| New morphs pierced the skin again. | They were added after the fitting. | Correctives per composition combination, interpolated with the same weights. |
| Tissue physics did not fix the pose. | Physics does not repair kinematics. | Separate support, inertia and display; fix kinematics first. |

## Clothing

| Symptom | Cause | What to do |
| --- | --- | --- |
| Rigid clothing; each T-shirt was a project (9 variants, 18 chest correctives). | The garment knew the skeleton but not the body's morphs or correctives. | GPU wrap after morph and skinning (see research/clothing-wrap.md). |
| Vertices outside the body but faces cutting into it. | Vertices were measured, not faces. | Sample faces; check three-quarter views and maximum chest. |
| Step on the T-shirt's chest. | Cloth draped from a single chest point. | Patterns per sex and a continuous front surface. |
| Broken folds after a global corrective. | It mixed incompatible conflicts (sleeve against torso, hem against thigh). | Fix by zone. Do not inflate the whole garment. |
| Sleeves offset 67–82 mm when lowering the arms. | The drape fit included arm influences. | Exclude influences the garment must not follow. |
| Shorts separated from the abdomen when posing. | One mesh being watertight does not prove it fits another. | Separate deformation by composition, fit between pieces, and skinning. |
| A base garment with swappable texture gave no believable pockets. | A texture does not change the silhouette; the UV had no stable places. | Base garment with sewing-pattern UV and detail modules bound to it. |
| Millimeter crossings chased by hand under clothing. | The prototype forbade hiding covered skin. | Conflict with the research: hide covered skin by region. **Proposal** (pending decision). |
| A garment that passed tests was rejected at the chest. | Functional tests and visual approval are different things. | Record both separately. |

## Review and evidence

| Symptom | Cause | What to do |
| --- | --- | --- |
| Failures invisible from the front. | Single view. | Both profiles, back, both three-quarters, continuous rotation. **Decided** (required by the user). |
| Failures at low fat, not only high. | Only extremes were tested. | Extreme and intermediate compositions, same camera, light and scale. |
| An improved metric hid 399 inverted faces. | The metric measured coverage, not orientation. | "Loads", "weights sum to one", "morph changes" and "looks good" are different tests. |
| False 39 µm differences between authoring and engine. | Weights quantized to 16 bits on one side; float32 precision on the other. | Compare with the same quantization and realistic tolerances. |
| Animation seemed not to play. | Sampling every 3 s, almost equal to the 3.14 s period. | Sample much faster than the period. |
| Screenshots labeled with another pose. | Controls not synced when changed from code. | The capture records pose, view, composition and time. |
| Translucency evaluated in a mode without scattering. | A renderer different from the game's. | Validate material in the real renderer. |
| Clip imported without errors but with no motion. | The import discarded curves. | Check curve excursion and actual playback. |

## Performance

| Symptom | Cause | What to do |
| --- | --- | --- |
| 150 ms per frame when flexing the elbow; 3.8 ms after the fix. | No LOD, validation repeated per query, two clocks and two skeletons. | One skeleton shared by layers, one clock, validate once per pose, real LOD. |
| Anatomical layers of ~1.6 M triangles versus 40 k for the outer body. | Full interior always loaded. | Light interior, loaded on demand. Hiding does not free memory. |
| Code copies in asset folders broke loading. | The importer scanned backups. | The importer reads only declared paths. |

## Cost versus realism

| Symptom | Cause | What to do |
| --- | --- | --- |
| Very expensive, slow iterations. | Skin built over an atlas; each pose creates new contacts; mixed criteria. | Stable outer base plus reduced simulation and interior. |
| No clear milestone closure. | Anatomy, crossings, naturalness and calibration evaluated at once. | Separate observable criteria: silhouette, face, movement, clothing, wound. |
| Explosion of test cases. | Cartesian product of all sliders. | Extremes, intermediates and problem combinations. Expand one dimension at a time. |
| New layers with no gameplay use. | They were added out of inertia. | Add organs or regions only if they change a decision or an image that matters. |

## Prototype sources

conocimiento-produccion-personajes.md, conocimiento-produccion-personajes-superficie-evidencia.md, conocimiento-produccion-personajes-anatomia-tejidos.md, conocimiento-produccion-personajes-reposo-movimiento.md, conocimiento-produccion-personajes-soporte-rigs.md, personajes-realistas.md, evaluacion-humano-costo-realismo.md, movimiento-capas-rendimiento.md, decisiones-generador-anatomia.md, decisiones-tejidos-esqueletos.md, bases-simulador-corporal-base-femenina.md. Also: research/clothing-wrap.md and engine research.
