# Perception-driven detail and the simulation running ahead of the view

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [status](../status.md) item 7 (the run-ahead proposal), [spec 27](../tier0/specs/27-detail-levels.md), [spec 28](../tier0/specs/28-pop-levels.md), [spec 29](../tier0/specs/29-embodied-pops.md), [spec 40](../tier0/specs/40-time.md), [spec 06](../tier0/specs/06-save-replay.md), [spec 18](../tier0/specs/18-noise-field.md), [engine architecture](../design/engine-architecture.md), [focus and compression](../design/focus-and-compression.md), [0041](../decisions/0041-threading-and-partition.md). **Evidence:** four web-research passes on 2026-10-09. Read first-hand: X-Ray (OpenXRay) source, the GGPO source and docs, the libretro docs, Forrest Smith's ChronoCam article, the Game AI Pro chapters by Sunshine-Hill, the AIIDE alibi paper, Booth's Left 4 Dead slides, the IVE paper, the WalkerSim 2 docs, the Outatime paper, the Carothers reverse-computation paper, Fujimoto's course slides, the ROSS and MATSim papers, the Nav2, Autoware and Waymo docs and papers. Not read: Sunshine-Hill's thesis (HTTP 403), Jefferson 1985 in full (read through a summary), the Overwatch, Rocket League, Braid and Days Gone talks (GDC Vault gated), the Photon Quantum docs (bot check), the two YouTube videos (bot check; titles only). **IDs:** RA.


The user's idea: like a self-driving car that builds its map from what it senses, the player is the car, and only its surroundings are computed when needed, with the simulation running ahead of what is shown. Our notes cover the first half well (DL, PP, LJ, CJ, PS). Nothing covered running ahead, local rollback, or how robots size what they compute. This note fills those gaps and says what the combination would be.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| RA-1 | S.T.A.L.K.E.R. (X-Ray) A-Life switches with two thresholds: online when distance ≤ switch_distance·(1 − switch_factor), offline when > switch_distance·(1 + switch_factor) | sourced | OpenXRay `src/xrGame/alife_switch_manager_inline.h`, `alife_dynamic_object.cpp` |
| RA-2 | Typical switch_factor is 0.1: a 150 m switch gives 135 m online and 165 m offline; mods use 450 m (GAMMA) and 750 m (Anomaly); offline time runs 6–10 game s per real s and offline animal speed is divided down | reported | Stalker GAMMA `alife.ltx`; ModDB Anomaly A-Life settings |
| RA-3 | Some object types can never go offline (space restrictors, breakables, climbables): permanent things are exempt from detail levels by type | sourced | OpenXRay `xrServer_Objects_ALife.cpp` |
| RA-4 | A squad goes online if any member is within the online distance and offline only when none is within the offline distance; offline, one "brain" updates and the commander's position is copied to every member | sourced | OpenXRay `alife_online_offline_group.cpp` |
| RA-5 | The offline-combat code in X-Ray 1.6 is commented out; only `kill_entity` runs. The dead code rolled detection, attacks, damage 0.5–1.5 × hit power and retreat | sourced | OpenXRay `alife_combat_manager.cpp` |
| RA-6 | X-Ray seeds the switch manager's RNG from the CPU timer, so offline outcomes are not reproducible | sourced | `alife_switch_manager_inline.h` |
| RA-7 | X-Ray walks objects round-robin, each switch-checked at most once per cycle, under a time budget (`process_time`, `objects_per_update`) | sourced | OpenXRay `alife_update_manager.cpp` |
| RA-8 | Iassenev (2008): offline is "the LoD of the online": no animation, sound or inventory, movement on a global graph, radius about 150 m; offline fights are turns by formula plus chance, ending in flight, death or avoidance | sourced | gamedeveloper.com, "Interview: Inside the AI of S.T.A.L.K.E.R." |
| RA-9 | Stalker 2 (2024): GSC said A-Life needs a much larger spawn area and much more memory; the area was shrunk for performance and the system broke | reported | insider-gaming.com, "What happened to A-Life 2.0" |
| RA-10 | Stalker 2 patch 1.1 fixed NPCs spawning behind the player, widened NPC visibility, removed an "aggressive optimization" that stopped NPCs existing offline, and fixed NPCs stuck in the online-offline transition | reported | shacknews.com, Stalker 2 patch 1.1 notes |
| RA-11 | Stalker 2 v1.1 as measured by a modder: spawn at about 60–80 m, despawn at about 100–120 m, config values ignored; patch 1.5 moved respawn farther away and made NPCs appear already animated and armed | reported | nexusmods Stalker 2 mod 273; insider-gaming 1.5 notes |
| RA-12 | LOD Trader: the risk of a "break in realism" is worked as x = −ln(1 − p) so independent risks add. Three kinds: unrealistic state, fundamental discontinuity (contradicts what the player remembers), unrealistic long-term behaviour. Risk = per-character criticality · per-level "audacity" | sourced | Sunshine-Hill, Game AI Pro ch. 14 |
| RA-13 | LOD Trader criticality: observability = min(pixels ÷ pixels of a character 4 m away, 1); attention is an EMA of 0.7·observability + 0.3·focus (95 % falloff in 1.5 s); memory learns 95 % in 5 s and forgets 50 % in 10 s; return time follows a Weibull hazard (k ≈ 0.8) | sourced | Game AI Pro ch. 14 |
| RA-14 | The trader buys the upgrade with the best risk reduction per cost until over budget, then sells downgrades back under budget; 57 µs per frame, 500 kB + 48 B per entity, hundreds of NPCs | sourced | Game AI Pro ch. 14 |
| RA-15 | "Existence" is one of the trader's features (yes/no) and replaces the simulation bubble; in multiplayer, criticality is summed over players | sourced | Game AI Pro ch. 14 |
| RA-16 | Alibi generation keeps only visible state and, when needed, samples the hidden state (origin, destination, reason) conditioned on what was seen, by Metropolis–Hastings; the alibi is needed only just before the two behaviours would diverge (e.g. at a sector exit) | sourced | Sunshine-Hill, Game AI Pro ch. 37 |
| RA-17 | When a sector comes into view: a Poisson draw per path; arrivals from unseen neighbours are exponential with rate = mean population ÷ traversal time | sourced | Game AI Pro ch. 37 |
| RA-18 | Alibi paper scale: 20,000 agents, 400 targets, 1,200 segments; 52 kB precomputed; a cell sampled in < 0.1 ms; an alibi is 500 MH iterations, about 9 ms; the exact steady state took "weeks" to solve | sourced | Sunshine-Hill and Badler, AIIDE 2010 |
| RA-19 | Industry survey in the same paper: Midtown Madness 2 recycled cars inside a radius; GTA IV used weighted random walks and removed occluded pedestrians; GTA IV's planned homes and jobs for every NPC was dropped | reported | AIIDE 2010; gtaboom.com |
| RA-20 | Left 4 Dead: wanderers are a count N per nav area; bodies are created when the area enters the active set and deleted (added back to N) when it leaves; N is zeroed when the area becomes visible; mobs of 20–30 every 90–180 s come from unseen areas, 75 % from behind | sourced | Booth, "The AI Systems of Left 4 Dead", 2009 |
| RA-21 | IVE (Šerý et al. 2006): a location tree cut by an "LOD membrane"; neighbours raise each other's detail so places expand before the player arrives; expansion is immediate, shrinking happens only in a periodic cleanup; a process cut mid-run is discarded | sourced | artemis.ms.mff.cuni.cz IVE-LOD-2006.pdf |
| RA-22 | Neverwinter Nights AI levels by heartbeat: very low with no player in the area (~10 s), low when a player is in the area but can't see the creature (~8 s), normal when visible (6–8 s), very high in combat (~6 s) | reported | nwnlexicon.com AI_LEVEL |
| RA-23 | Watch Dogs: Legion "Census": a citizen the player shows interest in is "uprezzed" (profile, relationships, 24 h schedule) and kept; off-screen citizens are not walked but placed along their route so the player can intercept them anywhere | reported | gamedeveloper.com, "How Watch Dogs: Legion's play-as-anyone simulation works" |
| RA-24 | Oblivion: dungeon NPCs do not process while the player is out of the cell, so they cannot loot first; Radiant AI was toned down because it broke quests | reported | UESP Oblivion fan interview III; gamesbeat.com |
| RA-25 | Ultima Online: creature AI was disabled over the cost of radial searches plus pathfinding; the closed economy failed because hoarded wool drained the resource bank | sourced | raphkoster.com |
| RA-26 | WalkerSim 2 (7 Days to Die mod) runs virtual zombies over the whole map: density 1–4,000 per km², activation radius 48–196 m (default 96), seeded and reproducible, boids plus wind, roads, POIs and a random walk; sound attracts them; despawned zombies keep type and health | sourced | 7dtd-walkersim2.readthedocs.io (2.0.5) |
| RA-27 | World War Z: the far swarm (> 5,000) is non-interactive GPU animation on pre-made tracks, separate from about 500 interactive zombies | reported | GPUOpen GDC 2019 "High zombie throughput" |
| RA-28 | Project Zomboid redistributes zombies every 12 h by default even with respawn off; respawn defaults are 72 h, 16 h unseen, 10 % | reported | pzfans.com guides |
| RA-29 | GGPO predicts a missing remote input by repeating the last one and records the first frame whose real input differs as the rollback point | sourced | github.com/pond3r/ggpo `input_queue.cpp` |
| RA-30 | GGPO caps speculation at 8 frames (133 ms at 60 fps) with a ring of 10 full saved states; past that it rejects local input and the game stalls | sourced | GGPO `sync.h`, `sync.cpp` |
| RA-31 | On a misprediction GGPO loads the saved state and re-runs every frame to the present inside one displayed frame, drawing nothing in between | sourced | GGPO README, `sync.cpp` |
| RA-32 | GGPO requires the whole game state in one saved block, RNG included, no wall-clock in logic, a fixed step; "SyncTest" forces a 1-frame rollback every frame and compares checksums | sourced | GGPO DeveloperGuide.md |
| RA-33 | Rollback length ≈ transit time − input delay; more than 1 frame of delay is noticeable in fighting games, 4–5 frames is tolerable in board and puzzle games | sourced | GGPO README, DeveloperGuide |
| RA-34 | Players change input about 5 times per second, so about 8 % of frames carry new input and repeating the last input is usually right; a wrong guess shows as characters jumping | sourced | Infil, "Netcode" part 4, words.infil.net |
| RA-35 | Photon Quantum predicts ahead of the last verified frame with a 60-frame rollback window by default; the verified frame only moves forward and is the truth | reported | Photon Quantum DeterministicSessionConfig (search summary) |
| RA-36 | Overwatch: 16 ms command frames, the client about ½ RTT + 1 frame ahead of the server; mispredicted movement is rolled back and inputs replayed; the server tells starved clients to simulate faster | reported | Ford, GDC 2017 (summaries) |
| RA-37 | Rocket League resimulates the whole physics world from the server state on a correction, at 120 Hz | reported | Cone, GDC 2018 (summaries) |
| RA-38 | RetroArch Run-Ahead: per displayed frame it runs hidden frames ahead and shows the last; cost about N + 1 frames plus one save and one load; cores without fast clean savestates cannot use it | sourced | docs.libretro.com/guides/runahead |
| RA-39 | RetroArch's second-instance mode runs a second core ahead and resyncs only when input changes; it never guesses input, it assumes the current input stays held | sourced | docs.libretro.com/guides/runahead |
| RA-40 | RetroArch 1.15 "preemptive frames" reruns only when the controller changes: 22 % faster than Run-Ahead in the published test | sourced | libretro.com 1.15.0 release notes |
| RA-41 | Planetary Annihilation ChronoCam: every server-to-client value is a curve of keyframes (linear, step, or pulse for one-off events and sounds), keyframed only on change; simulation at 10 Hz, clients interpolate to 60 fps | sourced | forrestthewoods.com, "The Tech of Planetary Annihilation: ChronoCam" |
| RA-42 | ChronoCam also sends predicted future keyframes ("predict things into the future as far as you can"); a wrong prediction costs one keyframe plus a removal | sourced | same |
| RA-43 | ChronoCam clients advance at 0.1×–3× and seek; any frame must be buildable from scratch; live spectating is a tape delay over buffered curves | sourced | same |
| RA-44 | Planetary Annihilation server memory grows with game time and interactions; "history is very compressible" | reported | palobby wiki, server performance |
| RA-45 | Braid rewinds 30–60 minutes of play within 40 MB on consoles, exactly | sourced | Blow, GDC "The Implementation of Rewind in Braid" (page) |
| RA-46 | Achron's designer says about 8 minutes of editable timeline works in multiplayer; changes travel forward on "time waves" faster than play; editing older events costs more | sourced | bit-tech.net Achron interview |
| RA-47 | Achron per fan wikis: editable from about 6 minutes back to 1 minute ahead; events past the back edge become final; the future view shows only what happens if units finish current orders | reported | tvtropes, giantbomb |
| RA-48 | A Factorio replay is a snapshot plus the input log, inside the save; no skipping ahead, no rewinding; mod or version changes break it | sourced | wiki.factorio.com/Replay_system |
| RA-49 | A player joining Factorio downloads the map, then resimulates every tick since the download started, so must run faster than 60 UPS | reported | forums.factorio.com 32384 |
| RA-50 | Outatime renders speculative frames one RTT ahead and hides up to 120 ms; movement predicted with a Markov chain plus Kalman filter; discrete actions run as parallel branches, at most 4 even at 256 ms RTT | sourced | Lee et al., MobiSys 2015 |
| RA-51 | Outatime checkpoints with page-level copy-on-write plus inverse operations: 0.076 ms per checkpoint vs 45.55 ms for object-only saving | sourced | same |
| RA-52 | Each Outatime branch costs a full normal execution; genres with many discrete actions (fighting, RTS) are a weak spot | sourced | same |
| RA-53 | Stadia's "negative latency" (2019) claimed to predict button presses; no evidence it shipped | reported | engadget.com, 2019-10-10 |
| RA-54 | Redis persistence: a base snapshot by fork and copy-on-write plus an append-only log; on large data the fork can pause up to about a second | sourced | redis.io persistence docs |
| RA-55 | Rewind buffers as base + deltas: about 20 MB per minute (RetroArch comment), about 20× less memory than full states (a C64 emulator) | reported | retropie forum 24729; vice-sharp wiki |
| RA-56 | Time Warp (Jefferson 1985): each process keeps its own virtual time; a late ("straggler") message rolls it back to a saved state, antimessages cancel what it sent after that time, and it re-executes | reported | Jefferson, ACM TOPLAS 7(3), 1985, via summary |
| RA-57 | Global virtual time (GVT) bounds every future rollback from below: older states are freed ("fossil collection") and irreversible output is released only after GVT passes it | reported | same |
| RA-58 | A parallel optimistic run equals sequential timestamp order only if event code is repeatable, simultaneous events are ordered the same every run, and interactive inputs get a logical time | sourced | Fujimoto, Lookahead slides (SIGSIM courseware) |
| RA-59 | ROSS keeps optimistic runs deterministic with tie-breaker values added to virtual time | reported | McGlohon and Carothers 2021, arXiv 2105.00069 (abstract) |
| RA-60 | Reverse computation undoes an event by inverse code, saving only control-flow bits; up to about 3× faster than copying state; a Mersenne Twister needs 2,496 B saved per event, L'Ecuyer's combined LCG none | sourced | Carothers, Perumalla, Fujimoto, ACM TOMACS 9(3), 1999 |
| RA-61 | Time Warp needs much more memory than sequential runs and is unbounded if a process runs far ahead; remedies are sparse or incremental state saving, moving time windows, and blocking runaway processes | sourced | Fujimoto, Memory Management slides |
| RA-62 | Cancelback lets Time Warp finish within sequential memory, but speedup needs several times that | reported | Jefferson, "Virtual Time II", PODC 1990 |
| RA-63 | Lazy cancellation (send antimessages only if re-execution gives different output) gains about 10 % typically | sourced | Fujimoto, Memory Management slides |
| RA-64 | Lazy cancellation loses when regenerated output usually differs; aggressive loses when it usually repeats | reported | Reiher, Fujimoto, Bellenot, Jefferson 1990 |
| RA-65 | ROSS on PHOLD: 12.26 billion committed events per second on 65,536 cores with rollback efficiency above 90 %; at 131,072 cores, "an avalanche of rollbacks" | sourced | Bauer, Carothers, Holder, PADS 2009 |
| RA-66 | In the same paper a wave-propagation model rolled back about as many events as it kept (efficiency 2.6 % and −4.3 %) | sourced | same |
| RA-67 | Conservative lookahead: a process at time T stamps every message T + L or later; L can come from the minimum time for one entity to affect another, or a top speed | sourced | Fujimoto, Lookahead slides |
| RA-68 | "Conservative algorithms live or die by their lookahead": zero-lookahead cycles livelock; small lookahead causes "time creep" | sourced | same |
| RA-69 | A time-stepped model has one step of lookahead implicitly, because events inside a step are independent | sourced | same |
| RA-70 | Chandy–Misra: messages only to neighbours, no shared variables, no central scheduler | reported | Chandy and Misra, IEEE TSE 1979 |
| RA-71 | Bounded lag: only nodes within the lag (from minimum propagation delays) can affect a node; reproducible for any lag; the rigid-disk variant assumes a top speed and falls back to a global checkpoint if it is broken | reported | Lubachevsky, CACM 1989; US Patent 4,901,260 |
| RA-72 | Distributed MATSim: each process waits only on its neighbours, one message per neighbour per step; 102× the single-process speed at 1,024 processes; communication dominates | sourced | Laudan, Heinrich, Nagel 2024 |
| RA-73 | A simulation is a space × time rectangle that can be cut into arbitrary regions, including cuts along time | reported | Chandy and Sherman, "Space-time and simulation", 1989 |
| RA-74 | HashLife returns a block's centre 2^(k−2) generations ahead with 13 memoized sub-calls; run time follows novelty | sourced | Erickson, HashLife notes; Gosper, Physica 10D, 1984 |
| RA-75 | HashLife is valid because information moves at most one cell per generation, so nothing outside a block reaches its centre in that time | reported | conwaylife.com wiki |
| RA-76 | HashLife ran a breeder to 10^12 generations in under a second, but is slower than stepping during warm-up | sourced | Rokicki, Dr. Dobb's 2006 |
| RA-77 | Self-adjusting computation re-executes only the affected part of a recorded dependence graph; large speedups are rare and it is often no faster than recomputing | reported | Acar et al., TOPLAS 2009; Hammer et al. (Adapton), PLDI 2014 |
| RA-78 | A minimal build system rebuilds only what depends on a changed input; "early cutoff" stops when a rebuilt output is unchanged | reported | Mokhov, Mitchell, Peyton Jones, ICFP 2018 |
| RA-79 | Factorio puts idle entities to sleep at zero cost | sourced | factorio.com FFF-148 |
| RA-80 | Folia gives each region its own tick counter, so a lagging region does not slow others; no rollback | sourced | docs.papermc.io/folia |
| RA-81 | EVE time dilation slows a whole node down to 10 %; CCP said a clock per solar system would need fundamental changes | reported | eveonline.com TiDi; CCP Veritas 2012 |
| RA-82 | Local lag + timewarp: correct means matching a "virtual perfect site" that runs every operation at its timestamp; 80–100 ms of lag is reported as unnoticeable | sourced | Mauve et al., IEEE Trans. Multimedia 6(1), 2004 |
| RA-83 | Game rollback rewinds the whole state (GGPO, Quake trailing state); we found no game that rolls back a region | reported | GGPO; Cronin et al., NetGames 2002 |
| RA-84 | Nav2 costmap defaults: update 5 Hz, publish 1 Hz, 0.1 m cells, 5 × 5 m | sourced | docs.nav2.org (Jazzy) costmap_2d |
| RA-85 | Nav2 example: a local costmap that rolls with the robot (3 × 3 m, 0.05 m, 5 Hz) and a global costmap at 1 Hz | sourced | same |
| RA-86 | Nav2 voxel example: marks obstacles to 2.5 m and clears free space to 3.0 m: clearing reaches farther than marking | sourced | same |
| RA-87 | Nav2 MPPI: 56 × 0.05 s = 2.8 s horizon, 1,000 sampled trajectories, 100+ Hz on a 4th-gen i5 | sourced | docs.nav2.org MPPI |
| RA-88 | If horizon × top speed exceeds the local costmap the robot is artificially limited (3 s at 0.5 m/s needs at least 1.5 m) | sourced | same |
| RA-89 | Autoware MPC: 50 × 0.1 s = 5 s horizon | sourced | autoware_universe lateral controller defaults |
| RA-90 | Baidu Apollo plans at least 8 s or 200 m ahead | reported | arXiv 1807.08048 |
| RA-91 | Apollo ranks obstacles "caution" or "normal"; caution vehicles get a learned 3 s prediction, extended to 8 s by a Kalman filter | sourced | arXiv 2006.06715 |
| RA-92 | Waymo Open Motion: 1 s of history, up to 8 agents predicted 8 s ahead | sourced | waymo.com open challenges; arXiv 2104.10133 |
| RA-93 | Waymo's "miss" thresholds widen with lookahead (lateral 1 m at 3 s, 1.8 m at 5 s, 3 m at 8 s) and scale with speed | reported | waymo.com 2021 motion challenge |
| RA-94 | Measured error grows faster than linearly: 0.61 m at 3 s, 1.32 m at 5 s, 2.63 m at 8 s; miss rate 11 % → 20 % | reported | arXiv 2504.15766 |
| RA-95 | MultiPath predicts a fixed set of trajectory anchors with a Gaussian mixture per step in one pass | sourced | arXiv 1910.05449 |
| RA-96 | Occupancy flow fields: a space-time grid of occupancy probability plus a flow vector per cell, including "speculative agents" hidden now that may appear | sourced | arXiv 2203.03875 |
| RA-97 | Waymo occupancy-flow challenge: 256 × 256 grid (about 80 × 80 m), 8 s ahead in 1 s steps, with a separate task for hidden vehicles | sourced | waymo.com 2022 occupancy-flow challenge |
| RA-98 | Tesla occupancy network: 8 cameras → one occupancy volume plus flow in about 10 ms; resolution spent "where it really matters" | reported | Elluswamy, CVPR 2022 keynote (transcript) |
| RA-99 | Waymo's prior map lets sensors "focus more on moving objects"; live data is compared with the map to detect changes and share them with the fleet | sourced | waymo.com blog 2016, 2020 |
| RA-100 | Mobileye REM: a crowdsourced map from about 10 KB per km uploaded by driver-assist cars | sourced | mobileye.com/technology/rem |
| RA-101 | Waymo ranks agents by their effect on the planner, using minimum time-to-collision and time to reach the car's path, not just distance; thousands ranked in about 0.5 ms; the top ones get expensive models | sourced | Refaat et al., IROS 2019, arXiv 1909.08792 |
| RA-102 | Prioritisation patents: the number of high-priority agents is set from compute available now; or cheap forecasts for all first, detailed ones with the time left | reported | US 11048253; US 12073244 |
| RA-103 | NHTSA judges forward-collision warnings by time-to-collision (about 2.0–2.4 s) | reported | NHTSA VRTC presentation |
| RA-104 | Marvel's Spider-Man: about 800 tiles of 128 m; tiles ahead load and tiles behind unload, about one tile per second at top swing speed | sourced | blog.playstation.com |
| RA-105 | Unreal World Partition: the player is a streaming source with a range; cells can be weighted by angle to the facing direction; "Block on Slow Streaming" stalls the game when loading falls behind | sourced | Epic World Partition docs |
| RA-106 | Horizon Zero Dawn places the world around the player on the GPU at runtime and requires placement to be "deterministic and locally stable" | reported | Guerrilla, "GPU-based procedural placement" |
| RA-107 | User's video Lr0t5inHar4 is "ASSURE Automatic Map Generation (AMG) APIs Experiment II" (Hatem Darweesh): tools that build an HD road map for autonomous vehicles | reported | YouTube oEmbed; title only |
| RA-108 | User's video nVJiMSXxpao is "VectorMap — HD map generation from camera BEV, semantic segmentation, and LiDAR SLAM" (Armando Genis): building a vector map from LiDAR and cameras | reported | YouTube oEmbed; title only |
| RA-109 | On our clock (tick 0.5 game s, 48 ticks per real second at 1×, spec 05), one real second ahead at 1× is 48 ticks = 24 game s; one game hour is 7,200 ticks; a game year about 63.1 M ticks | derived | spec 05 rules 3, table |
| RA-110 | Sound covers 171.5 m per tick, so an order's cone has two speeds: within a few ticks it fills the audible range of the loudest sound the order causes (spec 36 rule 6), then grows at the fastest mover (run 2.5 m per tick, vehicle about 15 m per tick) | derived | RA-67, RA-75; spec 36 rule 6 |
| RA-111 | If listeners answer what they hear with sounds of their own (moans, running, shots), the long-run cone grows at relay speed = audible range of the answer ÷ reaction delay (e.g. 50 m ÷ 2 s = 25 m/s): horde waves, not walking, set how far a rollback can reach | estimate | from RA-110 |
| RA-112 | If the noise field is recomputed from sources each tick instead of stored as state, a chunk with no listener above threshold is unchanged after a rollback and drops out (early cutoff): a quiet order dirties a few chunks, a gunshot dirties every chunk with a listener in range | estimate | from RA-63, RA-78 |
| RA-113 | Rolling back some (chunk, tick) cells inside the cone keeps one global tick; it is not the regional clocks 0041 rejected (RA-80, RA-81), because messages still land at their keyed tick and only how far each chunk has been computed is uneven | estimate | from RA-56, RA-58, RA-73 |
| RA-114 | Local rollback breaks if anything in the tick is global and mutable: an id counter shared by the world, a shared RNG stream, a world total. One extra zombie in a replayed chunk then shifts later ids everywhere | estimate | from RA-58, RA-59 |
| RA-115 | Where the hot and cold forms agree exactly (A1), the level a chunk runs at cannot change what happens, so warming the wrong chunks on a wrong guess about the player costs CPU only. Where agreement is A2, the hot set must come from simulation state (the body's simulated position, spec 27 rule 7), never from a prediction or the camera | estimate | spec 27 rules 6–7; 0061 rule 3 |
| RA-116 | Sunshine-Hill needs alibis and realism risks because his coarse model is not the truth; with `derive(pop_id, t)` as the truth (spec 28) there is nothing to fake, and an LOD Trader can only choose cost, never realism | estimate | from RA-12..RA-16 |
| RA-117 | Under direct control at fight 1:1 the 0.5 s tick is already coarser than human input (RA-34) and than unnoticeable lag (RA-82), so lookahead there should be zero or one tick; rollback serves watching and fast-forward, not the player's own hands | derived | RA-34, RA-82; 0056 caps |
| RA-118 | The hot ring must be at least (lookahead + promotion time) × top closing speed: 3 game s at 7 m/s player + 6 m/s zombie is about 40 m plus a margin | derived | RA-88, RA-104 |
| RA-119 | Recomputing ahead costs ticks × cost per tick: at 1 ms per world tick, one game hour is 7.2 s of CPU and one game year about 17.5 h; years ahead are possible only with cold jumps (MP-58) | derived | RA-109 |
| RA-121 | With orders arriving at rate r and a lead L, each order discards the L computed past it, so the wasted share of simulation work is r·L ÷ (1 + r·L); with local rollback touching a fraction f of the world, r·L·f ÷ (1 + r·L·f). Direct control at 1:1 (r ≈ 2 per game s, L = 0.5 s) wastes about 50 %; watching at 1× (one order per 240 game s, L = 24 game s) about 9 % | derived | RA-109, RA-117 |
| RA-122 | The lead should follow the measured order rate (short when orders are frequent, long when rare), down to zero under direct control, which is today's behaviour; RetroArch's rerun-only-on-input-change was 22 % faster than plain run-ahead (RA-40). The gain at high speed is spike absorption: top speed set by the average tick instead of the worst | estimate | RA-40, RA-121 |
| RA-120 | We found no game or paper that combines all three: running ahead with rollback, detail levels that never change results, and rollback bounded by a causal cone. Time Warp has rollback and cones without levels; ChronoCam has a future stream without rollback; X-Ray, Left 4 Dead and WalkerSim have levels that are not exact | estimate | absence in RA-1..RA-108 searches |

## 1. Answer in one page

**Simulating only near the player is solved in principle, and our specs already pick the better variants.** X-Ray, Left 4 Dead, IVE, Legion and WalkerSim all do it (RA-1..RA-26). Their failures are ones our rules already forbid:
- X-Ray seeds offline outcomes from the clock (RA-6) and shelved its offline combat (RA-5).
- Left 4 Dead empties an area when it becomes visible (RA-20), which breaks [0061](../decisions/0061-the-world-runs-without-the-player.md) rule 3.
- Stalker 2 shrank the bubble below what the player can perceive, so NPCs appeared behind the player (RA-9..RA-11).

**Running ahead is solved for small states and short leads.** GGPO, RetroArch, Photon, Overwatch and Rocket League all keep full saved states and re-run on a wrong guess (RA-29..RA-40). ChronoCam shows how the view should consume it: a timeline of curves and pulse events, never the live state (RA-41..RA-43). Time Warp gives the vocabulary (RA-56, RA-57):
- the view tick plays the role of GVT;
- nothing at or before it can roll back;
- alerts are output released when playback passes them.

**Rolling back only part of the world is possible and deterministic** (RA-56, RA-58, RA-73). It keeps one global tick (RA-113). It needs:
- no global mutable state inside a tick (RA-114);
- a cone to bound it (RA-110, RA-111);
- early cutoff so quiet orders stay cheap (RA-112).

We found no game that does it (RA-83).

**The robot analogy gives three sizing rules:**
- Size the bubble by lookahead × closing speed (RA-88, RA-118).
- Rank by time-to-contact, not distance (RA-101).
- Uncertainty grows faster than linearly with lookahead (RA-94): far futures are cheap guesses, near futures are exact.

The two videos are about building the car's prior map (RA-107, RA-108). That map is our cold layer: the persistent, cheap summary the car keeps from what it has sensed (RA-99, RA-100).

## 2. What we take, what we don't

| From | Take | Don't take |
| --- | --- | --- |
| X-Ray A-Life | Two thresholds (±10 %) for the hot ring, which fills the gap LJ-33 left in spec 27; whole groups switch together (RA-4); types that never go cold (doors, barricades) (RA-3) | Clock-seeded RNG; offline fights by ad-hoc formula |
| Left 4 Dead | Count per area, bodies made and returned to the count (RA-20) = spec 27 rule 5 | Zeroing an area when seen (breaks 0061) |
| IVE | Expand now, shrink lazily in a cleanup pass (RA-21) | — |
| Sunshine-Hill | His observability, memory and return-time terms as a *demotion delay* heuristic (RA-13); budget trading for optional extra hot ticks (RA-14) | Alibis: we don't need them (RA-116) |
| WalkerSim 2 | Closest working horde precedent: seeded, map-wide, sound-driven, hands bodies back intact (RA-26) | Boids per tick at map scale, until measured |
| GGPO / RetroArch | Default guess "no new orders"; a cap on how far ahead (RA-30); SyncTest as a debug mode (RA-32) | Visible corrections (we never show the unconfirmed future) |
| ChronoCam | The view reads curves and pulse events from the lookahead; seek and scrub for free (RA-41..RA-43) | — |
| Time Warp | View tick = GVT; free snapshots older than it; release alerts after it (RA-57) | Unbounded run-ahead (RA-61, RA-65) |
| Outatime | Copy-on-write snapshots (RA-51) | Parallel futures per possible input (RA-52), except perhaps one "with / without this order" preview |
| Robots | Bubble from lookahead × speed; rank by time-to-contact; occupancy-plus-flow for hidden hordes (RA-96) | — |

## 3. The combination: what would be new

Each part exists somewhere; the combination does not (RA-120). In one line: **wrong guesses cost time, never truth.**

1. **One timeline, computed ahead.** The simulation runs ahead of the view and the view plays back a recorded stream. The committed tick is the view tick. An order arrives at the view tick and invalidates only the future, which the player has never seen. Nothing visible is ever corrected, unlike GGPO.
2. **Detail that never changes outcomes.** Cold jumps are exact and draws are keyed (spec 27, CJ), so the lookahead can warm chunks by *predicted* player paths (time-to-contact, heading) without risk where agreement is A1 (RA-115). A wrong prediction costs CPU. The hot set itself still follows the body's simulated position.
3. **Rollback bounded by a causal cone, settled by early cutoff.** Sound and walkers have top speeds. The cone is the audible disc of the order's loudest sound, then growth at mover or relay speed (RA-110, RA-111). Inside it, only chunks whose inputs actually changed are recomputed (RA-112). A quiet order touches a few chunks; a gunshot touches every listener in range. That cost is the game's own rule, "the generator, not the shout, brings the horde".
4. **The future is a ChronoCam stream, never shown early.** Alerts are pulse events released at playback time. A cancelled future simply never plays.

Why it matters for the game:
- watching and fast-forward become smooth (the future is already there);
- late starts and servers get catch-up from the same machinery (RA-48, RA-49);
- rewind and replay of what *happened* come almost free (keyframes + input log, RA-45, RA-54);
- per-tick hash comparison between full re-simulation and local rollback becomes the test that keeps it honest (RA-32).

## 4. Numbers on our clock

| Quantity | Value | From |
| --- | --- | --- |
| Ticks per real second at 1× | 48 (24 game s) | spec 05 |
| Sound per tick | 171.5 m | RA-110 |
| Run, vehicle per tick | 2.5 m, about 15 m | RA-110 |
| Hot ring for 3 game s lookahead | about 40 m + margin | RA-118 |
| CPU for one game hour ahead at 1 ms per tick | 7.2 s | RA-119 |
| CPU for one game year ahead at 1 ms per tick | about 17.5 h; cold jumps only | RA-119 |
| GGPO cap for comparison | 8 frames, 10 full states | RA-30 |
| Achron editable window | about 6 minutes back, 1 ahead | RA-47 |

## 5. Where it lands (proposals for the run-ahead decision, not yet taken)

1. **Commit point = view tick.** The lookahead is speculative; saves hold the committed state plus the input log, never the lookahead (RA-48, RA-54).
2. **The view reads a stream** of curves and pulse events written by the lookahead; alerts fire at playback (RA-41, RA-57).
3. **Default guess: no new orders.** Cap the lead by a memory window; under direct control lead is 0–1 tick (RA-30, RA-117).
4. **Rollback in two steps.** First the whole lookahead (simple, two instances like RA-39). Later local rollback by cone and early cutoff, checked in debug against full re-simulation by per-tick hash (RA-32, RA-112).
5. **Detail.** The hot set comes from simulation state. Prediction may only warm A1 rules or preload data. Two thresholds; groups switch as one; never-cold types (RA-1, RA-3, RA-4, RA-115).

## 6. Open questions

1. **Ids.** [0041](../decisions/0041-threading-and-partition.md) says ids are monotonic and never recycled. With one global counter, local rollback breaks (RA-114). Mint ids per chunk, e.g. (chunk, tick, seq)?
2. **Relay speed.** Do Tier 0 walkers make sounds that other walkers hear? If so, measure the relay speed in RA-111; it sets the cone.
3. **Noise as state.** Is the block noise field in `engine/sim/src/noise.cpp` stored between ticks or recomputed from sources? RA-112 needs the second.
4. **How far ahead** at each speed (1×, 10×, rest-until-event), and the snapshot memory budget.
5. **Hysteresis numbers** for spec 27 (X-Ray ±10 % or IVE's lazy shrink).
6. **Hidden hordes** in cold chunks: a count per chunk, or occupancy plus flow with "speculative" zombies (RA-96)?

## 7. Sources

Perception and detail:
- OpenXRay: github.com/OpenXRay/xray-16
- Stalker GAMMA: github.com/Grokitach/Stalker_GAMMA
- gamedeveloper.com, "Interview: Inside the AI of S.T.A.L.K.E.R."
- insider-gaming.com, "What happened to A-Life 2.0" and 1.5 notes
- shacknews.com, Stalker 2 patch 1.1
- Sunshine-Hill, Game AI Pro ch. 14 and 37: gameaipro.com
- Sunshine-Hill and Badler, AIIDE 2010: ojs.aaai.org/index.php/AIIDE/article/view/12389
- Booth 2009: steamcdn-a.akamaihd.net/apps/valve/2009/ai_systems_of_l4d_mike_booth.pdf
- IVE: artemis.ms.mff.cuni.cz/main/papers/IVE-LOD-2006.pdf
- nwnlexicon.com/AI_LEVEL
- gamedeveloper.com on Watch Dogs: Legion
- app.uesp.net Oblivion fan interview III
- raphkoster.com
- WalkerSim 2: 7dtd-walkersim2.readthedocs.io
- GPUOpen GDC 2019
- pzfans.com

Running ahead:
- GGPO: github.com/pond3r/ggpo
- words.infil.net/w02-netcode-p4.html
- Photon Quantum: doc-api.photonengine.com
- GDC Vault 1024001 (Overwatch)
- docs.libretro.com/guides/runahead
- libretro.com 1.15.0
- forrestthewoods.com, ChronoCam
- palobby wiki
- GDC Vault 1012210 (Braid)
- bit-tech.net Achron interview
- wiki.factorio.com/Replay_system
- forums.factorio.com 32384
- Lee et al., Outatime, MobiSys 2015, DOI 10.1145/2742647.2742656
- engadget 2019-10-10
- redis.io persistence docs

Rollback and causality:
- Jefferson 1985, ACM TOPLAS 7(3) (via blog.acolyer.org/2015/08/20/virtual-time)
- Fujimoto courseware: sigsim.acm.org/conf/mskr/Courseware/Fujimoto
- Carothers, Perumalla, Fujimoto 1999
- Bauer, Carothers, Holder, PADS 2009
- McGlohon and Carothers 2021
- Jefferson PODC 1990
- Reiher et al. 1990
- Chandy and Misra 1979
- Lubachevsky 1989
- Laudan, Heinrich, Nagel 2024
- Chandy and Sherman 1989
- Erickson HashLife notes
- Rokicki 2006
- Acar et al. 2009
- Hammer et al. 2014
- Mokhov et al. 2018
- factorio.com FFF-148
- docs.papermc.io/folia
- eveonline.com TiDi
- Mauve et al. 2004
- Cronin et al. 2002

Robots and streaming:
- docs.nav2.org (Jazzy)
- autoware_universe
- arXiv 1807.08048, 2006.06715, 2104.10133, 2504.15766, 1910.05449, 2203.03875, 1909.08792
- waymo.com open challenges and blog
- mobileye.com/technology/rem
- US 11048253, US 12073244
- NHTSA VRTC
- blog.playstation.com (Spider-Man)
- Epic World Partition docs
- guerrilla-games.com
