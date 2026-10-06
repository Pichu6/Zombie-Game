# Global outbreak simulation research

2026-10-05. Pointer and summary. The full research, six proposer/critic rounds and a fortification round, lives outside the repo at `/mnt/project-files/research/global-sim/` (start with `README.md`; the current integrated design is `loop/19-design-v5.md` plus `loop/20`–`21` for fortifications). What the user decided is in [0012](../decisions/0012-world-simulation-layers.md); the resulting design is [design/world-simulation.md](../design/world-simulation.md).

## Findings

- **The planet is cheap, people are expensive.** A daily region-level epidemic model of the whole Earth costs almost nothing (GLEaM-style); individual people are what slow games down (Stellaris pop lag). Hence three levels of people: lazy census (regenerated from a seed), records (anyone the simulation has touched), embodied bodies near the player.
- **Scale-switching works only as views over one world with one rulebook** (Spore and similar games broke when each mode had its own rules). Hence the simulation contract and "one fact, every layer".
- **Calibration:** a slow, visible zombie fizzles once people know (reproduction number about 0.3 when people fight back). A hidden bite with 1–2 days to turn goes regional often and global sometimes, so it is the proposed realistic default. Fizzle odds use the branching-process formula with superspreading (Lloyd-Smith 2005); zombie epidemic models from Munz 2009 and Alemi 2015.
- **Human reaction:** panic is rare and flight is the dangerous behaviour; response lags come from real cases (Italy's red zone, Wuhan, Ebola cordons). Compliance with edicts uses thresholds with hysteresis (Granovetter, Epstein).
- **Economy:** production limited by the scarcest input with minimum crews; shops run out instead of raising prices; build orders become job postings and material requests (Cities: Skylines demand made of real jobs; Star Citizen economy as reference).
- **Communication:** one signal-to-noise rule for voice, walls, radio and zombie hearing (ISO 9613-2, ANSI S3.5); radio coverage from standard propagation models; when the grid dies, voices carry about 10–15× farther and engines about 30×.
- **Fortifications:** assemblies that break at their weakest part, zombie push from each body summed, breach time falling smoothly with horde size; tests from EN 1627, EN 356 and ASTM F3038.
- **Data licences:** use public-domain or CC BY data (Natural Earth, GeoNames, WorldPop, ILO after May 2023); MakeHuman/MPFB or Anny for bodies, not SMPL; OpenStreetMap only for the home region.
- **Community play styles** (Project Zomboid and others) map to modular scenarios that only set numbers, starting state, scheduled events and objectives.

## Sandbox, virus editor, world generation and layer flow (2026-10-06)

Files `loop/22a` (Plague Inc and a virus editor), `22b` (world generation, settlements, culture kits, data licences), `22c` (Stellaris, Hearts of Iron, OGame, Manor Lords, Cities: Skylines, Infection Free Zone, possession) and `23` (critique).

- Plague Inc adds trait points into three bars; no published formula exists. We keep sliders on real rates, with trait cards for the feel and a preview from many headless runs.
- Generate a plausible present (suitability function, four eras, checked against rank-size and gravity statistics) rather than simulating history; emergence during play is camps, safe zones, trails and barter.
- Enterable procedural buildings: interior first, facade second. Shadows of Doubt uses hand-made shells with procedural rooms, so it is evidence for generated interiors, not whole generated cities.
- Copy: Hearts of Iron map-targeted decisions, Stellaris build tooltips and jobs from buildings, Manor Lords self-staffing, Cities: Skylines info views, RimWorld/Kenshi job queues as the objective source, The Sims' now/today/life wants. Avoid: OGame queue wipes, Frostpunk manual-only staffing, per-pop priority grids.
- Licences: CHELSA (CC0), Beck Köppen (CC BY), SoilGrids (CC BY) usable; WorldClim, HWSD and IUCN range maps are non-commercial; RESOLVE ecoregions are CC BY but have no species lists; Google Open Buildings covers only parts of the world.

## Endgame, generation, activities and scale (2026-10-06)

`loop/24` (slow endgame, Haven & Hearth and Space Engineers, real infrastructure decay, generation on the go), `loop/25` (activities and mini-games, parity rule, eight interaction types), `loop/26` (scale across layers: real-size objects, smaller worlds by having less of them, time as the real constraint; XCOM 2 geoscape lessons).

## Sandbox engine vision (2026-10-06)

`loop/27`: precedents for one world played in several modes (Dwarf Fortress, Kenshi, Songs of Syx worked; Spore, early No Man's Sky, Star Citizen's scope are the warnings), war as fronts of real census people that zoom into battles, generations at macro speed only, and the growth order now in [vision](../vision.md).

## Country creation and inheritance (2026-10-06)

`loop/28`: Stellaris species and empire creation, Victoria 3, Crusader Kings 3, RimWorld and Dwarf Fortress models; real data on how religion, party, language and mixed identity pass between generations; genes pass only the body.
