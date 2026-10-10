# Coarse layers from the fine rules: equation-free and multiscale methods for detail levels

**Date:** 2026-10-10. **Status:** research note, not a decision. **Feeds:** [spec 27](../tier0/specs/27-detail-levels.md) (detail levels, agreement A1–A3), [spec 28](../tier0/specs/28-pop-levels.md), [spec 38](../tier0/specs/38-outbreak.md) (the one place a coarse model is already measured from the fine rule), [spec 40](../tier0/specs/40-time.md), [spec 06](../tier0/specs/06-save-replay.md), [focus and compression](../design/focus-and-compression.md), [layer jumps](../design/layer-jumps.md), research [layer-jumps-multiresolution](layer-jumps-multiresolution.md) (LJ: the military and game side of the same problem), [two-contagions](two-contagions.md) (TC: the first test world). **Evidence:**
- **Papers:** a local session read the bodies of Kevrekidis et al. 2003, Cisternas et al. 2004, Siettos 2010, E and Engquist 2003, Palmer's 2005 ECMWF introduction, Levine and Stuart 2022, Lamperti et al. 2017/2018 and Kuehn 2016 (arXiv PDFs converted to text). Vlachas et al. 2022 was read through an HTML extraction (`reported`); Liu et al. 2015 only as an abstract.
- **Talks:** the transcripts of 15 talks (EF-47 to EF-76) (YouTube captions, kept outside the repo) were read in full; findings from talks cite the video and the minute.
- **IDs:** EF.

The user asked what research could lead to a breakthrough (2026-10-10). Claude's answer, accepted by the user: **make every coarse layer derive itself from the fine rules** instead of writing each one by hand. A world could then be fully computed only where someone looks and still never contradict itself at Earth scale. Spec 38 already does this for one system: one keyed encounter rule is measured into the rates a region's compartments use, and the two runs are tested to agree in distribution. This note asks what applied mathematics knows about doing that in general, and where it breaks.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| EF-1 | The coarse time-stepper is `Tc = M ∘ Tf ∘ μ`: choose coarse statistics (restriction M); choose a lifting μ with `Mμ = I` (one-to-many); lift a coarse state to micro realisations; run the fine simulator for a short time; restrict. It assumes the coarse results obey a closed evolution law that is never written down | sourced | Kevrekidis et al. 2003, *Commun. Math. Sci.* 1(4), §2, https://arxiv.org/abs/physics/0209043 |
| EF-2 | "Healing": with separated time scales, the higher moments that lifting sets arbitrarily relax onto functionals of the low moments ("fresh" to "mature" states); until then they add noise. Some systems heal from any lifting; others (surface kinetic Monte Carlo, nematic rods) need a separate equilibration step and may need pair probabilities as extra coarse variables | sourced | Kevrekidis et al. 2003, §3 |
| EF-3 | Projective integration: run k inner steps, then extrapolate the coarse state over M steps. Fast modes are damped by a power of k while extrapolation error grows only linearly in M; the method needs a spectral gap | sourced | Kevrekidis et al. 2003, §4 |
| EF-4 | "Closure failure": the chosen coarse variables can stop closing as conditions change. Validation starts where an explicit macro model is known and reproduces it, then follows a continuation path into the regime of interest, validating all the way; whether coarse behaviour exists "cannot be a priori established" | sourced | Kevrekidis et al. 2003, §6, §10 |
| EF-5 | When single runs are too noisy, coarse determinism returns only for ensemble statistics (a Fokker–Planck-type law for the distribution of coarse states); variance reduction is called absolutely crucial | sourced | Kevrekidis et al. 2003, §5.3–5.4, §10 |
| EF-6 | Measured gaps: in the nematic-rod model the next-fastest moment is about 500× faster than the order parameter, which justifies a one-variable closure; the surface-kinetics coarse bifurcation diagram was checked against mean-field and quasi-chemical closures and direct runs | sourced | Kevrekidis et al. 2003, §9 |
| EF-7 | How often patches exchange data matters even when short runs look identical: in a gap-tooth model a 3× longer reporting horizon visibly changes the long-term attractor | sourced | Kevrekidis et al. 2003, §6 |
| EF-8 | Gray-box identification: coarse-stepper data can fit only the unknown parts of a partly known right-hand side | sourced | Kevrekidis et al. 2003, §10 |
| EF-9 | Epidemic precedent: an SIRS model with strains on a lattice (N from 10² to 10⁷) was given a 23-number coarse state (S, I, R plus 10-point inverse distribution functions for I and R), lifted by piecewise-linear inverse CDFs | sourced | Cisternas, Gear, Levin, Kevrekidis 2004, *Proc. R. Soc. A* 460, §2–3, https://arxiv.org/abs/nlin/0310011 |
| EF-10 | That study used ensembles of 250 copies of about 10,000 individuals, bursts of 0.05 yr, healing of about 0.01 yr and coarse modes of about 0.1 yr: a gap of about 10× | sourced | Cisternas et al. 2004, §4.1 |
| EF-11 | Coarse fixed points were found by least squares; noise persisted after averaging 10–250 realisations, and continuation accuracy was "rather low" | sourced | Cisternas et al. 2004, §4.2–4.3 |
| EF-12 | It reports no quantitative error between coarse and direct runs; agreement is shown only as plots | sourced | Cisternas et al. 2004, §4.1 |
| EF-13 | Finite populations have absorbing extinction states: the authors picked sizes where extinction was negligible and say rare-event models suit small N; a front speed had not converged even at N ≈ 10⁸; at low mutation rates the dynamics are long quiet spells broken by jumps triggered by one individual | sourced | Cisternas et al. 2004, §2, §5 |
| EF-14 | Network precedent: SIRS on a random regular graph (N = 20,000, degree 4), discrete-time and synchronous | sourced | Siettos 2010, §3–4, https://arxiv.org/abs/1012.4194 |
| EF-15 | Lifting on a network fixes densities while simulated annealing drives pair densities to targets; trajectories fall fast onto a one-dimensional manifold, so densities close and pair densities are slaved | sourced | Siettos 2010, §2.2, §4 |
| EF-16 | The coarse bifurcation diagram has turning points at p_SI ≈ 0.138 and 0.15 with a bistable band between; unstable states cannot be reached by long plain runs; mean-field and pair approximations "introduce systematic bias" | sourced | Siettos 2010, §1, §4 |
| EF-17 | The formal conditions are smooth slow and fast maps and a normally hyperbolic slow manifold (all fast eigenvalues with negative real part) | sourced | Siettos 2010, §2.1 |
| EF-18 | For rare events in an agent-based market model, the drift and diffusion of an effective Fokker–Planck equation were estimated from ensembles of short bursts, with diffusion maps finding the coarse variable | reported | Liu, Siettos, Gear, Kevrekidis 2015, *Math. Model. Nat. Phenom.* 10(3), doi 10.1051/mmnp/201510307 (abstract only) |
| EF-19 | Heterogeneous multiscale methods: a compression Q and reconstruction R with `QR = I` (R not unique); the macro rate is estimated as `(Q s(Δt) R U − U)/Δt` with micro relaxation ≪ Δt ≪ macro time; the dependence on R fades after relaxation | sourced | E and Engquist 2003, *Commun. Math. Sci.* 1(1), §2–3, https://arxiv.org/abs/physics/0205048 |
| EF-20 | Two problem types: type A, where the macro model is valid except near defects that need micro locally; type B, where micro supplies a missing closure everywhere | sourced | E and Engquist 2003, §1 |
| EF-21 | With strong relaxation the micro run's last value is the estimate (a flux settled after about 35 micro steps); with weak relaxation only time or ensemble averages converge | sourced | E and Engquist 2003, §5.2 |
| EF-22 | Error budget: total error is bounded by the macro scheme's own error plus the largest data-estimation error, so the two can be tested separately | sourced | E and Engquist 2003, §6 |
| EF-23 | Missing macro data may be estimated from several micro runs with different randomness | sourced | E and Engquist 2003, §4.2 |
| EF-24 | Stochastic sub-grid terms serve three purposes: representing model uncertainty in ensembles, reducing systematic error through noise-induced drift, and giving realistic internal variability | sourced | Palmer 2005, ECMWF workshop introduction, https://www.ecmwf.int/sites/default/files/elibrary/2005/11580-stochastic-dynamic-parametrisation-weather-and-climate-prediction-models-introduction.pdf |
| EF-25 | Ensembles of deterministic models are underdispersed: their spread is smaller than their error, more so at longer range | sourced | Palmer 2005, §1 |
| EF-26 | Parametrisation error grows with the parametrised tendency, so the noise is multiplicative; since 1999 ECMWF multiplies the parametrised tendency by a random number, with clearly better probabilistic scores | sourced | Palmer 2005, §1 |
| EF-27 | A deterministic model over-populates the dominant regimes and under-populates the others; stochastic backscatter of about 10 % of dissipated energy fixes the regime frequencies | sourced | Palmer 2005, §1, citing Jung et al. 2005 |
| EF-28 | Paradigm: a sub-grid scheme should be "a specific realisation" of the unresolved scales, not their mean | sourced | Palmer 2005, §1 |
| EF-29 | Learned Effective Dynamics: an encoder to a small latent state, a recurrent model that steps it, and a decoder; it alternates micro phases with latent phases and needs a micro warm-up | reported | Vlachas, Arampatzis, Uhler, Koumoutsakos 2022, *Nat. Mach. Intell.* 4, https://arxiv.org/abs/2006.13431 |
| EF-30 | On a reaction–diffusion test, latent-only steps were 60× faster than the fine solver (error ≈ 0.019); alternating equal micro and latent phases cut the gain to 2× and the error to ≈ 0.003 | reported | Vlachas et al. 2022, §II.1 |
| EF-31 | Flow past a cylinder: about 100× faster at Re = 100; at Re = 1000 the drag error stayed ≈ 0.15 even with micro phases, because the error concentrates near the body | reported | Vlachas et al. 2022, §II.2–II.3 |
| EF-32 | Validation used held-out initial conditions (10–100) and compared stochastic outputs by Wasserstein and histogram distances | reported | Vlachas et al. 2022, §II and SI |
| EF-33 | Its stated limits: no mechanism to decide when to switch scale, no generalisation to regimes unlike the training data, error accumulation in latent-only runs | reported | Vlachas et al. 2022, §III |
| EF-34 | For learned model error that is memoryless and linear in its parameters, the generalisation error shrinks as 1/√T with the training length T | sourced | Levine and Stuart 2022, *Commun. AMS* 2, §5, https://arxiv.org/abs/2107.06658 |
| EF-35 | Hybrid models (a mechanistic model plus a learned residual) beat purely learned ones on accuracy, data efficiency and parameter efficiency, whether the mechanistic part is good or poor | sourced | Levine and Stuart 2022, §6.1 |
| EF-36 | In multiscale Lorenz '96, learning from scratch did worse than the nominal model; a memoryless closure kept the long-run statistics under weak scale separation, but trajectory accuracy failed where memory is needed | sourced | Levine and Stuart 2022, §6.1, §6.2.4 |
| EF-37 | Their metrics: validity time (first time relative error passes 5 %), KL divergence between invariant measures, normalised difference of autocorrelation functions | sourced | Levine and Stuart 2022, §6.2.2 |
| EF-38 | A learned model with memory improved statistics but went unstable beyond T = 500; purely learned discrete-time models are unstable at small steps, and hybrids built on an integrator avoid it | sourced | Levine and Stuart 2022, §6.3.2, §7 |
| EF-39 | An agent-based model surrogate: a gradient-boosted classifier, trained by active learning on a budget, predicts whether a parameter vector meets a calibration test (two-sample KS against data, p > 5 %) | sourced | Lamperti, Roventini, Sani 2018, *JEDC* 90, §3–4, https://arxiv.org/abs/1703.10639 |
| EF-40 | On a 12-parameter market model, budgets of 250–2,500 runs gave F1 ≈ 0.8 at 2,500 and a true-positive rate of about 95 %, with predictions about 500× faster than running the model | sourced | Lamperti et al., §4.2–4.3 |
| EF-41 | On a growth model the surrogate was 3,750× faster; the gain grows with the cost of the agent model | sourced | Lamperti et al., §5.3 |
| EF-42 | With stochastic runs (each evaluation the mean of 100 seeds), precision reached 94–99 % | sourced | Lamperti et al., §5.4 |
| EF-43 | In the main exercises the seed was fixed, so "out of sample" meant new parameters, not new seeds; kriging meta-models are usually checked on 10–20 points and smooth away rugged responses; positive calibrations were under 1 % of the space | sourced | Lamperti et al., §3, §4, §6 |
| EF-44 | A moment-closure study has four steps (choose moments, derive their equations, close, justify); on accuracy no general results exist and justifications are rare and system-specific | sourced | Kuehn 2016, "Moment closure — a brief review", §1, §3.4, https://arxiv.org/abs/1505.02190 |
| EF-45 | On networks, SIS moment equations need pairs and triples; the mean-field closure is exact only for a complete graph as degree grows; the pair approximation gives five closed equations | sourced | Kuehn 2016, §2.3, §3.3 |
| EF-46 | Geometrically a closure is a quasi-steady state on a critical manifold; it needs normal hyperbolicity ("very strong and non-generic"), is correct only to first order in the scale ratio, and is generically local | sourced | Kuehn 2016, §3.4 |
| EF-47 | Scale separation can be measured: start bursts from the same slow state with different fast states and fit how fast the fast variables converge; repeated from several states it gives a number for the gap. It works because a simulator can be started anywhere | sourced | Ioannis Kevrekidis, "Equation-free approach to deriving effective macroscopic equations for complex interacting systems" (YouTube 4CztaV5jGx4, [0:49:40]) |
| EF-48 | In a 50,000-agent herding market, short agent bursts alone produced the coarse bifurcation diagram: a turning point near g ≈ 46 and an unstable state that explodes when kicked one way and settles the other; a feedback controller on the coarse statistics then held the unstable state | sourced | Ioannis Kevrekidis, "Equation-free approach to deriving effective macroscopic equations for complex interacting systems" (YouTube 4CztaV5jGx4, [1:16:51]) |
| EF-49 | The method pays most where the mean-field variables still close but the mean-field rate is wrong; it evaluates the equation only where needed. The speaker says it has no striking real application yet | sourced | Ioannis Kevrekidis, "Equation-free approach to deriving effective macroscopic equations for complex interacting systems" (YouTube 4CztaV5jGx4, [1:35:55]) |
| EF-50 | Tabulate offline when the coarse rate depends on few inputs (hop rates over temperature and a finite set of configurations); compute by micro bursts on the fly when a table would need about four continuous inputs (island-edge velocity) | sourced | Björn Engquist, "Heterogeneous multi-scale methods" (YouTube gfNwW7dUBs4, [0:33:58]) |
| EF-51 | A missing slow variable blocks convergence: for the Kapitza pendulum the naive method fails through resonance, and adding an averaged velocity as a restart constraint fixes it | sourced | Björn Engquist, "Heterogeneous multi-scale methods" (YouTube gfNwW7dUBs4, [0:40:13]) |
| EF-52 | Coupled methods can carry a developed structure (a hurricane), but its genesis needs a computation over a domain too large to afford: onset is the unsolved case | sourced | Björn Engquist, "Heterogeneous multi-scale methods" (YouTube gfNwW7dUBs4, [0:47:28]) |
| EF-53 | Concurrent learning: train 4–5 models and run the expensive micro model only where they disagree; of about 500 million configurations explored, under 0.05 % needed a micro label | sourced | Weinan E, "Machine learning based multi-scale modeling" (YouTube fDlG9HHopBU, [0:09:18], [0:21:41]) |
| EF-54 | A near-invariance that passed training and test errors still caused systematic drift in long runs through small discontinuities; exact invariant features fixed it | sourced | Weinan E, "Machine learning based multi-scale modeling" (YouTube fDlG9HHopBU, [0:15:56]) |
| EF-55 | A learned closure built as a known part plus a learned correction greatly reduced variance and data needs; classical truncated closures carry no accuracy guarantee | sourced | Weinan E, "Machine learning based multi-scale modeling" (YouTube fDlG9HHopBU, [0:42:03]) |
| EF-56 | Coarse-graining a high-resolution truth run shows sub-grid randomness growing with the parameterised tendency; multiplicative stochastic physics gained about 4 days of tropical rain skill, against the usual 1 day per decade | sourced | Tim Palmer, "Climate predictability, stochastic modelling and inexact computing" (YouTube RcTruY5m85I, [0:31:39], [0:33:14]) |
| EF-57 | In regime systems a forcing acts by shifting regime occupancy (50/50 to 70/30) whatever its direction; small noise lengthens time spent in each regime and large noise washes regimes out | sourced | Tim Palmer, "Climate predictability, stochastic modelling and inexact computing" (YouTube RcTruY5m85I, [0:40:29], [0:43:04]) |
| EF-58 | Independent noise on initial conditions and model gives badly underdispersed ensembles; perturbations must target unstable directions, and even then reliability diagrams show overconfidence at high probabilities. The ensemble mean is not a solution and hides extremes | sourced | Tim Palmer, "Uncertainty in Math Modelling of Weather & Climate Applied to Pandemics" (YouTube HXd3CYtB2Oc, [0:14:23], [0:12:22], [0:07:47]) |
| EF-59 | Mean-field limits hold only over finite times; on networks they need a graph limit (graphons for dense graphs, graphops for sparse ones), and heterogeneity enters as several coupled typical particles, one per node type | sourced | Christian Kuehn, "Mean-Field Limits of Network Dynamics" (YouTube Ocrdp1JBa28, [0:21:38], [0:45:35]) |
| EF-60 | Discrepancy modelling: keep the known physics and learn only the missing correction; sparse regression over a term library recovers the Lorenz system with 7 parameters | sourced | J. Nathan Kutz, "Data-driven methods for the discovery of governing equations" (YouTube auk9CpI14MM, [0:05:15], [0:07:16]) |
| EF-61 | Calibration depends on the error measure: correlation, L0, L1, L2 and L∞ each fit a different aspect of behaviour, and on real data none dominated | sourced | Bill Rand, "Agent-Based Modeling: Validation and Calibration" (YouTube wMR-CEv1MVc, [0:03:45]) |
| EF-62 | Coarse projective integration lifts a coarse state (here a pair correlation function, not a density), runs a short burst, estimates the derivative and Jacobian and takes a big step; lifting is the hard part | sourced | Yannis Kevrekidis, "No Equations, No Variables, No Parameters, No Space and No Time" (YouTube Xpo0cluOKW8, [0:07:50]) |
| EF-63 | A stochastic coarse law can be learned from many short bursts started at one coarse state: the mean of the next state gives the drift, the spread a state-dependent diffusivity; shown on a lattice SIR-type agent model, where the learned law differs from the mean-field one (Dietrich et al. 2023) | sourced | Yannis Kevrekidis, "No Equations, No Variables, No Parameters, No Space and No Time" (YouTube Xpo0cluOKW8, [0:29:31], [1:05:36]) |
| EF-64 | A model learned through an integrator template reproduces the data only with that integrator at that step; many generators fit the same data: "you learn a generator", not the law | sourced | Yannis Kevrekidis, "No Equations, No Variables, No Parameters, No Space and No Time" (YouTube Xpo0cluOKW8, [1:07:07]) |
| EF-65 | Rerunning a black box from the same state tells a deterministic coarse law (same result) from a stochastic one (a distribution) | sourced | Yannis Kevrekidis, "No Equations, No Variables, No Parameters, No Space and No Time" (YouTube Xpo0cluOKW8, [1:03:00]) |
| EF-66 | A Gaussian-process emulator interpolates its training runs and quantifies uncertainty between them; its usual kernel assumes smoothness (wrong for codes with jumps) and it is impractical with hundreds of inputs | sourced | Tony O'Hagan, "Gaussian process emulation" (YouTube l58fcysAqDE, [0:06:13], [0:18:13]) |
| EF-67 | Model discrepancy and calibration parameters cannot be told apart from model runs alone; calibrating without a discrepancy term gives meaningless parameters | sourced | Tony O'Hagan, "Gaussian process emulation" (YouTube l58fcysAqDE, [0:50:41]) |
| EF-68 | For dynamic codes, emulate one time step and iterate it; validate by checking that held-out runs fall inside the predicted distribution at the nominal rate | sourced | Tony O'Hagan, "Gaussian process emulation" (YouTube l58fcysAqDE, [0:35:42], [0:44:28]) |
| EF-69 | By the author's own account, alternating micro and latent phases barely lowered error on Kuramoto–Sivashinsky (only the speed-up held), and training coverage decides which regions the learned model can reach (qualifies EF-30) | sourced | Petros Koumoutsakos, "Learning the effective dynamics of complex systems" (YouTube WcCU_E1qrhM, [0:13:28]) |
| EF-70 | G-LED replaces the learned lifting with a generative diffusion model conditioned on the latent dynamics, with constraints as guidance, and tracks chaotic statistics far longer than LED | sourced | Petros Koumoutsakos, "Learning the effective dynamics of complex systems" (YouTube WcCU_E1qrhM, [0:20:40]) |
| EF-71 | A coarse grid with a local drift plus a small latent micro residual, trained so the coarse part explains all it can first; growing the micro part one dimension at a time from zero improved near-monotonically, and the micro part mattered most at sharp fronts and early times | sourced | Andrew Ilersich, "Learning Stochastic Multiscale Models of Spatiotemporal Systems" (YouTube KG_Y-UG4z_A, [0:14:35], [0:34:25]) |
| EF-72 | A classifier trained to tell two simulators' samples apart converges to a function of their likelihood ratio; calibrated by simulation, it gives valid tests even when the classifier is imperfect | sourced | Kyle Cranmer, "Navigating the Design Space of Simulation-Based Inference" (YouTube 2FGXAlUATs0, [0:20:21]) |
| EF-73 | When part of a dependence is known, learn only the unknown part and recombine; one-at-a-time ±1σ variations assume factorisation and are too restrictive | sourced | Kyle Cranmer, "Navigating the Design Space of Simulation-Based Inference" (YouTube 2FGXAlUATs0, [0:41:08]) |
| EF-74 | An agent model that will not calibrate over wide ranges has the wrong structure; diagnose by fitting one target at a time, check that calibrated values are unique across seeds, and treat values at the edge of their range as clues | sourced | Nathaniel Osgood, "An Introduction to ABM Calibration" (YouTube Wgztv4Zw1p0, [1:09:20], [1:24:44]) |
| EF-75 | Dropping realisations that fail to match a calibration period conditions the ensemble on history | reported | Nathaniel Osgood, "An Introduction to ABM Calibration" (YouTube Wgztv4Zw1p0, [1:03:44]); a colleague's practice |
| EF-76 | SINDy recovers known equations by sparse regression over a library of candidate terms; the hard parts are the coordinates and the library; an ensemble of random walkers yields the heat equation | sourced | Steve Brunton, "SINDy: Sparse Machine Learning Models 5 Years Later" (YouTube NxAn0oglMVw, [0:13:27], [0:20:57]) |

## What the literature says, in plain words

1. **It works when the fine model forgets fast** (EF-2, EF-10, EF-17, EF-19, EF-21). Lift a coarse state into many fine ones, let the fine model run long enough for the arbitrary details to "heal", then read the coarse result. The healing time must be much shorter than the coarse time step (about 10× in the epidemic study, 500× for rods).
2. **It fails in four known ways:**
   - the chosen coarse variables stop closing and need one more variable (EF-4, EF-15, EF-45);
   - small numbers and extinction, where one individual decides the outcome (EF-13), which is exactly the cascade Hossenfelder named;
   - bistable regions, where coarse and fine runs can settle on different branches (EF-16);
   - regimes the derivation never saw (EF-33).
3. **Keep the coarse step random** (EF-24 to EF-28). A deterministic coarse model is overconfident and over-populates the most likely outcome. Weather prediction learned this and now adds structured noise.
4. **Learn the error, not the model** (EF-8, EF-35, EF-36, EF-38). A hand-written coarse form plus a learned or tabulated residual beats learning from scratch and stays stable.
5. **Precedents reported agreement as plots, not numbers** (EF-12, EF-16, EF-43). Our A2 tests with held-out seeds, a power check and pass bands would be stricter than the published work.

## What the talks add

Read in full from 15 talks; rows EF-47 to EF-76.

- **Find the threshold directly** (EF-48). The outbreak's fizzle-or-take-off point is a coarse unstable state. It can be located by Newton's method on entity day-bursts, and the coarse layer must put it in the same place. Add this to EF-test 1 case 4.
- **Derive drift and spread, per step length** (EF-63, EF-64). Bursts from one coarse state give the mean and the spread of the next day. Compare the spread with spec 38's binomial variance. A derived day is valid only at the step it was derived for, so spec 40 jumps iterate derived days, never a rescaled rate.
- **Tabulate when inputs are few, burst when they are many** (EF-50). A table over 3 or fewer continuous inputs; beyond that, fine bursts on a fixed world schedule (rule 10).
- **A world-state rule for "run entities here"** (EF-33, EF-53, EF-66, EF-68, EF-69). It fires when the derived map is uncertain (its fits disagree) or the coarse state lies outside what the derivation saw. Coverage is checked on held-out points, and the rule depends on the world, never on the player's attention.
- **Spend fine runs where the fits disagree** (EF-53). Train 4–5 residual fits and add design points only where they disagree.
- **When closure fails, add readable variables one at a time** (EF-51, EF-71, EF-74, EF-76). Draw candidates from a small library of terms (the warned share among contacts, crowding, the last day's bites), refit one target at a time, and keep the coarse part explaining all it can first.
- **Judge probabilities, not only means** (EF-58, EF-68). Add a reliability check: events the coarse layer gives a 30 % chance must happen about 30 % of the time in held-out all-hot runs.
- **A stronger A2 test** (EF-72), our application: train a classifier to tell coarse runs from fine ones. If it cannot beat chance on held-out seeds, calibrated by simulation, the layers agree. It complements Anderson–Darling on chosen summaries.
- **Onset is the hard case** (EF-52). Engquist names genesis as unsolved, which is our cascade. EF rule 5 (run entities near thresholds) is the right answer, not a smarter closure.

## What this means for us (Proposal)

All rows are `estimate`, tied to the findings by ID; nothing here is decided.

1. **Derive each coarse step from the whole fine day** (specs 38, 27, 28). The coarse step becomes `restrict(entity burst(lift(x)))`, measured offline with keyed bursts over a design of coarse states (EF-1, EF-19). Spec 38 measures one isolated encounter; a whole-day burst also captures what its rule 6 lists as known biases (two bites on one person, crowding, warning that spreads within the day).
2. **Promotion is lifting and demotion is restriction** (27 rules 4, 5, 10; 28 rules 2–4). `restrict(lift(x)) == x` stays A1. New: an A2 check that two different liftings of the same state (random placement against "mature" states sampled from long runs) give the same coarse output once healed (EF-2, EF-15). When they differ, the coarse variables do not close: add one (for the outbreak, the warned share among a zombie's contacts) and record it on a closure-failure map (EF-4).
3. **Measure the healing time and the gap before trusting a step** (38; 40). A coarse step is valid only if ten times the healing time fits inside the coarse time scale, here the doubling time of the zombie count (EF-10, EF-21). A runner in a dense city fails this, so that region steps hourly or stays as entities.
4. **The coarse step stays random and is judged on outcome shares and spread** (38 rule 4; 27 tests). Keep keyed binomials, and add a spread ratio (coarse spread ÷ fine spread, pass 0.8–1.25) and the take-off share to every coarse layer: cohorts (28) and the economy (32), not only the outbreak (EF-25, EF-27).
5. **Small counts and thresholds run fine** (38; 27 rule 11). When z < 20 or 0.7 < r0 < 1.5, a region runs entities or an exact per-zombie branching form (EF-13, EF-20). The switch depends on the world's state, never on attention, so watching still changes nothing ([0061](../decisions/0061-the-world-runs-without-the-player.md)). This is where cascades live.
6. **Split the error budget** (38 rule 6). Step error is tested by halving the step (day against hour); estimation error by bootstrapping the burst replicas (EF-22).
7. **Validate by continuation over a held-out grid** (38 tests; 27 harness). Start from spec 38's closed-form preset and walk a speed × warning × size grid with held-out points and held-out seeds (EF-4, EF-43). Add a power check: the test must reject a deliberately wrong coarse model.
8. **Keep the hand-written forms and derive only their residual** (27; 06 rule 4). The residual is a versioned table over coarse states, hashed into the data hash; CI fails when the fine rules change and the table is stale (EF-8, EF-35). Memory breaks memoryless residuals (EF-36), so lagged quantities (the bitten who have not yet turned) stay coarse variables.
9. **Player inputs are parameters of the derived map** (06; 39). Bursts include input schedules (a broadcast on day d); tests hold some schedules out. An input outside the derived range forces entities for that region for a few days, because learned maps do not extrapolate (EF-33).
10. **Bursts are offline and keyed** (02; 27 rule 6). Bursts triggered by the player's attention would let watching change outcomes; fine phases at runtime (EF-29) are allowed only on a fixed world schedule.

## The first experiment

**EF-test 1: derive a district's outbreak-plus-warning day by lift, run and restrict, and test it against all-hot runs.** It doubles as the first test world for [two-contagions](two-contagions.md).

- **Fine model:** spec 38's entity day plus a per-person `warned` flag. A person becomes warned on surviving or witnessing an encounter; each warned person tells k contacts a day.
- **Coarse state:** (unwarned living, warned living, exposed, zombies). Restriction counts entities; lifting assigns states by key (seed, district, day, "lift").
- **Derivation:** 300 design states plus 40 with few zombies (1, 2, 4, 8), 64 keyed replicas each, two-day bursts (day 1 heals, day 2 is measured). Fit spec 38's tau-leap form plus a residual table. Derivation seeds use a key field number validation never uses.
- **Lift check (A2):** the same points with a "mature" lifting; day-2 shares within 3 SE at ≥ 95 % of points; failures go on the closure-failure map.
- **Validation:** 400 held-out all-hot runs against 2,000 coarse runs per case (400 detects a 0.10 take-off gap with 80 % power). Six cases:
  1. spec 38's middling preset (N = 400, three zombies, 40 days);
  2. a sure fizzle;
  3. a sure take-off;
  4. **cascade:** one zombie, unwarned r0 ≈ 1.2, warning spreading from sightings (take-off 20–60 %);
  5. **input:** a broadcast warning half the living on a held-out day;
  6. **scale:** N = 4,000.
- **Pass:** Anderson–Darling not rejected after Holm correction (5 % family-wise) on infected-ever, peak zombies, peak day and warned share; means within 4 SE; take-off shares within 3 SE; spread ratio 0.8–1.25. **Power check:** spec 38's frozen-warning rates and a deterministic mean-field model must both be rejected (p < 0.01) in cases 4 and 5. Deriving twice gives a bit-identical table (A1); hourly and daily steps agree.
- **Cost:** about 44,000 entity-days to derive (twice with the lift check) and 96,000 to validate. Spec 38's entity day is one step per entity per day, so this should be small, but it is not measured yet.

## What we don't take

- **Runtime latent models** (EF-29 to EF-33) as the coarse layer: fast, but not keyed, not exact across machines and unable to say when they are wrong. A tabulated residual on a hand-written form is deterministic and inspectable.
- **Fine bursts triggered by where the player looks.** They would make watching change outcomes.
- **Trusting plots.** The precedents' agreement was visual (EF-12, EF-16); ours is a test with a power check.

## Open questions

Carried to [focus and compression](../design/focus-and-compression.md) and [layer jumps](../design/layer-jumps.md):

- Should derived residual tables become the standard for every coarse layer, or stay for the outbreak and pop cohorts until EF-test 1 passes?
- Where do derived tables live (`data/derived/`, versioned with the fine rules' hash), and does CI re-derive them on every rule change or only on release?

## Sources

- Kevrekidis, I. G., Gear, C. W., Hyman, J. M., Kevrekidis, P. G., Runborg, O., Theodoropoulos, C. (2003). Equation-free, coarse-grained multiscale computation. *Commun. Math. Sci.* 1(4):715–762. arXiv physics/0209043
- Cisternas, J., Gear, C. W., Levin, S., Kevrekidis, I. G. (2004). Equation-free modelling of evolving diseases. *Proc. R. Soc. A* 460:2761. arXiv nlin/0310011
- Siettos, C. (2010/2011). Equation-free multiscale computational analysis of individual-based epidemic dynamics on networks. arXiv 1012.4194
- Liu, P., Siettos, C., Gear, C. W., Kevrekidis, I. G. (2015). *Math. Model. Nat. Phenom.* 10(3):71–90
- E, W., Engquist, B. (2003). The heterogeneous multiscale methods. *Commun. Math. Sci.* 1(1):87–132. arXiv physics/0205048
- Palmer, T. N. (2005). Stochastic-dynamic parametrisation in weather and climate prediction models: an introduction. ECMWF workshop; Palmer, T. N. (2001), *QJRMS* 127
- Vlachas, P. R., Arampatzis, G., Uhler, C., Koumoutsakos, P. (2022). Multiscale simulations of complex systems by learning their effective dynamics. *Nat. Mach. Intell.* 4:359. arXiv 2006.13431
- Levine, M. E., Stuart, A. M. (2022). A framework for machine learning of model error in dynamical systems. *Commun. AMS* 2:283–344. arXiv 2107.06658
- Lamperti, F., Roventini, A., Sani, A. (2018). Agent-based model calibration using machine learning surrogates. *JEDC* 90:366–389. arXiv 1703.10639
- Kuehn, C. (2016). Moment closure — a brief review. arXiv 1505.02190
