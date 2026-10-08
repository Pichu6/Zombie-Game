# Shenmue, as a reference for the Individual layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Why:** [0019](../decisions/0019-activities-and-delegation.md) and [0020](../decisions/0020-activity-answers.md) take its many activities; the user listed it again. Part of [reference games by layer](reference-games-by-layer.md).

**Evidence caveat:** checked by web search (Giant Bomb, Sega Retro, fan sites). The sources disagree on how the weather is chosen; see below.

## What it is

Yu Suzuki's open-world adventure (Sega, Dreamcast, 1999), set in Yokosuka in 1986. Suzuki called its genre "FREE" (Full Reactive Eyes Entertainment). Remastered with Shenmue II in 2018.

## Mechanics worth studying

| Mechanic | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **Everyone on a schedule** | Most people and shops follow their own daily cycles; a shop is closed at night and a person is at work at work time. Ryo wakes at 8:30 and must be home by 11:30 at night | Rosters and routines ([spec 28](../tier0/specs/28-pop-levels.md)); establishments with opening hours and staff ([spec 33](../tier0/specs/33-establishments-and-chains.md)) |
| **"Magic Weather"** | Weather follows long-term records for Yokosuka; a 1999 NHK documentary and a review say the real 1986-87 weather is used (one source says it is unlocked after the game is finished); another says days are drawn at random | Weather downscaled from the climate cell ([spec 36](../tier0/specs/36-fields-and-weather.md)): drawn, not recorded, but with the real statistics |
| **Asking around** | The investigation is done by asking people; what they know depends on who they are | Beliefs with source, hops and confidence ([spec 39](../tier0/specs/39-orders-and-belief.md)): asking a pop reads its beliefs |
| **A real job** | Driving a forklift at the harbour, on shifts, for wages | Work as an activity in an establishment, with a wage ([spec 32](../tier0/specs/32-stocks-goods-and-money.md)) |
| **Mini-games and pastimes** | Arcade games, darts, capsule toys, a kitten to feed, training moves in a parking lot | Activities, hands-on the first time and automatic after ([0020](../decisions/0020-activity-answers.md)) |
| **Quick-time events** | Button prompts during scenes | Nothing: we have no authored scenes |

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| A place that runs on a clock whether you are there or not | Authored story, scenes and quick-time events |
| Asking people as a way to learn the world, with answers from their beliefs | A curfew imposed by the story; ours would come from the pop's household |
| Jobs and pastimes as activities | Recorded weather replayed; ours is drawn from the climate |

## Questions this leaves (candidates for decisions)

1. Is "ask a pop what they know" an Individual-layer action from the start, reading [spec 39](../tier0/specs/39-orders-and-belief.md) beliefs?
2. Should a town's establishments show opening hours from their staff rosters?

## Sources

- FREE and NPC schedules: https://giantbomb.com/wiki/Concepts/Full_Reactive_Eyes_Entertainment ; https://giantbomb.com/wiki/Games/Shenmue
- Weather: https://www.phantomriverstone.com/search/label/Magic%20Weather ; https://www.bonusstage.co.uk/tag/shenmue/
