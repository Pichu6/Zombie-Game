# Hordes, city sound, storms and sky, and testing a deterministic world

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [spec 18](../tier0/specs/18-noise-field.md), [spec 19](../tier0/specs/19-light-field.md), [spec 36](../tier0/specs/36-fields-and-weather.md), [spec 10](../tier0/specs/10-fortification.md), [spec 23](../tier0/specs/23-walker-brain.md), [spec 27](../tier0/specs/27-detail-levels.md), [spec 50](../tier0/specs/50-movement.md), [spec 40](../tier0/specs/40-time.md), [spec 06](../tier0/specs/06-save-replay.md), [world simulation](../design/world-simulation.md) §9, [borrowed-from-other-fields](borrowed-from-other-fields.md), [perception-and-run-ahead](perception-and-run-ahead.md). **Evidence:** four web-research passes on 2026-10-09. Read first-hand, among others:
- **Crowds:** Weidmann's curve via Seyfried et al. and Kretz; Helbing et al. 2000 and Helbing and Molnár; Burstedde et al. 2001; Treuille et al. 2006; Narain et al. 2009; Sewall et al. 2011; Couzin et al. 2002 and 2005; Attanasi et al.; Helbing, Johansson and Al-Abideen; Fruin 1993; the Green Guide 5th edition.
- **Sound:** the Madrid, Barcelona and South Paris lockdown studies; Saurenman et al.; Raghuvanshi and Snyder 2014; the Steam Audio docs; Leonard 2003.
- **Sky:** Bloemendaal et al. 2020 (STORM); Emanuel et al. 2006; Sharkey et al.; Cowpertwait 2004; the BETWIXT NSRP note; the NOAA solar calculator; the USNO Circular 171 code; ECMWF Tech Memo 911.
- **Testing:** the FoundationDB SIGMOD paper and source; TigerBeetle's VOPR and TigerStyle docs; the Antithesis, Resonate and sled pages; Groce et al. 2012; RapidCheck; Csmith; Factorio FFF-47; Bettner and Terrano; GGPO; Pollard (GDC 2024); Box2D; the MSVC `/fp` docs; Dawson; CORE-MATH; libFuzzer.

Not read: Weidmann 1993 and the SFPE Handbook themselves, Hughes 2002 and Xiong et al. in full, ISO 9613-2 itself (via vendor appendices), Kasten and Czeplak's equation (garbled PDF), Wilson's Strange Loop talk, Spring RTS sync docs. **IDs:** HST.


Third round. The second note's last section left four gaps:
- how hordes move as fields;
- how sound works at city scale, and whether "voices carry ten times farther when the grid dies" holds;
- how storms, sun and moon become pure functions of place and time on our own planet;
- how to test every promise of a deterministic multi-level world.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| HST-1 | Weidmann speed–density: v = 1.34·[1 − exp(−1.913·(1/ρ − 1/5.4))] m/s; free speed 1.34 m/s, jam density 5.4 people per m² (25 earlier studies) | sourced | Seyfried et al. 2005, arXiv physics/0506170; Kretz arXiv 1512.01426 |
| HST-2 | From HST-1: peak flow about 1.22 people per m per s at 1.75 per m² (0.70 m/s); 0.33 m/s at 3 per m², 0.04 m/s at 5 per m² | derived | HST-1 |
| HST-3 | Below 0.7 people per m² speed stays near free; 0.7–2.3 it falls almost linearly; above, steeply | sourced | Seyfried et al. 2005 |
| HST-4 | SFPE hydraulic model: S = 1.40·(1 − 0.266·D) m/s in corridors; flow peaks at 1.316 people per s per m near 1.9 per m²; zero speed near 3.8 per m² | reported | Pathfinder door verification; SFPE Handbook 2016 |
| HST-5 | Door effective width = clear width − 0.15 m per side: a 1 m door passes about 0.92 people per s | sourced | Pathfinder verification docs |
| HST-6 | Measured bottleneck flow 1.61–2.31 people per m per s for 0.8–1.2 m widths, rising linearly with width | sourced | Seyfried et al., arXiv physics/0702004 |
| HST-7 | Social force calibrated to 0.73 people per s through an effective 1 m door; desired speed above 1.5 m/s causes arching and clogging ("faster is slower") | sourced | Helbing, Farkas, Vicsek, arXiv cond-mat/0009448 |
| HST-8 | Green Guide rates of passage: 1.1 people per m per s on steps, 1.37 on the level | sourced | Guide to Safety at Sports Grounds, 5th ed. 2008 |
| HST-9 | Fruin levels of service from A (≥ 3.2 m² per person) to F (< 0.5 m², variable flow); capacity near 75 people per minute per metre | reported | FHWA 98-107 |
| HST-10 | Hughes continuum crowd: density moves down a crowd-weighted time-to-goal potential with speed f(ρ) | reported | Hughes 2002, TR-B 36 |
| HST-11 | Continuum Crowds: one potential per group, cost (αf + β + γg) ÷ f, solved by fast marching; the potential is the expensive step, so keep groups few | sourced | Treuille, Cooper, Popović, SIGGRAPH 2006 |
| HST-12 | Continuum Crowds ran a 10,000-person chase at 5 fps on a 60 × 60 grid; agents as moving obstacles are "the most natural use" in games | sourced | same |
| HST-13 | Continuum Crowds ignores packed contact forces, inertia and individual goals | sourced | same |
| HST-14 | Agents plus a density grid capped at ρ_max: 100,000 agents at 447 ms per frame; grid cost independent of agent count; RVO failed above 70,000 | sourced | Narain et al., SIGGRAPH Asia 2009 |
| HST-15 | Hybrid traffic: a "flux capacitor" per border accumulates flow and emits one vehicle per whole unit; agents average back into cell densities | sourced | Sewall, Wilkie, Lin, TOG 30(6) 2011 |
| HST-16 | Pure continuum is 10–20× faster than agents, the hybrid 9–17×; visible areas are always agents | sourced | same |
| HST-17 | Hybrid crowd: micro and macro partitions joined by transition cells at chosen border sections (doors, cross-sections) | reported | Xiong et al. (TUM) |
| HST-18 | Pedestrian cell transmission model: flow between cells limited by sender outflow and receiver inflow capacity (min of demand and supply), using Weidmann | sourced | Hänseler et al., HEART 2013 (abstract); Daganzo CTM |
| HST-19 | Social force parameters: desired speed N(1.34, 0.26) m/s, τ = 0.5 s, 200° field of view | sourced | Helbing and Molnár, arXiv cond-mat/9805244 |
| HST-20 | Panic social force: 80 kg, body diameter 0.5–0.7 m (bodies must vary or symmetric gridlock occurs); injury when radial force per circumference exceeds 1,600 N/m | sourced | Helbing et al. 2000 |
| HST-21 | Floor-field cellular automaton: 0.4 m cells, at most one cell per step (~0.3 s), parallel update with conflicts resolved by chance; a decaying, diffusing dynamic field turns long-range interaction into local rules | sourced | Burstedde et al., arXiv cond-mat/0102397 |
| HST-22 | ORCA/RVO2 solves a small linear program per agent: thousands of agents in a few ms | reported | gamma.cs.unc.edu/ORCA; arXiv 1710.04296 |
| HST-23 | Couzin zones (repulsion, orientation, attraction) give swarm, torus or parallel groups depending on zone widths; the state depends on history | sourced | Couzin et al., J. Theor. Biol. 218 (2002) |
| HST-24 | The larger a group, the smaller the informed share needed to steer it; informed individuals need not be recognisable | sourced | Couzin et al., Nature 433 (2005) |
| HST-25 | From 10 to 200 individuals the informed share falls from about 50 % to under 5 %; about 5 % steered human crowds of 200 without talking | reported | Princeton 2005; Dyer et al. 2009 |
| HST-26 | Starling turns spread linearly from a few birds at 9–21 m/s (preprint), faster than flight speed, with almost no loss | sourced | Attanasi et al., arXiv 1303.7097 |
| HST-27 | In marathon corrals, speed changes travel as waves at about 1 m/s for hundreds of metres; direction changes die within metres | reported | Bain and Bartolo, Science 363 (2019) |
| HST-28 | Hajj: local densities above 10 per m²; above 6 per m² flow drops at least 3×; "crowd turbulence" when ρ·Var(v) > 0.02 s⁻² | sourced | Helbing, Johansson, Al-Abideen, arXiv physics/0701203 |
| HST-29 | Fruin: bent railings imply more than 4,500 N total; five people pushing gave 3,430 N; at about 7 per m² the crowd moves as a fluid and deaths are mostly compressive asphyxia | sourced | Fruin 1993 |
| HST-30 | Helbing restates crowd pressure as 4,450 N/m, enough to bend steel barriers and topple brick walls | sourced | Helbing et al. 2000 |
| HST-31 | Green Guide crush barriers are rated up to 5.0 kN/m (above that the person behind is injured); other barriers 1.0–3.0 kN/m; standing density capped at 4.7 per m² | sourced | Green Guide 5th ed., Tables 11.1–11.3 |
| HST-32 | Measured static crowd load on a barrier: 1.43 kN/m for 1 row, 2.12 for 2, 2.66 for 3; the Hillsborough barrier failed near 8 kN/m | reported | structuremag.org |
| HST-33 | 7 Days to Die "heat" per 16 × 16 chunk builds from activity, spills to neighbours, decays and spawns a screamer; Days Gone hordes keep their remaining count when the player escapes | reported | fan wikis |
| HST-34 | ISO 9613-2: total loss = divergence + air absorption + ground + barrier + other; divergence equals our 20·log10 d | reported | EMD windPRO appendix |
| HST-35 | Air absorption at 10 °C, 70 % humidity: 1.9 dB/km at 500 Hz, 3.7 at 1 kHz, 9.7 at 2 kHz, 32.8 at 4 kHz | reported | ISO 9613-1 via same |
| HST-36 | Ground loss up to 4.8 dB at long range over soft ground, about 0 over paving or water | reported | same |
| HST-37 | Barrier loss 10·lg(3 + 20·δ ÷ λ), capped at 20 dB for one edge, 25 for two | reported | Arup Strutt help |
| HST-38 | ISO 9613-2 models downwind or a moderate night inversion; inversions over large water are excluded and can be louder | reported | ISO 9613-2:2024 scope |
| HST-39 | Phoenix: night inversions raised freeway noise 5–8 dB beyond 400 m; effects start at 200–300 m; sunrise loudest | sourced | Saurenman et al., FHWA-AZ-05-555 (abstract) |
| HST-40 | Within 180 m, weather changes levels by −4 to +3 dB; traffic noise can change more than 10 dB within an hour from weather | reported | NCHRP Report 882 |
| HST-41 | Over open ground, upwind loss exceeded downwind by up to 25–30 dB | reported | Wiener and Keast, JASA 31 (1959) |
| HST-42 | Noise mapping (CNOSSOS-EU) blends favourable and neutral propagation by time share: about ⅓ of daytime and ⅔ of night-time favourable | reported | CadnaA docs |
| HST-43 | Night carrying is refraction as well as less masking: Humboldt heard an Orinoco waterfall better at night despite louder insects | sourced | Gabrielson, Acoustics Today 2(2) |
| HST-44 | Madrid lockdown: 4–6 dBA lower day, evening and night; up to 11.2 dB at one station; a quiet park only from 46 to 44 at night | sourced | Asensio et al., JASA 148 (2020) |
| HST-45 | Barcelona lockdown: average level −5.9 dB, background (L90) −10.8 dB, largest drops 00:00–05:00 | sourced | Bonet-Solà et al., IJERPH 18 (2021) |
| HST-46 | London lockdown −5.4 dB mean (1.2–10.7); Paris road network −7.6 dB | reported | Aletta et al., Noise Mapping 7 |
| HST-47 | South Paris main roads went from 67–72 to 57–63 dBA in lockdown | sourced | Abdmouleh and Dahech, Sci Rep 14 (2024) |
| HST-48 | UK lockdown night background still about 44 dB urban and 38.5 suburban | reported | Dance et al., The Quiet Project |
| HST-49 | Seismic noise fell up to 50 % for months in 2020, strongest in cities | reported | Lecocq et al., Science 369 (2020) |
| HST-50 | Natural quiet: Great Sand Dunes about 17 dBA day, 15 night; Haleakalā about 10; a rural Californian night 25–30 dBA | reported | NPS NRTR-2008-001; Portola Valley study |
| HST-51 | 113 million Europeans live with road noise at 55 dB Lden or more | reported | EEA 2020 |
| HST-52 | At the detection threshold a tone's power about equals the noise power in its critical band (in-band signal-to-noise about 0 dB) | reported | ASA critical-ratio glossary |
| HST-53 | Reliable alarm audibility: 15 dB above noise (A-weighted) or 10–13 dB above the masked threshold in a band; 10 dB above it is "easy to hear" | reported | ISO 7731:2003 summaries |
| HST-54 | Detection improves about 6.7 dB per tenfold duration up to about 200 ms; peak level overstates how audible an impulse is | reported | Z Audiol 6 (2024) |
| HST-55 | Direction: about 1° detectable change in front, 3° to the side; absolute error 4–10° in front, 3–10× larger to the side | reported | Front. Psychol. 2021 |
| HST-56 | Perceived distance ≈ 1.3·d^0.39: far sources are underestimated | reported | Zahorik, Brungart, Bronkhorst 2005 |
| HST-57 | Speech at 1 m: normal 58–60 dB; shout 89 dBA (men), 82 (women and children); talkers rise about 0.5 dB per dB of background | reported | Pearsons et al., EPA 1977 |
| HST-58 | Generators at 7 m: open-frame 68–70 dBA (about 85 at 1 m), inverter 52–59 | reported | manufacturer FAQs |
| HST-59 | Gunshots 152–165 dB peak at 1 m for pistols, some firearms above 177 | sourced | Barnard et al., INTER-NOISE 2012 (abstract) |
| HST-60 | Vehicle alarms: at least 100 dB(A) by regulation, aftermarket sirens to 123 dB at 0.5 m | reported | UNECE R116; Swiss VTS |
| HST-61 | Microsoft Triton bakes wave simulation into probes every 2–4 m: 56 TB raw became 41–54 MB; bakes 4–20 h; about 100 µs per source at runtime | sourced | Raghuvanshi and Snyder, TOG 33(4) 2014 |
| HST-62 | Doors on baked acoustics: find the portals on the precomputed path and apply each one's open or closed loss at runtime; cost scales with portals, not area | sourced | Raghuvanshi, arXiv 2107.11548 (abstract) |
| HST-63 | Reverberation graphs precompute per-room and per-portal transfer and combine them along graph routes at runtime | reported | Stavrakis, Tsingos, Calamia 2008 |
| HST-64 | Steam Audio pathing bakes probe-to-probe visibility (e.g. 2 m spacing, 50 m visibility, 100 m paths) and finds the shortest unblocked probe path | sourced | Steam Audio C API guide |
| HST-65 | Wwise: a diffraction path per portal (angle → attenuation and low-pass), a separate transmission path through walls when no portal path exists | reported | Audiokinetic blog 2023 |
| HST-66 | Hitman 2016: audio budget 1 ms per frame; rooms and portals; a listener in front of an opening hears the sound from the opening. Rainbow Six Siege: shortest paths over placed nodes, updated as walls break | reported | Boev, GDC Europe 2015; Davari and Verbrugge, AIIDE 2021 |
| HST-67 | Thief: AI hears from the real arrival direction with "attenuated awareness"; discrete states with delayed rise and gradual decay | sourced | Leonard, Game Developer 2003 |
| HST-68 | Streets: gaps between buildings cost 3–5 dB; facade reflections add up to about 8 dB | reported | Kang et al. 2001 (as cited) |
| HST-69 | STORM: tropical cyclone genesis as Poisson per basin per year (EP 14.5, NA 10.8, NI 2.0, SI 12.3, SP 9.3, WP 22.5); 38 years stretched to 10,000 at 3-hourly steps | sourced | Bloemendaal et al. 2020, Sci Data 7 |
| HST-70 | STORM tracks: Δlon and Δlat as autoregressive terms plus Gaussian residuals per basin and 5° band; genesis from a monthly 1° density | sourced | same |
| HST-71 | STORM land decay: V = Vb + R(V0 − Vb)·e^(−αt), R = 0.9, Vb = 26.7 kt, α = 0.095 h⁻¹ | sourced | same |
| HST-72 | STORM radius of maximum wind sampled by pressure class, global mean 49.7 km | sourced | same |
| HST-73 | Emanuel's tracks follow 0.8 × low-level + 0.2 × upper wind plus a 2.5 m/s drift; steering winds are random-phase Fourier series matched to monthly statistics | sourced | Emanuel et al. 2006, BAMS 87 |
| HST-74 | Emanuel generates about 10⁴ synthetic tracks per site; storms strengthen about 6 m/s per day | sourced | same |
| HST-75 | Vickery 2000's track model underlies US building-code design winds | reported | Vickery et al., J. Struct. Eng. 126 |
| HST-76 | Tropical cyclone forward speed: global mean about 4.9 m/s; radius about 40 km at 50 hPa deficit and 25° | reported | Vickery and Wadhera 2008; Kossin 2018 |
| HST-77 | North Atlantic winter cyclones: about 82 per season, mean life just under 4 days, modelled as a Markov chain | sourced | Sharkey, Tawn, Brown, arXiv 1905.08840 |
| HST-78 | European windstorm catalogues: the 200 strongest storms, return periods to 500+ years; a 10,000-year event set exists | reported | Della-Marta et al. 2009 |
| HST-79 | Extratropical cyclone radius 300–400 km over land, over 900 km over oceans, growing 50–150 % as they develop | reported | Rudeva and Gulev 2007 (abstract) |
| HST-80 | Storm-track variability sits at 2.5–6 days; mesoscale bands 2–2,000 km | reported | Blackmon 1976; Orlanski 1975 |
| HST-81 | Rainbands: warm-frontal about 50 km wide, narrow cold-frontal 3–5 km | reported | NOAA repository; EUMETSAT SatManu |
| HST-82 | US mesoscale convective systems move 13–16 m/s and last 10–12 h; squall lines about 20 m/s | reported | Princeton MCS climatology; COMET |
| HST-83 | A mesoscale convective complex is about 600 km across and lasts at least 6 h | reported | Maddox 1980 via COMET |
| HST-84 | A thunderstorm cell lives 20–45 min; radar mean cell about 5 km radius, 13 m/s, 18 min | reported | HESS 21 (2017); NWS spotter guide |
| HST-85 | Easterly waves: 3–5 days, 2,000–4,000 km, about 8 m/s westward; the MJO 30–60 days, 5 m/s eastward | reported | Burpee 1972; NOAA CPC |
| HST-86 | Neyman–Scott rectangular pulses: storms arrive as Poisson, each with a geometric number of rain cells (discs with exponential radius and lifetime) | sourced | BETWIXT TBN2 |
| HST-87 | NSRP fitted to Auckland: one cluster per about 10 days in January, 15–26 cells per storm in July, cells live 0.4–1 h; intensity set so each monthly mean is fitted exactly | sourced | Cowpertwait 2004, HESS 8 |
| HST-88 | Multi-site weather generators: per-site generators driven by random numbers correlated in space; ignoring day-to-day dependence underestimates 3-day extremes | reported | Wilks 1998, J. Hydrol. 210 |
| HST-89 | STREAP and RainFARM synthesise whole fields (RainFARM reweights so aggregates equal the coarse field) | reported | Paschalis et al. 2013; Rebora et al. 2006 |
| HST-90 | Grid and FFT generators must synthesise the whole domain; point-process cell models can be evaluated at one point if events are hashed per space-time bucket | estimate | from HST-86, HST-89 |
| HST-91 | NREL SPA is accurate to ±0.0003°; NOAA (Meeus) gives sunrise within 1 minute below 72° latitude; a plain sinusoidal declination is off by about 0.5° | reported | NOAA solar calculator; NREL SPA |
| HST-92 | Twilight: civil −6°, nautical −12°, astronomical −18°; about 400–600 lux at sunset, 2–3.5 at −6°, 0.008 at −12° | reported | AMS Glossary |
| HST-93 | USNO closed form for sky illuminance by solar altitude, continuous through twilight, with a cloud divisor of 1, 3 or 10 and a 0.0005 lux floor: about 124 klux at 90°, 51 at 30°, 980 lux at 0°, 3 at −6° | sourced | USNO Circular 171 via skylight code |
| HST-94 | The same model gives moonlight by phase and altitude: full moon at 60° about 0.33 lux, quarter about 0.022 | sourced | same |
| HST-95 | Measured full moon up to about 0.2 lux; overcast moonless nights about 0.5 millilux; clouds darken rural skies but brighten light-polluted ones up to 10× | sourced | PMC8526603; Jechow et al. 2019 |
| HST-96 | A quarter moon is about 1/10–1/12 of full (opposition surge) | reported | Krisciunas and Schaefer 1991 |
| HST-97 | Mean synodic month 29.530588861 days | reported | Reingold and Dershowitz |
| HST-98 | Kasten–Czeplak: irradiance = clear × (1 − 0.75·(N ÷ 8)^3.4); full overcast leaves 25 % | reported | Kasten and Czeplak 1980 |
| HST-99 | ECMWF 500 hPa forecasts reach 60 % anomaly correlation at about day 8–10; skill has improved about one day per decade | reported | ecmwf.int/node/25701 |
| HST-100 | ECMWF rain forecasts are skilful to about 3–4 days in detail and about 7 in probability; ensemble cyclone position error about 250 km at day 5 | sourced | ECMWF Tech Memo 911 |
| HST-101 | NHC Atlantic track errors grow about 46 km per day (31 n mi at 24 h to 126 at 120 h) | reported | NHC verification 2022 |
| HST-102 | Weather predictability is limited to about two weeks; forecasts converge to climatology by day 15 | reported | Zhang et al. 2019 (PSU); Froude et al. 2013 |
| HST-103 | FoundationDB runs the real database inside a deterministic simulation with network, disk, time and RNG abstracted | sourced | Zhou et al., SIGMOD 2021 |
| HST-104 | FoundationDB swarm-tests every run (random cluster, workload, faults, knobs, a random subset of BUGGIFY points) and counts how often rare conditions are reached (`TEST(cond)`) | sourced | same |
| HST-105 | FoundationDB jumps the clock to the next event; tens of thousands of simulations nightly; it cannot catch performance bugs | sourced | same; FoundationDB testing docs |
| HST-106 | BUGGIFY: each site is active with p = 0.25 per run and fires with p = 0.25 when active; only in simulation | sourced | FoundationDB `Buggify.h` |
| HST-107 | BUGGIFY also randomises tuning knobs (748); after 300 simulated s the chaos turns off to check recovery | sourced | Miller, transactional.blog |
| HST-108 | Wilson claims FoundationDB had only one or two customer-reported bugs, and about 50× team productivity | sourced | antithesis.com blog |
| HST-109 | TigerBeetle VOPR runs production code with clock, network and disk stubbed; seed plus commit reproduces a run; replicas must be byte-identical | sourced | TigerBeetle VOPR docs |
| HST-110 | VOPR draws its fault distributions at random per run and takes the seed from outside so it survives crashes | sourced | TigerBeetle blog 2025-11-28 |
| HST-111 | TigerBeetle runs VOPR on about 1,000 cores; 3.3 s of VOPR ≈ 39 min of real time | reported | TigerBeetle newsletter 2024-12 |
| HST-112 | TigerStyle: at least two assertions per function on average, paired on two code paths, asserting both what must and what must not happen | sourced | TigerBeetle TIGER_STYLE.md |
| HST-113 | Antithesis runs a deterministic hypervisor per core; exploration is a tree of input branch points | sourced | antithesis.com blog |
| HST-114 | Resonate runs every seed twice and diffs the logs: "same seed, different output is a failing run"; full-state snapshots every 200 steps against an in-memory oracle | sourced | Resonate testing docs |
| HST-115 | sled: logic as state machines driven by an event queue, invariants checked after every message, failures shrunk by dropping inputs | sourced | sled.rs/simulation |
| HST-116 | Swarm testing (each feature on with p = 0.5) found 42 % more distinct compiler crashes than the hand-tuned default | sourced | Groce et al., ISSTA 2012 |
| HST-117 | Stateful property tests compare the system against a simple model, check invariants after every step and shrink failures to short programs | reported | Hypothesis docs; MacIver and Donaldson, ECOOP 2020 |
| HST-118 | RapidCheck (C++) has shrinking and stateful Model/Command tests with seed and reproduce strings; no official doctest integration | sourced | github.com/emil-e/rapidcheck |
| HST-119 | Metamorphic testing checks relations between outputs of related inputs when there is no oracle; applied to agent-based and discrete-event models | reported | Chen et al., ACM CSUR 51 (2018); Olsen and Raunak 2019 |
| HST-120 | Covasim: "higher transmissibility gives more infections" as a metamorphic relation; a vaccine test exposed a bug | reported | Clark et al., arXiv 2209.00357 |
| HST-121 | A relabeling relation held but caught none of the seeded faults; conservation and mirror checks caught others | reported | arXiv 2606.17529 |
| HST-122 | Two-sample tests: Anderson–Darling has more power than Kolmogorov–Smirnov; discrete data need permutation p-values | reported | Scholz and Stephens 1987; SciPy docs |
| HST-123 | Holm's step-down procedure controls family-wise error with more power than Bonferroni; Benjamini–Hochberg controls false discoveries | reported | Holm 1979; Benjamini and Hochberg 1995 |
| HST-124 | Sequential probability ratio tests stop early when evidence is enough; statistical model checking uses them | reported | Wald; Younes and Simmons 2002 |
| HST-125 | Csmith compared compilers on generated programs and found 325+ bugs in 3 years; every compiler crashed and miscompiled | sourced | Yang et al., PLDI 2011 |
| HST-126 | Factorio's debug mode checksums the whole map every tick and saves every tick; replay finds the exact first desync tick | sourced | Factorio FFF-47 |
| HST-127 | Age of Empires checksummed world, objects, pathfinding and targeting; per-system sync tags were what they most wished they had built early | sourced | Bettner and Terrano 2001 |
| HST-128 | GGPO SyncTest runs every frame twice and aborts on a checksum mismatch | sourced | GGPO `ggponet.h` |
| HST-129 | Realms of Ruin: fixed-point simulation with `float` rejected at compile time, an RNG per system, padding bytes zeroed (the most common desync), an XML state diff tool | sourced | Pollard, GDC 2024 |
| HST-130 | Box2D v3 requires one transform hash across MSVC, Clang and GCC on x64 and ARM in CI, with FMA contraction off and its own sin/cos/atan2 | sourced | box2d.org determinism post, 2024 |
| HST-131 | MSVC `/fp:precise` stops contracting to FMA from VS 2022 17.0; `/fp:strict` never contracts | sourced | Microsoft `/fp` docs |
| HST-132 | GCC contracts to FMA by default outside ISO C modes; Clang reportedly contracts within one expression | reported | GCC docs; LLVM issue 54927 |
| HST-133 | IEEE 754 does not pin down transcendental functions; compile-time and run-time `sin` can differ | sourced | Dawson, randomascii 2013 |
| HST-134 | Factorio replaced every trig, exp and log function with its own for cross-platform determinism | sourced | Factorio Lua API docs |
| HST-135 | CORE-MATH provides correctly rounded math functions under MIT; glibc adopted some | sourced | core-math.gitlabpages.inria.fr |
| HST-136 | A libFuzzer target must be deterministic and fast; TigerBeetle biases generators toward valid inputs | sourced | LLVM libFuzzer docs; TigerBeetle blog |
| HST-137 | Sample size: a two-sample KS test needs about 10 ÷ Δ² runs per arm for 80 % power (about 1,000 for Δ = 0.1); with Holm over 20 metrics about 15 ÷ Δ² | estimate | from HST-122 |
| HST-138 | "Voices carry ten times farther" needs a 20 dB drop in background. Spec 18's 55/45 → 40/30 is 15 dB (5.6×). Lockdowns cut city background about 10 dB with people still home (HST-45); a dead grid with no traffic should reach rural levels, about 35 day and 25–28 night, a 20 and 17–20 dB drop: about 7–10× | derived | HST-45, HST-50; spec 18 |
| HST-139 | Spec 18's single +3 dB hearing threshold could become two: notice at signal-to-noise ≥ 0 dB (−5 for tonal or repeating sounds such as generators) and place the sound at +10 dB; spec 18 already lets the margin set certainty | estimate | HST-52, HST-53 |
| HST-140 | Spec 18's table agrees with the sources for shouts (80–90) and talk (60); its generator (89 dB at 1 m) is about 4 dB above a 68–70 dBA-at-7 m open frame (about 85) | derived | HST-57, HST-58; spec 18 |
| HST-141 | Two-level sound: inside each building the existing least-loss search, run once at generation from each opening, stored as opening-to-opening and room-to-opening losses with the doors on each path; between buildings a street graph. Walkers already cache path loss per source cell (`walkers.cpp`) | estimate | HST-62..HST-66 |
| HST-142 | A night rule for sound: clear calm nights +5–8 dB beyond about 300 m; sunny days up to −6 dB beyond 500 m; upwind up to −20 dB beyond 300 m. Wind and cloud come from the weather function | estimate | HST-39..HST-42 |
| HST-143 | Far hordes as integer counts per chunk moved like a cell transmission model on the chunk graph: flow = min(sender demand, receiver space, edge width × capacity), speed from Weidmann. Exact jumps only in free flow or behind one bottleneck; otherwise warm steps | estimate | HST-1, HST-15, HST-18 |
| HST-144 | Hot walkers could move as a floor-field cellular automaton (integer cells, hashed priority for conflicts, a dynamic field that doubles as a scent and sound trail); compare with spec 50's 8-direction cell paths before choosing | estimate | HST-21; spec 50 |
| HST-145 | Barricade load ≈ 1.4 kN/m for the first row plus about 0.6 per extra row, saturating near 3: improvised barricades fail at 1.5–3 kN/m, rated barriers 5, catastrophic failure about 8 | estimate | HST-31, HST-32 |
| HST-146 | A sound turns a horde as a wave: chunks inside the audible radius switch goal at once, then the switch spreads outward faster than walkers walk; about 5 % informed walkers steer a large group | estimate | HST-24..HST-26 |
| HST-147 | Storm records as pure functions: {class, start, position, track terms, size and strength envelopes} hashed per (seed, genesis tile, day, index); rain at a point = sum of footprints of the active storms. Storms give weights, not amounts: rain(c, d) = annual(c) × w(c, d) ÷ Σ_year w, which keeps spec 36's exact year sums | estimate | HST-69..HST-90 |
| HST-148 | Our planet's sun: a chosen axial tilt and year length give declination, hour angle and altitude in about 10 trig operations, exact by construction; light from HST-93/HST-94 with a cloud factor (HST-98); moon phase from a chosen synodic month | estimate | HST-91..HST-98 |
| HST-149 | In-world forecast = the same storm records perturbed by keys of (seed, storm, issue time): position error about 45 km per day of lead, blending to climatology by 14 days; storms not yet born appear only as climatology; an issued forecast never changes | estimate | HST-99..HST-102 |
| HST-150 | Our floating-point policy already matches shipping practice: `/fp:precise`, `-ffp-contract=off`, `-fno-fast-math` (root `CMakeLists.txt`) and our own transcendentals (`zs/detmath.h`, spec 01 rule 5) | derived | HST-129..HST-134 |
| HST-151 | CJ-18's protocol (about 2,000 cold jumps against 50 hot runs) has unequal arms; for the same total, equal arms give more power, so the A2 protocol should balance them where hot runs are affordable | estimate | HST-137; CJ-18 |

## 1. Answer in one page

**Hordes.** Crowd science gives us the numbers and the method:
- **Far away:** a horde is an integer count per chunk moved like traffic in cells, with speed falling with density (Weidmann) and flow capped by door and street width (HST-143).
- **Across the hot border:** an accumulator per edge emits one body per whole unit (HST-15).
- **Up close:** walkers can move as a floor-field cellular automaton, which is integer and replays exactly (HST-144).
- **A sound turns a horde** as a wave faster than walking (HST-146).
- **Barricades** fail between 1.5 and 3 kN/m of crowd load (HST-145).

**City sound.** Our model is right in kind, but three numbers and the cost need work:
- The "ten times farther" claim needs a 20 dB drop. Ours is 15. Lockdown data plus rural levels suggest 35 day and 25–28 night after the grid dies (HST-138).
- Hearing should have two thresholds: notice at 0 dB, place at +10 dB (HST-139).
- Night carries sound 5–8 dB better beyond about 300 m (HST-142).
- The cost at city scale is solved by computing losses between openings once per building and searching a street graph between buildings (HST-141).

**Storms and sky** become pure functions with no saved state:
- **Storms** are records hashed per tile and day, and rain at a point is the sum of their footprints. Storms give weights, so spec 36's exact year totals still hold (HST-147).
- **The sun** comes from our own planet's tilt and year (HST-148).
- **Light** comes from one closed formula, continuous through twilight, with moon and clouds (HST-93, HST-94, HST-98).
- **Forecasts** are the true future plus keyed error growing about 45 km per day, fading to climatology by two weeks (HST-149).

**Testing.** Our floating-point policy already matches Box2D, Factorio and Realms of Ruin (HST-150). What we lack is the harness that found FoundationDB's and TigerBeetle's bugs:
- nightly seeded swarm runs with BUGGIFY-style injections (HST-104..HST-110);
- every seed run twice, then diffed (HST-114);
- per-subsystem hashes every tick to find the first divergent tick (HST-126, HST-127);
- model-based tests where tick-by-tick stepping is the oracle for jumps and rollbacks (HST-117, HST-118);
- metamorphic relations (HST-119..HST-121);
- a balanced statistical protocol for A2 rules (HST-137, HST-151).

## 2. Proposals by spec (all Proposals)

| Spec | Change | From |
| --- | --- | --- |
| 18 noise | Grid-dead background 35 day / 25–28 night; two thresholds (notice 0 dB, place +10 dB); generator 85 dB at 1 m; night, sun and wind rule from the weather; sound marks at the last opening on the path with 3° front / 10–20° side bearing error | HST-138..HST-142, HST-55 |
| 16 view | Noise overlay drawn as contours at the notice and place thresholds, horde flow as arrows ([vector-graphics](vector-graphics.md) §4.2) | HST-139, HST-143 |
| 18 noise, at city scale | Opening-to-opening loss tables per building at generation; street graph between buildings; search pruned at the audible radius | HST-141 |
| 23 walkers | Horde turn as a wave; informed share; groups switch level as one | HST-146; RA-4 |
| 10 fortification | Barricade strength in kN/m against crowd load by queue depth | HST-145 |
| 27 detail levels | Horde counts on the chunk graph with min(demand, supply) and Weidmann speed; edge accumulators at the hot border | HST-143, HST-15 |
| 50 movement | Evaluate a floor-field automaton for walkers against the current 8-direction paths | HST-144 |
| 19 light | Light from the solar and lunar formulas with a cloud factor, replacing fixed hours | HST-148, HST-93 |
| 36 weather | Storm records as rain weights; forecasts as keyed perturbations | HST-147, HST-149 |
| 06 / 40 testing | Swarm harness, `SIM_BUGGIFY`, per-subsystem tick hashes, model-based jump vs step, metamorphic relations, balanced A2 protocol | HST-104..HST-137 |

## 3. Open questions

1. Is a 20 dB drop (true "ten times") the design target, or is about 6× enough? Either way the claim and the numbers must agree (HST-138).
2. Should walkers moan? Moans make the horde wave and relay sound (RA-111); today only pushing on doors makes walker sound (`walkers.cpp`).
3. Cell size for a floor-field automaton (0.4 m in the literature) against spec 50's grid (HST-144).
4. Our planet's axial tilt, year length and synodic month: generator parameters, shared with the real-size planet work (0063).
5. RapidCheck with doctest: not officially integrated; test a wrapper or call it inside a `TEST_CASE` (HST-118).

## 4. Sources

**Crowds:**
- Seyfried et al. 2005 and 2007; Kretz 2015; Pathfinder verification docs; SFPE Handbook 2016
- Helbing, Farkas, Vicsek 2000; Helbing and Molnár 1995; Helbing, Johansson, Al-Abideen 2007
- Green Guide 5th ed.; FHWA 98-107; Hughes 2002
- Treuille, Cooper, Popović 2006; Narain et al. 2009; Sewall, Wilkie, Lin 2011; Xiong et al.; Hänseler et al. 2013
- Burstedde et al. 2001; ORCA; Couzin et al. 2002 and 2005; Dyer et al. 2009; Attanasi et al. 2014; Bain and Bartolo 2019
- Fruin 1993; structuremag.org; 7 Days to Die and Days Gone wikis

**Sound:**
- EMD windPRO appendix; ISO 9613-1/-2; Arup Strutt; Saurenman et al. 2005; NCHRP 882; Wiener and Keast 1959; CadnaA docs; Gabrielson 2006
- Asensio et al. 2020; Bonet-Solà et al. 2021; Aletta et al. 2020; Abdmouleh and Dahech 2024; Dance et al.; Lecocq et al. 2020
- NPS NRTR-2008-001; EEA 2020; ASA glossary; ISO 7731; Z Audiol 2024; Front. Psychol. 2021; Zahorik et al. 2005; Pearsons et al. 1977
- Briggs & Stratton; Barnard et al. 2012; UNECE R116
- Raghuvanshi and Snyder 2014; Raghuvanshi 2021; Stavrakis et al. 2008; Steam Audio; Audiokinetic; Boev 2015; Davari and Verbrugge 2021; Leonard 2003; Kang et al. 2001

**Sky:**
- Bloemendaal et al. 2020; Emanuel et al. 2006; Vickery et al. 2000; Vickery and Wadhera 2008; Sharkey, Tawn, Brown 2019; Della-Marta et al. 2009; Rudeva and Gulev 2007
- Blackmon 1976; Orlanski 1975; NOAA and EUMETSAT rainband material; Princeton MCS climatology; COMET; HESS 21 (2017); NOAA CPC
- BETWIXT TBN2; Cowpertwait 2004; Wilks 1998; Paschalis et al. 2013; Rebora et al. 2006
- NOAA solar calculator; NREL SPA; AMS Glossary; USNO Circular 171 (skylight); PMC8526603; Jechow et al. 2019; Krisciunas and Schaefer 1991; Reingold and Dershowitz; Kasten and Czeplak 1980
- ECMWF node 25701 and Tech Memo 911; NHC verification; Zhang et al. 2019

**Testing:**
- Zhou et al. 2021 (FoundationDB); FoundationDB `Buggify.h`; transactional.blog; antithesis.com
- TigerBeetle VOPR, blog and TIGER_STYLE; Resonate docs; sled.rs
- Groce et al. 2012; Hypothesis; MacIver and Donaldson 2020; RapidCheck; Chen et al. 2018; Segura et al. 2016; Olsen and Raunak 2019; Clark et al. 2022
- Scholz and Stephens 1987; Holm 1979; Benjamini and Hochberg 1995; Younes and Simmons 2002; Yang et al. 2011
- Factorio FFF-47 and Lua docs; Bettner and Terrano 2001; GGPO; Pollard 2024; Box2D 2024; Microsoft `/fp` docs; GCC docs; Dawson 2013; CORE-MATH; libFuzzer
