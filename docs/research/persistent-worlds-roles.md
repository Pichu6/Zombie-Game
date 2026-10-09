# Persistent worlds, roles, chains of command, time and pre-simulated history

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [0058](../decisions/0058-offices-open-layers-single-player-is-multiplayer-with-ai.md), [0061](../decisions/0061-the-world-runs-without-the-player.md), [0062](../decisions/0062-the-game-loop-attention-possession-and-late-starts.md), [cameras and modes](../design/cameras-modes.md). **Evidence:** read in full (`sourced`): CCP's dev blog "Introducing Time Dilation (TiDi)" (2011), the EVE University wiki page on time dilation, the Factorio wiki "Multiplayer" page and Friday Facts #415, the Paradise Station wiki "Chain of Command" page, the Dwarf Fortress wiki world generation page (DF2014), the Game Developer Q&A with Tarn Adams, the Caves of Qud wiki "Sultan history" page and the RimWorld wiki "Scenario system" page. Everything else is from web-search summaries of wikis, forums, guides and news (`reported`). Blocked or not reached: the tgstation wiki (403), the Barotrauma wiki (403), the Squad fandom wiki (402), Paradox's own multiplayer documentation (not found). The search budget ran out before Squad and Hell Let Loose bots, Space Station 13 job preferences and RimWorld world generation could be searched; those are listed under Open. No Tarn Adams talk dedicated to history generation was found. **IDs:** MP.

Decisions 0058, 0061 and 0062 settled that every office is a pop with its own AI, that a player acts only by possessing a pop, that a server world has one clock with its speed fixed at creation, and that a scenario may start years into the world. They left open the server itself: who may pause, how players join and leave offices, how orders pass between players, and how the years before a late start are simulated. This note collects how other games handle these and ends with a proposal.

Already covered elsewhere and only cited here: Haven and Hearth's always-on persistent world ([HH-3](haven-and-hearth.md), [HH-20](haven-and-hearth.md)), Foxhole's player-run logistics, regiments and world size ([FH-1](foxhole.md), [FH-10](foxhole.md), [FH-11](foxhole.md), [FH-12](foxhole.md)), Dwarf Fortress tiers, legends size and its two-week skip on revisit ([PP-2](pop-simulation-and-persistence.md), [PP-4](pop-simulation-and-persistence.md), [PP-5](pop-simulation-and-persistence.md)), and the Dwarf Fortress world that keeps advancing after generation ([LJ-14](layer-jumps-multiresolution.md)).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| MP-1 | Space Station 13 (Paradise server): the Captain is the top authority and can overrule any head of staff, but is still under space law and can be arrested; six heads (Head of Personnel, Head of Security, Chief Engineer, Research Director, Chief Medical Officer, Quartermaster) are equal to each other and each runs one department | sourced | paradisestation.org/wiki/index.php/Chain_of_Command |
| MP-2 | Space Station 13 succession when the Captain is missing: the Head of Personnel acts as Captain; the Head of Security takes over during a major emergency and hands back after; if those are gone the remaining heads vote; if no head remains, the corporate representative acts. An acting Captain steps down when a real Captain is assigned | sourced | same as MP-1 |
| MP-3 | Space Station 13 missing heads: if the Head of Security is absent the Warden acts; a department with no head may get an acting head promoted from its members by the Captain or Head of Personnel; acting holders must secure their rank's sensitive items and return them to the official holder | sourced | same as MP-1 |
| MP-4 | Space Station 13 removal: the Captain may demote a head for a legitimate reason, the other heads may fire one by majority vote, a department may vote out its head with a two-thirds majority, and a capital crime removes rank automatically | sourced | same as MP-1 |
| MP-5 | In Space Station 13 only heads can call or recall the escape shuttle and use the command radio channel: some powers belong to the office, not to the person | sourced | same as MP-1 |
| MP-6 | Succession rules differ by server: Monkestation also puts the Head of Personnel first; a Beestation forum proposal ranks all heads in a fixed order for "Acting Captain". Players get a job at round start, grouped in about six departments (command, security, engineering, science, medical, service) | reported | wiki.monkestation.com/Chain_of_Command; forums.beestation13.com/t/restructuring-command-staff/20238; giantbomb.com/space-station-13/3030-32586 |
| MP-7 | Foxhole and Haven and Hearth are always-on worlds with no AI in player roles: everything is made and carried by players (FH-1, HH-3), and the unpopular jobs (logistics) are filled only by player goodwill (FH-12); an empty role is simply undone work | estimate | FH-1, FH-12, HH-3, HH-20 |
| MP-8 | EVE Online runs one shared world (one shard, Tranquility, hosted in London); in 2006 CCP reported about 30,000 concurrent players on it | reported | pcgamer.com/eve-online-1; datacenterknowledge.com "EVE Online: 30,000 Users on One Server Shard" |
| MP-9 | EVE corporations are run as hierarchies: in EVE University the CEO has overall control, directors set policy and managers run departments under them, and a Director of Operations can decide on the CEO's behalf | reported | wiki.eveuniversity.org/Management; wiki.eveuniversity.org/CEO |
| MP-10 | Squad: the Commander leads the team and manages the squad leaders; the post is elected by vote; nominating needs 2 squad members and at least 3 squads; re-election is possible after 300 s; the Commander's death costs 2 tickets instead of 1 | reported | squad.gamepedia.com/Commander (search summary) |
| MP-11 | Squad's chain is fireteam leader, squad leader, Commander; only squad leaders have a key to talk to command; fireteam leaders report to their squad leader, which keeps the command net clear | reported | squad.gamepedia.com/Commander; guildorder.com/games/squad/guides/sl-fundamentals |
| MP-12 | Hell Let Loose: the Commander has no squad and talks only to officers on a command voice channel; officers are the only squad members who can speak to command; the Commander manages manpower, fuel and munitions and calls in supply drops, bombing runs and tanks, marked on a command map | reported | guildorder.com/games/hell_let_loose/guides/commander-role-manpower-and-logistics; steelseries.com Hell Let Loose roles guide |
| MP-13 | Hell Let Loose players report that with no Commander, or a weak one, the team loses coordination and the command chat goes quiet; no source found on what the game does with an empty Commander seat; roles open to anyone after the first minute of warm-up | reported | tomsguide.com "Hell Let Loose is too good for its own good"; steamcommunity.com app 686810 discussions |
| MP-14 | PlanetSide 2: a squad leader has a squad waypoint (seen by the whole platoon) and four fireteam waypoints (seen by the squad); the platoon leader can place and move any squad's waypoint; a platoon holds 2 to 4 squads; squad leaders join a leadership channel | reported | planetside.fandom.com/wiki/Fireteam; forum.level1techs.com "Starting a Planetside 2 Platoon" |
| MP-15 | Barotrauma: each player gets a job at the start of a round; AI crew take initiative on their own, and orders (including "wait here" and "dismiss", which returns them to their own choices) help them act better; the wiki calls the crew AI buggy and advises taking manual control when needed | reported | en.wikipedia.org/wiki/Barotrauma_(video_game); barotrauma.gamepedia.com/Orders |
| MP-16 | Barotrauma bots hear orders through their headsets; a player reports that bots do not recharge the headset batteries, so they stop receiving orders when the batteries run out | reported | steamcommunity.com app 602960 discussions (player report) |
| MP-17 | Arma 3 Altis Life servers keep positions, bank balances and vehicle ownership in a MySQL database; police levels 1 to 7 and medic levels 1 to 5 gate weapons and vehicles; admins set them; some servers whitelist police after a forum application and a voice interview | reported | help.freakhosting.com Altis Life setup; zap-hosting.com arma3 Altis Life ranks; steamcommunity.com forum 24 thread |
| MP-18 | Mount and Blade: Warband's Persistent World mod (by Vornne) offers serf, woodcutter, miner, soldier, knight, trader, sailor and a lord with a castle as roles, with factions, diplomacy, a player-run economy and castle captures | reported | moddb.com/mods/persistent-world-mod; steamcommunity.com app 48700 discussions |
| MP-19 | Life is Feudal MMO: a guild monument needs a guild leader and at least 10 members; its claim grows with monument level; only members interact inside it; a guild that loses a war loses one monument level and so land; claims take effect at the next game-day update and need coin upkeep | reported | massivelyop.com Life is Feudal beta article; lifeisfeudal.fandom.com/wiki/Personal_claim; steamcommunity.com app 700030 discussions |
| MP-20 | EVE time dilation (TiDi): when a server node is overloaded the game clock slows in fine steps so the queue of waiting work stays near zero; in CCP's example of about 1,600 ships the clock ran at about 5 % during the warp-in, 30 % at the start of the fight, 60 % with 1,200 engaged, 10 % during the retreat jumps, then back to 100 % | sourced | eveonline.com/news/view/introducing-time-dilation-tidi |
| MP-21 | TiDi's timer rule: timers a player would check against a wall clock (structure reinforcement) do not dilate; things that play out inside the fight (modules, physics, travel, shield recharge) do. CCP called 0.1 % time clearly too slow and left the floor to tuning | sourced | same as MP-20 |
| MP-22 | Today TiDi bottoms out at 10 % (one game second takes ten real seconds), applies per node so every system on that node slows together, and does not affect server time, skill training, industry or structure repair timers | sourced | wiki.eveuniversity.org/Time_dilation |
| MP-23 | CCP reported that under TiDi module response stayed under one second for most of the action in large fights: players accepted slower time over unanswered commands | reported | engadget.com 2012-02-09 "EVE Online's time dilation keeping game in sync" |
| MP-24 | EVE's skill queue progresses while the player is offline; CCP limited it to skills that start within 24 h (up to 50) because they feared players would queue a year and stop playing; critics call the result "Skill Queue Online" | reported | eveonline.com/news/view/more-queue-queue; mmos.com "Offline Training: A Blessing Or a Curse?" |
| MP-25 | In Paradox multiplayer (Crusader Kings II and III, Hearts of Iron IV) only the host controls game speed; a community tool exists just to pass speed requests to the host; groups usually make the host the arbiter of speed with a convention for asking for a pause | reported | github.com/davue/paradoxspeedsync; guildorder.com/games/hoi4/wiki/multiplayer-campaigns |
| MP-26 | Europa Universalis IV multiplayer: the space bar pause is disabled; players report any player can pause from the date widget unless the host turns that off; the host picks the speed, and too high a speed makes other players lag and forces pauses | reported | steamcommunity.com app 236850 discussions |
| MP-27 | Hearts of Iron IV forces slowdowns and pauses when a client lags; a workshop mod exists to remove that | reported | steamcommunity.com HOI4 workshop "No forced Pause / Slowdown for MP Lag" comments |
| MP-28 | Crusader Kings III players report "Out of Sync: Living Characters" errors within about 10 game days in some games; lowering speed only delays them; players suspect AI character travel | reported | admin-forum.paradoxplaza.com thread 1733377; steamcommunity.com app 1158310 discussions |
| MP-29 | Factorio multiplayer is deterministic lockstep: every client simulates the whole world and only player inputs are sent, relayed by the server; movement is predicted locally to hide latency; versions and mods must match exactly (checksums on join); 3 desyncs disconnect a player; since 0.14 a slow client does not slow the server | sourced | wiki.factorio.com/Multiplayer |
| MP-30 | Factorio joining: the joiner downloads a save and then simulates forward through the inputs it missed until it reaches the live tick; on large maps or slow links this takes one to several minutes; 2.0 adds an option to pause the server while someone joins, and keeps an auto-paused server paused until a joiner has fully loaded | sourced | factorio.com/blog/post/fff-415; join sequence also in alt-f4.blog/ALTF4-41 |
| MP-31 | A Factorio headless server pauses when no player is connected, so an empty base is not attacked; `--no-auto-pause` turns that off | sourced | wiki.factorio.com/Multiplayer; factorio.com/blog/post/fff-415 |
| MP-32 | Factorio players objected that pausing for a joiner blocks everyone; one proposed running the server slower during catch-up instead; others said the pause suits small groups who know each other, not servers with frequent joins | reported | forums.factorio.com FFF #415 thread |
| MP-33 | The RimWorld Multiplayer mod runs deterministic lockstep; an optional "async time" gives each colony its own clock in one shared game; mods not marked compatible cause desyncs | reported | top-mods.com RimWorld Multiplayer listing; gameserverkings.com RimWorld multiplayer guide |
| MP-34 | Stellaris drop-in and drop-out: if the host allows, a player can join a running game and take an empire the AI has been running; a player who leaves hands the empire to the AI. Older reports say a quit empire finishes its last orders and then mostly guards, and a surrendered one stops acting | reported | steamcommunity.com app 281990 discussions 357286119114826101, 4203492762821134003 |
| MP-35 | Community advice for Stellaris campaigns: agree before starting what happens to a dropped empire (another player, the AI, or removal), and brief a joiner who inherits a history and relations they did not make; an AI-held empire is treated differently by the other players | reported | guildorder.com/games/stellaris/wiki/player-dropout-and-succession |
| MP-36 | Mount and Blade II: Bannerlord: the ruler has the final word on kingdom decisions, but every clan backs an option by spending influence; an AI clan weighs its own benefit, how much influence it would have left, the odds of winning and its relation to the sponsor; a minimum time between proposals keeps votes from flooding | reported | gamebanshee.com Bannerlord dev blog "Kingdom Decisions" |
| MP-37 | In Warband, AI lords given poor fiefs or treatment may leave, and in some mods they rebel and take their castles into a new kingdom: AI officeholders have their own interests, not just obedience | reported | steamcommunity.com app 48700 discussions; tvtropes Anno Domini 1257 |
| MP-38 | Dwarf Fortress history lengths: Very Short 5, Short 125, Medium 250, Long 550, Very Long 1,050 years; the youngest possible world is 2 years old | sourced | dwarffortresswiki.org/index.php/DF2014:World_generation |
| MP-39 | Dwarf Fortress map generation is usually much faster than history; after the map, civilizations and caves are placed and history runs; Tarn Adams calls it a zero-player strategy game with loose turn rules and weak AI but thousands of agents, and history is the record of it | sourced | same as MP-38; gamedeveloper.com Q&A with Tarn Adams |
| MP-40 | Dwarf Fortress history may stop before the end year: when a set share of megabeasts is dead, or when the player stops it by hand and saves; the same world seed and history seed with an earlier end year give the same world just before a chosen event | reported | dwarffortresswiki.org v0.31 and v0.34 advanced world generation pages |
| MP-41 | Dwarf Fortress cost: longer histories and bigger worlds take longer; a forum user estimates hours for a 2,000-year world; Adams said a default world needed over 1.2 GB of RAM | reported | steamcommunity.com app 975370 discussions; gamedeveloper.com Q&A with Tarn Adams |
| MP-42 | With "reveal all history" off, most Dwarf Fortress history is hidden from Legends mode until adventurers uncover it: what happened and what the player knows are kept apart | reported | dwarffortresswiki.org/index.php/v0.34:Legends |
| MP-43 | Caves of Qud history is five generated sultans plus one fixed; each gets 10 to 22 events (usually 11 to 14): an origin, 8 core events from 17 types, an added event if none made them ruler, added events so each of their 2 or 3 regions has one, then death; events create relics, sites, monuments and faction likes, found later through shrines and engravings | sourced | wiki.cavesofqud.com/wiki/Sultan_history |
| MP-44 | Ultima Ratio Regum generates myths, histories, languages, flags, coats of arms and civilizations with policy traits (war, trade, expansion), and later currencies and goods per civilization, all for one fixed historical era | reported | markrjohnsongames.com URR 0.9 posts; moddb.com URR 0.6 release |
| MP-45 | Crusader Kings III starts from hand-written history at fixed dates (867 and 1066 at launch, a third added later; sources disagree on 1178 or 1187) and runs to 1453; the history files set titles and wars up to and past the start | reported | gamewatcher.com CK3 time period article; steamcommunity.com workshop discussion 2270547030 |
| MP-46 | RimWorld scenarios set the player faction type, starting colonists, items, animals, research, conditions and incidents; the scenario system has no start date and no pre-simulated history | sourced | rimworldwiki.com/wiki/Scenario_system |
| MP-47 | Two ways to make a past: simulate it forward (Dwarf Fortress, MP-39) or write it backward from required outcomes (Caves of Qud forces a ruler event and region coverage, MP-43; Crusader Kings writes it by hand, MP-45). Our 0062 asks for the first; the second only fits flavour that no rule depends on | derived | MP-39, MP-43, MP-45 |
| MP-48 | Every multiplayer game here that lets players change speed has one person own it (the Paradox host, MP-25, MP-26) and lag forces pauses for everyone (MP-27); a fixed speed set at creation (0058 item 5) removes the speed dispute entirely | derived | MP-25, MP-26, MP-27 |
| MP-49 | Our world is too large for lockstep catch-up: Factorio joiners already wait minutes (MP-30), and our joiner would have to simulate the whole world. A server-authoritative world that sends a joiner only what their pop's scope can know (0044) fits better | estimate | MP-29, MP-30, MP-32 |
| MP-50 | Overload on a one-clock server: slow the whole world in fine steps like TiDi (MP-20) rather than per region, since 0058 has one clock; wall-clock commitments (a scheduled vote, an announced deadline in real days) should not exist, so every timer is a game timer and dilates together | estimate | MP-20, MP-21, MP-22, MP-23 |
| MP-51 | A seat is never empty: when a player releases a pop, its AI resumes at once from the same planner state; the Stellaris "passive after quit" behaviour (MP-34) is the failure to avoid, and 0061 already requires every AI role to run unattended | estimate | MP-34, MP-35, MP-13 |
| MP-52 | Offices need written succession and acting rules like Space Station 13's (MP-2, MP-3): deputy acts, a fixed order in emergencies, a vote among peers when the order runs out, acting holders hand back power and items. Removal by superior, by peers' majority or by subordinates' two-thirds (MP-4) gives players and AI the same levers | estimate | MP-2, MP-3, MP-4, MP-36 |
| MP-53 | Orders travel as messages over a channel each office owns (the Space Station 13 command channel, Squad's squad-leader key, Hell Let Loose's officers-only radio, MP-5, MP-11, MP-12), and a broken channel stops them (Barotrauma headsets, MP-16; 0014). A superior may override a subordinate's marker (PlanetSide 2, MP-14) but the order still arrives as an order the holder carries out | estimate | MP-5, MP-11, MP-12, MP-14, MP-16 |
| MP-54 | An AI subordinate should weigh an order against its own interest the way Bannerlord clans weigh votes (MP-36) and Warband lords leave bad masters (MP-37); a player subordinate decides freely. In both cases the result is the same kind of input (0061 item 2) | estimate | MP-36, MP-37 |
| MP-55 | Offline players: the possessed pop is released to its AI when the player leaves, with the player's standing orders kept for a bounded time (EVE's 24 h queue window, MP-24) so the AI does not undo them at once and a player cannot rule through a year-long queue | estimate | MP-24, MP-31, MP-51 |
| MP-56 | Late start, what to simulate exactly: stocks in the ledger, deaths and births as counts, infrastructure decay, faction territory and officeholders as named pops; aggregated: anonymous pops as counts by cohort (PP-2), most fights as outcomes. History is an append-only event log with salience promotion (PP-4), hidden from the player until learned (MP-42) | estimate | MP-39, MP-42, PP-2, PP-4; 0062 item 3 |
| MP-57 | Late starts must be reproducible and stoppable: world seed plus history seed plus stop date give the same world (MP-40); the run shows progress and may be stopped and saved early (MP-40), because years of history can cost minutes to hours (MP-41) | estimate | MP-40, MP-41 |
| MP-58 | A ten-year pre-run should cost minutes, not hours: at a budget of 10 min for 10 years the world must run at about 526,000 times real time (5.26 million minutes in 10 years, 3,650 days), which only cold jump forms reach (spec 40) | derived | MP-41; 0062 Consequences |
| MP-59 | A joiner in a running world inherits a pop with a history and relations they did not make (MP-35); the possession screen should show that pop's recent log and standing orders before the player takes it | estimate | MP-35, MP-42 |
| MP-60 | Always-on worlds with no AI (Foxhole, Haven and Hearth) lose roles nobody wants (MP-7); a world where AI fills every seat (0061) lets players pick the roles they enjoy, as Stellaris and Bannerlord do in their own way, without the world stalling | estimate | MP-7, MP-34, MP-36 |

## What other games do, side by side

| Game | Who holds the roles | Empty role | Who controls time | Joining late |
| --- | --- | --- | --- | --- |
| Space Station 13 | Players by job; heads and Captain | Acting holder by written order, then vote (MP-2, MP-3) | Real time, one round | Joins the running round |
| Foxhole | Players only (FH-1) | Undone work (MP-7) | Always-on war | Joins the running war |
| EVE Online | Players; corporations by hierarchy (MP-9) | Nothing happens | Always-on; TiDi under load (MP-20) | Offline skill queue (MP-24) |
| Squad, Hell Let Loose | Players; Commander by vote or pick (MP-10, MP-12) | Team drifts (MP-13) | Real time, one match | Joins the running match |
| Paradox games | Players and AI per country | AI holds it (MP-34) | Host only (MP-25) | Takes an AI country (MP-34) |
| Factorio | Players | Nothing | Lockstep; server pauses when empty (MP-31) | Download and simulate to catch up (MP-30) |
| Dwarf Fortress | AI in every role during history | Not relevant | One player, history before play | Starts after 5 to 1,050 years of history (MP-38) |

## What this means for us (Proposal)

All rows here are `estimate` unless marked; they argue from the findings above.

### 1. The server model

- **One clock.** The speed is fixed when the world is created (0058 item 5). No one pauses a server world, which ends the host-speed fights of MP-25 to MP-27 (MP-48). Single player keeps the speed ladder and pause.
- **Overload.** If the server falls behind, the whole world slows in fine steps and recovers when load drops, like TiDi (MP-20) but global, because there is one clock (MP-50). The current factor is shown to every player. A floor (EVE uses 10 %, MP-22) is a tuning number; below it the server must shed detail (lower hot sets) instead of slowing further.
- **No wall-clock timers in the rules.** Every timer is a game timer, so dilation never gives anyone an edge (MP-21, MP-50).
- **Empty server.** The world keeps running with no players (0061); unlike Factorio (MP-31) it does not pause. A server owner may stop the process, which freezes the whole world, not part of it.

### 2. Possession, chairs and leaving

- **Possession lock.** One player per pop (0062 item 2). The lock lasts until the player releases the pop or disconnects past a grace period.
- **AI in every empty chair.** On release the pop's AI resumes at once from the same planner state; there is no passive mode (MP-51).
- **Standing orders.** The player's last orders stay in force under the AI for a bounded window (EVE's 24 h queue as the model, MP-24); after that the AI may change them by its own judgement (MP-55).
- **Joining.** A joining player sees a list of pops their play style allows (0018): any unpossessed pop in a sandbox, or a random survivor. Before taking it they see its recent log, relations and standing orders (MP-59). Offices are taken only through the world's rules (succession, election, appointment, seizure), not from a menu, unless the play style says otherwise.
- **Network model.** Server-authoritative, sending each client only its pop's scope (0044). Lockstep catch-up does not scale to our world (MP-49).

### 3. Orders between players through the chain

- An order is a message: sender office, receiver office, content (programme, budget, site, target), channel, time sent. It travels by the communications that exist (0014); a broken channel delays or loses it (MP-16, MP-53).
- Each office has its channel, like the command net reserved to heads and squad leaders (MP-5, MP-11, MP-12).
- The receiver, player or AI, decides how to carry it out; the AI weighs it against its interests and its loyalty (MP-36, MP-37, MP-54). Refusal is possible and has consequences through the rules (demotion, removal, MP-4, MP-52).
- A superior may override a subordinate's marker or site choice (MP-14), but the override is itself an order that reaches the subordinate.
- Succession and acting rules are data per office type (MP-52): deputy, emergency order, vote among peers, hand-back on the holder's return.

### 4. Late starts: what is exact and what is aggregated

| Simulated exactly (jump forms, spec 40) | Aggregated |
| --- | --- |
| Ledger stocks: food, fuel, materials, ammunition | Anonymous pops as counts by cohort and place (PP-2) |
| Births and deaths as counts per cohort | Most fights, as outcomes with casualties |
| Infrastructure decay and repair | Daily routines |
| Faction territory, officeholders and their successions (named pops) | Minor events not promoted by salience |
| Outbreak spread at region level | |

- History is an append-only event log; pops and events are promoted to named records by salience (PP-4), and the player learns them through the world (MP-42).
- The pre-run is reproducible from world seed, history seed and start date, and can be stopped early and saved (MP-40, MP-57).
- Budget: about 10 minutes for 10 years means about 526,000 times real time (MP-58). That is a performance target for the macro systems, not a promise.

### 5. Tests

1. **Release and resume:** possess a mayor, give orders, disconnect; the AI keeps the orders for the window and then acts; the state hash matches a run where an AI made the same inputs at the same ticks (0061).
2. **Two players, one pop:** a second player's possession request on a locked pop is refused; after release it succeeds.
3. **Chain with a broken link:** president orders a plant; cut the radio between capital and region; the order does not arrive until communications return; the governor's AI does not act on it.
4. **Succession:** kill a governor; the deputy acts within one game day; when a new governor is appointed the acting holder hands back.
5. **Dilation:** load the server past its budget; the world slows uniformly; no timer completes earlier relative to another than in an undilated run.
6. **Pre-run reproducibility:** the same seeds and start date give the same state hash twice, on two thread counts.
7. **Pre-run budget:** ten years run in under the target on the reference machine; the progress display updates at least once per game month.
8. **No-player world:** a server with no players for a game month matches the single-player headless run with the same inputs.

## Open questions

1. Should a server world allow any pause at all (a vote, an admin pause), or only stopping the process? This note proposes none.
2. How long is the standing-order window (MP-55), and is it in game time or real time?
3. Can a player take an office directly on joining in a sandbox server, or only by the world's rules?
4. What is the dilation floor, and what detail is shed below it?
5. What does Space Station 13 do when no player picks Captain at round start, and how does job preference work? (tgstation wiki blocked, search budget ran out.)
6. Do Squad or Hell Let Loose ever put bots in empty seats on live servers? (Not searched.)
7. Does Barotrauma put bots into empty job slots in multiplayer, and what happens to a disconnected player's character? (Wiki blocked.)
8. Real timings for Dwarf Fortress history generation by world size and length were not found; a measured table would calibrate MP-58.
9. How Crusader Kings III handles a player leaving (AI takeover or not) was not found.

## Sources

- CCP, "Introducing Time Dilation (TiDi)", eveonline.com/news/view/introducing-time-dilation-tidi
- EVE University wiki, Time dilation: wiki.eveuniversity.org/Time_dilation; Management and CEO pages
- CCP, "More Queue Queue", eveonline.com/news/view/more-queue-queue; mmos.com/editorials/offline-training-a-blessing-or-a-curse
- Engadget, 2012-02-09, "EVE Online's time dilation keeping game in sync"
- PC Gamer, "Meet Tranquility"; Data Center Knowledge, "EVE Online: 30,000 Users on One Server Shard"
- Paradise Station wiki, Chain of Command: paradisestation.org/wiki/index.php/Chain_of_Command
- Monkestation wiki, Chain of Command; Beestation forum, "Restructuring Command Staff"; Giant Bomb, Space Station 13
- Squad wiki (gamepedia), Commander; Guild Order Squad SL fundamentals
- Guild Order, Hell Let Loose commander guide; SteelSeries Hell Let Loose roles guide; Tom's Guide on Hell Let Loose
- PlanetSide 2 wiki (fandom), Fireteam; Level1Techs forum, "Starting a Planetside 2 Platoon"
- Barotrauma wiki (gamepedia), Orders; Wikipedia, Barotrauma; Steam discussions app 602960
- Freak Hosting and ZAP-Hosting Altis Life guides; Steam discussions on Altis Life whitelisting
- ModDB, Persistent World mod; Steam discussions app 48700
- Massively OP and Life is Feudal wiki (fandom), personal claims; Steam discussions app 700030
- github.com/davue/paradoxspeedsync; Guild Order HOI4 multiplayer campaigns; Steam discussions apps 236850, 394360, 1158310; Paradox forum thread 1733377
- Factorio wiki, Multiplayer: wiki.factorio.com/Multiplayer; Factorio Friday Facts #415: factorio.com/blog/post/fff-415; Alt-F4 #41; Factorio forum FFF #415 thread
- RimWorld Multiplayer mod listing (top-mods.com); GameServerKings RimWorld multiplayer guide
- Steam discussions app 281990 (Stellaris); Guild Order, Stellaris player dropout and succession
- GameBanshee, Bannerlord dev blog "Kingdom Decisions"
- Dwarf Fortress wiki, DF2014 World generation; v0.31 and v0.34 advanced world generation; v0.34 Legends; Steam discussions app 975370
- Game Developer, "Q&A: Dissecting the development of Dwarf Fortress with creator Tarn Adams"
- Caves of Qud wiki, Sultan history: wiki.cavesofqud.com/wiki/Sultan_history
- Mark R. Johnson, Ultima Ratio Regum 0.9 posts (markrjohnsongames.com); ModDB URR 0.6 release
- GameWatcher, Crusader Kings 3 time period; Steam workshop discussion 2270547030
- RimWorld wiki, Scenario system: rimworldwiki.com/wiki/Scenario_system
- Our notes: [haven-and-hearth.md](haven-and-hearth.md), [foxhole.md](foxhole.md), [pop-simulation-and-persistence.md](pop-simulation-and-persistence.md), [layer-jumps-multiresolution.md](layer-jumps-multiresolution.md)
