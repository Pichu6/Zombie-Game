# War in other games, as a reference

**Date:** 2026-10-06. **Status:** research note, **not a decision**. **Feeds:** [sandbox parameters](../design/sandbox-parameters.md) §5.1, [world systems](../design/world-systems.md), [world numbers](../design/world-numbers.md), [0033](../decisions/0033-restoring-cost-and-two-way-projects.md). **Evidence:** web search only. Direct page fetches were blocked in the research environment, so every claim is from a search summary of the cited page and was not read first-hand. Several points come from player discussions, which are opinion, not documentation. "Not found" means the searches did not return it. Gladius was not searched. **IDs:** WAR.

War is secondary for now ([sandbox parameters](../design/sandbox-parameters.md) §5.1); this gathers what other games do.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| WAR-1 | Foxhole supports up to about 4000 players per world, regiments of up to 2000, and wars of weeks (outliers 55 to 71 days) that then reset; it has no aggregate layer, the war is the sum of players' acts. | reported | https://foxhole.wiki.gg/wiki/Regiment |
| WAR-2 | Hearts of Iron IV combat width is about 80 per fight, 50 in mountain and marsh; divisions come from player-designed templates. | reported | https://www.pcgamesn.com/hearts-of-iron-iv/meta-division-templates |
| WAR-3 | Hearts of Iron IV supply runs from the capital by rail to supply hubs, extended by trucks; bigger divisions need more supply, so players favour many smaller fully supplied divisions. | reported | https://eip.gg/hoi4/guides/heres-how-supply-works-in-hearts-of-iron-iv/ |
| WAR-4 | Victoria 3 battalions are 1000 workers; armies are assigned to fronts along borders through a general; barracks are standing, conscription centers temporary; mobilization raises military buildings' consumption. | reported | https://vic3.paradoxwikis.com/Warfare_overview |
| WAR-5 | Stellaris base naval capacity is 20, ship cost runs from 5 (corvette) to 80 (titan); going over the cap raises upkeep by the same percentage. | reported | https://stellaris.paradoxwikis.com/Ship |
| WAR-6 | Stellaris compresses fleet strength into one fleet-power number that misleads matchups, and its war score confuses players; wars end by war score or exhaustion. | reported | https://guildorder.com/games/stellaris/guides/fleet-design-and-counters |
| WAR-7 | Crusader Kings 3 armies are mostly levies (peasant conscripts) plus hired men-at-arms with upfront price and monthly upkeep; battles resolve automatically over days until one side breaks. | reported | https://ck3.paradoxwikis.com/Army |
| WAR-8 | Total War lets a battle be played or auto-resolved from unit strength, general, army size and balance; losses carry back, and replenishment (base 8% a turn) happens only on the campaign map. | reported | https://steamcommunity.com/app/1142710/discussions/0/601916800388894711/ |
| WAR-9 | Bannerlord battle size is 200 to 1000 troops (engine limit about 2048 actors, mounts count); auto-resolve uses troop priority and tactics, and the medic skill turns some deaths into wounds. | reported | https://steamcommunity.com/app/261550/discussions/0/844004758333188365/ |
| WAR-10 | Players of Total War and Bannerlord report that auto-resolve costs more losses than playing, which pushes them to play every fight. | reported | https://steamcommunity.com/app/1142710/discussions/0/6171701201027183765/ |
| WAR-11 | PlanetSide 2 launched with up to 2000 players per 64 km2 continent, reduced to 1200 for performance; a lattice of connected regions drives a persistent campaign with no aggregate layer. | reported | https://cmdrcyrious.com/planetside2/beginner-guides/continents/ |
| WAR-12 | Hell Let Loose (50v50) has commander and squad leaders manage fuel and munitions; Arma Reforger has players truck supplies to bases (Everon island 51 km2); matches carry nothing back. | reported | https://nerdburglars.net/arma-reforger-hell-let-loose-or-squad-which-game-is-right-for-you/ |
| WAR-13 | Supreme Commander allows up to about 1000 units per player on maps to 80 x 80 km, with one camera zooming from a unit to the whole map and icons at distance. | reported | https://en.wikipedia.org/wiki/Supreme_Commander_(video_game) |
| WAR-14 | Dwarf Fortress siege squads (16, later close to 100 mounted goblins plus trolls) are drawn from real world populations, so each dead attacker is one fewer, and sieges escalate with those survived. | reported | https://dwarffortresswiki.org/index.php/23a:Siege |
| WAR-15 | Battle Brothers switches from worldmap contact to a turn-based tactical battle; logistics is money, rations, armor parts, ammo and medical supplies. HighFleet constrains fleets by fuel and crew morale. | reported | https://battlebrothersgame.com/strategic-worldmap/ |
| WAR-16 | Played fights in these games stay in the hundreds to low thousands of bodies; campaign layers abstract to divisions, battalions, fleets and levies. | derived | this note (from WAR-2, WAR-4, WAR-9) |
| WAR-17 | Whether logistics is fun is divided in Foxhole's community: a repetitive time sink with macros for some, rewarding for others. | reported | https://steamcommunity.com/app/505460/discussions/0/3164335241626298436/ |
| WAR-18 | Hearts of Iron IV frontline AI constantly redeploys divisions and leaves gaps. | reported | https://steamcommunity.com/app/394360/discussions/0/2961670087544364807/ |
| WAR-19 | Auto-resolve and played fights must use the same inputs, or players learn that one is a trap. | estimate | this note |
| WAR-20 | Carry real losses and wounds back to the same pop records, and let treatment turn some deaths into wounds. | estimate | this note |
| WAR-21 | Draw enemy forces from the other side's real pops, so attrition is finite. | estimate | this note |
| WAR-22 | Supply should bind decisions (width, range, size) rather than add chores; hauling stays optional or batched. | estimate | this note |
| WAR-23 | Show strength as a breakdown (men, equipment, fuel, supply, morale), not one number. | estimate | this note |
| WAR-24 | Cap the played fight at a few hundred bodies; everything else stays aggregate. | estimate | this note |
| WAR-25 | Make fronts and supply lines visible on the map, since a watcher reads places, not stats. | estimate | this note |
| WAR-26 | A war needs an end condition that is not annihilation (such as exhaustion), without a hard reset. | estimate | this note |

## Comparison

| Game | Scale | What a unit is | How aggregate and individual connect | Logistics model | Source |
| --- | --- | --- | --- | --- | --- |
| Foxhole | Persistent war, up to about 4000 players per world; regiments up to 2000; wars last weeks (outliers 55 to 71 days), then reset | A real player | No aggregate layer: the war is only the sum of players' acts; a player's items come from other players' production | Refinery, Factory, Mass Production Factory, then hauling to depots, seaports and front bases | https://www.pcgamer.com/foxholes-latest-update-adds-mass-production-to-fuel-the-mmo-war/ , https://foxhole.wiki.gg/wiki/Regiment |
| Hearts of Iron IV | Divisions built from battalions; combat width about 80 per fight, 50 in mountain and marsh | Division from a player-designed template | Not played at body level; the front is moved by AI or by orders; no individual layer | Supply from the capital via rail to supply hubs, range extended by trucks; trains and equipment stockpiles; big divisions need more supply | https://www.pcgamesn.com/hearts-of-iron-iv/meta-division-templates , https://eip.gg/hoi4/guides/heres-how-supply-works-in-hearts-of-iron-iv/ |
| Victoria 3 | Battalions of 1000 workers; fronts along borders | Battalion; general with traits | Armies assigned to a front via a general; no tactical layer | Barracks (standing) and conscription centers (temporary, active in war); mobilization raises consumption of military buildings | https://vic3.paradoxwikis.com/Warfare_overview , https://forum.paradoxplaza.com/forum/threads/victoria-3-dev-diary-23-fronts-and-generals.1497106/ |
| Stellaris | Fleets; base naval capacity 20; ship cost 5 (corvette) to 80 (titan) | Ship, valued in naval capacity | Fleet power is one number; war ends by war score or exhaustion, not by the fights alone | Naval capacity over the cap raises upkeep by the same percentage | https://stellaris.paradoxwikis.com/Ship , https://steamcommunity.com/app/281990/discussions/0/599638172337480190/ |
| Crusader Kings 3 | Levies (peasant conscripts, most of any army) plus hired men-at-arms | Regiments of troop types, each with Attack and Defense | Battle is automatic over days until one side breaks; no played battle | Levies are raised from the realm; men-at-arms cost an upfront price and monthly upkeep | https://ck3.paradoxwikis.com/Army , https://www.thegamer.com/crusader-kings-3-how-does-the-combat-battle-system-work/ |
| Total War (Warhammer III as example) | Campaign armies, tactical battles with unit blocks | Unit (a block of entities) | Either play the battle or auto-resolve from unit strength, general rating, army size, balance; losses carry back; replenishment happens only on the campaign map (base 8% a turn) | Campaign upkeep and replenishment; no battle-level supply | https://steamcommunity.com/app/1142710/discussions/0/3832045251564133986/ , https://steamcommunity.com/app/1142710/discussions/0/601916800388894711/ |
| Mount & Blade: Bannerlord | Battle size setting 200 to 1000 troops (engine limit about 2048 actors, mounts count) | A named-tier soldier in a party | Auto-resolve (a simulation) uses troop priority and the tactics skill; medic skill turns some deaths into wounds; players report simulated battles cost more losses than played ones | Wages, food, party upkeep | https://steamcommunity.com/app/261550/discussions/0/844004758333188365/ , https://gist.github.com/jzebedee/be076d28f162c8d05fd7d2d72109a46f |
| PlanetSide 2 | Up to 2000 players per 64 km2 continent at launch, reduced to 1200 for performance (as reported); lattice of regions; continent locking | A real player | Persistent campaign by region capture; no aggregate layer | Not found beyond the lattice (regions must connect) | https://cmdrcyrious.com/planetside2/beginner-guides/continents/ , https://steamcommunity.com/app/218230/discussions/0/1698294337777105091/ |
| Hell Let Loose, Squad, Arma Reforger | 50v50 sector-control matches (HLL); Arma Everon island 51 km2 | A real player | Matches are self-contained, no carry-back (not found otherwise) | HLL: commander and squad leaders manage fuel and munitions, build garrisons; Arma Reforger: players truck supplies to bases, called more complicated than HLL | https://nerdburglars.net/arma-reforger-hell-let-loose-or-squad-which-game-is-right-for-you/ , https://steamcommunity.com/app/1874880/discussions/0/693122272400046966/ |
| Supreme Commander, Wargame | Up to about 1000 units per player (SupCom), maps to 80 x 80 km; Wargame over 1200 unit types | A vehicle or a platoon-sized card | One camera: strategic zoom from a single unit to the whole map, with icons at distance | Wargame: supply trucks refuel and rearm units; SupCom: not found | https://en.wikipedia.org/wiki/Supreme_Commander_(video_game) , https://wargame.fandom.com/wiki/Wargame:_Red_Dragon |
| Dwarf Fortress | Siege squads of 16, later close to 100 goblins mounted, plus trolls | A full individual creature | Armies are pulled from real settlements and populations of the world; each dead goblin is one fewer; escalates with sieges survived | None (sieging is by presence) | https://dwarffortresswiki.org/index.php/23a:Siege , https://steamcommunity.com/app/975370/discussions/0/3727324132820887081/ |
| Battle Brothers | A mercenary company, one tactical map per fight | A named brother | Worldmap contact switches to a turn-based tactical battle | Money, rations, armor parts, ammo, medical supplies | https://battlebrothersgame.com/strategic-worldmap/ |
| HighFleet | A fleet on a strategic map | Ship | Not found | Fuel and crew morale constrain movement | https://en.wikipedia.org/wiki/HighFleet |

Gladius: not searched.

## Findings by focus question

1. **Bridging aggregate and individual.** Total War and Bannerlord are the closest models: the same army can be auto-resolved from stats or played, and both carry losses back; both draw complaints that auto-resolve is worse than playing (https://steamcommunity.com/app/1142710/discussions/0/6171701201027183765/ , https://steamcommunity.com/app/261550/discussions/0/2264689848044028717/). Total War limits replenishment to the campaign map (https://steamcommunity.com/app/1142710/discussions/0/601916800388894711/). Bannerlord's medic skill turns deaths into wounds. Dwarf Fortress is the model for drawing attackers from real populations. Foxhole and PlanetSide 2 have no aggregate layer at all.
2. **Unit counts.** Played fights stay in the hundreds to low thousands of bodies (Bannerlord 200 to 1000, engine cap about 2048; HoI4 fights of about 80 width). Campaign layers abstract to divisions, battalions of 1000, fleets, levies.
3. **Fun logistics.** Not documented directly. Foxhole's own community is split: repetitive for some, rewarding for others, with macro use reported (https://steamcommunity.com/app/505460/discussions/0/3164335241626298436/). HoI4 players favor many smaller fully supplied divisions over big ones that run dry, so supply shapes decisions rather than adding chores (https://www.pcgamesn.com/hearts-of-iron-iv/meta-division-templates). Foxhole's mass production factory was added to fuel the war, for batching (https://www.pcgamer.com/foxholes-latest-update-adds-mass-production-to-fuel-the-mmo-war/).
4. **Readability for a watcher.** Not found as a documented design. Observed devices: fronts as border lines (Vic3), one scalar for strength (Stellaris, with the caveat below), strategic zoom with icons (Supreme Commander).
5. **What to avoid (as reported).** HoI4 frontline AI constantly redeploys divisions and leaves gaps (https://steamcommunity.com/app/394360/discussions/0/2961670087544364807/). Stellaris fleet power compresses matchups into a number that misleads (https://guildorder.com/games/stellaris/guides/fleet-design-and-counters), and war score confuses players (https://steamcommunity.com/app/281990/discussions/0/501693985385899372/). Auto-resolve that is worse than play pushes players to play every fight.

## Lessons for us

- Auto-resolve and played fights must use the same inputs, or players learn that one is a trap (Total War, Bannerlord complaints above).
- Carry back real losses and wounds to the same pop records; give treatment a chance to turn deaths into wounds (Bannerlord medic skill).
- Draw enemy forces from the other side's real pops, so attrition is finite (Dwarf Fortress).
- Make supply bind decisions (width, range, size) rather than add chores; keep any hauling optional or batched (HoI4, Foxhole mass production).
- Show strength as a breakdown (men, equipment, fuel, supply, morale), not one number (Stellaris fleet power).
- Cap the played fight to a few hundred bodies; everything else stays aggregate (Bannerlord, HoI4 width).
- Make fronts and supply lines visible on the map, since a watcher reads places, not stats (Vic3 fronts, Foxhole depots and bases).
- Give the war an end condition that is not annihilation, such as exhaustion (Stellaris), but without a hard reset (Foxhole resets; we do not).

## Questions this leaves for the design

1. Does the played fight resolve a slice of the campaign (one front, one night) or the whole engagement, and what ends it?
2. How are casualties assigned when aggregate-resolved: random draws over pops, weighted by role and armor?
3. Should aggregate results be allowed to be better or worse than expected play, or must their expected values match?
4. How much logistics should the player touch directly (hauling) versus set as policy?
5. What does a watching player see: a front on the map, a feed of events, a unit list, or all three?
6. What ends a war: exhaustion, objectives, collapse of loyalty or supply?
