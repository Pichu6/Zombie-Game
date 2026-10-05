# Survival world: vehicles, logistics and spatial environment

Buildings and vehicles are explored at 1:1 scale, with no loading screens. Vehicles are tools, mobile shelters and transport for heavy loot. Perception depends on what the character sees and hears, not on what the camera shows. This doc complements `world-survival.md`. Power and water networks are in `infrastructure.md`.

Labels: **Decided** (the source says the user confirmed it), **Proposal** (not confirmed), **Open** (pending question).

General note: no source records explicit user confirmation for these items. Everything is a Proposal unless stated otherwise.

## Vehicles: physics and damage

Conflict: the prototype used raycast suspension from an addon. The research points to Jolt Physics for physics and collision. Jolt is adopted. Proposal.

| Mechanic | Rule | Status |
| --- | --- | --- |
| Suspension | Per wheel: spring, damper, separate longitudinal and lateral friction | Proposal |
| Drivetrain | Front, rear or 4x4; manual or automatic; ABS | Proposal |
| Center of mass | Shifts with the load; poor stowing or a loaded roof rolls over in fast turns | Proposal |
| Crash damage | Per component: punctured radiator overheats, broken alternator does not charge, blown tire, bent steering | Proposal |
| Load-bearing or concrete wall | Stops the vehicle dead; whiplash and chest trauma for occupants | Proposal |
| Partition walls, doors, furniture | Splinter and remain as walkable rubble | Proposal |
| Ice | Skids in winter | Proposal |

Conflict: free deformation of chassis and walls does not fit the budget. The research proposes pre-fractured pieces with a connectivity check. Proposal.

## Habitable interiors (motorhomes and trucks)

| Mode | Rule | Status |
| --- | --- | --- |
| Parked | Engine off and handbrake on: roof and walls are cut away and you walk inside | Proposal |
| Moving | Occupants use the chassis local frame; acceleration and turns apply relative inertia without jitter | Proposal |
| Doors | Opening connects to the outside: horde, fire or gas enter immediately | Proposal |

| Subsystem | Detail | Status |
| --- | --- | --- |
| 12 V network | Deep-cycle batteries separate from the engine; lights, pump, mini fridge for medicine and meat | Proposal |
| Solar | Flexible panels on the roof; daytime charging | Proposal |
| External 220 V inlet | Plug into a generator or pole when camping; it is a node of the power network | Proposal |
| Drinking water | 100–200 L tank with 12 V pump; drink and wash off scent | Proposal |
| Gray water | Must be emptied or it overflows | Proposal |
| Kitchen | External LPG cylinder; hot food improves morale | Proposal |
| Heating | Stationary diesel heater or wood stove; a blocked chimney builds up CO | Proposal |
| Blackout curtains | No light escapes at night; stealth against the horde | Proposal |

## Furniture and heavy cargo logistics

| Mechanic | Rule | Status |
| --- | --- | --- |
| Dismantle from the world | Crowbar, screwdriver, wrench or sledgehammer; fridges, stoves, shelves, beds, safes, generators | Proposal |
| Result | Skill decides whether it comes out whole or as parts and materials | Proposal |
| Two-handed carry | 60–90 kg in front of the torso; cannot run, climb or draw a weapon | Proposal |
| Fatigue | Stamina drops fast; without enough strength there is a risk of lower back injury | Proposal |
| Hand truck (dolly) | −75 % effort; rolls up to the ramp | Proposal |
| Cooperative carry | Two people share the weight at normal speed | Proposal (requires multiplayer, deferred) |

| Transport | Advantage | Risk | Status |
| --- | --- | --- | --- |
| Pickup | Fast loading | Rain and zombies climbing into the bed | Proposal |
| Box van or box truck | Large protected volume | — | Proposal |
| Trailer | Extends any car with a hitch | — | Proposal |

- Tie down with ratchet straps or ropes to rails or the floor. Proposal.
- Loose cargo: braking or turning at 70 km/h makes it bang around and lose condition, fly out of the pickup or roll the truck. Proposal.

## Handmade vehicle upgrades (late game)

| Upgrade | Materials | Function | Status |
| --- | --- | --- | --- |
| Reinforced bumper | Welder + U-channels | Ram zombies and gates without damaging the radiator | Proposal |
| Window grilles | Bars or welded mesh | Zombies cannot break glass to bite | Proposal |
| Roof platform | Welded pipes + planks | Lookout and shooting, out of zombie reach | Proposal |
| Side awning | Tarp + pipes | Cook and rest in the rain | Proposal |
| Auxiliary tanks | Tanks + hoses | Doubles the range | Proposal |

## Spatial interaction

Behavior implemented and tested in the prototype, with no design confirmation from the user. Proposal.

| Step | Rule |
| --- | --- |
| 1 | Validate body, object, access, capacity and reservations; find a path before accepting the order |
| 2 | Reserve actor and object; "approach" phase |
| 3 | The object stays at its origin while the actor walks |
| 4 | On arrival, check reach and interaction line; manipulation starts on the next step |
| 5 | Complete the transfer; if requirements change, interrupt and release the reservation |

- Travel never counts as manipulation work. Proposal.
- Canceling keeps the object at its origin. A door closed midway blocks crossing. Proposal.
- Interaction reach 1.5 m; body radius 0.25 m; max test speed 1.2 m/s, reduced by pain. Proposal (test coefficients, not calibrated).
- Saving mid-route restores position, path and progress. Proposal.

Conflict: the prototype used a 0.5 m 2D grid. The research proposes Recast and Detour for navigation. Proposal.

## Barricades and siege

| Mechanic | Rule | Status |
| --- | --- | --- |
| Doors and windows | Have structural integrity | Proposal |
| Banging | Zombies that hear noise inside without seeing you pound the nearest door; several break it fast | Proposal |
| Planks | Nail 1 to 4 planks across doors and windows | Proposal |
| Furniture | Pushing wardrobes or tables against openings adds break time | Proposal |
| Breach | Demolition charges open masonry walls | Proposal |

## Surface noise

| Surface | Footstep noise | Status |
| --- | --- | --- |
| Parquet or old wood | 55 dB; alerts the adjacent room | Proposal |
| Broken glass | 70 dB; alerts within 15 m | Proposal |
| Sheet metal | 65 dB | Proposal |
| Carpet or grass | 30 dB | Proposal |
| Puddle of water or blood | Splash and footprints | Proposal |

## Perception and cameras

| Rule | Status |
| --- | --- |
| The camera gives no information the character cannot see: behind them or occluded by walls | Proposal (listed as a prototype requirement) |
| Third person: ~120° peripheral, ~60° focal, horizon 200 m or more | Proposal |
| No active entities behind the character unless they make sound | Proposal |
| Isometric: fog of war outside the cone or behind walls; last known position in gray | Proposal |
| Buildings and vehicles 1:1 with no loading screens, with roof and wall cutaway | Proposal (listed as a prototype requirement) |
| Third person and isometric on the same character | Proposal (research: "keep") |

### Visual acuity

| Condition | Effect | Status |
| --- | --- | --- |
| Myopia | Far is blurry; more diopters, less sharp distance (e.g. zombies are blobs at 4 m) | Proposal |
| Hyperopia or presbyopia | Blurry under 0.8 m; cannot read manuals or assemble fine parts | Proposal |
| Astigmatism | Chromatic aberration and halos around headlights | Proposal |
| Glasses | Cancel the blur; get wet, fog up going from cold to warm, fall off or break in combat | Proposal |

## Visual levels of detail

Proposal. Distances come from the prototype and must be adjusted to the research's ~50 m hot level.

| Distance | Representation |
| --- | --- |
| Over 80 m | Prism block with baked texture; no lights or furniture |
| 30–80 m | Full exterior; interiors faked by shader |
| Under 30 m | 1:1 mesh, full interior, furniture collisions |

The transition uses a dithered fade. Fog reduces pop-in. Proposal.

## Open

- Persistence: per-sector files written in the background, or an embedded database like SQLite?
- Does the isometric view allow speeding up time with no threats nearby?
- Do passengers walk freely inside a moving vehicle from the first slice?

## Prototype sources

- vehiculos-interiores-logistica.md
- vision-entorno-espacial.md
- interaccion-espacial.md
- sistemas-supervivencia-mundo.md
- redes-infraestructura-servicios.md
- dinamicas-inmersivas-avanzadas.md
- engine-research (new engine research)
