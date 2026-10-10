# Compute budget: what we measured, what the plans cost, where the risk is

**Date:** 2026-10-10. **Status:** working note, not a decision. Measured with the playable build in `play\zs.exe` (commit `bd4707e`, MSVC release, one thread) on the user's PC: Intel i7-12700KF (8 performance + 4 efficiency cores, 20 threads), 32 GB RAM. Everything else is our arithmetic, marked as such.

The user asked how we stand on compute, given everything simulated at different levels and everything still to add.

## 1. Measured today

| What | Result | Per unit |
| --- | --- | --- |
| The whole Tier 0 block (`zs --run`, 14 game days: 4 pops, about 60 walkers, generator, fuel, noise, light, AI, chronicle) | 10.3 s real | about 0.74 s per game day, **about 4.3 µs per tick**, about 230,000 ticks per real second: roughly 115,000× real time |
| Bodies only (`zs --bench`: 4 pops + 60 walkers, one game day) | 0.105 s | 0.6 µs per tick, about 10 ns per body per tick |
| Pop records (`zs --pop-bench` B1) | 1 million built in 0.40 s | 0.4 µs each, 17 bytes each saved (72 bytes in memory, measured earlier on GCC, spec 28) |
| Lazy pops (B2) | one snapshot 1.4–1.8 µs, the same at 1 million and 10 million | the world's size does not change the cost of reading one person |
| Region count (B2) | 0.1–3.9 µs | grows with recorded effects, not with people |
| Promotion (B3) | lazy → record 0.84 µs, record → lazy 0.26 µs | |
| Demography (B4) | 1 million founders over 60 years in 0.52 s | 0.34 µs per person visited |
| Rosters (B7) | a 10 % layoff of a 10,000-member roster 0.63 ms | about 63 ns per member |

## 2. The budget per tick

The clock ticks every 0.5 game s. The speed ladder sets how many ticks run each real second, and with that, how much CPU each tick may use:

| Speed | Ticks per real s | One core per tick | Six cores per tick |
| --- | --- | --- | --- |
| 1:1 (fights, forced) | 2 | 500 ms | 3 s |
| 3× (walking) | 6 | 167 ms | 1 s |
| 10× | 20 | 50 ms | 300 ms |
| 1 hour = 1 day (24×) | 48 | 20.8 ms | 125 ms |
| 60× | 120 | 8.3 ms | 50 ms |
| 360× | 720 | 1.4 ms | 8.3 ms |

Six cores is the plan once chunks run in parallel ([0041](../decisions/0041-threading-and-partition.md)); today everything runs on one.

## 3. What the plans would cost (our arithmetic, estimate)

Assumed: an active city agent (walking with paths, hearing and seeing) costs 0.5–2 µs per tick, about 10–30 times today's average, because Tier 0 walkers mostly idle. A car on the Intelligent Driver Model costs well under 0.2 µs.

| Scenario | Hot agents | CPU per tick | At 24× | At 360× |
| --- | --- | --- | --- | --- |
| Tier 0 today (measured) | 64 | 4.3 µs | 0.02 % of a core | 0.3 % |
| The demo's street corner | about 1,000 | about 1 ms | 5 % of a core | 72 % |
| Street corner + 4 pinned windows | about 5,000 | about 5 ms | 24 % | 3.6 cores |
| A big fight: 10,000 walkers and soldiers at 2 µs | 10,000 | about 20 ms | 1 core | 14 cores |
| The same fight at 1:1 (fights are capped there, 0056) | 10,000 | about 20 ms | **4 % of a core** | — |
| A warm city: 1 million people as counts in about 10,000 chunks, hourly, about 10 µs a chunk | — | 0.1 s per game hour | 0.07 % | 1 % |
| The cold world: billions of people as lazy pops | — | nothing until read; 1.5 µs a read | — | — |
| Run-ahead (RA-121) | — | +9 % to +25 % of the above, on spare cores | | |
| A server with 32 players, each at a street corner | 32,000 | about 32 ms | 1.5 cores | not offered (one speed per server, 0058) |

**Memory, estimate:**
- A million records is about 72 MB.
- 16 belief tracks per pop (XD-128) add 256 MB per million, so beliefs belong to records, never to lazy pops.
- Lazy pops cost nothing.

## 4. What this says

1. **The cost is attention × speed, not world size or the number of layers.**
   - Each place runs at one level at a time, so the layers do not add up.
   - The planet's billions are lazy and cost nothing until read.
   - A warm city costs a fraction of a percent.
   - What costs is the hot areas (the player's body, pinned windows, a watched spot, other players) multiplied by how fast the clock runs.
2. **The speed caps are what make big scenes affordable.** A 10,000-body fight would need a full core at 24× and 14 cores at 360×, but fights run at 1:1, where it is 4 % of a core. The caps already decided (0056, 0048: an attended area limits the speed) are a compute rule as much as a gameplay rule.
3. **Today's code is far from any limit.** Tier 0 runs about 115,000× real time, and reading a person costs the same at 10 million as at 1 million.
4. **The expensive combination is a busy hot area at high speed.** When the clock goes fast, watched-but-not-played areas should drop to warm (allowed where agreement is A1, RA-115), or the speed should cap.

## 5. Risks not measured yet (largest first)

1. **Sound at city scale.** `noise_field` runs a full least-loss search per source on every query; walkers cache per source cell. A city needs the two-level model (opening tables per building plus a street graph, HST-141).
2. **Paths for thousands of agents.** One A* per agent will not scale. Use shared potentials per goal (one field per destination, HST-11) and hierarchical paths on the chunk graph.
3. **Perception:** who sees whom is N × M without a spatial grid; checks should be spread over ticks (F.E.A.R. polled at 3 Hz, XD-80).
4. **Wake spikes:** half of all wakes come in about 2 % of the time, when entering a district (XD-91). It needs a wake budget in work units, prefetch and probation (XD-133..XD-136).
5. **AI decisions:** a utility or plan choice costs far more than a step, so decisions must be rare and event-driven, not per tick.
6. **Macro systems that scan everything:** the rule "no rule may scan all entities" (LC-13) must hold for the economy and orders at planet scale.
7. **Late starts:** ten years before play must run on cohorts and jump forms. Individuals, at 0.34 µs per person per visit, would take hours for billions; lazy pops make that unnecessary (MP-58).
8. **Rendering** is a separate GPU budget (renderer note). The screen caps what is drawn (LC-10), not what is simulated.

## 6. What to measure next (code)

1. **Per-subsystem time next to the per-subsystem hash** (ranked item 1): where each tick's microseconds go.
2. **A synthetic street corner:** 1,000 agents on a generated block with paths, hearing and sight; cost per agent per tick by system.
3. **Noise cost against map size and number of sources,** before and after the two-level model.
4. **Wake cost per chunk** and the spike when crossing into a new district.
5. **The 10,000-body fight at 1:1,** to confirm the cap makes it affordable.
