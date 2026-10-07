# 0043 · A new game starts at 1 real hour = 1 game day; time does not pass while the game is closed

- **Date:** 2026-10-07
- **Status:** Decided by the user ("lets go with recommended options"). The exact speed steps remain a Proposal.
- **Answers:** the two open points of [0042](0042-real-durations-played-as-progress-bars-with-a-speed-ladder.md), and the open point of [0032](0032-real-time-reference-and-generational-runs.md) about the starting speed.

## Decisions

1. **A new game starts at 1 real hour = 1 game day**, shown on the clock, with **1:1 real time one key away**. The rest of the ladder (pause, slow, 3×, 10×, 60×, 360×, long steps) is as in [sandbox parameters](../design/sandbox-parameters.md) §1.3 and [spec 16](../tier0/specs/16-view-2d.md).
2. **Time does not pass while the game is closed**, unless a setting says so. If the setting is on, loading catches the world up with the same jump forms used by fast-forward ([spec 27](../tier0/specs/27-detail-levels.md)): one mechanism, not two.

## Notes

- This restores the position of [0032](0032-real-time-reference-and-generational-runs.md) and [vision](../vision.md) on the usual speed; durations stay real ([0042](0042-real-durations-played-as-progress-bars-with-a-speed-ladder.md)).
- A reference for the long-wait style (progress bars, real durations, many things running at once) is Haven and Hearth: [research](../research/haven-and-hearth.md). Its own day is about 7.3 real hours, much slower than ours.
