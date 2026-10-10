# Cognitive load: how much a player can take in, and how deep games teach

**Date:** 2026-10-10. **Status:** research note, not a decision. **Feeds:** [interaction](../design/interaction.md), [demo flow](../design/demo-flow.md), [activities](../design/activities.md), [0020](../decisions/0020-activity-answers.md) (an activity is played hands-on the first time, as a tutorial), research [view-readability](view-readability.md). **Evidence:** one web-research pass on 2026-10-10 from search results only; the fetch tool failed on every host and the container's network policy blocks YouTube, so no primary source was read in that pass and the outside findings are `reported` (many from abstracts quoted in search results). A local session read the transcript of the video the user shared later the same day (CLT-1, now `sourced`). Numbers flagged in the rows (Andersen's on-demand help, Mayer's 3rd-edition table) need a check against the papers. **IDs:** CLT.

The user shared a video introducing cognitive load theory. Our game asks a lot of the player (bodies, needs, wounds, power, noise and light, an economy, three layers), and the window's first verdict was "hard to tell what is going on" ([VR-41](view-readability.md)). This note collects what the theory and the games that teach deep systems say about that.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CLT-1 | Video UpA6RdE0aYo is McGraw Hill PreK-12's "Teaching Strategies: Cognitive Load Theory" (9 January 2019, 2 min 55 s; search listed it as "Learning Science 101"). It covers Sweller, a working memory of limited capacity from which information is discarded or moves on to long-term memory, and schemas built on what the learner already knows (exponents taught from multiplication), with strategic practice and scaffolded feedback. It does not name the three kinds of load | sourced | the video's page and transcript, read by a local session 2026-10-10 |
| CLT-2 | Sweller (1988): means-ends problem solving takes so much processing capacity that little is left for building schemas, and schemas are what separate experts from novices | reported | *Cognitive Science* 12:257–285, doi 10.1207/s15516709cog1202_4 |
| CLT-3 | Goal-free effect: removing a specific goal ("find every unknown" instead of "find x") removes means-ends search and helps learning (Sweller, Mawer and Ward 1983) | reported | *J. Exp. Psych.: General* 112:639–661, via jurnal.uny.ac.id article 26914 |
| CLT-4 | Intrinsic load comes from how many elements interact in the material relative to what the learner knows (for an expert a complex thing is one element); extraneous load comes from how it is presented and what the learner must do with it | reported | Sweller, van Merriënboer and Paas 2019, *Educational Psychology Review* 31:261–292 |
| CLT-5 | Germane load was redefined (2019): it no longer adds to the total; it is working memory moved from extraneous work to the intrinsic task | reported | same as CLT-4; frontiersin.org feduc.2021.645284 |
| CLT-6 | Extraneous load matters mainly when intrinsic load is high; with simple material, poor presentation costs little (Sweller 2010) | reported | *EPR* 22:123–138; learningscientists.org blog 2023/7/6 |
| CLT-7 | Miller (1956): immediate memory holds about seven chunks, almost whatever a chunk contains; absolute judgement of one dimension about 2.5 bits (about six categories) | reported | *Psychological Review* 63:81–97 |
| CLT-8 | Cowan (2001): the central capacity limit is about four chunks (three to five); Miller's seven was rough | reported | *Behavioral and Brain Sciences* 24:87–114 (pubmed 11515286) |
| CLT-9 | Without rehearsal, recall of a three-letter item fell to about 80 % at 3 s, 50 % at 6 s and under 10 % at 18 s (Peterson and Peterson 1959) | reported | *J. Exp. Psych.* 58:193–198, via simplypsychology.org |
| CLT-10 | Chess masters rebuild a real position almost perfectly after 5 s but recall random positions about as poorly as anyone (three to four pieces): expertise is stored chunks (Chase and Simon 1973) | reported | *Cognitive Psychology* 4:55–81 |
| CLT-11 | Worked examples beat problem solving for novices (Sweller and Cooper 1985); a 2023 meta-analysis in mathematics (55 studies) found g = 0.48 | reported | Barbieri et al. 2023, *EPR*, doi 10.1007/s10648-023-09745-1 |
| CLT-12 | Completing and modifying a worked solution beat writing one from scratch (van Merriënboer 1990); fading the example's steps adaptively beat fixed fading, which beat plain problem solving (Salden et al. 2010) | reported | research.utwente.nl (van Merriënboer 1990); en.wikipedia.org/wiki/Expertise_reversal_effect |
| CLT-13 | Split attention: text placed in the diagram beats text beside or after it (Chandler and Sweller 1991); a meta-analysis gives d of about 0.85 (Ginns 2006) | reported | *Cognition and Instruction* 8:293–332; frontiersin.org feduc.2019.00086 |
| CLT-14 | Redundancy depends on the case: spoken plus written text did not lose to written alone, and the result varied with prior knowledge, pacing and pictures (Adesope and Nesbit 2012, 57 studies) | reported | *J. Ed. Psych.* 104:250–263 |
| CLT-15 | The modality effect (narration beats on-screen text) is small: d = 0.24 for retention and 0.25 for transfer in 91 studies, strongest when the system sets the pace and the material is short | reported | Wang, Xie and Li 2016, journal.psych.ac.cn |
| CLT-16 | Transient information: long spoken text did worse than the same text written, because speech vanishes; shortening it restored the advantage (Leahy and Sweller 2011) | reported | *Applied Cognitive Psychology* 25:943–951 |
| CLT-17 | Expertise reversal: help that works for novices loses its effect or hurts experienced learners (Kalyuga, Ayres, Chandler and Sweller 2003) | reported | *Educational Psychologist* 38:23–31 |
| CLT-18 | Minimal guidance fails novices (Kirschner, Sweller and Clark 2006); the reply: good inquiry learning is heavily scaffolded, which lowers load (Hmelo-Silver et al. 2007) | reported | *Educational Psychologist* 41:75–86 and 42:99–107 |
| CLT-19 | Mayer's median effect sizes (2014 handbook): coherence 0.86, signalling 0.41, redundancy 0.86, spatial contiguity 1.10, temporal contiguity 1.22, segmenting 0.79, pre-training 0.75, modality 0.76 | reported | *Cambridge Handbook of Multimedia Learning* (2014), chapters on reducing extraneous and managing essential processing |
| CLT-20 | An independent re-analysis of Mayer's studies (181 studies, 1990–2022) found g = 0.37 overall, falling over time, and least consistent for games, animation and simulations | reported | par.nsf.gov/biblio/10637927 |
| CLT-21 | Load is usually measured by one 9-point mental-effort rating (Paas 1992) | reported | *J. Ed. Psych.* 84:429–434 |
| CLT-22 | NASA-TLX rates six scales (mental, physical, temporal, performance, effort, frustration); "Raw TLX" skips the weighting | reported | Hart and Staveland 1988, *Advances in Psychology* 52:139–183 |
| CLT-23 | Critique: germane load is explained after the fact, there is no reliable measure of the separate loads, and whether they add is unclear (de Jong 2010) | reported | *Instructional Science* 38:105–134 |
| CLT-24 | Andersen et al. (2012), 8 tutorial designs in 3 games, over 45,000 players: tutorials raised play time by up to 29 % in the most complex game and had no significant effect in the two simpler ones | reported | CHI 2012:59–68, grail.cs.washington.edu (chi2012.pdf) |
| CLT-25 | Same study: on-demand help raised play time 16 % and progress 40 % in Foldit, did nothing in one game and hurt in another (figures from a garbled extract; check) | reported | same as CLT-24 |
| CLT-26 | Hodent (*The Gamer's Brain*, 2017) frames game UX as usability plus "engage-ability", built on perception, attention, memory, motivation and emotion | reported | thegamersbrain.com; GDC 2016 talk summary |
| CLT-27 | Valve playtested Portal almost weekly from the first week; Portal 2 splits each mechanic into core parts the player learns before it is used for fun ("checklisting") | reported | gameinformer.com 2010/03/17; gmtk.substack.com "Valve's secret weapon" |
| CLT-28 | Progressive disclosure: the most important options first, the rest on request; designs with more than two levels of disclosure usually test badly | reported | nngroup.com/articles/progressive-disclosure |
| CLT-29 | A target that differs by one feature is found in parallel, whatever the number of distractors; one defined by a conjunction of features needs a serial search (Treisman and Gelade 1980) | reported | *Cognitive Psychology* 12:97–136 |
| CLT-30 | Pre-attentive (one-glance, under 200–250 ms) features include hue, intensity, orientation, size and motion | reported | Healey, csc2.ncsu.edu/faculty/healey/PP |
| CLT-31 | Situation awareness has three levels: perceiving the elements, understanding what they mean, projecting where they are going (Endsley 1995) | reported | *Human Factors* 37:32–64 |
| CLT-32 | Removing the HUD raised immersion for expert shooter players (Iacovides et al. 2015); an ammo count shown on the weapon in the world worked best (Peacocke et al. 2015) | reported | discovery.ucl.ac.uk eprint 1470398; yorku.ca/mack/ieee_gem2015.html |
| CLT-33 | Crusader Kings III's tutorial highlights the real windows in place instead of walls of text; tooltips nest (a highlighted concept opens its own tooltip) with timed or manual locking; built from playtests across experience levels and reused in Victoria 3 | reported | CK3 dev diary 16, forum.paradoxplaza.com; pcgamesn.com (Victoria 3 nested tooltips) |
| CLT-34 | Project Zomboid's moodles show up to four levels by icon and colour, with a sentence on hover and no numbers; RimWorld's learning helper opens a lesson when the related event happens and marks it learned when the player does it | reported | pzfans.com moodles; rimworldwiki.com/wiki/Learning_helper |
| CLT-35 | Gee's learning principles include information "just in time" and "on demand", fish tanks (a simplified system with a few variables visible), sandboxes (real-feeling, low risk) and well-ordered problems | reported | gamesandimpact.org GoodVideoGamesLearning.pdf; psy.gla.ac.uk/~steve/best/gee.html |
| CLT-36 | Dwarf Fortress on Steam (December 2022) added a tutorial, a new interface and mouse support; reviewers said it eased the learning curve without removing it | reported | kitfoxgames.itch.io devlog 460421 |
| CLT-37 | Our window's "hard to tell what is going on" is extraneous load of the conjunction kind: pops, walkers and objects differ by colour plus letter, so each must be searched for; a distinct shape per kind makes it a one-glance task | derived | CLT-29, CLT-30, [VR-30](view-readability.md), [VR-41](view-readability.md) |
| CLT-38 | Our intrinsic load is high, so presentation costs us more than it costs simpler games, and a tutorial pays (it did only in the most complex game tested) | derived | CLT-6, CLT-24 |
| CLT-39 | Teach one layer at a time: start at Individual with one body and a few needs (a fish tank), name the parts before they interact (pre-training), then bring in City and Macro as their own well-ordered problems | derived | CLT-19, CLT-35 |
| CLT-40 | An activity played hands-on the first time (0020) works best as worked example, then completion, then free, faded per player and skipped for anyone who has already done it | derived | CLT-11, CLT-12, CLT-17 |
| CLT-41 | Help just in time and on demand: lessons that open on the event they explain, and nested tooltips that link terms to the glossary; the main interface keeps two levels of disclosure, the depth lives in inspectors and the story explorer | derived | CLT-28, CLT-33, CLT-34, CLT-35 |
| CLT-42 | Labels, events and reasons belong on the map next to their cause, not only in a log at the edge of the screen | derived | CLT-13, CLT-19 |
| CLT-43 | Alerts are transient information: they should stay reviewable and repeats should merge, rather than scroll away | derived | CLT-16 |
| CLT-44 | Show meaning and projection, not only values ("food lasts 3 days", "fuel out at 02:00"); a projection the player sees is the forecaster's belief, never a read of the run-ahead ([FW-71](simulation-and-free-will.md)) | derived | CLT-31 |
| CLT-45 | About four things can be watched at once; the 3–4 pinned windows of 0066 fit that limit | derived | CLT-8 |
| CLT-46 | Experienced players should be able to strip guidance and HUD, with tooltips that can be locked | derived | CLT-17, CLT-32, CLT-33 |
| CLT-47 | Playtests can measure the demo with a Paas rating after each segment, Raw TLX, time to first correct action and errors, reading the scores as rough and expecting smaller effects than lab lessons | estimate | CLT-20, CLT-21, CLT-22, CLT-23 |
| CLT-48 | Found stories (the Tier 0 case "who took the fuel", 0067) can double as worked examples of how the systems interact | estimate | CLT-11 |

## What we take

1. **Fix the extraneous load first** (CLT-37, CLT-38). It is the cheapest and it is what the user saw. A shape per kind (pops, walkers, objects), colour for one variable only, motion only for alerts, labels next to the thing. The window fixes in [view-readability](view-readability.md) already list most of this; the theory says why they matter most for a game as deep as ours.
2. **Teach by layers and by doing** (CLT-39, CLT-40, CLT-48). The demo starts as a fish tank at Individual; each activity's first hands-on run is a worked example that hands over step by step; the first found story shows systems meeting.
3. **Depth on demand, not on screen** (CLT-41, CLT-42, CLT-43, CLT-46). Two levels in the main interface, nested tooltips to the glossary, lessons triggered by events, alerts that stay, guidance that experts can turn off.
4. **Projections over raw numbers** (CLT-44). This meets the free-will note: what the player is shown about the future is someone's belief, labelled as such.
5. **Measure it** (CLT-47), with a light hand: the load measures are rough (CLT-23) and the classroom effect sizes shrink in games (CLT-20).

## Open questions (carried to the design docs)

- [Interaction](../design/interaction.md) Open: alerts kept in a reviewable list with repeats merged, and nested tooltips that link terms to the glossary (CLT-41, CLT-43)?
- [Activities](../design/activities.md) Open: the first hands-on run as worked example, completion, then free, faded per player (CLT-40)?
- [Demo flow](../design/demo-flow.md): start as a fish tank at Individual and measure each segment (CLT-39, CLT-47).

## Sources

- Sweller, J. (1988). Cognitive load during problem solving. *Cognitive Science* 12:257–285
- Sweller, van Merriënboer and Paas (2019). Cognitive Architecture and Instructional Design: 20 Years Later. *Educational Psychology Review* 31:261–292
- Cowan, N. (2001). The magical number 4 in short-term memory. *BBS* 24:87–114; Miller, G. (1956). *Psychological Review* 63:81–97
- Chase, W. and Simon, H. (1973). Perception in chess. *Cognitive Psychology* 4:55–81
- Kalyuga, Ayres, Chandler and Sweller (2003). The expertise reversal effect. *Educational Psychologist* 38:23–31
- Mayer, R. (ed.) (2014). *The Cambridge Handbook of Multimedia Learning*, 2nd ed.
- de Jong, T. (2010). Cognitive load theory, educational research, and instructional design. *Instructional Science* 38:105–134
- Andersen, E. et al. (2012). The impact of tutorials on games of varying complexity. CHI 2012
- Hodent, C. (2017). *The Gamer's Brain*. CRC Press
- Treisman, A. and Gelade, G. (1980). A feature-integration theory of attention. *Cognitive Psychology* 12:97–136; Healey, C., Perception in Visualization
- Endsley, M. (1995). Toward a theory of situation awareness in dynamic systems. *Human Factors* 37:32–64
- Gee, J. P. (2003). *What Video Games Have to Teach Us About Learning and Literacy*
- Paradox, CK3 dev diary 16 (tutorials, tooltips, encyclopedia); rimworldwiki.com Learning helper; Nielsen Norman Group, progressive disclosure
