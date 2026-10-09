# The zoom continuum ("Google Earth effect")

**Status:** Proposal (2026-10-07), from the user's request: infinite zoom from the whole planet down to one person, with the gameplay layers (Macro, City, Individual) sitting along that zoom. Nothing here is Decided until the user confirms; section 5 lists the earlier decisions it touches.

## 1. The idea in one paragraph

One camera moves continuously from the globe to a person's face. The **layers are what you can do at a given height**, not separate maps: orders at the top, management in the middle, a body at the bottom ([macro layer](macro-layer.md) §3.3 already picks the layer by camera height, with a gap between enter and leave heights). **Zoom never changes what is true**; it changes what is *drawn* and *materialised* ([world simulation](world-simulation.md) §8). "Tricks" are the cheap stand-ins used where nobody can tell, and each has a place and a rule.

## 2. Why our grid already fits

The world grid ([spec 30](../tier0/specs/30-world-grid.md)) is **nested**: every cell centre at frequency n is exactly a cell centre at frequency 2n (tested). So going down is subdividing: each doubling gives about 4 times the cells and about a quarter the spacing.

| Level | Spacing | What lives there |
| --- | --- | --- |
| World cell (n = 101) | about 70 km | Stored fields: elevation, climate, biome, later deposits and capacity ([spec 31](../tier0/specs/31-terrain-climate.md)) |
| Sector | hundreds of km | Owner, pops as cohorts, stocks ([world systems](world-systems.md) §4.1) |
| Zone cell | 100–500 m | Land use inside settlements and worked land (0038) |
| Plot | 0.5 m build grid | One building; interior generated on approach |

From a 70 km cell to a 200 m zone cell is about 8 to 9 doublings; to the 0.5 m grid about 8 more. Only the top levels are stored; the rest are computed when approached.

## 3. Where each trick goes

| Trick | What it is | Where it is allowed | Rule that keeps it honest |
| --- | --- | --- | --- |
| **T1 Impostor** | Draw many things as one: city as lights, crowd as a texture, pops as markers | Far camera | Render only; reads state, never writes it. A transition changes how a thing is drawn, never its size ([world simulation](world-simulation.md) §8) |
| **T2 Amplification** | Add terrain detail below the stored cells with seeded noise and drainage | Region and finer | A pure function of (seed, level, cell). **Children add up to the parent**: detail heights average to the cell's stored elevation, and a river enters and leaves a cell where the coarse flow graph says. Level k adds only finer octaves, so zooming in never changes the coarse shape |
| **T3 Generate on approach** | Buildings, interiors, props made from the plot's seed and the zone record | City and finer | Saved on first visit, so a generator update never changes a visited place ([0017](../decisions/0017-slow-endgame-and-generation-on-the-go.md)); each building's ledger is made at world creation, never rolled on arrival |
| **T4 Materialise** | A count becomes a record, a record becomes a body | Near a body the player controls | Spec 28 and 29 pairs: nothing created or lost; hysteresis (dwell) so there is no flicker; a pool caps the hot set |

**The rule behind all four:** a trick may add detail *below* committed truth and may change how things are *shown*; it never replaces truth, never contradicts the parent, and never sends a value *upward* (macro numbers stay sums of micro entities).

## 3b. Two different scale problems (added 2026-10-07, from the user's point about a city of millions seen from above)

A person, a car and a door are real size ([world simulation](world-simulation.md) §8), so from a city-management height they are far smaller than a pixel. Walking into one pop's home is not the same experience as watching a city of millions, and it should not be the same drawing. Two separate things are going on:

1. **Representation scale (what is drawn).** Solved by cartographic **symbol scale**, as a map does: a thing is drawn at `max(real size, minimum screen size)`. Below the height where its real size is smaller than the minimum, it becomes a symbol that stays readable (a road is drawn wider than a real road, a pop is a dot). The crossover is continuous (the size just stops shrinking), so there is no pop; when the type of drawing changes (mesh → block → footprint → density), the two are cross-faded over a range of heights. **Physics, collision and the simulation always use real size**; only the drawing is exaggerated, and only above the crossover. This does not break "nothing is ever shrunk": it is a drawing rule, like the width of a motorway on a road map.
2. **Information scale (what the player can read and do).** A million pops cannot be read one by one at any size. At city height the useful view is **aggregates and overlays** (density, traffic flow, power load, unrest, supply), which is what the City and Macro layers' verbs act on ([macro layer](macro-layer.md) §3.3). The layer changes what is *shown* (numbers and flows) as well as how big things are drawn.

| Height band (proposed) | Drawn as | Size | Selectable |
| --- | --- | --- | --- |
| Interior, street (under about 100 m) | Real meshes; pops as bodies where hot | Real | Anything, as itself |
| District (100 m to 2 km) | Buildings extruded from their footprints; pops and vehicles as markers | Real until the minimum, then a fixed screen size (about 6 px) | A building, a marker (a real pop) |
| City (2 to 30 km) | Buildings merged into zone-coloured blocks; roads at minimum width; **flow lines** for traffic; a **sample** of pops as dots; overlays | Symbol scale | A block, a road, an overlay cell; a dot opens a real pop |
| Region (30 to 500 km) | City as a footprint and night lights; major roads and rails only | Symbol scale | A city, a sector |
| Globe | City as an icon sized by population; borders; weather | Symbol scale | A sector, a nation |

**Rule for the million-pop city:** pops are never drawn all. The dots are a **sample** of the real pops, chosen by a keyed draw so the same pops stay shown from frame to frame; the sample's density equals the true density, the numbers on screen are exact sums over the whole city (never counted from the sample), and clicking a dot opens that real pop (a lazy pop becomes a record, [spec 28](../tier0/specs/28-pop-levels.md)). A cap on dots drawn is a rendering budget, not a rule of the simulation.

**Hiding the last jump, from the city into one home:** the dive itself is the cover. While the camera descends, the building's layout is generated (trick T3) and its pops are made records, so nothing waits on the player; the roof and upper floors fade out as the camera passes the roofline; the symbol dots turn into bodies by cross-fade. Taking a body far from the camera uses the GTA-style switch instead ([world simulation](world-simulation.md) §8).

**Prior art for the cost side:** [low-resource city simulation](../research/low-resource-city-simulation.md). Short version: ASCII City draws a city with a cost bounded by the screen (a raycaster over a height-map grid), it does not simulate one; for real populations, the rules that matter are a drawing budget set by the screen, a pure "where is this pop at time t" function for the dots, and no per-tick rule that scans every entity.

**The map itself is real size** (a 70 km world cell is 70 km). It is never shrunk to fit the screen; the player gets around by zooming and by fast travel with time passing, not by a smaller planet.

## 4. What this asks of the generator (the part we are building now)

- **Fields must be defined at every scale**, not only on the 70 km cells. Spec 31 stores cell values; T2 needs the same fields evaluable at any point from the cell and its neighbours (interpolate the stored values, add seeded detail constrained by them).
- **Noise on the sphere, not per tile:** defined from the position on the planet, so neighbouring tiles agree at every level and there are no seams.
- **Hydrology is the first thing that crosses scales**, so the next spec (hydrology and erosion, [prior art](../research/world-generation-prior-art.md)) should be written multi-resolution from the start: flow directions at the coarse level, refined inside each cell under the parent's constraints, with the same stream-power and Priority-Flood logic at each level.
- **Precision:** planet-scale coordinates break 32-bit floats near the ground; positions are kept as (cell, local offset) with a floating origin in the renderer. The renderer is separate from the simulation ([CLAUDE.md](../../CLAUDE.md)), so this is its problem, but cell-local coordinates must exist in the sim's data.
- **Real Earth later** replaces the coarse levels with data and keeps T2 and T3 below them, unchanged.

## 5. Where this meets earlier decisions (the user decides)

1. **[Cameras and modes](cameras-modes.md) line 46** prefers explicit modes over a continuous 2.5–60 m zoom, because the prototype dropped it. This note proposes the opposite, one continuous camera, with the layer chosen by height as macro layer §3.3 already says.
2. **Local zone as a 1–2 km crop** ([0021](../decisions/0021-recommended-defaults.md), world simulation §8). With T2 and T3 any place can be approached, so the crop is no longer needed to *see* anywhere. What stays small is the **hot set** (bodies and fields running every tick), which still follows the player's bodies and is capped. Proposal: keep real size and real walking speed everywhere; drop the crop as a limit on where the camera may go, keep it as the default start area.
3. **Time:** the calendar runs 24 times faster than a walking body, so a zoom to a distant town shows a place whose people were simulated cold (jump forms, [spec 27](../tier0/specs/27-detail-levels.md)); on arrival they are caught up, then promoted.
4. **Fog:** zooming out while playing a body shows what the character believes, not the truth ([macro layer](macro-layer.md) §3.3). Zoom does not grant vision.

## 6. Open questions

1. Truly continuous (no snap), or continuous with the layer shown in the interface changing at the enter and leave heights? Recommendation: continuous camera, layer switches at heights with a gap.
2. Should every building in the world be enterable (generated on approach), or only inside a bounded area? T3 makes the first possible; cost is first-visit generation time and save size.
3. Is the GTA-style switch still wanted for taking a *different* body far away, with plain zoom for everything else ([world simulation](world-simulation.md) §8)? Recommendation: yes.
4. Camera tilt: should the city view be a tilted, miniature-style perspective (as in city builders) that straightens to top-down at the Region height, or always top-down? Recommendation: tilted in City, so buildings read as buildings.
5. The height bands and the 6 px minimum are starting values to tune by looking, once there is a renderer.

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- Should ids encode their grid level so parent and child are bit operations, as H3 does? ([pop-storage-and-lenses](../research/pop-storage-and-lenses.md))
- Which crowd tier does each height band use, and over what distance does the cross-fade run? ([renderer](../research/renderer.md))

### From research (zoom-and-layer-two, 2026-10-09)

The note proposes six height bands by visible ground width (street under 150 m, tactical 150 m-1 km, district 1-5 km, city 5-40 km, region 40-500 km, globe), one continuous camera, and only three hard jumps: taking a body, materialising a new place, opening an office ([ZL-44, ZL-45](../research/zoom-and-layer-two.md)). Its questions:

- Are the band borders right (1920 px screen, 6 px minimum)? Tune by looking once a renderer exists. ([zoom-and-layer-two](../research/zoom-and-layer-two.md))
- Should the camera tilt automatically coming down, as Google Earth does ([ZL-10](../research/zoom-and-layer-two.md)), and from which band? ([zoom-and-layer-two](../research/zoom-and-layer-two.md))
- Does a minimap exist, or does zoom replace it ([ZL-17](../research/zoom-and-layer-two.md))? ([zoom-and-layer-two](../research/zoom-and-layer-two.md))
- How long may materialising a new place take before it feels like a loading screen? ([zoom-and-layer-two](../research/zoom-and-layer-two.md))

## Open

### From research (terrain-hydrology-climate, 2026-10-09)

The note proposes, in order: endorheic basins with a lake balance (about 18 % of land drains inland), a one-pass steady-state stream power solve on the Priority-Flood order, monthly temperature rebuilt from mean and range plus Koppen classes, biomes from Koppen, fertility from geological setting and climate, multi-resolution rivers with the parent's crossings fixed, and wider river tests (our largest river matches the Orinoco or Congo, not the Amazon). Its questions:

- Is the planet grid allowed to change elevation after spec 31 (erosion changes saved fields and the golden hash), or does erosion only feed new fields? ([terrain-hydrology-climate](../research/terrain-hydrology-climate.md))
- Lake evaporation rate over open water: PET from RV-5 or a separate lake rule? Caspian and Great Salt Lake evaporation numbers were not read. ([terrain-hydrology-climate](../research/terrain-hydrology-climate.md))
- Stream power K, m and the uplift scale: read Cordonnier 2016 and Braun and Willett 2013 for the values they used. ([terrain-hydrology-climate](../research/terrain-hydrology-climate.md))
- Rain seasonality: is a shifted latitude profile good enough for Köppen's s/w letters, or do we need monsoon winds? ([terrain-hydrology-climate](../research/terrain-hydrology-climate.md))
- The combined Ganges-Brahmaputra-Meghna discharge and basin area (probably near 38,000 m³/s and 1.7 M km², unconfirmed). ([terrain-hydrology-climate](../research/terrain-hydrology-climate.md))
- Köppen main-class land shares on Earth: compute from Beck's GeoTIFF in the data pipeline. ([terrain-hydrology-climate](../research/terrain-hydrology-climate.md))
- GAEZ suitability shares by climate zone: needed to turn HY-58 from a total check into a per-class table. ([terrain-hydrology-climate](../research/terrain-hydrology-climate.md))
- Hexagonal child weights for HY-50: do we store explicit child area shares, or move to an aperture that nests exactly? ([terrain-hydrology-climate](../research/terrain-hydrology-climate.md))
- Which classification Songs of the Eons uses. ([terrain-hydrology-climate](../research/terrain-hydrology-climate.md))

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [detail-levels](../research/detail-levels.md): Detail levels: one simulation, read and run in chunks
- [low-resource-city-simulation](../research/low-resource-city-simulation.md): Low-resource city simulation: "ASCII City" and the games that fake or tame big populations
- [pop-storage-and-lenses](../research/pop-storage-and-lenses.md): Pop simulation, saving data, and "lenses" across zoom
- [zoom-and-layer-two](../research/zoom-and-layer-two.md): Zoom from planet to street, and what the City layer plays like
- [world-generation-prior-art](../research/world-generation-prior-art.md): World generation: prior art
- [renderer](../research/renderer.md): The renderer, at engineering level
- [terrain-hydrology-climate](../research/terrain-hydrology-climate.md): Terrain, hydrology and climate: erosion, rivers, inland basins, Koppen, soils, multi-resolution terrain
