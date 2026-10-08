# World in Conflict, as a reference for command

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [cameras and modes](../design/cameras-modes.md), [interaction](../design/interaction.md), [spec 39](../tier0/specs/39-orders-and-belief.md), [spec 32](../tier0/specs/32-stocks-goods-and-money.md). **Evidence:** checked by web search (Wikipedia, Giant Bomb, reviews); the mechanics are as reported there, not verified in play. **IDs:** WIC.

The user listed it among the City-layer references. It fits the commander layer that opens as the player finds others ([vision](../vision.md)). Part of [reference games by layer](reference-games-by-layer.md). Related: [war in other games](war-in-other-games.md), [Foxhole](foxhole.md).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| WIC-1 | World in Conflict (Massive Entertainment, 2007) is a real-time tactics game with no base building and no resource gathering | reported | https://en.wikipedia.org/wiki/World_in_Conflict |
| WIC-2 | Each player spends a budget of reinforcement points on units, which arrive by air about 20 seconds later | reported | https://en.wikipedia.org/wiki/World_in_Conflict |
| WIC-3 | A lost unit's reinforcement points come back, so a replacement can be bought | reported | https://en.wikipedia.org/wiki/World_in_Conflict |
| WIC-4 | Tactical aid points are earned mostly by destroying enemies and holding command points, and spent on off-map support (artillery, airstrikes, paratroops, up to a tactical nuke) | reported | https://en.wikipedia.org/wiki/World_in_Conflict |
| WIC-5 | Capturing and fortifying command points on the map, and holding them, wins the match | reported | https://en.wikipedia.org/wiki/World_in_Conflict |
| WIC-6 | Each player takes one role (infantry, armour, air or support) and needs the others | reported | https://giantbomb.com/wiki/Games/World_in_Conflict |
| WIC-7 | Unit abilities (smoke, target marking, repair) each have a recharge | reported | https://en.wikipedia.org/wiki/World_in_Conflict |
| WIC-8 | Take: no base building in battle; the fight is about using what you have | estimate | this note |
| WIC-9 | Take: "call support" as a command verb, from real assets (a battery, an aircraft with fuel and crew, called by radio) | estimate | this note |
| WIC-10 | Take: roles, splitting command between pops by speciality (delegating a squad or sector to another pop) | estimate | this note |
| WIC-11 | Do not take: points that refund losses; a lost unit is lost people, so nothing comes back | estimate | this note |
| WIC-12 | Do not take: points earned by killing; units come from a levy and stocks, and objectives are places that matter for real reasons (bridge, depot, hospital) | estimate | this note |

## What it is

Massive Entertainment's real-time tactics game (2007), set in a Soviet invasion of the United States in 1989. No base building and no resource gathering.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Reinforcement points** | Each player has a budget of points and spends it on units, which arrive by air about 20 seconds later. A lost unit's points come back, so a new one can be bought | Units come from real people and stocks: a levy ([spec 39](../tier0/specs/39-orders-and-belief.md)) and equipment from stocks ([spec 32](../tier0/specs/32-stocks-goods-and-money.md)). A lost unit is lost people, so nothing comes back |
| **Tactical aid** | Points earned mostly by destroying enemies and by holding command points; spent on off-map support: artillery, airstrikes, paratroops, up to a tactical nuke | Support that exists somewhere: a battery, an aircraft with fuel and crew, called by radio |
| **Command points** | Capture and fortify points on the map; holding them wins | Places that matter for real reasons: a bridge, a depot, a hospital |
| **Roles** | Each player takes one role: infantry, armour, air or support, and needs the others | Delegation: the commander gives a squad or a sector to another pop, AI or later a co-op player |
| **Unit abilities** | Smoke, target marking, repair, each with a recharge | Actions with real durations and stocks (smoke grenades are items) |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| No base building in battle: the fight is about using what you have | Points that refund losses |
| Support calls as a command verb, from real assets | Points earned by killing |
| Roles: split command between pops by speciality | |

## Questions this leaves (candidates for decisions)

1. Is "call support" a verb of the commander layer, with the asset, its travel time and its ammunition shown?
2. When the player commands a group, can they hand a role (scouts, guards, builders) to another pop as a sub-commander?

## Sources

- https://en.wikipedia.org/wiki/World_in_Conflict ; https://giantbomb.com/wiki/Games/World_in_Conflict
