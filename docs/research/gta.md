# GTA, as a reference for the Individual layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Why:** the user listed GTA among the Individual-layer references, and [0020](../decisions/0020-activity-answers.md) and [0025](../decisions/0025-sandbox-engine-first-game.md) already name its activities and GTA Online. Part of [reference games by layer](reference-games-by-layer.md).

**Evidence caveat:** general knowledge of GTA V and GTA Online. The wanted-system search returned mostly mods and GTA VI speculation, so the details below are the widely known base behaviour, not checked numbers.

## What it is

Rockstar's open-world crime series. GTA V (2013) is a city with traffic, pedestrians, police and dozens of side activities; GTA Online adds owned businesses and property.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Wanted level** | One to five stars. A crime seen or reported raises it; the response escalates from patrol cars to helicopters and armed units; break line of sight and leave the search area to lose it. Pedestrians can phone the police | Beliefs with sources and confidence ([spec 39](../tier0/specs/39-orders-and-belief.md)): a witness is a pop who believes a crime happened and tells the police. The response is the police institution's real staff and vehicles |
| **Ambient population** | Pedestrians and traffic spawn around the player and vanish behind them; the mix depends on district and hour | Ours are persistent pops embodied near the player ([spec 29](../tier0/specs/29-embodied-pops.md)): the same visual density, but each one is someone |
| **Side activities** | Golf, tennis, darts, races, hunting, flight school, stock market: short, simple, optional | [0019](../decisions/0019-activities-and-delegation.md) and [0020](../decisions/0020-activity-answers.md): many simple activities, hands-on the first time |
| **The phone** | Contacts, jobs, messages, internet, camera: the in-world menu | A pop's phone as the place for contacts, beliefs heard and orders, only while the networks work ([spec 35](../tier0/specs/35-networks.md)) |
| **Businesses (Online)** | Buy a property, keep it supplied, sell the product; supply runs and raids | Establishments and chains ([spec 33](../tier0/specs/33-establishments-and-chains.md)); ownership |
| **Radio stations** | The car radio as the world's voice | Broadcasts ([spec 39](../tier0/specs/39-orders-and-belief.md)); the stage 9 radio ([spec 26](../tier0/specs/26-run.md)) |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| The escalation idea: the response grows with what the authorities believe | A star meter as the model. Ours comes from who saw what and what the police can send |
| Many short, optional side activities | Pedestrians that exist only while seen |
| The phone as the Individual layer's menu | Instant police response from nowhere; units travel from a station |

## Questions this leaves (candidates for decisions)

1. Is there a "how hunted am I" readout in the Individual layer, and does it show what the police believe or only what the player could know ([0044](../decisions/0044-information-scope-per-layer.md))?
2. Is the phone the menu in Individual, falling back to something else when the networks are down?

## Sources

- General knowledge; the wanted-system search found only fan mods and a GTA VI wiki comparison: https://thegameswiki.com/gta6/wiki/wanted-system/history/fdaab8d5-8738-4f79-b257-8d25058a4544
