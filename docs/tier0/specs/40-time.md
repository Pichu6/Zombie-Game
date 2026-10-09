# 40 · Time across levels

**Status:** Implemented and tested, headless (`World::jump`, the systems' wake ticks, the pacer); the window's time bar runs on the pacer since 2026-10-08 (rule 10). **Stage:** not Tier 0; the [layer jumps](../../design/layer-jumps.md) audit row 17. **Needs:** [05](05-clock.md), [06](06-save-replay.md), [27](27-detail-levels.md) rule 2. Design: [focus and compression](../../design/focus-and-compression.md) §9 (fast-forward is core play), [vision](../../vision.md) (alerts slow or pause). Decisions: [0049](../../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md), [0050](../../decisions/0050-start-speed-and-time-while-closed.md), [0039](../../decisions/0039-fill-the-layer-jumps.md).

## Purpose

Years per second at Macro, days at City, real seconds at Individual: one clock. Durations are real ([0049](../../decisions/0049-real-durations-played-as-progress-bars-with-a-speed-ladder.md)), so long things are played by speeding up, and the simulation must run cold at large multiples without changing what is true. This spec gives the one scheduler: each system names the next tick at which it has work, the world steps only those ticks, an attended body makes every tick a wake tick, and a pacer turns real seconds into ticks on the speed ladder, capped by what is attended.

## What the code forces

- **Every cold system already works by cadence.** The economy does its hourly work when `tick % hour == 0`, orders their daily work when `tick % day == 0`, grids only on commands, the registry only on commands unless bodies are attached. Their `update` is idle at every other tick. So a tick nobody needs can be skipped without changing anything, and the skipped world is **identical** to the stepped one, event log included (A1), which is stronger than the row's A2.
- **Commands apply at the start of a step** and scheduled events are popped by tick ([05](05-clock.md), [06](06-save-replay.md)). A jump must therefore stop for a pending command and for the next scheduled event, or they would land late.
- **Bodies step every tick** ([11](11-body-composition.md)–[14](14-action-executor.md)). A system with bodies attached returns `now` as its wake, and the jump degenerates to stepping: the row's "embodied bodies force a speed their rules allow".

## State

| Field | Notes |
| --- | --- |
| `System::next_wake(world, now)` | virtual: the first tick at or after `now` at which the system has work; default `now` (steps every tick); `kNever` for a system that only reacts to commands |
| `World::next_wake(now)` | `now` if a command is pending; else the minimum over the systems' wakes and the next scheduled event's tick |
| `Pacer` | the requested `Pace`; named caps (`at_most`, released-at tick); an alert text; the fractional tick carry; the dwell |

## Rules

1. **Wake ticks.** The economy wakes at the next hour boundary, orders at the next day boundary, grids never, the pop registry never unless bodies are attached (then now). A system that steps every tick keeps the default. New cold systems override `next_wake`; a wrong override can only make the world step more, never skip work, because `World::next_wake` takes the minimum.
2. **The jump.** `World::jump(to)`: while before `to`, find the next wake; if it is at or past `to`, set the clock to `to` (checking the ledger) and stop; else set the clock to the wake and take one ordinary `step()`. Nothing else: the step applies pending commands, runs every system's `update`, pops due scheduled events and checks the ledger, exactly as stepping does. Jumping to the present or the past is a no-op.
3. **Exactness.** A cold world jumped a week has the same event log, balances and state hash as the same world stepped tick by tick, because every skipped tick was one at which every system's `update` would have done nothing and no command or event was due (A1). The jump forms of [32](32-stocks-goods-and-money.md), [33](33-establishments-and-chains.md), [34](34-projects.md) and [37](37-flora-and-fauna.md) are what makes the hourly work exact over the skipped hours.
4. **An attended body.** With bodies attached the registry wakes now, so the jump steps every tick: the speed an area may run at is capped by what is attended there ([focus and compression](../../design/focus-and-compression.md) §9). Regions are not separate clocks in this slice: one body anywhere steps the whole world (open point 1).
5. **The ladder** ([0050](../../decisions/0050-start-speed-and-time-while-closed.md)): pause, 1/4, 1/2, 1:1, 3×, 10×, **1 real hour = 1 game day** (24×, the start speed), 60×, 360×, a day a second, a week, a month, a year a second. `pace_factor` is game seconds per real second; `faster` and `slower` walk the ladder.
6. **Caps and hysteresis.** A unit whose rules need it caps the speed by name ("Ana, fighting" at 1:1, a walking body at 3×); the effective speed is the request capped by the tightest cap in force; a released cap stays in force for the dwell (10 game seconds by default), so the speed does not flicker at the edge of a fight. `limiting` names the cap for the time bar; a cap above the request limits nothing.
7. **Alerts pause.** An alert sets the effective speed to pause until acknowledged (vision: "alerts and events slow or pause"); the request is kept.
8. **Real time to ticks.** `ticks_for(real_seconds, now)` = real seconds × the effective factor ÷ the tick length, whole ticks out, the fraction carried to the next frame. The app calls it each frame, then `World::jump(tick + ticks)`.
9. **Save.** The pacer's request, caps, alert, carry and dwell save with the app's time control; the wake ticks are derived, nothing to save. Time does not pass while the game is closed ([0050](../../decisions/0050-start-speed-and-time-while-closed.md) item 2); if that setting is ever on, loading calls `jump(now)` and the same rules catch the world up.

10. **The window uses the pacer** (added 2026-10-08, [0055](../../decisions/0055-defaults-for-fixes-from-engineering-research.md) §2; built 2026-10-08, [research fixes](../research-fixes.md) package 1). The sandbox time bar drops its own speeds and calls `ticks_for` each frame. Its caps: `walking` at 3x while the controlled pop moves under direct control, `fight` at Real while a walker is within its reach ([16](16-view-2d.md) rule 4). Its alerts: a walker comes into sight of a group member, a bite, a group member down ([16](16-view-2d.md) rule 11); they reach `Pacer::alert` and pause only when the player turns "Pause on alerts" on ([0056](../../decisions/0056-alerts-do-not-pause-by-default.md)).

## Invariants

- `World::next_wake(now) ≥ now`; with a pending command it is `now`.
- A jumped world and a stepped world over the same interval with the same commands at the same ticks have equal state hashes and event logs.
- The effective speed never exceeds the request, is pause while alerted, and equals the request when no cap below it is in force.
- Ticks produced over many frames equal the real time × factor ÷ tick length to within one tick.

## Tests (`test_time.cpp`)

- **A week cold (A1):** economy, orders, grids and a bodiless registry, with four commands at odd ticks: the jumped world equals the stepped world in hash, log size and balances, in about 170 steps against 1,209,600 ticks.
- **One call (A1):** `jump(3 days)` equals three days stepped; jumping to now or backwards changes nothing.
- **Wakes (A1):** hour boundaries, a scheduled event mid-hour, a pending command: `next_wake` returns each in turn.
- **An attended body (A1):** with bodies attached the wake is now and the jump steps every tick.
- **The pacer (A1):** the ladder's numbers and the start speed; 48 ticks a real second at 24× with the fraction carried; a named cap, hysteresis over the dwell, a cap above the request that limits nothing, an alert that pauses and is acknowledged; save and load.

## As built

- `engine/sim/include/zs/world.h`, `src/world.cpp`: `System::next_wake` (virtual, default now, `kNever`), `World::next_wake`, `World::jump`.
- Overrides: `EconomySystem` (next hour), `OrdersSystem` (next day), `GridSystem` (never), `PopSystem` (now with bodies, never without).
- `engine/sim/include/zs/pacer.h`, `src/pacer.cpp`: `Pace`, `pace_factor`, `pace_name`, `faster`, `slower`, `Pacer` (`cap`, `release`, `alert`, `acknowledge`, `effective`, `limiting`, `ticks_for`, save and load).

## Calibration

| Value | Source |
| --- | --- |
| The ladder | [0050](../../decisions/0050-start-speed-and-time-while-closed.md), [sandbox parameters](../../design/sandbox-parameters.md) §1.3 |
| Dwell 10 game seconds | Placeholder; tune against flicker in play |

## Not in this spec

Per-region clocks (one body steps the whole world here; chunks with their own `caught_up` are [27](27-detail-levels.md) Tier 1); the GTA-style switch that hides a promotion; threads ([0041](../../decisions/0041-threading-and-partition.md)); frame budgets (a jump of a year is a loop of 8,760 hourly steps; whether that fits a frame is the benchmark's question).

## Open

1. **Regions.** A fight in one town should not step the economy of a continent tick by tick. The fix is per-region wake ticks with the hot systems partitioned by chunk ([27](27-detail-levels.md), [0041](../../decisions/0041-threading-and-partition.md)); the interface here (a wake per system) extends to a wake per system per region without changing the world loop. Recommendation: when chunks exist.
2. **Hourly cadence as the floor.** A cold world still steps 24 times a day because the economy wakes hourly. For years-per-second play that is 8,760 steps a year, each cheap. Recommendation: measure before coarsening; the jump forms are exact at any cadence, so the economy could wake daily when nobody watches.
