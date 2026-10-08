# Stellaris, as a reference for the Macro layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Why:** [0022](../decisions/0022-macro-layer-intent.md), [0026](../decisions/0026-country-creation-inheritance.md) and [0028](../decisions/0028-births-military-mystery-tech-leaders.md) already take its feel, its empire creation, its leaders and a tech tree whose shape is still open. Part of [reference games by layer](reference-games-by-layer.md). Its pop grouping since 4.0 is in [sim-scale](sim-scale-in-other-games.md).

**Evidence caveat:** the tech draw is checked by web search (fan wikis, forum threads); faction and strata details came back thin and partly from older versions, so they are kept general.

## What it is

Paradox's space grand strategy game (2016). Its scripts and data ship as readable text files for modders, which is the legal way to study its numbers.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Empire creation** | Choose a species, ethics, civics, an origin and a government; they shape what is easy or hard later | Country creation ([0026](../decisions/0026-country-creation-inheritance.md)); real profiles ([spec 49](../tier0/specs/49-real-country-profiles.md)) |
| **Tech as weighted cards** | No visible tree: each research slot offers about three options drawn at random, weighted by ethics, civics, traditions, past choices, scientists and a random part. Prerequisites raise the weight of what they lead to | The tech tree of [0028](../decisions/0028-births-military-mystery-tech-leaders.md): a draw from the keyed random stream ([spec 02](../tier0/specs/02-draw.md)), weighted by who the researchers are and what the country already knows |
| **Factions from pops** | Interest groups form from pops by their ethics; a content faction gives influence, an unhappy one makes trouble and may push for independence | Factions ([0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md)) formed from real pops' beliefs |
| **Pops and jobs** | Planets offer jobs; pops fill them; jobs produce resources | Establishments staffed by rosters ([spec 33](../tier0/specs/33-establishments-and-chains.md)) |
| **Leaders** | Scientists, governors, admirals and generals age, gain traits and die | Leaders as real pops ([0028](../decisions/0028-births-military-mystery-tech-leaders.md)) |
| **Sectors and governors** | The player hands a group of planets to the AI under a governor | Delegation of a region to a pop, with orders |
| **Events and a late crisis** | Anomalies and event chains; an end-game crisis | The outbreak is our crisis ([spec 38](../tier0/specs/38-outbreak.md)) |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| Tech offered as a few weighted options, not a visible tree | Resources that appear from abstract jobs; ours come from real chains |
| Factions that form from the people and make demands | Pops as counts with no one inside |
| Handing regions to governors | Authored event chains as the main source of story |

## Questions this leaves (candidates for decisions)

1. Is the [0028](../decisions/0028-births-military-mystery-tech-leaders.md) tech tree a weighted draw of a few options per field, with weights from the researchers who are actually working on it?
2. Do factions form automatically from shared beliefs, or are they founded by a leader pop?

## Sources

- Tech draws: https://guildorder.com/es/games/stellaris/wiki/tech-tree-progression ; https://steamcommunity.com/app/281990/discussions/0/4203492762822384259
- Factions (older preview): https://www.keengamer.com/articles/news/stellaris-announces-ethics-and-pop-changes/
