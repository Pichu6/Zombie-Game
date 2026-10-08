# Living infrastructure: power, water and room-based fluids

The city has real power and water networks. They are coupled to each other and to the air inside buildings. They can be cut, blown up, looted and restored. This is the game's innovation bet: almost no 3D survival game does it, and Project Zomboid just shuts everything off on a timer. The networks are graphs, and interiors are a graph of rooms. Everything is recalculated on events, never per frame. A destroyed pole costs one graph update, not a continuous simulation.

Labels: **Decided** (the source states it as a project rule), **Proposal** (not confirmed), **Open** (pending question).

## Principles

| Rule | Status |
| --- | --- |
| Every network element can be repaired or built from scratch in the late game. | Decided (confirmed and extended to everything built in the game, see [0010](../decisions/0010-concept-v2.md)) |
| Networks as graphs of nodes and edges, not cables or physics. | Proposal (prototype and research agree) |
| Zero cost at rest: each receiver stores its state and nothing runs per tick. | Proposal |
| Recalculate only the part of the graph affected by a cut, repair or overload. | Proposal |
| Water, gas, smoke, heat, sound and zombie scent travel through one shared room graph. | Proposal (research) |
| Utility loss is emergent and reversible, not a fixed blackout on "day X". | Proposal (research; replaces the prototype idea) |

Conflict: the prototype proposed a blackout and water cut scheduled on a fixed day. The research prefers emergent decay. The latter is adopted as a Proposal.

## Simulation levels (sim LOD)

Proposal (research). Replaces the prototype's 0–80 m / 80–300 m / >300 m zones and 100 × 100 m sectors.

| Level | Where | What runs | Frequency |
| --- | --- | --- | --- |
| Hot | ~50 m from a player | Per-tile water surface, sparks, visible leaks, individual AI | Every frame |
| Warm | Loaded district | Pressure per segment, water level per room, power per sector | A few times per second or on event |
| Cold | Rest of the world | Statistics: "sector 12 without power", "building flooded to 1.2 m" | On event or slow tick (minutes) |

- When stepping down a level, quantities are preserved (water volume, load). When stepping up, detail is rebuilt from that number.
- Only regions marked dirty are updated.

## Power network

### Topology

Proposal. Hierarchy: power plant → substation → medium-voltage line → sector transformer ("grey can") → neighborhood poles → service drop → breaker panel → circuits (outlets, lamps, appliances).

| Node | Role | Attributes |
| --- | --- | --- |
| Source | Substation, generator, transformer | Max power (W), condition, quality |
| Switch | Pole, breaker, fuse | Condition, quality, open/closed state |
| Receiver | Lamp, outlet, fridge, pump, alarm | Consumption (W), powered yes/no |
| Edge | Cable span | Max load by quality and material |

- When an edge breaks: BFS from the sources over the affected subgraph only, or union-find. Proposal (research).
- An event like `SectorUnpowered(12)` is emitted. Lights, doors, alarms, fridges and pumps subscribe to it.
- Prototype: 500 poles and 3,000 houses recalculate in a fraction of a millisecond. Proposal (estimate, not measured in the new engine).

### Load and overload

| Mechanic | Numbers | Status |
| --- | --- | --- |
| Gasoline generator | 3,000 W max | Proposal |
| LED bulb | 10 W; engine at quiet idle | Proposal |
| Compressor or table saw | 1,800 W | Proposal |
| Brownout when over the limit | Lights flicker, black smoke, breaker trips | Proposal |
| Overloaded cable | Breaks (Oxygen Not Included style) | Proposal (research) |
| Cable quality | Q 60 carries more load than Q 30 before overheating | Proposal (research) |
| 220 V electric fence | Slows the horde; high consumption; can ignite dry grass | Proposal |

### Tampering with a pole

| Method | Tool | Consequence | Status |
| --- | --- | --- | --- |
| Cut at the base | Chainsaw (90 dB) or axe | Falls according to the notch; crush risk; cut cables spark | Proposal |
| Pulling | Sling or winch + 4x4 pickup | Uproots the base | Proposal |
| Cut guy wires | Bolt cutter | On corners the pole bends or falls under the weight of the lines | Proposal |
| Clean dismantling | Climbing spurs, harness, insulated pliers | Recovers transformer and insulators intact | Proposal |
| Vehicle crash | — | Breaks the edge and damages the vehicle | Proposal |

## Water network

Proposal (research). Hierarchy: treatment plant → water mains → pumping stations → district trunk lines → building risers → faucets.

- Connected pipe segments with shared contents are simulated, not individual pipes.
- Pressure per segment = supply head − demand − leaks.
- A burst pipe is a leak node with a flow rate. That flow enters the room graph.
- Closing shutoff valves concentrates pressure in open branches: hoses, sprinklers, turbines. Proposal.
- Pipe Q sets burst pressure: Q 80 holds more than Q 30. Proposal (research).

### Water hammer

Proposal. Closing a valve in under 0.5 s sends an overpressure wave with a metallic "clang". If the pipe is old or rusty, an elbow or joint bursts elsewhere. Closing slowly avoids it.

### Freezing

Proposal. Before the first frost, shut the main valve and open faucets to drain. Trapped water freezes and cracks pipes inside walls. During the thaw the house floods.

## Room graph (fluids through spaces)

Proposal (research). Each room, hallway, stairwell and sewer section is a container. Each opening is an edge with height and width: door, stairs, drain, hole, broken window. Water flows by level difference. It costs a few hundred numbers per building.

Conflict: the prototype described real-time physical cascades down stairs and storm drains. This is kept as a hot-level visual effect on top of the graph. Proposal.

| Fluid | Source | Effect | Status |
| --- | --- | --- | --- |
| Water | Broken pipe, rain, clogged drain, thaw | Level per room; overflows thresholds and runs down stairs | Proposal |
| Smoke | Fire without chimney, building fire | Rises to the ceiling and descends; at head height: 70 dB cough, lung damage, temporary blindness | Proposal |
| CO | Generator or stove indoors, blocked chimney | Colorless and odorless; headache, drowsiness, death in sleep; avoided with cross ventilation | Proposal |
| Sound | Gunshots, generators, impacts | Spreads through openings; attracts zombies | Proposal (research) |
| Zombie scent / smell | Bodies, excrement, food | Travels between rooms | Proposal (research) |
| Heat / fire | Building fire | Goes on the same graph later | Proposal (research defers it) |

### Indoor flooding

| Risk | Rule | Status |
| --- | --- | --- |
| Electrified water | If the level reaches an outlet on a live network, the water electrocutes | Proposal |
| Supplies | Cardboard, flour, loose ammo and books at floor level are ruined | Proposal |
| Wading | Knee-deep water: 3× stamina and fast chilling | Proposal |
| Drowning | Unconscious in more than 15 cm of water: dies within minutes | Proposal |
| Drainage | Electric sump pump, manual diaphragm pump, trenches dug with a shovel | Proposal |
| Storm drain | Clogged with trash, it pools water in the street; clearing it dumps the water into the sewer below | Proposal |
| Cascade into the sewer | Soaks whoever is below, puts out torches, echoes and alerts the underground | Proposal |
| Continuity | No loading screens between street, interiors, vehicles and sewer (Y = −4 m) | Proposal |

### Reference scenario

Proposal (research). A grenade explodes in an underground pump room. The damage breaks the pipes running through the room based on distance and condition. Each break is a leak. The level rises, exceeds the drain and the threshold, and runs down the stairs. The player below sees the water rise and suffers wading, noise and drowning.

## Couplings

Proposal (research unless noted).

| Cause | Chained effect |
| --- | --- |
| Sector without power | Pumps stop; the rooftop tank empties and then pressure drops |
| Sector without power | Fridges shut off; food rots |
| Sector without power | Battery alarms go off and attract zombies |
| Darkness | Changes zombie and player perception |
| Water over a live cable | Lethal electrified puddle (prototype) |
| Explosion in a tunnel | Piston effect: more damage than in open air; bursts mains, wiring falls into the water (prototype) |
| Large explosion under a street | Collapses the slab and opens a sinkhole between street and sewer (prototype) |
| Wall with pipes and cables | A breach cuts a sector or floods a basement; stripping copper from a house is a real decision |
| Sparks | Can start fires (prototype) |

## Decay and repair

- With no operators, substations accumulate faults on a cold-level timer. Proposal (research).
- Repair with parts whose Q decides how long the fix lasts. Proposal (research).
- Restoring a district is a mid-game goal. Proposal (research).

| Component | Repair | Built from scratch (late game) |
| --- | --- | --- |
| Pole | Shore up or re-raise with a winch | Fell a straight pine, debark, treat with charcoal tar |
| Insulator | Salvage from abandoned poles | Wheel-thrown clay, vitrified above 1,100 °C (porcelain) |
| Cable | Splice with terminal blocks and tape | Smelt copper, forge rod, draw wire; insulate with latex or resin |
| Generation | Spark plugs, oil, filters | Water or wind mill with alternator; wood gasifier (CO + H₂) |
| Water tower | Patch leaks with clamps and rubber | Wooden scaffold with tank; pumping windmill or piston pump |
| Plumbing | Replace elbows and nipples | Lead pipes or tar-treated cane, oakum and grease joints |
| Transformer fuse | Replace cartridge with a hot stick | Calibrate lead/tin wire to melt at the critical amperage |

The whole table is a Proposal in its details; the rule that everything can be repaired or built is Decided.

## Readability

- Isometric view: toggleable overlays for power (dead sectors) and water (pressure, flooded rooms). Proposal (research).
- Third person: the same information, diegetic. Dark windows, hissing pipes, puddles under doors. Proposal (research).

## First playable slice

Proposal (research). One block with 3–4 buildings, one with a pump room in the basement. One substation and about 20 poles. Slice win conditions:

1. Blow up a pole and the block goes dark.
2. Grenade in the basement: water reaches the floor below via the stairs.
3. Repair the pole with a crafted part; its Q decides how long it lasts.

## Open

- Is the hero system the infrastructure or the body?
- World size: a town, a district or a city?
- How many days does the network take to degrade on its own without players?
- Do we model voltage and amperage, or only power in W per connected component?

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- National-grid load factor, substations per 100,000 people, reservoir and pipe sizing rules: not found, so grid and water sizing has no anchor. ([world-systems-resources-networks](../research/world-systems-resources-networks.md))
- Pipeline logic and cost are unsourced (the rule is a proposal). ([world-systems-resources-networks](../research/world-systems-resources-networks.md))

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [concept-v2](../research/concept-v2.md): Juego Zombie · Concept v2
- [world-systems-resources-networks](../research/world-systems-resources-networks.md): World systems: where resources are, and how grids and networks are laid out

## Prototype sources

- redes-infraestructura-servicios.md
- sistemas-supervivencia-mundo.md
- dinamicas-inmersivas-avanzadas.md
- quimica-combustibles-balistica-clima.md
- engine-research (new engine research)
