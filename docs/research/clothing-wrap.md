# Clothing that follows the body, in our own engine

Research and proposal for Juego Zombie. Version 2, 2026-10-05: rewritten for a new engine built from scratch (Vulkan). The Godot/Mixamo/Tripo project is used only as a reference for what failed.

## Short answer

Do not fit each garment. **Each vertex of a garment is bound once to a point on the body surface, and every frame the GPU places it again on the already deformed body.** This way clothing follows any change in fat, muscle, swelling or pose with no shape keys of its own, no per-garment correctives and no further work in Blender.

In the prototype, each T-shirt was a whole project: nine composition variants, 18 chest correctives and rounds of armpit fixes. That is exactly the "garment by garment" work that does not scale. With our own engine we can do what big games do offline, but in real time and free per garment.

## How others solve it

| Game / tool | How clothing follows the body | What to copy |
| --- | --- | --- |
| **The Sims 4** | Body sliders are stored as *deformer maps*: textures in a UV space shared with the body, holding the shape displacement. Each garment reads the map through its second UV (projected from the body) and moves the same way. One map per slider serves every garment, even community-made ones. | The displacement lives in the body, not in each garment. |
| **Roblox (layered clothing)** | The body has an *outer cage* and each garment has an *inner cage* and an outer one. At runtime a "wrap" deformer places the garment on any avatar. A jacket stacks over a T-shirt because it uses the T-shirt's outer cage as its body. It works with millions of user-made garments. | Runtime wrap and stacked layers. |
| **MakeHuman / MPFB** | `.mhclo` format: each garment vertex stores three body vertices, barycentrics and an offset. It is the same binding I propose, done offline. | The binding format. |
| **Unreal Mutable / MetaHuman** | *Mesh Reshape* deforms the garment from the base body to the current body, and also *removes body parts that are not visible* under the clothing. MetaHuman already ships clothing that adapts on its own. | Reshape plus hiding covered skin. |
| **Marvelous, Wrap, Houdini** | Offline: they simulate or wrap the garment once per body shape and store blend shapes. | Useful only for hero garments; not needed as a base. |

The common pattern: bind once, transfer the shape, push outward, hide covered skin and only fix the rare extreme case by hand.

## Proposed architecture in the engine

### Per-character GPU pipeline (every frame, or when something changes)

1. **Morph** (compute): base + sparse deltas driven by the body simulation weights (fat, muscle, hydration, swelling, wounds).
2. **Skinning** (compute): skeleton, at most 4 bones per vertex.
3. **Normals** of the deformed body (compute, or recomputed from the deltas).
4. **Wrap** (compute), one pass per layer: underwear, T-shirt, jacket and gear. Each layer can use the previous layer's output as its "body", like Roblox cages.
5. **Render**: body with regions hidden according to what it wears, plus garments.

Because wrap runs after morph and skinning, clothing inherits **everything**: shape, pose and the body's pose correctives. The prototype's armpit crossings came largely from clothing lacking the correctives the body had; with wrap that disappears by construction.

### The binding (what the tool exports, once per garment)

For each garment vertex:

| Field | Type | Purpose |
| --- | --- | --- |
| `tri_id` | u32 | Nearest triangle of the body at rest |
| `bary` | 2 × f32 | Position inside that triangle |
| `offset_tbn` | 3 × f32 | Offset in the local basis (tangent, bitangent, normal): thickness and ease |
| `follow` | f32 | 1 = tight; ~0.5 = loose (does not hug the belly) |
| `island` | u32 | Rigid island or none |
| `bone_index` / `bone_weight` | 4 × u16 / 4 × f32 | Interpolated body weights, for skeleton-only mode |
| `body_uv` | 2 × f32 | Coordinate in the body's common UV (dirt, blood, hiding) |

That is about 60 bytes per vertex, and it can be compressed by half with f16 and 16-bit barycentrics.

### The wrap shader (one thread per garment vertex)

```glsl
uvec3 t = bodyTris[tri_id];
vec3 a = P[t.x], b = P[t.y], c = P[t.z];          // already deformed body
float u = bary.x, v = bary.y, w = 1.0 - u - v;
vec3 p = a*w + b*u + c*v;
vec3 n = normalize(N[t.x]*w + N[t.y]*u + N[t.z]*v);
vec3 e = b - a;
vec3 T = normalize(e - n*dot(e, n));
vec3 B = cross(n, T);
vec3 o = offset_tbn;
if (island == NONE) o.z = max(o.z, minGap);       // cloth never goes into the skin
vec3 wrapped = p + T*o.x + B*o.y + n*o.z;
out_pos = mix(skinnedOnly, wrapped, follow);
```

That is 6 reads and about 40 operations per vertex. Unmeasured estimate: 50 characters with 20,000 clothing vertices each make one million vertices, a fraction of a millisecond on an RTX 3060-class GPU.

### Three object classes, the same code

- **Soft** (T-shirt, pants, jacket): full wrap.
- **Mixed** (backpack, belt, holster, boots): straps and the boot shaft wrap; hard pieces (backpack body, buckles, sole) are **rigid islands**. All vertices of an island share the anchor at its center, so they move as a solid that "rides" on the surface. The shader has no special case.
- **Rigid** (helmet, watch, weapons): a single anchor (a "rivet") on the surface, or a bone. A helmet on a fatter face rises with the face.

### Safety net

- **Minimum gap** (in the shader above): no vertex goes below `minGap` above its anchor point.
- **Hide covered skin**: the body is tagged by region (torso, upper arm, thigh...) and each garment declares which regions it fully covers. In the vertex shader, triangles of hidden regions are collapsed with a per-character bit mask. Wounds or the anatomical view turn off the mask in that area. The prototype forbade this; here I recommend using it (see the decision at the end).
- **Contact between body parts** (sleeve against torso with the arm down): a few capsules per character (torso, thighs) and a push in the same compute. It is the only collision needed, and it is cheap.

### Base garments + detail modules (relief, pockets)

The prototype tried a deformable base garment mesh with a swappable texture. It failed on relief (pockets, flaps) for three reasons: a texture does not change the silhouette, the cylindrical UV gave no stable places, and the details were not bound to the garment. Proposal:

- **Base garment**: a few generic meshes (T-shirt, sweatshirt, pants, jacket) with **sewing-pattern UV** by panel (front, back, sleeve...). A coordinate means the same thing in every variant. It is bound to the body.
- **Detail modules**: pocket, flap, zipper, strap, knee pad, hood. They are small meshes with real thickness, bound to the **base garment surface** with the same binding and placed in pattern coordinates. Cargo pants are base pants plus four modules.
- **Material**: tileable fabric, a normal map only for seams, stitching and weave, plus the dirt and wear masks.
- **Crafting**: the garment is a list of pieces, each with material and quality. Disassembling returns the pieces and crafting combines them. Rendering and the crafting system read the same description.

### Continuous body change

Since clothing has no shape keys, the simulation can change composition continuously: thinning over weeks of hunger, swelling from an infection, gaining muscle from work. Clothing always fits with no preparation. **Size** comes almost for free: a garment smaller than the body looks tight (`follow` and limited negative ease), and a larger one hangs loose.

### Isometric / management camera

From above, cloth thickness is not visible. For distant characters, skip the wrap: bake the worn clothing into the body texture when the gear changes and draw a single mesh. This is the big performance win for crowds in RTS or management mode.

## Dirt, grease, blood and wetness

Same trick as the Sims deformer maps: a **common body UV space**.

- Each character carries small masks in that space (for example 256 × 256 RGBA: mud, grease/oil, blood, wetness). The game paints into them: crawling dirties knees and forearms, a splatter paints where it hit, rain wets from top to bottom.
- Each garment has a per-vertex `body_uv` (from the binding) and reads **the same** masks, so mud on the legs appears on the skin and on the pants covering it, in the same place.
- Each item also keeps its own copy of the masks in that same space. If the pants pass to another character, they keep their stains in the right place because all bodies share the UV space. This fits the Haven & Hearth idea: quality, wear and dirt live in the item.
- The look comes from a few tileable materials (mud, oil, dried blood, water darkening) blended by the masks. No per-garment texture painting is needed.

Requirement for all of this: **all bodies share one topology and one UV** (they change through morphs, not separate meshes). That rule was already right in the prototype and should be fixed from day one of the new engine.

## Included tools (tested)

- [export_garment_binding.py](../../tools/garment/export_garment_binding.py): Blender used only as an authoring tool. It reads the body at rest and the garments in a collection. Per garment it writes `<name>.bind.json` (header and layout) and `<name>.bind.bin` (little-endian), plus `body.tris.bin`. It reads the optional groups `fit_follow` and `fit_rigid`.
  ```
  blender scene.blend --background --factory-startup --python export_garment_binding.py -- --body Body --collection Garments --out-dir export/
  ```
- [wrap_reference.py](../../tools/garment/wrap_reference.py): the wrap step in numpy. It is the executable specification of the compute shader; use it to compare the GPU version vertex by vertex once it exists.

Test run with headless Blender 5.0, on a synthetic body (a cylinder with fat and thin shapes, two bones) and a garment with a rigid pocket. Results:
- Round trip at rest with a maximum error of 0.00002 mm.
- With fat and thin bodies, the cloth always stays outside the skin (minimum gap 9.8 mm).
- The rigid pocket does not deform (0 mm distortion).
- Bone weights sum to exactly 1.

**Not yet tested on a real human body or on the GPU.**

## Suggested order for the new engine

1. Fix the base body: one topology, one UV, one skeleton and morphs as sparse deltas. The CC0 MakeHuman/MPFB base is legally reusable as a starting point if it helps, or a new one is modeled.
2. First visible milestone: body with morph and skinning in compute, plus a wrapped T-shirt and a live fat slider. If that looks good, the clothing problem is essentially solved.
3. Then: backpack (rigid islands), layers, skin hiding, dirt masks and the baked distant mode for the isometric camera.

## Decision for you

**Does tight clothing hide the skin it covers?** I recommend yes, per covered region, except where there is a wound or in the anatomical view. All the games cited do this, and it avoids chasing millimeter crossings by hand.

## Sources

- [The Sims 4 deformer maps (TS4MorphMaker guide)](https://modthesims.info/showthread.php?t=613057) and [UV_1 on Sims 4 clothing](https://sims4studio.com/thread/19464/uv-mapping)
- [Roblox cages](https://create.roblox.com/docs/art/modeling/cage-meshes) and [layered accessories](https://create.roblox.com/docs/art/accessories/layered-clothing)
- [Mutable: remove body parts that are not visible](https://work.anticto.com/w/mutable/unreal-engine-4/user-documentation/remove-unseen-parts/) and [thread on Mesh Reshape](https://forums.unrealengine.com/t/is-mutable-mesh-reshape-possible-with-cloth-simulation/2689499)
- [Resizable clothing for MetaHumans (Epic tutorial)](https://dev.epicgames.com/community/learning/tutorials/KZ2r/unreal-engine-how-to-create-resizable-clothing-for-metahumans-auto-adapts-to-body-proportions)
- [MakeHuman Shirts01 pack](https://static.makehumancommunity.org/assets/assetpacks/shirts01.html) (`.mhclo` format)
