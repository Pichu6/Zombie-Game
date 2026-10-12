# Improvements from the 2026-10-09 research, ranked

Proposals only, drawn from the research notes of 2026-10-09 and 2026-10-10:
- [perception-and-run-ahead](../research/perception-and-run-ahead.md) (RA)
- [borrowed-from-other-fields](../research/borrowed-from-other-fields.md) (XD)
- [hordes-sound-sky-and-testing](../research/hordes-sound-sky-and-testing.md) (HST)
- [star-engine](../research/star-engine.md) (SE)
- [vector-graphics](../research/vector-graphics.md) (VG)
- [library-of-babel-and-sacred-geometry](../research/library-of-babel-and-sacred-geometry.md) (LB), added later the same day

Ranked by value against cost. Each item names the spec it would change and whether it touches the generator and planet work another session is doing (origin/main at `bd4707e` when ranked).

**Decided since:** item 3 and the moaning of item 13 are [0064](../decisions/0064-sound-and-sky-defaults.md); items 2 and 6 are [0065](../decisions/0065-ids-per-creator-and-positions-as-chunk-plus-offset.md); item 37 is [0066](../decisions/0066-player-states-embodied-detached-observer.md); the rules of the story drafts (items 46–66) are [0067](../decisions/0067-stories-are-found-never-written.md). Item 23, the run-ahead decision, is still a proposal.

## Now: cheap, high value, unblock the rest

| # | Improvement | Spec | Cost | Why now | Touches generators? |
| --- | --- | --- | --- | --- | --- |
| 1 | **Built 2026-10-11 ([spec 56](../tier0/specs/56-probes-and-divergence.md)).** Per-subsystem hash every tick in test builds, plus a tool that prints the first divergent tick and the subsystem | 06, 40 | Small | Every later piece (chunks, threads, run-ahead) needs it to debug; Age of Empires wished it had built this first (HST-126, HST-127) | No |
| 2 | **Decide how ids are made before chunks exist:** one global counter today (`World::new_entity`, 5 call sites) breaks local rollback and parallel chunks; mint ids per creator, e.g. (chunk, tick, sequence) | 0041, 27 | Small now, large later | Cheap at 5 call sites; the run-ahead decision depends on it (RA-114) | No |
| 3 | **Fix the noise numbers:** grid-dead background 35 day / 25–28 night (or state that the claim is about 6×, not 10×); notice at 0 dB, place at +10 dB; generator 85 dB at 1 m | 18 | Small, re-pins hashes | It's the core loop: "the generator, not the shout, brings the horde" (HST-138..HST-140) | No |
| 4 | **Testing harness, first slice:** run each seed twice and diff; `SIM_BUGGIFY` sites for save/load round trips, forced jumps and speed changes; a "jump = step" model test (tick-by-tick stepping as the oracle) | 06, 40 | Medium | FoundationDB and TigerBeetle got most of their quality from this (HST-104..HST-118) | No |
| 5 | **Vector export of the world map:** rivers, coasts and borders from `zs gen world` as polylines (well under 1 MB for the planet, against 32 MB for one globe image) | 52, view | Small | Useful before any renderer, and a check on the generators (VG-52, VG section 4.3 step 8) | **Yes: generator side; for the session working there** |
| 6 | **Positions as chunk plus local offset (or integers at 0.5 m cells), never one big float,** decided before chunks: a 32-bit float resolves only about 4 m at planet scale | 0063, 27 | Small now | Same reason as item 2: cheap before chunks, costly after (SE-26) | Touches the planet work; decide together |

## Next: when the chunk work starts (spec 27)

| # | Improvement | Spec | Cost | Notes |
| --- | --- | --- | --- | --- |
| 7 | Two thresholds and groups switching level as one; test that hot never touches cold | 27 | Small | RA-1, RA-4, XD-117 |
| 8 | Hot side owns border edges; integer accumulators emit one body per whole unit; warm → hot as an exact integer split | 27 | Medium | XD-119, XD-120, HST-15 |
| 9 | A minute-level step (the existing 60 s body step) between hot and hourly warm, or event-based coupling: our 7,200:1 ratio is far outside practice | 05, 27 | Medium | XD-116 |
| 10 | Hordes far away as counts on the chunk graph: flow = min(demand, supply, width × capacity), Weidmann speed | 27, 22 | Medium | HST-143 |
| 11 | Hot set control: working-set window, probation for chunks touched once, wake budget in work units (never milliseconds) | 27, 29 | Medium | XD-133..XD-136 |
| 12 | Region outcomes keyed by (region, tick), never rolled on arrival (Star Citizen's "probability volumes" depend on when a player comes) | 27, 28 | Small | SE-30; 0061 |

## Then: gameplay systems

| # | Improvement | Spec | Cost | Notes |
| --- | --- | --- | --- | --- |
| 13 | Walkers moan; a sound turns a horde as a wave | 23 | Medium | HST-146, RA-111 |
| 14 | Barricade strength in kN/m against crowd load by queue depth | 10 | Small | HST-145 |
| 15 | Pop knowledge as tracks (16 per pop, 256 B) extending the belief store (`beliefs.cpp`, spec 39): uncertainty and confidence computed when read; sound as a sector; cross-fixes | 39, 24 | Medium | XD-128..XD-132 |
| 16 | Sound marks in the window at the last opening on the path, with bearing error | 16, 18 | Small | HST-55; queued video idea 2 |
| 17 | Overlays as contour lines (notice and place thresholds) and flow arrows (horde flow) over the field heat maps | 16 | Small | VG section 4.2; HST-139, HST-143 |
| 18 | A radar view of what the controlled pop knows: blips and fading tracks drawn from its belief tracks, never the truth; player tool or debug lens is the user's call | 16, 39 | Small after 15 | VG section 4.2, VG-16; XD-128 |
| 19 | Two-level sound for cities: opening-to-opening tables per building at generation, a street graph between | 18 | Medium | HST-141; needs the city generator, **touches generators** |

## Later: map, sky, time and storage

| # | Improvement | Spec | Cost | Notes |
| --- | --- | --- | --- | --- |
| 20 | Map and strategy layers drawn as vector tiles keyed by our nested grid: one style table over camera height, instanced line quads (never native wide lines), flat polygons, SDF symbols and pop dots, MSDF labels on a collision grid; street and tactical stay raster | renderer, zoom continuum | Large | VG section 4.3; cartographic look by default with a "command display" style option |
| 21 | Sun and moon from formulas (tilt, year, synodic month); light from the USNO closed form with a cloud factor; rotate the sun, not the planet | 19 | Small | HST-148, HST-93, SE-27; planet parameters, **touches the planet work** |
| 22 | Storm records as rain weights; map and street read the same storms; each layer computes only its own effects of a storm | 36 | Medium | HST-147, SE-29; **touches the planet work** |
| 23 | Run-ahead decision (next free number): commit at the view tick, stream of curves and pulses, lead set by order rate, rollback whole first and local later | new decision, 40 | Large | RA section 5; needs 1, 2 and 4 first |
| 24 | Saves: a full save per game day, hourly deltas against it, regions stored once by hash | 06 | Medium | XD-124; zstd is vendored |
| 25 | Far entities as motion laws with a next-event queue; beacon flags for regions not ticked | 27 | Medium | XD-122, XD-125 |
| 26 | Forecasts, news carried by people, moon phases in play | 36, 39 | Small each | HST-149, XD-127 |
| 27 | High-contrast map style as a palette swap once the style table exists | 16, renderer | Small after 20 | VG section 4.2 |

## From the Library of Babel note (LB, added later on 2026-10-09)

| # | Improvement | Spec | Cost | Notes | Touches generators? |
| --- | --- | --- | --- | --- | --- |
| 28 | **Searchable traits assigned by rank, and queries through `CohortPermutation::inverse`** (it exists; only `forward` is called): "every pilot in France" across lazy pops with no storage, counts exact | 28, content tools | Small | LB-36, LB-21 | No |
| 29 | **The librarian:** incremental sifting patterns over the event log turn forming stories into leads for the director, observer camera, headlines and Chronicle; detects, never injects | macro layer, 0062, new spec | Medium | LB-37 (Ryan, Winnow) | No |
| 30 | **Call numbers:** the save header (build and data hashes, seed, settings, tick) as a shareable code with "open at tick" from the nearest daily save | 06 | Small | LB-38; after save format 5 | No |
| 31 | Mark every field Babel (pure function of seed, place, time) or chronicle (stepped); a stepped field needs a reason | 27, 28 | Small | LB-39; with item 12 | No |
| 32 | Exact partition for counted things on the nested grid (centre child plus 3 edge children); child area shares kept for smooth fields | 30, 27 | Medium | LB-40; answers the HY-50 open question | **Yes: planet grid** |
| 33 | Oatmeal signature test: towns and countries over N seeds must read differently in one line, and each difference must change play | content tools, 52 | Small | LB-41 | **Yes: test on generator output** |
| 34 | Golden-ratio (R2) scattering inside cells; spherical Fibonacci with Keinert's inverse for "nearest of N" | zoom continuum T2 | Small | LB-42 | **Yes: when T2 is built** |
| 35 | "Find my family": a survivor's relatives abroad are lazy pops; ask through beliefs, meet them with a consistent past | 28, 39 | Small after 28 | LB-43; a demo-flow hook | No |
| 36 | Rejection search for generator presets and curated seed lists | sandbox parameters | Small | LB-44 | Generator presets |

## From the playing, knowing, commanding and moving note (PKC, added later on 2026-10-09)

| # | Improvement | Spec | Cost | Notes | Touches generators? |
| --- | --- | --- | --- | --- | --- |
| 37 | **Player states decision:** embodied, detached, observer; detachment presets (free, knowledge, off) over separate permissions; the save records which were used; pinned picture-in-picture windows; the body stays in the world with alerts | new decision (0064 or later) | Small (a decision) | PKC-1..PKC-35, PKC-145; README proposal | No |
| 38 | **Belief grade and words:** each belief graded on source (A–F) and content (1–6) from the simulation; ICD 203 probability words, one word lost per retelling at spec 39's 0.7 | 39 | Small | PKC-141, PKC-142 | No |
| 39 | **Belief on the map:** solid seen, dashed believed, an uncertainty area growing with age (fuzzy, not a crisp cone), a last-known ghost with age and trailer; views read only `Beliefs::view` | 16, map | Medium | PKC-143, PKC-144 | No |
| 40 | **Military orders in the cause trace:** order kind with parent, intent, valid-until and medium; latency per hop; the soldier's card with intent one and two levels up and Locate, Follow, Play as | 39, 0058, 0060 | Medium | PKC-149..PKC-153 | No |
| 41 | **Paths to office:** assembly delegate, aid broker, armed protector; legitimacy vector; succession line per office | 0058, new spec | Medium | PKC-147, PKC-148 | No |
| 42 | **Vehicles and traffic spec:** IDM and MOBIL for hot cars at 0.5 s; capacity factors for warm and cold; level change only out of view, never despawned | new spec, 27 | Large | PKC-154..PKC-156; the demo's street corner | No |
| 43 | **Pop daily schedules from the seed,** calibrated to time-use and travel surveys; only deviations stored | 28, 47 | Medium | PKC-157 | Touches town data |
| 44 | **Delivery contracts and pay rule** in integer cents | 33 | Small | PKC-158 | No |
| 45 | **About 12 overlays and an attention list** in three tiers, ranked by severity × people ÷ time to failure | 16 | Medium | PKC-159, PKC-160 | No |

## From the story explorer draft (added later on 2026-10-09)

The user asked to navigate the simulated world like Google Earth and find its stories ([story-explorer](story-explorer.md)). Item 29 (the librarian) is its core; these are the steps around it, in build order.

| # | Improvement | Spec | Cost | Notes | Touches generators? |
| --- | --- | --- | --- | --- | --- |
| 46 | **Thread records and the first patterns** (a death with a cause, the hidden bite, the butterfly) on the built chronicle and cause chains; `zs stories` prints a run's threads, scores and crossings | 25, new spec | Small | Story explorer §5, §9 step 1; first slice of item 29 | No |
| 47 | **Derived ties** (family, household, coworkers, classmates, neighbours) from ids, rosters and lifepath terms, never stored; `zs who-knew <pop>` | 28, content tools §4.1 | Small | Story explorer §4; uses item 28 | Reads lifepaths |
| 48 | **Probability carried on chance events,** and surprise in bits (−log₂ p) for scoring | 04, 25 | Small | Story explorer §6 | No |
| 49 | **The world's own story spots:** a deterministic share of the detail budget chosen from world state, never from the player | 27, 0061 | Medium | Story explorer §3; measure with the spec 28 benchmark | No |
| 50 | **A Stories panel in the 2D window:** list, Reader, clickable names, Locate | 16 | Small | Story explorer §9 step 6; the only window change | No |

## From the cases and story blocks drafts (added later on 2026-10-09)

The user asked for mysteries solved from many sources ([cases-and-living-lore](cases-and-living-lore.md)), for blocks that make every kind of story, and for a diary per pop ([story-blocks](story-blocks.md)).

| # | Improvement | Spec | Cost | Notes | Touches generators? |
| --- | --- | --- | --- | --- | --- |
| 51 | **Diary, first slice:** a per-pop day page from the Tier 0 run, extending spec 25's day summary to each pop; quiet days fold; `zs diary <pop>` | 25 | Small | Story blocks §6, §8 step 1; everything it reads is built | No |
| 52 | **Lines with criteria, most specific wins,** shared by chronicle sentences, diaries and titles | 25 | Small | Story blocks §3; SB-6 | No |
| 53 | **The first case: who took the fuel.** Generalise "hide a bite" (spec 24 rule 7) to "hide an act"; ledger gap, generator hours, footsteps above the notice threshold, a false statement; `zs cases` with trace routes and solvability | 24, 25, 32 | Small | Cases §12 step 1; MY-24 | No |
| 54 | **Traces as things:** documents and marks with decay; witness memories from perception (specs 18, 19); statements that can lie (spec 39) | 18, 19, 39 | Medium | Cases §6; MY-22 | No |
| 55 | **Seeds (hooks) in the lifepath data:** a second person, a kind, residue and the wants they can wake, for every genre; `zs hooks <pop>` | content tools §4.1 | Medium | Cases §4; story blocks §3; MY-17 | **Yes: lifepath data and generator** |
| 56 | **Wants as plan templates** for pop AI; first Reunite and Protect | 24, pop behaviour | Medium | Story blocks §3, §8 step 4 | No |
| 57 | **Fortune curve and arcs** from pop state (six shapes) | 28 | Small | Story blocks §3; SB-3, SB-15 | No |
| 58 | **Genre shapes in the librarian** and the solvability solver for cases | new spec | Medium | Story blocks §4; cases §8 | No |
| 59 | **Schemes:** several actors recruited along ties and hooks; cover-ups as secret acts | 24, 0058 | Large | Cases §5; MY-4, MY-23 | No |
| 60 | **Places' affordances** (what a hospital, checkpoint or church makes possible) and physical diaries as items | city and building generators | Medium | Story blocks §3; cases §6 | **Yes: with the city and building generators** |

## From the lore note (LR, added on 2026-10-10)

How Project Zomboid and others handle lore ([lore-made-and-generated-worlds](../research/lore-made-and-generated-worlds.md)), turned into steps for the story blocks.

| # | Improvement | Spec | Cost | Notes | Touches generators? |
| --- | --- | --- | --- | --- | --- |
| 61 | **Scene blocks chosen by facts:** a derived interior shows the scene that fits what happened there (PZ's randomized scenes, but never by dice) | cases §11, content tools | Medium | LR-20 | **Yes: interiors (spec 51)** |
| 62 | **Broadcasts from events:** TV and radio lines over real events and each office's official version; hidden stations are real pops; the last loop when the power goes | 39, story blocks | Small | LR-21; PZ's scripted days are the model | No |
| 63 | **Objects that remember:** graves, memorials, graffiti, messages for missing family, made by pops' actions and pointing at real events | story blocks, 25 | Small | LR-22 | No |
| 64 | **Premise layer and canonical presets:** outbreak rules as parameters (PZ's "Zombie Lore"); optional authored origin presets | sandbox parameters | Small | LR-23 | No |
| 65 | **Hand-built landmarks placed by rule:** labs, prisons, bases carrying authored hooks, only where a real-world rule puts them | city and building generators | Medium | LR-24 | **Yes: with the city generator** |
| 66 | **Graded sources and ranks on beliefs;** knowledge-mode indexes that never leak hidden things | 39, explorer | Small | LR-25, LR-26 | No |

## From the sensors and soundscape draft (added 2026-10-10)

| # | Improvement | Spec | Cost | Notes | Touches generators? |
| --- | --- | --- | --- | --- | --- |
| 67 | **Probes:** per-subsystem and per-chunk timers and counters, a debug overlay and a log, beside the per-tick hash; never fed into the simulation (per-system timers built with spec 56; chunks, overlay and log remain) | 05, 40 | Small | [sensors-and-soundscape](sensors-and-soundscape.md) §1; goes with item 1 | No |
| 68 | **Readers module:** district loudness, activity counts, beacons and the attention list computed from the published snapshot; consumed by audio, UI and the director | engine architecture | Medium | sensors §1; XD-125, PKC-160 | No |
| 69 | **Governor rules:** what measured load may change (speed caps, detail where A1, pinned-window refresh, render and audio detail) and what it never may | 40, 27 | Small (a rule) | sensors §1; XD-136, RA-115 | No |
| 70 | **Soundscape spec:** silence or music on the world map, ambience fading in with zoom toward the zoom target, a listener per mode, beds by zoom band from the same dB the walkers hear, smoothing, knowledge-mode rule | new spec (audio) | Medium | sensors §2; answers ES-46 | No |

## Rejected or nothing to gain

- A separate low-end vector "tactical display" mode: a second art style to maintain; the upper zoom bands already do this (VG section 4.2).
- An oscilloscope audio view: no game need (VG section 4.2).
- Vector graphics for saves and network: the replay is already commands, not pixels (VG-53).
- A wireframe street view: conflicts with the Decided realism (VG-55).
- Copying Star Citizen's universe-scale agent economy in one step: announced 2019, not live in 2025 (SE-24, SE-31).
- Reversible simulation steps to roll back without snapshots: a step merges states, so information is lost (LB-26).
- "Every possible world exists" as a feature: oatmeal at the limit (LB-12).
- Story events that carry outcomes (relationship points, items), as storylets and Wildermyth events do: they would inject what the world should decide (SB-13).
- A murderer generator with calling cards instead of motives (MY-6).

## Order of dependence

- 1, 2 and 6 → 4 → (chunks: 7–12) → 23.
- 3 is independent and can go any time.
- 5 goes to the generator session whenever it suits it.
- 15 → 18.
- 20 waits for the Vulkan renderer.
- 21 and 22 wait for the planet work.
- 28 → 35; 24 → 30; 31 goes with 12; 32 goes with the chunk work or HY-59; 29 needs the event log with causes at macro scale.
- 37 can be written any time (a decision).
- 38 → 39 and 18.
- 40 and 41 go with the offices work (0058).
- 42, 43, 44 and 45 make the demo's street corner, after chunks (7–12).
- 46 → (47, 48) → 29; 47 needs 28; 49 goes with the chunk work (7–12); 50 after 46.
- 51 and 53 can start now (Tier 0 only); 52 → 51; 54 → 53; 55 → (56, 59); 57 → 58; 59 needs records and offices; 60 waits for the city generator.
- 62, 63, 64 and 66 can go any time after 52; 61 goes with interiors (spec 51) and 60; 65 waits for the city generator.
