# Cities: Skylines, as a reference for the City layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [world systems](../design/world-systems.md), [spec 35](../tier0/specs/35-networks.md), [spec 39](../tier0/specs/39-orders-and-belief.md), [spec 43](../tier0/specs/43-global-tables.md). **Evidence:** death waves checked by web search (player guides, Steam threads, a 2023 developer preview of the sequel); the rest is general knowledge of the game, not checked against sources. **IDs:** CS.

The user listed it among the City-layer references. Its citizen, unit and agent split is in [sim-scale](sim-scale-in-other-games.md) and [low-resource city simulation](low-resource-city-simulation.md); this note covers the play. Part of [reference games by layer](reference-games-by-layer.md).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CS-1 | Cities: Skylines is Colossal Order's city builder (Paradox, 2015), with a sequel in 2023; traffic is the heart of it. | reported | general knowledge |
| CS-2 | Districts and policies: the player paints a district and switches policies on in it (smoke ban, free transport, high-rise ban), each with a cost and an effect. | reported | general knowledge |
| CS-3 | Roads form a hierarchy; the player draws bus, metro and train lines; traffic jams come from real trips. | reported | general knowledge |
| CS-4 | Services travel: garbage trucks, hearses, fire engines and ambulances drive to each building, so a blocked road means uncollected dead. | reported | general knowledge |
| CS-5 | Death waves: every resident gets a fixed lifespan from the day it moves in; a large area zoned at once fills with adults who die together years later, overwhelming hearses and cutting taxes. | reported | https://en.number13.de/cities-skylines-how-to-deal-with-death-waves/ |
| CS-6 | Players work around death waves by zoning in small batches; a mod randomises starting ages. | reported | https://www.nexusmods.com/citiesskylines/mods/58 |
| CS-7 | The developers said in 2023 that the sequel avoids death waves. | reported | https://techraptor.net/gaming/news/cities-skylines-2-developers-reveals-infamous-death-waves-are-gone-in-new-video-about |
| CS-8 | Population milestones unlock buildings and services. | reported | general knowledge |
| CS-9 | Take: district policies as the City layer's second verb after zoning, acting only through ordinary commands below them. | estimate | this note |
| CS-10 | Take: services that travel, so roads matter; and transport lines drawn by the player. | estimate | this note |
| CS-11 | Do not take: fixed lifespans set on arrival; our pops get ages from real tables and deaths from real mortality, and migrants arrive with mixed ages. | estimate | this note |
| CS-12 | Do not take: milestone unlocks; what a town can build follows its people, money and materials. | estimate | this note |

## What it is

Colossal Order's city builder (Paradox, 2015), with a sequel in 2023. Traffic is the heart of it.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Districts and policies** | Paint a district, then switch policies on in it (smoke ban, free transport, high-rise ban); each has a cost and an effect | Orders per region ([spec 39](../tier0/specs/39-orders-and-belief.md)): an edict that acts only through ordinary commands below it |
| **Roads and transport lines** | A road hierarchy; bus, metro and train lines drawn by the player; traffic jams from real trips | Networks ([spec 35](../tier0/specs/35-networks.md)); travel is still a gap (layer jumps row 18) |
| **Services that must travel** | Garbage trucks, hearses, fire engines and ambulances drive to each building; a blocked road means uncollected dead | Services are staff and vehicles that travel; a jammed road has consequences |
| **Death waves** | Every resident gets a fixed lifespan from the day they move in. Zone a large area at once and everyone moves in together, all adults, and dies together some years later: skulls everywhere, hearses overwhelmed, taxes down. Players zone in small batches; a mod randomises starting ages. The developers said the sequel avoids it | Our pops get ages from real tables and deaths from real mortality ([spec 43](../tier0/specs/43-global-tables.md), [spec 44](../tier0/specs/44-child-mortality.md)). Arrivals in our towns should keep that: migrants of mixed ages |
| **Unlocks by population** | Milestones unlock buildings and services | Nothing: what a town can build follows its people, money and materials |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| District policies as the City layer's second verb, after zoning | Fixed lifespans set on arrival |
| Services that travel, so roads matter | Milestone unlocks |
| Transport lines drawn by the player | |

## Questions this leaves (candidates for decisions)

1. Does the City layer draw transport lines, or only build roads and leave the lines to establishments?
2. A golden test: a town that fills a new district all at once must not show a death wave in its age pyramid later.

## Sources

- Death waves: https://en.number13.de/cities-skylines-how-to-deal-with-death-waves/ ; https://techraptor.net/gaming/news/cities-skylines-2-developers-reveals-infamous-death-waves-are-gone-in-new-video-about ; https://www.nexusmods.com/citiesskylines/mods/58
