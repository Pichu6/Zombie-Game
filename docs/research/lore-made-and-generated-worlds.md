# Lore in made and generated worlds: Project Zomboid, other games, and tools outside games

**Date:** 2026-10-10. **Status:** research note, not a decision. **Feeds:** [inhabitants](../design/inhabitants.md) (outbreak origin, fact and account), [sandbox parameters](../design/sandbox-parameters.md), [content tools](../design/content-tools.md), [spec 39](../tier0/specs/39-orders-and-belief.md) (broadcasts, beliefs), and the design drafts [story-blocks](../design/story-blocks.md), [cases-and-living-lore](../design/cases-and-living-lore.md) and [story-explorer](../design/story-explorer.md). **Evidence:**
- **Read through a page summariser:** the NME report of The Indie Stone's NPC storylets (2022).
- **Search summaries only:**
  - fan wikis, guides and forums for Project Zomboid (no official lore document was found);
  - RimWorld wiki pages;
  - the Creation Kit wiki;
  - 7 Days to Die and Cataclysm: Dark Days Ahead documentation and forums;
  - World Anvil's help pages;
  - Genealogical Proof Standard and Gramps discussions;
  - ICIJ and Neo4j coverage, and i2 documentation;
  - Wikidata data-model descriptions.
- **Not read:** Jason Grinblat's GDC talk itself, only its session description.

**IDs:** LR.


The user asked how Project Zomboid manages its lore, given that its map is hand-made and its lore fixed even in sandbox. The user also asked how related games, and apps made for other uses, handle the same problem.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| LR-1 | Project Zomboid's premise is fixed: the Knox Event, Knox County, Kentucky, 1993. Phone lines inside the zone were cut days before; on 6 July soldiers in hazmat suits set roadblocks around the county. Players piece the timeline together from radio logs, broadcasts and early in-character tweets; fans compile it, and theories (a lab near Rosewood, chemical tests, meteors) stay theories | reported | ggservers.com and progameguides.com lore pages |
| LR-2 | PZ tells its outbreak through TV and radio scripted by game day, while the power lasts:<br>• Triple-N interviews people in the zone for about 12 days;<br>• WBLN News carries statements from a general and the President;<br>• Life and Living teaches skills (and grants experience) for the first 9 days;<br>• the end is a looping government message calling it an unidentified plague, then silence | reported | servers4gamers.com and namehero.com TV guides; Steam discussions |
| LR-3 | PZ radio:<br>• the Automated Emergency Broadcast System, on a frequency that changes per save, gives weather, power warnings and "air activity detected" before the helicopter event;<br>• hidden stations: 91.2 (a reporter just outside the zone), 107.6 (life in a quarantine zone, day 3), 95.0 (a coded military numbers broadcast);<br>• a talk-radio caller says a lab janitor hinted the outbreak was engineered | reported | projectzomboid.wiki radio broadcast guide; dotesports.com; pzfans.com |
| LR-4 | PZ's **randomized stories**: hand-written scenes placed at random into the hand-made map. They come in three kinds:<br>• **zone stories:** groups of zombies telling a scene, such as an office meeting, a birthday party, a fishing trip or a police blockade;<br>• **vehicle stories:** on roads only, crashes and abandoned cars;<br>• **house stories:** you break in and find clues to what happened there.<br>Their frequency is a world setting, and Build 42 added more vehicle stories | reported | Steam discussions app 108600; pzfans.com; pcgamesn.com |
| LR-5 | PZ annotated maps are ordinary town maps with marks and notes pointing to a stash. The stash may never spawn if you enter the building before reading the map | reported | projectzomboid.wiki annotated maps guide |
| LR-6 | PZ's sandbox calls its zombie rules **"Zombie Lore"**: speed, strength, toughness, transmission (blood and saliva by default: bite 100 %, laceration 25 %, scratch 7 %), mortality, reanimation, cognition, memory, decomposition, sight, hearing. The lore is literally a set of parameters | reported | xgamingserver.com zombie and infection settings docs |
| LR-7 | PZ Build 42 adds newspapers, each with its final three or four issues readable in a text UI (the build-up to the Knox Event, ordinary life, "breadcrumbs" about what is really going on), plus flyers, posters and brochures. It also roughly doubles the map with 1,400 new buildings | reported | projectzomboid.com blog (via search summary); pcgamesn.com; 4netplayers.com |
| LR-8 | PZ has no quests carrying its story. A reviewer reads its opening line, "This is how you died", as the design: the run is the story, and the lore frames the silence without filling it | reported | gamegeeker.com review |
| LR-9 | The Indie Stone's planned NPCs (Build 43, described in 2022):<br>• "context driven storylets" combine into "a loosely convincing narrative";<br>• events set flags on NPCs and move them along roads and between houses even where the map is not loaded, so a player who meets one later finds a plausible backstory;<br>• events carry dialogue lines the NPC can tell later;<br>• results are sometimes odd, and the published examples had "some massaging".<br>Example: a sister falls ill before the outbreak, the trip to hospital fails on a jammed highway, and the jam's cause can vary | reported | nme.com 2022-04-15 |
| LR-10 | 7 Days to Die builds a whole world from a seed (terrain, biomes, roads, towns, wilderness points of interest) and places hand-built prefabs, made in its Prefab Editor, by size and tags. A fan pack adds about 1,300 buildings to the pool | reported | wiki.7d2d.net advanced RWG configuration; 7d2dmodding.wiki.gg Compopack |
| LR-11 | Cataclysm: Dark Days Ahead builds map areas when the player first discovers them, from hand-written layouts plus placed items, monsters and vehicles. Map "extras" carve scenes such as drug-deal remains or minefields into them. Its design doc warns that the lore documents tell "a different story" than the one the team wants players to experience | reported | docs.cataclysmdda.org MAPGEN and design doc; discourse.cataclysmdda.org |
| LR-12 | Skyrim's Radiant Story:<br>• quests are templates with **aliases** (actors, locations, items) filled at run time by conditions;<br>• if a required alias cannot be filled, the quest never starts;<br>• a Story Manager starts quests from game events, and a "kill actor" event carries the victim, the killer and whether the death is known to be murder | reported | ck.uesp.net Radiant Story; Bethesda tutorial radiant quests |
| LR-13 | RimWorld pawns combine a written childhood backstory with a written adulthood backstory (about 36 and 85 in the base lists, per the wiki), each changing skills; Kickstarter backers' pairs always come together under one name. Art of excellent quality or better gets a title and a story drawn from the colony's real events ("tales"): one player's colonist sculpted the raider who killed a pet | reported | rimworldwiki.com Backstory and Sculpture; Steam discussions |
| LR-14 | Caves of Qud generates historical events and rationalises them afterwards, rather than simulating the whole past (Jason Grinblat's GDC talk on its histories) | reported | gdcvault.com/play/1024990 session description; intfiction.org |
| LR-15 | World Anvil keeps lore as linked articles with timelines, family trees and interactive maps; "Chronicles" join maps and timelines. Secrets inside articles can be revealed per player group as a story advances. Restricted articles still showed in the table of contents, spoiling them between groups | reported | worldanvil.com secrets workflow, subscribers, Chronicles; community voting threads |
| LR-16 | Genealogy treats a life as conclusions from records:<br>• the Genealogical Proof Standard asks for reasonably exhaustive research, a complete citation for each item, analysis and correlation, resolution of conflicting evidence, and a written conclusion;<br>• GEDCOM grades each citation 0–3: unreliable or estimated; questionable (interviews, census, oral); secondary; direct and primary;<br>• Gramps attaches a confidence to every citation | reported | en.wikipedia.org/wiki/Genealogical_Proof_Standard; gramps.discourse.group |
| LR-17 | Investigators work on the same board we designed:<br>• i2 Analyst's Notebook stores entities, links and properties, draws association charts, and puts events on a timeline to find meetings;<br>• ICIJ loaded the Panama Papers (11.5 million documents, more than 370 reporters in 76 countries) into Neo4j and showed the connections with Linkurious | reported | docs.i2group.com; neo4j.com ICIJ interview; linkurio.us |
| LR-18 | Wikidata keeps each statement with qualifiers (such as a point in time), references (provenance) and a rank: preferred, normal or deprecated. A claim that stopped being valid is deprecated, not deleted ("Pluto is a planet" after 2006); queries skip deprecated claims | reported | courses.cs.umbc.edu Wikibase notes; Dublin Core webinar slides |
| LR-19 | PZ's scenes are coherent inside themselves but connected to nothing: a birthday party scene has no tie to the house next door or to any history. A hand-made map lets writers know every building; a generated Earth-size world cannot be written by hand | derived | LR-4, LR-10 |
| LR-20 | **Proposal:** scene blocks chosen by what happened, not by dice. A building's derived interior shows the scene that fits the facts the simulation holds about it (a household lost to a hidden bite → a barricaded bedroom, a note, the bodies). This is PZ's texture with Caves of Qud's "event first, explained after", constrained to agree with recorded outcomes | derived | LR-4, LR-14; cases §11 |
| LR-21 | **Proposal:** PZ's TV and radio written from this world's events. Broadcasts are lines over real events and over each office's official version, which can lie. Hidden stations are real pops with transmitters. The last government loop is an office's final broadcast before the power goes | derived | LR-2, LR-3; spec 39 broadcasts |
| LR-22 | **Proposal:** objects that remember (RimWorld's tales, LR-13): graves with names and dates, memorials, graffiti, messages left on walls for missing family, photos. Each is made by a real pop's action and refers to real events and people | estimate | LR-13 |
| LR-23 | **Proposal:** keep an authored premise layer, as PZ does: the outbreak's nature and rules are sandbox parameters (PZ's "Zombie Lore", LR-6, matches our pathogen settings). Optional authored origin presets give a canonical lore, like PZ's Knox Event; history and lives stay generated | estimate | LR-1, LR-6 |
| LR-24 | **Proposal:** hand-built landmarks placed by generators (7 Days to Die prefabs, LR-10): a lab, a prison, a base, carrying authored hooks. They are islands of written lore in a generated sea, placed only where a real-world rule says such a place would be | estimate | LR-10, LR-12 |
| LR-25 | **Proposal:** every claim a pop holds carries a source type and a grade, as genealogy does (LR-16). A claim that turns out wrong is ranked down, not deleted (LR-18), so diaries and boards keep what was believed and when | derived | LR-16, LR-18; spec 39 |
| LR-26 | **Proposal:** hidden things must not leak through indexes. World Anvil's table of contents spoiled secrets (LR-15). In knowledge mode, counts, search results, pins and autocomplete must never reveal that a hidden case, person or place exists | derived | LR-15 |
| LR-27 | PZ's planned storylets (LR-9) are our closest relative, and also the warning: off-screen stories built from storylets needed hand-massaging and gave odd results. Our off-screen lives are pure functions that agree with the macro numbers (spec 28), wants are resolved by world rules, and coherence is tested ([story-explorer](../design/story-explorer.md) §8) | derived | LR-9 |

## 1. Answer in one page

**How Project Zomboid does it.** The lore is a fixed, thin **premise** (the Knox Event) and the map is hand-made. Its delivery is mostly indirect:
- scripted **TV and radio** for the first days, which die with the power;
- **newspapers, flyers and notes** (expanded in Build 42);
- **environmental scenes:** hand-written "randomized stories" dropped at random into houses, roads and zones;
- **annotated maps** to stashes.

The zombie rules sit in the sandbox under the name "Zombie Lore". There are no quests: the player's own death is the story. Each scene makes sense alone, but nothing connects them. A birthday party scene has no tie to the house next door. PZ's answer to living NPC stories (Build 43, planned) is storylets that evolve NPCs off-screen, and its developers admit the results need massaging.

**Other games sit on a line from hand-written to simulated:**

| Approach | Examples | Coherent? | Variety |
| --- | --- | --- | --- |
| Hand-made world, hand-made lore | PZ's premise and map, DayZ, The Long Dark | Fully, but finite and spoilable | Low |
| Hand-made pieces, placed by procedure | PZ scenes, 7 Days to Die prefabs, Cataclysm extras, Skyrim radiant quests, RimWorld backstories | Inside each piece | Medium; repeats show |
| Generated history, objects that remember it | Dwarf Fortress legends and engravings, RimWorld art tales, Caves of Qud histories | Globally, if built from one history | High |
| Live simulation | Shadows of Doubt, Watch Dogs: Legion, PZ's planned NPCs, *Bad News* | Only as good as the rules | Highest |

**Apps outside games already solve parts of our problem:**
- **Worldbuilding wikis** (World Anvil): lore as linked entities, timelines and maps, with secrets revealed per reader.
- **Genealogy:** lives rebuilt from records, every claim cited and graded, conflicts resolved by a written standard. It is a detective method for history.
- **Investigation tools** (i2, ICIJ's graph): our Board, used by real investigators.
- **Wikidata:** claims with time, source and rank, superseded rather than deleted.

**What we take.** Our design already sits at "live simulation" with a generated history. From the others we take:
- PZ's texture, chosen by facts instead of dice (LR-20);
- PZ's broadcasts, written from real events and official lies (LR-21);
- RimWorld's objects that remember (LR-22);
- PZ's authored premise as a layer and optional canonical presets (LR-23);
- 7 Days to Die's hand-built landmarks placed by rule (LR-24);
- genealogy's graded citations and Wikidata's ranks for beliefs (LR-25);
- World Anvil's lesson that indexes leak secrets (LR-26).

## 2. Lore as layers, who writes each

| Layer | PZ | Us |
| --- | --- | --- |
| **Premise:** what the outbreak is, its rules | Written (Knox Event; Zombie Lore settings) | Written as parameters (pathogen settings, outbreak grammar in inhabitants); optional authored origin presets (LR-23) |
| **Place:** the map and its landmarks | Hand-made map | Generated Earth-size world; hand-built landmarks placed by rule (LR-24) |
| **History:** what came before | Thin, implied | Generated: countries, towns, lifepaths, hooks |
| **Events:** what happens | The player's run; scripted broadcasts | Simulated, everywhere, with or without a player |
| **Telling:** how it reaches you | Scripted broadcasts, papers, notes, random scenes | Lines, broadcasts, diaries, scenes and objects, all chosen by facts ([story-blocks](../design/story-blocks.md)) |

## 3. Sources

- **Project Zomboid:** [lore timeline (GGServers)](https://ggservers.com/blog/project-zomboid-lore-timeline/), [lore (Pro Game Guides)](https://progameguides.com/project-zomboid/project-zomboid-lore-everything-we-know/), [radio broadcast guide](https://projectzomboid.wiki/guides/radio-broadcast/), [secret radio stations](https://dotesports.com/general/news/project-zomboid-all-secret-radio-station-channels-explained), [TV schedule](https://servers4gamers.com/blog/project-zomboid-tv-schedule-life-and-living-guide/), [annotated maps](https://projectzomboid.wiki/guides/annotated-maps/), [zombie settings](https://xgamingserver.com/docs/project-zomboid/zombie-settings), [infection settings](https://xgamingserver.com/docs/project-zomboid/infection-settings), [Build 42 (PCGamesN)](https://pcgamesn.com/project-zomboid/build-42-unstable-launch), [Build 42 changes](https://www.4netplayers.com/en-in/blog/project-zomboid/project-zomboid-build-42-changes/), [randomized stories discussion](https://steamcommunity.com/app/108600/discussions/0/3051733836214308485), [NPC storylets (NME)](https://www.nme.com/news/project-zomboid-studio-explains-how-branching-narratives-work-for-npcs-3205735), [review](https://gamegeeker.com/it/games/project-zomboid-108600/review).
- **Other games:** [7 Days to Die RWG](https://wiki.7d2d.net/guides/advanced-rwg-configuration.html), [Compopack](https://7d2dmodding.wiki.gg/wiki/Compopack_Classic_V3), [Cataclysm MAPGEN](https://docs.cataclysmdda.org/JSON/MAPGEN.html), [Cataclysm design doc](https://docs.cataclysmdda.org/design-balance-lore/design-doc.html), [Radiant Story](https://ck.uesp.net/wiki/Radiant_Story), [radiant quests tutorial](https://ck.uesp.net/wiki/Bethesda_Tutorial_Radiant_Quests), [RimWorld backstories](https://rimworldwiki.com/wiki/Backstories), [RimWorld sculpture](https://rimworldwiki.com/wiki/Sculpture), [Caves of Qud history talk](https://gdcvault.com/play/1024990/Procedurally-Generating-History-in-Caves).
- **Tools outside games:** [World Anvil secrets](https://www.worldanvil.com/learn/workflows/secrets-workflow), [World Anvil Chronicles](https://www.worldanvil.com/features/chronicles), [Genealogical Proof Standard](https://en.wikipedia.org/wiki/Genealogical_Proof_Standard), [Gramps confidence discussion](https://gramps.discourse.group/t/proposed-changes-to-source-and-citation-objects-to-better-support-evidence-analysis/1050), [i2 Analyst's Notebook](https://docs.i2group.com/anb/10.1.1), [ICIJ and Neo4j](https://neo4j.com/videos/how-neo4j-was-used-to-analyze-the-panama-papers-dataset-interview-with-mar-cabra-of-the-icij), [Linkurious and the Panama Papers](https://linkurio.us/panama-papers-how-linkurious-enables-icij-to-investigate-the-massive-mossack-fonseca-leaks/), [Wikibase data model notes](https://courses.cs.umbc.edu/graduate/691/fall22/kg/notes/24_wikidata/wikibase.pdf).
