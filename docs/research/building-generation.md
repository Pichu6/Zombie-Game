# Building and interior generation, and real housing numbers to calibrate it

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [spec 51](../tier0/specs/51-dwellings.md), [content tools](../design/content-tools.md), [world simulation](../design/world-simulation.md). **Evidence:** read in full: Lopes et al. 2010 (GAME-ON PDF), Merrell, Schkufza and Koltun 2010 (PDF), Mirahmadi and Shami 2012 (arXiv HTML), OECD Affordable Housing Database indicator HC2.1 (PDF, updated 2025-10-02), the UK nationally described space standard (gov.uk), the WHO sanitation fact sheet, the Destatis housing page, the abstract of World Bank working paper 9618, and Global Data Lab refrigerator tables for Nigeria, India, Ethiopia and Mexico (text extraction; year alignment not certain). Everything else is from web-search summaries (`reported`). Not read: House-GAN, Müller 2006, Wonka 2003, Marson and Musse 2010 and Yu 2011 in full; the Bethesda, Stålberg and Caves of Qud talks; the AHS 2021 tables; UN-Habitat SDG 11.1.1 data; DHS STATcompiler; Eurostat tables (the Statistics Explained pages returned only the portal). The web-search budget ran out before searches on India NSS housing, IEA floor area per capita and UN-Habitat SDG 11.1.1. **IDs:** BG.

Spec 51 builds a one-storey corridor house from each household's dwelling record, with estimated numbers for floor area per person, bathrooms, fridges and pantry days by income group. This note looks at how others generate buildings and interiors, and at real housing data to replace those estimates.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| BG-1 | Merrell, Schkufza and Koltun (SIGGRAPH Asia 2010) train a Bayesian network on 120 house programs hand-encoded from a plan catalogue (Wood 2007). They train separate networks for one-, two- and three-storey houses, on 40, 50 and 30 programs | sourced | cs.princeton.edu Merrell10a.pdf |
| BG-2 | In Merrell 2010 a program lists rooms, their type, area and aspect ratio, and which pairs are adjacent through a door or an open wall. Any subset of variables (bedrooms, bathrooms, total area) can be fixed and the rest sampled | sourced | Merrell10a.pdf |
| BG-3 | Merrell 2010 turns a program into a plan by Metropolis optimisation (wall moves and room swaps). The cost terms are accessibility (missing doors, enclosed entrances), room dimensions (log-likelihood of area and aspect), floor compatibility and shape. One example took 35 s; whole houses took a few seconds to 7 minutes, single-threaded | sourced | Merrell10a.pdf |
| BG-4 | In Merrell 2010 the optimiser sometimes stops with a room unreachable (for example a blocked stair). This almost never happens for one storey, about 1 run in 20 for two storeys and 1 in 5 for three. The failures are easy to detect automatically | sourced | Merrell10a.pdf |
| BG-5 | Merrell 2010 reports that residential architects treat the exterior style as largely independent of the interior layout: style (Craftsman, Colonial Revival) is applied as trim, windows and entrances | sourced | Merrell10a.pdf |
| BG-6 | Lopes, Tutenel, Smelik, de Kraker and Bidarra (GAME-ON 2010) work on a grid (for example 0.5 m cells). They split the house into zones (public, private), then rooms. Each room starts from a weighted seed cell, grows to the largest rectangle, then grows into L shapes, and leftover cells go to a neighbour. If an adjacency constraint fails, the plan is restarted | sourced | graphics.tudelft.nl bidarra.GAMEON10.pdf |
| BG-7 | Lopes 2010 places doors in this order: declared connections, then the hallway to every adjacent public room, then each private room to an adjacent public room. A final reachability test adds doors along adjacency paths until every room is reachable from the hallway | sourced | bidarra.GAMEON10.pdf |
| BG-8 | Lopes 2010 timings: an 11 x 9 m house with four rooms and a hallway takes under 100 ms; a 17 x 20 m L-shaped villa takes about 1 s. For more storeys, the stair room is copied to the same position on each floor and the other rooms grow around it | sourced | bidarra.GAMEON10.pdf |
| BG-9 | Lopes 2010 limits: a door goes at a random point of the chosen wall, so walking routes are not always good; rooms often come out L-shaped, which is wrong for a garage; room proportions cannot be constrained | sourced | bidarra.GAMEON10.pdf |
| BG-10 | Hahn et al. 2006 generate office buildings floor by floor and room by room, just in time near the player. A discarded room is regenerated from the same seed, and the player's changes are applied again | reported | described in Lopes 2010 |
| BG-11 | Marson and Musse 2010 split the footprint with squarified treemaps (rectangles kept near square), first into social, service and private areas, then into rooms. Doors follow predefined room-type pairs, and an A* path through the rooms that need it becomes the corridor | reported | described in Lopes 2010; Int. J. Computer Games Technology 2010 |
| BG-12 | Mirahmadi and Shami 2012 extend the treemap method. Room counts come from the 2001 Canadian census (a joint table of bedrooms and total rooms); bedrooms are drawn from 8 to 18 m² and other rooms from 3 to 11 m². The corridor is the shortest path (Dijkstra) on a graph of walls, and it counts as part of the living room | sourced | arXiv 1211.5842 |
| BG-13 | Martin 2006 grows a room graph from the front door: public rooms first, then private rooms, then small rooms such as closets and pantries; rooms then expand under "pressure" to fill the space | reported | described in Lopes 2010 |
| BG-14 | House-GAN (Nauata et al., ECCV 2020) takes a bubble diagram (room types and adjacencies) and outputs room boxes. It was evaluated on about 117,000 real plans. The authors call it an early step that still lacks real architectural constraints | reported | arXiv 2003.06988 |
| BG-15 | Müller, Wonka, Haegler, Ulmer and Van Gool (SIGGRAPH 2006), CGA shape grammar: the mass model first, then the facade structure, then windows, doors and ornaments. Context rules keep parts from clashing (no window through a wall). It is the basis of CityEngine | reported | history.siggraph.org; muller_buildings.pdf |
| BG-16 | Wonka, Wimmer, Sillion and Ribarsky (SIGGRAPH 2003), split grammars: a rule may only split a shape into parts inside it, so parts never collide. A control grammar and attribute matching choose rules to fit a style or goal | reported | cg.tuwien.ac.at Wonka-2003-Ins |
| BG-17 | Bad North builds its island dioramas with wave function collapse (WFC) from hand-made tile sets (Stålberg, Everything Procedural 2018) | reported | 80.lv "using wave function collapse algorithm for dioramas" |
| BG-18 | Townscaper grew from an earlier modular tool (Brick Block) whose output quickly looked repetitive; WFC on an irregular quad grid gave more variety | reported | gamedeveloper.com "How Townscaper works"; etao.blog podcast 94 |
| BG-19 | Caves of Qud makes ruins in several passes: early passes set the coarse layout, middle passes fill detail with WFC, final passes ensure connectivity and populate. The ruins reflect the generated culture that lived there | reported | gdcvault.com/play/1025913 |
| BG-20 | Skyrim modular kits (Burgess and Purkeypile, GDC 2013): pieces snap to a grid. Eight level designers built 300+ dungeons, 5 cities, 140+ points of interest and 400+ interior cells. The known cost is "art fatigue" from repeated pieces | reported | gamedeveloper.com "Skyrim's modular approach"; slideshare gdc2013-kit-building |
| BG-21 | Project Zomboid's buildings are hand-built in the BuildingEd editor, and early Muldraugh buildings were later rebuilt with it. Whether a tile is indoors is fixed per building | reported | projectzomboid.com blog; Steam discussions |
| BG-22 | RimWorld faction bases (since Alpha 16 and 17) come from a grammar system (BaseGen) mixing indoor and outdoor parts. Room types include barracks, dining room and storage; the faction's tech level sets materials (tribal bases use wooden walls and torches) | reported | rimworldwiki.com Faction_base |
| BG-23 | Prison Architect defines a room by its type, a minimum size and the objects it needs: a cell is at least 2 x 3 tiles with a bed and a toilet; a holding cell 5 x 5; a dormitory holds 1 prisoner per 4 tiles. Rooms get a quality grade from 1 to 10 | reported | prison-architect.fandom.com |
| BG-24 | The Sims 4 lots run from 15 x 10 to 64 x 64 tiles; Tiny Living caps homes at 32, 64 and 100 tiles per tier | reported | snootysims.com lot sizes; EA forums |
| BG-25 | Watabou's Dwellings generator lays out rooms first and assigns roles (kitchen, bedroom) afterwards; sometimes no room fits a role. The author says it gets unrealistic above "large" houses. It places no furniture | reported | watabou.itch.io/dwellings devlogs |
| BG-26 | Yu, Yeung, Tang, Terzopoulos, Chan and Osher (SIGGRAPH 2011), "Make it home": learns furniture relations from example rooms and optimises a cost built on accessibility, visibility and pair relations (a sofa faces the TV, doors can open) | reported | ww3.math.ucla.edu cam11-49; history.siggraph.org |
| BG-27 | OECD and EU overcrowding rule: a household needs one room, plus one per couple, one per single adult (18+), one per pair of same-sex children aged 12-17, one per other child aged 12-17, and one per pair of children under 12. Rooms exclude kitchens in Europe but include them elsewhere | sourced | OECD AHD HC2.1 (2025) |
| BG-28 | Outright owners have under 2 rooms per member in much of Central and Eastern Europe, Colombia, Korea, Mexico and Türkiye; 2 to 3 in Southern Europe, the Nordics, Germany, France, the Netherlands and Japan; about 3.8 in the United Kingdom and the United States | sourced | OECD AHD HC2.1 |
| BG-29 | Overcrowding affects at least 30% of all households in Latvia, Colombia and Mexico, and under 3% in Canada, Korea, Japan, Malta, New Zealand and the United Kingdom | sourced | OECD AHD HC2.1 |
| BG-30 | In the bottom income quintile, overcrowding is at least 30% in Colombia, Latvia, Mexico, Slovakia and Türkiye, and under 5% in Canada, Cyprus, Ireland, Japan, Korea, Malta, New Zealand and the UK. Over 20% of top-quintile households are overcrowded in Bulgaria, Croatia, Latvia, Romania and Slovakia. Overcrowding mostly falls as income rises | sourced | OECD AHD HC2.1 |
| BG-31 | EU 2023: 16% of people lived in overcrowded households (Latvia 40.9%, Romania 40.0%, Bulgaria 34.9%); 26.0% of those aged 15-29; 34.2% of non-EU citizens | reported | ec.europa.eu Eurostat news ddn-20241016-1; "Young people - housing conditions" |
| BG-32 | EU 2023: 51.7% of people lived in a house and 47.7% in a flat. In cities 72% lived in flats; in rural areas 82% lived in houses. Flats are most common in Spain (65-66%), Latvia (64-65%) and Germany (61%) | reported | Eurostat "Housing in Europe" 2023 and 2024 interactive publications |
| BG-33 | A 1990s UN indicator (median dwelling floor area divided by mean household size) gave about 17 m² per person for the world, from 3.7 in the most crowded countries to over 60 in the least, rising with income | reported | Behrmann, "Thoughts behind 18 square meters" (Lund HDM), citing UN-Habitat 1996 |
| BG-34 | 1992 UN-Habitat and World Bank housing indicators: Dhaka 3.72 m² per person, Hong Kong 37.1 | reported | housingauthority.gov.hk "Asia's housing profile" |
| BG-35 | Cambodia 2023: 14.9 m² per person nationally, 16.6 in Phnom Penh, 14.4 in rural areas | reported | ceicdata.com Cambodia occupied dwellings floor area |
| BG-36 | Malaysia 2017: about 294 sq ft (27 m²) per person, seventh of 14 Asian countries | reported | edgeprop.my |
| BG-37 | China: urban 31.6 and rural 34.1 m² per person in 2010 (rural 8.1 in 1978); later series give urban 38.6 (2020) and rural 48.9 (2019). The two series may use different bases | reported | stats.gov.cn yearbook 2011; ceicdata.com; [WEC-18](world-systems-economy.md) |
| BG-38 | Germany: 47.7 m² of living space per inhabitant (headline figure; the page gives no year) | sourced | destatis.de Housing |
| BG-39 | Finland 40.5 m² per person (2017), Lithuania 35.5 (2018) | reported | [WEC-18](world-systems-economy.md) |
| BG-40 | City averages, 2011: Delhi 3.31 m² of residential floor per person, Cairo 10.64, Beijing 21.85 | reported | archive.metabolismofcities.org indicator 345 |
| BG-41 | Lall, Lebrand and Soppelsa 2021 (World Bank WP 9618): if a city's population doubles at constant income, floor space per person falls about 40%; if income doubles at constant population, it rises about 29%. Building tall needs income, because it is capital intensive | sourced | econpapers.repec.org wbrwps 9618 (abstract) |
| BG-42 | Same paper: in low and lower-middle income countries, 90% of urban built-up expansion is outward, not upward | reported | search summary of WP 9618 |
| BG-43 | US new single-family houses sold in 2023 had a median of 2,286 sq ft (212 m²), from 2,170 sq ft in the West to 2,430 in the Northeast | reported | census.gov "national housing month" 2024 |
| BG-44 | US housing stock (AHS 2011): 62.7% detached single-family, 24.6% multifamily, about 16.6% in buildings of 5 or more units. ACS: median 5.5 rooms; 67.3% of units have 2 or 3 bedrooms and 20.8% have 4 or more | reported | huduser.gov USHMC 3q12; builderonline.com |
| BG-45 | US homes with more than one person per room fell from 15.7% in 1950 to 5.7% around 2000 | reported | secondary source in search; underlying source not confirmed |
| BG-46 | UK minimum gross internal floor area (one storey): 1 bedroom 1 person 39 m² (37 with a shower room), 1b2p 50, 2b3p 61, 2b4p 70, 3b4p 74, 3b5p 86, 4b6p 99; two storeys add 7 to 9 m². Built-in storage 1.0 to 3.0 m² | sourced | gov.uk nationally described space standard, Table 1 |
| BG-47 | UK bedroom minimums: single 7.5 m² and 2.15 m wide; double or twin 11.5 m², one at least 2.75 m wide and others 2.55 m; ceiling 2.3 m over at least 75% of the floor | sourced | gov.uk nationally described space standard |
| BG-48 | US International Residential Code: a habitable room is at least 70 sq ft (6.5 m²) and 7 ft (2.13 m) in every direction, with a 7 ft ceiling; kitchens are exempt. Before 2015 one room had to be at least 120 sq ft | reported | iccsafe.org sample; finehomebuilding.com |
| BG-49 | Sanitation 2022: 57% of the world used safely managed sanitation (33% sewer with treatment, 21% on-site safe disposal); 88% had at least basic service; 419 million defecated in the open | sourced | who.int sanitation fact sheet |
| BG-50 | Of the people without safely managed sanitation in 2022, about 570 million shared a toilet with other households and 545 million used poor latrines or buckets | reported | JMP 2023 update via search |
| BG-51 | Households with a refrigerator: Ethiopia about 8.6% (2023), Nigeria 24.7% (2023, states from 1% to 71%), India 46.1% (2022, states from 15% to 99.9%), Mexico 93.8% (2014); Albania and Algeria above 93% | reported | globaldatalab.org/wealth/fridge (DHS and other surveys; years aligned by us) |
| BG-52 | Refrigerators by wealth: western Tajikistan 2009, 2.7% of the bottom 40%, 22.2% of the middle 40%, 75.8% of the top 20%; Ontario 2006, 99.3% of the lowest quintile; urban China 2005, about 65 per 100 households in the lowest income group and 104 in the highest | reported | pmc.ncbi.nlm.nih.gov; rds.oeb.ca; stats.gov.cn 2006 yearbook |
| BG-53 | Food on hand for 3 days: 86.2% of US owner-occupied households (AHS 2013); 82.9% in 14 US states (CDC, 2006-2010); 52.7% in Corvallis, Oregon. On Kauai (2019): 90% had 3 days, 74% 7 days, 44% 14 days. In a Japanese survey of 800 mothers, 12.9% had more than 3 days of emergency food and 46.3% had none | reported | cambridge.org (DMPHP); stacks.cdc.gov 29135; civilbeat.org; nippon.com h00957 |
| BG-54 | We found no survey of household food stocks in days by income group | estimate | our searches, this note |
| BG-55 | UN-Habitat counts a home as lacking sufficient living area when more than three people share a habitable room (from memory; not re-checked this session) | estimate | UN-Habitat slum definition, SDG 11.1.1 |
| BG-56 | UK minimum area per person: 39 m² (1 person), 25 (2), 17.5 (2b4p), 17.2 (3b5p), 16.5 (4b6p). The minimum per person falls with household size, and spec 51's 24 m² for one person is below the UK one-person minimum | derived | BG-46 |
| BG-57 | Floor area per person by country income, from BG-33 to BG-40: low-income cities 3 to 11 m² (Dhaka, Delhi, Cairo); lower-middle 14 to 17 (Cambodia); upper-middle 27 to 49 (Malaysia, urban and rural China); high 35 to 48 (Lithuania, Finland, Germany). Spec 51's 12 / 20 / 30 / 40 fits lower-middle to high but is high for low income | derived | BG-33 to BG-40 |

## A. Methods, and which suit us

Our constraints: deterministic from a seed; every room reachable; contents follow the household ledger exactly; variety from "lego" modules per culture, era and income; fast enough to generate on the go.

| Method | Reachable by construction? | Deterministic and fast? | Fits our record-first rule? | Use |
| --- | --- | --- | --- | --- |
| Corridor plan (spec 51 as built) | Yes | Yes | Yes | Keep as the fallback and as the "row house" style |
| Grid growth with zones (Lopes, BG-6 to BG-9) | Yes, with the reachability pass (BG-7) | Yes, under 100 ms for a small house (BG-8) | Yes: the room list is the input | Main plan method for houses |
| Treemap split (Marson, Mirahmadi, BG-11, BG-12) | Needs a corridor pass | Yes | Yes | Good for apartments and regular offices |
| Learned program plus optimisation (Merrell, BG-1 to BG-4) | Not always (BG-4) | No: seconds to minutes | The program step is useful; the optimiser is not | Take the idea: the room list is drawn from a table conditioned on household facts |
| GAN (House-GAN, BG-14) | No guarantee | Model weights, not data packs | No | Skip |
| Split and shape grammars (BG-15, BG-16) | Not about rooms | Yes | Yes, for the outside | Facades and massing, styled per culture and era (BG-5) |
| WFC (BG-17 to BG-19) | Only with a pass after it (BG-19) | Yes, with a seed, but can contradict and need retries | For dressing | Exterior dressing, yards, ruins; not the room plan |
| Hand-made kits and prefabs (BG-20 to BG-22) | Per prefab | Yes | Yes, if prefabs carry tags | Rooms and furniture sets as "lego" modules, tagged by culture, era and income |
| Requirement rooms (Prison Architect, BG-23) | Yes, checked | Yes | Yes | The room-type validator: minimum size plus required objects |
| Learned furnishing (Yu 2011, BG-26) | Uses accessibility cost | Optimisation is slow | Partly | Take the rules (clearance, facing, doors open), not the optimiser |

The pattern that recurs in the games is several passes: coarse layout, then detail, then a connectivity check (BG-7, BG-19). Spec 51 already does reachability by construction plus a check per furniture piece; the same check should guard any new plan method.

## B. Housing numbers

**Floor area per person.** National averages run from about 3 to 11 m² in low-income cities to 35 to 48 m² in rich countries (BG-57). Within a country, crowding falls as income rises (BG-30, BG-41). Rooms per person and overcrowding are better measured than floor area and come by income quintile (BG-28 to BG-30).

**Rooms.** The EU and OECD rule (BG-27) gives a needed room count from the household's ages and couples. That count can drive bedrooms directly, and the overcrowding rate by income quintile (BG-29, BG-30) gives the chance a household has one room fewer.

**Houses and flats.** About half of EU people live in flats, but 72% in cities and 17% in rural areas (BG-32). In the US about 63% of homes are detached (BG-44). Building tall needs income (BG-41, BG-42).

**Amenities.** Basic sanitation is 88% worldwide; sewer connection 33% (BG-49). Fridges run from under 10% (Ethiopia) to above 90% (Mexico, Albania) and climb steeply with wealth inside a country (BG-51, BG-52).

**Food stocks.** Most US households report 3 days of food (BG-53); 14 days is under half. There is no breakdown by income (BG-54).

**Minimum sizes.** UK: single bedroom 7.5 m², double 11.5 m² (BG-47); US: 6.5 m² and 2.13 m wide (BG-48).

## What this means for us (Proposal)

All of this section is `estimate`, tied to the findings by ID.

### 1. Replace spec 51's estimates

| Item | Today | Proposed | Based on |
| --- | --- | --- | --- |
| Area per person by country income group | 12 / 20 / 30 / 40 | 8 / 15 / 30 / 45 m² | BG-33 to BG-40, BG-57 |
| Area per person inside a town | One value per town, ±15% | Scale by the household's income quintile (bottom about 0.7, top about 1.4 of the town value), then clamp to the room minimums | BG-30, BG-41 |
| Minimum dwelling | 24 m² | Sum of room minimums instead of one number; for high-income countries at least 37 m² for one person | BG-46, BG-56 |
| Bedrooms | Head and partner, one per two others, at most 4 | The overcrowding rule (BG-27) as the norm; with the quintile's overcrowding rate, one room fewer | BG-27, BG-29, BG-30 |
| Bathroom | None in low income | Draw by country sanitation shares: private indoor, shared, latrine outside, none | BG-49, BG-50 |
| Fridge | None in low income | Draw by country ownership share, tilted by quintile | BG-51, BG-52 |
| Pantry days | 3 / 5 / 7 / 10 by income | A per-household draw: high-income countries about 85% at 3+ days, about 45% at 14+; tilted by quintile; low and lower-middle: unknown, keep estimates | BG-53, BG-54 |
| Room minimums | Cells for door and furniture | Plus a floor: bedroom 7.5 m² single and 11.5 m² double in rich-country packs, 6.5 m² elsewhere; nothing under 2 cells wide | BG-47, BG-48 |

The Tier 0 town is a mid-size US town, so it should use US anchors: 3.8 rooms per member for outright owners (BG-28), detached houses for about 63% of homes (BG-44), and new houses around 212 m² (BG-43). Older and rental homes should be smaller.

Numbers go to `data/gen/dwellings.toml` with their source IDs, by country pack rather than by world income group alone.

### 2. Plan generator with modules

1. **Program step (BG-1, BG-2, BG-12).** From the household, draw the room list: bedrooms by BG-27, bathrooms and kitchen by amenity draws, extra rooms (dining, study, garage) from a small table per culture pack and era, conditioned on area. This is Merrell's idea with hand-made tables instead of a learned network.
2. **Zone and grow (BG-6, BG-7).** Public zone at the street, private zone behind; grid growth on 1 m cells; doors by Lopes' order plus the reachability pass. Restart with the next key on failure, at most N tries, then fall back to the corridor plan, so every house is still valid.
3. **Room types as requirements (BG-23).** Each type has a minimum area and width, required objects and allowed shapes (a garage must stay rectangular, BG-9). A plan that breaks a requirement is rejected, not patched.
4. **Lego modules (BG-20 to BG-22).** Furniture sets and small room prefabs tagged `{culture, era, income band}`, chosen by keyed draw. Different tags for the same room type give the variety; sets are small and many, to avoid art fatigue (BG-20).
5. **Exterior by grammar (BG-5, BG-15, BG-16).** The facade, roof and trim come from a split grammar per culture and era, independent of the plan except for window and door positions.
6. **WFC only for dressing (BG-17 to BG-19)**: yards, fences, ruins, debris, always followed by the reachability check.
7. **Furnishing rules from Yu (BG-26)** as checks, not optimisation: clearance in front of doors and wardrobes, beds against a wall, sofa facing the living room's focus.

### 3. Apartments and storeys

- The share of flats comes from the culture pack by settlement size: about 72% in cities and 17% in rural areas for EU-like packs (BG-32); fewer in low-income packs, where cities grow outward (BG-42).
- A block is a stack of identical floors (Merrell 2010 expects floors of apartment blocks to be regular enough to plan one at a time). The stair and lift core is placed first and copied up (BG-8, as Hahn and Lopes do, BG-10). Each floor is split into units by treemap (BG-11), with a corridor from the core.
- Each unit is a dwelling record as today; its plan comes from the same program and growth steps inside the unit's slot.
- Houses get a second storey when the area exceeds the lot's buildable footprint. The stair room is placed on both floors first.

### 4. Business premises

- A business record gives its type and staff. Its program comes from a table per type: a shop has a sales floor, storage, a toilet and an office; a diner has a dining room, kitchen, cold store and toilets.
- Area comes from staff and type; the numbers per worker are estimates until a note on commercial floor space exists.
- Contents follow the business ledger, as dwellings follow the household: stock in kg or kJ, cash and tools. The Prison Architect style requirements (BG-23) apply to each room type.

### 5. Tests

- Every house, flat and premise: every free cell reachable from the street (or from the stair core for flats); every room meets its type's minimum area and width.
- Bedrooms match BG-27 except in households drawn as overcrowded, and the overcrowding share over a large town is within a few points of the pack's rate by quintile.
- Floor area per person by quintile over a town rises from bottom to top; the median is within the pack's band.
- Fridge and bathroom shares over a large town match the pack shares within sampling error.
- Two-storey and apartment plans: the stair is at the same cells on every floor.
- Same seed, same bytes; goldens per plan method.
- A fallback test: forced growth failure gives the corridor plan, never an invalid house.

## Open questions

1. Is the income group per country or per household? Proposal: country pack for the anchor, household quintile for the spread.
2. Floor area per person for low-income countries rests on old or city data (BG-33, BG-34, BG-40). A search on UN-Habitat SDG 11.1.1, India NSS housing rounds and IEA floor area per capita should follow.
3. Food stocks by income have no data (BG-54). Keep estimates, or derive from shopping frequency and fridge ownership?
4. How many plan methods to keep: corridor plus grid growth for houses and treemap for flats, or one method for all?
5. Should rooms be counted in the OECD way (no kitchen in Europe) or ours (all rooms)? Packs must say which.
6. Where do era kits come from: one pack per decade band of construction, tied to the building's build year in the record?
7. Commercial floor space per worker by business type needs its own research.
8. Does a damaged or looted house change the record or only the placement? (World simulation §12 says the record is the truth.)

## Sources

- Merrell, Schkufza, Koltun, "Computer-generated residential building layouts", SIGGRAPH Asia 2010: https://www.cs.princeton.edu/courses/archive/spring11/cos598A/pdfs/Merrell10a.pdf ; https://history.siggraph.org/?p=197308
- Lopes, Tutenel, Smelik, de Kraker, Bidarra, "A constrained growth method for procedural floor plan generation", GAME-ON 2010: https://graphics.tudelft.nl/~rafa/myPapers/bidarra.GAMEON10.pdf
- Marson and Musse, "Automatic real-time generation of floor plans based on squarified treemaps algorithm", Int. J. Computer Games Technology 2010 (via Lopes 2010 and https://arxiv.org/pdf/2606.21159)
- Mirahmadi and Shami 2012, "A novel algorithm for real-time procedural generation of building floor plans": https://ar5iv.labs.arxiv.org/html/1211.5842
- Nauata et al., House-GAN, ECCV 2020: https://arxiv.org/abs/2003.06988
- Müller et al., "Procedural modeling of buildings", SIGGRAPH 2006: https://history.siggraph.org/?p=113547 ; https://course.ccs.neu.edu/cs5150f13/readings/muller_buildings.pdf
- Wonka et al., "Instant architecture", SIGGRAPH 2003: https://cg.tuwien.ac.at/research/publications/2003/Wonka-2003-Ins
- Yu et al., "Make it home", SIGGRAPH 2011: https://ww3.math.ucla.edu/camreport/cam11-49.pdf ; https://history.siggraph.org/?p=114673
- Stålberg, WFC in Bad North: https://80.lv/articles/using-wave-function-collapse-algorithm-for-dioramas ; Townscaper: https://www.gamedeveloper.com/game-platforms/how-townscaper-works-a-story-four-games-in-the-making ; https://etao.blog/2020/10/13/podcast-94/
- Caves of Qud WFC: https://www.gdcvault.com/play/1025913
- Burgess and Purkeypile, Skyrim modular kits: https://www.gamedeveloper.com/design/skyrim-s-modular-approach-to-level-design ; https://www.slideshare.net/slideshow/gdc2013-kit-buildingfinal/17728576
- Project Zomboid buildings: https://projectzomboid.com/blog/?p=1773 ; https://steamcommunity.com/app/108600/discussions/0/595142002438311985
- RimWorld faction bases: https://mail.rimworldwiki.com/wiki/Faction_base
- Prison Architect rooms: https://prison-architect.fandom.com/wiki/Category:Rooms ; https://prison-architect.fandom.com/wiki/Dormitory
- The Sims 4 lots: https://snootysims.com/wiki/sims-4/lot-sizes-list-for-sims-4/
- Watabou Dwellings: https://watabou.itch.io/dwellings/devlog/563591/initial-release ; https://watabou.itch.io/dwellings/devlog/567327/110-regular-staircases-and-basements
- OECD Affordable Housing Database, HC2.1 Living space (2025): https://webfs.oecd.org/Els-com/Affordable_Housing_Database/HC2-1-Living-space.pdf
- Eurostat overcrowding 2023: https://ec.europa.eu/eurostat/web/products-eurostat-news/w/ddn-20241016-1 ; https://ec.europa.eu/eurostat/statistics-explained/index.php/Young_people_-_housing_conditions
- Eurostat Housing in Europe: https://ec.europa.eu/eurostat/web/interactive-publications/housing ; https://ec.europa.eu/eurostat/en/web/products-eurostat-news/w/wdn-20241128-1
- Behrmann, "Thoughts behind 18 square meters" (Lund HDM): https://hdm.lth.se/fileadmin/hdm/Publications/Thoughts_behind_18_square_meters_-_Javiera_Behrmann.pdf
- Asia's housing profile: https://www.housingauthority.gov.hk/housconf/atbl_yym.htm
- Cambodia floor area: https://www.ceicdata.com/en/cambodia/occupied-dwellings-floor-area
- Malaysia home size: https://edgeprop.my/content/1471152/five-things-you-probably-did-not-know-about-size-malaysian-homes
- China residential area: https://www.stats.gov.cn/sj/ndsj/2011/html/J1035e.htm ; https://www.ceicdata.com/en/china/residential-area-per-capita
- Destatis housing: https://www.destatis.de/EN/Themes/Society-Environment/Housing/_node.html
- Metabolism of Cities, residential floor area per capita: https://archive.metabolismofcities.org/data/indicators/345
- Lall, Lebrand, Soppelsa 2021, WB WP 9618: https://econpapers.repec.org/paper/wbkwbrwps/9618.htm
- US Census new housing 2023: https://www.census.gov/library/stories/2024/06/national-housing-month.html
- AHS 2011 summary: https://www.huduser.gov/PORTAL/periodicals/ushmc/fall12/USHMC_3q12_ch1.pdf ; ACS rooms: https://www.builderonline.com/builder-100/marketing-sales/a-modest-home-remains-national-norm-according-to-u-s-census-bureaus-american-community-survey_o/
- UK nationally described space standard: https://www.gov.uk/government/publications/technical-housing-standards-nationally-described-space-standard/technical-housing-standards-nationally-described-space-standard
- IRC room sizes: https://shop.iccsafe.org/media/wysiwyg/material/4117S24-Sample.pdf ; https://finehomebuilding.com/?p=519766
- WHO sanitation fact sheet: https://www.who.int/en/news-room/fact-sheets/detail/sanitation
- Global Data Lab refrigerator tables: https://globaldatalab.org/wealth/fridge/NGA/ ; https://globaldatalab.org/wealth/fridge/IND/ ; https://globaldatalab.org/wealth/fridge/ETH/ ; https://globaldatalab.org/wealth/fridge/MEX/
- Refrigerators by wealth: https://pmc.ncbi.nlm.nih.gov/articles/PMC2945336/table/T3 ; https://www.rds.oeb.ca/CMWebDrawer/Record/82228/File/document ; https://www.stats.gov.cn/sj/ndsj/2006/html/J1014e.htm
- Food preparedness: https://www.cambridge.org/core/journals/disaster-medicine-and-public-health-preparedness/article/household-food-and-water-emergency-preparedness-practices-across-the-united-states/8979E601643EB8751C4C38E82860E950 ; https://stacks.cdc.gov/view/cdc/29135/cdc_29135_DS1.pdf ; https://www.civilbeat.org/2019/08/kauai-14-days-of-emergency-supplies-who-has-that ; https://nippon.com/en/japan-data/h00957 ; https://www.oregonpublichealth.org/assets/2017_Conference/2017_Abstracts/assessment%20of%20disaster%20preparedness%20among%20households%20in%20corvallis.pdf
