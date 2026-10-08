# Hearts of Iron IV, as a reference for the Macro layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [macro layer](../design/macro-layer.md), [world systems](../design/world-systems.md), [0022](../decisions/0022-macro-layer-intent.md), [spec 33](../tier0/specs/33-establishments-and-chains.md), [spec 35](../tier0/specs/35-networks.md), [spec 39](../tier0/specs/39-orders-and-belief.md). **Evidence:** supply and conscription are checked by web search (eip.gg guides, Steam changelogs); battle plans and production are general knowledge, not checked. Paradox changes these systems between patches. **IDs:** HOI.

Why: [0022](../decisions/0022-macro-layer-intent.md) takes its feel for politics and decisions; the user listed it again. Part of [reference games by layer](reference-games-by-layer.md). Related: [war in other games](war-in-other-games.md).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| HOI-1 | Hearts of Iron IV is Paradox's World War II grand strategy game (2016); its scripts and data ship as readable text. | reported | general knowledge |
| HOI-2 | Supply (since patch 1.11) starts at the capital, travels by railway (which needs trains) to supply hubs, then by trucks to divisions; trucks come from the stockpile. | reported | https://eip.gg/hoi4/guides/heres-how-supply-works-in-hearts-of-iron-iv/ |
| HOI-3 | A supply hub holds as much as its railway allows, and an upgrade helps only if the whole line back to the capital is upgraded (weakest link). | reported | https://eip.gg/hoi4/news/hearts-of-iron-iv-patch-1-11-logistics-overhaul-no-step-back-dlc/ |
| HOI-4 | Manpower is a share of the population set mainly by the conscription law; each step costs political power; more soldiers means fewer workers and slower training. | reported | https://staging2.eip.gg/hoi4/guides/impact-of-laws |
| HOI-5 | Factories are assigned to production lines, and a line's efficiency grows the longer it runs. | reported | general knowledge |
| HOI-6 | The player draws battle plans (fronts and arrows) and generals' AI moves the divisions; divisions are built from templates of battalions and support companies. | reported | general knowledge |
| HOI-7 | National focus trees are authored trees of goals, each taking about 70 days, with political and military effects. | reported | general knowledge |
| HOI-8 | Political power is a currency for decisions; ideology popularity drifts and decides elections. | reported | general knowledge |
| HOI-9 | Proposal: take supply as the hard part of war, with a weakest link, through our networks (spec 35) and shipments (spec 33). | estimate | this note |
| HOI-10 | Proposal: take conscription with a real cost to the workforce: levy edicts take real pops out of real jobs, with no manpower pool separate from the people. | estimate | this note |
| HOI-11 | Proposal: take plans drawn by the player and carried out by commanders who are pops. | estimate | this note |
| HOI-12 | Proposal: do not take political power as a currency or authored national focus trees (they go against 0036, everything follows logic). | estimate | this note |

## What it is

Paradox's World War II grand strategy game (2016). Like Stellaris, its scripts and data ship as readable text.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Supply (since patch 1.11)** | Everything starts at the capital, travels by railway (which needs trains) to supply hubs, and from there by trucks to divisions. A hub holds as much as its railway allows, and an upgrade only helps if the whole line back to the capital is upgraded. Trucks come from the stockpile | Networks ([spec 35](../tier0/specs/35-networks.md)) and shipments ([spec 33](../tier0/specs/33-establishments-and-chains.md)); see also [Foxhole](foxhole.md) |
| **Manpower and conscription laws** | Manpower is a share of the population, set mainly by the conscription law; each step costs political power; more soldiers means fewer workers and slower training | Levy edicts that take real pops out of real jobs ([spec 39](../tier0/specs/39-orders-and-belief.md)) |
| **Production lines** | Factories assigned to lines; efficiency grows the longer a line runs | Establishments and recipes ([spec 33](../tier0/specs/33-establishments-and-chains.md)); workers' skill as practice |
| **Battle plans** | The player draws a front and arrows; generals' AI moves the divisions | Delegation: orders to commanders who are pops |
| **Division designer** | Templates of battalions and support companies | Unit templates from real people and equipment |
| **Focus trees** | A national tree of goals, each taking about 70 days, with political and military effects | Long orders and projects ([spec 34](../tier0/specs/34-projects.md)); authored trees go against "everything follows logic" ([0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md)) |
| **Political power and ideologies** | A currency for decisions; ideology popularity drifts and decides elections | Beliefs per pop that add up to opinion ([spec 39](../tier0/specs/39-orders-and-belief.md)) |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| Supply as the hard part of war, with a weakest link | Political power as a currency |
| Conscription with a real cost to the workforce | Authored national focus trees |
| Plans drawn by the player and carried out by commanders | Manpower as a pool separate from the people |

## Questions this leaves (candidates for decisions)

1. Does the Macro layer draw fronts and arrows, executed by commander pops, as its main war verb?
2. What limits how often a country can change its laws, if not a political power currency? Candidates: time, legitimacy as a belief, the cost of unrest.

## Sources

- Supply: https://eip.gg/hoi4/guides/heres-how-supply-works-in-hearts-of-iron-iv/ ; https://eip.gg/hoi4/news/hearts-of-iron-iv-patch-1-11-logistics-overhaul-no-step-back-dlc/
- Laws: https://staging2.eip.gg/hoi4/guides/impact-of-laws
