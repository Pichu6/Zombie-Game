# 0022 · Macro layer (Layer 3): director play, countries, events that don't wait, world setup

- **Date:** 2026-10-06
- **Status:** Decided by the user (intent below, from Pichu's notes and reference screenshots of Plague Inc, Hearts of Iron IV and Stellaris). Models in [macro layer](../design/macro-layer.md) are Proposals.
- **Extends:** [0012](0012-world-simulation-layers.md), [0016](0016-sandbox-setup-and-layer-flow.md), [0018](0018-sandbox-answers.md).

## Decisions

- **Feel:** Layer 3 is a mix of Plague Inc (watch the outbreak on the world map), Hearts of Iron IV and Stellaris (politics and decisions). Politics can deepen later.
- **Not a nation:** the player doesn't play one country; they watch the simulation and take decisions here and there.
- **The game knows the layer:** it detects which layer the player is in and switches the gameplay, information and camera to match.
- **Start in Layer 3:** the default run opens on the world map with the outbreak running.
- **Countries on the map:** hovering highlights a country and its borders; selecting it centres the camera and shows its information (population, politics, standings and more) and the decisions it allows.
- **Zoom is the way down:** zooming into a place on the map enters Layer 2 there (a city struggling, an overwhelmed hospital); selecting a building shows its information and actions (hire staff, attract workers); selecting a pop and taking control enters Layer 1 with that pop's objectives.
- **Events don't wait:** important dates and situations pop up ("Council meeting on 21 July") and prompt a decision; if the player doesn't act, the simulation decides by itself and carries on. It never waits for player input.
- **Layer 3 sets the rules others live under:** laws there (for example sending a share of medical supplies to global aid) are the cause of what is seen below (a hospital short of staff).
- **World setup before the run:** world size, population, number of countries, cities or population clustering, cultures; each auto-filled, customised one by one, or taken from a saved preset (Stellaris empire creation, XCOM base management feel). The zombie and the cure are customised Plague Inc style.
- **Variety:** different countries, cultures and outbreak origins make each run very different.

## Why

The user wants the world view to be a place to watch and steer the outbreak politically, and to see the cost of each decision in the layers below.

Research and the proposed model: [macro layer](../design/macro-layer.md).
