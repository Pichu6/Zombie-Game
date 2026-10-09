# 0056 · Alerts do not pause by default

- **Date:** 2026-10-08
- **Status:** Decided by the user, after playing the window: "I don't like alerts pausing the game; let that be a sandbox option, off by default, so the simulation can run smoothly if the player decides to just watch."
- **Changes:** [0055](0055-defaults-for-fixes-from-engineering-research.md) §2 item 7 (alerts that pause at normal speed), [spec 16](../tier0/specs/16-view-2d.md) rule 11 and [spec 40](../tier0/specs/40-time.md) rule 10. Agrees with the auto-pause default already in [sandbox parameters](../design/sandbox-parameters.md) ("no auto-pause").

## Decision

1. Alerts (a walker in sight of a group member, a bite, a group member down) are shown in the alert column and the log; time keeps running.
2. "Pause on alerts" is a sandbox option in the time bar, **off by default**. On, an alert pauses through `Pacer::alert` until Space.
3. "Rest until something happens" still stops at an alert: stopping there is what it is for.
4. The speed caps of direct control (`walking` 3x, `fight` 1:1) are unchanged: they apply only while the player drives a pop.

## Consequences

- A player who only watches sees the run play out without stopping.
- The per-event-type auto-pause classes of the sandbox parameters (ignore, log, toast, pop-up, pop-up and pause) are the later, finer form of this switch.
