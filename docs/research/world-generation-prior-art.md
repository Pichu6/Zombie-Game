# World generation: prior art

**Date:** 2026-10-07. **Status:** research note, not a decision. **Feeds:** [world systems](../design/world-systems.md), [zoom continuum](../design/zoom-continuum.md), [spec 30](../tier0/specs/30-world-grid.md), [spec 31](../tier0/specs/31-terrain-climate.md). **Evidence:** web-search summaries and a few fetched pages. Several primary pages (Red Blob Games, the Eurographics and HAL paper pages, frozenfractal.com) were **blocked by the network proxy**, so what is said about them is the search engine's summary of them, not a reading of the source. Licences are as reported; confirm each on the project's own page before copying anything. **IDs:** WG.

The user asked not to reinvent the wheel before building more of the world generator ([spec 30](../tier0/specs/30-world-grid.md), [spec 31](../tier0/specs/31-terrain-climate.md)).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| WG-1 | Red Blob Games' polygon maps use a Delaunay+Voronoi mesh (Poisson-disc points, Lloyd relaxation), noise elevation, moisture from distance to sea and rivers, and a Whittaker biome table; mapgen4 adds wind, evaporation and rain and regenerates in real time. | reported | https://www.redblobgames.com/maps/mapgen4/ |
| WG-2 | mapgen4 deliberately prefers a look over realism ("elevation to match the desired look"); it is Apache 2.0. | reported | https://github.com/redblobgames/mapgen4 |
| WG-3 | WorldEngine (Python, MIT, active) chains plate simulation (pyplatec), noise, precipitation with latitude and rain shadow, erosion, humidity and permeability, Holdridge life zones and optional rivers. | reported | https://github.com/Mindwerks/worldengine |
| WG-4 | Songs of the Eons (SongsOfGPL, GPL) simulates tectonics, several climate models, erosion, watersheds and lakes from water movement, soils coupled to water, glaciers and plant growth; its code must not be copied without a decision. | reported | https://github.com/Calandiel/SongsOfGPL |
| WG-5 | Cordonnier et al. 2016 generate terrain as the balance of uplift and the stream power law, dh/dt = U - K A^m S^n (A drainage area, S slope, m/n about 0.5), giving consistent ranges and river networks at low cost. | reported | https://diglib.eg.org/handle/10.1111/cgf12820 |
| WG-6 | A 2024 follow-up gives a fast, consistent analytical (multigrid) solution of the stream power erosion. | reported | https://diglib.eg.org/handle/10.1111/cgf15033 |
| WG-7 | Cortial et al. 2019 simulate moving plates on a sphere (subduction, collision, rifting, then amplification), giving arcs, collision zones and coasts that look less random than static boundary rules. | reported | https://diglib.eg.org/handle/10.1111/cgf13614 |
| WG-8 | Barnes et al. 2014 Priority-Flood fills depressions and labels watersheds optimally with a priority queue in O(n log n), giving flow directions, lakes and drainage on any graph. | reported | https://arxiv.org/pdf/1511.04463 |
| WG-9 | Dwarf Fortress generates midpoint-displacement elevation, then temperature (latitude and height), rainfall with an orographic switch, drainage, volcanism and savagery, with variation sliders per axis that change numbers, not rules. | reported | https://dwarffortresswiki.org/index.php/40d:Advanced_World_Generation |
| WG-10 | Cataclysm: DDA generates an abstract overmap first and the detailed local map on discovery from JSON definitions. | reported | https://docs.cataclysmdda.org/OVERMAP.html |
| WG-11 | Azgaar's Fantasy Map Generator (MIT, reported) builds cell-based maps with climate, states, burgs and routes in the browser: a reference for political layers and map editing. | reported | https://github.com/Azgaar/Fantasy-Map-Generator |
| WG-12 | Hexagon-and-pentagon grids from a subdivided icosahedron are the common planet grid, which confirms the grid choice in 0038. | reported | Red Blob Games hex guides, hexasphere projects, IcoGrid |
| WG-13 | Whittaker (temperature, rain) and Holdridge (biotemperature, rain, evapotranspiration ratio) are the usual game-sized biome tables; Koppen-Geiger is the most used by hobby world builders and has real monthly data behind it. | reported | https://epa-dccs.ornl.gov/documents/Holdridge_LifeZones.pdf |
| WG-14 | Rain from a latitude band profile plus orographic lift and rain shadow is the common cheap model (WorldEngine, Dwarf Fortress, ours). | reported | WorldEngine and Dwarf Fortress (WG-3, WG-9) |
| WG-15 | Spec 31's grid, sliders, plates idea, latitude temperature with lapse, band-and-shadow rain, pure-function biome table and keyed seeds all match common practice. | estimate | this note |
| WG-16 | Our biggest gap is erosion and rivers: stream power with uplift plus Priority-Flood drainage both run on any graph, and they are the prerequisite for soil, farmland, water supply, river settlements and routes. | estimate | this note (from WG-5, WG-8) |
| WG-17 | Stream power erosion is a cheaper upgrade than moving plates for natural-looking mountains; moving plates is a later call. | estimate | this note (from WG-5, WG-7) |
| WG-18 | Our hand-tuned biome thresholds are starting values only; Holdridge or Koppen would replace them with published ones, and Koppen needs monthly data. | estimate | this note |
| WG-19 | Generation steps should stay fast and re-runnable from a changed input, so the map can be edited before a run (mapgen4's real-time regeneration). | estimate | this note |
| WG-20 | Proposal: the next generator spec is hydrology and erosion on the cell graph (Priority-Flood flow and lakes, flow accumulation, rivers above a flow threshold, a few stream power steps against plate uplift, then re-derive rain and biome), written from the published equations, not from GPL code. | estimate | this note |

## What exists

| Work | What it does | Licence (reported) | Use for us |
| --- | --- | --- | --- |
| **Red Blob Games, polygon map generation and mapgen4** (Amit Patel) | Delaunay+Voronoi mesh (Poisson-disc points, Lloyd relaxation); noise elevation; moisture from distance to sea and rivers; Whittaker biome table; mapgen4 adds wind, evaporation, rain and regenerates in real time. States it prefers a *look* over realism ("elevation to match the desired look") | mapgen4 Apache 2.0 | Confirms our biome-from-temperature-and-rain approach. Its realism is deliberately low, so it is a pattern for the *interface* (paint, regenerate fast), not for the physics |
| **WorldEngine** (Python, Bret Curtis and Federico Tomassetti) | Plate simulation (pyplatec) for mountain chains, noise, precipitation with latitude and rain shadow, erosion, humidity and permeability, Holdridge life zones, optional rivers | MIT, active | Same pipeline shape as spec 31; a reference to compare our maps against. Holdridge is the alternative to our Whittaker-style table |
| **Songs of the Eons / SongsOfGPL** (Calandiel) | Tectonic simulation and orogeny, multiple climate simulations, erosion, watersheds and lakes from simulated water movement, soils from a bedrock erosion model coupled to the water, glaciers, plant growth; later a successor, Gleba | GPL | The same ambition as our whole world-systems list (soil, flora, water). **GPL: read for ideas, do not copy code** into the game without a decision |
| **Cordonnier et al. 2016, "Large Scale Terrain Generation from Tectonic Uplift and Fluvial Erosion"** (Computer Graphics Forum) | Terrain as the balance of uplift and the **stream power law**, `∂h/∂t = U − K A^m S^n` (A drainage area, S slope, m/n about 0.5); consistent mountain ranges and river networks at low cost. A 2024 follow-up gives an **analytical** (multigrid) solution that is fast and consistent | paper | The proven way to get *believable rivers and mountains*: uplift from plates (we have it as boundary sources), erosion by stream power over a flow graph. Our heuristic elevation has no rivers or valleys; this is the gap |
| **Cortial et al. 2019, "Procedural Tectonic Planets"** (Computer Graphics Forum) | Plate tectonics on a **sphere**: moving crust, subduction, collision, rifting, with later amplification to detail | paper | Closest prior art to our plates on a sphere. Ours is static boundary rules; theirs moves plates over time and so makes arcs, collision zones and coasts that look less random |
| **Barnes et al. 2014, Priority-Flood** (Computers and Geosciences) | Optimal depression filling and watershed labelling with a priority queue, O(n log n); gives flow directions, lakes and drainage | paper, reference code on GitHub | The standard way to turn heights into **flow directions, lakes and rivers** on any graph, ours included. Needed before rivers, and rivers are needed by soil, farming and routes |
| **Dwarf Fortress** | Midpoint-displacement elevation, then temperature (latitude and height), rainfall (with an orographic switch), drainage, plus volcanism and "savagery"; variation sliders per axis | game, no code | Same four-field idea; confirms world *sliders* that change numbers and not rules |
| **Cataclysm: DDA** (overmap, mapgen) | Abstract overmap first (cities, roads, building types), detailed local map generated **on discovery** from JSON definitions | open source | The same "generate on approach" split already decided here ([0017](../decisions/0017-slow-endgame-and-generation-on-the-go.md)); its JSON region layouts are a model for our zone and building kits |
| **Azgaar's Fantasy Map Generator** | Cell-based fantasy maps with climate, states, burgs, routes, in the browser | MIT (reported) | Reference for *political* layers (states, cultures, routes) that our steps 6-8 will need, and for the editing experience in 0037 |
| **Hex and geodesic planet grids** (Red Blob Games hex guides, hexasphere projects, IcoGrid) | Hexagon-and-pentagon grids from a subdivided icosahedron | various | Confirms the grid choice in [0038](../decisions/0038-grid-sector-zone-sizes-accepted.md). Our spacing warp for near-equal area is a small known fix |

## Standard climate and biome models

- **Whittaker** (temperature, rain) and **Holdridge** (biotemperature, rain, evapotranspiration ratio) are the usual game-sized biome tables. Köppen-Geiger is reported as the most used by hobby world builders and is the one with real data behind it (monthly temperature and rain), which matters once we move to monthly series.
- Rain from a **latitude band profile** plus **orographic lift and rain shadow** is the common cheap model (WorldEngine, Dwarf Fortress, ours).

## What this says about our spec 31

1. **Keep:** the grid, the sliders, the plates idea, latitude temperature with lapse, band-and-shadow rain, a biome table as a pure function, keyed seeds. All match common practice.
2. **Our biggest gap is erosion and rivers.** Everyone who wants realistic mountains and rivers uses the stream power law with uplift, and Priority-Flood for drainage. Both work on any graph, so they can run on our cell graph. They are also the **prerequisite** for soil, farmland, water supply, settlements by rivers and routes, which are the next steps in [world systems](../design/world-systems.md).
3. **Our plates are static** (boundary rules from random plate motion). Cortial et al. show the moving-plate version; whether it is worth the cost is a later call. The cheaper upgrade is the stream power erosion above, which makes mountains look natural without moving plates.
4. **Biome table:** Holdridge or Köppen would replace our hand-tuned thresholds with published ones, and Köppen needs monthly data (a later step). Our thresholds, tuned by eye, are starting values only.
5. **Interface:** mapgen4's real-time regeneration while painting is what 0037's "edit the map before the run" needs, so generation steps should stay fast and re-runnable from a changed input.

## Proposal (not decided)

Next generator spec: **hydrology and erosion** on the cell graph: Priority-Flood flow directions and lakes, flow accumulation, rivers as cells above a flow threshold, then a few steps of stream power erosion against the plate uplift, then re-derive rain and biome. Implement from the published equations (own code, deterministic), not from GPL code. Check the equations and parameter ranges against the papers once the primary pages can be read.

## Sources

- Red Blob Games: [polygon map generation](https://www.redblobgames.com/maps/polygon-map-generation/), [mapgen4](https://www.redblobgames.com/maps/mapgen4/) and its [repository](https://github.com/redblobgames/mapgen4) (Apache 2.0).
- [WorldEngine](https://github.com/Mindwerks/worldengine) (MIT).
- [SongsOfGPL](https://github.com/Calandiel/SongsOfGPL) and its [FOSS version](https://github.com/Calandiel/SongsOfFOSS) (GPL).
- Cordonnier et al., [Large Scale Terrain Generation from Tectonic Uplift and Fluvial Erosion](https://diglib.eg.org/handle/10.1111/cgf12820) (2016); [Physically-based Analytical Erosion for fast Terrain Generation](https://diglib.eg.org/handle/10.1111/cgf15033) (2024).
- Cortial et al., [Procedural Tectonic Planets](https://diglib.eg.org/handle/10.1111/cgf13614) (2019).
- Barnes et al., [Priority-Flood](https://arxiv.org/pdf/1511.04463) (2014).
- Dwarf Fortress wiki, [world generation](https://dwarffortresswiki.org/index.php/40d:Advanced_World_Generation); Cataclysm: DDA [overmap](https://docs.cataclysmdda.org/OVERMAP.html) and [mapgen](https://docs.cataclysmdda.org/MAPGEN.html).
- [Azgaar's Fantasy Map Generator](https://github.com/Azgaar/Fantasy-Map-Generator); [Holdridge life zones](https://epa-dccs.ornl.gov/documents/Holdridge_LifeZones.pdf); frozenfractal's [Around The World](https://frozenfractal.com/blog/2023/11/2/around-the-world-1-continents/) series.
