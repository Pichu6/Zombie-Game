# 0023 · A sandbox world-life engine; the zombie outbreak is its first game

- **Date:** 2026-10-06
- **Status:** Decided by the user (framing below). Growth order and war and generations models are Proposals in [vision](../vision.md) and research `loop/27`.
- **Extends:** [0012](0012-world-simulation-layers.md), [0018](0018-sandbox-answers.md), [0022](0022-macro-layer-intent.md).

## Decisions

- **The engine is a sandbox world-life simulation:** one simulation with three layers of depth or zoom (world, regional, individual), "3 or more games in one", freedom of play and customisation, and no two runs alike.
- **The zombie game is built on it as the demo and first game:** the zombie apocalypse is an option on top of the simulation, not the simulation itself.
- **Range of play:** from a Sims / InZOI style life game (activities as in Second Life-type worlds and GTA Online), through Project Zomboid survival and World in Conflict style RTS, to global politics and generational strategy.
- **Examples the engine must allow:** watch a country at war, zoom into the battle and play a fit soldier; go anywhere and play John, who smokes, wears glasses and works 9 to 5 in IT at a tech company; go back to the world view, put a city under martial law, and let the simulation run to see what happens.
- **Watch or step in** at any layer, from macro decisions to individual survival.

## Defaults taken (Proposal, from research `loop/27`)

- The zombie game is the product that ships first; the engine is its identity and long-term promise.
- War and generational play are part of the vision but come after the outbreak game, in dependency order. Their requirements are recorded now so early code doesn't block them: soldiers are census people, battles return real dead and wounded, a quick battle resolver is calibrated against fought battles, the lifepath generator can run forward on the same rate tables it uses backward, and speeds above 10× run only in the world view with everything aggregated.

## Why

The user sees the depth of one simulation, playable at every scale, as what no one has done before. Dwarf Fortress, Kenshi and Songs of Syx show a small team can ship one world with several modes when every mode shares one rulebook; Spore and Star Citizen show what happens otherwise.
