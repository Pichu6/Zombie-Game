# Second Life, as a reference for ownership

**Date:** 2026-10-08. **Status:** research note, not a decision. **Why:** [0025](../decisions/0025-sandbox-engine-first-game.md) names Second Life-type worlds for the range of activities; the user listed it again. What it does best is ownership, which is still a gap in [layer jumps](../design/layer-jumps.md) (row 19). Part of [reference games by layer](reference-games-by-layer.md).

**Evidence caveat:** checked by web search against the Second Life wiki and knowledge base; some pages are from the Linden-era forums and may be dated.

## What it is

Linden Lab's virtual world (2003). Users build and script almost everything in it, own land, and buy and sell items for Linden dollars.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Permissions on every item** | Four permissions: move, modify, copy, transfer, set separately for the owner, group, everyone and the **next owner**. A copy keeps its creator and can never be more permissive than its source. Example: clothing that buyers can change and copy but not resell | Ownership by scope ([spec 32](../tier0/specs/32-stocks-goods-and-money.md)). Next-owner rights map to real ones: lent, rented, given, held in trust, confiscated |
| **Creator kept on every item** | Each object remembers who made it | Provenance: who made an item and with what quality ([items and crafting](../design/items-crafting.md)) |
| **Regions and parcels** | A region is 256 m × 256 m, run by one simulator process. A parcel is part of a region, owned by one user or group, made of 4 m × 4 m blocks, with its own access rules and settings | Building plots, owned by a pop, household, firm or state; access rules for who may enter or build. One region per process is a partition idea ([threading and partition](threading-and-partition.md)) |
| **Group ownership** | Land and items can belong to a group with roles | Owners by scope: household, establishment, region, country ([spec 32](../tier0/specs/32-stocks-goods-and-money.md)) |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| Rights separate from possession: who owns, who holds, who may use, who may pass on | Real-money trade; copying items (we have no duplication: everything is a real stock) |
| A creator and quality recorded on made things | User scripting |
| Parcels with an owner and access rules | |

## Questions this leaves (candidates for decisions)

1. Does layer jumps row 19 (ownership) take a Second Life-style split: owner, holder, and rights to use, lend and sell?
2. How do rights survive the end of the state that enforced them, when an outbreak brings everything down to "who holds it"?

## Sources

- Permissions: https://wiki.secondlife.com/wiki/Permission ; https://community.secondlife.com/knowledgebase/english/object-permissions-r57/
- Regions and parcels: https://wiki.secondlife.com/wiki/Viewerhelp:Types_and_sizes_of_land
