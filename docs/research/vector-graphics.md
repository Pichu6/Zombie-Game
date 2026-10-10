# Vector graphics: Battlezone, line rendering today, vector maps, and where lines fit our zoom

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [zoom continuum](../design/zoom-continuum.md), [art direction](../design/art-direction.md), [cameras and modes](../design/cameras-modes.md), [0063](../decisions/0063-real-size-planet-and-the-zoom-continuum.md), [0002](../decisions/0002-engine-stack.md), [spec 16](../tier0/specs/16-view-2d.md), [spec 30](../tier0/specs/30-world-grid.md), [spec 52](../tier0/specs/52-land-water-capacity.md), [spec 06](../tier0/specs/06-save-replay.md), [renderer](renderer.md), [zoom-and-layer-two](zoom-and-layer-two.md). **Evidence:** one web-research pass on 2026-10-09.

Read first-hand:
- Jed Margolin, "The Secret Life of Vector Generators" (the whole page, in two parts);
- the 6502disassembly.com Battlezone project page and its math box page, and the Asteroids project page;
- IEEE Spectrum, "The Invention of Battlezone";
- the Vectrex `Wait_Recal` page (Hochschule Pforzheim Vectrex lab);
- the Mapbox Vector Tile spec 2.1, the MapLibre style spec (sources page), the tippecanoe README (felt fork), the OpenMapTiles schema page, the Stadia Maps raster-to-vector guide, the Protomaps downloads docs, the deck.gl performance guide;
- Rye Terrell's "Instanced Line Rendering", Bærentzen's wireframe page, Lengyel's "A Decade of Slug", the msdfgen README;
- the Escapist article on DEFCON.

From search summaries only (not opened): the Vulkan line-rasterization refpages and the MoltenVK, ANGLE and SwiftShader line-width reports; the mirror of Käfer's Mapbox post on antialiased lines; the Valve 2007 SDF paper; Loop and Blinn (GPU Gems 3 ch. 25 and the 2005 paper); the helper-lane patent and Fatahalian et al. 2010; the Vello README figure; the Google Maps 5 coverage (TechCrunch); the OSM tile disk usage wiki; the Mapbox glyph and collision-detection docs; the game articles (Superhot, Geometry Wars, Rez, Uplink, Mini Metro, Rymdkapsel).

Could not confirm: a vector count per frame for any Atari game; the Vectrex Timer 2 count in cycles; the text of Google's 2010 "under the hood" post (the page body did not load); how Highfleet draws its maps (nothing found); Lucas Pope's own account of edges in Obra Dinn; what share of Vulkan devices report `wideLines`. **IDs:** VG.


The user's idea: look at Battlezone (1980) and vector graphics in general. Maybe drawing with lines is cheaper and faster than textured 3D, and maybe the idea helps elsewhere in the project. Short answer:
- Vector displays were cheap in 1980 because memory was expensive, not because lines are cheap to draw. On a GPU, a line is two triangles, so a line layer costs about the same as any flat-coloured geometry of the same screen area.
- Where vectors really pay for us is the **map**: a few megabytes of geometry and one style table can draw the globe, a region and a city at any zoom. Digital maps made that move between 2010 and 2015. It is the missing half of our symbol-scale rule.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| VG-1 | Battlezone runs on three processors: a 6502 at 1.5 MHz for all game logic, a "math box" (four AMD 2901 4-bit slices forming a 16-bit unit at 3 MHz) for perspective arithmetic, and Atari's Analog Vector Generator (AVG) for drawing; they run in parallel or in sequence, and the designers often just waited for the math box | sourced | IEEE Spectrum "The Invention of Battlezone"; 6502disassembly.com/va-battlezone |
| VG-2 | Battlezone memory: 1 KB of RAM (zero page and stack included), 4 KB of vector RAM split into two 2 KB halves for double buffering, 4 KB of vector ROM (about half of it vertex coordinates and math tables, not draw commands), 12 KB of program ROM | sourced | 6502disassembly.com/va-battlezone |
| VG-3 | There is no frame buffer: the 6502 writes a display list of 16-bit draw instructions (relative moves and draws, a "centre" command, subroutine calls with a stack) into vector RAM, and the generator walks the list and steers the beam | sourced | 6502disassembly.com/va-battlezone; Margolin, "The Secret Life of Vector Generators" |
| VG-4 | Battlezone has no vertical sync; software sets the refresh: a 3 kHz timer gives a 250 Hz interrupt, and the display list is restarted every 6th interrupt, 41.7 Hz | sourced | 6502disassembly.com/va-battlezone |
| VG-5 | Draw time is beam travel: Atari's Digital Vector Generator (DVG) drew at about 1 screen unit per µs (1 MHz clock), so a 50-unit vector takes 50 µs; a vector drawn faster looks dimmer, so speed is normalised to keep brightness even; on Star Wars the beam crosses the 1,024-unit screen in 160 µs | sourced | Margolin |
| VG-6 | At 1 unit per µs and 60 Hz, the beam has about 16,700 units of travel per frame, roughly 16 screen widths, shared by visible lines and blank moves; the budget is total line length, and a scene that needs more takes longer, so refresh drops | derived | VG-5 |
| VG-7 | Too much drawing shows as flicker: refresh falls as the vector load rises, and many vector monitors used long-persistence phosphor to hide it, which left trails (Asteroids) | reported | Wikipedia "Vector monitor"; Blur Busters forum, search summary |
| VG-8 | The AVG uses an integrator (a DAC feeding an op-amp with a capacitor) to make linear ramps; normalisation is done at run time because Battlezone's objects are computed during play; a window circuit blanks anything outside a rectangle, which keeps game vectors out of the radar and replaces software clipping | sourced | Margolin |
| VG-9 | Why vectors in 1978-1980: a 16 Kbit DRAM cost about $4.80; one 256×256×4-bit frame buffer took 16 chips (about $74), two took $148, more than the allowed cost of the whole board; two 512×512×4 buffers would have been 128 chips, about $614 | sourced | Margolin |
| VG-10 | A 1-bit frame buffer at Battlezone's 1024×768 resolution would be 96 KB, about 19 times its 1 KB of RAM plus 4 KB of vector RAM | derived | VG-2, VG-11 |
| VG-11 | Battlezone's drawing space is about ±512 by ±384 units, with the hardware accepting ±1024 (off-screen draws are blanked) | sourced | 6502disassembly.com/va-battlezone |
| VG-12 | Battlezone's math box is a fixed-function 3D helper: an in-plane rotation (movement is confined to a plane, so the transform is 2×2 instead of 4×4), a divide by depth for screen x and y, and an approximate distance 0.375·dx + dy; its clip function is unused because the hardware blanks off-screen lines | sourced | 6502disassembly.com/va-battlezone/mathbox.html; IEEE Spectrum |
| VG-13 | Battlezone culls whole objects, not lines: by centre point against a near plane, a far plane and a loose 45° field-of-view test on the 6502; only visible objects get their vertices transformed; a very close object vanishes at once when its centre leaves the view | sourced | 6502disassembly.com mathbox page; IEEE Spectrum |
| VG-14 | Battlezone does not remove hidden lines in general (IEEE Spectrum); the disassembly calls its hiding "approximate", with tank treads visible through the body at some angles. Distance is shown by brightness: far objects are drawn dimmer | sourced | IEEE Spectrum; 6502disassembly.com/va-battlezone |
| VG-15 | The mountains, moon and volcano are a 4,096-unit cylindrical strip with 1,024 units (90°) visible, while battlefield objects use a 45° cone, so the two drift out of step when the player turns; on a hit, the horizon drops a few units and recovers, faking a tilt | sourced | 6502disassembly.com/va-battlezone |
| VG-16 | Battlezone's radar shows enemy tanks and missiles only (not obstacles or saucers), as dots that flash bright and fade | sourced | 6502disassembly.com/va-battlezone |
| VG-17 | The number of objects strained all three processors; the designer was asked whether the same effect could be had with fewer lines and vertices | sourced | IEEE Spectrum |
| VG-18 | The DVG (Lunar Lander, Asteroids) used 10-bit DACs, an effective 1024×768 grid, so its lines stair-step like a frame buffer; the AVG (Battlezone onward) has no stair-stepping. Asteroids allows up to 27 asteroids on screen | sourced | Margolin; 6502disassembly.com/va-asteroids |
| VG-19 | Tempest added colour on a Wells-Gardner colour XY monitor (pincushion correction on the game board); Star Wars used a 6809 and a custom multiply-accumulate unit (one equation in 2.58 µs at 12 MHz) and made the Death Star explosion defocus by overdriving the colour inputs | sourced | Margolin |
| VG-20 | The Vectrex draws by setting X and Y beam velocities, opening the integrators for a timed interval and closing them; its BIOS `Wait_Recal` waits on a timer to hold 50 Hz, then recalibrates the beam; a beam held near the centre too long dims the screen (protection) | sourced | Pforzheim Vectrex lab, `Wait_Recal`; hardware summary from Wikipedia "Vectrex" |
| VG-21 | Vector monitors fell out of use as raster memory got cheaper; they were also known as troublesome to keep running | reported | Wikipedia "Vector monitor"; pc2jamma.mameworld.info monitor guide, search summary |
| VG-22 | In Vulkan, lines wider than 1 px are an optional feature (`wideLines`); MoltenVK on Apple reports no wide lines (range 1 to 1), SwiftShader also reports 1 to 1, and ANGLE clamps widths for that reason | reported | Khronos forum (MoltenVK); ANGLE and SwiftShader commits, search summary |
| VG-23 | Vulkan 1.4 made line rasterization core (rectangular, Bresenham, smooth and stippled modes); only Bresenham lines are required | reported | docs.vulkan.org VK_KHR_line_rasterization refpage, search summary |
| VG-24 | Native GL lines have no joins, no caps, no fractional widths, no widths over about 10 px and no varying width, and multisampling is unreliable, so Mapbox draws lines as triangles extruded from the centre line and fades the edges in the shader for antialiasing; its first version spent 6 triangles (8 vertices) per segment | reported | K. Käfer, "Drawing Antialiased Lines with OpenGL" (Mapbox blog), via a GitHub wiki mirror |
| VG-25 | Instanced lines: each segment is one 6-vertex quad instance stretched between its two end points in the vertex shader, joins (miter, bevel, round) and caps are separate instances, and width is set in screen pixels, independent of native line support; round caps overdraw | sourced | wwwtyro.net, "Instanced Line Rendering" (2019) |
| VG-26 | GPUs shade in 2×2 pixel quads, so thin or tiny triangles waste "helper" pixels; a patent puts helpers at 40-50 % or more of shaded pixels for small triangles; merging quads across pixel-sized triangles cut shading work by about 8 times in Fatahalian et al. 2010 | reported | US 9,779,542 and related patent; Fatahalian et al., SIGGRAPH 2010, search summary |
| VG-27 | Single-pass wireframe: draw the filled triangles and colour fragments within a set distance of an edge; hidden lines come free from the normal depth test, with none of the depth artefacts of drawing lines over polygons with an offset; the authors say it "beats the traditional method" but give no numbers | sourced | Bærentzen et al., SIGGRAPH 2006 sketch and project page; NVIDIA SolidWireframe whitepaper (2007) |
| VG-28 | deck.gl: simple point layers pan and zoom at 60 fps up to about 1 M items on a 2015 MacBook Pro and fall to 10-20 fps near 10 M; GPU cost follows vertex count and fragments drawn; a 5 px radius point covers about 100 px, so overdraw dominates; CPU attribute building is the costliest update step | sourced | deck.gl developer guide, Performance |
| VG-29 | A line's fill cost is about length × width: 20,000 visible road segments of 30 px drawn as 4 px quads (2 px line plus antialiasing) is 2.4 M fragments, about 1.2 full-HD screens, plus 80,000 vertices; small for any current GPU and far below a lit, textured 3D frame | estimate | VG-25, VG-28 |
| VG-30 | The cost model is the same then and now: a vector frame's time grew with total drawn length (beam travel), and a GPU line layer's cost grows with total drawn length times width (fragments). Line count matters mainly through per-draw overhead, which batching removes | derived | VG-6, VG-28, VG-29 |
| VG-31 | Valve (2007) stored vector art as a low-resolution signed distance field (for example 64×64 from a roughly 4K binary source) drawn with plain alpha testing, with outlines, glows and drop shadows from the same texture | reported | Green, SIGGRAPH 2007 course notes, search summary |
| VG-32 | Multi-channel SDF (MSDF) keeps sharp corners by colouring edges across three channels and taking the median; antialiasing fails when the field covers under 2 screen pixels per range unit, and perspective needs a derivative-based range; MIT licence | sourced | github.com/Chlumsky/msdfgen README |
| VG-33 | Loop and Blinn (2005) fill quadratic and cubic Bézier regions exactly by an implicit inside test in the pixel shader; preparing the triangles (and resolving overlapping control hulls) is the costly step, so shapes that change often can be slower than plain tessellation | reported | Loop and Blinn, SIGGRAPH 2005; GPU Gems 3 ch. 25; webkit-dev thread 2010, search summary |
| VG-34 | Slug draws glyphs and vector shapes straight from Bézier curves with no atlas, robust (no dropped pixels or streaks) at any scale or perspective; a vertex shader expands each glyph's quad by half a pixel; the patent (US 10,373,352) was dedicated to the public domain on 2026-03-17, with MIT reference shaders at github.com/EricLengyel/Slug | sourced | Lengyel, "A Decade of Slug" |
| VG-35 | Vello, a GPU compute 2D renderer, reports 177 fps on the paris-30k scene at 1600×1600 on an M1 Max, which it calls a best case | reported | Vello README, search summary |
| VG-36 | Map labels: glyphs are signed distance fields fetched in ranges of 256 code points on demand; a tile carries more labels than fit, and each frame a collision grid keeps a subset, synchronised across tiles so labels are not clipped at tile edges | reported | Mapbox style spec "glyphs"; mapbox-gl-native wiki "Collision Detection", search summaries |
| VG-37 | Mapbox Vector Tile 2.1: protobuf; a tile has named layers of features; coordinates are integers in a tile-local extent (4096 in the examples), y down; geometry is a stream of MoveTo, LineTo and ClosePath commands with zigzag-encoded deltas from a cursor; attribute keys and values are stored once per layer and features hold index pairs; geometry may spill past the extent as a buffer; styling is not in the tile | sourced | github.com/mapbox/vector-tile-spec 2.1 |
| VG-38 | Google Maps 5 for Android (December 2010) replaced downloaded 256 px image tiles with vector geometry drawn on the phone; Google estimated about 70 % less data | reported | Google Maps blog title and date; TechCrunch 2010-12-16 |
| VG-39 | MapLibre: a vector source's maxzoom defaults to 22, and data from maxzoom tiles keeps being used when the map zooms past it (overzoom); raster tiles default to 512 px | sourced | maplibre.org style spec, Sources |
| VG-40 | Vendor view: vector tiles cut tile requests by about 60 % on average, restyle at run time (night mode, label language), and do not need 4 times the pixels for high-DPI; raster can be smaller for very plain styles, and vector needs client rendering, a concern on old devices | sourced | Stadia Maps, "Switching from raster to vector tiles" |
| VG-41 | tippecanoe defaults: at most 500 KB and 200,000 features per tile; detail 12 (4,096 units); maxzoom 14; Douglas-Peucker simplification to tile resolution; points thinned by 2.5 per zoom below the base zoom; options to drop or merge the densest features until a tile fits; polygons under about 4 square subpixels merged; a 5-pixel buffer (pixel = 1/256 of the tile) | sourced | github.com/felt/tippecanoe README |
| VG-42 | Protomaps' whole-planet OpenStreetMap basemap, zooms 0-15, is one file of about 120 GB; an extract to zoom 6 is about 60 MB | sourced | docs.protomaps.com basemaps downloads; getting started |
| VG-43 | Raster tiles for OSM: zoom 18 alone has 68.7 billion tiles; full pre-rendering to z18 was estimated at 54 TB to well over 100 TB, only 1.79 % of tiles were ever viewed, and the tile server used 1,250 GB in 2017 by rendering on demand | reported | OSM wiki "Tile Disk Usage"; help.openstreetmap.org 6062; Geofabrik, search summary |
| VG-44 | Generalisation by swapping sources per zoom: OpenMapTiles uses Natural Earth for boundaries up to z4 and water up to z5, OSM above; place labels carry a rank used to thin label density | sourced | openmaptiles.org/schema |
| VG-45 | Map styles grow line width with zoom by interpolating stops (for example 1 px at z8, 6 at z14, 12 at z18), linear or exponential | reported | MapLibre Flutter expressions docs; maplibre style spec, search summary |
| VG-46 | DEFCON's glowing vector map was chosen for mood, to feel like a general buried in NORAD, detached; a lone forgotten submarine still reads as a red glowing marker; Introversion was four directors at the time | sourced | The Escapist, "Everybody Dies in DEFCON" |
| VG-47 | Superhot's palette is a rule: enemies red, world white, usable things black; the team says the minimal style came from the prototype's short time and few resources and made the play instantly readable | reported | VIVE blog Q&A, quoted; Wikipedia "Superhot", search summary |
| VG-48 | Geometry Wars' shapes were drawn on graph paper and typed in by hand; the background grid replaced a star field because stars hurt readability in the action; a true-3D sequel prototype lost the feel and was dropped | reported | Game Developer, "The Color and the Shape: Bizarre Creations on GeoWars' Sensible Aesthetic", search summary |
| VG-49 | Rez's wireframe look came from Kandinsky and synesthesia; levels start as a flat wireframe plane and build up to dense structures by the last layer | reported | Wikipedia "Rez"; Giant Bomb; Time Extension making-of, search summary |
| VG-50 | Uplink's world map is both a tool (plan a route through sites) and a status display (the trace running back to you); the interface is plain 2D, styled on WarGames-era films | reported | GeekDad, ATPM and Hardcore Gaming 101 reviews, search summary |
| VG-51 | Mini Metro (a minimalism-themed jam game) draws its networks in the style of Harry Beck's transit diagram; Rymdkapsel, by a one-person studio, uses flat shapes and clean lines | reported | Wikipedia "Mini Metro"; Rymdkapsel reviews, search summary |
| VG-52 | Our planet at frequency 101 has 102,012 cells ([spec 30](../tier0/specs/30-world-grid.md)); a hex grid has about 3 edges per cell, about 306,000 edges. Borders, coasts and rivers on a few percent of them, stored as delta-coded polylines (about 4 bytes per vertex), fit in well under 1 MB, against 32 MB for one 4096×2048 RGBA image of the globe that blurs below a few km per pixel | derived | spec 30; VG-37 |
| VG-53 | Our replay is already "commands, not pixels": start state, seed and input log ([spec 06](../tier0/specs/06-save-replay.md) rule 2), so vector graphics add nothing to saves; the replay viewer's frames are what could become vector | derived | spec 06 |
| VG-54 | Our symbol-scale rule ([zoom continuum §3b](../design/zoom-continuum.md)) is what map styles do with zoom-interpolated widths and per-zoom thinning (VG-41, VG-45); and our nested grid can serve as the tile pyramid ([ZL-1](zoom-and-layer-two.md), [ZL-5](zoom-and-layer-two.md)) | estimate | VG-41, VG-45; zoom continuum |
| VG-55 | A Battlezone-style wireframe of the street conflicts with the Decided realism direction ([art direction](../design/art-direction.md)); a vector look fits only where the drawing is already symbolic (map heights, overlays, analysis views) | derived | art direction |

## 1. How vector displays and Battlezone worked

**The display.** A vector monitor has no grid of pixels. The beam is steered straight from point to point by the deflection signals and lit only where a line should be. Atari's generators are small processors that read a **display list**: "move here, draw to there, call this shape, return". The game program builds that list in RAM. The generator walks it, and the list is the whole picture (VG-3). The first generator (DVG, Asteroids) counted in digital steps on a 10-bit grid. The analog one (AVG, Battlezone and later) ramps the beam with an integrator, so lines have no steps (VG-8, VG-18).

**The budget is beam time.** At about 1 screen unit per microsecond (VG-5), a 60 Hz picture allows roughly 16 screen widths of beam travel per frame, blank moves included (VG-6). Draw more and the picture takes longer, so refresh falls and the image flickers. Long-persistence phosphor hid this and left trails (VG-7). Brightness also depended on beam speed, so the hardware evened out the speed (VG-5). Battlezone chose its own refresh, 41.7 Hz, with no vertical sync at all (VG-4). The Vectrex BIOS held 50 Hz (VG-20).

**Why it was cheap.** Memory. Two frame buffers would have cost more than the whole board was allowed to cost (VG-9). Battlezone's whole picture lived in 4 KB of vector RAM, double-buffered, next to 1 KB of working RAM. A 1-bit frame buffer of the same resolution would have been 96 KB (VG-2, VG-10). The saving was storage, not drawing work.

**How Battlezone did 3D.** Three processors share the work: the 6502 plays the game, the math box does the arithmetic and the AVG draws (VG-1). The tricks are what matter for us:
- **Shrink the problem.** Tanks move on a plane, so rotation is 2×2. Distance is a cheap 0.375·dx + dy approximation (VG-12).
- **Cull whole objects** by their centre and transform only visible ones. The price is that a near object pops out at once (VG-13).
- **No real hidden-line removal.** Far things are dimmer instead (VG-14). Depth by brightness is the vector form of fog.
- **The backdrop is a panorama strip**, a 90° window on a 360° cylinder, not geometry. It slides slightly out of step with the 3D objects (VG-15). Today we would call it an impostor.
- **The hardware clips.** A window circuit blanks lines outside a rectangle, which also keeps the radar clean (VG-8, VG-11).
- **Radar is a second view**: blips for threats only, flashing and fading (VG-16).
- **Fewer lines was a design request**, not only an engineering one (VG-17).

**The rest of the family.** Asteroids proved the DVG (VG-18). Tempest added colour on an XY monitor. Star Wars added colour 3D with a custom multiply-accumulate unit (VG-19). The Vectrex brought the same idea home with integrators driven by the CPU (VG-20). Vector monitors faded out as raster memory got cheap (VG-21).

## 2. The cost model then and now

**Then:** cost = total beam travel per frame (VG-6). Memory was nearly free because the picture was its own description (VG-3).

**Now:** a GPU has no line hardware worth relying on. Native wide lines are optional in Vulkan and absent on Apple through MoltenVK (VG-22). The core line modes in 1.4 are 1-pixel-class lines with only Bresenham required (VG-23). Every serious line renderer (maps, plotting libraries) turns lines into **thin quads**, two triangles per segment, and does antialiasing in the fragment shader from the distance to the centre line (VG-24, VG-25). So a "vector" layer is just flat-coloured triangles. Its cost is the usual:
- **Fragments.** About length × width per line, plus the round-cap and join overdraw (VG-25, VG-28). As with the beam, cost follows the total drawn length (VG-30).
- **Vertices.** 4 to 6 per segment; trivial at the counts a map needs (VG-29).
- **The quad tax.** Very thin geometry wastes shading on 2×2 quads (VG-26). For a 1-2 px line, the shader should be trivial (no lighting, no texture), and then the waste does not matter.
- **CPU rebuilding.** Regenerating line buffers is the usual bottleneck, not drawing them (VG-28).

**When vector style is cheaper on a GPU:**
- small fill area (thin lines, small symbols);
- no textures, so no texture memory or bandwidth;
- no lighting or shadows;
- few draws thanks to batching (one buffer and indirect draws, [DMO-7](draw-call-and-mesh-optimizations.md));
- data that is tiny next to textures or meshes (VG-38, VG-52).

**When it is not cheaper:**
- **Glow and bloom.** The Geometry Wars and DEFCON look needs a blur pass over the full screen, a fill-rate cost (VG-46, VG-48).
- **Dense wireframes** of real meshes: thousands of edges per object, plus extracting silhouette and crease edges.
- **Hidden lines.** True hidden-line removal needs a depth pass anyway. The cheap modern form is the single-pass wireframe: draw the filled triangles and darken pixels near edges, so hiding comes free from the depth test (VG-27). By then you are rasterising triangles.
- **Huge counts of tiny marks**, where overdraw takes over (VG-28).

**Text and curved shapes.** Four practical routes:
- **SDF textures** (Valve) and **MSDF** for sharp corners: cheap, one texture fetch per pixel, with limits at small sizes and under perspective (VG-31, VG-32). Map renderers use SDF glyphs (VG-36).
- **Loop-Blinn:** exact curves, but costly preparation (VG-33).
- **Slug:** exact glyphs from curves at any scale, now patent-free with MIT reference shaders (VG-34).
- **Compute renderers** such as Vello, for whole 2D scenes (VG-35).

For a game map, MSDF labels are the cheap start. Slug is the upgrade if text must stay sharp under a tilted camera.

## 3. What modern games and maps do

**Games keep vector style for readability, not speed.**
- Superhot's three-colour rule (VG-47) and Geometry Wars' grid replacing a star field (VG-48) were both chosen so play reads at a glance.
- DEFCON's map is a mood (detached command) and still lets one unit stand out by colour and glow (VG-46).
- Uplink makes the world map both a tool and a status display (VG-50).
- Mini Metro borrows the transit diagram (VG-51).
- Rez builds from a bare wireframe to dense scenes as the level goes on (VG-49). The style is a dial, not a fixed level.

None of them is a large-scale simulation that uses vectors to save GPU time. Their savings are in art production and clarity. (Highfleet's maps, which the user named, could not be checked.)

**Maps made the move we need.**
- Google moved Android Maps to vector in 2010 for less data (about 70 %, VG-38). The web followed with Mapbox GL and its open fork MapLibre.
- **The format.** A tile is integer geometry in a local extent, delta-coded commands and shared attribute tables (VG-37).
- **Style.** It is separate from the data and evaluated per zoom: widths and colours interpolate with zoom (VG-45), and layers have zoom gates (VG-39).
- **Generalisation** is done when tiles are built:
  - simplify lines to the tile's resolution (Douglas-Peucker);
  - thin points by a fixed ratio per zoom;
  - drop or merge the densest features until a tile fits a byte and feature cap;
  - merge tiny polygons;
  - swap to a coarser source at low zoom (Natural Earth below z5) (VG-41, VG-44).
- **Above the deepest level**, the client simply magnifies the last tiles (overzoom, VG-39).
- **Labels** are SDF glyphs placed by a collision grid every frame (VG-36).

**Size, raster against vector.** The whole OSM planet to z15 as vector is about 120 GB in one file (VG-42). Raster to z18 was estimated at 54 to over 100 TB, and only 1.8 % of those tiles were ever looked at (VG-43). Vendors list the trade-off plainly:
- **Vector:** fewer requests, restyling, any DPI (VG-40).
- **Raster:** the simplest client, sometimes smaller for very plain styles (VG-40).

**Link to our zoom continuum.** Our rule "draw at max(real size, minimum screen size), thin by height, cross-fade" ([zoom continuum §3b](../design/zoom-continuum.md)) is what a vector map style does (VG-54). Refining by screen error is what 3D Tiles does ([ZL-6](zoom-and-layer-two.md)). Our height bands come from the 6 px rule ([ZL-45](zoom-and-layer-two.md)). Vector maps are the tested way to draw the upper bands of that table.

## 4. What this means for us (Proposal, all estimate)

### 4.1 Where vector rendering fits on the zoom

| Band ([ZL-45](zoom-and-layer-two.md)) | Proposal | Why |
| --- | --- | --- |
| Globe, Region (over 40 km wide) | **All vector over a cheap ground fill.** Coasts, borders, rivers, major roads and rails as lines; cities as symbols sized by population; labels. Ground: cell colours (biome, elevation, ownership) as flat filled hex polygons or one low-resolution texture of cell values | The whole planet's lines fit under 1 MB (VG-52); the style changes with no rebuild (VG-40); symbol scale is native (VG-45, VG-54) |
| City (5-40 km) | **Vector map, tilted.** Roads at minimum width, zone blocks as flat polygons, flow lines for traffic and supply, pops as a sample of dots, overlays | Matches [zoom continuum §3b](../design/zoom-continuum.md) row "City"; dots and lines cost about length × width (VG-29) |
| District (1-5 km) | **Hybrid.** Building footprints extruded at draw time as flat-shaded prisms (no textures), roads still vector, markers as instanced SDF symbols | Footprints are already vector data; extrusion is cheap geometry (VG-27 if outlines are wanted) |
| Tactical, Street (under 1 km) | **Raster and realism.** Meshes, materials, lighting; vector only for overlays (paths, selection outlines, noise rings, perception) and UI | Decided realism ([art direction](../design/art-direction.md), VG-55) |

The bands cross-fade as the continuum already says. Vector lines fade in as meshes fade out, so no new rule is needed.

### 4.2 The other uses, judged

| Use | Cost | Saves | Conflicts or limits |
| --- | --- | --- | --- |
| **Map and strategy layers** (borders, rivers, roads, fronts, supply lines) | Low: one line and polygon pipeline, a style table | Data size, art production, restyling per layer (ownership, front, infrastructure) | None found; [cameras and modes](../design/cameras-modes.md) gives layers 2 and 3 one map view each ([0060](../decisions/0060-one-fight-three-views-behaviour-from-jobs-and-skills.md)) |
| **Overlays** (noise, light, supply, power) | Fields ([spec 18](../tier0/specs/18-noise-field.md), [spec 19](../tier0/specs/19-light-field.md)) are per-cell grids, so a heat texture is the natural form; vector adds **contour lines and flow arrows** on top (natural contours: the notice and place thresholds of [HST-139](hordes-sound-sky-and-testing.md); flow arrows for horde flow, [HST-143](hordes-sound-sky-and-testing.md)) | Readable thresholds ("audible from here") instead of a colour wash; fixes the "overlay drew nothing" case ([VR-34](view-readability.md)) with an explicit outline | Contouring costs CPU per update; only for the shown field |
| **Radar or sonar perception view** | Low: blips and fading tracks drawn from what the pop knows (the per-pop track list of [XD-128](borrowed-from-other-fields.md) with radius and confidence computed when read, XD-129; shared [XD-69](borrowed-from-other-fields.md)-style occupancy grids for groups), Battlezone-style (VG-16) | A cheap, honest way to show belief; fits "zoom does not grant vision" ([zoom continuum §5](../design/zoom-continuum.md)) | Must draw belief, never truth; a design choice for the user |
| **Debug and replay viewer** | Low: the HTML replay viewer could draw vector frames (SVG or canvas) from the trace | Small pages, crisp zoom | The trace is already data; this only changes drawing (VG-53) |
| **Saves and network as commands** | None | Already true: replay is seed plus input log (VG-53) | Nothing to add |
| **Procedural buildings as footprints extruded at draw time** | Low | No mesh storage above District; footprints come from the generator anyway | Below District, real meshes ([0017](../decisions/0017-slow-endgame-and-generation-on-the-go.md) generate-on-approach) |
| **Pops as dots** | Already planned ([zoom continuum §3b](../design/zoom-continuum.md)); instanced SDF discs | Budget capped by the screen ([LC-10](low-resource-city-simulation.md)) | Shapes per kind from [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md) (discs, diamonds, labelled squares) |
| **Low-end "tactical display" fallback** | Medium: every street-level state would need a symbolic drawing | A mode for weak GPUs and for big fights | Risks a second art style to maintain; better as the existing upper bands reached by zooming out, not a separate mode ([0052](../decisions/0052-one-executable-one-simulation.md) one executable) |
| **High-contrast accessibility** | Low once the style table exists: one more style | Contrast without new assets | Colour never alone ([VR-24](view-readability.md)); shapes carry kind |
| **Oscilloscope audio visualisation** | Low | Little | No game need found; leave it out |

### 4.3 A minimal pipeline for the map and strategy views

1. **Source.** The simulation stays untouched. A view-side builder reads committed state: cells, rivers ([spec 52](../tier0/specs/52-land-water-capacity.md)), countries, settlements, later roads. It writes vector tiles keyed by our nested grid (a tile is a cell at a level), not by Mercator quadkeys ([ZL-5](zoom-and-layer-two.md)). It rebuilds a tile only when an event changes it (a border moves).
2. **Format.** MVT-like and internal (VG-37). Per tile: layers (water, border, river, road, place, zone, overlay); integer coordinates in a 4,096-unit tile-local extent, so precision holds at planet scale with a floating origin ([zoom continuum §4](../design/zoom-continuum.md)); delta-coded commands; shared attribute tables; a small buffer past the edge.
3. **Generalisation per level** (VG-41, VG-44):
   - simplify to one tile pixel;
   - gate classes by level (minor roads only at City and finer);
   - thin points by a keyed draw so the same ones stay;
   - merge tiny polygons;
   - cap features per tile.

   The children-add-up rule ([zoom continuum §3](../design/zoom-continuum.md) T2) applies: a river crosses a tile edge where the coarse level says.
4. **Style.** One data file (under `data/`) gives width, colour, dash and z-order per (layer, class) as stops interpolated over camera height, plus the minimum-size rule (VG-45, VG-54). A palette swap gives high contrast or night mode.
5. **GPU.** Four pipelines, all in one buffer with indirect draws ([DMO-7](draw-call-and-mesh-optimizations.md)):
   - **Lines:** instanced quads, width in pixels, antialiasing from the centre-line distance; never native wide lines (VG-22, VG-25).
   - **Polygons:** triangulated once per tile, flat colour; extruded on the GPU at District.
   - **Symbols:** instanced SDF icons and dots.
   - **Text:** an MSDF atlas first, Slug only if tilted text looks poor (VG-32, VG-34).
6. **Labels.** A screen collision grid each frame, priority by rank, fade in and out, and one index across tiles (VG-36).
7. **Budgets** (starting values, tune by looking):
   - at most about 2 M line fragments and 100,000 segments per frame (VG-29);
   - at most 50,000 features per tile;
   - pop dots capped by the screen ([LC-10](low-resource-city-simulation.md)).
8. **First step that fits today's code.** The current 2D window already draws antialiased shapes through ImGui draw lists, which are triangles ([RN-40](renderer.md)). The world map (`zs gen world --bmp`) is raster. A vector export of the same map (rivers and borders as polylines) would test steps 1-4 without Vulkan. This step is for the main session to schedule; nothing here edits `game/`.

### 4.4 What stays raster

- Street and interior views, bodies, clothing, materials and lighting ([renderer](renderer.md)).
- Terrain shading at Region and finer, whether a hillshade texture from cell elevations or a lit mesh.
- Field heat maps, which are grids by nature.
- Night lights at Region height (an impostor, T1).
- Post effects.

### 4.5 Risks

- **A sea of lines.** Like the "sea of icons" ([ZL-20](zoom-and-layer-two.md), [ZL-48](zoom-and-layer-two.md)), unranked lines become noise. Every class needs a level gate and a z-order, and the strong colours go to a few things (VG-46, VG-47).
- **Generalisation popping.** A line simplified differently at two levels jumps when the tile changes. Cross-fade over a height range, as the continuum already requires.
- **Label churn** when the camera moves. Use collision with fades and hysteresis (VG-36).
- **Portability.** Never rely on native wide lines or line smoothing (VG-22, VG-23).
- **Glow costs fill.** A DEFCON-like glow is a full-screen pass; keep it optional.
- **Two drawings of one world.** Map style and street art must agree on colour meaning (ownership, danger), or players relearn at each band.
- **Showing truth by accident.** The map must draw what the viewer's side believes, not the simulation's truth ([zoom continuum §5](../design/zoom-continuum.md)).
- **Text shaping** for non-Latin names (real Earth later) needs a shaping library; it is not covered here.
- **Art direction.** A vector look at street level would break the Decided realism (VG-55).

## 5. Open questions

1. Should the map's vector look be stylised (DEFCON-like glow, dark background) or cartographic (paper-map colours)? It sets the mood of layers 2 and 3. Recommendation: cartographic by default, with a "command display" palette as a style option.
2. Tiles keyed by our nested grid or by a standard quadtree? The grid nests exactly ([spec 30](../tier0/specs/30-world-grid.md)); a quadtree has tools such as tippecanoe and MapLibre. Recommendation: our grid, with an MVT-like internal format.
3. Does the City band draw roads and blocks only as vector, or as flat 3D extrusions under the tilt the continuum recommends ([zoom continuum §6](../design/zoom-continuum.md) question 4)?
4. Is a radar or sonar perception view wanted as a player tool (belief drawn as blips), or only as a debug lens?
5. Is the vector world map export (4.3 step 8) worth doing before the Vulkan renderer, as a check on generators ([spec 52](../tier0/specs/52-land-water-capacity.md) rivers and borders)?
6. MSDF or Slug for map text? Decide once a tilted City view exists.
7. Not confirmed here: Battlezone's line count per frame, the Vectrex timer count, how Highfleet draws its maps, Google's 2010 post text, and the share of PC GPUs with wide lines (irrelevant if we never use them).

## Sources

**Vector hardware:**
- Jed Margolin, "The Secret Life of Vector Generators": https://www.jmargolin.com/vgens/vgens.htm
- 6502disassembly.com, Battlezone: https://6502disassembly.com/va-battlezone/ ; math box: https://6502disassembly.com/va-battlezone/mathbox.html ; Asteroids: https://6502disassembly.com/va-asteroids/
- IEEE Spectrum, "The Invention of Battlezone": https://spectrum.ieee.org/battlezone
- Hochschule Pforzheim Vectrex lab, `Wait_Recal`: https://itwww.hs-pforzheim.de/daten/mitarbeiter/johannsen/vectrex_lab/projects/wait_recal/wait_recal.htm
- Wikipedia, "Vectrex": https://en.wikipedia.org/wiki/Vectrex ; "Vector monitor": https://en.wikipedia.org/wiki/Vector_monitor
- Arcade monitor guide: https://pc2jamma.mameworld.info/monitor3.html

**GPU lines, edges, text:**
- Vulkan VK_KHR_line_rasterization refpage: https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_line_rasterization.html
- MoltenVK wide lines thread: https://community.khronos.org/t/macos-vulkan-at-runtime-vkcreatedevice-fails-when-vkphysicaldevicefeatures-widelines-vk-ture-and-doestnt-support-vkcmdsetlinewidth-api-as-well/106443 ; ANGLE commit: https://chromium.googlesource.com/angle/angle/+/6ae0c3214c4152d8154366315d63ba5e4310d4de%5E%21/
- K. Käfer, "Drawing Antialiased Lines with OpenGL" (mirror): https://github-wiki-see.page/m/PaintLab/PixelFarm/wiki/Drawing-LineAA-with-OpenGL
- R. Terrell, "Instanced Line Rendering": https://wwwtyro.net/2019/11/18/instanced-lines.html
- Bærentzen et al., single-pass wireframe: https://people.compute.dtu.dk/janba/Wireframe ; NVIDIA SolidWireframe: https://developer.download.nvidia.com/whitepapers/2007/SDK10/SolidWireframe.pdf
- Helper pixels patent: https://patents.google.com/patent/US9779542 ; Fatahalian et al. 2010: https://www.microsoft.com/en-us/research/?p=159994
- deck.gl performance: https://deck.gl/docs/developer-guide/performance
- C. Green, Valve, SIGGRAPH 2007: https://cdn.akamai.steamstatic.com/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
- msdfgen: https://github.com/Chlumsky/msdfgen
- Loop and Blinn, GPU Gems 3 ch. 25: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch25.html ; webkit-dev: https://lists.webkit.org/pipermail/webkit-dev/2010-August/014099.html
- E. Lengyel, "A Decade of Slug": https://terathon.com/blog/decade-slug.html
- Vello: https://docs.rs/crate/vello/latest

**Maps:**
- Mapbox Vector Tile spec 2.1: https://github.com/mapbox/vector-tile-spec/blob/master/2.1/README.md
- MapLibre style spec, sources: https://maplibre.org/maplibre-style-spec/sources/ ; expressions (Flutter docs): https://maplibre.org/flutter-maplibre-gl/advanced/expressions/
- Mapbox glyphs: https://docs.mapbox.com/style-spec/reference/glyphs/ ; collision detection: https://github.com/mapbox/mapbox-gl-native/wiki/Collision-Detection
- Google Maps 5 for Android: https://maps.googleblog.com/2010/12/under-hood-of-google-maps-50-for.html ; https://techcrunch.com/2010/12/16/google-maps-5-with-3d-buildings-now-available-for-android
- Stadia Maps guide: https://docs.stadiamaps.com/guides/switching-your-maps-from-raster-to-vector-tiles/
- tippecanoe: https://github.com/felt/tippecanoe
- Protomaps downloads: https://docs.protomaps.com/basemaps/downloads
- OSM Tile Disk Usage: https://wiki.openstreetmap.org/wiki/Tile_Disk_Usage ; https://help.openstreetmap.org/questions/6062/how-large-in-disk-size-is-a-current-complete-tile-set/
- OpenMapTiles schema: https://openmaptiles.org/schema/

**Games:**
- The Escapist, DEFCON: https://www.escapistmagazine.com/everybody-dies-in-defcon/
- Superhot: https://blog.vive.com/us/superhot-vr/ ; https://en.wikipedia.org/wiki/Superhot
- Geometry Wars: https://www.gamedeveloper.com/game-platforms/the-color-and-the-shape-bizarre-creations-on-i-geowars-i-sensible-aesthetic
- Rez: https://en.wikipedia.org/wiki/Rez_(video_game) ; https://timeextension.com/features/the-making-of-rez-tetsuya-mizuguchis-timeless-masterpiece
- Uplink: https://geekdad.com/2011/06/geekdad-retro-gaming-uplink/ ; https://www.hardcoregaming101.net/uplink-trust-is-a-weakness-uplink-hacker-elite-windows-linux-macintosh-ios-android-2001/
- Mini Metro: https://en.wikipedia.org/wiki/Mini_Metro_(video_game) ; Rymdkapsel: https://grapefrukt.itch.io/rymdkapsel
