# Zoom from planet to street, and what the City layer plays like

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [zoom continuum](../design/zoom-continuum.md), [cameras and modes](../design/cameras-modes.md), [0043](../decisions/0043-city-layer-and-repercussion-loop.md), [0058](../decisions/0058-offices-open-layers-single-player-is-multiplayer-with-ai.md). **Evidence:** read in full: the Bing Maps Tile System article (Microsoft Learn), the 3D Tiles specification README (CesiumGS GitHub), the d3 `interpolateZoom` docs, the They Are Billions official page, the RimWorld wiki "Time" page, a Game Developer article on Cities: Skylines traffic (Tommy Thompson, 2020) and a 2011 essay on strategic zoom (Matchsticks for my Eyes). Everything else is from web-search summaries of reviews, wikis, forums and mod pages (`reported`): the Paradox forum, PC Gamer, TechRadar and WinCustomize pages were blocked or truncated. No GDC talk on strategic zoom was found. **IDs:** ZL.

On 2026-10-09 the user asked how Google Maps and Google Earth handle zooming on a planet, where the "jumps" between layers should go, and whether layer 2 should play as RTS combat or as city and base management. This note covers the zoom technique briefly. Most of it is about how games put play along a zoom. It ends with a proposal for height bands, jumps, time caps and readability.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| ZL-1 | Bing Maps tile pyramid: level 1 is 512x512 px and each level doubles width and height; tiles are 256x256 px; levels run 1 to 23; ground resolution at the equator is 156,543 m/px divided by 2^level (78 km/px at level 1, 0.6 m/px at level 18, 0.019 m/px at level 23), times cos(latitude) | sourced | learn.microsoft.com/en-us/bingmaps/articles/bing-maps-tile-system |
| ZL-2 | A quadkey's length equals its level, and a tile's key starts with its parent's key; it is a one-dimensional index that mostly keeps neighbouring tiles close, so neighbours load together | sourced | same as ZL-1 |
| ZL-3 | Google Maps uses the same scheme: 256x256 px Mercator tiles, zoom 1 is 4 tiles, each level doubles each axis | sourced | developers.google.com/maps/documentation/javascript/coordinates |
| ZL-4 | On a 1920 px wide screen the visible ground width is 1920 x m/px: level 22 is 72 m, level 19 is 570 m, level 16 is 4.6 km, level 13 is 37 km, level 10 is 290 km, level 6 is 4,700 km, level 3 is the whole planet | derived | ZL-1 |
| ZL-5 | From the whole planet to a 0.5 m plot is about 22 to 23 doublings, the same span as the map pyramid; our nested grid (70 km cell, about 9 doublings to a 200 m zone cell, about 8 more to 0.5 m) covers the lower 17 of them | derived | ZL-1; [zoom continuum §2](../design/zoom-continuum.md) |
| ZL-6 | 3D Tiles: each tile has a geometric error in metres ("the error introduced if this tile is rendered and its children are not"); the client turns it into a screen-space error in pixels and refines when it is too high; REPLACE draws children instead of the parent, ADD draws both; child content lies inside the parent's bounds; quadtrees and octrees can be implicit (addressed by tile coordinates) | sourced | github.com/CesiumGS/3d-tiles specification/README.adoc |
| ZL-7 | CesiumJS defaults: maximum screen-space error 16 px for 3D tilesets and 2 px for globe terrain | reported | cesium.com CesiumJS API docs, Cesium3DTileset and Globe (1.50 to 1.84), search summary |
| ZL-8 | Cesium hides cracks between terrain tiles of different levels with "skirts", extra geometry hanging down from every tile edge, always present; it replaced an earlier screen-space blur that could only fill cracks a few pixels wide | reported | community.cesium.com threads on skirts (Cesium developers' answers), search summary |
| ZL-9 | Microsoft Flight Simulator 2024: pop-in is the visible hand-off from simplified distant scenery to the detailed version streamed near the camera; late cloud data, CPU and VRAM limits make it worse; Terrain LOD and Objects LOD sliders move the hand-off distance; players report visible switching between the Bing imagery layer and photogrammetry | reported | flyawaysimulation.com guides; forums.flightsimulator.com thread 738065 |
| ZL-10 | Google Earth Pro tilts the camera automatically while zooming in (an option turns it off); R resets to straight down; the fly-to speed is a setting | reported | rpls.com and anandtech forum posts, search summary |
| ZL-11 | Van Wijk and Nuij (2003) give an optimal smooth zoom-and-pan path: zoom out, travel, zoom in, as one curve; d3's `interpolateZoom` implements it with curvature rho = sqrt(2) by default and a recommended duration from the path length | sourced | d3js.org/d3-interpolate/zoom |
| ZL-12 | Crusader Kings III has three zoom levels: a "paper map" far out for overview, a 3D map with a political overlay in the middle, details close in; the map stays flat, never a globe; terrain should be readable without tooltips | reported | CK3 Dev Diary 2, "The Medieval Map" (Paradox forum), search summary |
| ZL-13 | Hearts of Iron IV plays 90 % of the game on one scaling map; the widest zoom is a political overview, and as you zoom in the abstract layer fades and specifics appear | reported | PC Gamer, "Hearts of Iron IV: streamlined WW2 strategy", search summary |
| ZL-14 | Stellaris has separate galaxy and system views with their own enter and leave zoom steps; the galaxy camera's tilt resets when you come back from a system (a moderator called this a feature) | reported | Steam workshop mod discussion; Paradox forum thread 1747293, search summary |
| ZL-15 | Supreme Commander (2007) "strategic zoom" is one continuous zoom from a single unit to the whole map; at full zoom the map looks like a fullscreen minimap with units as icons, and orders are given from there | reported | en.wikipedia.org/wiki/Supreme_Commander_(video_game); shacknews.com/article/50813 |
| ZL-16 | Chris Taylor said strategic zoom came up only during development and called it the key design element; PC Gamer describes zooming from a truck-sized robot to a 6,000 km² theatre | reported | pcgamer.com "The making of Supreme Commander"; mcvuk.com Q&A, search summaries |
| ZL-17 | Supreme Commander players travel by zooming out to the strategic view and zooming in on the target, instead of scrolling or using a minimap | reported | shacknews.com/article/50813; Steam discussion app 9420 |
| ZL-18 | Players and reviewers found the dual-monitor setup far more useful than one screen: one monitor stays zoomed out to watch enemy movement, the other does the close work | reported | bit-tech.net preview; vogons.org thread, search summaries |
| ZL-19 | A Relic developer explained that a long zoom needs every asset to shrink to low-resolution versions and then to dots; their Essence engine lacked it, so Company of Heroes kept a short zoom; Gas Powered Games built the Supreme Commander engine for the zoom from the start | reported | Peter Sahui, "I can see my base from here! Strategic zoom in RTSes", Matchsticks for my Eyes, 2011-01-11 (secondary, quoting both studios) |
| ZL-20 | Complaints about icon views: dense icons merge into a blob (Planetary Annihilation forum, about Supreme Commander); Stardock's developers wrote that they did not want players zooming out to "a sea of icons" and playing most of the game from there | reported | forums.planetaryannihilation.com "Strategic Icons"; forums.wincustomize.com/469761, search summaries |
| ZL-21 | Sins of a Solar Empire zooms from one ship to several star systems without a pause; a ship too small to draw becomes an icon in the owner's colour; at planet scale ships and structures are pips around the planet; there is no separate map | reported | bit-tech.net review; Stardock forum post 174536, search summaries |
| ZL-22 | Total War splits a turn-based campaign map from real-time battles on separate maps; the campaign exists so the player chooses each battle (artist Nick Tresadern); since Rome, the battle map matches the campaign terrain where the armies met | reported | pcgamer.com "20 years ago Shogun: Total War..."; giantbomb review, search summaries |
| ZL-23 | Total War battles have pause (orders can be given while paused), slow motion and fast forward, and a tactical map key | reported | thegamer.com Warhammer 3 tips; Feral Interactive Warhammer manual, search summaries |
| ZL-24 | When a battle is not played but auto-resolved, players see it as a different model from the played battle (Bannerlord) | reported | [LJ-15](layer-jumps-multiresolution.md) |
| ZL-25 | Company of Heroes 3 players complain the camera is always too close; a zoom-out mod works only in skirmish; a CoH2 mod calls the default "claustrophobic" and warns that far zoom can cause graphical faults | reported | Steam discussions app 1677280; CoH2 workshop 433282419, search summaries |
| ZL-26 | Men of War "direct control" takes one unit: move with keys, aim with the mouse; reviewers say it is essential because path finding is poor around buildings; the target box's colour shows whether a shot can penetrate | reported | expertreviews.co.uk and gamespot.com Men of War reviews; Steam app 244450, search summaries |
| ZL-27 | They Are Billions: "Pause the game and take all the time you need"; while paused you place buildings, order the army and read information; the designers say the game is about strategy, not execution speed | sourced | numantiangames.com/TheyAreBillions |
| ZL-28 | They Are Billions' engine runs up to 20,000 infected, each with its own AI; noise draws them; one infected inside a building can turn everyone in it, so the colony "falls in minutes" | sourced | same as ZL-27 |
| ZL-29 | They Are Billions has no fast forward; players on long missions ask for one, and the game itself slows when the machine cannot keep up | reported | Steam discussions app 644930 (2017 to 2024), search summary |
| ZL-30 | Cities: Skylines caps how many citizens travel at once while the rest live off-screen; traffic is simulated 4 times a second and drawn interpolated; gridlocked vehicles teleport home | reported | gamedeveloper.com "How traffic works in Cities: Skylines" (Thompson 2020) |
| ZL-31 | Cities: Skylines' info views are map overlays (traffic paints roads green to red by density); Cities: Skylines II has pause and three speeds (1, 2, 3 keys), a cinematic camera, and a buildable area of 159 km² | reported | gamingonlinux.com preview; shacknews.com keybindings; pcgamesn.com map size, search summaries |
| ZL-32 | Cities: Skylines players mod the camera for more range: a mod raises minimum and maximum zoom to 6 and 9 times normal; a CS2 mod lets the camera go to 8 times the playable width | reported | Steam workshop 1756089251; thunderstore FreeRangeCameraLite |
| ZL-33 | Manor Lords fights on the same map as the town, managing troops and villagers at once, with no separate battle map and no pause screen for battles (preview); the developer calls it "a city builder with battles" | reported | keengamer.com preview; Yahoo/PC Gamer news on the developer's post, search summaries |
| ZL-34 | Manor Lords' militia are the town's own citizens with weapons made in its workshops; mercenaries cost treasury and walk in from other regions; a rally banner gathers the army; units take stances (give ground, stand your ground, push forward) | reported | gamespot.com Manor Lords army guide; esports.gg combat guide |
| ZL-35 | Manor Lords has a setting "Reaction when enemy was spotted": do nothing, pause, or slow down; by default it slows to 1x; unpausing returns to base speed | reported | game8.co Manor Lords settings; Steam discussions app 1363080 |
| ZL-36 | Manor Lords' "visit mode" lets the player walk the town in third person with WASD at any time, without interacting; it is marked experimental | reported | destructoid.com and gameshorizon.com guides |
| ZL-37 | RimWorld runs 60 ticks per real second at speed 1; speed 2 is x3, speed 3 is x6, speed 4 (x15) only in developer mode; an hour is 2,500 ticks, a day 60,000 | sourced | rimworldwiki.com/wiki/Time |
| ZL-38 | RimWorld forces speed back to x1 on some events (`TimeSlower.SignalForceNormalSpeed`); a popular mod removes it and players still ask for a vanilla toggle | reported | nexusmods.com/rimworld/mods/408; Steam workshop 2559936702 changelog |
| ZL-39 | Door Kickers has deployment, planning (paused) and real-time phases; the player can pause at any time to re-plan; paths carry action waypoints; "go codes" hold troopers until the code fires, which syncs breaches; the camera only pans and zooms | reported | doorkickers.fandom.com; en.wikipedia.org/wiki/Door_Kickers; gamepretty.com controls guide |
| ZL-40 | A Door Kickers 2 complaint: when one breach is compromised, troopers go before their go code; the developer said a compromised breach would really look like that | reported | Steam discussions app 1239080 |
| ZL-41 | Frostpunk's time bar holds pause, speed and the clock, and turns red as a warning; reviewers speed up while waiting and pause for hard decisions; Frostpunk 2 pulls the camera back to districts on hex tiles, with the city as one point in a region of resource hexes | reported | checkpointgaming.net 2018 review; gamerguides.com Frostpunk 2 guide; aftermath.site impressions |
| ZL-42 | Hearts of Iron IV has five speeds, but the top speed is bounded by the CPU: the game waits only if a day finishes early, so late games run near one game hour per second on slow machines | reported | Steam discussions app 394360 and workshop comments |
| ZL-43 | Every game here that mixes management and combat on one map (They Are Billions, Manor Lords, RimWorld, Supreme Commander) keeps one map and one clock and slows or pauses for the fight; Total War splits maps only because its campaign is turn-based. We have one clock (spec 40, [0058](../decisions/0058-offices-open-layers-single-player-is-multiplayer-with-ai.md) item 5), so layer 2 fights happen on the city map, not on a battle map | estimate | ZL-22, ZL-27, ZL-33, ZL-35, ZL-38 |
| ZL-44 | A continuous zoom works where only the drawing changes (bodies become icons, ZL-6, ZL-15, ZL-21); a hard jump is needed where the verbs or the materialised truth change: taking a body, materialising a new place ([macro layer §3.3](../design/macro-layer.md)), opening an office (0058) | estimate | ZL-15, ZL-21, ZL-24 |
| ZL-45 | The height bands in the proposal below follow from one rule: a thing is selectable as itself while its real size is at least about 6 px. A 0.5 m body reaches 6 px at a 160 m wide view; a 10 m house at 3.2 km; a 100 m block at 32 km | derived | ZL-4; [zoom continuum §3b](../design/zoom-continuum.md) (6 px minimum) |
| ZL-46 | In layer 2 the camera and the clock are shared; the tool comes from the selection: a squad selected shows command verbs, a plot or building shows management verbs; "build" is a cursor mode, not a camera mode | estimate | ZL-27, ZL-33, ZL-34 |
| ZL-47 | Time caps should follow attention, not only height: a fight on screen caps at 1:1 with pause allowed in single player; a fight anywhere in the player's city drops speed to 1x and raises an alert | estimate | ZL-23, ZL-27, ZL-35, ZL-38, ZL-39; spec 40 rule 10 |
| ZL-48 | Icon views stay readable only with: a fixed minimum size, type, count and state on the icon, clustering of overlapping icons into one with a count, and order lines; without these they become the "blob" and "sea of icons" | estimate | ZL-19, ZL-20, ZL-21 |
| ZL-49 | A "go to" between far places should fly the van Wijk path (out, across, in), because Supreme Commander players already do it by hand | estimate | ZL-11, ZL-17 |
| ZL-50 | A second view (a picture-in-picture window or a second monitor) on a fight while managing elsewhere is valued; a minimap alone is not enough when two things happen at once | estimate | ZL-18, ZL-19 |

## A. How planet maps zoom (only what decides our heights)

- **Doubling levels.** Map services cut the world into a pyramid: each level doubles the resolution (ZL-1, ZL-3). On a normal screen about 22 levels take you from the planet to a room (ZL-4). Our grid is nested the same way (ZL-5), so a "level" can be our unit of height.
- **Refine by pixels, not by distance.** 3D Tiles decides by screen-space error: draw the coarse tile until its error would show as more than N pixels, then draw the children (ZL-6, ZL-7). This is the same as our 6 px symbol rule, applied to geometry.
- **Seams and popping.** Cracks between levels are covered by skirts (ZL-8). Popping is a hand-off that comes too late, mostly because data arrives late (ZL-9). Our levels are computed from seeds, not downloaded, so the risk is compute time, not bandwidth. The fixes are the same: start the finer level early, and cross-fade over a range of heights.
- **Camera.** Google Earth tilts as you come down and goes straight down as you go up (ZL-10). A long move is a zoom-out, travel, zoom-in curve (ZL-11).
- **Map games.** Paradox games use one scaling map whose drawing changes with height (ZL-12, ZL-13). Stellaris is the exception, with two views and a jump between them (ZL-14).

## B. Play along a zoom

| Game | Zoom | What you order at each height | Combat and management | Time |
| --- | --- | --- | --- | --- |
| Supreme Commander | Continuous, unit to whole map; icons when far (ZL-15, ZL-16) | Everything, at every height; players move by zooming out and in (ZL-17) | Same map; second monitor for the far view (ZL-18) | Adjustable speed in skirmish |
| Sins of a Solar Empire | Continuous, ship to several systems; icons and pips (ZL-21) | Everything | Same map | Real time |
| Total War | Two maps: campaign and battle (ZL-22) | Campaign: armies, cities, diplomacy; battle: units | Separate; the battle loads | Campaign turn-based; battle pause, slow, fast (ZL-23) |
| Company of Heroes | Short tactical zoom (ZL-19, ZL-25) | Squads | Combat only | Real time |
| Men of War | Tactical zoom plus direct control of one unit (ZL-26) | Squads, or one unit by hand | Combat only | Real time |
| They Are Billions | One map | Buildings and army in the same view, also while paused (ZL-27) | Same map; hordes up to 20,000 (ZL-28) | Pause, no fast forward (ZL-29) |
| Cities: Skylines | City builder camera, limited range (ZL-32) | Zones, roads, services; overlays (ZL-31) | Management only | Pause, 3 speeds; agents capped, drawn interpolated (ZL-30) |
| Manor Lords | One map, town and battles; walk mode (ZL-33, ZL-36) | Villagers and buildings; army raised from the town (ZL-34) | Same map, same time | Slows to 1x when enemies are spotted (ZL-35) |
| RimWorld | One map | Colonists, drafted for combat | Same map | 1x, 3x, 6x; forced back to 1x on events (ZL-37, ZL-38) |
| Door Kickers | Short top-down zoom | One squad, path by path, with go codes (ZL-39) | Combat only | Plan while paused, run in real time (ZL-39) |
| Frostpunk | City view; FP2 pulls back to districts and a region (ZL-41) | Buildings, laws; FP2 districts and expeditions | Management under pressure | Pause and speed in the time bar (ZL-41) |
| Hearts of Iron IV | One scaling map (ZL-13) | Fronts, plans, production | Same map | Five speeds, bounded by the CPU (ZL-42) |

What repeats:

1. **Continuous zoom is loved where the drawing degrades cleanly.** Supreme Commander and Sins replace units with icons and keep every verb at every height (ZL-15, ZL-21). The cost is an engine built for it from the start (ZL-19). The known failure is a field of icons where the game is played as a board, not as a world (ZL-20).
2. **Management and combat share a map when the clock is shared.** They Are Billions and Manor Lords put both in one view; the fix for overload is pause or a forced slowdown, not a second map (ZL-27, ZL-35). Total War splits them because its campaign stops time between turns (ZL-22).
3. **Speed in combat is capped or paused.** Total War, They Are Billions and Door Kickers allow orders while paused (ZL-23, ZL-27, ZL-39). RimWorld and Manor Lords drop to 1x by themselves (ZL-35, ZL-38). Players fight forced slowdowns when they cannot turn them off (ZL-38).
4. **Close tactical views feel too close when they cannot zoom out** (ZL-25). Direct control of one unit fixes what orders cannot (ZL-26).

## What this means for us (Proposal)

Every row here is an `estimate`. It is tied to findings by ID. None of it is decided.

### Height bands

View width is the ground width across a 1920 px screen. Each band border is a factor of 2 in height, with a gap between the enter and leave heights ([macro layer §3.3](../design/macro-layer.md)).

| Band | View width | Map level (ZL-4) | Drawn as | Verbs | Layer | Speed offered (spec 40 ladder) |
| --- | --- | --- | --- | --- | --- | --- |
| B0 Street and interior | under 150 m | 21 to 23 | Real meshes, bodies | The controlled body (only after "take a body") | Individual | Real in a fight, 3x walking (spec 40 rule 10) |
| B1 Tactical | 150 m to 1 km | 19 to 20 | Buildings real; bodies become markers at 6 px (ZL-45) | Squad orders: move, hold, breach, go codes (ZL-39), call support | City (command tools) | 1:1 while a fight is on screen; up to 10x otherwise |
| B2 District | 1 to 5 km | 16 to 18 | Buildings from footprints; squads as icons with counts (ZL-48) | Build, place, assign jobs, priorities; squad orders still work on icons | City (management and command) | Up to 24x (1 real hour per day) |
| B3 City | 5 to 40 km | 13 to 15 | Zone blocks, flow lines, overlays, a sample of pops | Districts, policies, edicts in force, supply routes (0043) | City | Up to 360x |
| B4 Region | 40 to 500 km | 9 to 12 | City footprints and lights, major roads | Regional programmes and budgets, if the office allows (0058) | Macro | Up to a day a second |
| B5 Globe | over 500 km | 3 to 8 | Nations, borders, weather, flow tokens | National decisions | Macro | Up to a year a second |

Speed rules: the band gives the highest speed offered; attention lowers it (ZL-47). A fight on screen caps at 1:1. A fight anywhere in the player's city drops to 1x and raises an alert. Single player may pause and give orders while paused (ZL-27). A multiplayer server keeps one speed (0058 item 5). Each band remembers its last speed, as proposed on 2026-10-08.

### Where the hard jumps go

The camera is continuous. Only three things are jumps, and none is triggered by the scroll wheel (ZL-44):

1. **Taking a body** (Individual). An explicit action, like Men of War's direct control or Manor Lords' visit mode (ZL-26, ZL-36). Zooming down to B0 without it is a camera over the city, not a body.
2. **Entering a new place in City.** Coming down from B4 to B3 over a place that is not detailed yet materialises it behind a short frozen transition ([macro layer §3.3](../design/macro-layer.md)). Over the already detailed place this is not a jump.
3. **Opening an office.** Which bands are open at all depends on the office held at a desk (0058). Above that office the camera can still look, but shows what the office knows, with no verbs.

This settles the conflict in [cameras and modes](../design/cameras-modes.md): one continuous camera, with explicit **tool** modes instead of explicit camera modes.

### Layer 2: RTS and city management together

- **One map, one clock, two tool sets** (ZL-43, ZL-46). Selecting a squad shows command verbs; selecting a plot or building shows management verbs. "Build" is a cursor mode with its own key. No separate battle map: the fight is in the city and uses the city's real buildings.
- **Squads come from the city**, as militia and guards are real pops taken from real jobs (ZL-34; [HOI-10](hearts-of-iron.md)). Arming them is a management act; losing them is a management loss. This is the repercussion loop of 0043 in its shortest form.
- **Close fights at B1, wide control at B2.** Door Kickers style planning (paths, waypoints, go codes) works at B1 (ZL-39). At B2 squads are icons and take only move, hold and retreat. The camera may always zoom out while fighting (ZL-25).
- **A fight view while managing.** A small picture-in-picture of the fight, opened from its alert, keeps one fight in sight while the player works elsewhere (ZL-18, ZL-50).
- **Long trips** fly the out, across, in path (ZL-11, ZL-49).

### Readability rules

1. Below 6 px a thing becomes a symbol of fixed screen size; above, it is drawn real ([zoom continuum §3b](../design/zoom-continuum.md), ZL-45).
2. A squad icon shows type, head count, state (moving, engaged, routed) and order line. Overlapping icons merge into one icon with a count (ZL-48).
3. A drawing change is a cross-fade over a height range, started early so it is never late (ZL-8, ZL-9).
4. A mode or layer change shows in the interface at once, and uses the enter and leave gap so it never flickers.
5. Zoom never grants vision: fog and reports are the same at every height ([cameras and modes](../design/cameras-modes.md)).
6. A forced slowdown says why, and the player can turn it into a pause, or into nothing, in settings, because players fight slowdowns they cannot control (ZL-35, ZL-38; [0056](../decisions/0056-alerts-do-not-pause-by-default.md)).

## Open questions

1. Are the band borders right? They come from a 1920 px screen and a 6 px minimum; they should be tuned by looking once a renderer exists.
2. Does B1 belong to City (squad command) or to a fourth "tactical" layer with its own interface? Recommendation: City, with command tools.
3. In single player, may the player pause during a fight in City? (They Are Billions and Total War: yes; Manor Lords preview: no.) Recommendation: yes in single player, never on a server.
4. Should a fight anywhere in the city force 1x, or only alert? Default proposed: 1x plus alert, switchable to pause or nothing.
5. Do squad orders given from B2 icons use the same order record as orders from B1 (spec 39)? Recommendation: yes, one order type.
6. Is the picture-in-picture fight view in scope for the first City build, or later?
7. Should the camera tilt automatically as it comes down, as Google Earth does (ZL-10), and from which band?
8. Does a minimap still exist, or does zoom replace it, as in Supreme Commander (ZL-17)?
9. How long may the materialising transition (jump 2) take before it feels like a loading screen?

## Sources

- Microsoft Learn, "Bing Maps Tile System": https://learn.microsoft.com/en-us/bingmaps/articles/bing-maps-tile-system
- Google Maps JavaScript API, "Map and tile coordinates": https://developers.google.com/maps/documentation/javascript/coordinates
- CesiumGS, 3D Tiles specification: https://github.com/CesiumGS/3d-tiles/blob/main/specification/README.adoc
- CesiumJS API docs, Cesium3DTileset and Globe: https://cesium.com/downloads/cesiumjs/releases/1.84/Build/Documentation/Cesium3DTileset.html, https://cesium.com/downloads/cesiumjs/releases/1.84/Build/Documentation/Globe.html
- Cesium community, skirts: https://community.cesium.com/t/skirts-tablecloth-overhangs/1864, https://community.cesium.com/t/terrain-with-cracks/5543
- MSFS 2024 pop-in: https://flyawaysimulation.com/ask/answers/reduce-scenery-pop-in-msfs-2024/, https://forums.flightsimulator.com/t/massive-lod-changes-visible-on-ground-when-flying-over-photogrammetry-areas/738065
- Google Earth auto-tilt: https://rpls.com/forums/software-cad-mapping/google-earth-gripe
- d3 interpolateZoom (van Wijk and Nuij 2003): https://d3js.org/d3-interpolate/zoom
- CK3 Dev Diary 2: https://forum.paradoxplaza.com/forum/developer-diary/ck3-dev-diary-2-the-medieval-map.1274052/
- PC Gamer, Hearts of Iron IV: https://www.pcgamer.com/hearts-of-iron-iv-streamlined-ww2-strategy
- Stellaris galaxy tilt: https://forum.paradoxplaza.com/forum/threads/galactic-map-view-tilt-wont-stay-the-same.1747293
- Supreme Commander: https://en.wikipedia.org/wiki/Supreme_Commander_(video_game), https://www.shacknews.com/article/50813/supreme-commander-on-xbox-360, https://www.pcgamer.com/the-making-of-supreme-commander, https://www.mcvuk.com/development-news/qa-chris-taylor-talks-strategy/
- Sahui, "I can see my base from here! Strategic zoom in RTSes" (2011): https://www.matchstickeyes.com/tag/rts-zoom/
- Strategic icons: https://forums.planetaryannihilation.com/posts/719229, https://forums.wincustomize.com/469761
- Sins of a Solar Empire: https://www.bit-tech.net/reviews/gaming/pc/sins_of_a_solar_empire/3/
- Total War: https://www.pcgamer.com/20-years-ago-shogun-total-war-started-a-long-legacy-but-it-was-almost-a-very-different-game, https://www.thegamer.com/total-war-warhammer-3-beginner-tips/, https://www.feralinteractive.com/en/manuals/warhammertw/latest/linux/
- Company of Heroes camera: https://steamcommunity.com/app/1677280/discussions/0/3782498683131601031, https://steamcommunity.com/sharedfiles/filedetails/comments/433282419
- Men of War: https://www.expertreviews.co.uk/software/48270/505-games-men-of-war-review, https://www.gamespot.com/men-of-war/reviews/men-of-war-review-6206559/
- They Are Billions: https://numantiangames.com/TheyAreBillions/, https://steamcommunity.com/app/644930/discussions/0/1620599015883711370
- Cities: Skylines: https://www.gamedeveloper.com/design/how-traffic-works-in-cities-skylines, https://www.shacknews.com/article/137454/pc-buttons-controls-cities-skylines-2, https://www.pcgamesn.com/cities-skylines-2/map-size, https://steamcommunity.com/sharedfiles/filedetails/changelog/1756089251
- Manor Lords: https://keengamer.com/articles/previews/manor-lords-preview-total-war-meets-medieval-city-building/, https://Gamespot.com/articles/manor-lords-army-militia-retinue-mercenary-guide/1100-6523016/, https://game8.co/games/Manor-Lords/archives/452352, https://www.destructoid.com/how-to-walk-around-your-town-in-third-person-in-manor-lords/
- RimWorld: https://rimworldwiki.com/wiki/Time, https://www.nexusmods.com/rimworld/mods/408
- Door Kickers: https://en.wikipedia.org/wiki/Door_Kickers, https://doorkickers.fandom.com/wiki/Door_Kickers, https://steamcommunity.com/app/1239080/discussions/0/603030065018599903
- Frostpunk: https://checkpointgaming.net/reviews/2018/04/frost-punk-review-not-winter-wonderland/, https://www.gamerguides.com/frostpunk-2/guide/getting-started, https://aftermath.site/frostpunk-2-impressions/
- Hearts of Iron IV speed: https://steamcommunity.com/app/394360/discussions/0/154644045359901048
