# Star Engine (Star Citizen): what works across scales

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [0042](../decisions/0042-pop-presentation-per-layer.md), [0060](../decisions/0060-one-fight-three-views-behaviour-from-jobs-and-skills.md), [0061](../decisions/0061-the-world-runs-without-the-player.md), [spec 28](../tier0/specs/28-pop-levels.md), [spec 19](../tier0/specs/19-light-field.md), [spec 36](../tier0/specs/36-fields-and-weather.md), [zoom continuum](../design/zoom-continuum.md), [hordes-sound-sky-and-testing](hordes-sound-sky-and-testing.md). **Evidence:**
- **Read through verbatim mirrors** (starcitizen.tools, api.star-citizen.wiki): RSI Comm-Links and letters.
- **Secondhand:** press interviews (GamersNexus, Massively OP), the starcitizen.tools wiki and a forum transcript of a CIG talk.
- **Not read:** the 2016 "Multicrew & Physics Grids" article (teaser only) and Spectrum posts.

**IDs:** SE.


The user asked how Star Engine handles a rotating planet, wind and day/night across scales, and which cheap ideas apply to us. The user's own idea: each layer shows only the information it needs. A storm seen on the world map is the same storm that rains on a house in third person, where a pop is fat and smokes; on the world map that pop is a number.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| SE-1 | About 8 months of work moved Star Citizen to 64-bit world coordinates; rendering became camera-relative and the GPU stayed 32-bit because the visible range fits | reported | gamersnexus.net, Roberts interview |
| SE-2 | Only physics and positioning went 64-bit, not the whole engine; no performance gain | reported | gamersnexus.net, Sean Tracy 2016 |
| SE-3 | One hierarchical "zone system" for rendering, occlusion, updates and networking; a ship moves as one unit: "you just update the master, not the children" | reported | gamersnexus.net, PAX East 2015 |
| SE-4 | Alpha 3.0 (Dec 2017): "Planets and moons now rotate to give a day/night cycle"; physics grids for planets and stations | sourced | RSI Comm-Link 16348 |
| SE-5 | The sun is a fixed entity; day/night comes from the planet's rotation and orbit | reported | gamersnexus.net, Roberts 2016 |
| SE-6 | Days are short: Hurston's day is 2 h 28 m 48 s | reported | starcitizen.tools/Hurston |
| SE-7 | Terrain heightmaps of 2k per about 4 × 4 km are tiled and blended across whole planets; ground materials authored for 4 m patches | sourced | Adobe Substance magazine, CIG artists 2019 |
| SE-8 | Planet Tech v4 (from Alpha 3.8, Dec 2019): temperature and humidity maps choose the biome; features repeat wherever the same conditions occur; snow, wind, dust and sandstorms from a rotating planet-scale texture | reported | starcitizen.tools/Planet_Tech_v4 |
| SE-9 | "What you see from orbit is exactly the same terrain you'll see on the ground" (2019 panel); repeated for Genesis (2024) | reported | starcitizen.tools/Genesis_(Star_Engine) |
| SE-10 | Weather intent (2016): artists mark biomes hot or cold, a simulation generates wind, wind generates clouds; no hand-made cloud maps | reported | gamersnexus.net, Roberts 2016 |
| SE-11 | Volumetric clouds shipped in Alpha 3.14 (2021) | reported | api.star-citizen.wiki comm-link 19952 |
| SE-12 | Alpha 4.2 (June 2025) added rain, clouds and lightning at named locations as a first step to dynamic weather; Pyro I's two polar storms are permanent authored hazard regions | reported | starcitizen.tools, Alpha 4.2.0 |
| SE-13 | Planet Tech v5 (Genesis) adds geology, soil and nutrients so biomes "emerge"; still unreleased as of September 2026 | reported | starcitizen.tools, Alpha 4.11.0 |
| SE-14 | Object containers stream on clients (2018) and servers (2019); an entity graph database persists every replicated entity; each entity has one authoritative server | sourced | api.star-citizen.wiki comm-link 18397 |
| SE-15 | With server-side streaming, an entity with no player near is serialised to the database ("frozen"); a spatial index loads and unloads by client positions | reported | starcitizen.tools/Object_Container_Streaming |
| SE-16 | The streaming bubble is by apparent size: an entity streams in when it would cover more than 5 pixels on a virtual 1080p screen, so a moon streams from far and a ship only up close | reported | Frontier forums, transcript of a CIG talk |
| SE-17 | Persistent Entity Streaming (Alpha 3.18, March 2023): dropped items stay where left | reported | starcitizen.tools/Persistent_Entity_Streaming |
| SE-18 | March 2024: 350 players in one shard across two servers; replication separated from simulation | sourced | Letter from the Chairman 2024-03-14 |
| SE-19 | Alpha 4.0 (Dec 2024): each planet, landing zone or station on its own server, boundaries "effectively invisible" | sourced | Letter from the Chairman 2024-12-19 |
| SE-20 | August 2026: a territory still has one server and cannot add more when a crowd gathers; first dynamic meshing only in 4.10 instancing | sourced | Letter from the Chairman 2026-08-27 |
| SE-21 | Quantum (CitizenCon 2019): "quanta" are lightweight agents with desires, skills, jobs and inventory but no pathfinding, animation or geometry; "probability volumes" of pirates, freighters and security are rolled when a player enters | reported | starcitizen.tools/Quanta |
| SE-22 | Tests ran up to two million quanta; about 100,000 per system expected to be enough; virtual NPCs "go about their lives regardless of whether any players are in the vicinity" | sourced | api.star-citizen.wiki comm-link 17821 (Zurovec AMA 2020) |
| SE-23 | Porting Quantum to C# made it about 100× faster; rollout planned in stages (fuel, repairs, pirate and freighter movement) | reported | massivelyop.com, 2021 |
| SE-24 | Quantum, renamed StarSim, was still a roadmap item in November 2025; no evidence the full economy is live | reported | starcitizen.tools/StarSim |
| SE-25 | NPC schedules by role: shared baseline behaviours (eat, sleep, socialise) with job tasks on top | sourced | RSI Monthly Report, June 2020 |
| SE-26 | On a real-size planet (40,000 km around) a 32-bit float resolves only about 4 m, a double about 10 nm; positions as (chunk, local offset), or integers at 0.5 m cells (8 × 10⁷ around the equator, inside 32 bits) | derived | SE-1, SE-2 |
| SE-27 | A world that never leaves the ground gets the same day/night from a sun direction computed from the tick (tilt, year, rotation) as from rotating the planet, with no frame to update | derived | SE-4, SE-5; HST-148 |
| SE-28 | The 5-pixel rule is attention by apparent size; for us, what a layer shows (0042, 0060) sets what must be computed: a storm hundreds of km wide is visible on the world map, a pop's traits only in layer 1 | estimate | SE-16 |
| SE-29 | Each layer reads its own view of the same storm record: the world map its outline and regional totals, the city each building and squad, the street each body. Totals are sums over people who are determined (`derive(pop_id, t)`, keyed draws) but not computed until someone looks | estimate | 0042, 0060, spec 28 |
| SE-30 | Probability volumes rolled when a player enters (SE-21) make the result depend on when someone came; ours must be keyed by (region, tick) so arriving early or late finds the same thing (0061, spec 27 rule 6) | estimate | SE-21 |
| SE-31 | Universe-scale abstract agents were announced in 2019 and were not live by late 2025; a one-world simulation at several scales must arrive in small tested steps (GS-20 lists Star Citizen's scope as a warning) | estimate | SE-21..SE-24 |

## 1. Answer in one page

Star Engine reaches across scales with four ideas:
- **Nested frames:** update the parent, not the children (SE-3).
- **Derive, don't store:** the same function makes orbit and ground agree (SE-7..SE-9).
- **A fixed sun with a rotating planet** (SE-4, SE-5).
- **Stream by apparent size,** with everything else frozen in one entity store (SE-14..SE-16).

Its universe-scale agent simulation (Quantum, now StarSim) was announced in 2019 and is still not live (SE-24). That is a warning about scope, not about the idea.

For us:
- **Positions as chunk plus local offset,** never one big float (SE-26).
- **Rotate the sun, not the planet:** same result, no state (SE-27).
- **Weather from one planet-wide function** that every layer samples. Star Citizen uses a rotating texture (SE-8); our storm records do the same job (HST-147).
- **What a layer shows decides what is computed** (SE-28).

The 5-pixel rule (SE-16) and "orbit matches the ground" (SE-9) have a drawing counterpart in [vector-graphics](vector-graphics.md): vector tiles keyed by our nested grid with symbol-scale styling (VG-54) draw the upper layers from the same data at any zoom.

The user's point is already decision 0042 and 0060: at world level a pop is a number, in third person a body with traits. This note adds the rule that keeps the two honest. The same storm record feeds every layer, each layer computes only its own effects, and every statistic is the exact total of individuals already determined by the seed (SE-29). The fat smoker in the house belongs to the region's smoking share. Whether he gets sick in this storm is decided whether anyone watches (0061), and computed only when someone looks or a rule needs it.

## 2. Storm example across layers

| Layer | The storm is | Computed |
| --- | --- | --- |
| World (layer 3) | An outline over a region; crops damaged, travel slowed, illness up a few percent | Regional totals by formula |
| City (layer 2) | Rain over districts; roofs leak, squads slow, visibility drops | Per building and squad |
| Individual (layer 1) | Rain on the roof; this pop is cold, wet and coughing | Per body |

## 3. Sources

- **Press:** gamersnexus.net (Roberts 2015, 2016, 2019; Tracy 2016); massivelyop.com 2021.
- **Official, via mirrors:** RSI Comm-Links 16348, 17670, 17821, 18397, 19952; Letters from the Chairman 2024-03-14, 2024-12-19, 2026-08-27.
- **Wiki:** starcitizen.tools (Hurston, Planet Tech v4, Genesis, Alpha 4.2.0, 4.11.0, Object Container Streaming, Persistent Entity Streaming, Quanta, StarSim).
- **Other:** Adobe Substance magazine (CIG artists, 2019); Frontier forums (CIG talk transcript).
