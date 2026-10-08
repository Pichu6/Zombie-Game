# The Sims, as a reference for the Individual layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [interaction](../design/interaction.md), [spec 24](../tier0/specs/24-pop-ai.md), [spec 40](../tier0/specs/40-time.md), [0018](../decisions/0018-sandbox-answers.md), [0027](../decisions/0027-no-outbreak-game-and-parameter-catalogue.md). **Evidence:** the AI description is checked by web search (Game Maker's Toolkit's video on The Sims' AI, Ken Forbus's Northwestern lecture notes, a Spanish article citing a New York Times interview with Will Wright); the game's files and the original interview were not read. **IDs:** SIMS.

**Why:** [0018](../decisions/0018-sandbox-answers.md) and [0027](../decisions/0027-no-outbreak-game-and-parameter-catalogue.md) make a Sims-like life part of the game with no outbreak; [interaction](../design/interaction.md) already has a "Smart Objects" section. Part of [reference games by layer](reference-games-by-layer.md). Its population cap is in [sim-scale](sim-scale-in-other-games.md).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| SIMS-1 | The Sims 1 has eight motives (hunger, hygiene, fun, energy, bladder, social, comfort, room), each falling at its own rate, faster during related actions. | reported | https://users.cs.northwestern.edu/~forbus/c95-gd/lectures/The_Sims_Under_the_Hood_files/outline.htm |
| SIMS-2 | Smart objects advertise the actions they offer and how much each satisfies each motive; the Sim weights each offer by how low that motive is and picks from the top. | reported | https://amara.org/videos/cVoJS4OdmVql/en/4330642 |
| SIMS-3 | Advertisements let new objects add behaviour without new AI code. | reported | https://users.cs.northwestern.edu/~forbus/c95-gd/lectures/The_Sims_Under_the_Hood_files/outline.htm |
| SIMS-4 | The advertisement idea came from the pheromones of SimAnt. | reported | https://amara.org/videos/cVoJS4OdmVql/en/4330642 |
| SIMS-5 | Taken literally the scheme looks robotic, so motives are weighted unequally and choice keeps some randomness. | reported | https://amara.org/videos/cVoJS4OdmVql/en/4330642 |
| SIMS-6 | Will Wright reported early autonomy was so good that almost anything the player did was worse; breaking objects were kept to give the player chaos to manage. | reported | https://www.vidaextra.com/simuladores/que-sims-actuen-como-idiotas-nuestra-culpa-todo-farsa-will-wright-nos-sintamos-como-dioses |
| SIMS-7 | Work and school are rabbit holes: the Sim leaves the lot and returns with pay and changed motives. | reported | this note |
| SIMS-8 | Sims age through life stages and hold a relationship score per pair. | reported | this note |
| SIMS-9 | Take: objects advertise what they offer and the pop weighs offers by its needs; unequal weights and a little randomness; autonomy good enough to leave alone. | estimate | this note |
| SIMS-10 | Do not take: motives as abstract bars (ours come from the body), free building with no materials, rabbit holes (our work happens somewhere real). | estimate | this note |

## What it is

Maxis's life simulation (2000, with sequels in 2004, 2009 and 2014). The player builds a house and guides a household; the Sims act on their own when left alone.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Motives** | Eight needs in The Sims 1: hunger, hygiene, fun, energy, bladder, social, comfort, room. Each falls at its own rate, faster during related actions | Our needs come from the body ([spec 11](../tier0/specs/11-body-composition.md)): hunger is stomach and energy stores, not a bar |
| **Smart objects and advertisements** | Each object broadcasts the actions it offers and how much each one satisfies each motive. The Sim weights every offer by how low that motive is (a bed means more to a tired Sim) and picks from the top. The idea came from the pheromones of SimAnt | [Interaction offers](../design/interaction.md); the utility steps of pop AI ([spec 24](../tier0/specs/24-pop-ai.md)). Advertisements let new objects add behaviour without new AI code |
| **Tuning against robotic Sims** | Taken literally the scheme looks robotic or irrational, so motives are weighted unequally and choice keeps some randomness | Our weights are tuned by hand today ([spec 24](../tier0/specs/24-pop-ai.md): "no real source for utility weights") |
| **Autonomy against the player** | Wright reported that early autonomy was so good that almost anything the player did was worse; objects that break were kept so the player has chaos to manage | Delegation ([0019](../decisions/0019-activities-and-delegation.md)): automatic by default, hands-on optional |
| **Rabbit holes** | Work and school happen off-lot: the Sim leaves and comes back with pay and changed motives | Our compression jumps exactly ([spec 40](../tier0/specs/40-time.md)), so work can happen in a real establishment while unattended |
| **Build and buy mode** | Place walls, rooms and objects freely | Projects ([spec 34](../tier0/specs/34-projects.md)): building takes materials, labour and time |
| **Life stages and relationships** | Aging through stages; a relationship score per pair | Ages and births ([spec 28](../tier0/specs/28-pop-levels.md), [spec 44](../tier0/specs/44-child-mortality.md)); relationships not yet built |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| Advertisements: objects say what they offer, the pop weighs offers by its needs | Motives as abstract bars |
| Unequal weights and a little randomness so choice looks human | Free building with no materials |
| Autonomy good enough to leave alone | Rabbit holes: our work happens somewhere real |

## Questions this leaves (candidates for decisions)

1. Should [spec 24](../tier0/specs/24-pop-ai.md)'s utility choice move to advertisements carried by objects and places, as [interaction](../design/interaction.md) proposes, before the AI grows further?
2. Relationships between pops: a score per pair as in The Sims, or beliefs about each other ([spec 39](../tier0/specs/39-orders-and-belief.md))?

## Sources

- How the AI works (video transcript): https://amara.org/videos/cVoJS4OdmVql/en/4330642
- Ken Forbus, "The Sims under the hood": https://users.cs.northwestern.edu/~forbus/c95-gd/lectures/The_Sims_Under_the_Hood_files/outline.htm
- Wright on autonomy (Spanish, citing the NYT): https://www.vidaextra.com/simuladores/que-sims-actuen-como-idiotas-nuestra-culpa-todo-farsa-will-wright-nos-sintamos-como-dioses
