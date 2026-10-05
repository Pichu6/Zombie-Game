# 15 · Tier 0 items and recipes

**Status:** Proposal. **Stage:** 4. **Needs:** [04](04-hazard-event.md), [08](08-assembly.md), [14](14-action-executor.md).

## Purpose

The ~20 item families and the few recipes that make the generator problem playable. Each one serves a real way to survive or die.

## Model: materials, properties, stocks and things (Proposal, 2026-10-05)

Items will number in the thousands, so an item is a few rows of data, never code.

1. **Materials** (`data/tier0/materials.toml`, ~20 rows): density, hardness, flammability, food energy, water content, each with a source. Physical numbers live only here.
2. **Item definitions** (`items.toml`) inherit from a parent (`board` ← `wood_part`) and list only what differs: material, volume or mass, slots ([08](08-assembly.md)), and **properties**.
3. **Properties** are one fixed list of numeric keys that processes test: `edge`, `blunt`, `lever`, `grip`, `container_ml`, `cloth_area_m2`, `absorbent`, `fuel_kj`, `food_kj`, `flat_board`, `fastener`… A key not given is derived from the material (e.g. `blunt` from mass and hardness) or is 0. Q and condition scale a property continuously; the property's governing part decides which Q counts (an axe's edge follows its head).
4. **Stocks and things.** Bulk resources (water, fuel, nails, food, ammunition) are ledger stocks: counts in an owner, no identity, cheap at any number. Only things with their own Q, condition or slots (a hammer, a door, a radio) are instances; canned food stays a stock of cans. Most of the thousands are stocks.
5. **States** are a small enum per family (fresh/stale/spoiled, safe/unknown, charge %), stored on the stock lot or the instance.

The Tier 0 families below are the first rows of these tables.

## Items

| Family | Notes | Feeds |
| --- | --- | --- |
| Generator (assembly) | engine, alternator, tank, spark plug, air filter, oil | Power, noise |
| Spark plug, air filter, engine oil | Spare parts with Q and condition | Repair |
| Fuel can, gasoline | Gasoline as stock in mL | Fuel crunch |
| Radio (assembly) | board, antenna, battery, speaker | Goal |
| Batteries | Charge as stock | Radio, flashlight |
| Flashlight | Light without the grid | Light field |
| Lamps (fixtures) | Power receivers | Light leak |
| Fridge (fixture) | Power receiver; keeps food cold | Spoilage |
| Hot plate (fixture) and camping stove with gas | Electric vs gas cooking: a comfort-vs-noticed choice | Cooking |
| Canned food | Doesn't spoil in Tier 0 | Food |
| Fresh food | Spoils by temperature: fridge or not | Food poisoning |
| Water bottles; tap water | Tap water state: safe, unknown (after the grid fails) | Thirst |
| Cloth, bandage | Dressings with Q and condition | Wounds |
| Disinfectant (alcohol) | Cleans wounds | Infection |
| Planks, nails | From furniture or found | Barricades |
| Hammer, screwdriver, wrench | Tools with Q | Gates and Q caps |
| Crowbar, kitchen knife | Melee | Combat |
| Splint (improvised) | Boards + cloth | Fractures |
| Backpack | Container with volume | Carry |
| Circular saw (power tool) | ~1.8 kW, very loud; makes salvage and barricades faster | Power, noise |
| Curtain or cloth cover | Covers a window: less light leak, a little less noise | Light leak |
| Worn clothes | Jacket, jeans, boots: coverage as a 17-bit region mask and one protection value; two garments on one region combine as 1 − (1 − p₁)(1 − p₂); no layers or insulation | Bites |
| Stockpile | A marked floor area or shelf that the haul job fills ([09](09-site-map.md)) | Hauling |
| Doors, windows, walls, barricades | Map assemblies ([10](10-fortification.md)) | Fortification |

## Recipes and processes (Tier 0)

| Process | Method | Inputs and tools | Notes |
| --- | --- | --- | --- |
| Barricade window or door | Hand | planks, nails, hammer | Loud; resumable |
| Salvage door or window | Salvage | screwdriver, crowbar | Returns hinges, strike plate, panel, glass ([10](10-fortification.md) chain) |
| Repair door part | Hand | replacement hinge, strike plate or panel, screwdriver or hammer | Restores the failed part in the chain |
| Salvage furniture into planks | Salvage | hammer or crowbar | Loud; returns real boards |
| Refuel generator | Hand | fuel can | Engine must be off (fire risk later) |
| Service generator | Hand | spark plug, filter, oil, wrench | Mechanic knowledge improves odds and Q |
| Repair radio | Hand | parts, screwdriver | Needs electronics knowledge (one found survivor has it) |
| Clean and dress a wound | Hand | water or disinfectant, bandage or cloth | First aid knowledge improves it |
| Make a splint | Hand | planks, cloth | |
| Improvised tourniquet | Hand | cloth or belt, a stick | Stops limb bleeding; ischaemia clock ([12](12-body-wounds.md)) |
| Cook | Hand | food, hot plate (power) or gas stove | Cooking lowers food poisoning from fresh food |

## Rules

1. Food spoilage is a hazard per batch driven by temperature: a powered, closed fridge keeps it slow; without power the fridge warms over hours. Eating spoiled food risks food poisoning (vomiting, water loss).
2. Tap water after the grid fails becomes "unknown" quality; drinking it risks gastrointestinal illness (water loss). Boiling (stove) makes it safe.
3. Every item has Q, condition and states; Tier 0 states: fresh/stale/spoiled, safe/unknown water, charged %, wet (not used yet).

## Tests

- No power for 24 h at room temperature: fresh food reaches spoiled inside the band. (A3)
- A barricade built by a skilled pop with a good hammer has higher Q than one by a novice. (A3)

## Calibration

| Value | Source |
| --- | --- |
| Food safety in a powered-off fridge (~4 h closed) | USDA/FSIS food safety guidance |
| Food poisoning and waterborne illness courses | CDC clinical summaries |
| Generator fuel use (~1–1.5 L/h at half load, 3 kW class) | Manufacturer spec sheets (estimate; tune) |

## Not in Tier 0

Firearms (candidate stretch: a shotgun is a perfect comfort-vs-noticed item), clothing layers, insulation and fit, crops, water network (stretch: tank and basement leak).
