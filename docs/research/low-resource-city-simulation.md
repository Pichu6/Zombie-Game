# Low-resource city simulation: "ASCII City" and the games that fake or tame big populations

**Date:** 2026-10-07. **Status:** research note, not a decision. **Feeds:** [zoom continuum](../design/zoom-continuum.md), [focus and compression](../design/focus-and-compression.md), [spec 27](../tier0/specs/27-detail-levels.md), [spec 28](../tier0/specs/28-pop-levels.md). **Evidence:** web-search summaries and one fetched page (the CyberCity GitHub README, read directly). Most news and blog pages (TechSpot, Gizmodo, PC Gamer, Stack Overflow blog, Gamasutra, yewknee, runtimewire) were **blocked by the proxy**, so those are the search engine's summaries of them. Numbers from forums and wikis are player-reported. **IDs:** LC.

The user asked how "ASCII City" managed a city on low resources, in the context of the [zoom continuum](../design/zoom-continuum.md) (a city of millions seen from above, one pop's home up close).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| LC-1 | "ASCII City" (Grow Now! Games) is a walkable 3D city drawn in text characters in one HTML file: a rendering trick, not a city simulation. | reported | https://www.notebookcheck.net/Solo-dev-builds-ASCII-Cyberpunk-city-in-one-HTML-file.1379160.0.html |
| LC-2 | CyberCity's raycaster marches one screen column at a time through a height-map grid with a running silhouette, so occlusion and sky come free; cost is columns x march steps, and a bigger city adds nothing. | sourced | https://github.com/MiguelCarino/CyberCity |
| LC-3 | CyberCity's world is a pure function of a seed (streets, districts, blocks, plazas), byte-identical in the browser and offline; no city is stored. | sourced | https://github.com/MiguelCarino/CyberCity |
| LC-4 | CyberCity's people are decoration: about 70 modules with init, update and draw; 23 crowd silhouettes share one base shape; people stand outside lit shops and shelter in rain, but nobody has a job, home or body. | sourced | https://github.com/MiguelCarino/CyberCity |
| LC-5 | In Dwarf Fortress, units taking turns are over 60% of a large fort's time. | reported | https://dwarffortresswiki.org/index.php/23a:Maximizing_framerate |
| LC-6 | In a Dwarf Fortress profile, pathfinding is about 6% of a tick while line-of-sight and proximity checks between units are the biggest single piece, about 20%; the standard advice is fewer active units. | reported | https://dwarffortresswiki.org/index.php/DF2014:Path |
| LC-7 | Songs of Syx is reported as one developer in Java simulating 10 to 30 thousand citizens, each with their own life, religion, fears and desires; its techniques were not found. | reported | https://store.steampowered.com/app/1162750 |
| LC-8 | Cities: Skylines 2 has agents with no hard cap and multi-threaded pathfinding weighing time, comfort, money and behaviour; population growth is still the performance wall because path requests grow with it. | reported | https://paradoxinteractive.com/games/cities-skylines-ii/features/traffic-ai |
| LC-9 | Simulation LOD research cuts out-of-view agent cost by lowering update rate, reducing or skipping path planning, jumping to the destination and, far away, turning off collision avoidance. | reported | https://www.springerprofessional.de/en/simulation-level-of-detail-for-virtual-humans/2882154 |
| LC-10 | The renderer's drawing budget (dots, building meshes) should be capped by the screen, never by the city's size. | estimate | this note (from LC-2) |
| LC-11 | If a lazy pop's position at time t is a pure function of (pop, time) from its schedule, home and job, sampled dots in a city of millions can be drawn from records without simulating them and still match when you drop in. | estimate | this note |
| LC-12 | Cheap believable crowd rules are acceptable as T1 impostors only if they read the real state and never write it. | estimate | this note |
| LC-13 | No per-tick rule may scan all entities; it uses the chunk's neighbours (the Dwarf Fortress all-pairs lesson). | estimate | this note (from LC-6) |
| LC-14 | Simulation LOD technique names (update rate, skip pathing, jump to destination, drop far collision) match spec 27's jump forms and should be our vocabulary. | estimate | this note (from LC-9) |

## The main finding: ASCII City is a rendering trick, not a city simulation

"ASCII City" (and "ASCII CITY Prototype", by the solo developer Grow Now! Games; a similar open-source one, CyberCity, and a Rust/WebAssembly one by Rhys Gronow are reported too) is a **walkable 3D city drawn in text characters**, in one HTML file. It is cheap for three reasons, none of which is a population model:

1. **Cost follows the screen, not the city.** A raycaster marches one screen *column* at a time through a **height-map grid**, keeping a running silhouette, so occlusion and sky come "for free and at character resolution" (CyberCity README). The work is columns × march steps; a bigger city adds nothing. The output is a few thousand characters, not millions of pixels.
2. **The world is a pure function of a seed.** Streets, districts, blocks and plazas come from a seed; the same seed gives byte-identical output in the browser and offline (README). No stored city.
3. **The people are decoration with cheap rules.** About 70 small modules, each with `init`, `update`, `draw`, cover rain, traffic, signs and pedestrians; 23 crowd silhouettes share one base shape with different trim; people "stand outside lit shops rather than spreading evenly" and shelter when it rains (README). Believable, but nobody has a job, a home or a body. (Reported by news: ride elevators, interiors with several storeys, speeding cars.)

So the answer to "how did it simulate a city on low resources" is: **it did not simulate one; it drew one**, with a cost bounded by what the camera sees.

## The real low-resource simulations

| Game | What keeps it cheap or large | Source quality |
| --- | --- | --- |
| **Dwarf Fortress** | ASCII (or tiles) is only the display. The cost is the **units taking turns** (over 60% of a large fort's time, per the wiki's framerate page); pathfinding is a small part (about 6% of a tick, per a profile cited on the forum) while **line-of-sight and proximity checks between units** are the biggest single piece (about 20%), including creatures far away checking whether your dwarves can see them. The standard advice is to have fewer active units. World history is generated first, then played in one place | wiki and forum, player-reported |
| **Songs of Syx** | Reported as one developer, Java, 10–30 thousand citizens each simulated with their own life, religion, fears and desires, and well optimised. The techniques are not in what I could read | store and news text |
| **Cities: Skylines 2** | Agents (citizens, services, goods) with no hard cap, **multi-threaded** pathfinding that weighs time, comfort, money and behaviour; the first game used straight-line proximity instead of the road network. Population growth is still the performance wall because pathfinding requests grow with it | developer diary summaries |
| **Simulation LOD (research)** | The named technique: cut the cost of agents out of view by lowering update rate, reducing or skipping path planning, then jumping straight to the destination; far from the viewer, even **turning off collision avoidance** hides nothing and saves a lot | academic papers |
| **Cataclysm: DDA** | Abstract overmap first; detailed map generated only on discovery | docs |

## What this means for us (Proposal, nothing decided)

1. **The "cost follows the screen" idea is what [spec 27](../tier0/specs/27-detail-levels.md) and the zoom note already say**, stated for rendering: the drawing budget (how many dots, how many building meshes) is a cap set by the screen, never by the city's size. Worth stating as a rule of the renderer, separate from the simulation.
2. **Where is this pop at time t?** ASCII City's pedestrians are decoration, so they can be anywhere. Ours must be real pops. If a lazy pop's **position at a given time is a pure function** of (pop, time) from its schedule, home and job, then the sampled dots in the million-pop city can be *drawn from records without simulating them*, and they will line up with the truth when you drop in. [Spec 28](../tier0/specs/28-pop-levels.md)'s random-access lives are the base; the position function is a new, small unit to specify.
3. **Cheap believable rules are fine as T1 impostors** (people gathering near lit shops, sheltering in rain) as long as they read the real state (the shop is open, it is raining) and never write it.
4. **Dwarf Fortress's lesson is the all-pairs check.** Per-unit visibility and proximity tests against everything are the cost, not walking. Our hot set, chunking and spatial wake rules (spec 27 rule 3) already avoid it; the rule to add is that **no per-tick rule may scan all entities**; it uses the chunk's neighbours.
5. **Simulation LOD technique names** (update rate, skip pathing, jump to destination, drop collision far away) match spec 27's jump forms; keep them as the vocabulary.
6. **Seeded, deterministic, byte-identical output** is already our rule (golden hashes); ASCII City is a small proof it works for a whole generated city.

## Open

- I took "ASCII City" to mean the walkable text-mode city that went around in 2026. If the user meant a different text-mode city *simulation* game, name it and the note gets a section.
- Songs of Syx's actual data layout and Dwarf Fortress's own optimisation write-ups were not readable here. A later pass could read them if the proxy allows, or the user can paste the key parts.

## Sources

- [CyberCity (GitHub)](https://github.com/MiguelCarino/CyberCity), read directly. News on the Grow Now! Games prototype: [Notebookcheck](https://www.notebookcheck.net/Solo-dev-builds-ASCII-Cyberpunk-city-in-one-HTML-file.1379160.0.html), [PC Gamer](https://www.pcgamer.com/hardware/lone-game-dev-builds-an-explorable-cyberpunk-city-almost-entirely-out-of-ascii-characters/), [Gizmodo](https://gizmodo.com/this-3d-ascii-cyberpunk-city-should-absolutely-be-turned-into-a-full-game-2000799927), [TechSpot](https://techspot.com/news/113574-fully-walkable-3d-city-built-entirely-out-ascii.html); [Rust engine write-up](https://runtimewire.com/article/rhys-gronow-ascii-city-rust-webassembly-engine). Search summaries only.
- Dwarf Fortress wiki: [maximizing framerate](https://dwarffortresswiki.org/index.php/23a:Maximizing_framerate), [path](https://dwarffortresswiki.org/index.php/DF2014:Path); Stack Overflow blog: [how Dwarf Fortress is built](https://stackoverflow.blog/2021/12/31/700000-lines-of-code-20-years-and-one-developer-how-dwarf-fortress-is-built/).
- [Songs of Syx](https://store.steampowered.com/app/1162750) and [GamingOnLinux coverage](https://gamingonlinux.com/2019/07/songs-of-syx-a-city-builder-with-empire-management-tactical-battles-and-rpg-elements).
- Paradox, [Cities: Skylines II traffic AI](https://paradoxinteractive.com/games/cities-skylines-ii/features/traffic-ai); [80.lv on agent limits](https://80.lv/articles/cities-skylines-2-doesn-t-have-limit-for-people-it-can-track).
- Simulation LOD: ["Simulation Level of Detail for Virtual Humans"](https://www.springerprofessional.de/en/simulation-level-of-detail-for-virtual-humans/2882154), and the [SIGGRAPH history entry on relaxing collision avoidance in virtual crowds](https://history.siggraph.org/?p=197603).
- [Cataclysm: DDA overmap](https://docs.cataclysmdda.org/OVERMAP.html).
