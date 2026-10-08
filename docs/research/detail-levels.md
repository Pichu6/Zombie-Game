# Detail levels: one simulation, read and run in chunks

**Date:** 2026-10-06. **Status:** research note, not a decision (written as a Proposal; its rules were later taken into [spec 27](../tier0/specs/27-detail-levels.md)). **Feeds:** [world simulation](../design/world-simulation.md), [layer jumps](../design/layer-jumps.md), [pop continuity](../design/pop-continuity.md), [zoom continuum](../design/zoom-continuum.md), [engine architecture](../design/engine-architecture.md), [focus and compression](../design/focus-and-compression.md), [spec 27](../tier0/specs/27-detail-levels.md). **Evidence:** the game comparisons come from general knowledge of the games, with no pages cited or read in this note; the engineering evidence was added later in [pop-simulation-and-persistence.md](pop-simulation-and-persistence.md). The rest is our own proposal. **IDs:** DL.

Asked by the user: "do we need 3 levels of simulation, one per layer of gameplay? The simulation should be divided in chunks, read when needed."

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| DL-1 | Minecraft ticks chunks near players; far chunks are frozen on disk and nothing happens there | reported | this note (no source cited) |
| DL-2 | Factorio simulates every built chunk at full detail all the time, which works because its per-entity cost is tiny and deterministic | reported | this note (no source cited) |
| DL-3 | X4 runs full physics in the player's sector and a coarse out-of-sector model elsewhere, with the same ships and outcomes by formula | reported | this note (no source cited) |
| DL-4 | RimWorld keeps world-map pawns as records; off-map caravans and raids are abstract | reported | this note (no source cited) |
| DL-5 | Dwarf Fortress keeps world history and off-site armies as abstract events while the fort runs at full detail | reported | this note (no source cited) |
| DL-6 | Mount & Blade and Kenshi auto-resolve far battles from troop stats and fight near ones | reported | this note (no source cited) |
| DL-7 | Star Citizen gives each entity one authority at a time and streams containers of objects in and out | reported | this note (no source cited) |
| DL-8 | Not three simulations: separate copies of the facts drift apart (dead in one, alive in another) and every rule is written three times; one simulation runs each thing at the cheapest detail that gives the right answer | estimate | this note |
| DL-9 | Gameplay layers (Individual, City, Macro) are cameras and verb sets over one simulation; detail follows the player's body, never the camera | estimate | this note |
| DL-10 | Three detail levels for people, places, things and time: cold (lazy row of 0 bytes or a 64-byte record, region numbers, ledger stocks, daily), warm (record with schedule and needs, building ledger plus room graph, hourly or event-driven), hot (full body, cells and fields, every item, every 0.5 s tick) | estimate | this note |
| DL-11 | Each rule exists once as a rate; hot steps it, warm and cold apply it in one jump (a fridge off for 18 h spoils 18 h worth) | estimate | this note |
| DL-12 | A chunk is a building with its lot or a street segment, not a square of terrain; a hot chunk's neighbours are at least warm because fields cross borders | estimate | this note |
| DL-13 | The hot bubble is the chunks within reach of any player-controlled body, roughly what it can hear and see through, plus one ring | estimate | this note |
| DL-14 | Each chunk is saved on its own with the tick it was last brought up to date, and is caught up by the warm rules when it wakes | estimate | this note |
| DL-15 | Promotion and demotion go through one aggregate/disaggregate function pair checked by the ledger so no person, kilogram or litre is created or lost | estimate | this note |
| DL-16 | Draws keyed per entity and purpose make waking early or late give the same outcome for the same elapsed time | estimate | this note |
| DL-17 | Tier 0 stage 7 census is the first working case: 60 neighbours as rows, off-map walkers as a count that becomes a body at the map edge and a number again when destroyed, ledger-checked every tick | estimate | this note |
| DL-18 | Risks to test early: cold-versus-hot agreement in distribution, border effects (a hot generator heard in a cold house), wake-cost spikes, and step-by-step things (fights, doors) never resolved cold in view | estimate | this note |

## Answer in one paragraph

Not three simulations. Three simulations that each hold their own copy of the facts drift apart (a person dead in one, alive in another) and every rule has to be written three times. What we want is **one simulation where each thing runs at the cheapest detail that still gives the right answer for where the player is**, and is stored in **chunks that load and wake only when needed**. The gameplay layers (Individual, City, Macro) are cameras and verb sets over that one simulation ([world-simulation.md §1, §2 rule 8](../design/world-simulation.md)). Detail follows the player's body, never the camera (§3). This was already the direction for people (lazy census → record → embodied); this note extends it to places and things and says how chunks work.

## How others do it

| Game | What it does | Lesson for us |
| --- | --- | --- |
| Minecraft | Chunks near players tick; far chunks are frozen on disk, nothing happens there | Cheap and simple, but frozen means wrong for us (fuel doesn't burn, food doesn't rot, the dead don't move) |
| Factorio | Simulates every built chunk at full detail, all the time | Works only because the per-entity cost is tiny and deterministic; our bodies are not tiny |
| X4 | Sectors the player is in run full physics; other sectors run a coarse "out of sector" model with the same ships and outcomes by formula | Same entities, two fidelities; this is our model |
| RimWorld | Pawns on the world map are kept as records; caravans and raids off-map are abstract | Off-map people as records, embodied on arrival: we already do this with the census |
| Dwarf Fortress | World history and off-site armies are abstract events; the fort is full detail | Far away = events and rates, near = bodies |
| Mount & Blade, Kenshi | Battles far away auto-resolve from troop stats; near ones are fought | Aggregation must agree with the detailed result in distribution (our contract rule 12) |
| Star Citizen | Each entity has one authority at a time; containers of objects stream in and out | One owner per entity, hand-offs checked (our contract rule 6) |

## The proposal

### Three detail levels for everything (not three sims)

| Level | People | Places | Things | Time |
| --- | --- | --- | --- | --- |
| **Cold** | Lazy census row, regenerated from seed (0 bytes) or a 64-byte record | Region numbers: population by state, stocks, infrastructure condition | Stocks in the place's ledger (kg of food, litres of fuel), no positions | Daily, or only when asked |
| **Warm** | Record with schedule and needs as numbers | Building ledger plus room graph, no cells | Item list per building, no positions | Hourly, or event-driven |
| **Hot** | Full body, senses, executor | Cells, edges, fields (noise, light) | Every item with position and condition | Every tick (0.5 s) |

The same rule exists once as a rate. Hot runs it step by step; warm and cold apply the same rate in one jump ("this fridge has been off for 18 h → spoil 18 h worth"), which is exact for most of our rules because they are already written as rates over time (fuel burn, spoilage, healing, arrivals).

### Chunks

- **The unit is the building or block, not a square of terrain.** Our world is small and dense; what matters is "inside this house" vs "outside". A chunk = one building with its lot, or one street segment. Fields (noise, light) cross chunk borders, so a hot chunk's neighbours are at least warm.
- **The hot bubble** is the chunks within reach of any player-controlled body (and, later, any squad the player watches closely): roughly the walls it can hear and see through, plus one ring.
- **Each chunk is saved on its own** (its ledger, its items, its edges) with the tick it was last brought up to date. Loading a chunk = read it, then catch it up to now.
- **Catch-up on wake:** when a chunk turns hot, apply the elapsed time to it by the warm rules (spoil, burn, decay, walker drift as a draw), then hand its people to the full simulation. This is "read when needed": nothing ticks while cold, but nothing is frozen either.
- **Promotion and demotion** move each entity between levels through one function pair (aggregate / disaggregate), checked by the ledger so no person, kilogram or litre is created or lost (contract rules 2, 6, 12). Placement inside a building is generated from the building's seed on first visit and saved after ([0017](../decisions/0017-slow-endgame-and-generation-on-the-go.md)).

### What stays the same at every level

- Integer stocks and owners: a cold region with 412 walkers and a hot street where you can count them hold the same people.
- Draws keyed per entity and purpose, so waking a chunk early or late gives the same outcome for the same elapsed time. This is what keeps save/load and replays identical.
- Views only read. Switching from Individual to City to Macro changes what you see and what you can order, never the state.

## Where Tier 0 already does this

The stage 7 census is the first working case. The 60 neighbours start as rows; off-map walkers are a count with names; when one arrives it becomes a full body at the map edge; when it is destroyed it goes back to a number. The ledger checks the hand-off every tick.

## Risks to test early

1. **Agreement:** a cold jump must match the hot run in distribution (e.g. 10 hot runs of a fridge without power vs one cold jump). Every rule with a cold form gets that test.
2. **Border effects:** a hot generator next to a cold house must still be heard there; the warm ring handles it, but needs a test.
3. **Wake cost spikes:** waking a block with many walkers in one frame. Spread promotion over a few ticks.
4. **Things that only make sense step by step** (a fight, a door being pushed) are never resolved cold while the player could still see them; they finish hot or are auto-resolved only outside the bubble.

## Enriched (2026-10-07)

[pop-simulation-and-persistence.md](pop-simulation-and-persistence.md) adds the engineering evidence behind this note: Dwarf Fortress's five tiers and its lossy offload, Paradox pop grouping and merging, Project Zomboid's zombies as per-cell numbers, the academic simulation level-of-detail literature (Chenney's state distributions, alibi generation, multi-level aggregation with memorisation), mesoscopic traffic and GLEaM for networks and the outbreak, and the save-format precedents (Factorio, Minecraft region files, SQLite containers). Its section 3 extends the detail table above to groups, places, things, networks, vehicles, fields, the outbreak, the economy, weather and beliefs. The rules it adds (projection written field by field, memorisation blob, kept reasons and a merge path, constraint-checked promotion, importance beyond distance, groups as nodes) are now in [spec 27](../tier0/specs/27-detail-levels.md) rules 8–12.

## Recommended next step (by dependency, no dates)

1. Now: nothing to change in Tier 0 code. The block is one hot chunk, plus the cold census. That's the right size.
2. Write a one-page spec "27 · Detail levels and chunks" with the rules above, so stage 8 and 9 code doesn't paint us into a corner (for example: every new system states its rate so it can be jumped).
3. First real test at the start of Tier 1 (bigger map): split the block into building chunks, freeze and wake them, and check the hash and agreement tests above.
