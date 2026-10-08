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

Decided ([0014](../decisions/0014-open-questions-answered.md)): the research rule, plus the method ceiling and process stability from the [production ladder](#production-ladder). Numbers per family are still to tune.

- `Q_out = weighted mean of input Q`, capped by `min(skill, Q_tool, method ceiling, Q_station)`, with a spread set by process stability.
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

## Production ladder

Intent Decided ([0013](../decisions/0013-production-ladder.md)); model Proposal. Salvage is easy, manufacturing is capped by machinery, and top quality needs a whole chain running well.

### Methods cap quality

Each recipe lists one or more **methods**, and each method has a **ceiling**: the highest Q that method can physically reach, and sometimes a minimum below which the part does not work at all.

| Method | Needs | Example |
| --- | --- | --- |
| Salvage | Hands, basic tools, knowledge | Pull bricks, cable, pipe, a car door. Keeps the part's Q; only loss and condition change |
| Hand | Tools, skill | Wooden door, bow saw repair, rope. Low ceiling for precision parts |
| Workshop | Bench, power tools, power | Welded gate, furniture, simple engine repair |
| Machine shop | Lathe, mill, welder, measuring tools, steady power, trained operators | Shafts, bushings, a rebuilt cylinder head |
| Plant | A facility (a Project, see [world simulation](world-simulation.md)): crew, inputs, power, water | Cement (kiln near 1450 °C), steel, glass, tyres, transformer parts |

- A hand-made engine part cannot reach the fit an engine needs (bore tolerances are hundredths of a millimetre), so it either cannot be made or works badly. That is the cap the user asked for: not a rule on top, but the method's ceiling.
- Machines are assemblies with their own Q and condition, made by the same ladder. Better machines come from machines (the tool ladder), so the first steps are slow and the climb compounds.
- The Q formula above gains one cap: `Q_out ≤ min(skill, Q_tool, method ceiling, Q_station)`.

### Flow decides the top

Inputs carry their own chain: a Q90 engine needs Q90 steel, which needs good ore, fuel and a well-run furnace. The weighted mean of inputs makes quality compound down the chain, so one weak link caps everything after it.

The final spread is set by **process stability** for that batch, a product of factors between 0 and 1: power stayed on, machine condition, input consistency (similar Q across inputs), operator practice, full crew, the right water and temperature. A stable process lands near its cap; an unstable one scatters low and fails more often. A power cut in the middle of a kiln firing ruins the batch. So the rare top qualities come from excellent flow, not from one lucky roll.

### Why it is late game

- The world is finite and decays: salvaged parts lose condition, fuel goes stale, tyres and seals perish, and spare parts run out. Salvage covers the early game; production becomes necessary later.
- One person cannot know every trade (knowledge takes time to learn), and a plant needs a crew. Manufacturing needs a group.
- No settlement has every input, so groups specialise and trade, by the same economy rules as the world simulation. In multiplayer, groups are clans.
- Restarting a real factory from the world (a cement works, a machine shop) is a big goal: it needs its staff, inputs, power and repairs, exactly as in the world simulation.

### Endgame and upkeep

Intent Decided ([0017](../decisions/0017-slow-endgame-and-generation-on-the-go.md)); model Proposal. Research: `research/global-sim/loop/24`.

- **The late-game opponent is decay.** Every asset needs inspection, consumables, fuel and staff; the main late-game balance is upkeep against available labour and parts. No pop at the power station, no power.
- **Hard through scarcity and dependencies, not friction:** no real-time waiting (in-game clocks only), no clicking one unit at a time, no hidden formulas.
- **From Space Engineers:** each machine has critical parts and a working line (below it, the machine stops); grinding a machine returns its real parts, so stripping one machine to fix another is a real decision.
- **From Haven & Hearth:** quality builds up along the chain with diminishing returns (H&H uses √(Q/10) as a multiplier), and a character sheet. Our method ceilings keep quality from climbing forever.
- **Info screens:** character sheet; item card with its origin (who made it, from what); produced vs consumed over in-game windows (Factorio style); quality history; supply and demand with days of cover; infrastructure health; upkeep totals; a "why can't I make this" tree. By default they show what your group has recorded, not the hidden truth (fog, [0012](../decisions/0012-world-simulation-layers.md)); truth in Sandbox and spectator.
- **Real decay clocks** (calibration): a grid collapses within minutes once frequency can't be held; nuclear backup diesel is sized for about 7 days; water utilities plan for about 5 days of backup fuel; coal plants stock months of coal, so staff run out before fuel; old transformers fail far more often than new ones.

### Open

- Ceiling values per method and per family.
- How many production steps to model per family before the chain gets tedious (proposal: model only steps where the player makes a real choice; the rest are one recipe at a plant).

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

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- Should our quality number be a geometric mean of the body attribute and the skill, like Haven and Hearth's sqrt(attribute × skill) with softcaps? ([haven-and-hearth](../research/haven-and-hearth.md))

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [haven-and-hearth](../research/haven-and-hearth.md): Haven and Hearth: a reference game for long real-time play
- [prototype-sim-code](../research/prototype-sim-code.md): Prototype simulation code: what to keep, what to avoid
- [second-life](../research/second-life.md): Second Life, as a reference for ownership

## Prototype sources

- catalogo-items.md
- fundamentos-mundo-personajes-produccion-habitantes-objetos.md
- aprendizaje-especializacion.md
- interaccion-control-inventario.md
- base-funcional-habitantes-contratos-acciones.md
- arquitectura-sistemas.md
- engine-research.txt (engine research, most recent authority)
