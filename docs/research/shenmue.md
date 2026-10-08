# Shenmue, as a reference for the Individual layer

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [activities](../design/activities.md), [0019](../decisions/0019-activities-and-delegation.md), [0020](../decisions/0020-activity-answers.md), [spec 28](../tier0/specs/28-pop-levels.md), [spec 39](../tier0/specs/39-orders-and-belief.md). **Evidence:** checked by web search (Giant Bomb, Sega Retro, fan sites); the game was not played for this note. The sources disagree on how the weather is chosen; see below. **IDs:** SH.

Why: [0019](../decisions/0019-activities-and-delegation.md) and [0020](../decisions/0020-activity-answers.md) take its many activities; the user listed it again. Part of [reference games by layer](reference-games-by-layer.md).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| SH-1 | Shenmue (Sega, Dreamcast, 1999, Yu Suzuki) is an open-world adventure set in Yokosuka in 1986; Suzuki called the genre "FREE" (Full Reactive Eyes Entertainment). | reported | https://giantbomb.com/wiki/Concepts/Full_Reactive_Eyes_Entertainment |
| SH-2 | Most people and shops follow their own daily cycles: shops close at night, people are at work at work time. | reported | https://giantbomb.com/wiki/Games/Shenmue |
| SH-3 | The hero wakes at 8:30 and must be home by 11:30 at night, a curfew set by the story. | reported | https://giantbomb.com/wiki/Games/Shenmue |
| SH-4 | "Magic Weather": sources disagree; some say the real Yokosuka weather of 1986-87 is replayed (one says unlocked after finishing the game), another says days are drawn at random. | reported | https://www.phantomriverstone.com/search/label/Magic%20Weather |
| SH-5 | The investigation is done by asking people, and what they know depends on who they are. | reported | https://giantbomb.com/wiki/Games/Shenmue |
| SH-6 | The game has a real job (forklift driving at the harbour, on shifts, for wages) and pastimes (arcade, darts, capsule toys, a kitten to feed, move training). | reported | https://giantbomb.com/wiki/Games/Shenmue |
| SH-7 | Proposal: take a place that runs on a clock whether the player is there or not (rosters, routines, opening hours). | estimate | this note |
| SH-8 | Proposal: take asking people as a way to learn the world, with answers read from their beliefs (spec 39). | estimate | this note |
| SH-9 | Proposal: take jobs and pastimes as activities, hands-on the first time and automatic after (0020). | estimate | this note |
| SH-10 | Proposal: do not take authored story, scenes, quick-time events, a story curfew or replayed recorded weather; our weather is drawn from the climate statistics. | estimate | this note |

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
