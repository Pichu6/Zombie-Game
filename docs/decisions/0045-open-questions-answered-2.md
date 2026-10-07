# 0045 · Open questions answered: births, weather, waste, Q display, succession, amputation, power, saves, tech, nerves, data

- **Date:** 2026-10-07
- **Note:** written on 2026-10-07 in a local session with a lower number and renumbered on merge.
- **Status:** Decided by the user (answers below; "recommended" where the user said so).
- **Extends:** [0005](0005-finite-world-parity.md), [0013](0013-production-ladder.md), [0017](0017-slow-endgame-and-generation-on-the-go.md), [0043](0043-city-layer-and-repercussion-loop.md).

## Decisions

1. **Finite world, with births and crops.** Nothing respawns; pops can still be born and crops still grow. Both are real conversions through the ledger (seed, water and time into a crop; a pregnancy into a person), never something from nowhere.
2. **Weather and temperature: whatever is realistic.** In Individual the pop suffers it directly (a body system from the first tier that has bodies outdoors); in City and Macro it is a statistic that affects pops. Tier placement follows realism, not convenience: Tier 1 for temperature on the body.
3. **Bladder and bowel:** pops handle it automatically; in Individual the player can order it, as in Project Zomboid mods. Tier with water (Tier 2) as a Proposal.
4. **Q is shown as a range** through the character's eyes; with certainty (an assessment at full confidence: measured, expert, or made by the character) the exact number is shown.
5. **Succession and desertion:** none formal in Tier 0 or 1; trust-based obedience and succession arrive with relationships in Tier 2.
6. **Emergency amputation:** Tier 2, a procedure with a survival draw by skill, blood loss and tools.
7. **Power is watts per connected component.** Electrified water is a feature regardless: a live component touching water in a room makes that water a hazard, with no voltage model needed.
8. **Save container at the Tier 1 chunk split:** SQLite, one file per world.
9. **No research-points tree.** What a city can build is the country's real present plus the Projects it runs ([0043](0043-city-layer-and-repercussion-loop.md)).
10. **Nerves:** fear also widens aim spread and raises work-error odds; no new stored number.
11. **Data: only what is free and needs no permission.** Datasets whose licence is ambiguous or needs a written answer (UN DESA household and marriage data pages, Eurostat public-use files, IBGE 2022 microdata, IPUMS International) are excluded, not queried. Where a substitute with an explicit commercial-compatible licence exists it is used; where none exists the generator falls back to the synthetic route ([research/human-generator-data.md](../research/human-generator-data.md) §4).

## Why

Realism first, then simplicity: births and crops are real; weather is real; waste is real but should not be a chore; a reading is a belief until it is certain. Engineering choices follow the evidence in the research notes. The data rule removes a dependency on other people's answers.
