# TEG, as a reference for the Macro layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [macro layer](../design/macro-layer.md), [0044](../decisions/0044-information-scope-per-layer.md), [spec 39](../tier0/specs/39-orders-and-belief.md). **Evidence:** checked by web search (Wikipedia, shop listings, the manual of the free clone). Sources give 1976 or 1978 for the first edition; exact rules on pacts were not found. **IDs:** TEG.

Why: the user listed TEG with the Macro-layer references. Part of [reference games by layer](reference-games-by-layer.md).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TEG-1 | TEG (*Plan Táctico y Estratégico de la Guerra*) is an Argentine war board game based on Risk, first published in 1976 (one source says 1978). | reported | https://en.wikipedia.org/wiki/Plan_T%C3%A1ctico_y_Estrat%C3%A9gico_de_la_Guerra |
| TEG-2 | The board is a world map of 50 countries; T.E.G. II (1982) added ships, aircraft and nuclear rockets. | reported | https://en.wikipedia.org/wiki/Plan_T%C3%A1ctico_y_Estrat%C3%A9gico_de_la_Guerra |
| TEG-3 | Each player draws one of 15 secret objective cards (take certain continents, destroy a colour, and so on). | reported | https://www.fravega.com/p/teg-plan-tactico-y-estrategico-0014-990049553/ |
| TEG-4 | Players can also win by a common objective; one listing says 30 countries (not confirmed). | reported | https://www.fravega.com/p/teg-plan-tactico-y-estrategico-0014-990049553/ |
| TEG-5 | Players make pacts (such as not attacking across a border) and can break them; reputation is the only enforcement. Exact pact rules were not found. | reported | https://zff.dev/wfx/teg/wiki/manual |
| TEG-6 | Country cards for held countries are traded in for extra armies; holding a whole continent gives extra armies each turn. | reported | https://zff.dev/wfx/teg/wiki/manual |
| TEG-7 | Combat uses up to three dice per side; the defender wins ties. | reported | https://zff.dev/wfx/teg/wiki/manual |
| TEG-8 | Proposal: every faction, AI or player, has its own goal, which others learn only through beliefs and intelligence (0044). | estimate | this note |
| TEG-9 | Proposal: pacts hold only while both sides want them and leave a mark (changed beliefs) when broken. | estimate | this note |
| TEG-10 | Proposal: the board's simplicity (countries, borders, forces) is a model for the Macro view; do not take dice combat, armies as tokens or bonuses for holding a set of regions. | estimate | this note |

## What it is

*Plan Táctico y Estratégico de la Guerra*, an Argentine war board game based on Risk, first published in 1976 (one source says 1978). The board is a world map of 50 countries. T.E.G. II (1982) added ships, aircraft and nuclear rockets. A free computer clone is called *Tenes Empanadas Graciela*.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Secret objectives** | Each player draws one of 15 objective cards: take certain continents, destroy a colour, and so on. Players can also win by a common objective (one listing says 30 countries, not confirmed) | Faction goals: an AI country plays to its own aims, which the player learns only through beliefs and intelligence ([0044](../decisions/0044-information-scope-per-layer.md)) |
| **Pacts** | Players make agreements, such as not attacking each other across a border, and can break them; reputation is the only enforcement | Agreements between factions as beliefs about each other ([spec 39](../tier0/specs/39-orders-and-belief.md)); breaking one changes what others believe |
| **Country cards** | Cards for held countries, traded in for extra armies | Nothing direct |
| **Continent bonuses** | Holding a whole continent gives extra armies each turn | Real value of regions: resources, people, routes |
| **Dice combat** | Up to three dice each side; the defender wins ties | Our battles are resolved by the simulation, not dice |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| Every faction, AI or player, with its own goal | Armies as tokens; ours are people |
| Pacts that hold only as long as both want them, and leave a mark when broken | Dice combat |
| The simplicity of the board as a model for the Macro view: countries, borders, forces | Bonuses for holding a set of regions |

## Questions this leaves (candidates for decisions)

1. Does the player get a goal at all in Macro, or only AI factions, with the player free?
2. Are pacts explicit (an agreement record with terms) or only beliefs each side holds about the other?

## Sources

- https://en.wikipedia.org/wiki/Plan_T%C3%A1ctico_y_Estrat%C3%A9gico_de_la_Guerra
- Objective and country cards (shop listing): https://www.fravega.com/p/teg-plan-tactico-y-estrategico-0014-990049553/
- Free clone manual: https://zff.dev/wfx/teg/wiki/manual
