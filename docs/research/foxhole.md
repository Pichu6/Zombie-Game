# Foxhole, as a reference for war and logistics

**Date:** 2026-10-06. **Status:** research note, not a decision. **Feeds:** [sandbox parameters](../design/sandbox-parameters.md) §5.1 and §8, [world systems](../design/world-systems.md), [0029](../decisions/0029-leader-records-foxhole-tech-readout.md). **Evidence:** first written from general knowledge; see "Checked on 2026-10-06" below for what was verified. Check results come from web-search summaries of the pages cited (the wiki itself could not be fetched directly from the research environment), so treat numbers as secondary until read on the wiki. **IDs:** FH.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| FH-1 | Foxhole is a persistent online war between two factions where every bullet, weapon, vehicle and drop of fuel is produced and supplied by real players. | reported | https://www.pcgamer.com/foxholes-latest-update-adds-mass-production-to-fuel-the-mmo-war/ |
| FH-2 | A war ends when a side holds enough victory towns (32 on the full map by default), then a 15-minute celebration, then a Resistance Phase of usually 1 or 2 days before a new war. | reported | https://foxhole.wiki.gg/wiki/World_Conquest |
| FH-3 | Long wars are reported at about 55.8 days (War 100) and 71 days (Charlie 9); typical length was not found. | reported | https://www.pcgamesn.com/foxhole/longest-deadliest-war |
| FH-4 | The production chain is raw resources, then materials at a Refinery, then supply crates (weapons, ammo, tools, medical, base supplies) at a Factory. | reported | https://foxhole.wiki.gg/wiki/Community_Guides/Logistics |
| FH-5 | Mass Production Factories, in the largest industrial towns far from the front, make bigger batches at a discount and can make vehicles in bulk. | reported | https://www.pcgamer.com/foxholes-latest-update-adds-mass-production-to-fuel-the-mmo-war/ |
| FH-6 | Storage Depots and Seaports hold crates; a Seaport's public stockpile holds 1000 of each crate type, 100 of them shippable. | reported | https://foxhole.wiki.gg/wiki/Seaport |
| FH-7 | A Relic Base is the very durable home base where players spawn and rearm; owning it owns its region. | reported | https://foxhole.wiki.gg/wiki/Relic_Base |
| FH-8 | Supply cutting and sabotage can win battles; a rule for how a front "holds" was not found. | reported | https://foxhole.wiki.gg/wiki/Community_Guides/Logistics |
| FH-9 | The front moves as both sides capture enemy town halls while frontline builders extend trenches and bases behind the infantry; a precise capture rule was not found. | reported | https://en.wikipedia.org/wiki/Foxhole_(video_game) |
| FH-10 | Regiments persist between wars with up to 2000 members (update 1.61); creating one needs a minimum rank; squads are separate, max 40. | reported | https://foxhole.wiki.gg/wiki/Regiment |
| FH-11 | Foxhole is reported to support up to about 4000 players in one world; the per-side split was not found. | reported | https://foxhole.wiki.gg/wiki/Regiment |
| FH-12 | Players are divided on whether logistics is fun: a repetitive time sink (macros and autoclickers) for some, enjoyable for others who run public logistics groups. | reported | https://steamcommunity.com/app/505460/discussions/0/3164335241626298436/ |
| FH-13 | Logistics as the war (armies limited by the scarcest input), fronts as places with depots, roads and staff, and resources made into items through real stages all fit our ledger and production ladder. | estimate | this note |
| FH-14 | Foxhole's end-and-reset does not fit our one continuous world; take only the pressure it creates. | estimate | this note |

## Why it is relevant

The user named Foxhole as a game to study for gameplay and concept of war ([0029](../decisions/0029-leader-records-foxhole-tech-readout.md)). Our wish is military power that counts in the simulation, with combat gameplay later ([sandbox parameters](../design/sandbox-parameters.md) §5.1).

## What it is (general knowledge)

A persistent online war between two factions where almost everything is player-made: players haul, produce, build and fight; wars last weeks and then reset.

## Ideas worth studying

| Idea | Why it matters to us |
| --- | --- |
| Logistics is the war: front-line fights are fed by hauling and production chains | Fits "nothing easy, everything real" and the production ladder; armies are limited by the scarcest input |
| Facilities and supply networks decide where a front can hold | A front is a place with depots, roads and staff, not a number |
| Resources become items through real stages | Same ledger and assembly rules as our items |
| Regiments (player groups) with ranks | Our groups and leadership models ([sandbox parameters](../design/sandbox-parameters.md) §8) |
| Building and destruction by players | Breachable, rebuildable things, the same as ours |
| War ends and resets | **Does not** fit us (one continuous world); take only the pressure it creates |

## To investigate before specifying war

1. How the supply chain steps are shaped and how many there are.
2. How the front moves and what makes an attack succeed.
3. What a single soldier's job is at each scale, and what a commander sees.
4. How the game keeps large numbers readable.

Findings go here as dated notes; decisions go in `docs/decisions/`.

## Checked on 2026-10-06

Method: web search; direct fetches of foxhole.wiki.gg and fandom were blocked, so each line below is what the search summary of the cited page said. Numbers were not read first-hand on the wiki.

| Claim in this note | Result | Source |
| --- | --- | --- |
| Persistent online war, almost everything player-made | Confirmed: "every bullet, weapon, vehicle, and drop of fuel is produced and supplied by real players". | https://www.pcgamer.com/foxholes-latest-update-adds-mass-production-to-fuel-the-mmo-war/ |
| Wars last weeks and then reset | Confirmed, with detail: a war ends when a side holds enough victory towns (32 on the full map by default); then a 15-minute celebration, then a Resistance Phase of usually 1 or 2 days before a new war. | https://foxhole.wiki.gg/wiki/World_Conquest , https://foxhole.wiki.gg/wiki/Short_War , https://www.foxholegame.com/about-foxhole |
| War length | Long outliers reported: War 100 about 55.8 days; a later war (Charlie 9) 71 days. Typical length not found. | https://www.pcgamesn.com/foxhole/longest-deadliest-war |
| Resources become items through real stages | Confirmed: raw resources -> materials at a Refinery -> supplies (crates of weapons, ammo, tools, medical, base supplies) at a Factory. Mass Production Factories (added to the largest industrial towns) make bigger batches at a discount, far from the front, and can make vehicles in bulk. | https://foxhole.wiki.gg/wiki/Community_Guides/Logistics , https://www.pcgamer.com/foxholes-latest-update-adds-mass-production-to-fuel-the-mmo-war/ |
| Facilities and supply networks decide where a front can hold | Partly confirmed: Storage Depots and Seaports hold crates (Seaport public stockpile: 1000 of each crate type, 100 shippable crates of each); a Relic Base is the home base where players spawn and rearm, is very durable, and owning it owns its region; supply cutting and sabotage can win battles. How a front "holds" as a rule: not found. | https://foxhole.wiki.gg/wiki/Seaport , https://foxhole.wiki.gg/wiki/Relic_Base , https://foxhole.wiki.gg/wiki/Community_Guides/Logistics |
| How the front moves | Both sides capture enemy town halls; frontline builders extend trenches and bases behind the infantry advance. A precise capture rule: not found. | https://en.wikipedia.org/wiki/Foxhole_(video_game) , https://foxholeacademy.blogspot.com/2021/07/how-to-urban-defenses.html |
| Regiments (player groups) with ranks | Confirmed and corrected: regiments are persistent between wars, up to 2000 members (update 1.61); creation needs a minimum rank (sig for Colonials, cpl for Wardens); squads are separate, max 40. | https://foxhole.wiki.gg/wiki/Regiment , https://grokipedia.com/page/Regiments_in_Foxhole |
| Scale | Reported to support up to about 4000 players in one world. Per-side split: not found. | https://foxhole.wiki.gg/wiki/Regiment (search summary) |
| Is logistics fun? | Divided: some find it a repetitive time sink (macros and autoclickers mentioned), others enjoy it and run public logistics groups. | https://steamcommunity.com/app/505460/discussions/0/3164335241626298436/ |

Still open from the list above: what a commander sees (not found), and how large numbers stay readable (not found beyond the map of towns and regions). See [war in other games](war-in-other-games.md).
