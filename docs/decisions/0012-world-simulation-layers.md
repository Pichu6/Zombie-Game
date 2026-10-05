# 0012 · One world simulation, three gameplay layers, play style chosen in setup

- **Date:** 2026-10-05
- **Status:** Decided by the user (the items below). Everything in [design/world-simulation.md](../design/world-simulation.md) not listed here is a Proposal.
- **Supersedes:** in [0011](0011-setting-scale-lenses.md), "three lenses on one zoom" becomes three gameplay layers with their own verbs, and the open "region" question is answered (real Earth by default). In [0010](0010-concept-v2.md), "start alone, the commander layer unlocks" becomes one of several play styles instead of the only one.

## Decisions

- **World creation:** the player creates the world from parameters in setup: world generation, then outbreak generation. **Real Earth is the default world**; generated worlds are a sandbox option. Population, settlements, infrastructure and supply chains come from that world.
- **Real data is the default** for the world and the outbreak (population, response timelines, CDC preparedness material, real pandemic behaviour), with sandbox settings to change the numbers.
- **One simulation, three gameplay layers** over it:
  - **Macro** (god view, globe): watch the simulation run, Plague Inc / Stellaris / Hearts of Iron style, and take decisions here and there through laws and edicts that change simulation parameters.
  - **City** (isometric RTS, Cities: Skylines / Infection Free Zone style): the local consequences of those decisions in pops and resources; local management.
  - **Individual** (isometric or over the shoulder, Project Zomboid / The Last of Us / Ghost Recon style): select a pop and play as it.
  - The user numbers them Individual = 1st, City = 2nd, Macro = 3rd; docs use the names.
- **One fact, every layer:** a hand wound or a power plant exists once and is represented in every layer.
- **Build orders create demand:** ordering a power plant creates jobs, material and labour needs that attract people, like zoning in Cities: Skylines, and the simulation delivers it. The same behaviour applies to other things. An individual cannot order a power plant.
- **Play style is chosen in setup, no main mode:** micro → macro (start as one person, grow), macro → micro (start in the god view, drop in), one layer only, or spectator (watch only).
- **Fog:** when playing an individual, the macro view shows what that character believes, not the truth.
- **Custom character**, Project Zomboid style: police officer, military, nurse, civilian or custom, inserted into the simulation. Pops get TTRPG-style backgrounds for the years they have lived.
- **Starting weapons follow real custody rules** by default (most police and soldiers don't keep weapons at home); game-friendly kits are a sandbox option.
- **The inspector (dossier) of a pop can contain wrong facts** when seen through a character's knowledge.
- **Outbreak warning:** about 7 days, or whatever is realistic for the virus settings; **all 7 days are live play** by default (sandbox setting).
- **Outbreak realism:** every system is entangled (infection affects people, then jobs, supply chains and infrastructure). Virus types spread at different speeds and outbreaks can fail. A slow, visible zombie probably would not spread far; the default must be calibrated on real outbreaks and human reaction.
- **Zombie origins:** selectable, Plague Inc style. Origins that change rules (not only numbers) are DLC.
- **Communication between pops by real behaviour:** voice and sound propagation, radios with coverage, phones; radio parts have quality.
- **Contextual options:** what you can do depends on knowledge, access to items and their quality.
- **Every item and resource has quality, condition and states.**
- **Destruction and durability** for doors, walls and fortifications.
- **Zombies don't tire** by default (sandbox setting).
- **Modular starting scenarios**, informed by what players build in Project Zomboid and similar games.
- **No test slice yet:** research and design first; Tier 0 stays as decided in [0010](0010-concept-v2.md).
- **No time or effort estimates** in docs or replies; scope is described by size and order.

## Why

The user sees the game as one outbreak simulation that can be played at several scales ("like 3 or 4 games in one"), built carefully from the smallest units up. Research (Universe Sandbox, Stellaris, GLEaM, Dwarf Fortress, Spore) showed the planet is cheap to simulate and people are expensive, and that scale-switching games work only when every mode is a view over one world with one rulebook.

## Consequences

- One simulation contract for every layer (rates, integer conservation, one owner per entity, keyed randomness). See [design/world-simulation.md](../design/world-simulation.md).
- People exist in three levels of detail (lazy census, records, full bodies), and looking at someone never changes them.
- Laws, edicts, scenarios and sandbox sliders share one validator: they may change numbers, never rules.
- Tier 0 is unchanged; the world layers come after it, in dependency order.

Details and research: [design/world-simulation.md](../design/world-simulation.md), [research/global-sim.md](../research/global-sim.md).
