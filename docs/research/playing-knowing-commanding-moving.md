# Playing, knowing, commanding and moving: player modes, belief on the map, leaders and orders, city traffic

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [0062](../decisions/0062-the-game-loop-attention-possession-and-late-starts.md), [0058](../decisions/0058-offices-open-layers-single-player-is-multiplayer-with-ai.md), [0044](../decisions/0044-information-scope-per-layer.md), [0060](../decisions/0060-one-fight-three-views-behaviour-from-jobs-and-skills.md), [0019](../decisions/0019-activities-and-delegation.md), [spec 39](../tier0/specs/39-orders-and-belief.md), [spec 16](../tier0/specs/16-view-2d.md), [spec 33](../tier0/specs/33-establishments-and-chains.md), [spec 27](../tier0/specs/27-detail-levels.md), [cameras and modes](../design/cameras-modes.md), [zoom continuum](../design/zoom-continuum.md), [macro layer](../design/macro-layer.md), [world vehicles and environment](../design/world-vehicles-environment.md), [demo-flow](../design/demo-flow.md). **Evidence:** four web-research passes on 2026-10-09. Read first-hand:
- **SS13 and SS14:** the Goonstation and Paradise wikis; SS14 admin and access docs.
- **Game docs and wikis:** the CK3, EU4 and Factorio wikis; Egosoft's X4 pages; txAdmin docs; Forrest Smith on ChronoCam; the Berlin Interpretation; van Wijk and Nuij via Reach and North; Google Earth help.
- **Intelligence and uncertainty:** ICD 203, Kent 1964, the UK PHIA yardstick, Kelly et al. 2025, MacEachren et al. 2012, Padilla et al. 2017, the NHC cone page, the War in the East 2 manual, the Command dev blog, Google Maps help.
- **Military doctrine:** FM 3-19.4 (2002), ADRP 6-0 (2012 and its 2014 glossary), Third Army SOP 1944, CALL January 2026.
- **Traffic and travel:** Treiber et al. 2000 and the book errata, Treiber and Kanagaraj 2015, SUMO docs, the MOBIL page, the Cities: Skylines II traffic pages, the Watch Dogs: Legion tools article, ATUS 2023, NHTS 2022, UK NTS 2022, MATSim source, Shipt's pay page, the Cities: Skylines info-views wiki.

Not read: AJP-2.1 and STANAG 2511 (restricted), MIL-STD-2525D's suspect rule, ADP 6-0 (2019) itself, Fritz 1961 and Stallings and Quarantelli 1985 in full, the official ETS2 pay formula, GTA V's spawn radii. **IDs:** PKC.


Four questions from the user's last ideas:
- **Playing:** how to step out of a character, watch as a god or admin, and come back. Decided as [0066](../decisions/0066-player-states-embodied-detached-observer.md).
- **Knowing:** how to show what a pop believes rather than the truth.
- **Commanding:**
  - how leaders and offices emerge after a collapse;
  - how an order can be traced from a soldier up to the event that caused it.
- **Moving:** the traffic, schedules and delivery contracts that the [demo-flow](../design/demo-flow.md) street corner needs.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| PKC-1 | Goonstation SS13 ghosts fly freely and can watch any player's screen; they talk only in dead chat; living players nearby rarely hear them; seeing antagonists requires giving up respawn for the round | sourced | wiki.ss13.co/Ghost |
| PKC-2 | Paradise SS13 ghosts orbit, jump to and teleport to any mob, see in the dark and use one HUD; using the antagonist HUD after minute 5, or ghosting while alive, removes respawn; observe but "not take part" | sourced | paradisestation.org/wiki/Ghost |
| PKC-3 | SS14 ghosts see the whole map and may claim ghost roles, but the new character has no memory of the old; using dead information is metagaming | reported | wiki.spacestation14.com/wiki/Ghost |
| PKC-4 | SS14 admins: `aghost` is an admin ghost with all access that can return to its body; "Control Mob" takes any entity; admin logs record every interaction, filterable per entity | sourced | docs.spacestation14.com admin tooling; paradisestation.org Player Panel |
| PKC-5 | SS13 jobs come from preferences High/Medium/Low/Never, filled in that order after antagonists; leftovers become Assistant | sourced | paradisestation.org Job Selection |
| PKC-6 | SS14 access: a reader lists the access needed to use an entity; any matching group grants it, deny tags veto; ID cards carry editable access | sourced | SS14 AccessReaderComponent API |
| PKC-7 | SS13 launched in 2003; players without a role are civilians with no department access; the plot is player-driven | reported | Wikipedia "Space Station 13" |
| PKC-8 | GTA V's switch: the camera rises, zooms out up to 3 steps, pans across the map, zooms in, then plays a scene of the new character; close characters cut directly; switching is blocked while wanted or in a mission | reported | gta.fandom.com Switch Scenes |
| PKC-9 | GTA V's leads have "established lives"; the switch scenes are scripted per character and story stage | reported | Polygon interview with Dan Houser, 2013 |
| PKC-10 | Crusader Kings III: after death you play your heir; switching ruler by hand, the console or easier rules disable that save's achievements | sourced | CK3 wiki, Succession and Achievements |
| PKC-11 | Dwarf Fortress: adventure, fortress and legends share one world; retired forts and adventurers keep living and can be resumed | reported | DF wiki, Adventurer mode |
| PKC-12 | X4: fly any ship from its cockpit or order it remotely from the map; teleport unlocks by research in steps (sector → 3 jumps → 5 → all property) | sourced | egosoft.com X4 info; X4 wiki research |
| PKC-13 | X4: sitting in a crewed ship's pilot chair displaces the pilot, who resumes the previous orders when you leave | reported | Egosoft feature text; Steam threads |
| PKC-14 | Bannerlord companions lead their own parties; disbanded parties walk to the nearest settlement | reported | primagames.com Bannerlord guide |
| PKC-15 | The Sims 4: the controlled Sim may have autonomy off; the Sim you leave regains free will; Neighborhood Stories changes unplayed households | reported | EA forums; gamerevolution.com |
| PKC-16 | State of Decay 2: switch survivors only at home base; the one left behind rests and recovers | reported | tomsguide.com |
| PKC-17 | State of Decay (2013): the world kept running while the game was closed, with no option to turn it off | reported | Steam discussions |
| PKC-18 | Rust: a logged-off character stays as a "sleeper" others can kill and loot, added so players could not log out of fights; Rust+ sends phone alerts when your base is raided | reported | rust.facepunch.com companion app update |
| PKC-19 | ARK: when you log off "your body becomes unconscious" and others can rob, kill, feed or jail it | reported | Kotaku, 2015 |
| PKC-20 | EVE: a logged-off ship stays about 60 s, or 15 min after fighting players, so nobody escapes a fight by logging off | reported | EVE University wiki, Timers |
| PKC-21 | Project Zomboid build 42 has five access levels (admin, moderator, overseer, gm, observer) and separate toggles for invisible, godmode, noclip and teleport | reported | winternode.com PZ admin commands |
| PKC-22 | FiveM's txAdmin splits admin powers into 26 permissions (spectate, noclip/god, teleport…) plus a root "all" | sourced | docs.fivem.net txAdmin permissions |
| PKC-23 | DayZ admin free cameras hide the admin's body; server owners complain that permissions are all or nothing | reported | Steam Workshop changelog |
| PKC-24 | Dota 2 spectating: follow a hero, see a player's own view, an auto-director camera, a free camera, and a choice of whose fog of war to see | reported | liquipedia.net Spectating |
| PKC-25 | Counter-Strike broadcasts run about 1:45 late so teams cannot scout through them; an auto-director can be taken over and handed back | reported | Valve SourceTV wiki |
| PKC-26 | Supreme Commander (2007): smooth zoom from one unit to an 80 × 80 km map; units become icons, crowds blur into a mass; split screen across monitors; camera lock on a unit | reported | Wikipedia "Supreme Commander" |
| PKC-27 | Planetary Annihilation: the picture-in-picture is a full, usable view of another planet, swappable with the main view, off by default because it costs performance | reported | GameSpot, 2014 |
| PKC-28 | ChronoCam stores every property as a timeline the player can scrub; units a player cannot see get no updates, which blocks map hacks | sourced | forrestthewoods.com ChronoCam |
| PKC-29 | Cities: Skylines II lets you follow citizens; their life journal starts recording only from that moment | reported | Paradox CS2 features |
| PKC-30 | EU4 Ironman: one save plus backup, autosave every 3 months and on quit, no console, no loading during play; achievements need Ironman and unmodified files | sourced | EU4 wiki, Ironman |
| PKC-31 | XCOM 2012 stored the random seed in the save, so reloading gave the same result; the expansion added an option to reseed on load | reported | gamebanshee.com, 2013 |
| PKC-32 | Factorio: console commands disable achievements with a warning; modded games keep separate achievements | sourced | Factorio wiki, Achievements |
| PKC-33 | Minecraft Bedrock: turning cheats on blocks a world's achievements permanently, even if turned off again | reported | minecraft.wiki Commands |
| PKC-34 | The Berlin Interpretation defines roguelike permadeath as "the savefile is deleted upon loading" | sourced | roguebasin.com |
| PKC-35 | The best camera path between distant views is zoom out, pan, zoom in (van Wijk and Nuij 2003), kept smooth when the target changes mid-move (Reach and North 2018); Google Earth has north-up and top-down reset keys but no back history | sourced | arXiv 1801.09358; Google Earth help |
| PKC-36 | Admiralty/NATO grading: source reliability A (completely reliable) to E (unreliable), F cannot be judged; information credibility 1 (confirmed by others) to 5 (improbable), 6 cannot be judged | reported | Wikipedia "Admiralty code" |
| PKC-37 | The two axes should be independent, but encoders mostly give diagonal codes (A1, B2, C3), and trust follows the source more than the content: A5 rated above E1 | sourced | Kelly, Budescu, Dhami, Mandel 2025, JDM 20 |
| PKC-38 | MIL-STD-2525 prints the grade on the map symbol as modifier J, "Evaluation Rating" (e.g. "B2") | reported | ArcGIS 10.2 military symbology help |
| PKC-39 | ICD 203 likelihood words: almost no chance 1–5 %, very unlikely 5–20, unlikely 20–45, roughly even 45–55, likely 55–80, very likely 80–95, almost certain 95–99 | sourced | ICD 203 |
| PKC-40 | ICD 203: confidence must not appear in the same sentence as likelihood; sources should be described by age and possible deception; information kept separate from judgment | sourced | ICD 203 |
| PKC-41 | Kent 1964: "probable" 75 % (±12), "chances about even" 50 % (±10), "probably not" 30 % (±10) | sourced | Kent, "Words of Estimative Probability" |
| PKC-42 | Kent's origin: one phrase approved by a board was read from 20:80 to 80:20 by its own members | sourced | same |
| PKC-43 | UK PHIA yardstick (2025): seven terms with deliberate gaps; confidence rated separately High/Moderate/Low | sourced | gov.uk PHIA |
| PKC-44 | NATO doctrine draws the bands differently: "even chance" 40–60 %, "likely" 60–90 % | reported | NATO STO-MP-SAS-OCS-ORA-2019 |
| PKC-45 | Rounding 888,328 forecasts into verbal bins lost accuracy, most for the best forecasters | reported | Friedman et al. 2018, ISQ |
| PKC-46 | The public reads "likely" as 55–90 % and "real possibility" as 20–80 % (middle 90 % of about 1,700 people) | reported | jasoncollins.blog survey |
| PKC-47 | APP-6/2525: "present" is a solid frame, "planned, anticipated or suspected" a dashed frame | reported | Wikipedia "NATO Joint Military Symbology" |
| PKC-48 | Affiliations (pending, unknown, assumed friend, friend, neutral, suspect, hostile) are carried by frame shape as well as colour, so they read in monochrome | reported | same |
| PKC-49 | APP-6 "area of uncertainty": a circle or ellipse where the object most likely is, sized from the last report and the sensor's accuracy | reported | Wikipedia; Luciad docs |
| PKC-50 | APP-6 "dead reckoning trailer": a dotted line or circle where the object should be now from its last course and speed | reported | Wikipedia |
| PKC-51 | Link 16 grades track quality 0–15 by position error; the full covariance is sent only on request | reported | US 8,052,054 |
| PKC-52 | The common operational picture is a "single identical display shared by more than one command"; NATO authors prefer "consistent" to "common" | reported | Wikipedia "Common operational picture" |
| PKC-53 | Search theory's farthest-on circle: radius = top speed × time since the last fix; detection payoff falls roughly as 1/t² | reported | Washburn, NPS |
| PKC-54 | Search and rescue plans drift error at 1/8–1/3 of the drift distance; the area grows with delay | reported | AMSA 2018; NOAA |
| PKC-55 | The NHC cone is circles sized so two-thirds of past forecast errors fall inside | sourced | nhc.noaa.gov/aboutcone |
| PKC-56 | 70 % of cone viewers believed the hurricane grows over time, against 39 % of ensemble viewers | sourced | Padilla, Ruginski, Creem-Regehr 2017 |
| PKC-57 | Ensemble lines bias viewers toward places on a drawn line (304 of 688 choices against 12 off the lines) | sourced | same |
| PKC-58 | MacEachren 2012: the best visual channels for uncertainty are fuzziness, location and value; size and transparency acceptable; saturation, hue, orientation and shape rejected | sourced | MacEachren et al. 2012, TVCG |
| PKC-59 | Iconic symbols were rated slightly more intuitive than abstract ones but took longer to read | sourced | same |
| PKC-60 | Hypothetical outcome plots (animated draws) beat error bars for untrained viewers comparing a few quantities | reported | Hullman et al. 2015 |
| PKC-61 | Google Maps: "you could be anywhere within the light blue circle"; a grey dot means no fix and shows the last location | sourced | Google Maps help 2839911 |
| PKC-62 | Radar used long-afterglow phosphor so contacts stayed readable after the sweep; slow decay leaves trails | reported | US 3,114,907; NEETS 18 |
| PKC-63 | Splinter Cell: Conviction's last-known-position ghost is where enemies converge and fire; players used it as a decoy | reported | Wikipedia; The Register 2010 |
| PKC-64 | Frozen Synapse "Dark" shows last known position and time since seen, and lets players give ghost enemies hypothetical orders | reported | Wikipedia "Frozen Synapse" |
| PKC-65 | War in the East 2: detection 0–10; at 1 a blank counter, 3 type, 5 name and strength, 7 soft factors; strength error grows as detection falls; detection decays each turn | sourced | WitE2 manual §10 |
| PKC-66 | Hearts of Iron IV splits intel into civilian, army, navy and air with thresholds that reveal detail | reported | Paradox dev diary (search excerpts) |
| PKC-67 | Shadow Empire's recon gates what you see; players report 3–4 visible units turning into about 20, and 700 estimated troops being 2,400 | reported | Steam discussions; wargamer.fr |
| PKC-68 | Dwarf Fortress artifact knowledge has six tiers from holding it to no idea; knowledge fades over weeks to years and updates merge by timestamp | reported | DF wiki, Rumor |
| PKC-69 | Command dev blog: enemy range rings are dashed and come only from the player's sensors; an unidentified radar gets no weapon ring until identified | sourced | command.matrixgames.com |
| PKC-70 | Highfleet picks up radar by intercept at about twice radar range and lets players mark suspected moves; EVE's directional scan gives ship types, not pilots or positions | reported | Highfleet guide; EVE University wiki |
| PKC-71 | Argentina 2002: 329 neighbourhood assemblies nationwide at the peak (122 in Buenos Aires City); 275 counted in December 2002 | reported | Dialnet; Página/12 2002-12-22 |
| PKC-72 | The assemblies had no formal leaders; meetings grew from 50–100 to 200–300 and settled near 50; the citywide Interbarrial limited votes to one rotating delegate per assembly in April 2002, then collapsed when parties packed the votes | reported | Página/12 2002-04-29, 2003-01-20 |
| PKC-73 | Barter clubs: founded 1995 by 3 neighbours with about 20 people; 2002 peak of about 4,500–6,000 nodes and 2.5 million people; collapsed late 2002–2003 from over-issued and counterfeit credits | reported | Wikipedia "Crédito"; Cambridge (Argentina's parallel currency) |
| PKC-74 | Piqueteros controlled 100,000–150,000 of about 2 million Plan Jefes y Jefas plans and wrote the recipient lists: leaders gained power by brokering aid | reported | Svampa; Tiempo Argentino |
| PKC-75 | Katrina: Common Ground was founded 5 September 2005 with about $50; a street-medic clinic opened within days | reported | commongroundrelief.org |
| PKC-76 | Algiers Point: a dozen to 30 armed residents barricaded streets; at least 11 shootings, uninvestigated until 2008 | reported | ProPublica |
| PKC-77 | The DRC's Katrina field study found improvised prosocial behaviour dominated, not the media's anarchy | reported | Rodríguez, Trainor, Quarantelli 2006, Annals AAPSS 604 |
| PKC-78 | "Elite panic": elites fear disorder and threats to their legitimacy and use force on rumour; disaster makes a "community of sufferers" (Fritz) | reported | Clarke and Chess 2008; Fritz 1961 via Solnit |
| PKC-79 | DRC typology: established, expanding, extending and emergent organisations (routine or new tasks × old or new structures) | reported | Sutton (Colorado hazards archive) |
| PKC-80 | Emergent groups form when authority is unclear, collapsed or seen to fail, or when tasks are new | reported | Stallings and Quarantelli 1985 |
| PKC-81 | Sarajevo 1992: criminal networks held the line before a formal army existed and profited from trading with the enemy | reported | Andreas 2008 (Wilson Center) |
| PKC-82 | Sarajevo 1993: gangsters were given commands; on 26 October 1993 the state moved against rogue brigades (Operation Trebević), with deaths on both sides | reported | Wikipedia (Topalović, Prazina) |
| PKC-83 | The Sarajevo tunnel (about 800 m) moved food, money, arms and wounded; city water fell from about 2,500 to 5 l/s | reported | Wikipedia; FAMA collection |
| PKC-84 | Olson: a stationary bandit turns theft into taxes and gains an interest in order; a short-tenure ruler confiscates | reported | Olson 1993, APSR 87 |
| PKC-85 | Arjona: rebelocracy (the armed group rules), aliocracy (locals rule and pay tribute) or disorder; strong prior institutions limit armed rule | reported | Arjona 2016 (Northwestern IPR) |
| PKC-86 | Weber's legitimacy: legal-rational, traditional, charismatic; charisma must be passed by designation, heredity or office, or the group splits | reported | Weber via Cambridge |
| PKC-87 | Bannerlord clan tiers by renown: 50, 150, 350, 900 (found a kingdom), 2,350, 6,150 | reported | Mount & Blade wiki, Clan |
| PKC-88 | Crusader Kings III: war needs a casus belli paid in prestige or piety (county 100, independence 500, empire claim 3,000) | reported | magicgameworld.com |
| PKC-89 | Hearts of Iron IV: war goals take time to justify and alert the target; a field marshal commands 5 (7) generals, a general 24 (30) divisions | reported | guildorder.com |
| PKC-90 | SS13 chain of command: Captain over equal heads; the Head of Personnel acts as Captain if the Captain is gone, then Security | reported | paradisestation.org Chain of Command |
| PKC-91 | Suzerain: a 10-member cabinet mostly appointed by the president, but the General Staff can impose the defence minister; Football Manager gates top jobs by reputation and licences | reported | suzerain.wiki.gg; Wikipedia UEFA licences |
| PKC-92 | Dwarf Fortress legends export can reach about 1 GB; RimWorld's Cassandra runs about 8.5 major threats a year | reported | DF wiki; RimWorld wiki |
| PKC-93 | The operation order has five paragraphs (situation, mission, execution, sustainment, command and signal); warning orders give notice, fragmentary orders carry changes; the mission states who, what, when, where, why | reported | Wikipedia "Operations order"; STANAG 2014 |
| PKC-94 | FM 3-19.4 paragraph 1b: "Include the mission, the commander's intent, and the concept of operations for HQ one and two levels up"; warning orders go "down to the squad" | sourced | FM 3-19.4 (2002), App. D |
| PKC-95 | ADRP 6-0 (2012): intent is typically 3–5 sentences; "Soldiers two echelons down must easily remember and clearly understand the commander's intent" | sourced | ADRP 6-0, para 2-14 |
| PKC-96 | Intent is "a clear and concise expression of the purpose of the operation and the desired military end state", letting subordinates act "without further orders" | sourced | ADRP 6-0 C2 (2014) glossary |
| PKC-97 | Mission command descends from German Auftragstaktik: officers did what the situation required as they saw it | sourced | ADRP 6-0 |
| PKC-98 | ADP 6-0 (2019): mission command "empowers subordinate decision making and decentralizes execution" | reported | army.mil article 225414 |
| PKC-99 | Planning rule: one-third of the time for the commander's planning, two-thirds left to subordinates; often ignored in practice | sourced | FM 3-19.4 para 2-17 |
| PKC-100 | "Commanders can effectively command and effectively control two to five subordinate headquarters" | sourced | ADRP 6-0 para 2-101 |
| PKC-101 | Typical echelons: squad 9; platoon 30–40; company 100–200; battalion 300–1,000; brigade 3,000–5,000; division 10,000–15,000; corps 20,000–45,000+ | reported | Wikipedia "Squad"; USIP |
| PKC-102 | An army of 100,000 is about 1–2 corps, 6–8 divisions, 20–25 brigades, 70–100 battalions, 300–400 companies, about 1,000 platoons and 3,500 squads: 8–9 echelons | derived | PKC-100, PKC-101 |
| PKC-103 | Third Army 1944: a division "should have twelve hours and better eighteen hours" between receiving and executing an order; oral orders repeated back | sourced | Third Army SOP 1944 |
| PKC-104 | 1914: German supreme headquarters' messages to armies 250 km away were often delayed up to 24 hours | reported | german1914.com |
| PKC-105 | A modern division plans in about 2 days, gives targeting 72–96 h ahead, issues fragmentary orders daily and plans in 2-hour blocks | sourced | CALL, January 2026 |
| PKC-106 | Moltke: no plan survives first contact with the enemy's main force with certainty; Clausewitz: "the simplest thing is difficult" | reported | Quote Investigator; Clausewitz Book I ch. 7 |
| PKC-107 | The Intelligent Driver Model's original calibration: T = 1.6 s, a = 0.73, b = 1.67 m/s², s0 = 2 m; outflow from a jam about 1,689 vehicles per hour | sourced | Treiber, Hennecke, Helbing 2000, PRE 62 |
| PKC-108 | IDM city cars (book errata): 54 km/h, T = 1.0 s, s0 = 2 m, a = 1.5, b = 2.0; trucks 80 km/h, T 1.8 s, a 0.5, b 1.0; pedestrians 5 km/h, T 0.8 s, s0 0.2 m | sourced | Traffic Flow Dynamics errata, Table 11.2 |
| PKC-109 | The ballistic update (v += a·h; x += v·h + ½a·h²) has about 30 % of Euler's error at the same cost; a step h acts like a reaction time h/2; a stopping rule prevents negative speed; tested to h = 0.5 s | sourced | Treiber and Kanagaraj 2015, Physica A 419 |
| PKC-110 | Gipps' model is collision-free by construction, with the step equal to the reaction time | reported | Gipps 1981, TR-B 15 |
| PKC-111 | SUMO's default (Krauss) uses random dawdling sigma 0.5, accel 2.6, decel 4.5, tau 1.0, minGap 2.5, step 1 s | sourced | SUMO docs |
| PKC-112 | MOBIL lane changing: safe if the new follower brakes less than 4 m/s²; worth it if own gain exceeds politeness × followers' loss + 0.2 m/s² | sourced | traffic-simulation.de MOBIL; Kesting et al. 2007 |
| PKC-113 | Highway Capacity Manual: each parking move blocks the next lane for 18 s; a bus stop 14.4 s | reported | NPTEL HCM notes |
| PKC-114 | SUMO parallel parking takes 3 s to enter and 4 s to leave, doubled for delivery vehicles | sourced | SUMO ParkingArea docs |
| PKC-115 | One blocked lane of two leaves 35 % of capacity, not 50 %; of three, 49 % | reported | FHWA HCM 2010 incidents |
| PKC-116 | A queue discharges about 6–10 % below the flow before it formed (capacity drop) | reported | Cassidy and Bertini 1999; Hall and Agyemang-Duah 1991 |
| PKC-117 | Manhattan traffic weaves past double-parked trucks; Seattle delivery vehicles park 15 min or less 60 % of the time; median cruising for parking 2.3 min | reported | MTA; Seattle Final 50 Feet |
| PKC-118 | Cities: Skylines caps active vehicles at 16,384 and despawns stuck ones | reported | Steam and TM:PE wiki |
| PKC-119 | Cities: Skylines II routes by time, comfort, money and behaviour; parking cost enters the route; vehicles change lanes around blocked lanes | sourced | Paradox CS2 traffic AI page |
| PKC-120 | CS2 patch (May 2026): U-turns had cost 0 in routing and now cost more; lanes are chosen earlier to stop late merges | sourced | Paradox CS2 news |
| PKC-121 | SimCity 2013 agents took the shortest, not fastest, route and ignored congestion until a patch | reported | gamewatcher.com |
| PKC-122 | GTA III–San Andreas spawned visible cars about 70 m ahead and removed them at about 90 m; San Andreas required 2 s off-screen before removal | reported | gtaboom.com (Vermeij) |
| PKC-123 | Modern ambient traffic: visual actors only inside a radius, pooled, distance checks on a timer | reported | arXiv 2604.08497; GDC 2022 Kythera |
| PKC-124 | Watch Dogs: Legion's Census layer generates characters and a simulation layer makes them persistent with schedules | sourced | Ubisoft news, Legion tools |
| PKC-125 | Shenmue gave every character a daily routine on one clock; KCD2 builds routines with GOAP and behaviour trees | reported | Sega Retro; GDC 2025 |
| PKC-126 | US time use 2023: employed people work on 64.7 % of days (80.4 % weekdays, 28.1 % weekends), 7.75 h on days worked | sourced | BLS ATUS 2023 |
| PKC-127 | US travel 2022: 2.3 trips per person per day (3.4 in 2017); errands 35 %, social 30 %, work 18 %; 18 % of trips start 6–9 a.m. | sourced | NHTS 2022 |
| PKC-128 | UK 2022: about 2.4 trips per day, 53 min travelling; car 58 %, walking 31 % | sourced | UK NTS 2022 |
| PKC-129 | MATSim replans about 10 % of agents per iteration; departure times shift up to 1,800 s by default | sourced | MATSim source; Horni et al. 2016 |
| PKC-130 | Counter-based generators (Philox) give random numbers as a function of key and counter with no stored state | reported | Salmon et al. 2011 |
| PKC-131 | UPS drivers make about 120–175 stops a day | reported | trellis.net; freightwaves.com |
| PKC-132 | Time per delivery stop: about 1.8 min on average, 2.5 in cities, mostly parking and walking | reported | arXiv 2409.06730; FTC2050 survey |
| PKC-133 | Shipt: the on-time bonus only inside the window; up to 30 min late keeps base pay, beyond that loses both | sourced | shoppers.shipt.com |
| PKC-134 | LTL guaranteed freight costs 20–25 % more and is refunded if late | reported | The Loadstar; FleetOwner |
| PKC-135 | Euro Truck Simulator 2 (defaults read from a mod table): late delivery −40 %; manual parking earns extra XP | reported | Steam Workshop discussion |
| PKC-136 | Death Stranding grades deliveries on time, condition and volume; Crazy Taxi adds seconds per fare by speed | reported | twinfinite.net; giantbomb.com |
| PKC-137 | Cities: Skylines has 37 info views (electricity, water, garbage, traffic, pollution, noise, crime, fire, land value…) | sourced | skylines.paradoxwikis.com Info views |
| PKC-138 | Cities: Skylines shows problem icons over buildings in two tiers; SimCity 2013's power layer animates pulses from plants | reported | simplegamer.co.uk; bit-tech.net |
| PKC-139 | RimWorld keeps alerts listed until fixed, in two tiers, critical in red with its own sound | reported | Steam forum |
| PKC-140 | Industrial alarm standards: about 80 % low, 15 % medium, 5 % high priority; at most 1 alarm per operator per 10 min; 10 or more in 10 min is a flood | reported | Emerson (EEMUA 191, ISA-18.2) |
| PKC-141 | Grade each belief from the simulation on both axes: source letter from how it was learned (seen A–B, told C–D by the teller's record, broadcast by the broadcaster, unknown F) and content digit from corroboration (1 when an independent second source agrees, 5 when contradicted); show "B2"; the player never assigns it | estimate | PKC-36..PKC-38; spec 39 |
| PKC-142 | Spec 39's confidence × 0.7 per retelling gives 70 %, 49 %, 34 %, 24 %, 17 % for hops 1–5: on the ICD 203 ladder likely, roughly even, unlikely, unlikely, very unlikely; each retelling drops about one word | derived | PKC-39; spec 39 rule 6 |
| PKC-143 | Map states for beliefs: seen now solid frame; believed dashed; affiliation by frame shape and colour; an uncertainty area of radius r0 + v_max × age drawn soft and fading as it grows, the icon size fixed; a last-known ghost with an age stamp and a dead-reckoning trailer; ages in 3–4 buckets (today, days, over a week) | estimate | PKC-47..PKC-63 |
| PKC-144 | Knowledge mode holds in multiplayer only if views are computed from the belief store (spec 39 rule 7, `Beliefs::view`), never from the truth; the server sends a player nothing it does not know | derived | PKC-28; spec 39 rule 7 |
| PKC-145 | Detachment levels as presets over separate permissions (see the truth, possess others, move the camera anywhere, sandbox tools); the save records which were used and when | estimate | PKC-10, PKC-21..PKC-23, PKC-30..PKC-33 |
| PKC-146 | "Play as" and "Locate" move the camera by zoom out, pan, zoom in, cutting directly when close; a deterministic world can show a followed pop's full past, not only from the moment of following | estimate | PKC-8, PKC-29, PKC-35 |
| PKC-147 | Three real paths to leading after collapse: assembly delegate (rotating, mandated), broker of aid (controls who receives), armed protector (defends the place); a group forms only where authority is missing, unclear or failed | estimate | PKC-72, PKC-74, PKC-80, PKC-81 |
| PKC-148 | Legitimacy as a vector (charismatic, traditional, legal-rational, performance, fear); charisma dies with its holder unless a successor was named; every office stores a succession line | estimate | PKC-84, PKC-86, PKC-90 |
| PKC-149 | An order record from the operation order: id, kind (warning, order, change), parent, cause events, issuer office and pop, recipient, mission (who, what, when, where, why), intent (purpose, key tasks, end state), child orders, effective and valid-until conditions, medium, issued and received ticks | estimate | PKC-93..PKC-96 |
| PKC-150 | A soldier's card shows his own task, the intent one and two levels up (doctrine's paragraph 1b seen from below) and the root cause; each name links to Locate, Follow, Play as | derived | PKC-94, PKC-95 |
| PKC-151 | An army of 100,000 has about 5,000 units at all echelons (PKC-102); a few orders per unit per day is about 15,000–25,000 records, 1–2 MB a day at 64 B each. This corrects the earlier 15,000-unit estimate in demo-flow | derived | PKC-102 |
| PKC-152 | Order latency per hop = processing (each echelon uses at most a third of the remaining time) + transmission by medium (runner or telegraph hours to a day per strategic hop; radio near-instant if the station is staffed) | estimate | PKC-99, PKC-103..PKC-105 |
| PKC-153 | Orders degrade by hop and medium with keyed noise (spec 39's distortion), carry a valid-until condition such as first contact, and when it breaks the subordinate acts on the intent one or two levels up and issues a change order | estimate | PKC-96, PKC-106; spec 39 rule 6 |
| PKC-154 | Hot cars: IDM with the errata city set, ballistic update at 0.5 s (effective reaction about 0.25 s), no random dawdling, ties by vehicle id, MOBIL to pass stopped cars; a parking move holds the lane 6–8 s. Collision-freedom at 0.5 s in emergency stops needs our own test | estimate | PKC-108, PKC-109, PKC-112, PKC-114 |
| PKC-155 | The same blockage in warm and cold: one lane of two blocked multiplies capacity by about 0.35; curbside parking churn by the HCM factor; a further −10 % once a queue forms | derived | PKC-113, PKC-115, PKC-116 |
| PKC-156 | Vehicles change level only out of view (2 s unseen, as San Andreas) and are never despawned: leaving the ring returns them to counts | estimate | PKC-118, PKC-122; spec 27 rule 5 |
| PKC-157 | A pop's day = Philox(world seed, pop, day) → activity chain → times jittered ±30 min, calibrated to 65 % days worked, 7.75 h, 2.3–3.4 trips; only event-caused deviations are stored | estimate | PKC-124, PKC-126..PKC-130 |
| PKC-158 | Delivery contract {id, driver, origin, destination, cargo, value, issued, early-by, deadline, grace, base pay, bonus %, late %, damage %, status}, money in integer cents; pay = base × (1 + bonus) early, base on time, base × 0.6 within grace, nothing beyond (refund to the shipper) | estimate | PKC-133..PKC-135 |
| PKC-159 | About 12 overlays (water, power, zoning, traffic load, curb occupancy, deliveries, crime, fire, health, garbage, noise and pollution, people by hour) | estimate | PKC-137 |
| PKC-160 | Attention list ranked by severity × people affected ÷ time to failure, three tiers targeting 5/15/80 %, about one new item per 10 game minutes, kept until fixed, an icon over the building, click to locate | estimate | PKC-138..PKC-140 |

## 1. Answer in one page

**Playing.** Every precedent keeps the body in the world while the player is away:
- Rust sleepers, ARK unconscious bodies and EVE's logoff timer all do this (PKC-18..PKC-20).
- X4's displaced pilot resumes his orders when you leave the seat (PKC-13).

That is our detach proposal. Space Station 13 is the closest model for the rest:
- **Ghosts** watch everything but cannot take part (PKC-1, PKC-2).
- **Admins** have separate god powers, and every interaction is logged (PKC-4).
- **Jobs and ID cards** gate departments the way our stations gate offices (PKC-5, PKC-6).

Integrity is a mark on the save, never a ban: CK3, Factorio and Minecraft (PKC-10, PKC-32, PKC-33). Permissions should be separate switches with presets (PKC-145). The camera moves by zoom out, pan, zoom in (PKC-35, PKC-146).

**Knowing.** Intelligence practice gives us:
- a two-axis grade (source A–F, content 1–6, PKC-36);
- seven probability words (PKC-39), never mixed with confidence (PKC-40);
- map symbols for "suspected" as dashed frames (PKC-47);
- areas that grow with time (PKC-49, PKC-53).

Uncertainty should show as fuzziness, lightness and spread, not hue (PKC-58). A crisp growing outline gets misread as "it is growing", the hurricane cone (PKC-56). Spec 39's 0.7 per retelling drops one probability word per hop (PKC-142). Knowledge mode holds only if views read the belief store, never the truth (PKC-144).

**Commanding.** Leaders emerge where authority fails (PKC-80), by three real paths: assembly, aid broker, armed protector (PKC-147). Legitimacy has several sources and needs a succession rule (PKC-148). Doctrine confirms the soldier's card: intent "one and two levels up" (PKC-94), "two echelons down must… understand" (PKC-95). An army of 100,000 is about 5,000 units, so order records cost 1–2 MB a day (PKC-151). Spec 39's cause trace already carries the chain (PKC-153).

**Moving.** For the demo street corner:
- **Cars:** IDM at 0.5 s with the ballistic update; MOBIL to pass a parked car (PKC-154).
- **Warm and cold traffic:** the same blockage becomes a capacity factor of about 0.35 (PKC-155).
- **Level changes:** only out of view, never despawned (PKC-156).
- **Schedules:** each pop's day drawn from the seed and calibrated to time-use surveys (PKC-157).
- **Contracts:** a delivery pays a bonus early, base on time, 60 % within grace, nothing after (PKC-158).
- **Overlays and alerts:** about 12 overlays; an attention list in three tiers, ranked by severity, people and time (PKC-159, PKC-160).

## 2. Proposals by spec or decision (all Proposals)

| Where | Change | From |
| --- | --- | --- |
| Decision [0066](../decisions/0066-player-states-embodied-detached-observer.md): player states | Embodied, detached, observer; detachment presets over permissions; the save records uses; pinned windows; body stays in the world with alerts | PKC-1..PKC-35, PKC-145, PKC-146; README proposal |
| 39 orders and belief | Two-axis grade per belief; probability words; military orders with parent, intent and valid-until; latency per hop by medium | PKC-141, PKC-142, PKC-149, PKC-152, PKC-153 |
| 16 view and the map | Belief symbols (solid, dashed, uncertainty area, ghost, trailer, age buckets); the soldier's card; Locate, Follow, Play as; overlays and attention list | PKC-143, PKC-150, PKC-159, PKC-160 |
| 0058 offices, new spec | Paths to office, legitimacy vector, succession line per office | PKC-147, PKC-148 |
| New spec: vehicles and traffic | IDM and MOBIL hot; capacity factors warm and cold; level change only out of view | PKC-154..PKC-156 |
| 33 establishments, new spec | Delivery contracts and pay rule; pop daily schedules from the seed | PKC-157, PKC-158 |

## 3. Open questions

1. Does the user want achievements or a public "integrity mark" at all, or only a private record in the save (PKC-145)?
2. Should the probability words be shown, or only the grade and age? Words read faster, but people read them differently (PKC-46).
3. Which settlement types start the game: assemblies, brokers, protectors, or all three by place (PKC-147)?
4. Do military orders reuse the `Edict` record of spec 39, or get their own kind with parent and intent (PKC-149)?
5. Test needed: is the ballistic IDM collision-free at 0.5 s in emergency stops (PKC-154)?

## 4. Sources

**Playing:**
- **SS13 and SS14:** wiki.ss13.co Ghost; paradisestation.org (Ghost, Player Panel, Job Selection, Chain of Command); wiki.spacestation14.com Ghost; docs.spacestation14.com admin tooling; SS14 AccessReaderComponent API.
- **Other games:** gta.fandom.com Switch Scenes; Polygon (Houser 2013); CK3 wiki; DF wiki; egosoft.com and the X4 wiki; primagames.com; EA forums; tomsguide.com; rust.facepunch.com; Kotaku 2015; EVE University wiki; winternode.com; docs.fivem.net txAdmin; liquipedia.net; Valve SourceTV wiki; Wikipedia (Supreme Commander, SS13); GameSpot 2014.
- **Saves and integrity:** forrestthewoods.com ChronoCam; Paradox CS2 features; EU4 wiki Ironman; gamebanshee.com; Factorio wiki; minecraft.wiki; roguebasin.com.
- **Camera:** arXiv 1801.09358; Google Earth help.

**Knowing:**
- **Grading and words:** Wikipedia Admiralty code; Kelly et al. 2025; ArcGIS symbology help; ICD 203; Kent 1964; gov.uk PHIA; NATO STO 2019; Friedman et al. 2018; jasoncollins.blog.
- **Map symbols and search:** Wikipedia (NATO Joint Military Symbology, Common operational picture); Luciad docs; US 8,052,054; Washburn (NPS); AMSA 2018.
- **Showing uncertainty:** NHC cone page; Padilla et al. 2017; MacEachren et al. 2012; Hullman et al. 2015; Google Maps help; US 3,114,907.
- **Games:** Splinter Cell and Frozen Synapse (Wikipedia); WitE2 manual; Paradox HOI4 dev diary; Steam and wargamer.fr (Shadow Empire); DF wiki Rumor; Command dev blog; Highfleet guide; EVE University wiki.

**Commanding:**
- **Argentina:** Dialnet; Página/12; Wikipedia Crédito; Cambridge (Argentina's parallel currency); Svampa; Tiempo Argentino.
- **Katrina:** commongroundrelief.org; ProPublica; Rodríguez et al. 2006; Clarke and Chess 2008; Fritz 1961.
- **Disaster sociology:** Sutton; Stallings and Quarantelli 1985.
- **Sarajevo:** Andreas 2008; Wikipedia (Topalović, Prazina, Sarajevo Tunnel); FAMA.
- **Theory:** Olson 1993; Arjona 2016; Weber.
- **Games:** Mount & Blade wiki; magicgameworld.com; guildorder.com; suzerain.wiki.gg; DF and RimWorld wikis.
- **Doctrine and history:** STANAG 2014; FM 3-19.4; ADRP 6-0 and its C2 glossary; army.mil 225414; Third Army SOP 1944; german1914.com; CALL 2026; Quote Investigator; Clausewitz.

**Moving:**
- **Car-following and lanes:** Treiber et al. 2000; Traffic Flow Dynamics errata; Treiber and Kanagaraj 2015; Gipps 1981; SUMO docs; traffic-simulation.de MOBIL.
- **Capacity and parking:** NPTEL HCM notes; FHWA; Cassidy and Bertini 1999; MTA; Seattle Final 50 Feet.
- **Game traffic:** Steam and the TM:PE wiki; Paradox CS2 pages; gamewatcher.com; gtaboom.com; arXiv 2604.08497; GDC 2022.
- **Schedules:** Ubisoft Legion tools; Sega Retro; GDC 2025 KCD2; BLS ATUS 2023; NHTS 2022; UK NTS 2022; MATSim; Salmon et al. 2011.
- **Deliveries:** trellis.net; freightwaves.com; arXiv 2409.06730; FTC2050; shoppers.shipt.com; The Loadstar; FleetOwner; Steam Workshop (ETS2); twinfinite.net; giantbomb.com.
- **Overlays and alerts:** skylines.paradoxwikis.com; simplegamer.co.uk; bit-tech.net; Steam RimWorld forum; Emerson (EEMUA 191).
