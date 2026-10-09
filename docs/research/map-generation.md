# Map generation: Earth-like land on the world grid

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [spec 31](../tier0/specs/31-terrain-climate.md), [world systems](../design/world-systems.md). **Evidence:** read in full or in the relevant parts: Red Blob Games "Making maps with noise functions", "Polygon map generation" and the mapgen4 `map.ts` source; Inigo Quilez's fBm and domain-warping articles; Azgaar's heightmap templates and generator source; Martin O'Leary's `terrain.js` (mewo2/terrain); Undiscovered Worlds `globalterrain.cpp`; Cortial et al., "Procedural Tectonic Planets" (2019, HAL PDF); Mandelbrot 1967 (PDF); the 1-degree Earth hypsometry table (Gates and Nelson, tabulated by S. Dutch). Everything else from summaries (`reported`): WorldEngine and PlaTec, Tectonics.js, World Orogen, Worldbuilding Pasta, Wikipedia on islands and shelves, abstracts for Sayre et al. 2019, Charette and Smith 2010, Courtillot et al. 2003. Not confirmed: Harris et al. 2014's shelf share (paywalled), Britannica's modal heights (403), the PlaTec thesis method, the primary source of the 6.7 % island share, the length of the Hawaiian-Emperor chain. **IDs:** MG.

On 2026-10-09 the quality check of the generated map against Hearts of Iron IV, Plague Inc, Europa Universalis IV and a political world map (kept outside the repo, `reference/QC-mapa-mundo-2026-10-09.md`) found blobby continents that follow plate outlines, few islands, sea holes inside continents and a different planet at each grid resolution. This note gathers how other generators make land look like continents and the Earth numbers to check against, for spec 31's revised rules 2-4.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| MG-1 | Standard fBm: octaves at doubling frequency with amplitude halved each octave (gain 0.5); octaves decorrelated by offsets or separate seeds; wavelength = map size over frequency | sourced | https://www.redblobgames.com/maps/terrain-from-noise/ |
| MG-2 | Island shaping blends noise with a distance function (lerp toward 1 - d, mix about 0.5), mostly on the low octaves so fine detail stays uniform; raising elevation to a power above 1 flattens lowlands | sourced | https://www.redblobgames.com/maps/terrain-from-noise/ |
| MG-3 | Ridged noise 2(0.5 - abs(0.5 - n)), with higher octaves multiplied by lower ones so detail gathers on mountains | sourced | https://www.redblobgames.com/maps/terrain-from-noise/ |
| MG-4 | Quilez: gain 2^-H; H = 1 (gain 0.5) gives a power spectrum near f^-3, which he roughly measured on mountain skylines (informal by his own account); lacunarity slightly off 2 (1.99, 2.01) avoids aligned artefacts | sourced | https://iquilezles.org/articles/fbm/ |
| MG-5 | Quilez domain warping nests fbm(p + 4 q) with q itself an fbm vector; the factor 4 is artistic and very swirly; the offsets only decorrelate | sourced | https://iquilezles.org/articles/warp/ |
| MG-6 | mapgen4 weights coast noise by (1 - e^4) so it is strongest near sea level and weak deep inland or high up, using only its three highest octaves; the ocean floor is elevation times (depth + noise) | sourced | https://github.com/redblobgames/mapgen4 |
| MG-7 | mapgen4 mountains come from a breadth-first distance field from peak seeds with jittered step lengths; land blends hills and mountains with weight e^2 | sourced | https://github.com/redblobgames/mapgen4 |
| MG-8 | Polygon map generation tells ocean from lakes by a flood fill from the map border, sets elevation by distance from the coast, then redistributes it to a target distribution with more lowland than highland; mapgen4 carves channels to remove local minima | sourced | http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/ |
| MG-9 | Azgaar builds continents from scripted primitives: a few big hills and many small ones, ranges near edges, straits, troughs and pits for gulfs and inland seas, smoothing and an edge mask; sea level at 20 of 100 | sourced | https://github.com/Azgaar/Fantasy-Map-Generator/blob/master/src/data/heightmap-templates.ts |
| MG-10 | Azgaar spreads blobs ring by ring in cell steps and retunes the decay by a cell-count table (0.93 at 1,000 cells to 0.9973 at 100,000): a patch for spreading in cells instead of km | sourced | https://github.com/Azgaar/Fantasy-Map-Generator/blob/master/src/generators/heightmap-generator.ts |
| MG-11 | O'Leary: a rough base relaxed and shaped, five erosion passes with rate from flux and slope, Planchon-Darboux sink fill between them, a random quantile sea level, then a coast clean-up that sinks land cells with one land neighbour and fills sea cells with one sea neighbour | sourced | https://github.com/mewo2/terrain |
| MG-12 | O'Leary's motivation: pure fractal noise makes valleys that start and stop for no reason; erosion over a rough base fixes it | reported | https://www.nationalgeographic.com/culture/article/bot-fantasy-map-generator |
| MG-13 | Undiscovered Worlds (Earth-like type): warped fractals and continent shapes, then explicit passes: remove inland seas smaller than a quarter of the cells, widen channels, shelf near -200 m and deep ocean near -5,000 m with noise, ridges, trenches, hotspot volcanoes, coast-preserving smoothing, depression fill | sourced | https://github.com/fegennari/Undiscovered_Worlds |
| MG-14 | Undiscovered Worlds hotspots: a volcano with 1-6 extinct neighbours stepped along a noise direction; submarine ones skipped near shelves | sourced | https://github.com/fegennari/Undiscovered_Worlds |
| MG-15 | Cortial et al. 2019: plates carry mixed oceanic and continental crust and terranes; subduction uplift peaks at a control distance and fades by 1,800 km; ridge -1 km, abyssal -6 km, trench -10 km; rifting splits plates along warped Voronoi cells; island arcs come from ocean-ocean subduction; the coarse crust (50-500 km) is then amplified with noise | sourced | https://hal.science/hal-02136820/document |
| MG-16 | Cortial et al. use a uniform continental erosion term and oceanic damping at 50 km sampling, not hydraulic erosion | sourced | https://hal.science/hal-02136820/document |
| MG-17 | WorldEngine uses the pyplatec plate simulation for mountain chains, then rain and erosion; the PlaTec coastline method was not read | reported | https://github.com/Mindwerks/worldengine |
| MG-18 | Worldbuilding Pasta: active margins have a trench, a coastal volcanic range and a steep coast; passive margins have wide shelves and plains; zig-zag coasts follow rift segments; plate speeds 10-20 cm/yr oceanic, 2-5 at active continental margins, under 1 passive | reported | https://worldbuildingpasta.blogspot.com/2020/01/an-apple-pie-from-scratch-part-va.html |
| MG-19 | Coastline fractal dimension (Richardson's data via Mandelbrot): west coast of Britain 1.25, Australia 1.13, German land frontier 1.15, South Africa 1.02; values depend on data and scale range | sourced | https://gsp.humboldt.edu/olm_2021/Courses/GSP_510/Articles/Mandelbrot1967.pdf |
| MG-20 | Norway's coast about 1.52; studies report coastline dimensions from about 1.02 to 1.46 | reported | https://pmc.ncbi.nlm.nih.gov/articles/PMC7973742 |
| MG-21 | From a 30 m global shoreline: 5 continental mainlands, 21,818 islands over 1 km², 318,868 under 1 km² | reported | https://www.usgs.gov/publications/a-new-30-meter-resolution-global-shoreline-vector-and-associated-global-islands |
| MG-22 | Sea islands total about 9.96 million km², about 6.7 % of Earth's land (original source not named) | reported | https://en.wikipedia.org/wiki/Island |
| MG-23 | Continental shelf about 27 million km², 7-8 % of the ocean; mean width about 80 km; shelf break about 140 m | reported | https://en.wikipedia.org/wiki/Continental_shelf |
| MG-24 | Earth on a 1-degree grid: land 29.6 % of the surface; 0-1,000 m 22.4 %, 1,000-2,000 m 4.0 %, over 2,000 m 3.2 % (so about 24 % of land is above 1,000 m); 0 to -500 m 7.2 % (about 10 % of the ocean); -4,000 to -5,000 m 23.2 % (the ocean's mode); area above -2,000 m about 40.5 % | sourced | https://stevedutch.net/usefuldata/earthhypso1deg.htm |
| MG-25 | Mean ocean depth 3,682 m | reported | https://tos.org/oceanography/article/the-volume-of-earths-ocean |
| MG-26 | 49 hotspots considered, 7 of them "primary" by deep-origin criteria (Courtillot et al. 2003) | reported | https://mantleplumes.awh.durham.ac.uk/WebDocuments/Courtillot2003.pdf |
| MG-27 | At frequency 24 (about 88,500 km² a cell) about 19 Earth islands are a cell or larger, about 4.8 % of land; at Earth size (about 5,000 km²) a few hundred qualify | estimate | this note (MG-22, island areas) |
| MG-28 | The blobby look comes from land being continental plate: on Earth plates mix crust types and most passive coasts lie inside plates (MG-15, MG-18) | derived | this note |
| MG-29 | The sea holes come from per-cell roughness against a low continental base under a quantile sea level; other generators remove them explicitly (MG-8, MG-11, MG-13) | derived | this note |
| MG-30 | Resolution coherence needs every distance in km and noise on sphere positions; a flood over cells or keyed cell ids gives another world at each resolution (MG-10) | derived | this note |
| MG-31 | Hydraulic erosion is not worth it at 300 km cells (MG-16); depression fill and flow accumulation are enough, and an erosion pass only at Earth size would make the two resolutions diverge | estimate | this note (MG-11, MG-16) |

## 1. What generators do

Noise-first generators (Red Blob) take fBm, a shaping mask and a threshold; coast detail comes from fine noise weighted toward sea level (MG-6). Template generators (Azgaar) script blobs, ranges, troughs and straits, but spread in cell steps and need per-resolution tables (MG-10), the thing to avoid. Erosion generators (O'Leary) get believable drainage at regional scale (MG-11). Hybrid planet tools (Undiscovered Worlds) add many targeted fix-up passes: inland seas filled, shelf and deep ocean set separately, hotspot chains (MG-13, MG-14). Tectonic synthesis (Cortial et al.) gets coasts from history, rifts and terranes, and amplifies a coarse crust with noise (MG-15).

## 2. What this means for us (Proposal)

All `estimate`. Spec 31's revision of 2026-10-09 already takes: noise on sphere positions in km with octaves dropped below two cell spacings (MG-1, MG-4, MG-30); one level of domain warp, gentler than Quilez's (MG-5); a margin profile by signed distance to the continent-ocean contact with noise strongest near the coast (MG-6, MG-23); a very long wave that carves continents across plates, a partial answer to MG-28; keyed hotspot chains along plate motion (MG-14, MG-26); plate seeds as keyed points.

Still open (carried to spec 31 Open): a continental-crust field separate from plates (MG-28); island arcs broken by along-strike noise; enclosed seas below about 500,000 km² filled and the quantile solved again (MG-29); ocean floor deepening with distance from ridges (MG-15, MG-24: our mean depth is about 2,600 m against 3,682); a shelf-share check (MG-23, MG-24) and a coastline-roughness check at Earth size (MG-19); Earth's elevation data put on our grids as the reference for every metric.

## Sources

- Red Blob Games: https://www.redblobgames.com/maps/terrain-from-noise/ ; http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/ ; https://github.com/redblobgames/mapgen4
- Quilez, I.: https://iquilezles.org/articles/fbm/ ; https://iquilezles.org/articles/warp/
- Azgaar, Fantasy Map Generator: https://github.com/Azgaar/Fantasy-Map-Generator
- O'Leary, M., mewo2/terrain: https://github.com/mewo2/terrain
- Undiscovered Worlds: https://github.com/fegennari/Undiscovered_Worlds
- Cortial, Y., Peytavie, A., Galin, E., Guérin, E. (2019). Procedural Tectonic Planets. https://hal.science/hal-02136820/document
- Mandelbrot, B. (1967). How long is the coast of Britain? Science 156. https://gsp.humboldt.edu/olm_2021/Courses/GSP_510/Articles/Mandelbrot1967.pdf
- Dutch, S., Earth hypsometry on a 1-degree grid (after Gates and Nelson). https://stevedutch.net/usefuldata/earthhypso1deg.htm
- Sayre, R. et al. (2019), global shoreline and islands: https://www.usgs.gov/publications/a-new-30-meter-resolution-global-shoreline-vector-and-associated-global-islands
- Charette, M., Smith, W. (2010). The volume of Earth's ocean. https://tos.org/oceanography/article/the-volume-of-earths-ocean
- Courtillot, V. et al. (2003). Three distinct types of hotspots. https://mantleplumes.awh.durham.ac.uk/WebDocuments/Courtillot2003.pdf
- WorldEngine: https://github.com/Mindwerks/worldengine ; Worldbuilding Pasta: https://worldbuildingpasta.blogspot.com/2020/01/an-apple-pie-from-scratch-part-va.html
