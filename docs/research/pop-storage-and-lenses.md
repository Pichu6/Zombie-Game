# Pop simulation, saving data, and "lenses" across zoom

**Date:** 2026-10-07. **Status:** research note, not a decision. **Asked by the user:** keep researching pop simulation and methods to save data; how to apply "different lenses" on the zoom layers (not only for pops, for everything in the simulation); look for engineering-quality videos and write-ups. Feeds [focus and compression](../design/focus-and-compression.md), [zoom continuum](../design/zoom-continuum.md), [spec 28](../tier0/specs/28-pop-levels.md), [spec 29](../tier0/specs/29-embodied-pops.md), [layer jumps](../design/layer-jumps.md). Earlier notes this builds on: [sim-scale-in-other-games](sim-scale-in-other-games.md), [layer-jumps-multiresolution](layer-jumps-multiresolution.md), [low-resource-city-simulation](low-resource-city-simulation.md).

**Evidence caveat (as in the other notes):** every primary page I tried (GDC schedule, Uber, CMU, arXiv, Factorio forum, the Stack Overflow blog) was **blocked by the proxy**. All findings are from **web-search summaries**, so numbers are "reported" and details of a paper or talk are second-hand. Sections marked *our synthesis* are my reading, not a source's claim. Talks and papers are listed in section 6 so the user (or a later session with open access) can read or watch the originals.

## 1. Short answers

1. **Agents:** the proven pattern is the one we already chose, in three tiers: a few **real** agents near the player, many **cheap** ones whose behaviour is simplified or scheduled, and the rest **counts**; a pool swaps agents between tiers without the player noticing (section 2).
2. **Saving:** nobody stores everything. The recurring methods are **seeds that regenerate**, **exceptions to the generated value**, **delta and relative encoding**, **independent chunks**, **compressed integer sets**, **event log plus snapshots**, and **state hashes to prove nothing drifted** (section 3).
3. **Lenses:** the idea has well-studied relatives in four fields: **semantic zoom** (user interfaces), **multiple-representation databases and generalisation** (cartography), **hierarchical spatial indexes** (H3) and **read models and projections** (software architecture, "CQRS"). Together they say how to build one lens per layer for *any* domain (sections 4 and 5).
4. The cartography literature has a warning that supports the user's rule directly: **no single representation of a thing works at every scale** ("the fallacy of the golden feature"), so each layer's view must be designed for its purpose, not made by shrinking the finest one.

## 2. Simulating many agents: level of detail

| Source | What it reports | Fits our |
| --- | --- | --- |
| **Kingdom Come: Deliverance 2**, GDC talk by Petr Smrček (Warhorse) | About 2,400 NPCs on the map (four times the first game), around half in one city, each doing a daily routine **even when the player is far away**; AI LOD solutions, memory and frame-time management, threads, "moving standardised logic into code", and knowing what can be stripped in each case; target 60 FPS | Our cold/warm/hot ([spec 27](../tier0/specs/27-detail-levels.md)); daily routines for lazy pops |
| **Assassin's Creed Unity**, GDC 2015, François Cournoyer (Ubisoft): "AI Recycling" | AI level of detail with a **pooling system** that swaps low-resolution NPCs for high-resolution ones **without the player noticing**; a limit of 40 real AIs and 120 high-res models gave 10,000 crowd NPCs on screen | The embodied pool and its hysteresis ([spec 29](../tier0/specs/29-embodied-pops.md)); also the "sample to show" rule |
| **Hitman: Absolution**, GDC 2013 (IO Interactive) | Techniques for crowds of 1,200 characters at 30 fps | Crowd budgets |
| **Simulation level of detail** (CMU, "for multiagent control"; "for virtual humans", 2007) | Decrease the cost of out-of-view agents by lowering update rate, simplifying or skipping path planning, jumping to destinations; a **scheduler gives more time to visible and near agents**; at distance even collision avoidance can be switched off unseen | Spec 27 jump forms; one scheduler for the speed ladder |
| **Representative agents in epidemic models** (EpiRust and others) | Small numbers of agents "scaled up" to stand for more; a published scaling-and-shifting method matched a 12.8-million model with a 1-million-agent one in about 12.8 times less time; EpiRust itself reached about 3.2 million agents | Outbreak aggregates (layer-jump row 14). *Caution:* shown for branching processes whose proportions settle; it is not a method for a city's daily life |

*Our synthesis:* all of these keep a **bounded set of fully simulated agents chosen by attention**, and treat the rest by a rule that is cheap, deterministic and can hand over in either direction. That is the shape of specs 27 to 29. What the sources add: a **pool** with fixed budgets (AC Unity), a **time scheduler weighted by visibility** (simulation LOD papers) and **routines that continue off-screen** (KCD2).

## 3. Methods to save data

| Method | What the sources report | Fit for us |
| --- | --- | --- |
| **Seeds, not data** | No Man's Sky generates planets continuously on demand from maths, not stored content (GDC 2017, Sean Murray; "Continuous World Generation in No Man's Sky") | Already our rule: keyed draws ([spec 02](../tier0/specs/02-draw.md)); fields on the nested grid |
| **Exceptions to the generated value** | (Our spec 28 rule 3; the same idea as storing only edited chunks) | Keep; applies to terrain, buildings, pops, stocks |
| **Relative and small-width encoding** | Factorio's savegame shrink: positions saved as vectors **relative to the last saved position, 2 bytes instead of 4**; decorative entities cut to 4 bytes each, several small chunks of them packed together (before: about 144 bytes per entity) | Positions inside a cell stored as small offsets from the cell ([zoom continuum](../design/zoom-continuum.md) §4) |
| **Independent compressed chunks** | Minecraft's Anvil region files hold up to 1,024 chunks, each compressed on its own with zlib at the fastest level; chunks are vertical sections of 16×16×16 blocks. Known waste: **empty sections stored explicitly, redundant cached data (height maps) saved, and fragmentation gaps** after updates | Chunked saves that wake independently ([spec 27](../tier0/specs/27-detail-levels.md)); lessons: store nothing that is the default, nothing that can be recomputed, and avoid in-place growth |
| **Delta transfer of changed sections** | Mods send only changed sections and merge them | Save deltas per chunk against the last full snapshot |
| **Compressed sets of integers** | Roaring bitmaps (Lemire et al.): a set of 32-bit integers kept as packed arrays or bitmaps, **whichever is smaller**, in 2^16 containers; often about 2× smaller than run-length alternatives and much faster to intersect (up to 900× reported) | Rosters (who works here, who lives here, who is in this region) and "which pops match" queries over ids |
| **Event log plus snapshots, with projections** | Event sourcing: the log is the truth; **projections** (read models) are built from it and **materialised** so reads are cheap; snapshots avoid replaying from the start | Our save-and-replay ([spec 06](../tier0/specs/06-save-replay.md)); a lens is a projection (section 4) |
| **Persistent structures, structural sharing** | New versions share unchanged parts; copying a container is O(1); cheap snapshots, undo, branching | Cheap quick-saves and "preview what happens if" forks (the Macro forecast preview) |
| **Deterministic simulation and state hashes** | RTS lockstep keeps clients identical by running the same deterministic simulation and checksumming state **every turn**, so a drift is found at the turn it happens; floating point is "the most common culprit" across compilers and optimisation levels | Already ours: strict floating point, detmath, golden hashes pinned across compilers |
| **Data-oriented layout** | Mike Acton (CppCon 2014): organise data for the cache and for transformations, not for convenience; Overwatch (GDC 2017, Timothy Ford) runs gameplay as an ECS, reported as about 46 systems over 103 component types on the client, with netcode reduced to a few systems; ECS stores either as **archetypes** (fast iteration, costly add and remove) or **sparse sets** (cheap add and remove, more cache misses) | Pop columns stored as arrays per field, hot and cold split; archetype versus sparse set is a choice for the engine spec (stable ids favour sparse set) |
| **Merge the small** | Victoria 3 pops merge small groups; RimWorld and CK3 cull unimportant records ([sim-scale](sim-scale-in-other-games.md)) | Spec 28 demotion rule; record count budget (failure mode 15) |

*Our synthesis for pops:* a pop is **an id plus a seed plus a short list of exceptions**; most of its life is a function of those and of time; groups are **compressed id sets** (rosters); only the embodied few hold a full state. The save is the exceptions, the sets, the event log and the world seed.

## 4. Lenses across zoom: four existing ideas that are the same idea

| Field | Idea | What it says about our lenses |
| --- | --- | --- |
| **Semantic zoom** (Perlin and Fox, Pad, 1993; Pad++, Jazz, Piccolo) | An object **changes how it looks, and what it shows, as its size and the view change**: zoomed out a simplified form, zoomed in more detail; plus **portals and lenses** that look at part of the surface | The zoom continuum is semantic zoom; a lens is a portal onto one thing. Confirms the layer is a *rendering of the same record* chosen by view |
| **Multiple-representation databases (MRDB) and generalisation** (cartography) | The **same phenomenon is stored at several levels of thematic and geometric detail in one database, with the objects at different levels linked**; derived by operations such as **selection, simplification, aggregation, exaggeration and symbolisation**; and the "**fallacy of the golden feature**": no single representation serves all scales and purposes | Our links between levels are ids and sums; the operations are our compression techniques (C1 to C10). The fallacy is the user's rule: each layer's information must be designed, not shrunk |
| **Hierarchical hex index (Uber H3)** | The Earth cut into hexagons at **16 resolutions (0 to 15)**; resolution 0 has 122 cells (12 pentagons), each hexagon has 7 children (aperture 7), the finest cells are under 1 m²; moving between resolutions is cheap (bit operations); data is **aggregated into the parent cell where it is sparse** | The same family as our grid ([spec 30](../tier0/specs/30-world-grid.md), aperture 4, exactly nested, 12 pentagons). A cell id that encodes its level makes "aggregate to the parent" a bit operation: worth copying for ids |
| **CQRS read models and materialised views** | The **write side** holds the truth; **projections** build separate **read models** shaped for each reader; a **materialised view** is computed once when the event is processed, so reads are cheap | A lens is a read model. Macro, City and Individual are three read models of one truth; sums are materialised views kept up to date incrementally |
| **Interest management** (MMO servers; Unreal replication graph; spatial hashing) | Each observer **subscribes to the keys it cares about** (or to the neighbouring grid cells) and receives only that; also **hides state from observers outside the region** | Pins are subscriptions; attention levels are the interest set; **fog follows for free** (a layer only receives what its observer may know) |

## 5. A lens for every domain (our synthesis, a table to check, not a decision)

Truth is the finest level, or its seed and exceptions. Each layer reads a **projection**, with its own purpose.

| Domain | Truth (stored) | Macro lens | City lens | Individual lens | Compression |
| --- | --- | --- | --- | --- | --- |
| **Pops** | id, seed, exceptions, event log | Counts by age, work, wealth, loyalty | Rosters, households, a sample as dots | A body with senses, beliefs, ties | Id sets, seeds, exceptions |
| **Terrain, climate** | World-cell fields; seed for detail | Biome, climate, capacity | Land use, zones, rivers as lines | Ground, water, weather at the spot | Amplification below the cells |
| **Stocks, goods** | Ledger by owner (kg, litres) | Province totals, scarcity | Warehouse and shop levels, flows | Items on shelves and in hands | Totals as sums; items made on a visit |
| **Money, prices** | Ledger, market state | Budget, GDP, inflation | Company accounts, wages | Wallet, price at the till | Sums and rates |
| **Buildings** | Plan seed, ledger, exceptions | Stock count by type | The building at its stage | Walkable interior | Generated on approach, saved on first visit |
| **Networks** (power, water, roads) | Component graph | Capacity, load, outages | District graph, faults | Switch, tap, wire | Hierarchical graph, summed capacity |
| **Weather, fields** | Coarse fields plus sources | Climate and forecasts | Local conditions | What a body feels, hears, smells | Coarsen and refine, mean preserved |
| **Flora, fauna** | Stock per cell, seed | Stocks, harvest, depletion | Forests, fields, herds | A tree, an animal, a hunt | Individuals drawn from the stock on approach |
| **Outbreak** | Individual infection states (lazy ones as counts) | Compartments, reproduction number | Infected rosters, clinics | Wounds, symptoms, a turned body | Counts in cold; entities near the player |
| **Territory, politics** | Sector owner, controller, claims | Borders, wars, treaties | Local authority, unrest | Orders reaching me, rumours | Event records |
| **Information, belief** | Beliefs per pop (source, age) | Headlines, what the player knows | Local rumours | What this person believes | Beliefs only for followed pops; the rest derived |

## 6. Talks, videos and papers worth the time (engineering quality)

Ordered by how directly they help. Links are from the searches; the proxy blocked reading most of them here.

| Kind | Title | Why |
| --- | --- | --- |
| Talk | [Supporting Thousands of NPCs in Kingdom Come: Deliverance and KCD2](https://schedule.gdconf.com/session/supporting-thousands-of-npcs-in-kingdom-come-deliverance-kingdom-come-deliverance-ii/915120) (Petr Smrček, Warhorse; also on [YouTube](https://www.youtube.com/watch?v=yMlTT-yqdmc)) | The closest match to lazy pops with routines at game scale: AI LOD, memory, frame time |
| Talk | [Combining GOAP and MBTs to Create NPCs' Behaviors for KCD2](https://gdcvault.com/play/1035576/Game-AI-Summit-Combining-GOAP) (GDC AI Summit) | How the routines are authored |
| Talk | [Massive Crowd on Assassin's Creed Unity: AI Recycling](https://gdcvault.com/play/1022141/Massive-Crowd-on-Assassin-s) (GDC 2015) | The pool that swaps cheap and real agents invisibly |
| Talk | [Data-Oriented Design and C++](https://isocpp.org/blog/2015/01/cppcon-data-oriented-design-and-c-mike-acton) (Mike Acton, CppCon 2014 keynote) | The case for column layouts; how to think about data first |
| Talk | ['Overwatch' Gameplay Architecture and Netcode](https://gdcvault.com/play/1024001/-Overwatch-Gameplay-Architecture-and) (Timothy Ford, GDC 2017) | ECS in production, deterministic replay, few systems for a hard problem |
| Talk | [Continuous World Generation in No Man's Sky](https://gdcvault.com/play/1024265/Continuous-World-Generation-in-No) and Sean Murray's "Building Worlds Using Math(s)" (GDC 2017) | Seeds instead of stored worlds, seamless scale change |
| Talk | Hitman: Absolution crowds, [GDC 2013 note](https://mcvuk.com/?p=99805) | 1,200-character crowds |
| Blog series | [Factorio Friday Facts](https://forums.factorio.com/viewtopic.php?p=615004) (#421 "Optimizations 2.0"; #157; #66) | Real numbers on save size and update cost from a team that optimises in public |
| Article | [700,000 lines of code, 20 years, and one developer: how Dwarf Fortress is built](https://stackoverflow.blog/2021/12/31/700000-lines-of-code-20-years-and-one-developer-how-dwarf-fortress-is-built/) and Tarn Adams's Roguelike Celebration 2018 talk | One simulation kept tractable by abstraction |
| Paper | [Simulation level of detail for multiagent control](https://www.ri.cmu.edu/publications/simulation-level-of-detail-for-multiagent-control) (CMU) | The scheduler idea |
| Paper | [Pad (Perlin and Fox, SIGGRAPH 1993)](https://www.cs.umd.edu/projects/hcil/jazz/learn/papers/siggraph-93-origpad.pdf); Bederson's [Jazz](https://cs.umd.edu/~bederson/images/pubs_pdfs/p23-bederson.pdf) | Semantic zoom, portals and lenses |
| Paper | [Managing multiple representations of "base carto" features](https://cartogis.org/docs/proceedings/2005/buttenfield.pdf) (Buttenfield, 2005) and the [ICA generalisation workshop papers](https://kartographie.geo.tu-dresden.de/downloads/ica-gen/workshop2004/Neun-v2-ICAWorkshop.pdf) | Linked levels of detail; the golden-feature fallacy |
| Paper | [Roaring bitmaps](https://arxiv.org/pdf/1603.06549) (Lemire et al.) | Compressed id sets |
| Paper | [Agent-based simulators for epidemic modelling: simulating larger models using smaller ones](https://arxiv.org/pdf/2209.02887) | Scaling representative agents, and its limits |
| Docs | [Uber H3](https://www.uber.com/en-US/blog/h3/) | A production hierarchical hex index |
| Docs | [Live projections for read models](https://www.eventstore.com/blog/live-projections-for-read-models-with-event-sourcing-and-cqrs) | Projections and materialised views |
| Docs | [Interest management (Photon Fusion)](https://doc.photonengine.com/fusion-unreal/v3/manual/replication/interest-management), [spatial hashing (Mirror)](https://mirror-networking.gitbook.io/docs/manual/interest-management/spatial-hashing) | Subscriptions by area |
| Article | [ECS storage: archetypes versus sparse sets](https://habr.com/en/articles/651921/); [run-time comparison paper](https://diglib.eg.org/items/6e291ae6-e32c-4c21-a89b-021fd9986ede/full) | Engine choice for stable ids |

## 7. What this suggests for our design (Proposal)

1. **Write the Lens as a unit:** a pure function `(records, layer, observer, region) → view`, with the table in section 5 as its checklist; a subscription is a pin; materialised sums are kept per cell and level.
2. **Give ids a level.** Make a cell or entity id encode its grid level (as H3 does) so "parent" and "children" are bit operations. Our grid is nested, which makes this clean.
3. **Pop storage:** specify pops as `(id, seed, exceptions)` with rosters as compressed id sets; evaluate Roaring-style containers against plain sorted arrays at our sizes before adopting.
4. **Chunked saves with deltas:** per-chunk independent compression, a full snapshot plus per-chunk deltas, no stored defaults, no stored caches; state hash per chunk.
5. **A pool and a scheduler for attention:** fixed budgets for embodied and detailed agents, with time slices weighted by distance and visibility (AC Unity, simulation LOD).
6. **Design each lens on purpose:** the golden-feature fallacy is a reason to review each layer's view with the user, domain by domain, not to auto-derive them.

## 8. Not found, to check later

- The content of the KCD2 talk beyond its abstract (what is stored per NPC, how the LOD tiers differ).
- Dwarf Fortress's own save and abstraction details; Factorio's current entity-activity model.
- H3's exact child-nesting behaviour (our grid nests exactly; whether H3's children do was not in what I read).
- Palette and bit-packed block storage in Minecraft sections: referred to by name in my query, not confirmed by a source.
- Whether a Roaring-style structure beats sorted arrays at our roster sizes (needs a measurement, not a citation).
