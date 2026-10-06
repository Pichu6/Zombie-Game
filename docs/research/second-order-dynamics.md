# Second-order dynamics for procedural motion

**Status:** Research (2026-10-06). Not a decision. Pichu asked to keep it so it is easy to find when renderer or animation work starts.
**Source:** t3ssel8r, "Giving Personality to Procedural Animations using Math" (https://www.youtube.com/watch?v=KPoeNZZ6H4s). Pichu's full review, in Spanish: `D:/Proyectos/juego zombie/reference/video transcripts/TECH-REVIEW-second-order-dynamics.md`.

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
