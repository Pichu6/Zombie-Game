# Project Zomboid, as a reference for the Individual layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [body](../design/body.md), [body trauma](../design/body-trauma.md), [threats](../design/threats.md), [sandbox parameters](../design/sandbox-parameters.md), [spec 21](../tier0/specs/21-infection.md), [0010](../decisions/0010-concept-v2.md). **Evidence:** general knowledge of the game, with the infection figures checked by web search (third-party guides, not The Indie Stone); figures are reported and may differ between Build 41 and Build 42. **IDs:** PZ.

**Why:** the user named it first among the Individual-layer references; [0010](../decisions/0010-concept-v2.md) already takes its hero, its sandbox settings and its custom character. Part of [reference games by layer](reference-games-by-layer.md). Its zombie counts per cell are in [sim-scale](sim-scale-in-other-games.md).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| PZ-1 | Project Zomboid is an isometric zombie survival game by The Indie Stone, in early access since 2013, written in Java with much game logic in readable Lua; one survivor, a rural Kentucky map, permanent death | reported | this note (general knowledge) |
| PZ-2 | Moodles: a column of small icons, each a state with up to four levels (hungry, thirsty, tired, panicked, bleeding, injured, sick, cold, wet, stressed, bored, unhappy), a sentence on hover, no bars or numbers | reported | this note (general knowledge) |
| PZ-3 | With default settings about 7 % of scratches, 25 % of lacerations and 100 % of bites give zombie infection; death follows in about 2-3 days and disinfectant does not stop it | reported | https://pzfans.com/bitten_in_pz_build_41_vs_42_survival_hacks_unveiled/ |
| PZ-4 | The health panel shows wounds per body part (scratch, laceration, deep wound, bite, fracture, burn) with bandage state (clean, dirty) and ordinary infection | reported | this note (general knowledge) |
| PZ-5 | Skills are learned by doing: XP from use, books multiply the XP rate, occupations and traits are bought with points at character creation | reported | this note (general knowledge) |
| PZ-6 | Nutrition tracks calories, carbohydrates, proteins and fats; weight changes over weeks and with it strength and speed | reported | this note (general knowledge) |
| PZ-7 | Gunshots, alarms and meta-events such as the helicopter draw zombies from far away | reported | this note (general knowledge) |
| PZ-8 | Water and power shut off on a random day inside a range set in the sandbox | reported | this note (general knowledge) |
| PZ-9 | Sandbox settings offer dozens of sliders: zombie speed, infection transmission, population, loot rarity, start day | reported | this note (general knowledge) |
| PZ-10 | Barricades are planks over windows with health; cars have part condition, fuel and keys | reported | this note (general knowledge) |
| PZ-11 | While incubating, PZ shows "queasy" with the same symptoms as food poisoning | reported | this note (general knowledge) |
| PZ-12 | The game opens with "This is how you died" | reported | this note (general knowledge) |
| PZ-13 | Take the moodle column as the Individual layer's body lens, read from the real body, and the health panel by body part; do not make moodles the model | estimate | this note |
| PZ-14 | Do not take fixed infection chances per wound type: ours come from a dose from the wound (spec 21) | estimate | this note |
| PZ-15 | Take "This is how you died" as the tone of the death screen and chronicle, and meta-events that move the dead across a whole area; do not respawn the dead to meet a target count | estimate | this note |
| PZ-16 | Do not take XP bars and book multipliers as abstract numbers; reading should change what a pop knows | estimate | this note |

## What it is

An isometric zombie survival game by The Indie Stone, in early access since 2013. Written in Java, with much of the game logic in Lua files that ship readable for modders. One survivor, a large rural Kentucky map, permanent death. The opening text says it: "This is how you died."

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Moodles** | A column of small icons, each one a state with up to four levels: hungry, thirsty, tired, panicked, bleeding, injured, sick, cold, wet, stressed, bored, unhappy. Hover for a sentence. No bars, no numbers | Our body has the real state ([spec 11](../tier0/specs/11-body-composition.md), [12](../tier0/specs/12-body-wounds.md), [13](../tier0/specs/13-capabilities.md)) but no small readable summary. A moodle column is a lens on it, not a second model |
| **Health panel by body part** | Each part shows its wounds: scratch, laceration, deep wound, bite, fracture, burn, with bandage state (clean, dirty) and ordinary infection | Wounds per region already exist ([spec 12](../tier0/specs/12-body-wounds.md)) |
| **Zombie infection by wound type** | With default settings, about 7 % of scratches, 25 % of lacerations and 100 % of bites infect; death in about 2-3 days; disinfectant does not stop it | Ours is a dose from the wound ([spec 21](../tier0/specs/21-infection.md), [20](../tier0/specs/20-damage.md)), hidden from the UI. PZ's fixed table is the thing we replace |
| **Skills learned by doing** | XP from using a skill; books multiply the XP rate; occupations and traits bought with points at character creation | Capabilities from practice and body ([spec 13](../tier0/specs/13-capabilities.md)); custom character ([0012](../decisions/0012-world-simulation-layers.md)) |
| **Nutrition** | Calories, carbohydrates, proteins and fats; weight changes over weeks and with it strength and speed | Body composition in kg ([spec 11](../tier0/specs/11-body-composition.md)) |
| **Noise and sight** | Gunshots, alarms, a "helicopter event" and other meta-events draw zombies from far away | Noise field ([spec 18](../tier0/specs/18-noise-field.md)), light field ([spec 19](../tier0/specs/19-light-field.md)) |
| **Utilities shut off** | Water and power stop on a random day inside a range set in the sandbox | Stage 6 grid failure ([spec 17](../tier0/specs/17-power-network.md)); later the networks above ([spec 35](../tier0/specs/35-networks.md)) |
| **Sandbox settings** | Dozens of sliders: zombie speed, infection transmission, population, loot rarity, start day | [Sandbox parameters](../design/sandbox-parameters.md); settings change numbers, never rules ([0010](../decisions/0010-concept-v2.md)) |
| **Carpentry, barricades, vehicles** | Planks over windows with health; cars with part condition, fuel, keys | Fortification ([spec 10](../tier0/specs/10-fortification.md)); vehicles in [world vehicles](../design/world-vehicles-environment.md) |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| The moodle column as the Individual layer's body lens: few icons, levels, a sentence on hover, all read from the real body | Moodles as the model. PZ's moodles are the state; ours would only show it |
| The health panel by body part | Fixed infection chances per wound type; ours come from dose |
| "This is how you died" as the tone of the death screen and chronicle | XP bars and multipliers from books as abstract numbers; reading should change what a pop knows ([0030](../decisions/0030-pop-continuity-and-capability-loss.md)) |
| Meta-events (a helicopter, a siren) that move the dead across a whole area | Respawning the dead to meet a target count; ours come from the census ([spec 22](../tier0/specs/22-census.md)) |

## Questions this leaves (candidates for decisions)

1. Should the window show a moodle-style column for the selected pop as the first step towards "easy to tell what is going on"?
2. Which states get an icon, and from which body fields? A first list: hunger, thirst, fatigue, cold or heat, pain, bleeding, fear, sickness.
3. Does the player see "sick" while a pop is incubating, given that [spec 21](../tier0/specs/21-infection.md) hides infection from the UI? (PZ shows "queasy" with the same symptoms as food poisoning.)

## Sources

- Infection odds and timeline (third-party guides, Build 41 and 42): https://pzfans.com/bitten_in_pz_build_41_vs_42_survival_hacks_unveiled/ ; https://pzfans.com/bitten_project_zomboids_turn_timeline_exposed_b41_vs_b42/
- Zombie population model: https://projectzomboid.com/blog/news/2015/05/bring-out-your-zed/
