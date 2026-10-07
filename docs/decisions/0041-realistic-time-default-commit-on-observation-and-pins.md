# 0041 · Real time is the default speed; observed detail is stored only where it saves memory; pins, with one view per layer

- **Date:** 2026-10-07
- **Status:** Decided by the user (the three answers below). The mechanisms are Proposals in [focus and compression](../design/focus-and-compression.md).
- **Supersedes:** item 3 of [0033](0033-restoring-cost-and-two-way-projects.md) ("a new game starts at 1 real hour = 1 game day") and the open point and recommendation at the end of [0032](0032-real-time-reference-and-generational-runs.md) (which speed a new game starts at). The rest of 0032 stands.

## Decisions

1. **Realistic time for everything is the default.** A new game runs at 1:1 real time: the clock, every duration, every project (a hospital takes its real years). Pause and fast-forward are always available. "1 real hour = 1 game day" is no longer the default; it can remain one named speed on the fast-forward ladder, and the exact ladder is a Proposal.
2. **Commit on observation, where it is better for memory.** Detail that can be re-derived from seeds and records is not stored until the player sees it or acts on it; then what was seen is kept as an exception ([0017](0017-slow-endgame-and-generation-on-the-go.md), [spec 28](../tier0/specs/28-pop-levels.md) rule 3, extended to projects and interiors). Used where it saves memory; anything that cannot be re-derived exactly is stored.
3. **The player can keep track of things (pins), and every layer gives its own information about the same thing.** A pin keeps a thing watched; what it shows depends on the layer the player is in: numbers, causes and forecasts at Macro, the building, staffing and flows at City, and what a person standing there could perceive and believe at Individual (fogged, not the truth).

## What follows

- **Fast-forward is core play, not an extra.** Real durations of years are played by speeding up, so the simulation must run cold at large multiples using jump forms ([spec 27](../tier0/specs/27-detail-levels.md)). The speed an area may run at is capped by what is attended there (an embodied body in a fight forces 1×); alerts and events slow or pause ([vision](../vision.md)).
- **The walking-versus-calendar problem shrinks.** [0021](0021-recommended-defaults.md) and [world simulation](../design/world-simulation.md) §8 sized the local zone because the calendar ran 24× against real-speed bodies. At 1:1 a game minute is a real minute, so that pressure applies only while fast-forwarding. Whether the zone crop is still needed is open in the [zoom continuum](../design/zoom-continuum.md) §5.
- **Tier 0 keeps its own scale for now.** The Tier 0 sandbox window's time bar (1× = one real hour per game day, [spec 16](../tier0/specs/16-view-2d.md)) is built; relabelling it is a later task, not part of this decision.
- Realism is still judged against real data ([0032](0032-real-time-reference-and-generational-runs.md)).
