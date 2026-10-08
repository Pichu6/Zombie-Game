# SimCity, as a reference for the City layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [world systems](../design/world-systems.md), [world simulation](../design/world-simulation.md), [spec 33](../tier0/specs/33-establishments-and-chains.md), [spec 34](../tier0/specs/34-projects.md). **Evidence:** the 2013 agent behaviour was checked by web search (Wikipedia, EA's own explainer, patch coverage, a player's account on Kotaku); the classic mechanics are general knowledge. **IDs:** SC.

Why: the user listed it among the City-layer references. Part of [reference games by layer](reference-games-by-layer.md). Related: [low-resource city simulation](low-resource-city-simulation.md).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| SC-1 | In SimCity the player zones residential, commercial and industrial land, a demand meter (RCI) says which is wanted, and buildings grow by themselves | reported | general knowledge |
| SC-2 | Services (police, fire, schools) cover a radius; land value rises with services and falls with pollution and crime | reported | general knowledge |
| SC-3 | The budget sets tax rates per zone type, plus ordinances | reported | general knowledge |
| SC-4 | SimCity 2013 (GlassBox) used an agent-and-sink model: buildings are sinks that agents fill | reported | https://www.pcgamer.com/uk/simcity-inside-the-glassbox-engine |
| SC-5 | In SimCity 2013 Sims owned no home or job: each morning they went to the nearest open job, each evening to the nearest open house, crowding the first place until it filled | reported | https://kotaku.com/with-simple-ai-like-this-why-does-simcity-need-cloud-c-5990362 |
| SC-6 | The August 2013 traffic update made a Sim reserve a slot before setting out | reported | https://www.gamewatcher.com/2013-21-08-simcity-traffic-tuning-update-7-0-releases-august-22nd |
| SC-7 | Disasters (fires, floods, monsters) are answered by the city's systems | reported | general knowledge |
| SC-8 | The 2013 failure is the case for pops as persistent records with a home and a workplace of their own | estimate | this note |
| SC-9 | Take zoning as the City layer's main verb, but buildings are projects with budgets, not growth by themselves | estimate | this note |
| SC-10 | Take demand meters, computed from real shortages (homeless households, unfilled jobs, unsold goods) | estimate | this note |
| SC-11 | Take service coverage as a readout, but coverage comes from staff and travel, not a radius rule | estimate | this note |

## What it is

Maxis's city builder: SimCity (1989), 2000, 3000, 4 (2003) and the 2013 reboot on the GlassBox engine.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Zoning and RCI demand** | The player zones residential, commercial and industrial land; a demand meter says which is wanted; buildings grow by themselves | Zones and sectors ([world systems](../design/world-systems.md), [spec 48](../tier0/specs/48-people-sectors-countries.md)). A demand readout would be computed from real shortages: homeless households, unfilled jobs, unsold goods |
| **Land value and coverage** | Services (police, fire, schools) cover a radius; land value rises with services and falls with pollution and crime | Services as establishments with staff and reach ([spec 33](../tier0/specs/33-establishments-and-chains.md)) |
| **Budget and taxes per zone** | Tax rates per zone type; ordinances | Taxes as orders ([spec 39](../tier0/specs/39-orders-and-belief.md)) and money ([spec 32](../tier0/specs/32-stocks-goods-and-money.md)) |
| **GlassBox agents (2013)** | An agent-and-sink model: buildings are sinks that agents fill. Sims did not own homes or jobs: each morning they went to the nearest open job, each evening to the nearest open house, crowding the first place until it filled. The traffic update of August 2013 made a Sim reserve a slot before setting out | Our pops are persistent records with a home and a workplace ([spec 28](../tier0/specs/28-pop-levels.md), [spec 45](../tier0/specs/45-households-and-sectors.md)). The 2013 failure is the case for that choice |
| **Disasters** | Fires, floods, monsters; the city's systems respond | Hazards ([spec 04](../tier0/specs/04-hazard-event.md)); the outbreak ([spec 38](../tier0/specs/38-outbreak.md)) |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| Zoning as the City layer's main verb | Buildings that appear by themselves; ours are projects with budgets ([spec 34](../tier0/specs/34-projects.md)) |
| Demand meters, computed from real shortages | Agents without homes or jobs of their own |
| Service coverage as a readout | Coverage as a radius rule; ours is staff and travel |

## Questions this leaves (candidates for decisions)

1. Does the City layer zone land, as SimCity does, with projects following, or does it only place projects?
2. Which shortages make up a demand readout, and is it shown in Macro too?

## Sources

- GlassBox: https://en.wikipedia.org/wiki/SimCity_(2013_video_game) ; https://www.pcgamer.com/uk/simcity-inside-the-glassbox-engine ; https://www.ea.com/en-gb/news/simcity-workers
- Nearest-job behaviour and the fix: https://kotaku.com/with-simple-ai-like-this-why-does-simcity-need-cloud-c-5990362 ; https://www.gamewatcher.com/2013-21-08-simcity-traffic-tuning-update-7-0-releases-august-22nd
