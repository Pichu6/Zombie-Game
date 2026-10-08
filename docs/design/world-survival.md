# Survival world: weather, fuels, ballistics and ecology

The world decays on its own and forces adaptation. Fuel degrades, winter breaks pipes and ice, indoor gunshots damage hearing and food rots. Nature stays alive: crops, wildlife with a food chain, hunting, traps and insects. These systems turn time into a resource and a punishment. Vehicles, logistics and spatial environment are in `world-vehicles-environment.md`.

Labels: **Decided** (the source says the user confirmed it), **Proposal** (not confirmed), **Open** (pending question).

General note: no source for this section records explicit user confirmation. Everything is a Proposal unless stated otherwise.

Conflict: the engine research defers fire, temperature and weather until after the first slice. They are later built on top of the room graph. Proposal.

## Fuels

| Fuel | Shelf life | Failure | Remedy | Status |
| --- | --- | --- | --- | --- |
| Gasoline | 4–6 months | Oxidizes into gummy varnish; clogs injectors, fouls spark plugs, misfires | Chemical stabilizer or distillation | Proposal |
| Diesel | 12–24 months | Condensed water breeds fungi and bacteria that clog filters | Filtering, draining the tank bottom | Proposal |
| Bioethanol | Late game | — | Ferment and distill corn or potato from the garden | Proposal |
| Used vegetable oil | Late game | Only indirect-injection diesel | Collect it in kitchens, filter finely through cloth | Proposal |
| Producer gas (gasifier) | Late game | — | Wood chips; runs engines without petroleum | Proposal |

## Weather and winter

| Mechanic | Rule | Status |
| --- | --- | --- |
| Rain on clothing | Soaks the outer layer; soaked, it loses up to 80 % insulation and triples its weight | Proposal |
| Thin ice | Strength by thickness; if body + backpack or vehicle exceeds the load, it breaks at once | Proposal |
| Falling into 0 °C water | Cold shock, disorientation, respiratory arrest from hypothermia in under 90 s | Proposal |
| Frozen pipes | See `infrastructure.md` | Proposal |
| Deep snow | Slower, higher calorie burn | Proposal |
| Tracks in snow | Footprints and tire tracks last until the next snowfall; allow tracking prey, hordes or raiders | Proposal |
| Fog, rain, smoke | Reduce sight range for player and AI | Proposal |

## Natural water

| Mechanic | Rule | Status |
| --- | --- | --- |
| River current | Drag vector; swimming upstream drains stamina; cross diagonally using eddies | Proposal |
| Thermocline | Warm on top; below 3 m it is 8–10 °C; without neoprene it causes reflex hyperventilation while breath-holding | Proposal |
| Mud | Traps footwear (boots can be lost); clouds the water and hides obstacles | Proposal |
| Submerged zombies | Do not breathe or drown; wait in the mud and grab swimmers' ankles | Proposal |

## Weapon ballistics and acoustics

| Mechanic | Rule | Status |
| --- | --- | --- |
| Indoor gunshot | 5.56 rifle or 12G shotgun in an enclosed space: over 160 dB | Proposal |
| Consequence | Temporary total deafness, tinnitus, 15–30 s disorientation, without hearing protection | Proposal |
| Suppressor | Dampens the muzzle blast, not the sonic crack of bullets above 340 m/s | Proposal |
| Real stealth | Suppressor + heavy subsonic ammo (.45 ACP, subsonic .300 Blackout); less range | Proposal |
| Overpenetration | 5.56 and 7.62 go through solid doors and drywall | Proposal |
| Jamming | A weapon with mud or blood fails to eject; needs manual clearing | Proposal |

Gunshot sound travels through the room graph in `infrastructure.md`. Proposal.

## Spoilage and decomposition

| Process | Stages | Effect | Status |
| --- | --- | --- | --- |
| Perishable food | Fresh → Stale → Rotten | Shelf life in hours or days by temperature; rotten causes food poisoning | Proposal |
| Corpse | Fresh → Bloated → Decomposed → Skeleton | Smell and flies; lowers mental health; attracts scavengers | Proposal |
| Burned zombie | Flesh charred down to the skeleton | Reuses the anatomical skeleton | Proposal |
| Untreated water | — | Cholera and dysentery; boil, tablets or collect rain | Proposal |

## Fire on living beings

| Case | Rule | Status |
| --- | --- | --- |
| Burning human or animal | Dry clothing ignites on contact; 100 % pain, panic, cannot aim or reload; 2nd and 3rd degree burns | Proposal |
| Extinguishing | Stop, drop and roll, or get into water | Proposal |
| Burning zombie | No pain or panic; a living torch that keeps advancing and sets doors, barricades, curtains and people on fire | Proposal |

## Agriculture

State machine per plot, no complex biology. Proposal.

Plowed soil → (seed + watering) → Sprout → (days + sun) → Ripe → (not harvested) → Rotten. Sprout and ripe stages give off a smell that attracts pests.

| Factor | Rule | Status |
| --- | --- | --- |
| Plot | Moisture, sun and fertility | Proposal |
| Watering | Clean fresh water; salty or stagnant water ruins the crop | Proposal |
| Compost or cured manure | +40 % growth speed, adds nitrogen | Proposal |
| Looted NPK | Maximum yield, not renewable | Proposal |
| Homemade pesticide | Garlic/chili, tobacco, ash; repels aphids | Proposal |
| Commercial pesticide | Eliminates pests; requires a filter mask | Proposal |
| Pests | Rats, rabbits and crows eat sprouts; stopped with fences, scarecrows or traps | Proposal |

| Tool | Use |
| --- | --- |
| Shovel | Initial plowing, hard soil |
| Hoe | Furrows and weeding |
| Watering can or bucket | Watering |
| Sprayer | Plant protection products and fungicides |

## Wildlife and food chain

Lightweight three-state AI: feed → alert → flee to the burrow. Proposal.

| Level | Species | Behavior | Status |
| --- | --- | --- | --- |
| Zombies | — | Tear apart any animal that fails to escape | Proposal |
| 3. Medium predators | Foxes, wild dogs, wolves | Hunt levels 2 and 1 | Proposal |
| 2. Larger herbivores | Deer, birds | Smell humans downwind and flee before seeing them | Proposal |
| 1. Small prey | Rats, mice, rabbits, squirrels | Flee to the burrow at over 40 dB or a silhouette closer than 8 m | Proposal |

- Wildlife avoids infested areas by smell and moans. Proposal.
- Animals leave droppings: wet and warm, under 5 min old; dry and cold, over 1 h old. Proposal.

## Hunting and camouflage

| Mechanic | Rule | Status |
| --- | --- | --- |
| Crouching on wet grass or mud | Under 25 dB | Proposal |
| Dry branch | 60 dB snap that scares off prey | Proposal |
| Wind | Approaching with the wind in your face is mandatory with ungulates | Proposal |

| Scent camouflage | Effectiveness | Drawback | Status |
| --- | --- | --- | --- |
| Wet mud | 75 %; repels insects | Cracks as it dries; rain washes it off | Proposal |
| Resin and pine needles | 85 % | — | Proposal |
| Herbivore dung | 90 % against deer | Flies; 50 % sepsis or tetanus in open wounds | Proposal |
| Zombie viscera | 95 % against the horde | Nausea; 85 % lethal infection with open cuts | Proposal |

| Trap | Prey | Status |
| --- | --- | --- |
| Snare | Rabbits, foxes on trails | Proposal |
| Baited box | Rats, squirrels; seed, bread or cheese bait | Proposal |
| Leg-hold trap | Deer, or zombies that step on it | Proposal |

## Insects and foraging

| Element | Effect | Status |
| --- | --- | --- |
| Flies | On corpses; their buzzing signals carrion; they contaminate | Proposal |
| Mosquitoes and horseflies | Swamps and forests in summer; bite bare skin: itching, fever, infection | Proposal |
| Earthworms and crickets | Found when plowing or foraging; bait or emergency food, with nausea if raw | Proposal |
| Foraging | Firewood, stones for stone tools, berries, fibers, roots | Proposal |
| Toxic mushrooms and berries | Without knowledge you only see "wild mushroom"; eating it risks liver toxins | Proposal |
| Medicinal plants | With knowledge: plantain for bleeding, willow as a painkiller | Proposal |

## Open

- Do weather and temperature come before or after the first infrastructure slice?
- Do seasons last a fixed or configurable number of game days?
- Does wildlife use the same density promotion as hordes at the cold level?

### From research ([0054](../decisions/0054-research-notes-findings-and-links.md), 2026-10-08)

- Wildlife regrowth by body size and herbivore or carnivore biomass by biome: not found (only sandbox bands). ([world-systems-ecology](../research/world-systems-ecology.md))
- No livestock stocking rates by biome, no milk or draught output, no livestock per person by era. ([world-systems-ecology](../research/world-systems-ecology.md))
- The yearly soil-fertility regrowth rate (0.05-0.2) is a design choice from fallow durations alone. ([world-systems-ecology](../research/world-systems-ecology.md))

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [world-systems-ecology](../research/world-systems-ecology.md): World systems: flora, fauna, soils and fisheries in numbers

## Prototype sources

- quimica-combustibles-balistica-clima.md
- ecologia-fauna-agricultura.md
- sistemas-supervivencia-mundo.md
- dinamicas-inmersivas-avanzadas.md
- redes-infraestructura-servicios.md
- engine-research (new engine research)
