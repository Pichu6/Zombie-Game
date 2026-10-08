# Catch-up: jump forms for drawn and competing rules

**Date:** 2026-10-07. **Status:** research note, not a decision. **Feeds:** [layer jumps](../design/layer-jumps.md), [spec 27](../tier0/specs/27-detail-levels.md), [spec 04](../tier0/specs/04-hazard-event.md), [spec 02](../tier0/specs/02-draw.md), [specs README](../tier0/specs/README.md). **Evidence:** papers and documentation listed in Sources, primary unless marked; Chatterjee 2005 read through a review, the SciPy and R manuals, the Unloaded Activity mod, the RimWorld wiki and the idle-game article are secondary. **IDs:** CJ.

Follows [pop-simulation-and-persistence.md](pop-simulation-and-persistence.md), whose biggest open risk was this: [spec 27](../tier0/specs/27-detail-levels.md) rule 2 asks every rule for a jump form `advance(state, from, to)`, and the sources solved it only for exact rates. This note gives the mathematics for drawn rules (arrivals, bites, infection clocks), competing rules (which of bleeding, dehydration and infection kills first) and coupled rules (a door that breaks and lets a horde in), with the agreement level each can honestly claim.

Notation: tick Δt = 0.5 s; interval [t0, t1], length T; rate λ per second; Λ = ∫λ dt over the interval.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CJ-1 | Gillespie's direct method (and Anderson's 2007 next-reaction method with time-dependent rates and delays) is exact for any Markov jump process whose rates depend on state and time; it is the universal jump for coupled rules | sourced | Gillespie 1977; Anderson 2007 |
| CJ-2 | An event loop costs the number of events, not ticks: an 18 h jump is a few dozen events instead of 129,600 ticks of 0.5 s (from CJ-1) | derived | this note |
| CJ-3 | Arrivals whose rate does not depend on what the arrivals change jump exactly as count ~ Poisson(Λ), with times as sorted uniforms over the interval | sourced | Gallager §2.5 |
| CJ-4 | Relaxations (fridge cooling, water and energy drift, logistic growth, healing, fuel) jump exactly with closed-form ODE solutions chained across breakpoints such as power on/off | sourced | this note (Newton cooling, standard ODEs) |
| CJ-5 | Consumption of an integer stock must draw Binomial(stock, 1 − e^(−λT)), never Poisson; channels sharing a stock draw one after another from what the previous left, so nothing goes negative | sourced | Tian and Burrage 2004; Chatterjee 2005 via review |
| CJ-6 | Competing causes of death: one Exp(1) draw E_k per (person, cause); death by cause k when cumulative hazard H_k reaches E_k; the earliest wins | sourced | Competing risks encyclopedia entry |
| CJ-7 | If the hot path also keeps one E_k per cause and tests H_k ≥ E_k each tick, hot and cold give identical death times and causes | estimate | this note |
| CJ-8 | Key draws by event identity (person, bite index, purpose), not by tick; counter = (canonical interval index, draw slot); sub-keys folded through the hash, never by arithmetic on ids | sourced | Salmon et al. 2011 (Random123); JAX PRNG design |
| CJ-9 | Tau-leaping freezes rates over a leap τ chosen so no rate changes by more than ε ≈ 3 %, and falls back to the exact method when τ < about 10/a0 | sourced | Gillespie 2001; Cao, Gillespie, Petzold 2006 |
| CJ-10 | Below about 30 items, use the exact event loop instead of a leap (the Cao–Gillespie–Petzold switch) | sourced | Cao, Gillespie, Petzold 2006 |
| CJ-11 | Thinning handles growing hazards with no closed-form H, but needs a bound λ* ≥ λ(t) over the whole interval; unbounded hazards (panic feedback) must be capped in their definition | sourced | Lewis and Shedler 1979 |
| CJ-12 | Quantised state: a fridge at 0.5 °C quanta is dozens of events over 18 h, with error bounded by the quantum | sourced | Kofman–Junco 2001 (QSS review) |
| CJ-13 | Hot per-tick probability must be 1 − e^(−λΔt), not λΔt, and decay must use exact factors e^(−kΔt), for hot and cold to agree | estimate | this note |
| CJ-14 | A horde pushing a door with constant Z breaks it at the k-th Poisson hit: one Gamma(k, μZ) draw; with growing Z, walk sorted arrival times with a piecewise-constant rate | derived | this note (from CJ-3) |
| CJ-15 | Infection clocks drawn once at the bite and keyed (person, bite index, purpose) become deterministic scheduled events, identical across hot and cold if hot rounds the turning to the same tick | estimate | this note |
| CJ-16 | Events generated per rule must be merged and applied in time order inside the interval; drawing all independently and applying at once is exact only when no rule's rate depends on another's outcome | estimate | this note |
| CJ-17 | If a post-leap check would cross zero, halve τ and redraw with a fresh counter, never the rejected one | sourced | Anderson 2008, post-leap checks |
| CJ-18 | 50 hot runs against 1 cold jump tests nothing; run about 2,000 cold jumps and compare 50 hot samples with Anderson–Darling (univariate) or energy distance with permutation p-values (joint vector) | estimate | this note; SciPy and R manuals (secondary) |
| CJ-19 | The specs README's "90 % interval over 50 runs" check only detects gross errors, a shift of about half a standard deviation | estimate | this note |
| CJ-20 | Ledgers kept as integers from a shared float; summing 129,600 tick increments differs from one closed form by rounding, so allow ±1 unit in the agreement test | estimate | this note |
| CJ-21 | Aggregate and detailed models are consistent only under restrictions: a rate rule is consistent only when it does not depend on sub-interval history; otherwise the event loop is the jump | sourced | Davis 1993; Hillestad and Juncosa (RAND) |
| CJ-22 | The Minecraft mod Unloaded Activity computes a reloaded chunk from missed random ticks with binomial and negative-binomial draws | reported | https://modrinth.com/mod/unloaded-activity |
| CJ-23 | RimWorld uses rare and long tickers every 250 and 2,000 ticks: multi-rate without catch-up | reported | https://rimworldwiki.com/wiki/Time |
| CJ-24 | Idle games compute offline progress as rate × elapsed with closed-form sums | reported | https://www.gamedeveloper.com/design/the-math-of-idle-games-part-i |
| CJ-25 | Stellaris evaluates jobs every 7 days | reported | Stellaris dev diary 170 |

## 1. Answer in one page

1. **One exact event loop is the universal jump for coupled rules.** Gillespie's direct method, or Anderson's 2007 next-reaction method with time-dependent rates and delays: each rule is a channel with a rate; draw the next event time, apply it, recompute the rates it changed, repeat until t1. Cost is the number of events, not the number of ticks (an 18 h jump is a few dozen events, not 129,600 ticks). It is exact for any Markov process with rates that depend on state and time, which is every rule we have.
2. **Independent external rules get closed forms.** Arrivals with a rate that does not depend on what the arrivals change: count ~ Poisson(Λ), times uniform in the interval, sorted. Relaxations (fridge cooling, water and energy drift, logistic growth): exact ODE solutions chained across breakpoints (power on, power off).
3. **Anything that consumes an integer stock draws Binomial, never Poisson.** Binomial(stock, 1 − e^(−λT)) per item, and channels sharing a stock draw one after another, each with what the previous left (Chatterjee 2005). That is contract rule 3 (ordered draws so nothing goes negative), now with its exact form.
4. **Competing causes of death are cause-specific hazards with one Exp(1) draw per cause.** Draw E_k ~ Exp(1) keyed (person, cause) once; death by cause k at the time its cumulative hazard H_k reaches E_k; the first to reach wins. If the hot path uses the same construction (test H_k ≥ E_k each tick instead of a coin per tick), hot and cold give **identical** death times and causes.
5. **Key draws by event identity, not by tick.** (person, bite number, "latent") rather than (person, tick). Then the hot run and the cold jump consume the same draw for the same event, and infection clocks are bit-identical across levels. Counter = (canonical interval index, draw slot); sub-keys folded through the hash, never built by arithmetic on ids.
6. **The agreement test must change.** Fifty hot runs against one cold jump cannot test a distribution. Run 50 hot against about 2,000 cheap cold jumps and compare with Anderson–Darling (univariate) or energy distance (the joint vector of people, kg, litres, walkers), plus deterministic invariants on every run (ledger never negative, people conserved). The "90 % interval" check in the specs README only catches gross errors.

## 2. Techniques

| Technique | Method | Exact when | Our recipe |
| --- | --- | --- | --- |
| **Direct method** (Gillespie 1977) | total rate a0 = Σ a_j; wait τ ~ Exp(a0); pick channel j with weight a_j/a0; apply; repeat | Always, for Markov jump processes | The generic `advance` for a chunk with coupled rules |
| **Next-reaction method** (Gibson–Bruck 2000; Anderson 2007) | one Exp(1) draw per channel viewed through its internal time T_j = ∫a_j; next firing when T_j reaches it; priority queue; rates may depend on time and state; delays (latent periods) are a future-event queue | Always | Same as above with day/night rates and scheduled turnings; one draw per channel, which is what makes hot and cold identical |
| **Poisson one-jump** | N ~ Poisson(Λ); given N, times are sorted uniforms (order statistics); for a time-varying but state-independent rate, inverse of Λ(t)/Λ or thinning | Rate does not depend on what the events change | Arrivals from off map when attention is held fixed over the interval |
| **Tau-leaping** (Gillespie 2001; Cao–Gillespie–Petzold 2006) | freeze rates over a leap τ, fire K_j ~ Poisson(a_j τ); choose τ so no rate changes by more than ε ≈ 3 %; fall back to the exact method when τ < about 10/a0 | Rates constant over the leap (then it is the Poisson jump) | Large counts (a cell's walkers) where the event loop would be long |
| **Binomial tau-leap** (Tian–Burrage 2004; Chatterjee 2005) | K_j ~ Binomial(remaining stock, a_j τ / stock); channels sharing a stock draw sequentially | Same mean as Poisson, bounded support | Every consumption of an integer stock: water, food, fuel, unbitten people |
| **Competing hazards** | min of independent Exp(λ_k) ~ Exp(Σλ_k), P(cause k) = λ_k/Σλ; for time-varying hazards, one E_k ~ Exp(1) per cause and t_k = H_k⁻¹(E_k) | Causes' hazards are functions of the deterministic trajectory, not of each other's draws | Which of bleeding, thirst, infection kills first |
| **Thinning** (Lewis–Shedler 1979) | candidates at a bound λ* ≥ λ(t), keep each with λ(t)/λ* | Bound holds on the whole interval | Hazards that grow (a wound getting worse) when no closed form of H exists |
| **Closed-form ODEs** | Newton cooling T(t1) = T_env + (T0 − T_env) e^(−kT); linear with source; logistic; chained across breakpoints | Exact | Fridge, body water and energy drift, healing, fuel |
| **Quantised state** (Kofman–Junco 2001, QSS) | a continuous variable emits an event only when it moves by one quantum | Error bounded by the quantum | A fridge at 0.5 °C quanta is dozens of events over 18 h |
| **DEVS** (Zeigler) | a component sleeps for its time advance, wakes on it or on an input | Formalism | Our chunk: sleeps until its own next event or a neighbour's message |

Counter-based randomness (Salmon et al. 2011, Random123; the JAX PRNG design): a draw is a pure function of (counter, key). "A unique (counter, key) tuple will always produce the same result." Sub-keys by `fold_in(key, data)` through the hash, never by adding small integers to an entity id.

## 3. The six hard cases

| Case | Jump algorithm | Agreement with hot |
| --- | --- | --- |
| **Arrivals** (rate from noise and light attention) | Attention is recomputed at warm ticks, so the rate is piecewise constant: per piece N ~ Poisson(Λ) keyed (chunk, "arrivals", hour), times sorted uniforms if anything downstream needs them. Hot uses p = 1 − e^(−λΔt) per tick (already [spec 04](../tier0/specs/04-hazard-event.md) rule 1), so hot is Binomial(ticks, p) → Poisson; the difference is O(λΔt), negligible | Same distribution (hot draws once per tick, cold once per piece) |
| **Bites** (each unbitten person at rate b·Z(t); bites change Z later) | Small group: exact event loop over {bite_i} ∪ {arrival} ∪ {departure}. Large: binomial tau-leap on "unbitten people", ordered draws | Same distribution (loop); same mean, slightly lower variance (leap); stock never negative |
| **Infection clocks** (latent, death, rising) | Drawn once at the bite, keyed (person, bite index, purpose), as [spec 21](../tier0/specs/21-infection.md) already does; from then they are scheduled deterministic events applied in time order (a turning changes Z for later bites) | Identical numbers, if hot applies the turning at the same time rounded to a tick |
| **Who kills first** (bleeding, dehydration, infection) | Each cause's hazard is a function of the body's deterministic trajectory (blood volume, water, fever). Draw E_k ~ Exp(1) keyed (person, cause) once; t_k = the time H_k reaches E_k (bisection on the closed form); death = earliest, cause = that one; a treatment scheduled inside the interval splits it. Deterministic thresholds (blood volume reaches zero) are just fixed t_k | Identical numbers if hot also keeps one E_k per cause and tests H_k ≥ E_k each tick; otherwise same distribution |
| **Fridge with power failing at a random time** | Failure time from the generator's fuel exhaustion (deterministic from fuel burn) or τ_f ~ Exp(λ_fail) keyed (fridge, "power", interval); temperature by Newton cooling with k_on until τ_f, k_off after; spoilage ∫r(T(t)) dt by a fixed Simpson rule (deterministic, so the same in both paths); kg spoiled = floor of the shared float | Identical numbers if hot uses the exact per-tick decay factor e^(−kΔt), not 1 − kΔt |
| **Horde pushing a door** (hits at rate μ per walker, damage d, door needs k hits) | With Z constant, break time is the k-th Poisson arrival: one Gamma(k, μZ) draw. With Z growing by arrivals, walk the sorted arrival times with a piecewise-constant rate. Stop at the break and hand the rest of the interval to the post-break rules (walkers pour in) | Same distribution; identical only if hot uses the same keyed construction rather than per-tick coins |

The group of four pops over 18 hours: needs are exact linear rates; jobs are a schedule; consumption draws Binomial from stocks in one canonical order (water, food, fuel, medicine); accidents and bites are channels in a small event loop. One pass, exact.

## 4. Pitfalls

1. **Negative stocks.** Poisson counts are unbounded. Binomial per consumption, sequential for shared stocks, canonical order. If a post-leap check would cross zero, halve τ and redraw with a fresh counter, never the rejected one.
2. **Time ordering.** Events generated per rule (arrivals, turnings, deaths, breaks) are merged and applied in time order inside the interval, because later rates depend on earlier events. Drawing everything independently and applying it all at once is exact only when no rule's rate depends on another rule's outcome.
3. **Rates that change because of the entity's own state** are the tau-leap error. Bound with the ε rule; below about 30 items, use the exact loop (the Cao–Gillespie–Petzold switch). Deterministic feedback (temperature to spoilage) goes through the closed form along the trajectory, not through freezing.
4. **Key reuse.** The same (entity, purpose, interval) for two draws (a count and its times) is a collision: counter = (interval index, draw slot). Hot and cold may consume different numbers of draws; what must match is the meaning of each draw. Interval indices must be canonical (the world clock's hour), so a jump over 18 h and a hot run split at hour 6 hit the same slots.
5. **Discretisation mismatch.** Per-tick p = λΔt is biased; use 1 − e^(−λΔt) (we do) and exact decay factors. Summing 129,600 tick increments differs from one closed form by rounding; keep ledgers as integers from a shared float and allow ±1 unit in the test.
6. **Thinning needs a bound.** A hazard that can grow without limit (panic feedback) must be capped in its definition.
7. **Dependence between causes.** If fever accelerates dehydration, draw the infection first (causal order), then the dependents.

## 5. Testing agreement

- 50 hot runs against 1 cold jump tests nothing. Cold jumps are cheap: run about 2,000 and compare the 50 hot samples against the cold empirical distribution with a one-sample Anderson–Darling test (Scholz–Stephens k-sample form in SciPy), which weights tails and detects scale and shape; for the joint vector (people, kg, litres, walkers) use energy distance with permutation p-values.
- The "90 % interval over 50 runs" check in the specs README has power only against gross errors (a shift of about half a standard deviation). Keep it as a smoke test; add means and variances separately, and Bonferroni across seeds.
- Invariants are deterministic and run on every sample: ledger never negative, people conserved, stages only forward.
- Davis and Hillestad (RAND, 1993): aggregate and detailed models are consistent only under restrictions; the formal version of "a rule written once as a rate is consistent only when the rate does not depend on sub-interval history". Where it does, the event loop is the jump, not a formula.

## 6. Where it lands in the specs (Proposals)

| Spec | Change |
| --- | --- |
| [04 hazard and event](../tier0/specs/04-hazard-event.md) | Rule 6: every hazard carries its cumulative-hazard form H(t0, t1) along the entity's trajectory, so it can be inverted for a jump. Rule 7: a per-entity exit set may keep one Exp(1) draw per hazard and fire when H_k reaches it; this is the preferred form for causes of death because it makes hot and cold identical. Rule 8: consumption of an integer stock draws Binomial, channels sharing a stock draw in order. |
| [02 draw](../tier0/specs/02-draw.md) | Keys name the event (person, bite index, purpose), not the tick; counters are (canonical interval index, slot); sub-keys folded through the hash. |
| [27 detail levels](../tier0/specs/27-detail-levels.md) | The jump of a chunk with coupled rules is an event loop (next-reaction method), with closed forms for rules that are independent inside the interval; the agreement test becomes 50 hot against about 2,000 cold with Anderson–Darling and energy distance. |
| [specs README](../tier0/specs/README.md) | A2 gains the stronger test. |

Precedents outside the literature: the Minecraft mod "Unloaded Activity" computes a reloaded chunk's state from the ticks it missed with binomial and negative-binomial draws (exactly our jump form for random ticks); RimWorld's rare and long tickers (every 250 and 2,000 ticks) are multi-rate without catch-up; idle games compute offline progress as rate × elapsed with closed-form sums; Dwarf Fortress skips two weeks on the calendar when a site is reclaimed (wiki, [pop-simulation-and-persistence.md](pop-simulation-and-persistence.md) §2.1).

## 7. Sources

Primary unless marked.

- Gillespie 1977, direct method: https://www.cl.cam.ac.uk/teaching/2324/Bioinfo/papers/DanielGillespie1.pdf
- Gibson and Bruck 2000, next reaction method: https://www.math.pku.edu.cn/teachers/litj/notes/stoch_topics2015/JPhysChemA_104_1876_GibsonBruck.pdf
- Anderson 2007, modified next reaction method with time-dependent rates and delays: https://people.math.wisc.edu/~anderson/papers/AndNRM.pdf
- Gillespie 2001, tau-leaping: https://zenodo.org/record/1231854
- Cao, Gillespie, Petzold 2006, leap size selection: https://cse.cs.ucsb.edu/sites/default/files/publications/leapsizeselection03.pdf
- Anderson 2008, post-leap checks: https://arxiv.org/pdf/0708.0377
- Tian and Burrage 2004, binomial leap: https://www.math.pku.edu.cn/teachers/litj/notes/stoch_topics2015/JChemPhys_121_10356_TianBinomial.pdf
- Chatterjee, Vlachos, Katsoulakis 2005, binomial tau-leap, via the review https://arxiv.org/pdf/1710.10860 (secondary)
- Auger, Chatelain, Koumoutsakos 2006, R-leaping: https://cse-lab.seas.harvard.edu/sites/projects.iq.harvard.edu/files/cse-lab/files/research_numerics_assa_1_auger2006a.pdf
- Lipková et al. 2018, S-leaping: https://arxiv.org/pdf/1802.00296
- Lewis and Shedler 1979, thinning: https://apps.dtic.mil/sti/tr/pdf/ADA062207.pdf
- Gallager, Discrete Stochastic Processes §2.5, order statistics of Poisson arrivals: https://eng.libretexts.org/Bookshelves/Electrical_Engineering/Signal_Processing_and_Modeling/Discrete_Stochastic_Processes_(Gallager)/02%3A_Poisson_Processes/2.05%3A_Conditional_Arrival_Densities_and_Order_Statistics
- Competing risks, cause-specific versus subdistribution hazards: https://bo.folk.ntnu.no/forskning/CompRiskEncycl.pdf
- Salmon, Moraes, Dror, Shaw 2011, Random123: https://users.cs.utah.edu/~hari/teaching/bigdata/random123sc11.pdf and https://github.com/DEShawResearch/random123
- JAX PRNG design (splittable keys, fold_in): https://docs.jax.dev/en/latest/jep/263-prng.html
- Quantised state systems: https://icc.fcen.uba.ar/en/discrete-event-simulation-of-continuous-time-systems-evolution-and-state-of-the-art-of-quantized-state-system-methods/ ; DEVS lecture notes (secondary): https://www.cs.mcgill.ca/~hv/classes/MS/lecture.DEVS.pdf
- Davis 1993, variable-resolution modelling: https://apps.dtic.mil/sti/pdfs/ADA282105.pdf ; Hillestad and Juncosa, "Cutting some trees to see the forest": https://www.rand.org/pubs/monograph_reports/MR189.html
- Scholz and Stephens k-sample Anderson–Darling (SciPy docs, secondary): https://docs.scipy.org/doc/scipy-1.15.0/reference/generated/scipy.stats.anderson_ksamp.html ; energy distance (R package manual): https://cran.ma.ic.ac.uk/web/packages/energy/refman/energy.html
- Unloaded Activity mod (secondary, working design): https://modrinth.com/mod/unloaded-activity
- RimWorld tick rates (secondary): https://rimworldwiki.com/wiki/Time
- Stellaris dev diary 170, job evaluation every 7 days: https://admin-forum.paradoxplaza.com/forum/developer-diary/stellaris-dev-diary-170-performance-and-other-technical-issues.1335134/
- Idle-game maths (secondary): https://www.gamedeveloper.com/design/the-math-of-idle-games-part-i
