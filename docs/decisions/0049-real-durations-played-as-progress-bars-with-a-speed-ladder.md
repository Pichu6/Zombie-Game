# 0049 · Durations are real; long waits are played as progress-bar activities, with a speed ladder

- **Date:** 2026-10-07
- **Status:** Decided by the user (intent below). The speed steps, the mini-game hooks and the offline question are Proposals or open.
- **Supersedes:** item 1 of [0048](0048-realistic-time-default-commit-on-observation-and-pins.md) ("a new game runs at 1:1 real time"), which misread the user's answer. Items 2 and 3 of 0048 stand. Returns the speed question to the position of [0032](0032-real-time-reference-and-generational-runs.md).

## Decisions

1. **Durations and rates are real for everything** (a hospital takes its real years, a crop its real days, a wound its real weeks). Real time is the reference, so long things are long and, as in real life, can be boring.
2. **Waiting is played, not sat through.** A long activity is shown as a loop animation with a progress bar. Mini-games can sit inside it "here and there" (and affect its rate or quality); mostly the player watches the bar and uses speed control.
3. **Speed control is how long things stay playable:** pause, slow, 1:1, faster speeds including **1 real hour = 1 game day**, and long steps. Long-time play of the Haven and Hearth kind is the reference (the user's example; not yet researched here).

## Open (not decided)

- **Which speed a new game starts at.** Recommendation, as in 0032: 1 real hour = 1 game day, with 1:1 one key away.
- **Whether time passes while the game is closed** (as in Haven and Hearth, whose world runs on a server). Single-player default proposed: no, unless a setting says so; on load the world would catch up with the same jump forms ([spec 27](../tier0/specs/27-detail-levels.md)).
