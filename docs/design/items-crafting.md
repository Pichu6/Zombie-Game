# Items, assembly and crafting

Every object in the world is an assembly of components with material, quality (Q) and condition. Disassembly walks that graph and returns real parts; crafting builds it. A result's Q comes from the Q of its inputs, capped by the worker's skill and the tool's Q. Quality, condition and material are three separate numbers. The world is finite: disassembling, making or transferring never creates or loses mass without a declared rule. Inventory and containers hold instances with identity, not loose icons.

Labels: **Decided** (the user confirmed it in the prototype), **Proposal** (not confirmed), **Open** (pending question).

## Principles

| Rule | Status |
| --- | --- |
| Existing objects can be disassembled and built. | Decided |
| Most appear already assembled: tools, furniture, props, vehicles. | Decided |
| Every object has Q tied to its components, physical data and visual representation. | Decided |
| Components have production paths tied to knowledge and skills. | Decided |
| Haven & Hearth is the canonical reference for knowledge, profession and Q. | Decided |
| Finite world: no respawn of loot, people or zombies. | Decided |
| Do not copy Haven & Hearth's respawn or world resets. | Decided |
| Assembly graph as the data model for every object. | Proposal (engine research) |
| First slice: disassembly and Q over about 30 item families. | Proposal (engine research) |

## Data layers of an object

Separate material, component type, instance, assembly and process (recipe). Proposal.

| Layer | Minimum content |
| --- | --- |
| Definition | Stable ID, version, name, family, capabilities, allowed materials, compatibilities, Q model, processes |
| Physical | Mass in kg (or a rule to derive it), dimensions in m, volume and capacity if applicable, collision |
| Instance | Unique ID, definition, Q, condition, quantity, location and owner, real components, functional state |
| Visual | Icon, 2D silhouette, 3D model, materials, mount and grip points, LOD |
| Production | Recipes, inputs, outputs, requirements, times, recovery policy |
| Validation | Valid references, units and ranges, consistent mass, ownership without cycles |

- The definition shares assets; each instance keeps its own state.
- No texture is needed per screw or per Q value.
- A placeholder is marked as such; it is never presented as a final asset.
- A part hidden inside an assembly needs its own representation once extracted.

## Assembly graph

Proposal. An object is a list of components; each has material, Q and condition. Nothing is physically simulated until it breaks.

- Parts occupy functional slots and have a joint state.
- Stored contents are not a mounted part. The loot in a backpack is not structure.
- Mass is not counted twice: mounted parts and contents are summed separately.
- Ownership and location form a hierarchy without cycles.
- Physical joints and links between subsystems are modelled separately, without giving a part two owners.
- A vehicle is an assembly of subsystems, not thousands of active nodes.
- Composition is fixed when the object is generated, either explicitly or reproducibly from a seed.
- Disassembly materialises those same parts; new loot is never rolled.
- Identical screws can stack if they share Q, condition and provenance.
- Buildings carry their infrastructure: walls and floors contain cable and pipe edges. Proposal (research).
- Breaking a wall can cut off a sector or flood a basement. Stripping copper from a house is a real decision. Proposal.

## Quality, condition and material

Three separate values. A rusted high-Q pipe is still worth salvaging.

| Value | What it measures | Changes over time |
| --- | --- | --- |
| Q (quality) | How well it was made or assembled | No; fixed except through a real process |
| Condition | Wear, breakage, corrosion | Yes; drops with use, rises with repair |
| Material | What it is made of | No |
| Functional properties | Load, edge, tolerance, insulation, burst pressure | Derived from relevant parts, design, Q and condition |

### Confirmed rules (Haven & Hearth model)

Decided (confirmed 2026-09-10):

- Every item, material, tool and component has Q.
- The Q of an assembly or repair inherits from the Q of its parts.
- One very bad part drags the whole down: a Q70 axe head with a rotten Q10 handle gives a heavily penalised tool.
- The worker's skill is a soft cap on the Q of tools and materials.
- Excellent results (Q > 70) require high skill, a good tool and good components at the same time.
- Higher effective Q cuts action time by up to 50 %.
- High-Q tools wear less per use.
- Higher effective Q reduces critical failures: snapped screws, jammed weapons, burst pipes.

### Formula

Conflict: the prototype used a geometric mean and a `sqrt(Q·S)` cap; the engine research proposes another rule. The research rule is preferred. Proposal:

- `Q_out = weighted mean of input Q`, capped by `min(skill, Q_tool)`, plus a small random spread.
- Weights represent function, not mass.
- Q is continuous, with no steps. Candidate scale 0–100. Open: the prototype also used [1, 100+].
- Example: a Q40 head and a Q90 handle with 70/30 weights give Q55 before the cap.

Prototype examples with the `sqrt(Q·S)` cap, for reference only: a novice S10 with a Q90 scalpel yields ≈30; a surgeon S80 with a Q15 knife yields ≈34.

### Limits on how Q is used

Proposal:

- Q is not a universal multiplier for every stat.
- Each property declares which part governs it. Edge depends on the head, not on the axe's average Q.
- Useless brakes are not offset by excellent seats.
- Q scales properties continuously: a Q80 pipe bursts at higher pressure than a Q30 one. Research.
- A Q60 cable carries more load before overheating. This is how Q feeds the power and water networks.
- Missing or broken parts are handled explicitly; the average does not let an incomplete object work.
- True Q belongs to the object. The character only sees their assessment, based on knowledge and instrument.
- Studying never raises any object's Q. Decided.

## Disassembly, crafting and repair

- Disassembly returns components with their Q, minus a loss that drops with skill and tool Q. Proposal (research).
- Not all disassembly is reversible: screwed, welded, glued and consumed require different processes. Proposal.
- Each process declares intact recovery, breakage and waste. Proposal.
- Assembling, disassembling, repairing and making base material are not the same recipe reversed. Proposal.
- Disassembling and reassembling does not raise Q. Proposal.
- No make/disassemble cycle can generate free mass, parts or upgrades. Proposal.
- A lossy refining step can raise a property if its recipe defines it. Proposal.
- Repair restores condition and can replace parts; it does not leave the object as new. Proposal.
- The Q of a repair part decides how long the fix lasts. Proposal (research).
- Poor assembly caps the whole without lowering each part's intrinsic Q. Proposal.
- The result and its defects are recorded; checking Q does not re-roll them. Proposal.
- A part having a recipe does not mean a survivor can make it by hand. It may require industry. Proposal.
- Building furniture and structures uses the same objects, plus supports, fasteners and space. Proposal.

### Recipe content

Proposal. Inputs and alternatives; quantities and units; tools and capabilities; station and conditions; knowledge; difficulty and skill; phases and time; consumed resources; outputs, by-products and losses; Q and recovery rules; interruptions and partial results.

## Inventory

- Separate type, instance, state, location and quantity or stack. Proposal.
- Candidate families: clothing, container, tool, consumable, resource. Proposal.
- The object's capabilities (open, store, equip, consume, handle, disassemble) define its actions. Proposal.
- Do not design an icon grid first and invent rules afterwards. Proposal.
- Carrier mass = body + clothing + real contents of their containers. Proposal.
- Body load is always derived from real contents, never from a copied value. Proposal.
- An instance is never duplicated or lost when transferred or interrupted. Proposal.

## Containers

- A container's capacity is distinct from the carrier's physical capacity. Proposal.
- The container owns: volume, access opening, content compatibility and structural limit. Proposal.
- The carrier owns what they can lift and carry, depending on their state. Proposal.
- A backpack does not lose volume because its carrier is tired. Proposal.
- An unowned container belongs to the world; an owned one's contents weigh on that inhabitant. Proposal.
- Seeing a container does not reveal all its contents. Proposal.
- Open: nested containers, volumetric inventory and stacking.

## Clothing and gear

- CDDA-style clothing layers: inner, middle and outer. Proposal.
- Clothing provides insulation (clo) and encumbrance. Proposal.
- Clothing can have a size; after losing weight, size L trousers become loose. Proposal (research).
- Backpacks, belts and holsters: rigid body attached to a bone, straps that follow the body. Proposal (research).

## Open questions

- Final Q scale, and whether it is shown exactly or as a range with confidence.
- Weights per family, and whether random spread applies to every recipe.
- Rules for renewable production (crops, births) in a world without respawn.
- How to keep inspecting many similar objects from becoming tedious.

## Prototype sources

- catalogo-items.md
- fundamentos-mundo-personajes-produccion-habitantes-objetos.md
- aprendizaje-especializacion.md
- interaccion-control-inventario.md
- base-funcional-habitantes-contratos-acciones.md
- arquitectura-sistemas.md
- engine-research.txt (engine research, most recent authority)
