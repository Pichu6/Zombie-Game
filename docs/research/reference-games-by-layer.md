# Reference games by layer: which mechanic, from which game, for which layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [interaction](../design/interaction.md), [layer jumps](../design/layer-jumps.md), [macro layer](../design/macro-layer.md), [spec 24](../tier0/specs/24-pop-ai.md), [spec 39](../tier0/specs/39-orders-and-belief.md), [0012](../decisions/0012-world-simulation-layers.md). **Evidence:** written from general knowledge of the games, checked by web search on 2026-10-08 where a fact was specific (numbers, dates, named systems). Direct page reads were not attempted; the proxy blocks many sites (see the other notes). Each game note lists its sources and marks what is reported. **IDs:** RG.

The user listed the games whose mechanics they want in Juego Zombie: Project Zomboid, Haven and Hearth, GTA, Shenmue, The Sims, inZOI and Second Life at the Individual layer; SimCity, Cities: Skylines and World in Conflict at the City layer; Stellaris, Hearts of Iron and TEG ("Plan Táctico y Estratégico de la Guerra") at the Macro layer. This note is the map. Each game has its own note with the details.

**How we study them (and how we don't).** We study a game's mechanics from how it plays and from public sources: wikis, developer talks and diaries, and the script files games ship for modders. We never decompile code and never copy code, art, text or names. Copyright covers code and assets, not mechanics, so a mechanic rebuilt from scratch on our own simulation is ours. Copying how a whole game looks and plays is still a risk (the Tetris v. Xio and Spry Fox v. 6waves cases), which is one more reason to take single mechanics rather than whole games. Their code also wouldn't fit: each of these games fakes the layers it doesn't simulate, and our one simulation under all three layers ([0012](../decisions/0012-world-simulation-layers.md)) is the part none of them has.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| RG-1 | Project Zomboid's moodles summarise the body's state in a few icons, and the game tells you how you died; the model for a readable body lens and our death screen. | reported | [project-zomboid](project-zomboid.md) |
| RG-2 | Haven and Hearth uses real durations, progress bars and quality softcaps. | reported | [haven-and-hearth](haven-and-hearth.md) |
| RG-3 | GTA escalates police response from witnesses, offers side activities and uses the phone as the menu. | reported | [gta](gta.md) |
| RG-4 | Shenmue puts every person and shop on a schedule, with real weather, jobs and mini-games. | reported | [shenmue](shenmue.md) |
| RG-5 | The Sims uses smart objects that advertise what they give, with needs-weighted autonomy. | reported | [the-sims](the-sims.md) |
| RG-6 | inZOI has karma that adds up from one person to a whole city, and lets you read a character's thoughts. | reported | [inzoi](inzoi.md) |
| RG-7 | Second Life puts ownership and permissions on every item and parcel. | reported | [second-life](second-life.md) |
| RG-8 | SimCity zones by demand; SimCity 2013's agents had no home of their own, a lesson in cutting corners on persistence. | reported | [simcity](simcity.md) |
| RG-9 | Cities: Skylines has district policies and services that must travel; its death waves came from cutting corners on age. | reported | [cities-skylines](cities-skylines.md) |
| RG-10 | World in Conflict has no base building: units arrive from a pool, support strikes are earned, and each player has one role. | reported | [world-in-conflict](world-in-conflict.md) |
| RG-11 | Stellaris has empire creation, tech drawn as weighted cards, and factions formed from pops. | reported | [stellaris](stellaris.md) |
| RG-12 | Hearts of Iron IV supplies over rails and hubs, takes manpower from conscription law, and runs battle plans by AI generals. | reported | [hearts-of-iron](hearts-of-iron.md) |
| RG-13 | TEG plays on a simple map of 50 countries with secret objectives and pacts between players. | reported | [teg](teg.md) |
| RG-14 | Each of these games fakes the layers it doesn't simulate (Sims rabbit holes, GTA disposable people, Stellaris pops as counts, TEG tokens); we take only the player-facing mechanic, rebuilt on our one simulation. | estimate | this note |
| RG-15 | The best mechanics are readable summaries of a deeper state (moodles, demand meters, faction approval, wanted stars): the models for the lens each layer needs. | estimate | this note |
| RG-16 | Our persistent pops with ages from real tables already avoid SimCity 2013's homeless agents and Cities: Skylines' death waves; golden tests should keep it so. | estimate | this note (from RG-8, RG-9) |
| RG-17 | Mechanics are studied from play and public sources and rebuilt from scratch; code, art, text and names are never copied, and taking single mechanics rather than whole games avoids look-and-feel risk (Tetris v. Xio, Spry Fox v. 6waves). | estimate | this note |
| RG-18 | Candidate decisions: a moodle-style body summary in the window; smart objects with advertisements for pop AI; witnesses, not a meter, for crime; permissions on ownership; weighted tech draws; secret objectives and pacts for Macro factions. | estimate | this note |

## The map

| Layer | Game | The mechanic worth studying | Our counterpart | Note |
| --- | --- | --- | --- | --- |
| Individual | Project Zomboid | Moodles: a few icons that summarise the body's state; "this is how you died" | A readable lens on our body model; the death screen ([spec 26](../tier0/specs/26-run.md)) | [project-zomboid](project-zomboid.md) |
| Individual | Haven and Hearth | Real durations, progress bars, quality softcaps | [0049](../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md) | [haven-and-hearth](haven-and-hearth.md) (2026-10-07) |
| Individual | GTA | Escalating police response from witnesses; side activities; the phone as the menu | Belief and orders ([spec 39](../tier0/specs/39-orders-and-belief.md)); activities ([0019](../decisions/0019-activities-and-delegation.md)) | [gta](gta.md) |
| Individual | Shenmue | Every person and shop on a schedule; real weather; jobs and mini-games | Rosters ([spec 28](../tier0/specs/28-pop-levels.md)); weather ([spec 36](../tier0/specs/36-fields-and-weather.md)); activities | [shenmue](shenmue.md) |
| Individual | The Sims | Smart objects that advertise what they give; needs-weighted autonomy | [interaction](../design/interaction.md) (Smart Objects); pop AI ([spec 24](../tier0/specs/24-pop-ai.md)) | [the-sims](the-sims.md) |
| Individual | inZOI | Karma that adds up from one person to a whole city; reading a character's thoughts | Beliefs and reputation ([spec 39](../tier0/specs/39-orders-and-belief.md)) | [inzoi](inzoi.md) |
| Individual | Second Life | Ownership and permissions on every item and parcel | Ownership (layer jumps row 19, a gap) | [second-life](second-life.md) |
| City | SimCity | Zoning and demand; the 2013 lesson: agents with no home of their own | Zones and sectors ([world systems](../design/world-systems.md)); persistent pops | [simcity](simcity.md) |
| City | Cities: Skylines | District policies; services that must travel; the death-wave lesson | Orders per region; networks ([spec 35](../tier0/specs/35-networks.md)); cohorts ([spec 44](../tier0/specs/44-child-mortality.md)) | [cities-skylines](cities-skylines.md) |
| City, Individual | World in Conflict | No base building: units arrive from a pool; earned support strikes; one role per player | The commander layer; levies and stocks ([spec 32](../tier0/specs/32-stocks-goods-and-money.md)) | [world-in-conflict](world-in-conflict.md) |
| Macro | Stellaris | Empire creation; tech drawn as weighted cards; factions formed from pops | [0026](../decisions/0026-country-creation-inheritance.md), [0028](../decisions/0028-births-military-mystery-tech-leaders.md) | [stellaris](stellaris.md) |
| Macro | Hearts of Iron IV | Supply over rails and hubs; manpower from conscription law; battle plans run by AI generals | Networks; levy edicts ([spec 39](../tier0/specs/39-orders-and-belief.md)); delegation | [hearts-of-iron](hearts-of-iron.md) |
| Macro | TEG | A simple map of 50 countries; secret objectives; pacts between players | Factions ([0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md)); faction AI goals | [teg](teg.md) |

## The pattern across all of them

1. **Each game fakes the layers it doesn't simulate.** The Sims hides work in a "rabbit hole", GTA spawns disposable people around the player, Stellaris pops are counts, TEG armies are tokens. We can't take those shortcuts, because our layers must agree ([0012](../decisions/0012-world-simulation-layers.md)). What we take is the *player-facing* mechanic, rebuilt on the one simulation.
2. **The best mechanics are readable summaries of a deeper state.** Moodles, demand meters, faction approval and wanted stars are all small views on a big state. Our window is hard to read today ([status](../status.md)); these are the models for the lens each layer needs.
3. **The bad lessons are as useful as the good ones.** SimCity 2013's agents without homes and Cities: Skylines' death waves both came from cutting corners on persistence and age. Our pops are persistent records with ages drawn from real tables, so both are already avoided. Golden tests should keep them that way.

## Candidates for decisions

Each game note ends with its own questions. The ones most likely to matter soon:

1. **A moodle-style body summary in the window** (Zomboid), as the first fix for "hard to tell what is going on".
2. **Smart objects with advertisements** (The Sims) as the shape of pop AI choice, built on the utility steps of [spec 24](../tier0/specs/24-pop-ai.md).
3. **Witnesses, not a meter** (GTA): crime and its response flow through beliefs and the police institution.
4. **Permissions on ownership** (Second Life) to close layer jumps row 19.
5. **Weighted tech draws** (Stellaris) as the shape of the tech tree [0028](../decisions/0028-births-military-mystery-tech-leaders.md) left open.
6. **Secret objectives and pacts** (TEG) for Macro factions, AI and player alike.
