# Borrowed from other fields: nested models, telemetry, tracking and paging

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [spec 27](../tier0/specs/27-detail-levels.md), [spec 28](../tier0/specs/28-pop-levels.md), [spec 29](../tier0/specs/29-embodied-pops.md), [spec 06](../tier0/specs/06-save-replay.md), [spec 18](../tier0/specs/18-noise-field.md), [spec 19](../tier0/specs/19-light-field.md), [spec 36](../tier0/specs/36-fields-and-weather.md), [spec 05](../tier0/specs/05-clock.md), [layer jumps](../design/layer-jumps.md), [zoom continuum](../design/zoom-continuum.md), [focus and compression](../design/focus-and-compression.md), [pop-behaviour](pop-behaviour.md), [perception-and-run-ahead](perception-and-run-ahead.md). **Evidence:** four web-research passes on 2026-10-09.

Read first-hand:
- the WRF technical note, best practices and moving-nest pages; Harris and Lin 2013; Berger and Colella 1989;
- the AMReX, Clawpack and p4est docs; Lilly et al. on MPAS local time stepping; Koutsoyiannis and Onof 2001; the Leclercq and Burghout hybrid-traffic papers;
- the NPS DIS enumerations; Fujimoto's dead-reckoning slides; the IETF LSMA draft; Möller et al. on HLA;
- the AVEVA PI compression presentation; the swinging-door patent; Gorilla (VLDB 2015);
- NASA tech briefs, the TMO progress report and the Remote Agent results; RFC 4838 and RFC 9171; Vahdat and Becker; Chaintreau et al.;
- the HLS spec, Git internals, MathWorks tracker docs, Moravec and Elfes 1985, OctoMap, Game AI Pro 2 chapters 33–34, Orkin 2005, Isla and Blumberg, Demers et al. 1987;
- Denning's 1979 working-set report, ARC, 2Q, CLOCK-Pro, Linux `workingset.c`, TinyLFU, GreedyDual-Size, Srinath et al. 2007, Mowry et al. 1992, Padmanabhan and Mogul 1996, RFC 2697, the Go GC pacer design, a TI Schmitt-trigger note, the IBM Easy Tier redpaper and the Ceph cache-tiering docs.

Not read: the IEEE 1278.1 text (DIS thresholds via a patent quoting it), Bristol's 1990 paper, the DS1 beacon validation report (host unreachable), Debreu et al. 2012, Chu and Opderbeck 1972, the LIRS paper (font-encoded), Thrun's tables first-hand. **IDs:** XD.


The user asked which technologies outside games could serve the simulation: mapping, sonar, navigation, radar, telemetry, space communication, compression. Four fields had the most leverage:
- **Nested weather models and adaptive meshes**, for the border between hot and cold chunks.
- **Military dead reckoning, industrial telemetry and spacecraft operations**, for far entities, the view stream, saves and in-world news.
- **Radar, sonar and robot maps**, for what each pop knows.
- **Operating-system memory paging**, for choosing the hot set.

The user also asked whether a storm and the day/night cycle would show the same way on the world map and in the street (XD-133..XD-136).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| XD-1 | WRF one-way nesting sends information only coarse → fine; in two-way nesting the fine solution replaces the coarse one where they overlap, fed back every coarse step; the coarse grid steps first and the child catches up | sourced | Skamarock et al. 2019, WRF-ARW Tech Note v4, ch. 7 |
| XD-2 | WRF space and time ratios between parent and nest must be integers; odd ratios (3:1) share coincident points; NCAR recommends 3:1 or 5:1 | sourced | Tech Note §7.1–7.2; NCAR nesting slides |
| XD-3 | WRF's outer boundary is 1 specified row plus a 4-row relaxation zone, with Davies–Turner relaxation ramping linearly from the specified row to zero at row 5 | sourced | Tech Note §6.4 |
| XD-4 | Nests use only the specified outer row, interpolated from the parent in space and time; that interpolation is not mass-conserving | sourced | Tech Note §7.2–7.3 |
| XD-5 | Feedback is conservative averaging (9 fine cells → 1 coarse at 3:1) and skips the outermost row, which drives the child | sourced | Tech Note §7.2, §7.4 |
| XD-6 | Nest terrain is blended over a pure-coarse zone plus 5 cells ramping 100/75/50/25/0 % coarse "to reduce lateral boundary noise" | sourced | Tech Note §7.4 |
| XD-7 | A moving nest shifts its interior state, discards the trailing edge and interpolates the leading edge from the parent; it moves only when away from the parent's boundary | sourced | Tech Note §7.1, Fig. 7.5 |
| XD-8 | A specified nest move is at most 1 coarse cell per axis; moving costs under 2 % overhead | sourced | WRF moving nest page (v3.1) |
| XD-9 | Vortex-following nest: position recomputed every 15 min, max vortex speed 40 m/s (search radius about 36 km), kept 8 coarse cells from the parent edge | sourced | same |
| XD-10 | HWRF needed mass adjustment because terrain differed by more than 1 km between domains; tracking failed on weak systems until the tracker moved into the model | sourced | Trahan et al., AMS 31st Hurricanes |
| XD-11 | One-way nesting reflected outgoing waves at up to nearly twice their amplitude; two-way nesting plus relaxation at the nest edge made reflections negligible | sourced | Harris and Durran, AMS 13th Mesoscale (abstract) |
| XD-12 | FV3 keeps mass exact to machine precision by never feeding back the mass field; the update is an equal-weight average of r² cells | sourced | Harris and Lin 2013, MWR |
| XD-13 | WRF practice: domains at least 100 × 100 points, at least 5 in the boundary zone, boundaries away from the area of interest; a 9 → 3 km nest showed "large discrepancy along the nested boundary" | sourced | WRF Best Practices |
| XD-14 | Proper nesting (AMR): at least one level l−1 cell separates level l from level l−2; a fine grid starts and ends on coarse-cell corners | sourced | Berger and Colella 1989, JCP 82 |
| XD-15 | Time is refined by the space ratio r: fine levels take r steps per coarse step; fine border cells are interpolated linearly in time from the coarse level | sourced | same |
| XD-16 | Refluxing: a coarse cell next to a fine grid swaps its own face flux for the sum of fine fluxes over the r substeps, so conservation is exact | sourced | same |
| XD-17 | A buffer zone keeps features from leaving the fine grid before the next regrid; typical width 2 cells; wider buffers allow rarer regrids | sourced | same |
| XD-18 | AMR cost split: integration 78 %, interpolation 13 %, conservation updates 2.8 %, regridding 1.7 %; a point-by-point boundary fill had taken 40 % until rewritten patch-wise | sourced | same |
| XD-19 | Berger–Oliger create and remove nested grids recursively from truncation-error estimates; finer space implies a finer time step | sourced | Berger and Oliger 1984, JCP 53 (abstract) |
| XD-20 | Clawpack: regrid interval and buffer width are typically equal (Courant number near 1), small integers like 2–3 | sourced | Clawpack 5.7 refinement docs |
| XD-21 | AMReX defaults: error buffer 1, proper nesting 1, grid efficiency 0.7, blocking factor 8; a flux register stores area- and time-weighted fluxes, then reflux, then average down | sourced | AMReX AmrCore docs |
| XD-22 | p4est enforces 2:1 balance (neighbours differ by at most one level); 5 × 10¹¹ octants on 220,000 cores | sourced | Burstedde et al. 2011, SISC |
| XD-23 | Multirate Runge–Kutta lets regions use different time steps, conservatively and stably under local CFL | sourced | Constantinescu and Sandu 2007 (abstract) |
| XD-24 | MPAS-Ocean local time stepping: fine cells step Δt/M; an edge shared by two regions belongs to the region nearer the fine one; fluxes accumulated over substeps conserve mass exactly; up to 10× faster than RK4 | sourced | Lilly et al., arXiv 2405.10505 |
| XD-25 | ROMS-AGRIF two-way nesting conserves volume and tracers by refluxing | reported | Debreu et al. 2012, Ocean Modelling |
| XD-26 | Proportional adjusting X_s = X̃_s · Z ÷ ΣX̃ makes fine values sum exactly to the coarse total and stay non-negative | sourced | Koutsoyiannis and Onof 2001, J Hydrol 246 |
| XD-27 | Canonical cascades conserve mass only in expectation; microcanonical cascades conserve it exactly at each step; RainFARM can conserve each coarse pixel exactly | reported | HESS 24:169 (2020); rainfarmr docs |
| XD-28 | Wilks: downscaling has intrinsic indeterminacy, so explicit randomness is "natural and logically consistent"; generator parameters can follow the coarse circulation day by day | sourced | Wilks 2010, WIREs Climate Change (abstract) |
| XD-29 | Hybrid traffic (Leclercq): the coarse–fine interface is a point with flow = min(upstream demand, downstream supply); a vehicle is created when cumulative flow reaches 1 | sourced | Leclercq 2007, TR-B 41 |
| XD-30 | Hybrid traffic (Burghout): borders mid-link, never at intersections; the fine side sends blocking and density back so queues back up across the border | sourced | Burghout et al. 2005, KTH |
| XD-31 | DIS Entity State PDU dead-reckoning record: an algorithm code (static; fixed or rotating orientation; constant velocity or acceleration; world or body frame) plus linear acceleration and angular velocity | sourced | NPS DIS enumerations; Open-DIS docs |
| XD-32 | Standard DIS thresholds: 1 m position, 3° orientation, 5 s heartbeat | reported | US 8,834,163 B2 (patent quoting IEEE 1278.1) |
| XD-33 | The sender runs the receivers' model next to the true state and sends only when the error passes the threshold or the heartbeat expires; receivers extrapolate from the sender's timestamp and smooth corrections | sourced | Fujimoto, Dead Reckoning slides |
| XD-34 | Without dead reckoning a 144-byte PDU at 30 Hz is 34,560 bit/s per entity: 29 entities per 1 Mbit/s | sourced | same |
| XD-35 | With dead reckoning, land vehicles average about 2 updates per second, aircraft 5, up to 15 when manoeuvring: about 6–15× fewer than 30 Hz | sourced | IETF draft-ietf-lsma-scenarios-00 |
| XD-36 | HLA "Smart Update Rate Reduction": each subscriber asks for a maximum update rate per attribute, so an active threat gets a high rate and a distant friendly a low one | sourced | Möller et al., 08F-SIW-064 |
| XD-37 | RPR FOM 2.0 carries DIS into HLA; MAK VR-Link exposes per-entity translation and rotation thresholds and smoothing periods | reported | Wikipedia RPR FOM; MAK VR-Link docs |
| XD-38 | OSIsoft PI defaults: exception deviation 0.1 % of span, 600 s max; compression deviation 0.2 %, 8 h max; compression is swinging door, to drop "data that can safely be recreated with interpolation" | sourced | Bregenzer, AVEVA 2023 |
| XD-39 | PI field results: most tags shrank by 25 % or more, half met 3:1, every maximum error stayed below instrument precision; an offshore pilot archived 94 % fewer events | sourced | same |
| XD-40 | Swinging door: two "doors" pivot at ±E from the corridor's first point and may only narrow; when parallel, a point is archived and a new corridor starts | sourced | US 4,669,097 (Bristol, Foxboro) |
| XD-41 | DNP3 SCADA: static values plus event classes; analog events by fixed or integrating deadband; rare integrity polls back up report-by-exception; an overflow flag forces a full re-read | reported | ProSoft KB 130; OPC UA Part 4 §7.22.2 |
| XD-42 | Facebook Gorilla: 16 → 1.37 bytes per point (12×); about 96 % of timestamps take 1 bit (delta-of-delta 0), 51 % of values 1 bit (XOR 0); 2 h immutable blocks | sourced | Pelkonen et al., VLDB 2015 |
| XD-43 | Deep Space 1 beacon tones: nominal (no downlink needed), interesting, important, urgent; no tone means an anomaly | reported | Hackett et al. 2019 citing DeCoste et al. 2004 |
| XD-44 | Beacon monitoring was projected to cut monitoring to minutes per day and engineering telemetry to once every several weeks | sourced | Sue, Peng, Wyatt 1997, TMO PR 42-130 |
| XD-45 | Beacon tones need about 10 dB less signal than telemetry, so small antennas suffice | sourced | NASA Tech Brief NPO-20187 |
| XD-46 | New Horizons beacons saved an estimated 2,274 h of 70 m antenna time (about 80 % of track time) | reported | Hackett et al. 2019 |
| XD-47 | On a problem, the next pass carries a concise summary since the last pass: high-rate data around each event, low-rate snapshots between; routine activities caused false alarms | reported | NASA Tech Brief NPO-20822 |
| XD-48 | If uplink is lost the spacecraft keeps executing its stored sequence; a command-loss timer forces safing when it expires (Stardust 9 days, cut to 3) | reported | Wikipedia command-loss timer; NASA Stardust status |
| XD-49 | Remote Agent (DS1, 1999) took goals, not commands, planned one horizon at a time and replanned on board after failures; ground tests "warped" over idle periods | sourced | Nayak et al. 1999 |
| XD-50 | Delay-tolerant networking: store-and-forward on persistent storage; contacts scheduled, opportunistic or predicted; custody transfer is "analogous to a database commit"; bundles expire | sourced | RFC 4838 |
| XD-51 | Bundle Protocol v7: lifetime, a bundle-age block for nodes without accurate clocks, a hop limit, a previous-node block | sourced | RFC 9171 |
| XD-52 | Epidemic routing: nodes that meet swap summary vectors and copy what they lack; hop counts bound the spread; full buffers drop the oldest; delivery reached 100 % with buffers of 10–25 % of all messages | sourced | Vahdat and Becker 2000 |
| XD-53 | Gaps between meetings of human-carried devices are heavy-tailed: 8–25 % over 1 h, 2–3 % over a day (20 % at Dartmouth) | sourced | Chaintreau et al. 2006 |
| XD-54 | Market data recovery: sequence numbers detect gaps; each snapshot records the last update it includes; recovery buffers updates, applies the snapshot, drops the buffered ones it already includes | reported | CME MDP 3.0 (search summary) |
| XD-55 | Apple HLS: keyframes should come every 2 s and every segment must start with one; 6 s segments | sourced | Apple HLS authoring spec |
| XD-56 | x264: keyframes every 250 frames at most, earlier on scene cuts; longer groups save bits with diminishing returns but make seeking coarser | reported | MeGUI x264 settings |
| XD-57 | Git stores objects by hash, the newest version whole and older ones as deltas (an edit to a 22 KB file became about 20 bytes); deep delta chains slow reads | sourced | Git internals: packfiles; git-pack-objects |
| XD-58 | Content-defined chunking (restic, Borg) changes only nearby chunks on an insertion; chunks of 512 KiB–8 MiB | reported | rustic design docs; Borg docs |
| XD-59 | MATLAB tracker defaults: confirm a track at 2 hits in 3 updates, delete at 5 misses in 5 | sourced | MathWorks trackerGNN |
| XD-60 | Other radar confirmation rules: 3-of-4; low thresholds for faint targets raise false tracks | reported | MathWorks trackHistoryLogic; arXiv 2606.05526 |
| XD-61 | A Kalman filter's uncertainty grows every predict step and shrinks every update; under constant velocity, position variance from process noise grows as q·t³/3 | reported | Bar-Shalom, Li and Kirubarajan 2001, ch. 6 (via secondary) |
| XD-62 | The α-β filter is a fixed-gain Kalman filter, chosen by track-while-scan radars because it is cheap over many tracks | reported | Wikipedia; IMECS 2008 |
| XD-63 | Gating: accept a 2-D measurement if its normalised distance squared ≤ 9.21 for 99 % (about 3σ) | reported | arXiv 2512.18508 |
| XD-64 | Airport radar revisits a target every 4.8 s (ASR-9); long-range radar every 12 s | reported | Wikipedia ASR-9; FAA Order 1812.8 |
| XD-65 | Exact JPDA grows exponentially with target count (infeasible above about 9 close targets); MHT is pruned to stay tractable | reported | arXiv 2308.06326 |
| XD-66 | Ekelund ranging: range = change in own speed across the line of sight ÷ change in bearing rate; needs two legs and one turn | reported | Wikipedia TMA; NavOps notes |
| XD-67 | Bearings-only tracking needs the listener to manoeuvre to get range; a constant-bearing geometry carries no range information; at least 4 bearings | reported | Nardone and Aidala (DTIC ADA102073) |
| XD-68 | A cross-fix from two bearings is best near 90° and weak below 30° or above 150° | reported | USCG chart-plotting guide |
| XD-69 | Moravec and Elfes 1985: 6-inch cells, 24 sonars with 30° beams; each echo marks a wedge empty and the arc weakly occupied; a map from a few hundred readings in under 1 s | sourced | Moravec and Elfes 1985 |
| XD-70 | Log-odds occupancy update: l ← l + inverse sensor model − l0 | reported | Thrun, Probabilistic Robotics ch. 9 (slides) |
| XD-71 | OctoMap clamps log-odds to [−2, 3.5] (p 0.12–0.97) so cells can change after 5 agreeing hits; one float per node | sourced | Hornung et al. 2013 |
| XD-72 | Plain log-odds weighs old and new evidence equally and fails when things move; fixes are decay toward the prior or time-weighted updates | reported | AV2 docs; US 12,523,498 |
| XD-73 | Merging robot maps by adding log-odds double-counts shared observations; one system keeps the larger value per cell; bandwidth-limited systems send deltas | reported | MathWorks answers 559550; arXiv 2110.04390 |
| XD-74 | ACT-R memory activation decays as a power law with d = 0.5: one memory loses 1.15 activation per tenfold elapsed time | reported | Anderson and Lebiere 1998 (via secondary) |
| XD-75 | Animals remember predators from days to months (salmonids 10–21 days, minnows 2+ months) and re-inspect fading risks | reported | PMC3521018; EcoEvoRxiv 11964 |
| XD-76 | The Last of Us: an NPC that perceives the player creates a record (position + time) and tells the others; after 10 s unseen one goes to check; a forced check when the player is more than 5 m off cut fights from about 2 min to 30 s | sourced | McIntosh, Game AI Pro 2 ch. 34 |
| XD-77 | The Last of Us search map: once the player is lost, active cells bleed into neighbours over time; any cell an NPC sees is cleared every frame | sourced | same |
| XD-78 | The Last of Us infected hear logical sound events with designer radii multiplied by type and state (infected about 6× human hunters); rays test occlusion | sourced | Botta, Game AI Pro 2 ch. 33 |
| XD-79 | Infected search: breadth-first over the navmesh from the last known spot, adding points where line of sight breaks, then losing interest | sourced | same |
| XD-80 | F.E.A.R. working memory: facts with 16 attributes, each with a 0–1 confidence; sensors event-driven or polled at 3 Hz; Orkin suggests an expiry time per fact | sourced | Orkin 2005 |
| XD-81 | Isla and Blumberg: a probability grid where unobserved probability spreads to neighbours at a rate set by the target's mobility; cells seen empty are zeroed and the rest renormalised | sourced | Isla and Blumberg, "Object persistence" |
| XD-82 | Source engine enemy memory: last known and last seen position, first and last seen times, an "eluded me" flag; forgotten after 60 s by default | sourced | Source SDK `ai_memory.h` (unofficial mirror) |
| XD-83 | Splinter Cell: Conviction shows the enemies' last known position as a silhouette; reviewers found the fixation exploitable | reported | Wikipedia |
| XD-84 | Thief propagated sound through designer-placed rooms, with three awareness tiers and suspicion that builds over time | reported | Shacknews; ModDB Dark Engine |
| XD-85 | Dwarf Fortress: each historical figure keeps a knowledge list; rumours start with witnesses and spread by retelling; no false rumours except secret identities; knowledge fades over weeks to years | reported | DF wiki, Rumor |
| XD-86 | Demers et al. 1987 anti-entropy gossip: push reaches everyone from one site in log₂n + ln n + O(1) rounds (about 17 for n = 1,000); the paper uses SIR terms | sourced | Demers et al., PODC 1987 |
| XD-87 | Rumour mongering: the share never reached solves s = e^(−(k+1)(1−s)); with counter k = 2, about 3.7 % never hear at about 3.3 messages per site; s ≈ e^(−traffic) | sourced | same |
| XD-88 | Daley–Kendall rumours stop by contact (two spreaders meeting both stop), unlike SIR recovery; the never-hear fraction tends to 0.203 | reported | arXiv 2002.06821; 1512.03022 |
| XD-89 | Denning's working set W(t, θ) is the set of pages referenced in the last θ; a larger θ never raises the fault rate (inclusion) | sourced | Denning, "Working Sets Past and Present", 1979 TR / IEEE TSE 1980 |
| XD-90 | The fault rate is the slope of mean working-set size against the window; both come from one pass over the re-reference histogram | sourced | same |
| XD-91 | Programs spend at least 98 % of time in phases; 40–50 % of faults happen in transitions taking about 2 % of the time, at 100–1,000× the in-phase rate | sourced | same |
| XD-92 | After a phase change the working set keeps the old locality for θ ("overshoot"); a lookahead policy beat tuned WS by about 10 % | sourced | same |
| XD-93 | Space-time is minimised near the knee of the lifetime curve; across 8 programs within 2 % | sourced | same |
| XD-94 | Thrashing control caps the active load near its optimum and queues the rest; a fixed low cap leaves the system "chronically underloaded", so the cap must adapt | sourced | same |
| XD-95 | Feedforward admission compares a job's measured memory demand with the free pool before activating it, preventing overload instead of reacting to it | sourced | same |
| XD-96 | Page-fault-frequency control: if faults come more often than 1/θ add pages, otherwise shrink to what was used since the last fault; within 10 % of tuned WS | sourced | same |
| XD-97 | The textbook PFF variant uses two thresholds: above the upper add frames, below the lower remove them | reported | UW CSE 451 lecture notes |
| XD-98 | WS and PFF cost curves have flat minima, so varying θ dynamically is of "little point" | sourced | Denning 1979 |
| XD-99 | ARC keeps recently-seen-once and seen-twice lists plus ghost lists, adapting their split at O(1) cost | sourced | Megiddo and Modha, FAST 2003 |
| XD-100 | ARC is scan-resistant: a one-time pass flushes only the seen-once list; hit ratios about double LRU's on database traces | sourced | same |
| XD-101 | 2Q: a 25 % probation queue and a ghost list of 50 % of the buffer; with a third of references from scans it keeps about 70 % of its hit rate, LRU about 60 % | sourced | Johnson and Shasha, VLDB 1994 |
| XD-102 | Reuse distance: a page touched once has infinite reuse distance and is evicted quickly; under LRU a loop one page larger than memory gets zero hits; CLOCK-Pro cut a run from 22.1 s to 11.7 s | sourced | Jiang, Chen, Zhang, USENIX ATC 2005 |
| XD-103 | Linux refault distance: a page refaulting within the working-set size of its eviction is activated straight away | sourced | Linux `mm/workingset.c` |
| XD-104 | TinyLFU admits a candidate only if its estimated frequency beats the victim's, with counters halved every W samples | sourced | Einziger, Friedman, Manes, arXiv 1512.00727 |
| XD-105 | GreedyDual-Size: priority H = L + cost ÷ size; evict the minimum and set L to it, which ages everything else | sourced | Cao and Irani, USITS 1997 |
| XD-106 | Prefetching is judged by coverage (issued), accuracy (used ÷ sent), timeliness (arrived in time) and pollution (misses it caused) | sourced | Srinath et al., HPCA 2007 |
| XD-107 | Below 40 % accuracy a stream prefetcher always hurt; feedback-directed prefetching gained 6.5 % with 18.7 % less bandwidth | sourced | same |
| XD-108 | Prefetch distance = ⌈latency ÷ time per step⌉; prefetching too early gets the data evicted before use | sourced | Mowry, Lam, Gupta, ASPLOS 1992 |
| XD-109 | A Markov prefetcher cut memory stalls by 54 % on average | reported | Joseph and Grunwald, ISCA 1997 (abstract) |
| XD-110 | Web predictive prefetch: access time drops by about the same fraction that traffic rises | sourced | Padmanabhan and Mogul 1996 |
| XD-111 | Token bucket: tokens arrive at a committed rate up to a depth; work conforms if enough tokens are in the bucket | sourced | RFC 2697 |
| XD-112 | Go's GC pacer makes allocators do collection work in proportion to what they allocate, with a 25 % background target smoothed by a PI controller | sourced | Go GC pacer redesign proposal |
| XD-113 | A Schmitt trigger switches up at one threshold and down at a lower one, to stop noise-induced oscillation | sourced | TI SCEA046B |
| XD-114 | IBM Easy Tier tracks heat per extent, plans at least daily, moves a limited number every 5 minutes, skips moves worth less than they cost, and moves only between adjacent tiers | sourced | IBM Redpaper REDP-4667 |
| XD-115 | Ceph promotes an object only if seen in 2 of the last 12 periods, with a minimum age before eviction; cache tiering was deprecated because warm-up cost dominated | sourced | Ceph cache-tiering docs |
| XD-116 | Our hot → warm step ratio is 7,200 (0.5 s → 1 h), against 2–5 per level in nested and adaptive models. The clock already has a 60 s body step (spec 05), so a minute level would give ratios of 120 and 60; the rest must be event-based coupling | derived | XD-2, XD-15, XD-21; spec 05 |
| XD-117 | Spec 27 rule 3 (neighbours of hot are at least warm) is proper nesting: hot never touches cold (XD-14); keep it as a test | derived | XD-14, XD-22 |
| XD-118 | Buffer width ≥ top speed × regrid interval: with a 10 s regrid, a horde at 1.5 m/s needs 15 m, a runner at 6 m/s 60 m, a car at 15 m/s 150 m | derived | XD-17, XD-20 |
| XD-119 | Warm → hot as an integer split of the exact count, drawn by identity (an integer form of proportional adjusting); hot → warm as an exact sum | estimate | XD-26, XD-27, XD-5 |
| XD-120 | The hot side owns border edges: it records integer crossings per edge, and warm applies them at its step and never computes its own flux there (refluxing), so the ledger stays exact | estimate | XD-16, XD-24 |
| XD-121 | Agents cross a border with flow = min(demand, supply) and a fractional accumulator that spawns an individual at 1, keyed by edge and crossing number; borders mid-street, not at doors; blocking sent back across | estimate | XD-29, XD-30 |
| XD-122 | A far entity or group can be a motion-law record (static, linear, accelerating; start tick, start position, velocity, end tick) evaluated in integer ticks, with its next event (arrival, border, wake trigger) in a queue: the jump form of movement | estimate | XD-31, XD-33, XD-49 |
| XD-123 | The view stream can be lossy (PI-style deadband plus swinging door, Gorilla blocks, about 1.5 bytes per point) because replay truth is seed + inputs; saves stay lossless | estimate | XD-38..XD-42 |
| XD-124 | Saves: a full save per game day as the keyframe, hourly deltas against that keyframe only (never chained), state chunked by its own structure and stored once by hash, so a save writes only changed regions | estimate | XD-55..XD-58; spec 06 rule 18 |
| XD-125 | Beacon flags for regions not being ticked: each computes a cheap summary every N ticks and raises nominal / interesting (log) / important (wake soon) / urgent (wake now); routine schedules stay out of the alarm rules | estimate | XD-43..XD-47 |
| XD-126 | Distant factions can run goal-based stored plans between contacts, with a command-loss-style timer forcing a replan so plan and world cannot drift far | estimate | XD-48, XD-49 |
| XD-127 | In-world news as delay-tolerant bundles carried by people (origin, creation tick, lifetime, hop limit, priority), swapped on meeting, oldest dropped when memory is full, with heavy-tailed meeting gaps | estimate | XD-50..XD-53 |
| XD-128 | Per-pop knowledge as 16 track slots × 16 B = 256 B per pop (2.5 MB for 10,000 pops), all integers; per-pop grids would be about 640 MB; shared grids belong to a group or settlement | derived | XD-76, XD-80, XD-82 |
| XD-129 | A track's uncertainty radius r = min(r0 + v·Δt, r_max) and its confidence l0 + (l − l0)·2^(−Δt ÷ half-life) are computed when read, never per tick; half-lives per kind (threats minutes, danger zones days to weeks) | estimate | XD-72, XD-75, XD-81 |
| XD-130 | A track is created by one hearing, confirmed by 2 of 3 perceptions or one sighting, deleted after 5 of 5 misses counted only when the pop could have perceived that cell | estimate | XD-59, XD-77, XD-81 |
| XD-131 | A hearing gives a sector (bearing, with a range band only if the sound class implies a source level); two pops' sectors cross-fix when they cross at 30–150°; a lone listener gets range only by moving | estimate | XD-66..XD-69 |
| XD-132 | Pops sharing knowledge exchange their top 3–4 tracks on contact, merge by maximum (never sum), degrade one step per retelling and stop after 2 unhelpful contacts: about 4 % never hear, about 3.3 messages per pop | derived | XD-73, XD-87 |
| XD-133 | The hot set as a working set W(t, τ) with τ at the knee; wake cost concentrates in transitions (entering a new district), about half of all wakes in about 2 % of play time | estimate | XD-89..XD-93 |
| XD-134 | Scan resistance for a sprinting player: a chunk touched once goes to a small probation tier, joins the protected hot set on a second touch; a ghost list of recently demoted chunks re-promotes at once on a short refault | estimate | XD-99..XD-103 |
| XD-135 | Prefetch lead ≥ player speed × wake latency (queue included); measure prewarm accuracy and stop prewarming below about 40 % | estimate | XD-106..XD-108 |
| XD-136 | Budgets must be counted in deterministic work units, never measured milliseconds: where a rule's agreement is only A2, a machine-dependent tier decision would change outcomes | estimate | XD-111; RA-115 |
| XD-137 | Day and night: the sun's elevation is a pure function of latitude, longitude, date and time (standard solar-position formula, e.g. the NOAA solar calculator or Meeus); spec 19's fixed hours (twilight 05–07 and 19–21) cannot hold on a real-size planet (0063). The moon phase sets night light (spec 19 lists 0.1–0.3 lux moonlit, 0.001 moonless, but uses 0.1) | estimate | spec 19; formula source to read |
| XD-138 | Storms: spec 36 draws rain per day per cell independently, which its open question 2 calls "wrong for a front". A crossing storm needs storm records keyed per weather region and period (track, speed, radius, strength) moving with the prevailing wind; rain at a point = background + the storms over it | estimate | spec 36 |
| XD-139 | Storms must redistribute each cell's daily rain total, not add to it (proportional adjusting or a microcanonical split), so spec 36's exact year sums still hold | estimate | XD-26, XD-27; spec 36 rule 5 |
| XD-140 | Map and street read the same storm and sun functions, so they agree by construction; clouds from the storm field dim daylight in the light field; an in-world forecast is the future of that function plus an error growing with lead time | estimate | DL-8/9; RA-94 |

## 1. Answer in one page

**Nested weather models solved our hot–cold border.** Five rules carry over:
- Keep a warm ring between hot and cold; spec 27 rule 3 already does this (XD-117).
- The finer side owns the border edges and the coarser side applies its crossings (XD-120).
- Fill the leading strip and shift the interior when the bubble moves (XD-7).
- Size the buffer as top speed × regrid interval (XD-118).
- Make coupling two-way for derived fields and keep counts owned by one level (XD-11, XD-12).

They also flag a real problem: our 7,200:1 hot-to-warm step ratio is far outside practice (XD-116).

**Telemetry shows how to move, store and report things without ticking them:**
- **Far entities** follow exact motion laws with a next-event queue (XD-122).
- **The view stream** is compressed like industrial data historians (XD-123).
- **Saves** are a daily keyframe plus non-chained deltas, deduplicated by hash (XD-124).
- **Regions not being ticked** raise beacon flags (XD-125).
- **News** travels as bundles people carry (XD-127).

**Radar, sonar and robots give each pop a cheap memory:**
- 16 tracks per pop (XD-128); uncertainty and confidence computed when read (XD-129).
- Radar-style confirm and delete rules (XD-130); sound as a sector, with cross-fixes from two listeners (XD-131).
- Rumours with gossip mathematics (XD-132).

**Paging gives the hot set its control law:**
- The working-set window (XD-133); probation against sprint flicker (XD-134).
- Feedforward admission against wake spikes (XD-95).
- Prefetch distance and accuracy (XD-135); budgets in work units (XD-136).

**Weather and sun need no simulation.** They are pure functions of place and time, read by every view (XD-137..XD-140). They cost nothing to run ahead and never roll back.

## 2. How the pieces fit with the run-ahead note

| Layer | From [perception-and-run-ahead](perception-and-run-ahead.md) | From this note |
| --- | --- | --- |
| What is hot | Hot set from simulation state; prewarm only where agreement is A1 (RA-115) | Working set, probation, ghost list, admission, prefetch lead (XD-133..XD-136) |
| The border | Two thresholds, groups switch as one (RA-1, RA-4) | Warm ring, edge ownership, integer splits, buffer width (XD-117..XD-121) |
| Far away | Cold jumps (CJ) | Motion laws and next-event queue; beacon flags; stored plans (XD-122, XD-125, XD-126) |
| Time | Run ahead, commit at the view tick, local rollback by cone (RA-110..RA-113) | Minute level between hot and warm (XD-116) |
| What is shown | Stream of curves and pulses (RA-41) | Deadband + swinging door + Gorilla blocks (XD-123) |
| What is stored | Committed state + input log (RA-48) | Daily keyframe + hourly deltas, deduplicated (XD-124) |
| What pops know | — | Track lists, sectors, cross-fixes, gossip, news bundles (XD-127..XD-132) |
| Sky | — | Sun and storm functions (XD-137..XD-140) |
| How the map draws it | Belief at playback time, never the future (RA-57) | Vector tiles on the nested grid, contours and flow arrows over fields, a radar view of the pop's tracks ([vector-graphics](vector-graphics.md) §4) |

## 3. What we don't take

- **WRF's one-way nesting:** it reflects waves (XD-11). Ours is two-way for derived fields.
- **Interpolated boundaries that lose mass** (XD-4): every count crossing a border is an integer and owned by one side.
- **JPDA or MHT for pop knowledge:** exponential cost (XD-65). Nearest match within a gate is enough.
- **Adding log-odds when merging what two pops know:** double-counting (XD-73). Take the maximum.
- **Measured milliseconds in any tier decision** (XD-136).
- **Content-defined chunking for structured state:** chunk by region and id range; keep CDC for opaque blobs (XD-58).

## 4. Open questions

1. **A minute level?** Would a warm-fast level at 60 s (the existing body step) between hot and hourly warm fix the 7,200:1 ratio, or is event-based coupling enough (XD-116)?
2. **Regrid interval** for the hot set, which sets the buffer width (XD-118).
3. **Knowledge per pop:** 16 tracks enough? Which kinds, and their half-lives (XD-128, XD-129)? This belongs with the pop behaviour model ([PB](pop-behaviour.md)).
4. **Do pops share bearings to cross-fix sounds,** or is that a skill (XD-131)?
5. **The solar-position formula and moon phase** in spec 19, and the weather-region size for storm records in spec 36 (XD-137, XD-138); both touch the other session's real-size planet work (0063).
6. **Save format 5** (spec 06 rules 16–18, zstd ready): adopt daily keyframe + non-chained hourly deltas + hash dedup (XD-124)?

## 5. Sources

**Nested models:**
- WRF-ARW Tech Note v4 (Skamarock et al. 2019); NCAR nesting slides; WRF Best Practices; WRF moving nest page (v3.1)
- Trahan et al., AMS 31st Hurricanes; Harris and Durran, AMS 13th Mesoscale; Harris and Lin 2013, MWR
- Berger and Colella 1989, JCP 82:64; Berger and Oliger 1984, JCP 53:484
- Clawpack 5.7 docs; AMReX docs; Burstedde et al. 2011, SISC
- Constantinescu and Sandu 2007, J Sci Comput 33; Lilly et al., arXiv 2405.10505; Debreu et al. 2012, Ocean Modelling
- Koutsoyiannis and Onof 2001, J Hydrol 246; HESS 24:169 (2020); rainfarmr; Wilks 2010, WIREs CC 1:898
- Leclercq 2007, TR-B 41:701; Burghout et al. 2005 (KTH)

**Telemetry:**
- NPS DIS enumerations; Open-DIS; US 8,834,163 B2; Fujimoto Dead Reckoning slides (SIGSIM courseware)
- IETF draft-ietf-lsma-scenarios-00; Möller et al. 08F-SIW-064; MAK VR-Link docs
- Bregenzer, AVEVA 2023; US 4,669,097; ProSoft KB 130; OPC UA Part 4 §7.22.2; Pelkonen et al., VLDB 2015
- Hackett et al., IWPSS 2019; Sue, Peng, Wyatt 1997 (TMO PR 42-130); NASA Tech Briefs NPO-20187 and NPO-20822; Stardust status; Nayak et al. 1999
- RFC 4838; RFC 9171; Vahdat and Becker 2000; Chaintreau et al. 2006
- CME MDP 3.0; Apple HLS authoring spec; MeGUI x264; Git internals; rustic; Borg docs

**Tracking and knowledge:**
- MathWorks trackerGNN and trackHistoryLogic; Bar-Shalom, Li and Kirubarajan 2001; IMECS 2008; arXiv 2512.18508, 2308.06326, 2606.05526
- FAA Order 1812.8; Wikipedia ASR-9, TMA, Splinter Cell: Conviction; DTIC ADA102073; NavOps notes; USCG chart-plotting guide
- Moravec and Elfes 1985; Thrun, Probabilistic Robotics; Hornung et al. 2013; AV2 docs; US 12,523,498; arXiv 2110.04390
- Anderson and Lebiere 1998; PMC3521018; EcoEvoRxiv 11964
- Game AI Pro 2 ch. 33 (Botta) and ch. 34 (McIntosh); Orkin 2005; Isla and Blumberg; Source SDK `ai_memory.h`; Shacknews; ModDB; DF wiki Rumor
- Demers et al. 1987; arXiv 2002.06821, 1512.03022

**Paging:**
- Denning, CSD-TR-276 (1979) / IEEE TSE 6(1) 1980; UW CSE 451 notes
- Megiddo and Modha, FAST 2003; Johnson and Shasha, VLDB 1994; Jiang, Chen, Zhang, USENIX ATC 2005; Linux `mm/workingset.c`
- Einziger, Friedman, Manes, arXiv 1512.00727; Cao and Irani, USITS 1997
- Srinath et al., HPCA 2007; Mowry, Lam, Gupta, ASPLOS 1992; Joseph and Grunwald, ISCA 1997; Padmanabhan and Mogul 1996
- RFC 2697; Go GC pacer redesign; TI SCEA046B; IBM REDP-4667; Ceph docs
