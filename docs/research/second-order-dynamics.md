# Second-order dynamics for procedural motion

**Date:** 2026-10-06. **Status:** research note, not a decision. **Feeds:** [cameras and modes](../design/cameras-modes.md), [engine architecture](../design/engine-architecture.md), [art direction](../design/art-direction.md). **Evidence:** one source, t3ssel8r, "Giving Personality to Procedural Animations using Math" (https://www.youtube.com/watch?v=KPoeNZZ6H4s), through Pichu's full review of it, in Spanish, kept outside the repo at `D:/Proyectos/juego zombie/reference/video transcripts/TECH-REVIEW-second-order-dynamics.md`; the model and tips are the video's, the placement rules are ours; nothing was tested in our code. **IDs:** SO.

Pichu asked to keep it so it is easy to find when renderer or animation work starts.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| SO-1 | Follow motion can be modelled as y + k1 y' + k2 y'' = x + k3 x', with x the target and y the shown value. | reported | https://www.youtube.com/watch?v=KPoeNZZ6H4s |
| SO-2 | Three artist knobs set it: f (Hz) is response speed and does not change the shape; zeta is damping (0 rings forever, below 1 overshoots and settles, 1 is critical like Unity SmoothDamp, above 1 creeps); r is initial response (0 eases in, above 1 overshoots, below 0 anticipates). | reported | https://www.youtube.com/watch?v=KPoeNZZ6H4s |
| SO-3 | The coefficients are k1 = zeta / (pi f), k2 = 1 / (2 pi f)^2, k3 = r zeta / (2 pi f). | reported | https://www.youtube.com/watch?v=KPoeNZZ6H4s |
| SO-4 | It integrates per frame with semi-implicit Euler from y = x0, yd = 0, estimating xd as (x - x_prev) / T when unknown. | reported | https://www.youtube.com/watch?v=KPoeNZZ6H4s |
| SO-5 | It explodes when the step T reaches T_crit = sqrt(4 k2 + k1^2) - k1 (high f or a lag spike). | reported | https://www.youtube.com/watch?v=KPoeNZZ6H4s |
| SO-6 | Two stability fixes: sub-step ceil(T / (0.8 T_crit)) times, capped; or clamp k2 >= T^2/4 + T k1 / 2, which is O(1) and slightly slows motion on spikes. | reported | https://www.youtube.com/watch?v=KPoeNZZ6H4s |
| SO-7 | Use the k2 clamp for crowds (hordes), with k1..k3 precomputed per archetype. | estimate | this note |
| SO-8 | It is view-side only (render, animation, camera, UI): it smooths the snapshot, runs on the real frame dt outside replays and golden hashes, and never writes back to the sim. | estimate | this note |
| SO-9 | Zombie speed, gait and a hit's real displacement come from the body model; the spring only shapes how it looks, with parameters chosen from sim state (wounded, stunned, exhausted). | estimate | this note |
| SO-10 | Pitfalls: feed angles as the shortest-path difference wrapped to [-pi, pi]; reset y = x, yd = 0 on teleport; finite-difference xd is noisy on jumps and not needed with r = 0; f, zeta and r can change at runtime; chained systems settle slower. | reported | https://www.youtube.com/watch?v=KPoeNZZ6H4s |
| SO-11 | Starting values to tune by eye: smooth f=1, zeta=1, r=0; bouncy f=2, zeta=0.4, r=2; anticipation f=1.5, zeta=0.5, r=-2; the head uses body values with lower f so it lags. | estimate | this note |
| SO-12 | Pick it up the first time a view needs smooth follow motion (the third-person follow camera first), not before a vertical-slice scenario needs it. | estimate | this note |

## When to pick this up

The first time a view needs smooth motion that follows a moving target: the third-person follow camera (first candidate is the sandbox window), procedural animation (head lag, body sway, hit reactions) or UI/weapon feedback. Not before a vertical-slice scenario needs it.

## Where it goes, and where it doesn't

- **View side only** (render, animation, camera, UI). It smooths what the snapshot says; it never writes back to the sim ([engine-architecture §3](../design/engine-architecture.md#3-time-fixed-sim-tick-free-frame-rate), contract rule 8).
- Runs on the real frame `dt`, outside replays and golden hashes. The source's tip to integrate in the fixed step for determinism does not apply here: the sim is already deterministic and this is cosmetic.
- **Not in the sim.** Zombie speed, gait or a hit's real displacement come from the body model and rules. "Personality per zombie type" is not a free knob: a broken leg drags because the sim says so; the spring only shapes how that looks. Parameters may be chosen from sim state (wounded, stunned, exhausted).
- Camera: keep it subtle ([cameras-modes](../design/cameras-modes.md): shake optional, no sway to communicate states).

## The model

`y + k1·y' + k2·y'' = x + k3·x'`, where `x` is the target and `y` the shown value. Three artist knobs:

- **f** (Hz): response speed. Does not change the shape.
- **ζ**: damping. 0 rings forever; < 1 overshoots and settles; 1 is critical (Unity SmoothDamp); > 1 creeps.
- **r**: initial response. 0 eases in; > 1 overshoots; < 0 anticipates (moves the other way first).

```
k1 = ζ / (π f)    k2 = 1 / (2π f)²    k3 = r ζ / (2π f)
```

Semi-implicit Euler per frame, state `y = x0, yd = 0`, `xd ≈ (x − x_prev) / T` if unknown:

```
y  += T * yd
yd += T * (x + k3*xd - y - k1*yd) / k2      // uses the new y
```

**Stability:** explodes when `T ≥ T_crit = sqrt(4 k2 + k1²) − k1` (high f or a lag spike). Two fixes: (A) sub-step `ceil(T / (0.8 T_crit))` times, capped; (B) clamp `k2 ≥ T²/4 + T k1/2`, O(1), slightly slows motion on spikes. Use B for crowds (hordes), with k1..k3 precomputed per archetype.

## Pitfalls

- Angles: feed the shortest-path difference (wrap to [−π, π]); never integrate quaternion components without renormalising.
- Teleport or respawn: reset `y = x`, `yd = 0`.
- `xd` from finite differences is noisy when `x` jumps; with `r = 0` it is not needed.
- Changing f, ζ, r at runtime is safe (keep `y`, `yd`, recompute k and `T_crit`).
- Chained systems give richer secondary motion but settle slower.

Starting values (tune by eye): smooth `f=1, ζ=1, r=0`; bouncy `f=2, ζ=0.4, r=2`; anticipation `f=1.5, ζ=0.5, r=−2`. Head = body values with lower f, so it lags.
