# Combat at three resolutions that agree, including humans against zombies

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [0060](../decisions/0060-one-fight-three-views-behaviour-from-jobs-and-skills.md), [0061](../decisions/0061-the-world-runs-without-the-player.md), [spec 38](../tier0/specs/38-outbreak.md), [threats](../design/threats.md). **Evidence:** read in full: Alemi, Bierbaum, Myers and Sethna (2015, arXiv 1503.01104, text extracted from the PDF); Davis, Bigelow and McEver (Winter Simulation Conference 2000, PDF); DeLong and West (WSC 2002, Lanchester state equations and Janus, PDF); the Hearts of Iron IV wiki page "Land battle"; the RimWorld wiki page "Raid points"; the Infection Free Zone Steam page; two Dupuy Institute posts (QJM attrition, "What is a breakpoint?"); the Wikipedia page on Lanchester's laws. Everything else is from web-search summaries (`reported`). Not read: Engel (1954), Bracken (1995), Fricker (1998), Davis and Bigelow's RAND report MR-1004 (DTIC and RAND returned 403), Munz et al. (2009) (the PDF link now redirects to a home page with no copy), Dupuy's own books, the NPS theses on ATCAL. The web-search budget ran out before Brooks' Zombie Survival Guide, XCOM, They Are Billions combat internals and more on Infection Free Zone could be searched; see "Not found". **IDs:** CB.

Decision 0060 rule 2 says one fight is resolved at the detail attention allows (bodies, squads, counts) and the three must agree on average. This note gathers what military operations research, combat games and zombie epidemic models say about that, and proposes one combat model at three resolutions with a calibration chain and an agreement test. Already covered elsewhere and only cited here: Lanchester basics and the Dupuy critique ([WSF-17, WSF-18](world-systems-factions.md)), the multi-resolution failure modes ([LJ-1..LJ-35](layer-jumps-multiresolution.md)), auto-resolve complaints ([WAR-8..WAR-10, WAR-19](war-in-other-games.md), [LJ-15](layer-jumps-multiresolution.md)), HoI4 width ([WAR-2](war-in-other-games.md)), World in Conflict ([WIC-1..WIC-12](world-in-conflict.md)), They Are Billions hordes ([ZL-28](zoom-and-layer-two.md)), the reproduction number of about 0.3 ([GS-4](global-sim.md)).

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CB-1 | Lanchester state equations as taught with Janus: square law (aimed fire) keeps a_B (B0^2 - Bt^2) = a_R (R0^2 - Rt^2); the second linear law (unaimed area fire) keeps the same form with first powers; the "guerrilla" or ambush law mixes them, one side squared and the other linear | sourced | DeLong and West, WSC 2002, informs-sim.org/wsc02papers/247.pdf |
| CB-2 | Engel (1954) fitted the square law, with a term for troops landing each day, to Iwo Jima and found a close fit; he needed only daily US landings and casualties, Japanese start and end strength, and the end date | reported | Engel, Operations Research 2(2):163-171, 1954 (summary) |
| CB-3 | A 1972 technical note re-ran Iwo Jima with revised casualty data and still found a close fit by Theil's inequality coefficient | reported | Operations Research 20(1):49, doi 10.1287/opre.20.1.49 (summary) |
| CB-4 | A 2022 NPS thesis (Stymfal) re-tested Iwo Jima: R^2 0.9937 for the square law, 0.9027 linear, 0.9414 logarithmic, as cited by a 2025 arXiv paper | reported | arxiv.org/pdf/2512.15058 (summary) |
| CB-5 | Bracken (1995) fitted four Lanchester models to the Ardennes campaign; the linear law fitted in all four; the two sides' per-man effectiveness was about equal when support troops were counted | reported | Naval Research Logistics 42(4):559-577 (summary) |
| CB-6 | Fricker (1998) re-fitted the Ardennes with regression over the whole campaign plus air sorties and found that neither the linear nor the square law fits; he proposed a new form | reported | Naval Research Logistics 45(1):1-22 (summary) |
| CB-7 | Later re-analyses (Wiper, Pettit and Young 2000; Turkes and Lucas 2004 on Ardennes and Kursk) found several Lanchester forms fit about equally well, which explains why studies with different fitting methods disagree; basic laws did poorly on Kursk | reported | ideas.repec.org navres v47 p541; Dupuy Institute 2016-02-29 (summaries) |
| CB-8 | A Korean War study predicted casualties within 1 to 13 % when the simulated units followed their historical tactics | reported | dupuyinstitute.org/tag/attrition (summary) |
| CB-9 | Helmbold's parameters come from initial and final strengths only and do not assume the square law; the defender's advantage has been roughly constant since 1600, while "bitterness" (loss intensity) fell steeply after 1939 | reported | en.wikipedia.org/wiki/Lanchester%27s_laws |
| CB-10 | Thompson and Helmbold relate the attacker's chance of winning to a "defender's advantage" variable built from strengths and survivors, tied to the force exchange ratio; a 2026 paper notes Lanchester and Helmbold relations give outcomes but not win probability or duration directly | reported | nap.nationalacademies.org/read/10710/chapter/11; JDMS doi 10.1177/15485129261430106 (summaries) |
| CB-11 | Dupuy's QJM daily loss = base rate x size x mission x opposition (force ratio) x day or night x special conditions; base 2.8 % a day for the attacker, 1.5 % for the defender; size factor 2.0 under 5,000 men, 1.5 at 5,000 to 10,000, 1.0 at 10,000 to 20,000, so small units lose a larger share | reported | dupuyinstitute.org/2019/12/26/how-attrition-is-calculated-in-the-qjm-vs-the-tndm/ (blog on the 1979 book) |
| CB-12 | QJM one-day ceilings without special conditions: 13.44 % attacker, 5.76 % defender. The TNDM uses 4 % and 6 % bases and many more factors; without surprise and similar factors it tops out near 18 % and 21.6 % a day above 15,000 men, and under 2,000 men a unit can lose 100 % in a day | reported | same as CB-11 |
| CB-13 | Clark (1954) studied 43 US infantry battalions in World War II: casualties at the breakpoint averaged about 40 % but ranged from about 1 % to near 100 %; she called a fixed casualty threshold a gross oversimplification | reported | dupuyinstitute.org/2024/03/26/the-40-rule/ (summary) |
| CB-14 | A 1964 US field manual curve put a defender's chance of breaking near certainty at about 40 % casualties and an attacker's halt near 20 %; many combat models then adopted a 30 to 40 % breakpoint for units of any size, divisions included | reported | dupuyinstitute.org/2018/04/16/c-wam-4-breakpoints/ (summary) |
| CB-15 | In 15 of Clark's 43 cases (35 %) battalions passed 40 % before breaking; Clark judged casualties were probably not the main cause, pointing to weather, terrain and heavy artillery; collapsing units under-record losses (exhaustion, trench foot) | reported | dupuyinstitute.org "The 40% Rule" (summary) |
| CB-16 | Only 2 of 91 US divisions committed in World War II ever lost more than 30 % in a week, yet many changed posture far below that | reported | same as CB-15 |
| CB-17 | Clark defined the breakpoint as the onset of inability to carry out the mission, judged from what units did, not from orders; DMSi (1988) recast it as a forced change of combat posture: attack to defend, defend to withdraw, withdraw to collapse | sourced | dupuyinstitute.org/2018/04/27/what-is-a-breakpoint/ |
| CB-18 | Opposed advance rates by division: Western Europe 1943-45 1.81 km a day (271 engagements), Eastern Front 1943 4.54 km a day (173); after a breakthrough 4.12 and 11.28 km a day; failed attacks often gain nothing or are pushed back | reported | dupuyinstitute.org/2023/04/26/advance-rates-in-combat/ (summary) |
| CB-19 | Dupuy's advance-rate rules: rates fall day by day in sustained operations and are lower in rough terrain, at rivers, in bad weather and at night; force ratio acts only through the chance and margin of winning | reported | dupuyinstitute.org/2016/07/27/trevor-dupuys-combat-advance-rate-verities/ (summary) |
| CB-20 | Davis and Bigelow (RAND MR-1004, 1998) define multi-resolution modelling as one model with several levels of resolution, an integrated family of consistent models, or both | reported | apps.dtic.mil/sti/pdfs/ADA355041.pdf (title and secondary summaries; report not read) |
| CB-21 | Davis, Bigelow and McEver built a small stochastic model (PEM) to explain puzzling results from RAND's entity-level JANUS and MADAM runs, then calibrated PEM to that data; PEM predicted changes in effectiveness over two orders of magnitude with terrain, tactics and sensors | sourced | informs-sim.org/wsc00papers/046.PDF |
| CB-22 | Their first lesson: the detailed model explains output variation with inputs the coarse one lacks ("hidden variables"), so the coarse model needs a random process to reproduce the same spread | sourced | same as CB-21 |
| CB-23 | Their second and third lessons: a concept exact at one level (a "clearing" of a set width) is ambiguous at the other and must be defined for calibration; the two models also differ in scope (PEM ignored trucks that changed the JANUS results) | sourced | same as CB-21 |
| CB-24 | ATCAL and COMAN (NPS theses, 1990s): run the entity model JANUS, then estimate Lanchester attrition rates by maximum likelihood, with target acquisition as a three-state Markov chain and bootstrap confidence intervals; the aggregate is "calibrated" when it reproduces the entity results for that weapon mix | reported | calhoun.nps.edu ATCAL and Janus(A) theses (summaries) |
| CB-25 | A National Research Council appendix warns that calibrated coefficients are highly situation-dependent: a detailed simulation does not behave like a constant-coefficient Lanchester equation | reported | nationalacademies.org/read/5869/chapter/19 (summary) |
| CB-26 | Taylor and Olwell (2002) found entity and aggregate models treat time between shots and target acquisition inconsistently, and that modelling the Lanchester coefficients to include them reduces the gap | reported | NPS paper (summary) |
| CB-27 | A study fitting a Markov Lanchester model to a discrete-event "meeting engagement" found least squares beat maximum likelihood, and method of moments can be biased; a 2023 IWARS study compared scaling old coefficients by a combat multiplier with deriving them from model runs | reported | arxiv.org/pdf/2607.15312; centreforevidence.org IWARS paper (summaries) |
| CB-28 | Hearts of Iron IV land combat runs in one-hour rounds: attacks = attack / 10, each attack uses up one of the target's defenses (or breakthrough when attacking), hits land 10 % of the time while defenses last and 40 % after; organization damage per hit is a d4 x 0.053, strength a d2 x 0.06; equipment and manpower lost are 70 % of the strength loss | sourced | hoi4.paradoxwikis.com/Land_battle |
| CB-29 | HoI4 width depends on terrain (80 urban, 70 plains) and grows with extra attack directions; overfill up to 33 % with a penalty; units that do not fit wait in reserve and join with a base 2 % chance an hour; a battle ends when one side has no front-line divisions able to fight, and the loser retreats, or shatters with nowhere to go | sourced | same as CB-28 |
| CB-30 | HoI4 division organization is the average of its battalions; at zero organization a division leaves the fight; this is the game's morale | reported | gameslantern.com combat stats guide (summary) |
| CB-31 | Stellaris since 3.6 gives each ship a limited number of disengage rolls once its hull falls below a threshold (25 % in a later diary, 50 % in an older forum post): 1 per drive, +2 for the Hit and Run doctrine, +1 for admirals at levels 5 and 10; the developers said repeatable techs made range decide everything | reported | Stellaris dev diary 271 thread, forum.paradoxplaza.com (summary) |
| CB-32 | No official Total War auto-resolve formula was found; players report that it overrates artillery, armour and melee and underrates army size, and a modder says the AI chooses attacks from the expected auto-resolve result | reported | steamcommunity.com app 1142710 threads; legacyofgames.com 2021-07-09 (summaries) |
| CB-33 | Bannerlord's vanilla auto-resolve picks a random troop from each side, applies the attacker's power as damage, and kills with chance damage / victim max HP, so even a looter can kill an elite; mods spread power by tier (one sets tier 6 at +140 % of tier 3) or simulate each troop's HP | reported | nexusmods.com Bannerlord mods 673, 1151, 3453 (summaries) |
| CB-34 | Football Manager's Instant Result is built into the game (skins only expose the button); FM Classic is described as using the same match engine as Career mode, with highlights-only and instant options; no Sports Interactive statement on whether instant uses the same code was found | reported | fmscout.com instant result skins; forum.lowyat.net/topic/2500038 (summaries) |
| CB-35 | A player test ran the same FM setup five times played and five times by Instant Result: overall outcomes similar in both, match details random; another player says the result is computed before kick-off, so a replay is a new roll (unconfirmed) | reported | fmscout.com/q-34423 |
| CB-36 | RimWorld raid size: points = (wealth points + pawn points) x difficulty (0.1 to 2.2) x starting factor (0.7 to 1.0) x adaptation (0.4 to about 1.47), capped at 10,000; one point buys about one unit of combat power (10,000 points: about 250 tribals or 100 pirates) | sourced | rimworldwiki.com/wiki/Raid_points |
| CB-37 | RimWorld's adaptation lowers the next raid after each colonist death (minus 30 adapt days in a small colony) or downing; the threat is sized to the colony, not drawn from a finite enemy | sourced | same as CB-36 |
| CB-38 | Infection Free Zone builds its map from OpenStreetMap for any real place; hordes may attack each night; defence uses melee and ranged weapons, vehicles, walls, gates, towers and light; no combat numbers are published on its store page | sourced | store.steampowered.com/app/1465460 |
| CB-39 | Munz, Hudea, Imad and Smith (2009) model humans S, zombies Z and removed R with kill rate alpha, infection beta, resurrection zeta, natural death delta and birth Pi; the zombie-free state is unstable, and only quick, aggressive, repeated culls eradicate ("hit hard and hit often") | reported | Munz et al. 2009, in Tchuenche and Chiyaka (eds), Infectious Disease Modelling Research Progress, pp 133-150 (summaries) |
| CB-40 | Press coverage of Munz et al.: in a city of 500,000 the zombies outnumber the living in about three days | reported | press summaries of Munz et al. 2009 |
| CB-41 | Alemi et al. (2015), fully mixed: bites at beta S Z, kills at kappa S Z, alpha = kappa / beta; P = Z + (1 - alpha) S is conserved; if zombies win, final Z = Z0 + (1 - alpha) S0 and no humans remain; if humans win, final S = S0 - Z0 / (alpha - 1). Unlike SIR, someone always ends with everything | sourced | arxiv.org/pdf/1503.01104 |
| CB-42 | Alemi et al., stochastic: one zombie in a large population dies out with probability alpha (for alpha under 1); with alpha 0.6, 200 people and one zombie, zombies win only 40 % of runs, with a wide spread of final counts | sourced | same as CB-41 |
| CB-43 | Alemi et al., lattice of one person per site: critical alpha 0.4373 (zombies can spread without bound below it), in the percolation class; near it, surviving pockets of all sizes appear | sourced | same as CB-41 |
| CB-44 | Alemi et al., US run: alpha 0.8 taken from a Bayesian fit of SZR to two films (Witkowski and Blais), meaning zombies bite 1.25 times as well as humans kill; latent half-life 30 minutes; zombies 1 ft/s; humans do not move (transport shuts down); most of the US is turned within a week, but remote Montana and Nevada are still free after four months | sourced | same as CB-41 |
| CB-45 | Alemi et al.: over 7,000 runs, the 28-day risk is highest between metro areas (north-east Pennsylvania, Bakersfield), not in them | sourced | same as CB-41 |
| CB-46 | The fully mixed SZR is Lanchester's linear law with conversion: both loss rates go as the product S Z (one-on-one melee or area contact), and each human loss is a zombie gain. So human against zombie in a crowd follows the linear law, while aimed gunfire at range follows the square law | derived | CB-1, CB-41, [WSF-17](world-systems-factions.md) |
| CB-47 | Spec 38 computes r0 = bite x convert / destroyed, the inverse of Alemi's alpha. Our r0 of about 0.3 for warned people ([GS-4](global-sim.md)) is alpha about 3.3; the films' alpha 0.8 is r0 1.25; Alemi's extinction chance alpha equals 1 / r0, so when r0 is above 1 the share of single-zombie starts that take off in a large town should be close to 1 - 1 / r0, a check for spec 38 | derived | CB-41, CB-42, CB-44, spec 38 rule 3 |
| CB-48 | Historical fits show no single exponent: square at Iwo Jima, linear or neither in the Ardennes, poor at Kursk, many forms fitting equally; so the coarse law's form must be measured from our own finer level, not chosen from history | derived | CB-2..CB-7, CB-25 |
| CB-49 | Every game examined uses a separate, cruder model for the unplayed fight (Total War, Bannerlord) or has no played fight at all (HoI4, Stellaris); none publishes a check that the two agree, and Football Manager is the only one claimed to use one engine for both | derived | CB-28..CB-35, [WAR-8](war-in-other-games.md), [LJ-15](layer-jumps-multiresolution.md) |
| CB-50 | Proposal: one fight model at three resolutions: R1 bodies (spec 20 strikes, bites, shots, each tick), R2 squads (per pair of squads, rates of acquisition, hits and kills per second, casualties assigned to named pops), R3 counts (per front or block, stochastic Lanchester per hour with the exponent and coefficients from a table) | estimate | CB-21, CB-24, CB-46, [LJ-31](layer-jumps-multiresolution.md) |
| CB-51 | Proposal: calibrate each level from the one below by Monte Carlo: many seeded R1 runs fit R2's per-pair rates; many R2 runs fit R3's coefficients and exponent by least squares; spec 38's `measure()` becomes this step for the zombie encounter | estimate | CB-22, CB-24, CB-27, spec 38 "Not in this spec" |
| CB-52 | Proposal: the calibration table is keyed by situation (weapon class, cover and building, light, fortification, warning, training, fatigue), not one number per unit, because coefficients depend on situation; the table lives in `data/` with its generating run | estimate | CB-23, CB-25, CB-26 |
| CB-53 | Proposal: R2 and R3 draw outcomes (binomial tau-leap with keyed draws), never take expected values, so small fights keep their spread and their extinctions (one zombie can still die before biting) | estimate | CB-22, CB-42, [LJ-21](layer-jumps-multiresolution.md), [LJ-34](layer-jumps-multiresolution.md) |
| CB-54 | Proposal: handover mid-fight moves the fight state, not just counts: who is alive, wounds, ammunition, organization or morale, posture, and the front geometry; going down picks which pops took the losses by keyed draws weighted by exposure, and places bodies inside their squad's area; going up sums; a ledger check runs at each switch, with a dwell time against flicker | estimate | [LJ-3](layer-jumps-multiresolution.md), [LJ-7](layer-jumps-multiresolution.md), [LJ-25](layer-jumps-multiresolution.md), [LJ-26](layer-jumps-multiresolution.md) |
| CB-55 | Proposal: the agreement test runs a grid of scenarios at all three levels and compares distributions (winner share, losses on each side, duration, breaks) with the spec 38 harness (Anderson-Darling plus means); a handover test runs R3 to mid-fight, drops to R1, finishes, and must match pure R1 and pure R3 | estimate | CB-47, spec 38 rule 6, [LJ-34](layer-jumps-multiresolution.md) |
| CB-56 | Proposal: morale is a state per squad or front, and a break is a forced posture change (attack to hold, hold to withdraw, withdraw to rout) drawn each step with a chance that grows with recent losses, suppression, surprise, isolation, fatigue and darkness and shrinks with leadership, training and cover; no fixed 40 % threshold; walkers never break | estimate | CB-13..CB-17, CB-30 |
| CB-57 | Proposal: historical rates are sanity bands for human against human at R3, not rules: daily losses within the QJM and TNDM ranges (about 1.5 to 20 % a day for large forces, more for small ones) and opposed advances of about 2 to 5 km a day, 4 to 11 after a breakthrough | estimate | CB-11, CB-12, CB-18 |
| CB-58 | Proposal: whether a fight is watched must not change its expected result (0061 rule 3); a player in command may change it only through orders an AI in that chair could also give, and a test compares AI-commanded R1 with R3 so that unplayed fights are never a trap | estimate | CB-32, CB-33, [WAR-10](war-in-other-games.md), [WAR-19](war-in-other-games.md) |
| CB-59 | Proposal: hordes are counts with positions (a density field), not squads, because zombies have no command, morale or formation; a walker horde against a held building is resolved at R2 by breach points (doors, windows) as queues, since a breach changes everything at once | estimate | CB-38, CB-46, [ZL-28](zoom-and-layer-two.md), spec 20 rule 5 |
| CB-60 | Proposal: zombie forces are never sized to the defender (RimWorld's adaptation); they come from the real dead in the region (spec 38 compartments), so a lost fight makes the next horde larger, not smaller | estimate | CB-36, CB-37, [WAR-21](war-in-other-games.md) |

## 1. Aggregate laws, and how well they fit history

**The laws.** Lanchester's linear law (losses depend on both sides' numbers multiplied, as in one-on-one melee or area fire) and square law (losses depend on the enemy's numbers, as in aimed fire) each have a conserved quantity, so the end of a fight can be read from its start (CB-1, [WSF-17](world-systems-factions.md)). A mixed "ambush" law applies when one side aims and the other fires blind (CB-1).

**Fit to history.** The only clean success is Iwo Jima, a closed island with good records (CB-2..CB-4). The Ardennes gave contradictory answers depending on the fitting method (CB-5..CB-7), and Kursk fitted poorly. When a simulation followed historical tactics, casualties came within 1 to 13 % (CB-8). The lesson for us is CB-48: the shape of the coarse law should come from our own finer level, and history only bounds the result.

**Dupuy's numbers.** The QJM gives daily loss rates with a few multipliers, and lists the ceilings (CB-11, CB-12). Small forces lose a larger share each day. The defender's advantage has been stable for centuries, while the intensity of fighting fell after 1939 (CB-9).

**Breaking.** The "units break at 40 %" rule came from an average in a small study whose author warned against it (CB-13..CB-16). Breaks are posture changes caused by many things, losses among them (CB-17). Games use the same idea: HoI4 organization (CB-29, CB-30), Stellaris disengagement (CB-31).

**Advance.** Opposed advances are slow, about 2 to 5 km a day by division, faster after a breakthrough, slower at night, in rough ground and at rivers (CB-18, CB-19).

## 2. Multi-resolution modelling in military simulation

| Practice | What it says | Findings |
| --- | --- | --- |
| Family of consistent models | One model with levels, or several models kept consistent | CB-20, [LJ-1](layer-jumps-multiresolution.md) |
| Calibrate coarse from fine | Run the entity model many times, fit the coarse parameters | CB-21, CB-24, CB-27 |
| Hidden variables | The coarse model needs randomness to match the fine model's spread | CB-22 |
| Ambiguous concepts | Define each coarse concept in terms of the fine one before calibrating | CB-23 |
| Situation-dependent coefficients | No single coefficient per weapon; key them by situation | CB-25, CB-26 |
| Estimator choice | Least squares can beat maximum likelihood; check bias | CB-27 |

## 3. Games

| Game | Played fight | Unplayed fight | Morale or breaking | Agreement |
| --- | --- | --- | --- | --- |
| Hearts of Iron IV | None (divisions only) | Hourly dice rounds (CB-28, CB-29) | Organization, retreat at zero (CB-30) | Not applicable |
| Stellaris | None | Ship-by-ship combat | Disengage rolls below a hull share (CB-31) | Not applicable |
| Total War | Unit blocks in real time | Auto-resolve, formula unpublished (CB-32) | Units rout when played | Players say it differs ([WAR-10](war-in-other-games.md)) |
| Bannerlord | Up to about 1000 bodies ([WAR-9](war-in-other-games.md)) | Random troop duels (CB-33) | Morale when played | Players say it differs ([LJ-15](layer-jumps-multiresolution.md)) |
| Football Manager | Match engine, watched | Instant Result, built in (CB-34) | Not applicable | Players find similar outcomes (CB-35) |
| RimWorld | Pawns, ticks | None; raids sized to the colony (CB-36, CB-37) | Raiders flee | Not applicable |
| They Are Billions | Up to 20,000 bodies ([ZL-28](zoom-and-layer-two.md)) | None | Zombies never break | Not applicable |
| Infection Free Zone | Squads and towers against hordes (CB-38) | Not found | Not found | Not found |
| World in Conflict | Units in real time ([WIC-1](world-in-conflict.md)) | None | Not found | Not applicable |

CB-49 sums it up: no game found checks that its two models agree. Football Manager is the closest to "simulate the match" (0061), with one engine and a fast path, but even that is not confirmed by the developer.

## 4. Humans against zombies

- Munz et al. (CB-39, CB-40) started the field: without fast, repeated culls the zombies win.
- Alemi et al. (CB-41..CB-45) give the clearest results. The fully mixed model has a closed solution with a conserved quantity, exactly like a Lanchester state equation (CB-46). One number, alpha (kills per bite), decides who wins, and with small numbers chance decides a lot (CB-42). The film fit puts alpha at 0.8, near the edge (CB-44).
- Our spec 38 already has the same structure (CB-47): its r0 is 1 / alpha, and its "took off" share should match 1 - alpha for a single zombie in a large town.
- Brooks' Zombie Survival Guide figures were not found (budget ran out).

## What this means for us (Proposal)

Every row here is an `estimate`. It is tied to findings by ID. None of it is decided.

### One model, three resolutions

| Level | When | State | Step | Resolves |
| --- | --- | --- | --- | --- |
| R1 bodies | A body is attended (Individual, or a zoomed City view) | Each pop's body, position, weapon, ammunition, fear | One tick | Strikes, bites, shots, grabs (spec 20) |
| R2 squads | A City view watches, or the fight is in a watched city | Each squad: members (named pops), area, posture, morale, ammunition, cover | About one second | Per pair of squads in contact: acquisitions, hits, kills drawn from rates; losses assigned to named pops |
| R3 counts | Nobody looks | Per front or block: counts by kind, wounded, morale, posture, front line | One hour | Stochastic Lanchester with exponent and coefficients from the table; posture changes drawn |

Zombies are counts with positions at R2 and R3, never squads (CB-59). Human against zombie contact uses the linear form (CB-46); aimed fire at range uses the square form; an ambush uses the mixed form (CB-1). The exponent in each table cell is fitted, not assumed (CB-48).

### Calibration chain (CB-51, CB-52)

1. Define each R2 and R3 concept in R1 terms first (CB-23): what "in contact", "in cover" and "suppressed" mean for bodies.
2. For each situation cell, run R1 many times with keyed seeds (the Tier 0 walker fights are the first source, as spec 38 already plans). Fit R2's per-pair rates: time to acquire, chance to hit, chance to kill or bite, chance to break.
3. Run R2 many times per cell and fit R3's coefficients and exponent by least squares (CB-27), keeping the spread with a random process (CB-22).
4. Store the table in `data/` with the run that made it. A change in spec 20 regenerates it, and the agreement test fails until it is regenerated.

### Handover mid-fight (CB-54)

- **Down (R3 to R2 to R1):** the fight state already names who is in it, since every soldier is a pop (0060 rule 3). Which pops are dead or wounded is picked by keyed draws weighted by exposure and role. Squads keep their real membership. Bodies are placed inside the squad's area facing the front. Ammunition and morale are carried, never reset.
- **Up (R1 to R2 to R3):** sums. A squad's morale is the share of its members not fleeing; its posture is the majority posture.
- **Rules:** a ledger check at every switch; a dwell time before going up again; a fight being watched never goes up ([LJ-28](layer-jumps-multiresolution.md)).

### The test that checks they agree (CB-55, CB-58)

- **Grid:** for example 1 pop against 3 walkers in the open (the spec 20 test), 10 with tools against 30 walkers at a door, 40 armed against 400 walkers in a street, two squads against two squads in a block, a company against a company over a day.
- **Three-way agreement:** for each cell, R1, R2 and R3 over many seeds give the same distribution of winner, losses on each side, duration and breaks (Anderson-Darling and means, as in spec 38 rule 6), tails included ([LJ-34](layer-jumps-multiresolution.md)).
- **Handover:** R3 to mid-fight, then R1 to the end, matches pure R1 and pure R3.
- **No trap:** AI-commanded R1 matches R3; the player can only beat that through orders (CB-58).
- **Sanity bands:** human against human daily losses and advance rates at R3 fall within the historical bands (CB-57).
- **Zombie law:** with spec 38's rates, the share of single-zombie starts that fizzle is close to 1 / r0 (CB-47).

### Morale and breaking (CB-56)

A break is a drawn posture change, not a threshold. Its chance rises with recent losses (losses this hour weigh more than total losses), suppression, surprise, isolation, fatigue and darkness, and falls with leadership, training, cover and a way out. Defenders hold longer than attackers (CB-14). At R1 it is each pop's fear (spec 20 already uses fear for aim); at R2 the squad's share of fleeing members; at R3 a drawn posture change per front. The same chance table feeds all three and is calibrated like the rest. Walkers never break; living infected, if a sandbox preset adds them, might.

## Open questions

1. What R3 step suits a horde against a city? Spec 38 steps by day; a fight needs an hour or less. Recommendation: hour by default, shorter when losses in a step pass a share of either side.
2. How large is the situation table before it is too costly to calibrate? Recommendation: start with weapon class, cover, light and warning, and add a key only when a test shows it is needed.
3. Who decides a squad's posture at R3 when no player commands: the commander pop's AI (0058), or a rule? Recommendation: the commander pop, through the same order record.
4. Does a wounded pop picked at handover keep a specific wound (region, depth), drawn from the R1 wound distribution for that weapon? Recommendation: yes, drawn and kept.
5. Does a building under siege count as one R2 "squad" (its breach points) or as terrain? Recommendation: breach points as queues (CB-59).
6. How many seeds per cell are enough? The spec 38 test used 300 against 60; a grid of many cells may need fewer per cell with a pooled test.
7. Brooks' Zombie Survival Guide and similar fiction give no data, only rules of thumb; should any of them be used as design targets?

## Not found

- Munz et al. parameter values and eradication numbers (PDF not reachable; values in blog posts disagree).
- Witkowski and Blais' fit itself (only as cited by Alemi et al.).
- Brooks' Zombie Survival Guide combat observations, XCOM's hit and auto-resolve design, They Are Billions' combat internals, Infection Free Zone's horde mechanics, a developer statement on Football Manager's Instant Result: the search budget ran out or nothing was found.
- The exact Total War auto-resolve formula: not published.
- The contents of Davis and Bigelow MR-1004 (403); only its definition is known through others.

## Sources

- Alemi, Bierbaum, Myers, Sethna (2015), "You can run, you can hide: the epidemiology and statistical mechanics of zombies", Phys. Rev. E: https://arxiv.org/abs/1503.01104
- Munz, Hudea, Imad, Smith (2009), "When zombies attack!": https://qubeshub.org/publications/4032/ ; model record https://www.omicsdi.org/dataset/biostudies-other/MODEL1008060000
- Davis, Bigelow, McEver (2000), "Informing and calibrating a multiresolution exploratory analysis model with high resolution simulation": https://informs-sim.org/wsc00papers/046.PDF
- Davis, Bigelow (1998), Experiments in Multiresolution Modeling, RAND MR-1004: https://apps.dtic.mil/sti/pdfs/ADA355041.pdf
- DeLong, West (2002), Janus and Lanchester analysis for an armed UAV design: https://informs-sim.org/wsc02papers/247.pdf
- Engel (1954): https://ideas.repec.org/a/inm/oropre/v2y1954i2p163-171.html ; 1972 note: https://pubsonline.informs.org/doi/10.1287/opre.20.1.49 ; least-action Lanchester paper citing Stymfal: https://arxiv.org/pdf/2512.15058
- Bracken (1995): https://ideas.repec.org/a/wly/navres/v42y1995i4p559-577.html ; Fricker (1998): https://ideas.repec.org/a/wly/navres/v45y1998i1p1-22.html ; Wiper et al. (2000): https://ideas.repec.org/a/wly/navres/v47y2000i7p541-558.html ; Kursk heterogeneous Lanchester: https://arxiv.org/pdf/1903.06666
- Lanchester's laws: https://en.wikipedia.org/wiki/Lanchester%27s_laws ; NRC chapters: https://nap.nationalacademies.org/read/10710/chapter/11 , https://www.nationalacademies.org/read/5869/chapter/19
- Dupuy Institute: https://dupuyinstitute.org/2019/12/26/how-attrition-is-calculated-in-the-qjm-vs-the-tndm/ , https://dupuyinstitute.org/2024/03/26/the-40-rule/ , https://dupuyinstitute.org/2018/04/27/what-is-a-breakpoint/ , https://dupuyinstitute.org/2018/04/16/c-wam-4-breakpoints/ , https://dupuyinstitute.org/2023/04/26/advance-rates-in-combat/ , https://dupuyinstitute.org/2016/07/27/trevor-dupuys-combat-advance-rate-verities/ , https://dupuyinstitute.org/tag/attrition/
- NPS calibration work: https://calhoun.nps.edu/entities/publication/7efa9063-ef58-43d2-a8f0-3ad1fe55bdb0 , https://calhoun.nps.edu/entities/publication/353b6f02-7165-4612-b98e-9a0d3bd9a213 ; meeting engagement: https://arxiv.org/abs/2607.15312 ; IWARS: https://www.centreforevidence.org/article/20231109-estimating-attrition-coefficients-for-the-lanchester-equations-from-small-unit-combat-models-3
- Hearts of Iron IV: https://hoi4.paradoxwikis.com/Land_battle , https://gameslantern.com/article/all-combat-stats-explained-in-hearts-of-iron-iv
- Stellaris: https://admin-forum.paradoxplaza.com/forum/developer-diary/stellaris-dev-diary-271-ready-fight.1546912/
- Total War: https://legacyofgames.com/2021/07/09/the-autoresolve-rework-made-it-even-worse/ , https://steamcommunity.com/app/1142710/discussions/0/3832045251561220120
- Bannerlord: https://www.nexusmods.com/mountandblade2bannerlord/mods/673 , https://www.nexusmods.com/mountandblade2bannerlord/mods/1151/ , https://nexusmods.com/mountandblade2bannerlord/mods/3453
- Football Manager: https://www.fmscout.com/a-wannachup-fm23-instant-result-skin.html , https://forum.lowyat.net/topic/2500038 , https://www.fmscout.com/q-34423-Magicians-tactical-testing-new-ME.html
- RimWorld: https://rimworldwiki.com/wiki/Raid_points
- Infection Free Zone: https://store.steampowered.com/app/1465460/Infection_Free_Zone/
