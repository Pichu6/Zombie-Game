# The demo flow: from globe to a driver and back

**Date:** 2026-10-09. **Status:** the user's description of a showcase, turned into a storyboard with a gap list. Not a decision. Statuses come from the specs index on origin/main `bd4707e` and a read of the code; check them before planning.

## The user's words (verbatim, lightly cleaned)

> "There's an app, like a Google Earth geo app kind of thing. It shows a menu with parameters; some are touched and a world is created. The user rotates the world, shows its cities, different on/off layers to see countries, biomes, etc. Then he goes to a city and leaves the camera still for a while on a street corner. We see the city activity, with vehicles and pedestrians coming and going. Then the sim is paused, a random vehicle is clicked, it displays its info, vehicle type, driver, etc. Then the driver is clicked and more info about him is displayed: name, work, profession, address, etc. Then he's possessed and the simulation resumed. Because the player is now driving, he needs to stop to pay attention to the pop info screen, like the health and status in PZ, so the player parallel parks the car. The traffic reacts to this. When parked he takes a look at the pop info: now he sees the perks positive and negative, state of the body, diet, background story (with clickable names), etc. Then he sees the missions or tasks the pop was doing: it says deliver X to X. Hovering or clicking expands it and shows there's a timer: delivered before time gives a bonus, in time normal pay, with delay perhaps a loss. Now the player decides not to play that, so he leaves the pop; the pop returns to the truck and his tasks. We are in a street view type of thing; we navigate the busy city before panning out, see the city above, cycle some overlays, like in Cities: Skylines (water pipes, electric lines, zoning, etc.). Then it goes back to normal and we see that there are many things going on in the city, places and situations needing attention. Because it's too much, we zoom out again and see the country from above, and are back to the beginning. That flow didn't require decision making at a station and was more of a spectator type of use."

## What it shows about the design

- **It is the observer path:** watch, select, possess, detach. No station is needed because nothing above the driver's own life is decided (0062 rules 1 and 4; detaching is [0066](../decisions/0066-player-states-embodied-detached-observer.md)). The player here is the admin of their own world (the **free** level).
- **Possessing the driver is playing his life** (Individual layer). Parking is an input, so the delay it causes is real: if the player leaves the pop, the delivery may now be late. Playing changes inputs; watching changes nothing (0061).
- **It walks through every detail level:**
  - the globe is cold;
  - the city is warm;
  - the street corner becomes hot because the camera rests there (attention, 0062 rule 4);
  - panning the busy city wakes chunks ahead (working set and prefetch, XD-133..XD-135);
  - zooming out lets them go back down.
- **It is one zoom continuum with no loading screens** ([0063](../decisions/0063-real-size-planet-and-the-zoom-continuum.md)).

## Storyboard and gaps

Status words: **Built** (code and tests), **Specified** (a spec, not built), **Decided** (a decision, no spec), **Researched** (a note only), **Missing**.

| # | Beat | Systems needed | Status |
| --- | --- | --- | --- |
| 1 | A menu of world parameters creates a world | Sandbox parameters; generators (grid, terrain, climate, land, countries, towns) | Generators **Built** (specs 30, 31, 43–49, 52); parameters exist on the `zs gen` command line; a menu is **Missing** |
| 2 | Rotate the globe, toggle layers (countries, biomes, cities) | Globe renderer, vector or raster layers, style table | Data **Built**; view is a raster BMP export only; the globe and layer toggles are **Missing** (renderer; [vector-graphics](../research/vector-graphics.md) §4) |
| 3 | Fly down to a city | Zoom continuum, city layout (streets, blocks, buildings) | Continuum **Decided** (0063); towns as data **Built** (spec 47, 48); city streets **Researched** only ([city-layout-generation](../research/city-layout-generation.md)) |
| 4 | Camera rests on a street corner: vehicles and pedestrians come and go | Pops on daily schedules (home, work, shops); vehicles; traffic; hot bubble around the camera | Pops, households, jobs and dwellings as data **Built** (28, 45–47, 51); daily movement across a city **Missing**; vehicles and traffic **Missing** (design: world vehicles and environment, Proposal); the hot bubble **Specified** (27) |
| 5 | Pause; click a vehicle: type, driver | Vehicle entity and card; inspector | Inspector for pops **Built** (2D window); vehicles **Missing** |
| 6 | Click the driver: name, work, profession, address | Pop identity, household, occupation, dwelling | Mostly **Built** as data (28, 45–47, 51); the card's links (address → house, employer → building) **Missing** |
| 7 | Possess the driver; resume | Possession, one player per pop | **Decided** (0062); controlling a pop on foot **Built** (spec 50, WASD); driving **Missing** |
| 8 | Parallel park; traffic reacts | Vehicle control, traffic flow that yields and queues around an obstacle | **Missing** (traffic research: LJ-9..LJ-12, HST-15, HST-16) |
| 9 | Pop panel: perks and flaws, body, diet, background with clickable names | Body (11–13), traits, lore and relations | Body **Built** (Tier 0); traits **Researched** ([pop-behaviour](../research/pop-behaviour.md)); lore and custom pops **Decided** (0040); a Project Zomboid-style panel **Missing** |
| 10 | The pop's task: deliver X to Y, with a timer: early bonus, on time, late loss | Contracts between establishments, money, deadlines | Establishments and chains, stocks and money **Specified** (32, 33); delivery contracts with deadlines **Missing** |
| 11 | Leave the pop; he walks back to the truck and his tasks | Detach; the pop resumes on its AI and standing orders | Delegation **Decided** (0019); detach **Proposal** (README) |
| 12 | Street view, navigate the busy city, pan out | Camera, streaming of chunks ahead | Renderer **Missing**; chunks **Specified** (27) |
| 13 | City from above; cycle overlays: water, power, zoning | Overlays per network; zoning | Power network **Built** at building scale (17) and **Specified** above it (35); noise and light overlays **Built** in 2D (16); water pipes and zoning **Missing** |
| 14 | Many places and situations need attention | An attention list per layer (fires, fights, shortages, crowds) ranked by urgency | Alerts **Built** (spec 40 rule 7, 0056); a city-level list **Missing** (beacon flags, XD-125, could feed it) |
| 15 | Zoom out to the country, back to the start | Zoom continuum, cold country view | Same as 2 and 3 |

## What the demo needs that nothing covers yet

1. **Vehicles and traffic across levels.** Cars near the camera as bodies that park, yield and queue. Traffic far away as flows on the street graph (min(demand, supply), the same rule as hordes, HST-143).
2. **Daily movement of pops across a city.** Home → work → shops on schedules, consistent between the street and the statistics (lazy pops placed by formula, RA-23).
3. **Delivery contracts.** A job with goods, a destination, a deadline and a pay rule (bonus early, loss late), owned by an establishment, paid from its ledger.
4. **The pop panel.** A Project Zomboid-style status panel with traits, body, diet and a background with clickable relations.
5. **Water network and zoning overlays.**
6. **An attention list** for a city: what needs looking at, ranked.
7. **The globe and city renderer**, with the vector map layers for the top of the zoom.

## Second flow: tracing an order from a soldier to its cause

The user's words (verbatim, lightly cleaned):

> "If I'm panning the sim, watching what's going on in layer 2 or mid zoomed in, and I stumble over a fight, I zoom out to the world to see better where I am, and perhaps country overlays to see who is who. Then go back in to see the fight. Maybe we see some vehicles, explosions, soldiers fighting. We pause the sim, select a soldier and see his orders somehow, by name, clicking perhaps, being able to trace the order: Colonel Titus gave the order to strike the enemy force; clicking his name would take me to him, perhaps to see the order came from above in another form, 'secure the north border', which could take me to some politician pop. Or a family in a town or city."

**How much detail orders need: none of the usual detail levels.** Orders are the cheapest thing in the simulation and the spine that keeps zooming coherent, so they can be kept exact at every level:
- **An order is a small record:**
  - issuer;
  - recipient (a unit or an office);
  - intent;
  - parent order;
  - cause events;
  - tick;
  - how it travelled (radio, courier).
- **It is stored once, at the level where it was given:** one record for a battalion, not one per soldier.
- **A soldier's order is derived, not stored:** his unit's current order plus his role. Only exceptions are stored: a personal order, or a refusal.
- **Estimate (our arithmetic, corrected by [PKC-151](../research/playing-knowing-commanding-moving.md)):** an army of 100,000 has about 5,000 units at all echelons (about 3,500 squads, 1,000 platoons, 350 companies, 85 battalions, 22 brigades, 7 divisions, 2 corps). At a few orders per unit per day that is about 15,000–25,000 records a day, 1–2 MB at 64 bytes each. Old orders can be pruned once nothing living descends from them, the same rule as Time Warp's fossil collection (RA-57).
- **The trace already exists for economic orders.** Spec 39 is built: every change carries its cause, `Edicts::trace` walks the chain, and beliefs carry what each pop was told. Military orders need:
  - an order kind with a parent order and an intent;
  - the chain of command as offices (0058).
- **The chain can cross from army to politics to an event:**
  - soldier ← sergeant ← Colonel Titus ("strike the enemy force at X") ← general ("secure the north border") ← minister ← president;
  - and the cause of the policy: a raid on a town that killed a family, reported by radio.
  - In knowledge mode the chain shows what each link *believed*. A war can start from a distorted rumour, and the trace shows it.
- **On the soldier's card:**
  - his order;
  - his commander's intent one and two levels up, which doctrine confirms ([PKC-94, PKC-95](../research/playing-knowing-commanding-moving.md));
  - a breadcrumb of names, each with Locate, Follow and Play as.

**Status:**
- Cause chain, trace and beliefs **Built** (spec 39).
- Military orders and the chain of command **Decided** in outline (0058, 0060).
- Their records and the soldier's card **Missing**.

## Third flow: the observer finds a story

The user's request (2026-10-09): to navigate the simulated world like Google Earth and find its stories: who knew whom and did what, like an ensemble show, all coherent with the world's places. The design is in [story-explorer](story-explorer.md). As a demo beat it would run like this:
1. From the globe, press **Lucky**.
2. Fly to a forming story.
3. **Read** its recap.
4. Open the **Board** to see how its cast knew each other.
5. Follow a crossing to a second story in another country.
6. **Watch** both intercut.
7. Possess one of the cast.

Status: chronicle, cause chains and beliefs **Built** (25, 39); threads, ties, scoring and story spots **Missing** (items 46–50 in [improvements-ranked](improvements-ranked.md)).

## Fourth flow: from a border fight to a detective's life

**Date:** 2026-10-10. **Status:** the user's description, turned into a storyboard with a gap list. Not a decision.

> "I'm scrolling the globe and see a fight going on at a border, perhaps two countries fighting. I feel like playing some action, so I go in. First I see what's going on, who is who, and choose where to get involved. I play in RTS, move some units and play as one. After shooting for a few minutes I see I made things somewhat worse, decide 'oops' and move on, so I decouple and keep panning the globe. I pass roads, fields, forests, and arrive at a small town. It's already night; I see a few lights, so I zoom in. It's a store open at night, and I see a robbery near it, the thief running away. I click on the thief and read his background. I learn he is being investigated by a detective from the nearest city, which I hadn't noticed. I select the detective and go to where he is: he's at home, doing his last chores of the day. I read his background and learn he's having an affair and trouble in his marriage. I feel bad for him and decide to try to fix his life: catch the thief and fix his marriage. I go to the isometric view (Project Zomboid style) and check him further: his tasks and itinerary. He goes to work at the police station Monday to Friday, 8 to 5, or whatever is standard for that work. I check the date: it's Sunday night. So I play, finishing the pop's tasks of the day. The next day I go to his office and sit at his desk. It's a station, so I see a city-management view of what a detective could do: pins on the city map for crimes, calls, and so on. I play for a while, get bored, leave the detective, zoom out and keep watching the simulation. All that flow requires a level of procedural generation lore-wise, especially for it to have coherence with the world around it."

**What it shows:**
- **Every layer and every player state in one session, chosen by curiosity, not by a mission.** The sequence:
  1. observer at the globe;
  2. City command (RTS) at the fight, then embodied in one soldier;
  3. out of the body, panning;
  4. observer over the town at night;
  5. selecting the thief, then the detective;
  6. embodied in isometric view for the detective's evening;
  7. at a station (the detective's desk);
  8. observer again.

  The rules for all of this exist: [0062](../decisions/0062-the-game-loop-attention-possession-and-late-starts.md), [0066](../decisions/0066-player-states-embodied-detached-observer.md), [0068](../decisions/0068-entering-a-world-the-home-pop-and-pins.md), [0058](../decisions/0058-offices-open-layers-single-player-is-multiplayer-with-ai.md).
- **The world never waits.**
  - The fight goes on, worse for the player's mistake ([0061](../decisions/0061-the-world-runs-without-the-player.md), [0048](../decisions/0048-realistic-time-default-commit-on-observation-and-pins.md)).
  - The detective's life goes on after the player leaves.
  - Driving him out of character may leave a strange case ([cases](cases-and-living-lore.md) §14).
- **The lore was there before the click, coherent across distance.** The thief in the town and the detective in the city were already linked by a case. The rules for that are in [pop lore](pop-lore-and-custom-pops.md) §7.

| # | Beat | Systems needed | Status |
| --- | --- | --- | --- |
| 1 | Scrolling the globe, a fight at a border between two countries | War at Macro: fronts, forces, a cause; an attention list | Countries **Built** (47–49, 52); war **Decided** as three views (0060) and **Researched** ([combat-resolutions](../research/combat-resolutions.md), [world-in-conflict](../research/world-in-conflict.md)); fronts and forces **Missing**; the attention list **Missing** (first flow, gap 6) |
| 2 | Go in: see what is going on, who is who, choose where to join | The front lifted into units and soldiers with names, ranks and sides; unit and side cards | Pop levels **Built** (28); units as rosters of pops with military lifepaths **Missing**; lifting consistent with Macro strength **Researched** ([EF](../research/coarse-layers-from-fine-rules.md)) |
| 3 | Move units in RTS | The City command view; squad orders | View **Decided** ([cameras and modes](cameras-modes.md), 0060); orders **Built** headless (39); squad orders **Missing** |
| 4 | Play as one soldier and shoot | Possession; firearms | Possession **Decided** (0062, 0068); firearms **Missing** (spec 20 keeps them out of Tier 0) |
| 5 | Things got worse: leave and pan on | Detach; the fight continues with the player's mistake in it | **Decided** (0066, 0068, 0061, 0048) |
| 6 | Pan over roads, fields and forests to a small town | Terrain, land use and roads at map zoom; the renderer | Terrain and land **Built** as data (31, 52); roads **Researched** ([city-layout-generation](../research/city-layout-generation.md)); renderer **Missing** |
| 7 | Night, a few lights: zoom in | Lights of homes and open businesses, visible from afar; opening hours | Light field **Built** (19); sky **Decided** (0064); hours and shifts named as next in spec 33; lights at map scale **Missing** |
| 8 | A store open at night, a robbery, the thief runs | Crime as an AI choice under pressure; witnesses; flight | **Proposal** ([cases](cases-and-living-lore.md) §5, §12); robbery **Missing**; running **Built** (50) |
| 9 | Click the thief: his background | Dossier with lifepath and hooks; names that are links | Dossier and lifepath **Built** (`zs gen town`); hooks **Proposal** ([pop lore](pop-lore-and-custom-pops.md) §3); links **Proposal** ([content tools](content-tools.md) §4.2) |
| 10 | A detective from the nearest city is investigating him | Police with jurisdiction; case records; derived criminal history; the link from the thief to his case | Police departments and officers **Built** as generator data (`data/gen/`); detectives, jurisdiction and cases **Missing**; rules in [pop lore](pop-lore-and-custom-pops.md) §7 (**Proposal**) |
| 11 | Go to the detective: at home on Sunday evening, doing chores | Calendar (day of the week); household and dwelling; where a pop is at any time | Households and dwellings **Built** (45, 51); calendar **Missing**; position as a function of time **Missing** (first flow, gap 2; RA-23) |
| 12 | His background: an affair, a troubled marriage | Bonds; secret ties as hooks with residue | **Proposal** ([inhabitants](inhabitants.md) bonds; [cases](cases-and-living-lore.md) §5); **Missing** in code |
| 13 | Decide to help: catch the thief, fix the marriage | The player's goals are the player's; the game gives the means | Activities **Decided** (0019, 0020) |
| 14 | Isometric view: his tasks and itinerary (Monday–Friday, 8–5); it is Sunday night | Isometric view; a weekly schedule per occupation; the date | View **Decided** ([cameras and modes](cameras-modes.md)); schedules and calendar **Missing** |
| 15 | Play out the rest of his day | Household chores as activities | Tier 0 actions **Built** (14); chores **Missing** |
| 16 | Next day at his desk: a city view of a detective's work (crime pins, calls) | A station per occupation; the police office's knowledge (reports, calls, open cases) as a map layer | Stations **Decided** (0058); the case board **Proposal** ([cases](cases-and-living-lore.md) §9); beliefs and reports **Built** (39); dispatch and calls **Missing** |
| 17 | Bored: leave, zoom out, keep watching | Detach; the detective goes on with his life | **Decided** (0066, 0068); strange cases may record it ([cases](cases-and-living-lore.md) §14) |

**Build order** (simulation first, headless, each slice testable):
1. **Calendar and weekly schedules per occupation.** Where any pop is at any time becomes a function of time and pop. This also closes the first flow's gap 2.
2. **Police and jurisdiction:** case records and a detective role, built on the town generator's police data.
3. **Crime under pressure:** a robbery, plus derived criminal history through hooks. This generalises the cases doc's first case ("who took the fuel").
4. **Bonds and secret ties** (the affair) with residue.
5. **A headless flow test:** `zs flow --seed N` picks a night robbery and prints the chain (thief → case → detective → household → schedule), then checks the click-order, distance and replay tests ([pop lore](pop-lore-and-custom-pops.md) §7).
6. **Stations per occupation:** the detective's desk view.
7. **War:** fronts, forces as rosters of pops, lifting into soldiers. It needs the EF method.

The views (globe, command, isometric) come with the renderer.

## A testing bonus

The demo is deterministic. Once it exists, record it as a replay (start state, seed, input log; spec 06) and run it in CI. The showcase then doubles as a regression test that walks every detail level, every promotion and demotion, and one possession.

## Research done for this flow

For the fourth flow (2026-10-10): [daily-lives-and-time-use](../research/daily-lives-and-time-use.md) (TU, schedules and the calendar), [services-by-settlement-size](../research/services-by-settlement-size.md) (SV, which services and who covers the rest), [institutions-as-queues](../research/institutions-as-queues.md) (IQ, police intake, reporting, times to outcome; the detective's desk) and [absence-and-missing-persons](../research/absence-and-missing-persons.md) (AB, what happens when a possessed pop goes missing).

[playing-knowing-commanding-moving](../research/playing-knowing-commanding-moving.md) (PKC) covers:
- **the gaps:** vehicles and traffic (PKC-154..PKC-156), daily schedules (PKC-157), delivery contracts (PKC-158), overlays and the attention list (PKC-159, PKC-160);
- **the order trace and the soldier's card** (PKC-149..PKC-153);
- **switching between characters and layers** (PKC-1..PKC-35).

[cognitive-load](../research/cognitive-load.md) (CLT) adds how to teach it: start as a fish tank at Individual (one body, a few needs), name the parts before they interact, then City and Macro as their own steps ([CLT-39](../research/cognitive-load.md)); the first found story as a worked example ([CLT-48](../research/cognitive-load.md)); measure each segment with a short effort rating ([CLT-47](../research/cognitive-load.md)).

## Suggested research to add (done, see above)

Next to the three topics already proposed (moving between characters and layers, showing what you know, how leaders emerge):

4. **Traffic and deliveries across levels:**
   - micro traffic near the camera (car following, lane changes, parking), meso and macro flows far away, and the hand-off between them (SUMO, MATSim, Aimsun, Cities: Skylines);
   - delivery contracts and deadlines in games (Euro Truck Simulator, Death Stranding, Factorio logistics);
   - how Cities: Skylines and similar games show overlays and "needs attention" lists.
