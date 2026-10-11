# 0075 · Parameters change in play through stations; setup is locked at creation; the AI holds stations too

- **Date:** 2026-10-11
- **Status:** Decided by the user. The user's words (lightly cleaned): "Certain jobs or stations, when a player makes an input in the world, it is like changing a parameter in the sandbox options in-game. Once you set the sandbox options and create the world you cannot change them anymore, like in most games; but certain jobs in the simulation do. Not all sandbox options need a station: changing world parameters is not realistic, a president changing country parameters is, and the same applies to different areas. This gives the AI tools to change the simulation parameters in real time." Asked whether the free sandbox panel mid-run stays as well: "keep both." The scope table below is a default taken by Claude ([0052](0052-one-executable-one-simulation.md)).
- **Builds on:** [sandbox parameters](../design/sandbox-parameters.md) §1.2 (the four kinds: setup, sandbox rule, policy, derived) and §1.1 (settings change numbers, never rules), [0066](0066-player-states-embodied-detached-observer.md) (stations gate deciding, not looking), [spec 39](../tier0/specs/39-orders-and-belief.md) (edicts act only through the ordinary commands below them, the signing as root cause), [0018](0018-sandbox-answers.md), [0074](0074-worlds-create-save-edit-fork-delete.md) (a world's setup forks once it has a save).

## Decision

1. **Setup is locked at creation.** The world's setup parameters (the planet, the nations' starting state, the outbreak's first cases, the scenario) are set before the world is made and never change inside it. Changing one makes another world (0074: a fork with the same seed).
2. **In play, a parameter changes through a station.** A station is a job or office in the simulation (a presidency, a ministry, a mayor's office, a company board, a household's head, a group's leader, a radio, a generator) that exposes a few parameters of its scope. Holding the station is what lets an actor set them, at a real cost and with a real delay, through the same records every actor uses (spec 39's edicts and their successors). This is the catalogue's **policy** kind, made concrete: the station is the office.
3. **Scope decides what is realistic.** No station changes the world: the planet, the physics, the sizes. A country's parameters change through its government's stations; a region's or town's through its own; a building's through its owner; a group's through its leader; a pop's own through the pop. Not every parameter gets a station, and a station with no parameters is decoration.
4. **The AI holds stations too.** An AI president, mayor, board or leader sets the same parameters through the same stations with the same costs and delays, so the simulation changes its own numbers in real time, by the actors that hold the offices, and every change traces to who decided it (spec 39's root cause). The player and the AI are two holders of the same stations.
5. **The free sandbox panel stays** (0018, §1.2 sandbox rules): a parameter can also be changed mid-run outside any station, free and immediate, and the save and chronicle tag it as a sandbox change. A player who wants the game plays stations only; the save says which they did.

## Scope table (default taken by Claude; one column added to the catalogue row by row as the systems are built)

| Scope | Stations | Parameters they may set (examples from the catalogue) | Cost and delay |
| --- | --- | --- | --- |
| World | none | the planet, the start date, the outbreak's rules | setup only |
| Country | the government's offices: head of state, treasury, interior, health, defence, communications | tax rates, levies, a curfew, a trade ban, a lockdown, a broadcast, conscription, aid | trust, money, time to take effect, the chain of commands below (spec 39) |
| Region, town | the mayor's office, the district's services | local rationing, patrols, a shelter opened, a road closed, a market day | the same, at town scale |
| Building, establishment | the owner, the manager | staffing, hours, prices, what to make, who may enter | wages, stock, the staff's willingness |
| Group | the leader | the group's rules: rations, watches, who scavenges, where to go | morale, obedience (0070: broken commitments are noticed) |
| Pop | the pop itself | their own day: what to do, with whom, what to believe | their body, their ties |

## Consequences

- [Sandbox parameters](../design/sandbox-parameters.md) gains §1.6 (this decision) and, row by row, a "station" column: which station may set the parameter in play, its cost and its delay; rows with none are setup or sandbox only.
- The commander layer (vision; the XCOM-like layer) is the set of stations the group holds, each exposing its rows. Military orders (pinned question 3 in the hand-off) are a station's parameters like any other.
- Spec 39's edict record is the first station mechanism; new stations extend it rather than adding a second path.
- The Atlas's country and pop cards get editing only through this: a card's editable rows are the rows some station exposes, and a player edits them by holding the station, or free and tagged through the sandbox panel ([spec 55](../tier0/specs/55-atlas.md) Open).
